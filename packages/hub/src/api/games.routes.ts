/**
 * Games Marketplace API Routes
 * Public game store/marketplace where users can discover and play games
 */

import { FastifyInstance, FastifyRequest, FastifyReply } from 'fastify';
import { GamePublishingService } from '../services/GamePublishingService';
import { ProjectStorageService } from '../services/ProjectStorageService';
import { z } from 'zod';

const publishGameSchema = z.object({
  projectId: z.string().uuid(),
  buildId: z.string().uuid(),
  name: z.string().min(1).max(200),
  description: z.string(),
  version: z.string().regex(/^\d+\.\d+\.\d+$/), // Semantic versioning
  category: z.string(),
  tags: z.array(z.string()).optional(),
  visibility: z.enum(['public', 'unlisted']).optional(),
});

const rateGameSchema = z.object({
  rating: z.number().min(1).max(5),
});

export async function gamesRoutes(server: FastifyInstance) {
  const gameService = new GamePublishingService();
  const projectService = new ProjectStorageService();

  // Browse all published games
  server.get('/', async (request: FastifyRequest, reply: FastifyReply) => {
    try {
      const { category, tags, search, sortBy, limit, offset } =
        request.query as any;

      const games = await gameService.getPublishedGames({
        category,
        tags: tags ? tags.split(',') : undefined,
        search,
        sortBy: sortBy || 'newest',
        limit: limit ? parseInt(limit, 10) : 10000,
        offset: offset ? parseInt(offset, 10) : 0,
      });

      return { success: true, games };
    } catch (error) {
      // Return empty array on error (in-memory mode)
      return { success: true, games: [] };
    }
  });

  // Get featured/trending games
  server.get(
    '/featured',
    async (request: FastifyRequest, reply: FastifyReply) => {
      try {
        const games = await gameService.getPublishedGames({
          sortBy: 'popular',
          limit: 1000,
        });
        return { success: true, games };
      } catch (error) {
        return { success: true, games: [] };
      }
    }
  );

  // Get new releases
  server.get('/new', async (request: FastifyRequest, reply: FastifyReply) => {
    try {
      const games = await gameService.getPublishedGames({
        sortBy: 'newest',
        limit: 1000,
      });
      return { success: true, games };
    } catch (error) {
      return { success: true, games: [] };
    }
  });

  // Get top rated games
  server.get(
    '/top-rated',
    async (request: FastifyRequest, reply: FastifyReply) => {
      try {
        const games = await gameService.getPublishedGames({
          sortBy: 'rating',
          limit: 1000,
        });
        return { success: true, games };
      } catch (error) {
        return { success: true, games: [] };
      }
    }
  );

  // Get games by category
  server.get(
    '/category/:category',
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { category } = request.params as { category: string };
      const { limit, offset } = request.query as any;

      const games = await gameService.getPublishedGames({
        category,
        sortBy: 'newest',
        limit: limit ? parseInt(limit, 10) : 10000, // Increased from 50
        offset: offset ? parseInt(offset, 10) : 0,
      });

      return { success: true, games };
    }
  );

  // Get specific game details
  server.get('/:id', async (request: FastifyRequest, reply: FastifyReply) => {
    const { id } = request.params as { id: string };

    const game = await gameService.getPublishedGame(id);
    if (!game) {
      return reply.code(404).send({ error: true, message: 'Game not found' });
    }

    return { success: true, game };
  });

  // Publish a game
  server.post(
    '/publish',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { userId } = request.user as any;
      const data = publishGameSchema.parse(request.body);

      // Check project ownership
      const isOwner = await projectService.isProjectOwner(
        data.projectId,
        userId
      );
      if (!isOwner) {
        return reply
          .code(403)
          .send({ error: true, message: 'Permission denied' });
      }

      // Handle multipart form data for thumbnail and screenshots
      // This would be enhanced to handle file uploads
      const game = await gameService.publishGame({
        ...data,
        authorId: userId,
      } as any);

      return reply.code(201).send({ success: true, game });
    }
  );

  // Play game (increment play count and redirect)
  server.get(
    '/:id/play',
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { id } = request.params as { id: string };

      const game = await gameService.getPublishedGame(id);
      if (!game) {
        return reply.code(404).send({ error: true, message: 'Game not found' });
      }

      // Increment play count
      await gameService.incrementPlayCount(id);

      // Return play URL or redirect
      return { success: true, playUrl: game.playUrl, game };
    }
  );

  // Download game
  server.get(
    '/:id/download/:platform',
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { id, platform } = request.params as {
        id: string;
        platform: 'web' | 'desktop' | 'mobile';
      };

      const game = await gameService.getPublishedGame(id);
      if (!game) {
        return reply.code(404).send({ error: true, message: 'Game not found' });
      }

      const downloadUrl = game.downloadUrls[platform];
      if (!downloadUrl) {
        return reply
          .code(404)
          .send({ error: true, message: `No ${platform} build available` });
      }

      // Increment download count
      await gameService.incrementDownloadCount(id);

      // Return download URL or redirect
      return reply.redirect(downloadUrl);
    }
  );

  // Rate game
  server.post(
    '/:id/rate',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { id } = request.params as { id: string };
      const { userId } = request.user as any;
      const { rating } = rateGameSchema.parse(request.body);

      await gameService.rateGame(id, userId, rating);

      return { success: true, message: 'Rating submitted' };
    }
  );

  // Unpublish game (remove from marketplace)
  server.delete(
    '/:id/unpublish',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { id } = request.params as { id: string };
      const { userId } = request.user as any;

      const game = await gameService.getPublishedGame(id);
      if (!game) {
        return reply.code(404).send({ error: true, message: 'Game not found' });
      }

      // Check ownership
      const isOwner = await projectService.isProjectOwner(
        game.projectId,
        userId
      );
      if (!isOwner) {
        return reply
          .code(403)
          .send({ error: true, message: 'Permission denied' });
      }

      const unpublished = await gameService.unpublishGame(id);
      if (!unpublished) {
        return reply
          .code(500)
          .send({ error: true, message: 'Failed to unpublish game' });
      }

      return { success: true, message: 'Game unpublished' };
    }
  );

  // Get my published games
  server.get(
    '/my/published',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { userId } = request.user as any;

      // Get user's projects
      const projects = await projectService.getUserProjects(userId);
      const projectIds = projects.map((p) => p.id);

      // Get published games for these projects
      const allGames = await gameService.getPublishedGames({ limit: 1000 });
      const myGames = allGames.filter((game) =>
        projectIds.includes(game.projectId)
      );

      return { success: true, games: myGames };
    }
  );
}
