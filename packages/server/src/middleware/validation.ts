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
 * Input Validation Middleware
 * Validates and sanitizes user input to prevent injection attacks
 */

import { Request, Response, NextFunction } from 'express';

/**
 * Validation rule types
 */
type ValidationRule = {
  type: 'string' | 'number' | 'boolean' | 'email' | 'url' | 'object' | 'array';
  required?: boolean;
  min?: number;
  max?: number;
  pattern?: RegExp;
  enum?: any[];
  custom?: (value: any) => boolean | string;
};

type ValidationSchema = {
  [key: string]: ValidationRule;
};

interface ValidationError {
  field: string;
  message: string;
}

/**
 * Validate request body against schema
 */
export function validateBody(schema: ValidationSchema) {
  return (req: Request, res: Response, next: NextFunction): void => {
    const errors = validateData(req.body, schema);

    if (errors.length > 0) {
      res.status(400).json({
        error: 'Validation Error',
        message: 'Request body validation failed',
        errors,
      });
      return;
    }

    next();
  };
}

/**
 * Validate query parameters against schema
 */
export function validateQuery(schema: ValidationSchema) {
  return (req: Request, res: Response, next: NextFunction): void => {
    const errors = validateData(req.query, schema);

    if (errors.length > 0) {
      res.status(400).json({
        error: 'Validation Error',
        message: 'Query parameters validation failed',
        errors,
      });
      return;
    }

    next();
  };
}

/**
 * Validate path parameters against schema
 */
export function validateParams(schema: ValidationSchema) {
  return (req: Request, res: Response, next: NextFunction): void => {
    const errors = validateData(req.params, schema);

    if (errors.length > 0) {
      res.status(400).json({
        error: 'Validation Error',
        message: 'Path parameters validation failed',
        errors,
      });
      return;
    }

    next();
  };
}

/**
 * Core validation logic
 */
function validateData(data: any, schema: ValidationSchema): ValidationError[] {
  const errors: ValidationError[] = [];

  // Check for required fields
  Object.keys(schema).forEach((field) => {
    const rule = schema[field];
    if (!rule) return;

    const value = data[field];

    // Required check
    if (
      rule.required &&
      (value === undefined || value === null || value === '')
    ) {
      errors.push({
        field,
        message: `${field} is required`,
      });
      return;
    }

    // Skip validation if field is not required and not provided
    if (!rule.required && (value === undefined || value === null)) {
      return;
    }

    // Type validation
    const typeError = validateType(field, value, rule.type);
    if (typeError) {
      errors.push(typeError);
      return;
    }

    // String validations
    if (rule.type === 'string' && typeof value === 'string') {
      if (rule.min !== undefined && value.length < rule.min) {
        errors.push({
          field,
          message: `${field} must be at least ${rule.min} characters`,
        });
      }
      if (rule.max !== undefined && value.length > rule.max) {
        errors.push({
          field,
          message: `${field} must be at most ${rule.max} characters`,
        });
      }
      if (rule.pattern && !rule.pattern.test(value)) {
        errors.push({
          field,
          message: `${field} format is invalid`,
        });
      }
    }

    // Number validations
    if (rule.type === 'number' && typeof value === 'number') {
      if (rule.min !== undefined && value < rule.min) {
        errors.push({
          field,
          message: `${field} must be at least ${rule.min}`,
        });
      }
      if (rule.max !== undefined && value > rule.max) {
        errors.push({
          field,
          message: `${field} must be at most ${rule.max}`,
        });
      }
    }

    // Enum validation
    if (rule?.enum && !rule.enum.includes(value)) {
      errors.push({
        field,
        message: `${field} must be one of: ${rule.enum.join(', ')}`,
      });
    }

    // Custom validation
    if (rule?.custom) {
      const customResult = rule.custom(value);
      if (customResult !== true) {
        errors.push({
          field,
          message:
            typeof customResult === 'string'
              ? customResult
              : `${field} is invalid`,
        });
      }
    }
  });

  return errors;
}

/**
 * Validate value type
 */
function validateType(
  field: string,
  value: any,
  type: string
): ValidationError | null {
  switch (type) {
    case 'string':
      if (typeof value !== 'string') {
        return { field, message: `${field} must be a string` };
      }
      break;

    case 'number':
      if (typeof value !== 'number' || isNaN(value)) {
        return { field, message: `${field} must be a number` };
      }
      break;

    case 'boolean':
      if (typeof value !== 'boolean') {
        return { field, message: `${field} must be a boolean` };
      }
      break;

    case 'email':
      if (typeof value !== 'string' || !isValidEmail(value)) {
        return { field, message: `${field} must be a valid email address` };
      }
      break;

    case 'url':
      if (typeof value !== 'string' || !isValidUrl(value)) {
        return { field, message: `${field} must be a valid URL` };
      }
      break;

    case 'object':
      if (typeof value !== 'object' || value === null || Array.isArray(value)) {
        return { field, message: `${field} must be an object` };
      }
      break;

    case 'array':
      if (!Array.isArray(value)) {
        return { field, message: `${field} must be an array` };
      }
      break;
  }

  return null;
}

/**
 * Email validation
 */
function isValidEmail(email: string): boolean {
  const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
  return emailRegex.test(email);
}

/**
 * URL validation
 */
function isValidUrl(url: string): boolean {
  try {
    new URL(url);
    return true;
  } catch {
    return false;
  }
}

/**
 * Sanitize HTML to prevent XSS
 */
export function sanitizeHtml(html: string): string {
  return html
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#x27;')
    .replace(/\//g, '&#x2F;');
}

/**
 * Common validation schemas
 */
export const authSchemas = {
  register: {
    username: {
      type: 'string' as const,
      required: true,
      min: 3,
      max: 20,
      pattern: /^[a-zA-Z0-9_-]+$/,
    },
    email: {
      type: 'email' as const,
      required: true,
      max: 255,
    },
    password: {
      type: 'string' as const,
      required: true,
      min: 8,
      max: 128,
      custom: (value: string) => {
        // Must contain at least one uppercase, lowercase, number, and special char
        const hasUpperCase = /[A-Z]/.test(value);
        const hasLowerCase = /[a-z]/.test(value);
        const hasNumber = /[0-9]/.test(value);
        const hasSpecialChar = /[!@#$%^&*(),.?":{}|<>]/.test(value);

        if (!hasUpperCase || !hasLowerCase || !hasNumber || !hasSpecialChar) {
          return 'Password must contain uppercase, lowercase, number, and special character';
        }
        return true;
      },
    },
  },
  login: {
    email: {
      type: 'email' as const,
      required: true,
    },
    password: {
      type: 'string' as const,
      required: true,
    },
  },
};
