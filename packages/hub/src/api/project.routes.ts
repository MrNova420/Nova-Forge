import { FastifyInstance } from 'fastify';

export async function projectRoutes(server: FastifyInstance) {
  server.get('/', async () => ({ message: 'Project routes - Coming soon' }));
}
