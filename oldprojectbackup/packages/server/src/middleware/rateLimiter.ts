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
 * Rate Limiting Middleware
 * Protects API endpoints from abuse and brute force attacks
 */

import { Request, Response, NextFunction } from 'express';

interface RateLimitConfig {
  windowMs: number; // Time window in milliseconds
  maxRequests: number; // Max requests per window
  message?: string;
  skipSuccessfulRequests?: boolean;
  skipFailedRequests?: boolean;
}

interface RateLimitStore {
  [key: string]: {
    count: number;
    resetTime: number;
  };
}

/**
 * Create a rate limiter middleware
 * @param config - Rate limit configuration
 * @returns Express middleware function
 */
export function createRateLimiter(config: RateLimitConfig) {
  const {
    windowMs,
    maxRequests,
    message = 'Too many requests, please try again later.',
    skipSuccessfulRequests = false,
    skipFailedRequests = false,
  } = config;

  // In-memory store (for production, use Redis)
  const store: RateLimitStore = {};

  // Cleanup old entries periodically
  setInterval(() => {
    const now = Date.now();
    Object.keys(store).forEach((key) => {
      if (store[key].resetTime < now) {
        delete store[key];
      }
    });
  }, windowMs);

  return (req: Request, res: Response, next: NextFunction): void => {
    // Get client identifier (IP address + user ID if authenticated)
    const clientId = getClientIdentifier(req);
    const now = Date.now();

    // Initialize or get existing entry
    if (!store[clientId] || store[clientId].resetTime < now) {
      store[clientId] = {
        count: 0,
        resetTime: now + windowMs,
      };
    }

    // Increment request count
    store[clientId].count++;

    // Set rate limit headers
    const remaining = Math.max(0, maxRequests - store[clientId].count);
    const resetTime = Math.ceil(store[clientId].resetTime / 1000);

    res.setHeader('X-RateLimit-Limit', maxRequests.toString());
    res.setHeader('X-RateLimit-Remaining', remaining.toString());
    res.setHeader('X-RateLimit-Reset', resetTime.toString());

    // Check if limit exceeded
    if (store[clientId].count > maxRequests) {
      res.setHeader('Retry-After', Math.ceil((store[clientId].resetTime - now) / 1000).toString());
      res.status(429).json({
        error: 'Too Many Requests',
        message,
        retryAfter: Math.ceil((store[clientId].resetTime - now) / 1000),
      });
      return;
    }

    // Handle skip options
    if (skipSuccessfulRequests || skipFailedRequests) {
      const originalSend = res.send;
      res.send = function (body): Response {
        const statusCode = res.statusCode;
        
        if (skipSuccessfulRequests && statusCode >= 200 && statusCode < 300) {
          store[clientId].count--;
        } else if (skipFailedRequests && (statusCode < 200 || statusCode >= 400)) {
          store[clientId].count--;
        }
        
        return originalSend.call(this, body);
      };
    }

    next();
  };
}

/**
 * Get unique client identifier
 * Uses IP address and authenticated user ID if available
 */
function getClientIdentifier(req: Request): string {
  // Try to get user ID from auth middleware
  const userId = (req as any).userId;
  
  // Get IP address (consider X-Forwarded-For for proxies)
  const ip = req.headers['x-forwarded-for'] || req.socket.remoteAddress || 'unknown';
  
  return userId ? `user:${userId}` : `ip:${ip}`;
}

/**
 * Predefined rate limiters for common use cases
 */

// Very strict rate limiter for authentication endpoints
export const authRateLimiter = createRateLimiter({
  windowMs: 15 * 60 * 1000, // 15 minutes
  maxRequests: 5, // 5 requests per 15 minutes
  message: 'Too many login attempts, please try again after 15 minutes.',
  skipSuccessfulRequests: true, // Don't count successful logins
});

// Strict rate limiter for registration
export const registerRateLimiter = createRateLimiter({
  windowMs: 60 * 60 * 1000, // 1 hour
  maxRequests: 3, // 3 registrations per hour
  message: 'Too many registration attempts, please try again after 1 hour.',
});

// Standard rate limiter for API endpoints
export const apiRateLimiter = createRateLimiter({
  windowMs: 15 * 60 * 1000, // 15 minutes
  maxRequests: 100, // 100 requests per 15 minutes
  message: 'Too many API requests, please try again later.',
});

// Lenient rate limiter for public endpoints
export const publicRateLimiter = createRateLimiter({
  windowMs: 15 * 60 * 1000, // 15 minutes
  maxRequests: 300, // 300 requests per 15 minutes
  message: 'Too many requests, please slow down.',
});
