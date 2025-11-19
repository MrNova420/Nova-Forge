/**
 * Project Import API Routes
 * Allows users to import existing projects from various sources
 */

import { FastifyInstance, FastifyRequest, FastifyReply } from 'fastify';
import { ProjectImportService } from '../services/ProjectImportService';
import { z } from 'zod';

const importProjectSchema = z.object({
  projectName: z.string().min(1).max(200),
  description: z.string().optional(),
  source: z.object({
    type: z.enum([
      'zip',
      'git',
      'url',
      'local',
      'unity',
      'unreal',
      'godot',
      'other',
    ]),
    location: z.string(),
    credentials: z
      .object({
        token: z.string().optional(),
        username: z.string().optional(),
        password: z.string().optional(),
      })
      .optional(),
  }),
  preserveStructure: z.boolean().optional(),
  convertAssets: z.boolean().optional(),
  createVersion: z.boolean().optional(),
});

const importNovaProjectSchema = z.object({
  projectName: z.string().min(1).max(200).optional(),
  projectPath: z.string(),
  syncAssets: z.boolean().optional(),
});

const validateSourceSchema = z.object({
  type: z.enum([
    'zip',
    'git',
    'url',
    'local',
    'unity',
    'unreal',
    'godot',
    'other',
  ]),
  location: z.string(),
});

export async function importRoutes(server: FastifyInstance) {
  const importService = new ProjectImportService();

  // Get supported import formats
  server.get(
    '/formats',
    async (_request: FastifyRequest, _reply: FastifyReply) => {
      const formats = importService.getSupportedFormats();
      return { success: true, formats };
    }
  );

  // Validate import source before importing
  server.post(
    '/validate',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, _reply: FastifyReply) => {
      const data = validateSourceSchema.parse(request.body);

      const validation = await importService.validateImportSource(data as any);

      return {
        success: validation.valid,
        ...validation,
      };
    }
  );

  // Import project from external source
  server.post(
    '/project',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { userId } = request.user as any;
      const data = importProjectSchema.parse(request.body);

      const result = await importService.importProject({
        ...data,
        userId,
      } as any);

      if (result.success) {
        return reply.code(201).send({
          success: true,
          ...result,
        });
      } else {
        return reply.code(400).send({
          success: false,
          ...result,
        });
      }
    }
  );

  // Import existing Nova Engine project
  server.post(
    '/nova-project',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { userId } = request.user as any;
      const data = importNovaProjectSchema.parse(request.body);

      const result = await importService.importExistingNovaProject(
        userId,
        data.projectPath,
        {
          projectName: data.projectName,
          syncAssets: data.syncAssets,
        }
      );

      if (result.success) {
        return reply.code(201).send({
          success: true,
          ...result,
        });
      } else {
        return reply.code(400).send({
          success: false,
          ...result,
        });
      }
    }
  );

  // Upload project ZIP for import
  server.post(
    '/upload',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { userId } = request.user as any;

      const data = await request.file();
      if (!data) {
        return reply.code(400).send({
          error: true,
          message: 'No file uploaded',
        });
      }

      const buffer = await data.toBuffer();
      const { projectName, description } = data.fields as any;

      if (!projectName) {
        return reply.code(400).send({
          error: true,
          message: 'projectName is required',
        });
      }

      // Save uploaded file temporarily
      const tempPath = `/tmp/upload-${Date.now()}-${data.filename}`;
      const fs = await import('fs/promises');
      await fs.writeFile(tempPath, buffer);

      // Import from the uploaded file
      const result = await importService.importProject({
        userId,
        projectName: projectName.value,
        description: description?.value,
        source: {
          type: 'zip',
          location: tempPath,
        },
        convertAssets: true,
        createVersion: true,
      });

      // Clean up temp file
      try {
        await fs.unlink(tempPath);
      } catch (_error) {
        // Ignore cleanup errors
      }

      if (result.success) {
        return reply.code(201).send({
          success: true,
          ...result,
        });
      } else {
        return reply.code(400).send({
          success: false,
          ...result,
        });
      }
    }
  );

  // Get import progress
  server.get(
    '/:importId/progress',
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { importId } = request.params as { importId: string };

      const progress = await importService.getImportProgress(importId);

      if (!progress) {
        return reply.code(404).send({
          error: true,
          message: 'Import not found or already completed',
        });
      }

      return { success: true, progress };
    }
  );

  // Import from GitHub repository
  server.post(
    '/github',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { userId } = request.user as any;
      const { repoUrl, projectName, description, token } = request.body as any;

      if (!repoUrl || !projectName) {
        return reply.code(400).send({
          error: true,
          message: 'repoUrl and projectName are required',
        });
      }

      const result = await importService.importProject({
        userId,
        projectName,
        description,
        source: {
          type: 'git',
          location: repoUrl,
          credentials: token ? { token } : undefined,
        },
        convertAssets: true,
        createVersion: true,
      });

      if (result.success) {
        return reply.code(201).send({
          success: true,
          ...result,
        });
      } else {
        return reply.code(400).send({
          success: false,
          ...result,
        });
      }
    }
  );

  // Import from Unity project
  server.post(
    '/unity',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { userId } = request.user as any;
      const { projectPath, projectName, description } = request.body as any;

      if (!projectPath || !projectName) {
        return reply.code(400).send({
          error: true,
          message: 'projectPath and projectName are required',
        });
      }

      const result = await importService.importProject({
        userId,
        projectName,
        description,
        source: {
          type: 'unity',
          location: projectPath,
        },
        convertAssets: true,
        createVersion: true,
      });

      if (result.success) {
        return reply.code(201).send({
          success: true,
          ...result,
          warning: 'Unity scripts must be manually converted to TypeScript',
        });
      } else {
        return reply.code(400).send({
          success: false,
          ...result,
        });
      }
    }
  );
}
