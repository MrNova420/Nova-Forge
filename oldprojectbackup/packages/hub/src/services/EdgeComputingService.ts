/**
 * NOVA ENGINE - Proprietary Software
 * 
 * Copyright (c) 2025 Kayden Shawn Massengill. All Rights Reserved.
 * Operating as: WeNova Interactive
 * 
 * This software is proprietary and confidential. Unauthorized copying,
 * modification, distribution, or use of this software, via any medium,
 * is strictly prohibited without prior written permission.
 * 
 * See LICENSE file in the root directory for full license terms.
 */

/**
 * Edge Computing Service
 * Distributes game logic computation to edge nodes closest to players
 * Reduces latency and centralizes server load
 */

import { DatabaseService } from '../database/DatabaseService';
import { RedisService } from '../database/RedisService';

export interface EdgeNode {
  id: string;
  region: string;
  location: {
    latitude: number;
    longitude: number;
    city: string;
    country: string;
  };
  capacity: {
    maxGames: number;
    maxPlayers: number;
    cpuCores: number;
    ramGB: number;
    storageGB: number;
  };
  current: {
    games: number;
    players: number;
    cpuUsage: number;
    ramUsage: number;
    networkLoad: number;
  };
  endpoints: {
    http: string;
    https: string;
    websocket: string;
  };
  status: 'online' | 'offline' | 'maintenance';
  latencyToRegions: Record<string, number>;
}

export interface EdgeDeployment {
  gameId: string;
  edgeNodes: string[];
  replicationStrategy: 'full' | 'partial' | 'on-demand';
  assetCaching: boolean;
  stateSyncInterval: number;
}

export class EdgeComputingService {
  private db = DatabaseService.getInstance();
  private redis = RedisService.getInstance();

  /**
   * Deploy game to edge nodes
   */
  async deployToEdge(
    gameId: string,
    targetRegions: string[],
    strategy: EdgeDeployment['replicationStrategy'] = 'partial'
  ): Promise<EdgeDeployment> {
    const edgeNodes: string[] = [];

    for (const region of targetRegions) {
      // Find best edge node in region
      const node = await this.selectEdgeNode(region);
      if (node) {
        // Deploy game build to edge node
        await this.deployGameToNode(gameId, node.id);
        edgeNodes.push(node.id);
      }
    }

    const deployment: EdgeDeployment = {
      gameId,
      edgeNodes,
      replicationStrategy: strategy,
      assetCaching: true,
      stateSyncInterval: 100, // 100ms
    };

    // Cache deployment info
    await this.redis.setJson(`edge:deployment:${gameId}`, deployment, 3600);

    return deployment;
  }

  /**
   * Select optimal edge node for a region
   */
  private async selectEdgeNode(region: string): Promise<EdgeNode | null> {
    // Get all edge nodes in region
    const result = await this.db.query<any>(
      `SELECT * FROM edge_nodes 
       WHERE region = $1 
       AND status = 'online'
       ORDER BY (current_players::float / capacity_max_players::float) ASC
       LIMIT 1`,
      [region]
    );

    if (result.rows.length === 0) {
      return null;
    }

    const row = result.rows[0];
    return {
      id: row.id,
      region: row.region,
      location: JSON.parse(row.location),
      capacity: JSON.parse(row.capacity),
      current: JSON.parse(row.current),
      endpoints: JSON.parse(row.endpoints),
      status: row.status,
      latencyToRegions: JSON.parse(row.latency_to_regions || '{}'),
    };
  }

  /**
   * Deploy game build to specific edge node
   */
  private async deployGameToNode(
    gameId: string,
    nodeId: string
  ): Promise<void> {
    // In production:
    // 1. Package game build
    // 2. Transfer to edge node
    // 3. Extract and prepare runtime
    // 4. Start game server process

    await this.redis.set(
      `edge:node:${nodeId}:game:${gameId}`,
      'deployed',
      3600
    );
  }

  /**
   * Find closest edge node to player
   */
  async findClosestEdgeNode(
    playerRegion: string,
    playerLatLon?: { latitude: number; longitude: number }
  ): Promise<EdgeNode | null> {
    // Get edge nodes sorted by proximity
    const nodes = await this.getEdgeNodesInRegion(playerRegion);

    if (nodes.length === 0) {
      // No nodes in region, find nearest region
      return this.findNearestRegionNode(playerRegion);
    }

    // If player location provided, use geographic distance
    if (playerLatLon) {
      const sortedByDistance = nodes.sort((a, b) => {
        const distA = this.calculateDistance(
          playerLatLon.latitude,
          playerLatLon.longitude,
          a.location.latitude,
          a.location.longitude
        );
        const distB = this.calculateDistance(
          playerLatLon.latitude,
          playerLatLon.longitude,
          b.location.latitude,
          b.location.longitude
        );
        return distA - distB;
      });
      return sortedByDistance[0];
    }

    // Return least loaded node
    return nodes.sort(
      (a, b) =>
        a.current.players / a.capacity.maxPlayers -
        b.current.players / b.capacity.maxPlayers
    )[0];
  }

  /**
   * Get edge nodes in region
   */
  private async getEdgeNodesInRegion(region: string): Promise<EdgeNode[]> {
    const result = await this.db.query<any>(
      'SELECT * FROM edge_nodes WHERE region = $1 AND status = $2',
      [region, 'online']
    );

    return result.rows.map((row) => ({
      id: row.id,
      region: row.region,
      location: JSON.parse(row.location),
      capacity: JSON.parse(row.capacity),
      current: JSON.parse(row.current),
      endpoints: JSON.parse(row.endpoints),
      status: row.status,
      latencyToRegions: JSON.parse(row.latency_to_regions || '{}'),
    }));
  }

  /**
   * Find nearest region node when no nodes in player's region
   */
  private async findNearestRegionNode(
    playerRegion: string
  ): Promise<EdgeNode | null> {
    // Get all online nodes
    const result = await this.db.query<any>(
      'SELECT * FROM edge_nodes WHERE status = $1',
      ['online']
    );

    const nodes = result.rows.map((row) => ({
      id: row.id,
      region: row.region,
      location: JSON.parse(row.location),
      capacity: JSON.parse(row.capacity),
      current: JSON.parse(row.current),
      endpoints: JSON.parse(row.endpoints),
      status: row.status,
      latencyToRegions: JSON.parse(row.latency_to_regions || '{}'),
    }));

    if (nodes.length === 0) {
      return null;
    }

    // Sort by latency to player region
    const sorted = nodes.sort((a, b) => {
      const latencyA = a.latencyToRegions[playerRegion] || 999;
      const latencyB = b.latencyToRegions[playerRegion] || 999;
      return latencyA - latencyB;
    });

    return sorted[0];
  }

  /**
   * Calculate geographic distance (Haversine formula)
   */
  private calculateDistance(
    lat1: number,
    lon1: number,
    lat2: number,
    lon2: number
  ): number {
    const R = 6371; // Earth's radius in km
    const dLat = this.toRadians(lat2 - lat1);
    const dLon = this.toRadians(lon2 - lon1);

    const a =
      Math.sin(dLat / 2) * Math.sin(dLat / 2) +
      Math.cos(this.toRadians(lat1)) *
        Math.cos(this.toRadians(lat2)) *
        Math.sin(dLon / 2) *
        Math.sin(dLon / 2);

    const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
    return R * c;
  }

  private toRadians(degrees: number): number {
    return degrees * (Math.PI / 180);
  }

  /**
   * Cache game assets at edge
   */
  async cacheAssetsAtEdge(
    gameId: string,
    edgeNodeId: string,
    assetPaths: string[]
  ): Promise<void> {
    // In production, would:
    // 1. Fetch assets from central storage
    // 2. Upload to edge node CDN
    // 3. Configure edge node to serve assets

    for (const assetPath of assetPaths) {
      await this.redis.set(
        `edge:${edgeNodeId}:asset:${gameId}:${assetPath}`,
        'cached',
        86400
      );
    }
  }

  /**
   * Synchronize game state across edge nodes
   */
  async syncStateAcrossEdges(
    gameId: string,
    roomId: string,
    state: any
  ): Promise<{ nodesSynced: number; latency: number }> {
    const startTime = Date.now();

    // Get deployment info
    const deployment = await this.redis.getJson<EdgeDeployment>(
      `edge:deployment:${gameId}`
    );

    if (!deployment) {
      return { nodesSynced: 0, latency: 0 };
    }

    // Publish state update to all edge nodes
    const syncPromises = deployment.edgeNodes.map(async (nodeId) => {
      await this.redis.setJson(`edge:${nodeId}:state:${roomId}`, state, 60);
    });

    await Promise.all(syncPromises);

    const latency = Date.now() - startTime;

    return {
      nodesSynced: deployment.edgeNodes.length,
      latency,
    };
  }

  /**
   * Get edge network statistics
   */
  async getEdgeNetworkStats(): Promise<{
    totalNodes: number;
    onlineNodes: number;
    totalCapacity: number;
    currentLoad: number;
    averageLatency: number;
    regionalDistribution: Record<string, number>;
  }> {
    const result = await this.db.query<any>('SELECT * FROM edge_nodes');

    const nodes = result.rows.map((row) => ({
      region: row.region,
      capacity: JSON.parse(row.capacity),
      current: JSON.parse(row.current),
      status: row.status,
    }));

    const onlineNodes = nodes.filter((n) => n.status === 'online');
    const totalCapacity = onlineNodes.reduce(
      (sum, n) => sum + n.capacity.maxPlayers,
      0
    );
    const currentLoad = onlineNodes.reduce(
      (sum, n) => sum + n.current.players,
      0
    );

    const regionalDistribution: Record<string, number> = {};
    for (const node of onlineNodes) {
      regionalDistribution[node.region] =
        (regionalDistribution[node.region] || 0) + 1;
    }

    return {
      totalNodes: nodes.length,
      onlineNodes: onlineNodes.length,
      totalCapacity,
      currentLoad,
      averageLatency: 0, // Would calculate from actual measurements
      regionalDistribution,
    };
  }

  /**
   * Auto-scale edge nodes based on demand
   */
  async autoScaleEdgeNodes(region: string): Promise<{
    nodesAdded: number;
    nodesRemoved: number;
  }> {
    const nodes = await this.getEdgeNodesInRegion(region);
    const avgLoad =
      nodes.reduce(
        (sum, n) => sum + n.current.players / n.capacity.maxPlayers,
        0
      ) / nodes.length;

    let nodesAdded = 0;
    let nodesRemoved = 0;

    // Scale up if average load > 70%
    if (avgLoad > 0.7) {
      const nodesToAdd = Math.ceil((avgLoad - 0.7) * 10);
      // In production, would provision new edge nodes
      nodesAdded = nodesToAdd;
    }

    // Scale down if average load < 30% and have extra capacity
    if (avgLoad < 0.3 && nodes.length > 2) {
      const nodesToRemove = Math.floor((0.3 - avgLoad) * 10);
      // In production, would decommission underutilized nodes
      nodesRemoved = Math.min(nodesToRemove, nodes.length - 2);
    }

    return { nodesAdded, nodesRemoved };
  }
}
