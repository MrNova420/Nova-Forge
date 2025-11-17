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

const loginSchema = z.object({
  emailOrUsername: z.string(),
  password: z.string(),
});

export async function authRoutes(server: FastifyInstance) {
  const authService = new AuthService();

  // Rate limiting configuration for auth endpoints
  const authRateLimit = {
    max: 5, // Maximum 5 requests
    timeWindow: '15 minutes', // Per 15 minutes
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
        });
      } catch (error: any) {
        if (error.code === '23505') {
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
      const result = await authService.login(
        data.emailOrUsername,
        data.password
      );

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
    async (request: FastifyRequest, reply: FastifyReply) => {
      return { success: true, message: 'Logged out successfully' };
    }
  );
}
