/**
 * User API Routes
 */

import { FastifyInstance, FastifyRequest, FastifyReply } from 'fastify';

export async function userRoutes(server: FastifyInstance) {
  // Get user statistics
  server.get(
    '/me/stats',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const { userId } = request.user as any;
      return {
        level: 1,
        xp: 0,
        xpToNext: 1000,
        gamesPlayed: 0,
        gamesCreated: 0,
        achievementsUnlocked: 0,
        totalAchievements: 100,
        friendsCount: 0,
        totalPlaytime: 0,
      };
    }
  );

  // Get user profile
  server.get(
    '/:userId',
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const { userId } = request.params as any;
      // Return minimal profile for now
      return {
        id: userId,
        username: `User_${userId.substring(0, 8)}`,
        avatar_url: null,
        title: 'Player',
        bio: '',
        created_at: new Date().toISOString(),
      };
    }
  );
}

/**
 * Social API Routes (Friends, Achievements, etc.)
 */
export async function socialRoutes(server: FastifyInstance) {
  // Get friends list
  server.get(
    '/friends',
    { onRequest: [server.authenticate] },
    async (_request: FastifyRequest, _reply: FastifyReply) => {
      // Return empty friends list for now
      return [];
    }
  );

  // Add friend
  server.post(
    '/friends/:friendId',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const { friendId } = request.params as any;
      return { success: true, message: 'Friend request sent' };
    }
  );

  // Remove friend
  server.delete(
    '/friends/:friendId',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const { friendId } = request.params as any;
      return { success: true, message: 'Friend removed' };
    }
  );

  // Get achievements
  server.get(
    '/achievements',
    { onRequest: [server.authenticate] },
    async (_request: FastifyRequest, _reply: FastifyReply) => {
      // Return empty achievements list for now
      return [];
    }
  );
}

/**
 * Notifications API Routes
 */
export async function notificationRoutes(server: FastifyInstance) {
  // Get notifications
  server.get(
    '/',
    { onRequest: [server.authenticate] },
    async (_request: FastifyRequest, _reply: FastifyReply) => {
      return [];
    }
  );
}
