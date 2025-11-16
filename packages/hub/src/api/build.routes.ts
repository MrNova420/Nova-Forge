import { FastifyInstance } from 'fastify';

export async function buildRoutes(server: FastifyInstance) {
  server.get('/', async () => ({ message: 'Build routes - Coming soon' }));
}
