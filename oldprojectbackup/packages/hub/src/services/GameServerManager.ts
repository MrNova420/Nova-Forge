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
 * Game Server Manager
 * Manages shared game servers for multiplayer games
 */

import { DatabaseService } from '../database/DatabaseService';
import { RedisService } from '../database/RedisService';
import { v4 as uuidv4 } from 'uuid';

export interface GameServer {
  id: string;
  gameId: string;
  region: string;
  host: string;
  port: number;
  status: 'starting' | 'running' | 'stopping' | 'stopped';
  currentPlayers: number;
  maxPlayers: number;
  rooms: number;
  cpuUsage: number;
  memoryUsage: number;
  startedAt: Date;
  lastHeartbeat: Date;
}

export interface GameRoom {
  id: string;
  serverId: string;
  gameId: string;
  name: string;
  hostUserId: string;
  currentPlayers: number;
  maxPlayers: number;
  isPublic: boolean;
  status: 'waiting' | 'starting' | 'playing' | 'finished';
  gameMode?: string;
  settings: Record<string, any>;
  players: RoomPlayer[];
  createdAt: Date;
}

export interface RoomPlayer {
  userId: string;
  username: string;
  deviceId: string;
  joinedAt: Date;
  isHost: boolean;
  isReady: boolean;
  team?: number;
  character?: string;
}

export interface ServerPool {
  region: string;
  totalServers: number;
  activeServers: number;
  totalCapacity: number;
  currentPlayers: number;
  avgCpuUsage: number;
  avgMemoryUsage: number;
}

export class GameServerManager {
  private db = DatabaseService.getInstance();
  private redis = RedisService.getInstance();

  /**
   * Allocate a game server for a published game
   */
  async allocateGameServer(
    gameId: string,
    region = 'us-east-1',
    maxPlayers = 100
  ): Promise<GameServer> {
    // Find available server or create new one
    let server = await this.findAvailableServer(gameId, region);

    if (!server) {
      server = await this.createGameServer(gameId, region, maxPlayers);
    }

    return server;
  }

  /**
   * Create a new game server instance
   */
  private async createGameServer(
    gameId: string,
    region: string,
    maxPlayers: number
  ): Promise<GameServer> {
    const serverId = uuidv4();
    const port = await this.allocatePort();

    const server: GameServer = {
      id: serverId,
      gameId,
      region,
      host: `server-${region}-${serverId.substring(0, 8)}.novaengine.io`,
      port,
      status: 'starting',
      currentPlayers: 0,
      maxPlayers,
      rooms: 0,
      cpuUsage: 0,
      memoryUsage: 0,
      startedAt: new Date(),
      lastHeartbeat: new Date(),
    };

    // Save to database
    await this.db.query(
      `INSERT INTO game_servers (
        id, game_id, region, host, port, status, max_players, started_at, last_heartbeat
      ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9)`,
      [
        server.id,
        server.gameId,
        server.region,
        server.host,
        server.port,
        server.status,
        server.maxPlayers,
        server.startedAt,
        server.lastHeartbeat,
      ]
    );

    // Cache server info
    await this.redis.setJson(`game_server:${serverId}`, server, 300);

    // Add to server pool
    await this.redis.set(`server_pool:${region}:${serverId}`, '1');

    // Start server process (would spawn actual server process)
    await this.startServerProcess(server);

    return server;
  }

  /**
   * Find available server with capacity
   */
  private async findAvailableServer(
    gameId: string,
    region: string
  ): Promise<GameServer | null> {
    const result = await this.db.query<any>(
      `SELECT * FROM game_servers
       WHERE game_id = $1 AND region = $2 AND status = 'running'
       AND current_players < max_players
       ORDER BY current_players ASC
       LIMIT 1`,
      [gameId, region]
    );

    if (result.rows.length === 0) {
      return null;
    }

    const row = result.rows[0];
    return {
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
    };
  }

  /**
   * Create a game room on a server
   */
  async createGameRoom(
    gameId: string,
    hostUserId: string,
    hostUsername: string,
    deviceId: string,
    options: {
      name: string;
      maxPlayers?: number;
      isPublic?: boolean;
      gameMode?: string;
      settings?: Record<string, any>;
    }
  ): Promise<GameRoom> {
    // Allocate server
    const server = await this.allocateGameServer(
      gameId,
      'us-east-1',
      options.maxPlayers || 8
    );

    const roomId = uuidv4();
    const room: GameRoom = {
      id: roomId,
      serverId: server.id,
      gameId,
      name: options.name,
      hostUserId,
      currentPlayers: 1,
      maxPlayers: options.maxPlayers || 8,
      isPublic: options.isPublic !== false,
      status: 'waiting',
      gameMode: options.gameMode,
      settings: options.settings || {},
      players: [
        {
          userId: hostUserId,
          username: hostUsername,
          deviceId,
          joinedAt: new Date(),
          isHost: true,
          isReady: false,
        },
      ],
      createdAt: new Date(),
    };

    // Save to database
    await this.db.query(
      `INSERT INTO game_rooms (
        id, server_id, game_id, name, host_user_id, max_players, is_public,
        status, game_mode, settings, players, created_at
      ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12)`,
      [
        room.id,
        room.serverId,
        room.gameId,
        room.name,
        room.hostUserId,
        room.maxPlayers,
        room.isPublic,
        room.status,
        room.gameMode,
        JSON.stringify(room.settings),
        JSON.stringify(room.players),
        room.createdAt,
      ]
    );

    // Cache room
    await this.redis.setJson(`game_room:${roomId}`, room, 3600);

    // Update server room count
    await this.db.query(
      'UPDATE game_servers SET rooms = rooms + 1, current_players = current_players + 1 WHERE id = $1',
      [server.id]
    );

    return room;
  }

  /**
   * Join a game room
   */
  async joinGameRoom(
    roomId: string,
    userId: string,
    username: string,
    deviceId: string
  ): Promise<GameRoom> {
    const room = await this.getGameRoom(roomId);
    if (!room) {
      throw new Error('Room not found');
    }

    if (room.currentPlayers >= room.maxPlayers) {
      throw new Error('Room is full');
    }

    if (room.status === 'playing' || room.status === 'finished') {
      throw new Error('Room has already started');
    }

    // Check if player is already in room (might be reconnecting from another device)
    const existingPlayer = room.players.find((p) => p.userId === userId);
    if (existingPlayer) {
      // Update device ID for cross-device support
      existingPlayer.deviceId = deviceId;
    } else {
      // Add new player
      room.players.push({
        userId,
        username,
        deviceId,
        joinedAt: new Date(),
        isHost: false,
        isReady: false,
      });
      room.currentPlayers++;
    }

    // Update database
    await this.db.query(
      `UPDATE game_rooms 
       SET players = $1, current_players = $2
       WHERE id = $3`,
      [JSON.stringify(room.players), room.currentPlayers, roomId]
    );

    // Update server player count
    await this.db.query(
      'UPDATE game_servers SET current_players = current_players + 1 WHERE id = $1',
      [room.serverId]
    );

    // Update cache
    await this.redis.setJson(`game_room:${roomId}`, room, 3600);

    return room;
  }

  /**
   * Leave game room
   */
  async leaveGameRoom(roomId: string, userId: string): Promise<void> {
    const room = await this.getGameRoom(roomId);
    if (!room) {
      return;
    }

    // Remove player
    const playerIndex = room.players.findIndex((p) => p.userId === userId);
    if (playerIndex === -1) {
      return;
    }

    room.players.splice(playerIndex, 1);
    room.currentPlayers--;

    // If host left, assign new host
    if (room.hostUserId === userId && room.players.length > 0) {
      room.hostUserId = room.players[0].userId;
      room.players[0].isHost = true;
    }

    // If room is empty, close it
    if (room.currentPlayers === 0) {
      await this.closeGameRoom(roomId);
      return;
    }

    // Update database
    await this.db.query(
      `UPDATE game_rooms 
       SET players = $1, current_players = $2, host_user_id = $3
       WHERE id = $4`,
      [
        JSON.stringify(room.players),
        room.currentPlayers,
        room.hostUserId,
        roomId,
      ]
    );

    // Update server player count
    await this.db.query(
      'UPDATE game_servers SET current_players = current_players - 1 WHERE id = $1',
      [room.serverId]
    );

    // Update cache
    await this.redis.setJson(`game_room:${roomId}`, room, 3600);
  }

  /**
   * Get game room
   */
  async getGameRoom(roomId: string): Promise<GameRoom | null> {
    // Check cache first
    const cached = await this.redis.getJson<GameRoom>(`game_room:${roomId}`);
    if (cached) {
      return cached;
    }

    const result = await this.db.query<any>(
      'SELECT * FROM game_rooms WHERE id = $1',
      [roomId]
    );

    if (result.rows.length === 0) {
      return null;
    }

    const row = result.rows[0];
    const room: GameRoom = {
      id: row.id,
      serverId: row.server_id,
      gameId: row.game_id,
      name: row.name,
      hostUserId: row.host_user_id,
      currentPlayers: row.current_players,
      maxPlayers: row.max_players,
      isPublic: row.is_public,
      status: row.status,
      gameMode: row.game_mode,
      settings: JSON.parse(row.settings),
      players: JSON.parse(row.players),
      createdAt: row.created_at,
    };

    await this.redis.setJson(`game_room:${roomId}`, room, 3600);
    return room;
  }

  /**
   * List available rooms for a game
   */
  async listGameRooms(gameId: string, region?: string): Promise<GameRoom[]> {
    let query = `
      SELECT * FROM game_rooms 
      WHERE game_id = $1 
      AND is_public = true 
      AND status IN ('waiting', 'starting')
      AND current_players < max_players
    `;
    const params: any[] = [gameId];

    if (region) {
      query += ` AND server_id IN (
        SELECT id FROM game_servers WHERE region = $2
      )`;
      params.push(region);
    }

    query += ' ORDER BY created_at DESC LIMIT 50';

    const result = await this.db.query<any>(query, params);

    return result.rows.map((row) => ({
      id: row.id,
      serverId: row.server_id,
      gameId: row.game_id,
      name: row.name,
      hostUserId: row.host_user_id,
      currentPlayers: row.current_players,
      maxPlayers: row.max_players,
      isPublic: row.is_public,
      status: row.status,
      gameMode: row.game_mode,
      settings: JSON.parse(row.settings),
      players: JSON.parse(row.players),
      createdAt: row.created_at,
    }));
  }

  /**
   * Start game in room
   */
  async startGame(roomId: string): Promise<void> {
    await this.db.query(
      `UPDATE game_rooms SET status = 'starting' WHERE id = $1`,
      [roomId]
    );
    await this.redis.del(`game_room:${roomId}`);
  }

  /**
   * Close game room
   */
  async closeGameRoom(roomId: string): Promise<void> {
    const room = await this.getGameRoom(roomId);
    if (!room) {
      return;
    }

    await this.db.query('DELETE FROM game_rooms WHERE id = $1', [roomId]);

    // Update server stats
    await this.db.query(
      `UPDATE game_servers 
       SET rooms = rooms - 1, current_players = current_players - $1 
       WHERE id = $2`,
      [room.currentPlayers, room.serverId]
    );

    await this.redis.del(`game_room:${roomId}`);
  }

  /**
   * Get server pool statistics
   */
  async getServerPoolStats(region?: string): Promise<ServerPool[]> {
    let query = `
      SELECT 
        region,
        COUNT(*) as total_servers,
        SUM(CASE WHEN status = 'running' THEN 1 ELSE 0 END) as active_servers,
        SUM(max_players) as total_capacity,
        SUM(current_players) as current_players,
        AVG(cpu_usage) as avg_cpu_usage,
        AVG(memory_usage) as avg_memory_usage
      FROM game_servers
    `;

    const params: any[] = [];
    if (region) {
      query += ' WHERE region = $1';
      params.push(region);
    }

    query += ' GROUP BY region';

    const result = await this.db.query<any>(query, params);

    return result.rows.map((row) => ({
      region: row.region,
      totalServers: parseInt(row.total_servers, 10),
      activeServers: parseInt(row.active_servers, 10),
      totalCapacity: parseInt(row.total_capacity, 10),
      currentPlayers: parseInt(row.current_players, 10),
      avgCpuUsage: parseFloat(row.avg_cpu_usage) || 0,
      avgMemoryUsage: parseFloat(row.avg_memory_usage) || 0,
    }));
  }

  /**
   * Server heartbeat
   */
  async serverHeartbeat(
    serverId: string,
    stats: {
      cpuUsage: number;
      memoryUsage: number;
      currentPlayers: number;
      rooms: number;
    }
  ): Promise<void> {
    await this.db.query(
      `UPDATE game_servers 
       SET last_heartbeat = NOW(), cpu_usage = $1, memory_usage = $2,
           current_players = $3, rooms = $4
       WHERE id = $5`,
      [
        stats.cpuUsage,
        stats.memoryUsage,
        stats.currentPlayers,
        stats.rooms,
        serverId,
      ]
    );
  }

  /**
   * Allocate port for new server
   */
  private async allocatePort(): Promise<number> {
    // Simple port allocation (would be more sophisticated in production)
    return 10000 + Math.floor(Math.random() * 50000);
  }

  /**
   * Start server process (stub - would actually spawn server)
   */
  private async startServerProcess(server: GameServer): Promise<void> {
    // In production, this would:
    // 1. Pull the game build from storage
    // 2. Spawn a Docker container or VM with the game server
    // 3. Configure networking and ports
    // 4. Start the game server process
    // 5. Monitor server health

    // For now, just mark as running
    await this.db.query(
      `UPDATE game_servers SET status = 'running' WHERE id = $1`,
      [server.id]
    );
  }
}
