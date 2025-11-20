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
 * Project API Routes
 */

import { FastifyInstance, FastifyRequest, FastifyReply } from 'fastify';
import { ProjectStorageService } from '../services/ProjectStorageService';
import { VersionControlService } from '../services/VersionControlService';
import { ProjectSyncService } from '../services/ProjectSyncService';
import { z } from 'zod';

const createProjectSchema = z.object({
  name: z.string().min(1).max(200),
  description: z.string().optional(),
  visibility: z.enum(['public', 'private', 'unlisted']).optional(),
});

const updateProjectSchema = z.object({
  name: z.string().min(1).max(200).optional(),
  description: z.string().optional(),
  visibility: z.enum(['public', 'private', 'unlisted']).optional(),
});

export async function projectRoutes(server: FastifyInstance) {
  const projectService = new ProjectStorageService();
  const versionService = new VersionControlService();
  const syncService = new ProjectSyncService();

  // List user projects
  server.get(
    '/',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const { userId } = request.user as any;
      const projects = await projectService.getUserProjects(userId);
      return { success: true, projects };
    }
  );

  // Get public projects
  server.get(
    '/public',
    async (_request: FastifyRequest, _reply: FastifyReply) => {
      const projects = await projectService.getPublicProjects();
      return { success: true, projects };
    }
  );

  // Search projects
  server.get(
    '/search',
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { q } = request.query as { q?: string };
      if (!q) {
        return reply
          .code(400)
          .send({ error: true, message: 'Query parameter "q" is required' });
      }
      const projects = await projectService.searchProjects(q);
      return { success: true, projects };
    }
  );

  // Create project
  server.post(
    '/',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { userId } = request.user as any;
      const data = createProjectSchema.parse(request.body);

      const project = await projectService.createProject({
        ...data,
        ownerId: userId,
      } as any);

      // Create initial version
      await versionService.createVersion({
        projectId: project.id,
        commitMessage: 'Initial commit',
        createdBy: userId,
        snapshotData: { initial: true },
      });

      return reply.code(201).send({ success: true, project });
    }
  );

  // Get project
  server.get('/:id', async (request: FastifyRequest, reply: FastifyReply) => {
    const { id } = request.params as { id: string };
    const project = await projectService.getProject(id);

    if (!project) {
      return reply
        .code(404)
        .send({ error: true, message: 'Project not found' });
    }

    return { success: true, project };
  });

  // Update project
  server.put(
    '/:id',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { id } = request.params as { id: string };
      const { userId } = request.user as any;
      const updates = updateProjectSchema.parse(request.body);

      // Check ownership
      const isOwner = await projectService.isProjectOwner(id, userId);
      if (!isOwner) {
        return reply
          .code(403)
          .send({ error: true, message: 'Permission denied' });
      }

      const project = await projectService.updateProject(id, updates);
      if (!project) {
        return reply
          .code(404)
          .send({ error: true, message: 'Project not found' });
      }

      return { success: true, project };
    }
  );

  // Delete project
  server.delete(
    '/:id',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { id } = request.params as { id: string };
      const { userId } = request.user as any;

      // Check ownership
      const isOwner = await projectService.isProjectOwner(id, userId);
      if (!isOwner) {
        return reply
          .code(403)
          .send({ error: true, message: 'Permission denied' });
      }

      const deleted = await projectService.deleteProject(id);
      if (!deleted) {
        return reply
          .code(404)
          .send({ error: true, message: 'Project not found' });
      }

      return { success: true, message: 'Project deleted' };
    }
  );

  // Get project versions
  server.get(
    '/:id/versions',
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const { id } = request.params as { id: string };
      const versions = await versionService.getProjectVersions(id);
      return { success: true, versions };
    }
  );

  // Get project sync state
  server.get(
    '/:id/sync',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const { id } = request.params as { id: string };
      const syncState = await syncService.getSyncState(id);
      return { success: true, syncState };
    }
  );

  // Sync project
  server.post(
    '/:id/sync',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const { id } = request.params as { id: string };
      const { userId } = request.user as any;

      const result = await syncService.syncProject(id, userId);
      return { success: result.success, ...result };
    }
  );
}
