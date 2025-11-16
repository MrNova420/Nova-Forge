/**
 * PostgreSQL Database Service
 */

import { Pool, PoolClient, QueryResult } from 'pg';
import { logger } from '../utils/logger';

export class DatabaseService {
  private static instance: DatabaseService;
  private pool: Pool | null = null;

  private constructor() {}

  static getInstance(): DatabaseService {
    if (!DatabaseService.instance) {
      DatabaseService.instance = new DatabaseService();
    }
    return DatabaseService.instance;
  }

  async connect(): Promise<void> {
    if (this.pool) {
      logger.warn('Database already connected');
      return;
    }

    this.pool = new Pool({
      host: process.env.DB_HOST || 'localhost',
      port: parseInt(process.env.DB_PORT || '5432', 10),
      database: process.env.DB_NAME || 'nova_hub',
      user: process.env.DB_USER || 'postgres',
      password: process.env.DB_PASSWORD || 'postgres',
      max: 20,
      idleTimeoutMillis: 30000,
      connectionTimeoutMillis: 2000,
    });

    // Test connection
    try {
      const client = await this.pool.connect();
      await client.query('SELECT NOW()');
      client.release();
      logger.info('Database connection successful');
    } catch (error) {
      logger.error('Database connection failed:', error);
      throw error;
    }

    // Initialize schema
    await this.initializeSchema();
  }

  async disconnect(): Promise<void> {
    if (this.pool) {
      await this.pool.end();
      this.pool = null;
      logger.info('Database disconnected');
    }
  }

  async query<T = any>(text: string, params?: any[]): Promise<QueryResult<T>> {
    if (!this.pool) {
      throw new Error('Database not connected');
    }
    return this.pool.query<T>(text, params);
  }

  async getClient(): Promise<PoolClient> {
    if (!this.pool) {
      throw new Error('Database not connected');
    }
    return this.pool.connect();
  }

  private async initializeSchema(): Promise<void> {
    const schema = `
      -- Users table
      CREATE TABLE IF NOT EXISTS users (
        id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
        email VARCHAR(255) UNIQUE NOT NULL,
        username VARCHAR(100) UNIQUE NOT NULL,
        password_hash VARCHAR(255) NOT NULL,
        display_name VARCHAR(200),
        avatar_url TEXT,
        created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
        updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
        last_login TIMESTAMP WITH TIME ZONE,
        is_active BOOLEAN DEFAULT true,
        is_verified BOOLEAN DEFAULT false
      );

      -- Projects table
      CREATE TABLE IF NOT EXISTS projects (
        id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
        owner_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
        name VARCHAR(200) NOT NULL,
        description TEXT,
        visibility VARCHAR(20) DEFAULT 'private' CHECK (visibility IN ('public', 'private', 'unlisted')),
        created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
        updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
        last_build_at TIMESTAMP WITH TIME ZONE,
        size_bytes BIGINT DEFAULT 0,
        asset_count INTEGER DEFAULT 0,
        UNIQUE(owner_id, name)
      );

      -- Project versions table
      CREATE TABLE IF NOT EXISTS project_versions (
        id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
        project_id UUID NOT NULL REFERENCES projects(id) ON DELETE CASCADE,
        version_number INTEGER NOT NULL,
        commit_message TEXT,
        created_by UUID NOT NULL REFERENCES users(id),
        created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
        snapshot_data JSONB,
        UNIQUE(project_id, version_number)
      );

      -- Assets table
      CREATE TABLE IF NOT EXISTS assets (
        id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
        project_id UUID NOT NULL REFERENCES projects(id) ON DELETE CASCADE,
        path VARCHAR(500) NOT NULL,
        type VARCHAR(50) NOT NULL,
        size_bytes BIGINT NOT NULL,
        hash VARCHAR(64) NOT NULL,
        storage_url TEXT NOT NULL,
        created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
        updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
        UNIQUE(project_id, path)
      );

      -- Builds table
      CREATE TABLE IF NOT EXISTS builds (
        id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
        project_id UUID NOT NULL REFERENCES projects(id) ON DELETE CASCADE,
        version_id UUID REFERENCES project_versions(id),
        target VARCHAR(50) NOT NULL,
        status VARCHAR(20) DEFAULT 'queued' CHECK (status IN ('queued', 'building', 'success', 'failed')),
        started_at TIMESTAMP WITH TIME ZONE,
        completed_at TIMESTAMP WITH TIME ZONE,
        build_url TEXT,
        error_message TEXT,
        created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
      );

      -- API Keys table
      CREATE TABLE IF NOT EXISTS api_keys (
        id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
        user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
        name VARCHAR(100) NOT NULL,
        key_hash VARCHAR(255) NOT NULL,
        last_used_at TIMESTAMP WITH TIME ZONE,
        expires_at TIMESTAMP WITH TIME ZONE,
        created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
        is_active BOOLEAN DEFAULT true
      );

      -- Indexes for better query performance
      CREATE INDEX IF NOT EXISTS idx_projects_owner ON projects(owner_id);
      CREATE INDEX IF NOT EXISTS idx_projects_updated ON projects(updated_at DESC);
      CREATE INDEX IF NOT EXISTS idx_assets_project ON assets(project_id);
      CREATE INDEX IF NOT EXISTS idx_builds_project ON builds(project_id);
      CREATE INDEX IF NOT EXISTS idx_builds_status ON builds(status);
      CREATE INDEX IF NOT EXISTS idx_versions_project ON project_versions(project_id);
    `;

    try {
      await this.query(schema);
      logger.info('Database schema initialized');
    } catch (error) {
      logger.error('Schema initialization failed:', error);
      throw error;
    }
  }
}
