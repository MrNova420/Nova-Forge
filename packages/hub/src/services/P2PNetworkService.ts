/**
 * Peer-to-Peer Network Service
 * Enables direct player-to-player connections to reduce server load
 * and utilize distributed computing power from player devices
 */

import { DatabaseService } from '../database/DatabaseService';
import { RedisService } from '../database/RedisService';

export interface P2PPeer {
  peerId: string;
  userId: string;
  deviceId: string;
  region: string;
  isHost: boolean;
  capabilities: PeerCapabilities;
  connections: string[]; // Connected peer IDs
  bandwidth: number; // Mbps
  latency: Record<string, number>; // Latency to other peers
  role: 'mesh-node' | 'relay' | 'edge-compute' | 'client';
}

export interface PeerCapabilities {
  cpuCores: number;
  cpuSpeed: number; // GHz
  ramAvailable: number; // MB
  gpuAvailable: boolean;
  bandwidth: number; // Mbps
  canHost: boolean;
  canRelay: boolean;
  canCompute: boolean;
}

export interface P2PTopology {
  roomId: string;
  architecture: 'full-mesh' | 'star' | 'hybrid' | 'hierarchical';
  peers: P2PPeer[];
  relayPeers: string[];
  computePeers: string[];
  hostPeer: string;
}

export interface ComputeTask {
  id: string;
  type: 'physics' | 'ai' | 'pathfinding' | 'terrain-gen' | 'state-sync';
  data: any;
  assignedPeer?: string;
  priority: number;
  estimatedMs: number;
}

export class P2PNetworkService {
  private db = DatabaseService.getInstance();
  private redis = RedisService.getInstance();

  /**
   * Initialize P2P topology for a game room
   */
  async initializeP2PTopology(
    roomId: string,
    peers: P2PPeer[],
    maxPlayers: number
  ): Promise<P2PTopology> {
    // Determine best architecture based on player count
    let architecture: P2PTopology['architecture'];

    if (maxPlayers <= 8) {
      architecture = 'full-mesh'; // All players connected to each other
    } else if (maxPlayers <= 32) {
      architecture = 'hybrid'; // Some mesh, some relayed
    } else {
      architecture = 'hierarchical'; // Multiple layers with relay nodes
    }

    // Sort peers by capabilities
    const sortedPeers = this.sortPeersByCapability(peers);

    // Select host peer (best capabilities)
    const hostPeer = sortedPeers[0];

    // Select relay peers (high bandwidth, stable connection)
    const relayPeers = sortedPeers
      .filter((p) => p.capabilities.canRelay && p.capabilities.bandwidth > 10)
      .slice(0, Math.ceil(maxPlayers / 10))
      .map((p) => p.peerId);

    // Select compute peers (high CPU/GPU)
    const computePeers = sortedPeers
      .filter((p) => p.capabilities.canCompute && p.capabilities.cpuCores >= 4)
      .slice(0, Math.min(5, Math.ceil(maxPlayers / 4)))
      .map((p) => p.peerId);

    const topology: P2PTopology = {
      roomId,
      architecture,
      peers,
      relayPeers,
      computePeers,
      hostPeer: hostPeer.peerId,
    };

    // Cache topology
    await this.redis.setJson(`p2p:topology:${roomId}`, topology, 3600);

    return topology;
  }

  /**
   * Establish P2P connections between peers
   */
  async establishConnections(
    topology: P2PTopology
  ): Promise<Map<string, string[]>> {
    const connections = new Map<string, string[]>();

    switch (topology.architecture) {
      case 'full-mesh':
        return this.createFullMesh(topology.peers);

      case 'star':
        return this.createStarTopology(topology.peers, topology.hostPeer);

      case 'hybrid':
        return this.createHybridTopology(
          topology.peers,
          topology.relayPeers,
          topology.hostPeer
        );

      case 'hierarchical':
        return this.createHierarchicalTopology(
          topology.peers,
          topology.relayPeers,
          topology.hostPeer
        );
    }
  }

  /**
   * Full mesh - all peers connected to all peers
   */
  private createFullMesh(peers: P2PPeer[]): Map<string, string[]> {
    const connections = new Map<string, string[]>();

    for (const peer of peers) {
      const otherPeers = peers
        .filter((p) => p.peerId !== peer.peerId)
        .map((p) => p.peerId);
      connections.set(peer.peerId, otherPeers);
    }

    return connections;
  }

  /**
   * Star topology - all peers connected to host
   */
  private createStarTopology(
    peers: P2PPeer[],
    hostPeerId: string
  ): Map<string, string[]> {
    const connections = new Map<string, string[]>();

    // Host connects to all
    connections.set(
      hostPeerId,
      peers.filter((p) => p.peerId !== hostPeerId).map((p) => p.peerId)
    );

    // Others only connect to host
    for (const peer of peers) {
      if (peer.peerId !== hostPeerId) {
        connections.set(peer.peerId, [hostPeerId]);
      }
    }

    return connections;
  }

  /**
   * Hybrid topology - mesh for low latency groups, relay for others
   */
  private createHybridTopology(
    peers: P2PPeer[],
    relayPeers: string[],
    hostPeerId: string
  ): Map<string, string[]> {
    const connections = new Map<string, string[]>();

    // Group peers by region
    const regionGroups = new Map<string, P2PPeer[]>();
    for (const peer of peers) {
      if (!regionGroups.has(peer.region)) {
        regionGroups.set(peer.region, []);
      }
      regionGroups.get(peer.region)!.push(peer);
    }

    // Create mesh within each region
    for (const [region, regionPeers] of regionGroups) {
      if (regionPeers.length <= 8) {
        // Small group - full mesh
        for (const peer of regionPeers) {
          connections.set(
            peer.peerId,
            regionPeers
              .filter((p) => p.peerId !== peer.peerId)
              .map((p) => p.peerId)
          );
        }
      } else {
        // Large group - use relay
        const regionRelay = regionPeers.find((p) =>
          relayPeers.includes(p.peerId)
        )?.peerId;
        if (regionRelay) {
          for (const peer of regionPeers) {
            if (peer.peerId === regionRelay) {
              connections.set(
                peer.peerId,
                regionPeers
                  .filter((p) => p.peerId !== peer.peerId)
                  .map((p) => p.peerId)
              );
            } else {
              connections.set(peer.peerId, [regionRelay]);
            }
          }
        }
      }
    }

    // Connect relays to each other and host
    for (const relayId of relayPeers) {
      const existing = connections.get(relayId) || [];
      const otherRelays = relayPeers.filter((r) => r !== relayId);
      connections.set(relayId, [
        ...new Set([...existing, hostPeerId, ...otherRelays]),
      ]);
    }

    return connections;
  }

  /**
   * Hierarchical topology - multiple layers of relays
   */
  private createHierarchicalTopology(
    peers: P2PPeer[],
    relayPeers: string[],
    hostPeerId: string
  ): Map<string, string[]> {
    const connections = new Map<string, string[]>();

    // Layer 1: Host
    const layer1 = [hostPeerId];

    // Layer 2: Primary relays (connect to host)
    const layer2 = relayPeers.slice(0, 5);

    // Layer 3: Secondary relays (connect to primary relays)
    const layer3 = relayPeers.slice(5, 15);

    // Layer 4: All other peers (connect to nearest relay)
    const layer4 = peers
      .filter(
        (p) =>
          p.peerId !== hostPeerId &&
          !layer2.includes(p.peerId) &&
          !layer3.includes(p.peerId)
      )
      .map((p) => p.peerId);

    // Host connects to all layer 2
    connections.set(hostPeerId, layer2);

    // Layer 2 connects to host and layer 3
    for (const relay of layer2) {
      const layer3Assigned = layer3.slice(
        layer2.indexOf(relay) * Math.ceil(layer3.length / layer2.length),
        (layer2.indexOf(relay) + 1) * Math.ceil(layer3.length / layer2.length)
      );
      connections.set(relay, [hostPeerId, ...layer3Assigned]);
    }

    // Layer 3 connects to their layer 2 parent and layer 4 clients
    for (const relay of layer3) {
      const parent =
        layer2[
          Math.floor(
            layer3.indexOf(relay) / Math.ceil(layer3.length / layer2.length)
          )
        ];
      const clients = layer4.slice(
        layer3.indexOf(relay) * Math.ceil(layer4.length / layer3.length),
        (layer3.indexOf(relay) + 1) * Math.ceil(layer4.length / layer3.length)
      );
      connections.set(relay, [parent, ...clients]);
    }

    // Layer 4 connects to their assigned relay
    for (const client of layer4) {
      const relayIndex = Math.floor(
        (layer4.indexOf(client) / layer4.length) * layer3.length
      );
      const relay = layer3[relayIndex] || layer2[0];
      connections.set(client, [relay]);
    }

    return connections;
  }

  /**
   * Distribute computation across peers
   */
  async distributeComputation(
    topology: P2PTopology,
    tasks: ComputeTask[]
  ): Promise<Map<string, ComputeTask[]>> {
    const distribution = new Map<string, ComputeTask[]>();

    // Get compute-capable peers
    const computePeers = topology.peers.filter((p) =>
      topology.computePeers.includes(p.peerId)
    );

    if (computePeers.length === 0) {
      // No compute peers, assign to host
      distribution.set(topology.hostPeer, tasks);
      return distribution;
    }

    // Sort tasks by priority and estimated time
    const sortedTasks = tasks.sort((a, b) => {
      if (a.priority !== b.priority) return b.priority - a.priority;
      return a.estimatedMs - b.estimatedMs;
    });

    // Calculate peer compute scores
    const peerScores = computePeers.map((peer) => ({
      peerId: peer.peerId,
      score:
        peer.capabilities.cpuCores * peer.capabilities.cpuSpeed +
        (peer.capabilities.gpuAvailable ? 100 : 0),
      currentLoad: 0,
    }));

    // Distribute tasks based on peer capabilities
    for (const task of sortedTasks) {
      // Find peer with lowest current load relative to capability
      const selectedPeer = peerScores.reduce((best, current) =>
        current.currentLoad / current.score < best.currentLoad / best.score
          ? current
          : best
      );

      if (!distribution.has(selectedPeer.peerId)) {
        distribution.set(selectedPeer.peerId, []);
      }
      distribution.get(selectedPeer.peerId)!.push(task);

      // Update load
      selectedPeer.currentLoad += task.estimatedMs;
      task.assignedPeer = selectedPeer.peerId;
    }

    // Cache task distribution
    await this.redis.setJson(
      `p2p:tasks:${topology.roomId}`,
      Object.fromEntries(distribution),
      60
    );

    return distribution;
  }

  /**
   * Handle peer failure and reassignment
   */
  async handlePeerFailure(
    topology: P2PTopology,
    failedPeerId: string
  ): Promise<{
    newTopology: P2PTopology;
    tasksReassigned: number;
  }> {
    // Remove failed peer
    topology.peers = topology.peers.filter((p) => p.peerId !== failedPeerId);

    // If failed peer was host, select new host
    if (topology.hostPeer === failedPeerId) {
      const newHost = this.sortPeersByCapability(topology.peers)[0];
      topology.hostPeer = newHost.peerId;
    }

    // Remove from relay peers
    topology.relayPeers = topology.relayPeers.filter(
      (id) => id !== failedPeerId
    );

    // Remove from compute peers
    topology.computePeers = topology.computePeers.filter(
      (id) => id !== failedPeerId
    );

    // Reassign tasks that were on failed peer
    const tasksKey = `p2p:tasks:${topology.roomId}`;
    const tasks =
      await this.redis.getJson<Record<string, ComputeTask[]>>(tasksKey);
    let tasksReassigned = 0;

    if (tasks && tasks[failedPeerId]) {
      const failedTasks = tasks[failedPeerId];
      delete tasks[failedPeerId];

      // Redistribute failed tasks
      const taskArray = Object.values(tasks).flat();
      taskArray.push(...failedTasks);
      const redistributed = await this.distributeComputation(
        topology,
        taskArray
      );

      tasksReassigned = failedTasks.length;
    }

    // Update topology
    await this.redis.setJson(`p2p:topology:${topology.roomId}`, topology, 3600);

    return {
      newTopology: topology,
      tasksReassigned,
    };
  }

  /**
   * Optimize topology based on measured latencies
   */
  async optimizeTopology(topology: P2PTopology): Promise<P2PTopology> {
    // Measure actual latencies between peers
    const latencyMatrix = await this.measureLatencies(topology);

    // Identify high-latency connections
    const highLatencyPairs: Array<[string, string]> = [];

    for (const [peerId, latencies] of Object.entries(latencyMatrix)) {
      for (const [otherPeerId, latency] of Object.entries(latencies)) {
        if (latency > 200) {
          // >200ms is high latency
          highLatencyPairs.push([peerId, otherPeerId]);
        }
      }
    }

    // Add relay nodes for high-latency pairs
    if (
      highLatencyPairs.length > 0 &&
      topology.relayPeers.length < topology.peers.length / 4
    ) {
      // Find potential relay candidates with good connectivity
      const candidates = topology.peers
        .filter((p) => {
          const avgLatency =
            Object.values(latencyMatrix[p.peerId] || {}).reduce(
              (sum, l) => sum + l,
              0
            ) / Object.keys(latencyMatrix[p.peerId] || {}).length;
          return avgLatency < 100 && p.capabilities.canRelay;
        })
        .map((p) => p.peerId);

      // Add best candidates as relay peers
      topology.relayPeers.push(...candidates.slice(0, 3));
    }

    // Update topology
    await this.redis.setJson(`p2p:topology:${topology.roomId}`, topology, 3600);

    return topology;
  }

  /**
   * Measure latencies between peers
   */
  private async measureLatencies(
    topology: P2PTopology
  ): Promise<Record<string, Record<string, number>>> {
    // In production, would use actual RTT measurements
    // For now, return cached or estimated values
    const cached = await this.redis.getJson<
      Record<string, Record<string, number>>
    >(`p2p:latencies:${topology.roomId}`);

    return cached || {};
  }

  /**
   * Sort peers by capability (best first)
   */
  private sortPeersByCapability(peers: P2PPeer[]): P2PPeer[] {
    return peers.slice().sort((a, b) => {
      const scoreA =
        a.capabilities.cpuCores * a.capabilities.cpuSpeed * 10 +
        a.capabilities.ramAvailable +
        a.capabilities.bandwidth * 100 +
        (a.capabilities.gpuAvailable ? 1000 : 0) +
        (a.capabilities.canHost ? 500 : 0);

      const scoreB =
        b.capabilities.cpuCores * b.capabilities.cpuSpeed * 10 +
        b.capabilities.ramAvailable +
        b.capabilities.bandwidth * 100 +
        (b.capabilities.gpuAvailable ? 1000 : 0) +
        (b.capabilities.canHost ? 500 : 0);

      return scoreB - scoreA;
    });
  }

  /**
   * Calculate network bandwidth requirements
   */
  calculateBandwidthRequirements(
    playerCount: number,
    updateRate: number // Updates per second
  ): {
    perPlayer: number; // Kbps
    total: number; // Kbps
  } {
    // Estimate state update size
    const stateUpdateSize = 1024; // 1KB per update
    const perPlayerBandwidth = (stateUpdateSize * updateRate * 8) / 1000; // Kbps

    // In full mesh, each player sends to all others
    const totalBandwidth = perPlayerBandwidth * playerCount * (playerCount - 1);

    return {
      perPlayer: perPlayerBandwidth,
      total: totalBandwidth,
    };
  }

  /**
   * Enable peer to share compute resources
   */
  async enableComputeSharing(
    peerId: string,
    maxCpuPercent: number,
    maxRamMB: number
  ): Promise<void> {
    await this.redis.setJson(
      `p2p:compute:${peerId}`,
      {
        maxCpuPercent,
        maxRamMB,
        enabled: true,
        tasksCompleted: 0,
      },
      3600
    );
  }

  /**
   * Track compute contribution for rewards/credits
   */
  async trackComputeContribution(
    peerId: string,
    taskId: string,
    cpuMs: number,
    ramMBSeconds: number
  ): Promise<void> {
    const key = `p2p:contribution:${peerId}`;
    const contribution = (await this.redis.getJson<any>(key)) || {
      totalCpuMs: 0,
      totalRamMBSeconds: 0,
      tasksCompleted: 0,
    };

    contribution.totalCpuMs += cpuMs;
    contribution.totalRamMBSeconds += ramMBSeconds;
    contribution.tasksCompleted += 1;

    await this.redis.setJson(key, contribution, 86400); // 24 hours
  }
}
