import { FastifyInstance } from 'fastify';

export async function assetRoutes(server: FastifyInstance) {
  server.get('/', async () => ({ message: 'Asset routes - Coming soon' }));
}
