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
 * Security Headers Middleware
 * Implements security headers following OWASP best practices
 * Similar to helmet.js but custom implementation
 */

import { Request, Response, NextFunction } from 'express';

/**
 * Security Headers Configuration
 */
interface SecurityHeadersConfig {
  contentSecurityPolicy?: boolean;
  strictTransportSecurity?: boolean;
  xFrameOptions?: boolean;
  xContentTypeOptions?: boolean;
  referrerPolicy?: boolean;
  permissionsPolicy?: boolean;
}

/**
 * Apply all security headers
 */
export function securityHeaders(config: SecurityHeadersConfig = {}) {
  const {
    contentSecurityPolicy = true,
    strictTransportSecurity = true,
    xFrameOptions = true,
    xContentTypeOptions = true,
    referrerPolicy = true,
    permissionsPolicy = true,
  } = config;

  return (req: Request, res: Response, next: NextFunction): void => {
    // Remove potentially dangerous headers
    res.removeHeader('X-Powered-By');

    // Content Security Policy - prevents XSS and injection attacks
    if (contentSecurityPolicy) {
      const cspDirectives = [
        "default-src 'self'",
        "base-uri 'self'",
        "font-src 'self' https: data:",
        "form-action 'self'",
        "frame-ancestors 'self'",
        "img-src 'self' data: https:",
        "object-src 'none'",
        "script-src 'self' 'unsafe-inline' 'unsafe-eval'", // Note: 'unsafe-inline' and 'unsafe-eval' should be removed in production
        "script-src-attr 'none'",
        "style-src 'self' https: 'unsafe-inline'",
        "upgrade-insecure-requests",
      ].join('; ');
      
      res.setHeader('Content-Security-Policy', cspDirectives);
    }

    // HTTP Strict Transport Security - forces HTTPS
    if (strictTransportSecurity) {
      res.setHeader(
        'Strict-Transport-Security',
        'max-age=31536000; includeSubDomains; preload'
      );
    }

    // X-Frame-Options - prevents clickjacking
    if (xFrameOptions) {
      res.setHeader('X-Frame-Options', 'SAMEORIGIN');
    }

    // X-Content-Type-Options - prevents MIME sniffing
    if (xContentTypeOptions) {
      res.setHeader('X-Content-Type-Options', 'nosniff');
    }

    // Referrer-Policy - controls referrer information
    if (referrerPolicy) {
      res.setHeader('Referrer-Policy', 'strict-origin-when-cross-origin');
    }

    // Permissions-Policy - controls browser features
    if (permissionsPolicy) {
      const permissionsPolicyDirectives = [
        'geolocation=(self)',
        'microphone=()',
        'camera=()',
        'payment=()',
        'usb=()',
        'magnetometer=()',
        'gyroscope=()',
        'accelerometer=()',
      ].join(', ');
      
      res.setHeader('Permissions-Policy', permissionsPolicyDirectives);
    }

    // X-DNS-Prefetch-Control - controls DNS prefetching
    res.setHeader('X-DNS-Prefetch-Control', 'off');

    // X-Download-Options - prevents IE from executing downloads
    res.setHeader('X-Download-Options', 'noopen');

    // X-Permitted-Cross-Domain-Policies - controls cross-domain policies
    res.setHeader('X-Permitted-Cross-Domain-Policies', 'none');

    next();
  };
}

/**
 * CORS headers with security considerations
 */
export function secureCors(allowedOrigins: string[]) {
  return (req: Request, res: Response, next: NextFunction): void => {
    const origin = req.headers.origin;

    // Check if origin is allowed
    if (origin && allowedOrigins.includes(origin)) {
      res.setHeader('Access-Control-Allow-Origin', origin);
      res.setHeader('Access-Control-Allow-Credentials', 'true');
      res.setHeader(
        'Access-Control-Allow-Methods',
        'GET, POST, PUT, DELETE, PATCH, OPTIONS'
      );
      res.setHeader(
        'Access-Control-Allow-Headers',
        'Origin, X-Requested-With, Content-Type, Accept, Authorization'
      );
      res.setHeader('Access-Control-Max-Age', '86400'); // 24 hours
    }

    // Handle preflight requests
    if (req.method === 'OPTIONS') {
      res.status(204).end();
      return;
    }

    next();
  };
}

/**
 * Prevent caching of sensitive data
 */
export function noCache(req: Request, res: Response, next: NextFunction): void {
  res.setHeader('Cache-Control', 'no-store, no-cache, must-revalidate, proxy-revalidate');
  res.setHeader('Pragma', 'no-cache');
  res.setHeader('Expires', '0');
  res.setHeader('Surrogate-Control', 'no-store');
  next();
}

/**
 * Set caching headers for static assets
 */
export function cacheStatic(maxAge: number = 31536000) {
  return (req: Request, res: Response, next: NextFunction): void => {
    res.setHeader('Cache-Control', `public, max-age=${maxAge}, immutable`);
    next();
  };
}
