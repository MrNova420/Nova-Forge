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
 * Asset API Routes
 */

import { FastifyInstance, FastifyRequest, FastifyReply } from 'fastify';
import { AssetStorageService } from '../storage/AssetStorageService';
import { ProjectStorageService } from '../services/ProjectStorageService';

export async function assetRoutes(server: FastifyInstance) {
  const assetService = new AssetStorageService();
  const projectService = new ProjectStorageService();

  // Upload asset
  server.post(
    '/upload',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { userId } = request.user as any;

      // Get uploaded file
      const data = await request.file();
      if (!data) {
        return reply
          .code(400)
          .send({ error: true, message: 'No file uploaded' });
      }

      const buffer = await data.toBuffer();
      const { projectId, path: assetPath } = data.fields as any;

      if (!projectId || !assetPath) {
        return reply.code(400).send({
          error: true,
          message: 'projectId and path are required',
        });
      }

      // Check project ownership
      const isOwner = await projectService.isProjectOwner(
        projectId.value,
        userId
      );
      if (!isOwner) {
        return reply
          .code(403)
          .send({ error: true, message: 'Permission denied' });
      }

      // Upload asset
      const asset = await assetService.uploadAsset({
        projectId: projectId.value,
        path: assetPath.value,
        type: data.mimetype,
        buffer,
      });

      return reply.code(201).send({ success: true, asset });
    }
  );

  // Get project assets
  server.get(
    '/:projectId',
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const { projectId } = request.params as { projectId: string };
      const assets = await assetService.getProjectAssets(projectId);
      return { success: true, assets };
    }
  );

  // Download asset
  server.get(
    '/:projectId/:assetId',
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { assetId } = request.params as { assetId: string };

      const asset = await assetService.getAsset(assetId);
      if (!asset) {
        return reply
          .code(404)
          .send({ error: true, message: 'Asset not found' });
      }

      const buffer = await assetService.downloadAsset(assetId);

      reply.header('Content-Type', asset.type);
      reply.header('Content-Length', asset.sizeBytes);
      reply.header(
        'Content-Disposition',
        `attachment; filename="${asset.path}"`
      );

      return reply.send(buffer);
    }
  );

  // Delete asset
  server.delete(
    '/:projectId/:assetId',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { projectId, assetId } = request.params as {
        projectId: string;
        assetId: string;
      };
      const { userId } = request.user as any;

      // Check ownership
      const isOwner = await projectService.isProjectOwner(projectId, userId);
      if (!isOwner) {
        return reply
          .code(403)
          .send({ error: true, message: 'Permission denied' });
      }

      const deleted = await assetService.deleteAsset(assetId);
      if (!deleted) {
        return reply
          .code(404)
          .send({ error: true, message: 'Asset not found' });
      }

      return { success: true, message: 'Asset deleted' };
    }
  );

  // Get storage stats
  server.get(
    '/:projectId/stats',
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const { projectId } = request.params as { projectId: string };
      const stats = await assetService.getProjectStorageStats(projectId);
      return { success: true, stats };
    }
  );

  // Search assets
  server.get(
    '/:projectId/search',
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { projectId } = request.params as { projectId: string };
      const { q } = request.query as { q?: string };

      if (!q) {
        return reply
          .code(400)
          .send({ error: true, message: 'Query parameter "q" is required' });
      }

      const assets = await assetService.searchAssets(projectId, q);
      return { success: true, assets };
    }
  );
}
