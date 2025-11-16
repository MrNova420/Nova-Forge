/**
 * Load Balancer Service
 * Distributes player connections across multiple game servers for optimal performance
 */

import { DatabaseService } from '../database/DatabaseService';
import { RedisService } from '../database/RedisService';
import { GameServerManager, GameServer } from './GameServerManager';

export interface LoadBalancingStrategy {
  type:
    | 'round-robin'
    | 'least-connections'
    | 'geographic'
    | 'resource-based'
    | 'hybrid';
  weights?: Record<string, number>;
}

export interface ServerLoad {
  serverId: string;
  region: string;
  cpuUsage: number;
  memoryUsage: number;
  networkLatency: number;
  currentPlayers: number;
  maxPlayers: number;
  load: number; // 0-1 normalized load score
}

export interface PlayerConnection {
  playerId: string;
  region: string;
  latency: number;
  bandwidth: number;
}

export class LoadBalancerService {
  private db = DatabaseService.getInstance();
  private redis = RedisService.getInstance();
  private serverManager = new GameServerManager();

  /**
   * Select optimal server for a player
   */
  async selectOptimalServer(
    gameId: string,
    playerConnection: PlayerConnection,
    strategy: LoadBalancingStrategy = { type: 'hybrid' }
  ): Promise<GameServer> {
    // Get all available servers
    const servers = await this.getAvailableServers(
      gameId,
      playerConnection.region
    );

    if (servers.length === 0) {
      // No servers available, create new one
      return this.serverManager.allocateGameServer(
        gameId,
        playerConnection.region
      );
    }

    // Calculate load for each server
    const serverLoads = await Promise.all(
      servers.map((server) =>
        this.calculateServerLoad(server, playerConnection)
      )
    );

    // Select best server based on strategy
    let selectedServer: GameServer;

    switch (strategy.type) {
      case 'round-robin':
        selectedServer = await this.selectRoundRobin(servers);
        break;

      case 'least-connections':
        selectedServer = this.selectLeastConnections(servers, serverLoads);
        break;

      case 'geographic':
        selectedServer = await this.selectGeographic(
          servers,
          playerConnection.region
        );
        break;

      case 'resource-based':
        selectedServer = this.selectResourceBased(servers, serverLoads);
        break;

      case 'hybrid':
      default:
        selectedServer = this.selectHybrid(
          servers,
          serverLoads,
          playerConnection
        );
        break;
    }

    // Reserve slot on selected server
    await this.reserveServerSlot(selectedServer.id);

    return selectedServer;
  }

  /**
   * Get available servers with capacity
   */
  private async getAvailableServers(
    gameId: string,
    region?: string
  ): Promise<GameServer[]> {
    let query = `
      SELECT * FROM game_servers 
      WHERE game_id = $1 
      AND status = 'running'
      AND current_players < max_players
    `;
    const params: any[] = [gameId];

    if (region) {
      // Prefer same region, but include nearby regions
      const nearbyRegions = this.getNearbyRegions(region);
      query += ` AND region = ANY($2)`;
      params.push(nearbyRegions);
    }

    query += ' ORDER BY current_players ASC';

    const result = await this.db.query<any>(query, params);

    return result.rows.map((row) => ({
      id: row.id,
      gameId: row.game_id,
      region: row.region,
      host: row.host,
      port: row.port,
      status: row.status,
      currentPlayers: row.current_players,
      maxPlayers: row.max_players,
      rooms: row.rooms,
      cpuUsage: row.cpu_usage,
      memoryUsage: row.memory_usage,
      startedAt: row.started_at,
      lastHeartbeat: row.last_heartbeat,
    }));
  }

  /**
   * Calculate server load score
   */
  private async calculateServerLoad(
    server: GameServer,
    playerConnection: PlayerConnection
  ): Promise<ServerLoad> {
    // Calculate geographic latency estimate
    const geoLatency = this.estimateLatency(
      server.region,
      playerConnection.region
    );

    // Calculate resource usage
    const cpuLoad = server.cpuUsage / 100;
    const memoryLoad = server.memoryUsage / 100;
    const playerLoad = server.currentPlayers / server.maxPlayers;

    // Combined load score (0-1, lower is better)
    const load =
      cpuLoad * 0.3 +
      memoryLoad * 0.2 +
      playerLoad * 0.3 +
      Math.min(geoLatency / 200, 1) * 0.2;

    return {
      serverId: server.id,
      region: server.region,
      cpuUsage: server.cpuUsage,
      memoryUsage: server.memoryUsage,
      networkLatency: geoLatency,
      currentPlayers: server.currentPlayers,
      maxPlayers: server.maxPlayers,
      load,
    };
  }

  /**
   * Round-robin selection
   */
  private async selectRoundRobin(servers: GameServer[]): Promise<GameServer> {
    const counter = await this.redis.get('lb:round_robin_counter');
    const index = counter ? parseInt(counter, 10) % servers.length : 0;
    await this.redis.set(
      'lb:round_robin_counter',
      ((index + 1) % servers.length).toString()
    );
    return servers[index];
  }

  /**
   * Least connections selection
   */
  private selectLeastConnections(
    servers: GameServer[],
    loads: ServerLoad[]
  ): GameServer {
    let minLoad = Infinity;
    let selectedIndex = 0;

    for (let i = 0; i < loads.length; i++) {
      if (loads[i].currentPlayers < minLoad) {
        minLoad = loads[i].currentPlayers;
        selectedIndex = i;
      }
    }

    return servers[selectedIndex];
  }

  /**
   * Geographic selection (lowest latency)
   */
  private async selectGeographic(
    servers: GameServer[],
    region: string
  ): Promise<GameServer> {
    // Prefer same region
    const sameRegion = servers.find((s) => s.region === region);
    if (sameRegion && sameRegion.currentPlayers < sameRegion.maxPlayers * 0.8) {
      return sameRegion;
    }

    // Find nearest region with capacity
    const nearbyRegions = this.getNearbyRegions(region);
    for (const nearbyRegion of nearbyRegions) {
      const server = servers.find((s) => s.region === nearbyRegion);
      if (server && server.currentPlayers < server.maxPlayers * 0.8) {
        return server;
      }
    }

    return servers[0];
  }

  /**
   * Resource-based selection (lowest CPU/memory usage)
   */
  private selectResourceBased(
    servers: GameServer[],
    loads: ServerLoad[]
  ): GameServer {
    let minLoad = Infinity;
    let selectedIndex = 0;

    for (let i = 0; i < loads.length; i++) {
      const resourceLoad = (loads[i].cpuUsage + loads[i].memoryUsage) / 2;
      if (resourceLoad < minLoad) {
        minLoad = resourceLoad;
        selectedIndex = i;
      }
    }

    return servers[selectedIndex];
  }

  /**
   * Hybrid selection (balanced approach)
   */
  private selectHybrid(
    servers: GameServer[],
    loads: ServerLoad[],
    playerConnection: PlayerConnection
  ): GameServer {
    // Find servers that meet minimum criteria
    const viableServers = loads
      .map((load, index) => ({ server: servers[index], load }))
      .filter((item) => {
        // Filter out overloaded servers
        return (
          item.load.cpuUsage < 80 &&
          item.load.memoryUsage < 80 &&
          item.load.currentPlayers < item.load.maxPlayers * 0.9
        );
      })
      .sort((a, b) => a.load.load - b.load.load);

    if (viableServers.length === 0) {
      // All servers stressed, pick least loaded
      return this.selectLeastConnections(servers, loads);
    }

    // Return best server
    return viableServers[0].server;
  }

  /**
   * Reserve a slot on server
   */
  private async reserveServerSlot(serverId: string): Promise<void> {
    // Increment player count temporarily to prevent race conditions
    await this.db.query(
      'UPDATE game_servers SET current_players = current_players + 1 WHERE id = $1',
      [serverId]
    );

    // Set TTL on reservation
    await this.redis.set(`server:${serverId}:reservation`, '1', 30); // 30 second reservation
  }

  /**
   * Auto-scale servers based on demand
   */
  async autoScaleServers(gameId: string): Promise<{
    serversAdded: number;
    serversRemoved: number;
  }> {
    const servers = await this.getAvailableServers(gameId);
    let serversAdded = 0;
    let serversRemoved = 0;

    // Calculate total capacity and usage
    const totalCapacity = servers.reduce((sum, s) => sum + s.maxPlayers, 0);
    const totalPlayers = servers.reduce((sum, s) => sum + s.currentPlayers, 0);
    const utilizationRate = totalPlayers / totalCapacity;

    // Scale up if utilization > 70%
    if (utilizationRate > 0.7) {
      const serversNeeded = Math.ceil(
        (totalPlayers * 1.3 - totalCapacity) / 100
      );
      for (let i = 0; i < serversNeeded; i++) {
        await this.serverManager.allocateGameServer(gameId, 'us-east-1', 100);
        serversAdded++;
      }
    }

    // Scale down if utilization < 30% and multiple servers exist
    if (utilizationRate < 0.3 && servers.length > 2) {
      // Find empty servers to remove
      const emptyServers = servers.filter((s) => s.currentPlayers === 0);
      const removeCount = Math.min(
        emptyServers.length,
        Math.floor(servers.length / 3)
      );

      for (let i = 0; i < removeCount; i++) {
        await this.shutdownServer(emptyServers[i].id);
        serversRemoved++;
      }
    }

    return { serversAdded, serversRemoved };
  }

  /**
   * Shutdown server gracefully
   */
  private async shutdownServer(serverId: string): Promise<void> {
    await this.db.query(
      `UPDATE game_servers SET status = 'stopping' WHERE id = $1`,
      [serverId]
    );

    // Wait for players to drain (would implement migration in production)
    // After graceful period, mark as stopped
    setTimeout(async () => {
      await this.db.query(
        `UPDATE game_servers SET status = 'stopped' WHERE id = $1`,
        [serverId]
      );
    }, 60000); // 1 minute grace period
  }

  /**
   * Get nearby regions for failover
   */
  private getNearbyRegions(region: string): string[] {
    const regionMap: Record<string, string[]> = {
      'us-east-1': ['us-east-1', 'us-west-1', 'us-west-2'],
      'us-west-1': ['us-west-1', 'us-west-2', 'us-east-1'],
      'us-west-2': ['us-west-2', 'us-west-1', 'us-east-1'],
      'eu-west-1': ['eu-west-1', 'eu-central-1', 'eu-north-1'],
      'eu-central-1': ['eu-central-1', 'eu-west-1', 'eu-north-1'],
      'ap-southeast-1': ['ap-southeast-1', 'ap-northeast-1', 'ap-south-1'],
      'ap-northeast-1': ['ap-northeast-1', 'ap-southeast-1', 'ap-south-1'],
    };

    return regionMap[region] || [region];
  }

  /**
   * Estimate latency between regions (simplified)
   */
  private estimateLatency(serverRegion: string, playerRegion: string): number {
    if (serverRegion === playerRegion) return 10; // Same region

    // Simplified latency map (in production, use actual measurements)
    const latencyMap: Record<string, Record<string, number>> = {
      'us-east-1': { 'us-west-1': 60, 'eu-west-1': 80, 'ap-southeast-1': 200 },
      'us-west-1': { 'us-east-1': 60, 'eu-west-1': 120, 'ap-southeast-1': 150 },
      'eu-west-1': { 'us-east-1': 80, 'us-west-1': 120, 'ap-southeast-1': 160 },
      'ap-southeast-1': {
        'us-east-1': 200,
        'us-west-1': 150,
        'eu-west-1': 160,
      },
    };

    return latencyMap[serverRegion]?.[playerRegion] || 100;
  }

  /**
   * Monitor and rebalance load
   */
  async rebalanceLoad(gameId: string): Promise<{
    playersMigrated: number;
    serversRebalanced: number;
  }> {
    // Get all servers
    const servers = await this.getAvailableServers(gameId);
    const serverLoads = await Promise.all(
      servers.map((s) =>
        this.calculateServerLoad(s, {
          playerId: 'system',
          region: s.region,
          latency: 0,
          bandwidth: 1000,
        })
      )
    );

    // Identify overloaded and underutilized servers
    const overloaded = serverLoads.filter(
      (l) => l.cpuUsage > 80 || l.memoryUsage > 80
    );
    const underutilized = serverLoads.filter(
      (l) => l.cpuUsage < 30 && l.memoryUsage < 30 && l.currentPlayers > 0
    );

    // In production, would implement player migration
    // For now, just return stats
    return {
      playersMigrated: 0,
      serversRebalanced: overloaded.length,
    };
  }
}
