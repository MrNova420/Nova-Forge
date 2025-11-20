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
 * Build API Routes
 */

import { FastifyInstance, FastifyRequest, FastifyReply } from 'fastify';
import { BuildQueueService } from '../services/BuildQueueService';
import { ProjectStorageService } from '../services/ProjectStorageService';
import { z } from 'zod';

const createBuildSchema = z.object({
  projectId: z.string().uuid(),
  versionId: z.string().uuid().optional(),
  target: z.enum(['web', 'desktop', 'mobile']),
});

export async function buildRoutes(server: FastifyInstance) {
  const buildService = new BuildQueueService();
  const projectService = new ProjectStorageService();

  // Create build
  server.post(
    '/',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { userId } = request.user as any;
      const data = createBuildSchema.parse(request.body);

      // Check ownership
      const isOwner = await projectService.isProjectOwner(
        data.projectId,
        userId
      );
      if (!isOwner) {
        return reply
          .code(403)
          .send({ error: true, message: 'Permission denied' });
      }

      const build = await buildService.queueBuild(data as any);

      return reply.code(201).send({ success: true, build });
    }
  );

  // Get build
  server.get('/:id', async (request: FastifyRequest, reply: FastifyReply) => {
    const { id } = request.params as { id: string };

    const build = await buildService.getBuild(id);
    if (!build) {
      return reply.code(404).send({ error: true, message: 'Build not found' });
    }

    return { success: true, build };
  });

  // Get project builds
  server.get(
    '/project/:projectId',
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const { projectId } = request.params as { projectId: string };

      const builds = await buildService.getProjectBuilds(projectId);

      return { success: true, builds };
    }
  );

  // Cancel build
  server.post(
    '/:id/cancel',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { id } = request.params as { id: string };
      const { userId } = request.user as any;

      // Get build to check project ownership
      const build = await buildService.getBuild(id);
      if (!build) {
        return reply
          .code(404)
          .send({ error: true, message: 'Build not found' });
      }

      // Check ownership
      const isOwner = await projectService.isProjectOwner(
        build.projectId,
        userId
      );
      if (!isOwner) {
        return reply
          .code(403)
          .send({ error: true, message: 'Permission denied' });
      }

      const cancelled = await buildService.cancelBuild(id);
      if (!cancelled) {
        return reply.code(400).send({
          error: true,
          message: 'Build cannot be cancelled (already completed or failed)',
        });
      }

      return { success: true, message: 'Build cancelled' };
    }
  );

  // Get build stats
  server.get(
    '/stats/:projectId',
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const { projectId } = request.params as { projectId: string };

      const stats = await buildService.getBuildStats(projectId);

      return { success: true, stats };
    }
  );

  // Get queued builds (admin only in production)
  server.get(
    '/queue/list',
    async (_request: FastifyRequest, _reply: FastifyReply) => {
      const builds = await buildService.getQueuedBuilds();

      return { success: true, builds };
    }
  );
}
