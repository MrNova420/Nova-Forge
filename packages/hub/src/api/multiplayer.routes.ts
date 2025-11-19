/**
 * Multiplayer API Routes
 * Handles multiplayer game sessions, rooms, and matchmaking
 */

import { FastifyInstance, FastifyRequest, FastifyReply } from 'fastify';
import { GameServerManager } from '../services/GameServerManager';
import { z } from 'zod';

const createRoomSchema = z.object({
  gameId: z.string().uuid(),
  name: z.string().min(1).max(200),
  maxPlayers: z.number().min(2).max(100).optional(),
  isPublic: z.boolean().optional(),
  gameMode: z.string().optional(),
  settings: z.record(z.any()).optional(),
});

const joinRoomSchema = z.object({
  roomId: z.string().uuid(),
  deviceId: z.string(),
});

export async function multiplayerRoutes(server: FastifyInstance) {
  const serverManager = new GameServerManager();

  // Get server pool statistics
  server.get(
    '/servers/stats',
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const { region } = request.query as { region?: string };
      const stats = await serverManager.getServerPoolStats(region);
      return { success: true, stats };
    }
  );

  // Create a multiplayer room
  server.post(
    '/rooms',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { userId } = request.user as any;
      const data = createRoomSchema.parse(request.body);

      // Get user info
      const { AuthService } = await import('../auth/AuthService');
      const authService = new AuthService();
      const user = await authService.getUserById(userId);

      if (!user) {
        return reply.code(404).send({ error: true, message: 'User not found' });
      }

      // Generate device ID if not provided
      const deviceId = `device-${Date.now()}-${Math.random().toString(36).substring(7)}`;

      const room = await serverManager.createGameRoom(
        data.gameId,
        userId,
        user.username,
        deviceId,
        {
          name: data.name,
          maxPlayers: data.maxPlayers,
          isPublic: data.isPublic,
          gameMode: data.gameMode,
          settings: data.settings,
        }
      );

      return reply.code(201).send({ success: true, room });
    }
  );

  // List all available lobbies (for multiplayer module)
  server.get(
    '/lobbies',
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const { gameId } = request.query as { gameId?: string };

      // If gameId provided, filter by game
      if (gameId) {
        const rooms = await serverManager.listGameRooms(gameId);
        return rooms;
      }

      // Return empty list for now - would need to aggregate all rooms across games
      return [];
    }
  );

  // List available rooms for a game
  server.get(
    '/rooms/:gameId',
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const { gameId } = request.params as { gameId: string };
      const { region } = request.query as { region?: string };

      const rooms = await serverManager.listGameRooms(gameId, region);

      return { success: true, rooms };
    }
  );

  // Get room details
  server.get(
    '/rooms/:gameId/:roomId',
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { roomId } = request.params as { roomId: string };

      const room = await serverManager.getGameRoom(roomId);

      if (!room) {
        return reply.code(404).send({ error: true, message: 'Room not found' });
      }

      return { success: true, room };
    }
  );

  // Join a room
  server.post(
    '/rooms/join',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { userId } = request.user as any;
      const data = joinRoomSchema.parse(request.body);

      // Get user info
      const { AuthService } = await import('../auth/AuthService');
      const authService = new AuthService();
      const user = await authService.getUserById(userId);

      if (!user) {
        return reply.code(404).send({ error: true, message: 'User not found' });
      }

      try {
        const room = await serverManager.joinGameRoom(
          data.roomId,
          userId,
          user.username,
          data.deviceId
        );

        return { success: true, room };
      } catch (error: any) {
        return reply.code(400).send({
          error: true,
          message: error.message,
        });
      }
    }
  );

  // Leave a room
  server.post(
    '/rooms/:roomId/leave',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const { roomId } = request.params as { roomId: string };
      const { userId } = request.user as any;

      await serverManager.leaveGameRoom(roomId, userId);

      return { success: true, message: 'Left room successfully' };
    }
  );

  // Start game in room (host only)
  server.post(
    '/rooms/:roomId/start',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { roomId } = request.params as { roomId: string };
      const { userId } = request.user as any;

      const room = await serverManager.getGameRoom(roomId);

      if (!room) {
        return reply.code(404).send({ error: true, message: 'Room not found' });
      }

      if (room.hostUserId !== userId) {
        return reply
          .code(403)
          .send({ error: true, message: 'Only the host can start the game' });
      }

      await serverManager.startGame(roomId);

      return { success: true, message: 'Game starting...' };
    }
  );

  // Close room (host only)
  server.delete(
    '/rooms/:roomId',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { roomId } = request.params as { roomId: string };
      const { userId } = request.user as any;

      const room = await serverManager.getGameRoom(roomId);

      if (!room) {
        return reply.code(404).send({ error: true, message: 'Room not found' });
      }

      if (room.hostUserId !== userId) {
        return reply
          .code(403)
          .send({ error: true, message: 'Only the host can close the room' });
      }

      await serverManager.closeGameRoom(roomId);

      return { success: true, message: 'Room closed' };
    }
  );

  // Quick match (matchmaking)
  server.post(
    '/quickmatch',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { userId } = request.user as any;
      const { gameId, gameMode, region } = request.body as any;

      if (!gameId) {
        return reply
          .code(400)
          .send({ error: true, message: 'gameId is required' });
      }

      // Get user info
      const { AuthService } = await import('../auth/AuthService');
      const authService = new AuthService();
      const user = await authService.getUserById(userId);

      if (!user) {
        return reply.code(404).send({ error: true, message: 'User not found' });
      }

      // Find available room
      const rooms = await serverManager.listGameRooms(gameId, region);
      let room = rooms.find((r) => {
        if (gameMode && r.gameMode !== gameMode) return false;
        return r.currentPlayers < r.maxPlayers && r.status === 'waiting';
      });

      // If no room found, create one
      if (!room) {
        const deviceId = `device-${Date.now()}-${Math.random().toString(36).substring(7)}`;
        room = await serverManager.createGameRoom(
          gameId,
          userId,
          user.username,
          deviceId,
          {
            name: `Quick Match - ${user.username}`,
            maxPlayers: 8,
            isPublic: true,
            gameMode,
          }
        );
      } else {
        // Join existing room
        const deviceId = `device-${Date.now()}-${Math.random().toString(36).substring(7)}`;
        room = await serverManager.joinGameRoom(
          room.id,
          userId,
          user.username,
          deviceId
        );
      }

      return { success: true, room };
    }
  );

  // Get player's active rooms
  server.get(
    '/my-rooms',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const { userId } = request.user as any;

      // Query rooms where user is a player
      const { DatabaseService } = await import('../database/DatabaseService');
      const db = DatabaseService.getInstance();

      const result = await db.query<any>(
        `SELECT * FROM game_rooms 
         WHERE players::text LIKE $1 
         AND status IN ('waiting', 'starting', 'playing')
         ORDER BY created_at DESC`,
        [`%"userId":"${userId}"%`]
      );

      const rooms = result.rows.map((row) => ({
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

      return { success: true, rooms };
    }
  );

  // WebSocket endpoint for real-time multiplayer (placeholder for WebSocket setup)
  server.get(
    '/ws/:roomId',
    { websocket: true } as any,
    (async (connection: any, request: FastifyRequest) => {
      const { roomId } = request.params as { _roomId: string };

      // WebSocket connection for real-time game state sync
      connection.socket.on('message', (_message: any) => {
        // Handle game state updates
        // Broadcast to all players in room
      });

      connection.socket.on('close', () => {
        // Handle player disconnect
      });
    }) as any
  );
}
