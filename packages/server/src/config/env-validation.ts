/**
 * Environment Variable Validation
 * Ensures all required environment variables are set with proper types
 */

interface EnvConfig {
  // Server
  NODE_ENV: 'development' | 'production' | 'test';
  PORT: number;
  HOST: string;

  // Database
  DATABASE_URL?: string;
  DATABASE_HOST: string;
  DATABASE_PORT: number;
  DATABASE_NAME: string;
  DATABASE_USER: string;
  DATABASE_PASSWORD: string;

  // Redis (optional)
  REDIS_URL?: string;
  REDIS_HOST?: string;
  REDIS_PORT?: number;

  // Security
  JWT_SECRET: string; // REQUIRED - no fallback
  JWT_EXPIRES_IN: string;
  SESSION_SECRET: string;

  // CORS
  CORS_ORIGIN: string;

  // File Upload
  UPLOAD_DIR: string;
  MAX_FILE_SIZE: number;
  ALLOWED_FILE_TYPES: string;

  // AWS (optional)
  AWS_REGION?: string;
  AWS_ACCESS_KEY_ID?: string;
  AWS_SECRET_ACCESS_KEY?: string;
  AWS_S3_BUCKET?: string;

  // Third-party (optional)
  GOOGLE_ANALYTICS_ID?: string;
  SENTRY_DSN?: string;
  SMTP_HOST?: string;
  SMTP_PORT?: number;
  SMTP_USER?: string;
  SMTP_PASSWORD?: string;
  SMTP_FROM?: string;

  // Development
  DEBUG: boolean;
  LOG_LEVEL: 'error' | 'warn' | 'info' | 'debug' | 'verbose';
  ENABLE_PROFILING: boolean;
}

/**
 * Validates and parses environment variables
 * Throws error if required variables are missing or invalid
 */
export function validateEnv(): EnvConfig {
  const errors: string[] = [];

  // Helper to get required string
  const getRequiredString = (key: string): string => {
    const value = process.env[key];
    if (!value || value.trim() === '') {
      errors.push(`${key} is required but not set`);
      return '';
    }
    return value;
  };

  // Helper to get optional string
  const getOptionalString = (key: string): string | undefined => {
    const value = process.env[key];
    return value && value.trim() !== '' ? value : undefined;
  };

  // Helper to get required number
  const getRequiredNumber = (key: string): number => {
    const value = process.env[key];
    if (!value) {
      errors.push(`${key} is required but not set`);
      return 0;
    }
    const num = parseInt(value, 10);
    if (isNaN(num)) {
      errors.push(`${key} must be a valid number, got: ${value}`);
      return 0;
    }
    return num;
  };

  // Helper to get optional number
  const getOptionalNumber = (key: string): number | undefined => {
    const value = process.env[key];
    if (!value) return undefined;
    const num = parseInt(value, 10);
    if (isNaN(num)) {
      errors.push(`${key} must be a valid number, got: ${value}`);
      return undefined;
    }
    return num;
  };

  // Helper to get boolean
  const getBoolean = (key: string, defaultValue: boolean): boolean => {
    const value = process.env[key];
    if (!value) return defaultValue;
    return value.toLowerCase() === 'true';
  };

  // Validate NODE_ENV
  const nodeEnv = process.env.NODE_ENV || 'development';
  if (!['development', 'production', 'test'].includes(nodeEnv)) {
    errors.push(
      `NODE_ENV must be 'development', 'production', or 'test', got: ${nodeEnv}`
    );
  }

  // Validate LOG_LEVEL
  const logLevel = process.env.LOG_LEVEL || 'info';
  if (!['error', 'warn', 'info', 'debug', 'verbose'].includes(logLevel)) {
    errors.push(
      `LOG_LEVEL must be one of: error, warn, info, debug, verbose, got: ${logLevel}`
    );
  }

  // Build config
  const config: EnvConfig = {
    // Server
    NODE_ENV: nodeEnv as EnvConfig['NODE_ENV'],
    PORT: getRequiredNumber('PORT'),
    HOST: process.env.HOST || '0.0.0.0',

    // Database
    DATABASE_URL: getOptionalString('DATABASE_URL'),
    DATABASE_HOST: getRequiredString('DATABASE_HOST'),
    DATABASE_PORT: getRequiredNumber('DATABASE_PORT'),
    DATABASE_NAME: getRequiredString('DATABASE_NAME'),
    DATABASE_USER: getRequiredString('DATABASE_USER'),
    DATABASE_PASSWORD: getRequiredString('DATABASE_PASSWORD'),

    // Redis (optional)
    REDIS_URL: getOptionalString('REDIS_URL'),
    REDIS_HOST: getOptionalString('REDIS_HOST'),
    REDIS_PORT: getOptionalNumber('REDIS_PORT'),

    // Security - JWT_SECRET is REQUIRED with no fallback
    JWT_SECRET: getRequiredString('JWT_SECRET'),
    JWT_EXPIRES_IN: process.env.JWT_EXPIRES_IN || '7d',
    SESSION_SECRET: getRequiredString('SESSION_SECRET'),

    // CORS
    CORS_ORIGIN: process.env.CORS_ORIGIN || 'http://localhost:3000',

    // File Upload
    UPLOAD_DIR: process.env.UPLOAD_DIR || './uploads',
    MAX_FILE_SIZE: getOptionalNumber('MAX_FILE_SIZE') || 10485760, // 10MB default
    ALLOWED_FILE_TYPES:
      process.env.ALLOWED_FILE_TYPES ||
      'image/png,image/jpeg,image/jpg,image/gif,model/gltf-binary,audio/mpeg',

    // AWS (optional)
    AWS_REGION: getOptionalString('AWS_REGION'),
    AWS_ACCESS_KEY_ID: getOptionalString('AWS_ACCESS_KEY_ID'),
    AWS_SECRET_ACCESS_KEY: getOptionalString('AWS_SECRET_ACCESS_KEY'),
    AWS_S3_BUCKET: getOptionalString('AWS_S3_BUCKET'),

    // Third-party (optional)
    GOOGLE_ANALYTICS_ID: getOptionalString('GOOGLE_ANALYTICS_ID'),
    SENTRY_DSN: getOptionalString('SENTRY_DSN'),
    SMTP_HOST: getOptionalString('SMTP_HOST'),
    SMTP_PORT: getOptionalNumber('SMTP_PORT'),
    SMTP_USER: getOptionalString('SMTP_USER'),
    SMTP_PASSWORD: getOptionalString('SMTP_PASSWORD'),
    SMTP_FROM: getOptionalString('SMTP_FROM'),

    // Development
    DEBUG: getBoolean('DEBUG', false),
    LOG_LEVEL: logLevel as EnvConfig['LOG_LEVEL'],
    ENABLE_PROFILING: getBoolean('ENABLE_PROFILING', false),
  };

  // Additional validation rules

  // Production-specific checks
  if (config.NODE_ENV === 'production') {
    if (config.JWT_SECRET.length < 32) {
      errors.push('JWT_SECRET must be at least 32 characters in production');
    }
    if (config.SESSION_SECRET.length < 32) {
      errors.push(
        'SESSION_SECRET must be at least 32 characters in production'
      );
    }
    if (
      config.JWT_SECRET === 'your-secure-secret-key-change-this-in-production'
    ) {
      errors.push(
        'JWT_SECRET must be changed from default value in production'
      );
    }
    if (config.SESSION_SECRET === 'your-session-secret-change-this') {
      errors.push(
        'SESSION_SECRET must be changed from default value in production'
      );
    }
    if (!config.SENTRY_DSN) {
      console.warn(
        '⚠️  SENTRY_DSN not set - error tracking disabled in production'
      );
    }
  }

  // AWS validation - if one is set, all should be set
  const awsVars = [
    config.AWS_REGION,
    config.AWS_ACCESS_KEY_ID,
    config.AWS_SECRET_ACCESS_KEY,
    config.AWS_S3_BUCKET,
  ];
  const awsVarsSet = awsVars.filter((v) => v !== undefined).length;
  if (awsVarsSet > 0 && awsVarsSet < awsVars.length) {
    errors.push('If using AWS, all AWS_* variables must be set');
  }

  // SMTP validation - if one is set, all should be set
  if (config.SMTP_HOST) {
    if (!config.SMTP_PORT)
      errors.push('SMTP_PORT required when SMTP_HOST is set');
    if (!config.SMTP_USER)
      errors.push('SMTP_USER required when SMTP_HOST is set');
    if (!config.SMTP_PASSWORD)
      errors.push('SMTP_PASSWORD required when SMTP_HOST is set');
  }

  // If there are validation errors, throw
  if (errors.length > 0) {
    console.error('❌ Environment variable validation failed:\n');
    errors.forEach((error) => console.error(`  - ${error}`));
    console.error(
      '\nPlease check your .env file and ensure all required variables are set.'
    );
    console.error('See .env.example for reference.\n');
    throw new Error(
      `Environment validation failed with ${errors.length} error(s)`
    );
  }

  // Log configuration summary (safe - no secrets)
  console.log('✅ Environment variables validated successfully');
  console.log(`   NODE_ENV: ${config.NODE_ENV}`);
  console.log(`   PORT: ${config.PORT}`);
  console.log(
    `   DATABASE: ${config.DATABASE_HOST}:${config.DATABASE_PORT}/${config.DATABASE_NAME}`
  );
  console.log(`   REDIS: ${config.REDIS_URL || 'disabled (in-memory)'}`);
  console.log(
    `   AWS S3: ${config.AWS_S3_BUCKET || 'disabled (local storage)'}`
  );
  console.log(
    `   ERROR_TRACKING: ${config.SENTRY_DSN ? 'enabled' : 'disabled'}`
  );
  console.log('');

  return config;
}

// Export validated config singleton
export const env = validateEnv();
