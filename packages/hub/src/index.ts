/**
 * Nova Hub Server
 * Platform server for game hosting, distribution, and collaboration
 */

import { createServer } from './server';
import { DatabaseService } from './database/DatabaseService';
import { RedisService } from './database/RedisService';
import { logger } from './utils/logger';

const PORT = parseInt(process.env.PORT || '3001', 10);
const HOST = process.env.HOST || '0.0.0.0';

async function startServer() {
  try {
    logger.info('🚀 Starting Nova Hub Server...');

    // Initialize database (optional - will use in-memory if not available)
    try {
      const db = DatabaseService.getInstance();
      await db.connect();
      logger.info('✅ Database connected');
    } catch (error) {
      logger.warn('⚠️  Database not available, using in-memory storage');
    }

    // Initialize Redis (optional - will use in-memory if not available)
    try {
      const redis = RedisService.getInstance();
      await redis.connect();
      logger.info('✅ Redis connected');
    } catch (error) {
      logger.warn('⚠️  Redis not available, using in-memory cache');
    }

    // Create and start Fastify server
    const server = await createServer();
    await server.listen({ port: PORT, host: HOST });

    logger.info(`🎮 Nova Hub Server running on http://${HOST}:${PORT}`);
    logger.info('📚 API Documentation: http://localhost:3001/docs');
  } catch (error) {
    logger.error('Failed to start server:', error);
    process.exit(1);
  }
}

// Handle shutdown gracefully
process.on('SIGINT', async () => {
  logger.info('Shutting down gracefully...');
  await DatabaseService.getInstance().disconnect();
  await RedisService.getInstance().disconnect();
  process.exit(0);
});

// Start the server
startServer();
