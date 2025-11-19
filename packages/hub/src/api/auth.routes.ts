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
 * Authentication API Routes
 */

import { FastifyInstance, FastifyRequest, FastifyReply } from 'fastify';
import { AuthService } from '../auth/AuthService';
import { z } from 'zod';

const registerSchema = z.object({
  email: z.string().email(),
  username: z.string().min(3).max(50),
  password: z.string().min(8),
  displayName: z.string().optional(),
});

const loginSchema = z
  .object({
    email: z.string().optional(),
    emailOrUsername: z.string().optional(),
    password: z.string(),
  })
  .refine((data) => data.email || data.emailOrUsername, {
    message: 'Either email or emailOrUsername is required',
  });

export async function authRoutes(server: FastifyInstance) {
  const authService = new AuthService();

  // Rate limiting configuration for auth endpoints
  // Very permissive for actual development use
  const authRateLimit = {
    max: 10000, // Essentially unlimited for development
    timeWindow: '1 minute',
    errorResponseBuilder: () => ({
      error: true,
      message: 'Too many requests, please try again later.',
    }),
  };

  // Register new user
  server.post(
    '/register',
    {
      config: {
        rateLimit: authRateLimit,
      },
    },
    async (request: FastifyRequest, reply: FastifyReply) => {
      try {
        const data = registerSchema.parse(request.body);
        const result = await authService.register(
          data.email,
          data.username,
          data.password
        );
        const token = server.jwt.sign({ userId: result.id });

        return reply.code(201).send({
          success: true,
          user: result.user,
          token,
          refreshToken: result.tokens.refreshToken,
        });
      } catch (error: unknown) {
        if (
          error &&
          typeof error === 'object' &&
          'code' in error &&
          error.code === '23505'
        ) {
          return reply.code(409).send({
            error: true,
            message: 'Email or username already exists',
          });
        }
        throw error;
      }
    }
  );

  // Login
  server.post(
    '/login',
    {
      config: {
        rateLimit: authRateLimit,
      },
    },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const data = loginSchema.parse(request.body);
      const emailOrUsername = data.email || data.emailOrUsername || '';
      const result = await authService.login(emailOrUsername, data.password);

      if (!result) {
        return reply.code(401).send({
          error: true,
          message: 'Invalid credentials',
        });
      }

      const token = server.jwt.sign({ userId: result.user.id });

      return {
        success: true,
        user: result.user,
        token,
        refreshToken: result.tokens.refreshToken,
      };
    }
  );

  // Get current user
  server.get(
    '/me',
    { onRequest: [server.authenticate] },
    async (request: FastifyRequest, reply: FastifyReply) => {
      const { userId } = request.user as any;
      const user = await authService.getUserById(userId);

      if (!user) {
        return reply.code(404).send({
          error: true,
          message: 'User not found',
        });
      }

      return { success: true, user };
    }
  );

  // Logout (client-side token removal)
  server.post(
    '/logout',
    { onRequest: [server.authenticate] },
    async (_request: FastifyRequest, _reply: FastifyReply) => {
      return { success: true, message: 'Logged out successfully' };
    }
  );
}
