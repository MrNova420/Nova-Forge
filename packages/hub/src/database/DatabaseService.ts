/**
 * PostgreSQL Database Service
 */

import { Pool, PoolClient, QueryResult } from 'pg';
import { logger } from '../utils/logger';
import { InMemoryDatabase } from './InMemoryDatabase';

export class DatabaseService {
  private static instance: DatabaseService;
  private pool: Pool | null = null;
  private inMemoryDb: InMemoryDatabase | null = null;
  private useInMemory: boolean = false;

  private constructor() {}

  static getInstance(): DatabaseService {
    if (!DatabaseService.instance) {
      DatabaseService.instance = new DatabaseService();
    }
    return DatabaseService.instance;
  }

  async connect(): Promise<void> {
    if (this.pool || this.inMemoryDb) {
      logger.warn('Database already connected');
      return;
    }

    // Try PostgreSQL first
    try {
      this.pool = new Pool({
        host: process.env.DB_HOST || 'localhost',
        port: parseInt(process.env.DB_PORT || '5432', 10),
        database: process.env.DB_NAME || 'nova_hub',
        user: process.env.DB_USER || 'postgres',
        password: process.env.DB_PASSWORD || 'postgres',
        max: 100,
        idleTimeoutMillis: 300000,
        connectionTimeoutMillis: 10000,
      });

      const client = await this.pool.connect();
      await client.query('SELECT NOW()');
      client.release();
      logger.info('Database connection successful');

      // Initialize schema
      await this.initializeSchema();
    } catch (_error) {
      logger.warn(
        'PostgreSQL not available, falling back to in-memory database'
      );

      // Cleanup failed pool
      if (this.pool) {
        try {
          await this.pool.end();
        } catch {}
        this.pool = null;
      }

      // Use in-memory database
      this.useInMemory = true;
      this.inMemoryDb = InMemoryDatabase.getInstance();
      await this.inMemoryDb.connect();
    }
  }

  async disconnect(): Promise<void> {
    if (this.pool) {
      await this.pool.end();
      this.pool = null;
      logger.info('Database disconnected');
    }
    if (this.inMemoryDb) {
      await this.inMemoryDb.disconnect();
      this.inMemoryDb = null;
      logger.info('In-memory database disconnected');
    }
  }

  async query<T = any>(text: string, params?: any[]): Promise<QueryResult<T>> {
    if (this.useInMemory && this.inMemoryDb) {
      return this.inMemoryDb.query<T>(text, params);
    }

    if (!this.pool) {
      throw new Error('Database not connected');
    }
    return this.pool.query<T>(text, params);
  }

  async getClient(): Promise<PoolClient> {
    if (this.useInMemory) {
      throw new Error('Client not available in in-memory mode');
    }

    if (!this.pool) {
      throw new Error('Database not connected');
    }
    return this.pool.connect();
  }

  isUsingInMemory(): boolean {
    return this.useInMemory;
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

      -- Published games table (marketplace)
      CREATE TABLE IF NOT EXISTS published_games (
        id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
        project_id UUID UNIQUE NOT NULL REFERENCES projects(id) ON DELETE CASCADE,
        build_id UUID NOT NULL REFERENCES builds(id),
        name VARCHAR(200) NOT NULL,
        description TEXT,
        version VARCHAR(50) NOT NULL,
        author VARCHAR(100) NOT NULL,
        author_id UUID NOT NULL REFERENCES users(id),
        tags JSONB DEFAULT '[]',
        category VARCHAR(50) NOT NULL,
        thumbnail TEXT,
        screenshots JSONB DEFAULT '[]',
        play_url TEXT NOT NULL,
        download_urls JSONB NOT NULL,
        visibility VARCHAR(20) DEFAULT 'public' CHECK (visibility IN ('public', 'unlisted')),
        downloads INTEGER DEFAULT 0,
        plays INTEGER DEFAULT 0,
        rating DECIMAL(3,2) DEFAULT 0,
        rating_count INTEGER DEFAULT 0,
        published_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
        updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
      );

      -- Game ratings table
      CREATE TABLE IF NOT EXISTS game_ratings (
        id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
        game_id UUID NOT NULL REFERENCES published_games(id) ON DELETE CASCADE,
        user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
        rating INTEGER NOT NULL CHECK (rating >= 1 AND rating <= 5),
        created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
        updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
        UNIQUE(game_id, user_id)
      );

      -- Game servers table (multiplayer)
      CREATE TABLE IF NOT EXISTS game_servers (
        id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
        game_id UUID NOT NULL REFERENCES published_games(id) ON DELETE CASCADE,
        region VARCHAR(50) NOT NULL,
        host VARCHAR(255) NOT NULL,
        port INTEGER NOT NULL,
        status VARCHAR(20) DEFAULT 'starting' CHECK (status IN ('starting', 'running', 'stopping', 'stopped')),
        max_players INTEGER DEFAULT 100,
        current_players INTEGER DEFAULT 0,
        rooms INTEGER DEFAULT 0,
        cpu_usage DECIMAL(5,2) DEFAULT 0,
        memory_usage DECIMAL(5,2) DEFAULT 0,
        started_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
        last_heartbeat TIMESTAMP WITH TIME ZONE DEFAULT NOW()
      );

      -- Game rooms table (multiplayer sessions)
      CREATE TABLE IF NOT EXISTS game_rooms (
        id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
        server_id UUID NOT NULL REFERENCES game_servers(id) ON DELETE CASCADE,
        game_id UUID NOT NULL REFERENCES published_games(id),
        name VARCHAR(200) NOT NULL,
        host_user_id UUID NOT NULL REFERENCES users(id),
        current_players INTEGER DEFAULT 0,
        max_players INTEGER DEFAULT 8,
        is_public BOOLEAN DEFAULT true,
        status VARCHAR(20) DEFAULT 'waiting' CHECK (status IN ('waiting', 'starting', 'playing', 'finished')),
        game_mode VARCHAR(50),
        settings JSONB DEFAULT '{}',
        players JSONB DEFAULT '[]',
        created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
      );

      -- Indexes for better query performance
      CREATE INDEX IF NOT EXISTS idx_projects_owner ON projects(owner_id);
      CREATE INDEX IF NOT EXISTS idx_projects_updated ON projects(updated_at DESC);
      CREATE INDEX IF NOT EXISTS idx_assets_project ON assets(project_id);
      CREATE INDEX IF NOT EXISTS idx_builds_project ON builds(project_id);
      CREATE INDEX IF NOT EXISTS idx_builds_status ON builds(status);
      CREATE INDEX IF NOT EXISTS idx_versions_project ON project_versions(project_id);
      CREATE INDEX IF NOT EXISTS idx_published_games_category ON published_games(category);
      CREATE INDEX IF NOT EXISTS idx_published_games_published ON published_games(published_at DESC);
      CREATE INDEX IF NOT EXISTS idx_published_games_plays ON published_games(plays DESC);
      CREATE INDEX IF NOT EXISTS idx_published_games_rating ON published_games(rating DESC);
      CREATE INDEX IF NOT EXISTS idx_game_ratings_game ON game_ratings(game_id);
      CREATE INDEX IF NOT EXISTS idx_game_servers_game ON game_servers(game_id);
      CREATE INDEX IF NOT EXISTS idx_game_servers_region ON game_servers(region);
      CREATE INDEX IF NOT EXISTS idx_game_servers_status ON game_servers(status);
      CREATE INDEX IF NOT EXISTS idx_game_rooms_server ON game_rooms(server_id);
      CREATE INDEX IF NOT EXISTS idx_game_rooms_game ON game_rooms(game_id);
      CREATE INDEX IF NOT EXISTS idx_game_rooms_status ON game_rooms(status);
      CREATE INDEX IF NOT EXISTS idx_game_rooms_public ON game_rooms(is_public) WHERE is_public = true;
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
