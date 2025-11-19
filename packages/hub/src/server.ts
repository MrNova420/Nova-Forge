/**
 * Fastify Server Configuration
 */

import Fastify from 'fastify';
import cors from '@fastify/cors';
import jwt from '@fastify/jwt';
import multipart from '@fastify/multipart';
import rateLimit from '@fastify/rate-limit';

import { authRoutes } from './api/auth.routes';
import { projectRoutes } from './api/project.routes';
import { assetRoutes } from './api/asset.routes';
import { buildRoutes } from './api/build.routes';
import { logger } from './utils/logger';

export async function createServer() {
  const server = Fastify({
    logger: false, // Using custom logger
    requestIdHeader: 'x-request-id',
    requestIdLogLabel: 'requestId',
    disableRequestLogging: false,
    trustProxy: true,
  });

  // Register plugins
  await server.register(cors, {
    origin: process.env.CORS_ORIGIN || '*',
    credentials: true,
  });

  await server.register(jwt, {
    secret: process.env.JWT_SECRET || 'nova-engine-secret-change-in-production',
    sign: {
      expiresIn: '7d',
    },
  });

  await server.register(multipart, {
    limits: {
      fileSize: Infinity, // No file size limit
      files: Infinity, // Unlimited files per upload
      fields: Infinity, // Unlimited fields
      parts: Infinity, // Unlimited parts
      fieldSize: Infinity, // No field size limit
      headerPairs: Infinity, // Unlimited headers
    },
  });

  // Rate limiting - very permissive for development
  await server.register(rateLimit, {
    max: 10000, // 10000 requests per window (essentially unlimited for dev)
    timeWindow: '1 minute', // Short window
  });

  // Add authenticate decorator for JWT authentication
  server.decorate('authenticate', async function (request: any, reply: any) {
    try {
      await request.jwtVerify();
    } catch (_err) {
      reply
        .status(401)
        .send({ error: 'Unauthorized', message: 'Invalid or missing token' });
    }
  });

  // Health check endpoint
  server.get('/health', async () => {
    return {
      status: 'healthy',
      timestamp: new Date().toISOString(),
      version: '0.1.0',
    };
  });

  // Root endpoint
  server.get('/', async () => {
    return {
      name: 'Nova Hub API',
      version: '0.1.0',
      endpoints: {
        health: '/health',
        auth: '/api/auth',
        projects: '/api/projects',
        assets: '/api/assets',
        builds: '/api/builds',
        docs: '/docs',
      },
    };
  });

  // Register API routes
  await server.register(authRoutes, { prefix: '/api/auth' });
  await server.register(projectRoutes, { prefix: '/api/projects' });
  await server.register(assetRoutes, { prefix: '/api/assets' });
  await server.register(buildRoutes, { prefix: '/api/builds' });

  // Import additional routes
  const { gamesRoutes } = await import('./api/games.routes');
  const { importRoutes } = await import('./api/import.routes');
  const { multiplayerRoutes } = await import('./api/multiplayer.routes');
  const { userRoutes, notificationRoutes, socialRoutes } = await import(
    './api/user.routes'
  );
  const { syncRoutes } = await import('./api/sync.routes');
  const { gameStatsRoutes } = await import('./api/games-stats.routes');

  await server.register(gamesRoutes, { prefix: '/api/games' });
  await server.register(gameStatsRoutes, { prefix: '/api/games-stats' });
  await server.register(importRoutes, { prefix: '/api/import' });
  await server.register(multiplayerRoutes, { prefix: '/api/multiplayer' });
  await server.register(userRoutes, { prefix: '/api/users' });
  await server.register(socialRoutes, { prefix: '/api/social' });
  await server.register(notificationRoutes, { prefix: '/api/notifications' });
  await server.register(syncRoutes, { prefix: '/api/sync' });

  // Error handler
  server.setErrorHandler((error, request, reply) => {
    logger.error('Request error:', {
      requestId: request.id,
      method: request.method,
      url: request.url,
      error: error.message,
      stack: error.stack,
    });

    reply.status(error.statusCode || 500).send({
      error: true,
      message: error.message || 'Internal Server Error',
      statusCode: error.statusCode || 500,
    });
  });

  // Not found handler
  server.setNotFoundHandler((request, reply) => {
    reply.status(404).send({
      error: true,
      message: 'Route not found',
      statusCode: 404,
      path: request.url,
    });
  });

  return server;
}
