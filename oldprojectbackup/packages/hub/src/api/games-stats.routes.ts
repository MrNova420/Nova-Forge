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
 * Game Statistics API Routes
 * Real tracking of game plays, downloads, and ratings
 */

import { FastifyInstance, FastifyRequest, FastifyReply } from 'fastify';
import { z } from 'zod';

// In-memory storage for real game statistics
interface GameStats {
  gameId: string;
  downloads: number;
  totalRating: number;
  ratingCount: number;
  averageRating: number;
}

const gameStatsStore = new Map<string, GameStats>();

const recordPlaySchema = z.object({
  gameId: z.string(),
});

const rateGameSchema = z.object({
  gameId: z.string(),
  rating: z.number().min(1).max(5),
});

export async function gameStatsRoutes(server: FastifyInstance) {
  // Get stats for a specific game
  server.get(
    '/:gameId/stats',
    async (
      request: FastifyRequest<{ Params: { gameId: string } }>,
      _reply: FastifyReply
    ) => {
      const { gameId } = request.params;

      const stats = gameStatsStore.get(gameId);

      if (!stats) {
        return {
          gameId,
          downloads: 0,
          averageRating: 0,
          ratingCount: 0,
        };
      }

      return {
        gameId: stats.gameId,
        downloads: stats.downloads,
        averageRating: stats.averageRating,
        ratingCount: stats.ratingCount,
      };
    }
  );

  // Get stats for all games
  server.get('/all', async (_request: FastifyRequest, _reply: FastifyReply) => {
    const allStats = Array.from(gameStatsStore.values()).map((stats) => ({
      gameId: stats.gameId,
      downloads: stats.downloads,
      averageRating: stats.averageRating,
      ratingCount: stats.ratingCount,
    }));

    return allStats;
  });

  // Record a game play (increments download count)
  server.post(
    '/play',
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const data = recordPlaySchema.parse(request.body);

      let stats = gameStatsStore.get(data.gameId);

      if (!stats) {
        stats = {
          gameId: data.gameId,
          downloads: 0,
          totalRating: 0,
          ratingCount: 0,
          averageRating: 0,
        };
        gameStatsStore.set(data.gameId, stats);
      }

      stats.downloads++;

      return {
        success: true,
        gameId: data.gameId,
        downloads: stats.downloads,
      };
    }
  );

  // Rate a game
  server.post(
    '/rate',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const data = rateGameSchema.parse(request.body);

      let stats = gameStatsStore.get(data.gameId);

      if (!stats) {
        stats = {
          gameId: data.gameId,
          downloads: 0,
          totalRating: 0,
          ratingCount: 0,
          averageRating: 0,
        };
        gameStatsStore.set(data.gameId, stats);
      }

      // Add the new rating
      stats.totalRating += data.rating;
      stats.ratingCount++;
      stats.averageRating = stats.totalRating / stats.ratingCount;

      return {
        success: true,
        gameId: data.gameId,
        averageRating: stats.averageRating,
        ratingCount: stats.ratingCount,
      };
    }
  );
}
