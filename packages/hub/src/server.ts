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
      fileSize: 100 * 1024 * 1024, // 100MB max file size
      files: 10, // Max 10 files per upload
    },
  });

  await server.register(rateLimit, {
    max: 100, // 100 requests
    timeWindow: '15 minutes',
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
