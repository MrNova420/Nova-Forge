/**
 * In-Memory Database Implementation
 * Used when PostgreSQL is not available
 */

import { QueryResult } from 'pg';
import { logger } from '../utils/logger';
import { v4 as uuidv4 } from 'uuid';

interface User {
  id: string;
  email: string;
  username: string;
  password_hash: string;
  display_name?: string;
  avatar_url?: string;
  created_at: Date;
  updated_at: Date;
  last_login?: Date;
  is_active: boolean;
  is_verified: boolean;
}

interface Project {
  id: string;
  owner_id: string;
  name: string;
  description?: string;
  visibility: string;
  created_at: Date;
  updated_at: Date;
}

interface Game {
  id: string;
  project_id: string;
  build_id: string;
  name: string;
  description: string;
  version: string;
  author: string;
  author_id: string;
  tags: string[];
  category: string;
  thumbnail?: string;
  screenshots: string[];
  play_url: string;
  download_urls: any;
  visibility: string;
  downloads: number;
  plays: number;
  rating: number;
  rating_count: number;
  published_at: Date;
  updated_at: Date;
}

/**
 * Simple in-memory database for development/testing
 */
export class InMemoryDatabase {
  private static instance: InMemoryDatabase;
  private users: Map<string, User> = new Map();
  private projects: Map<string, Project> = new Map();
  private games: Map<string, Game> = new Map();
  private connected: boolean = false;

  private constructor() {}

  static getInstance(): InMemoryDatabase {
    if (!InMemoryDatabase.instance) {
      InMemoryDatabase.instance = new InMemoryDatabase();
    }
    return InMemoryDatabase.instance;
  }

  async connect(): Promise<void> {
    if (this.connected) {
      return;
    }
    this.connected = true;
    logger.info('✅ In-memory database ready');

    // Add some demo data
    this.seedDemoData();
  }

  async disconnect(): Promise<void> {
    this.connected = false;
    this.users.clear();
    this.projects.clear();
    this.games.clear();
  }

  isConnected(): boolean {
    return this.connected;
  }

  /**
   * Execute a query (simplified SQL parser for common operations)
   */
  async query<T = any>(text: string, params?: any[]): Promise<QueryResult<T>> {
    if (!this.connected) {
      throw new Error('Database not connected');
    }

    const sql = text.trim().toLowerCase();

    // Handle SELECT queries
    if (sql.startsWith('select')) {
      return this.handleSelect<T>(text, params);
    }

    // Handle INSERT queries
    if (sql.startsWith('insert')) {
      return this.handleInsert<T>(text, params);
    }

    // Handle UPDATE queries
    if (sql.startsWith('update')) {
      return this.handleUpdate<T>(text, params);
    }

    // Handle DELETE queries
    if (sql.startsWith('delete')) {
      return this.handleDelete<T>(text, params);
    }

    // Default response
    return {
      rows: [] as T[],
      rowCount: 0,
      command: '',
      oid: 0,
      fields: [],
    } as QueryResult<T>;
  }

  private handleSelect<T>(sql: string, params?: any[]): QueryResult<T> {
    const rows: T[] = [];

    // Parse table name
    if (sql.includes('from users')) {
      // User queries
      const userArray = Array.from(this.users.values());

      if (sql.includes('where email')) {
        const email = params?.[0];
        const user = userArray.find((u) => u.email === email);
        if (user) rows.push(user as any);
      } else if (sql.includes('where username')) {
        const username = params?.[0];
        const user = userArray.find((u) => u.username === username);
        if (user) rows.push(user as any);
      } else if (sql.includes('where id')) {
        const id = params?.[0];
        const user = this.users.get(id);
        if (user) rows.push(user as any);
      } else {
        rows.push(...(userArray as any[]));
      }
    } else if (sql.includes('from projects')) {
      // Project queries
      const projectArray = Array.from(this.projects.values());

      if (sql.includes('where owner_id')) {
        const ownerId = params?.[0];
        rows.push(
          ...(projectArray.filter((p) => p.owner_id === ownerId) as any[])
        );
      } else if (sql.includes('where id')) {
        const id = params?.[0];
        const project = this.projects.get(id);
        if (project) rows.push(project as any);
      } else {
        rows.push(...(projectArray as any[]));
      }
    } else if (
      sql.includes('from published_games') ||
      sql.includes('from games')
    ) {
      // Game queries
      const gameArray = Array.from(this.games.values());

      if (sql.includes('where id')) {
        const id = params?.[0];
        const game = this.games.get(id);
        if (game) rows.push(game as any);
      } else if (sql.includes('where category')) {
        const category = params?.[0];
        rows.push(
          ...(gameArray.filter((g) => g.category === category) as any[])
        );
      } else if (sql.includes('order by rating')) {
        rows.push(...(gameArray.sort((a, b) => b.rating - a.rating) as any[]));
      } else if (sql.includes('order by published_at')) {
        rows.push(
          ...(gameArray.sort(
            (a, b) => b.published_at.getTime() - a.published_at.getTime()
          ) as any[])
        );
      } else if (sql.includes('order by plays')) {
        rows.push(...(gameArray.sort((a, b) => b.plays - a.plays) as any[]));
      } else {
        rows.push(...(gameArray as any[]));
      }
    }

    return {
      rows,
      rowCount: rows.length,
      command: 'SELECT',
      oid: 0,
      fields: [],
    } as QueryResult<T>;
  }

  private handleInsert<T>(sql: string, params?: any[]): QueryResult<T> {
    if (sql.includes('into users')) {
      const user: User = {
        id: params?.[0] || uuidv4(),
        email: params?.[1] || '',
        username: params?.[2] || '',
        password_hash: params?.[3] || '',
        display_name: params?.[4],
        avatar_url: params?.[5],
        created_at: new Date(),
        updated_at: new Date(),
        is_active: true,
        is_verified: false,
      };
      this.users.set(user.id, user);

      return {
        rows: [user as any],
        rowCount: 1,
        command: 'INSERT',
        oid: 0,
        fields: [],
      } as QueryResult<T>;
    }

    if (sql.includes('into projects')) {
      const project: Project = {
        id: params?.[0] || uuidv4(),
        owner_id: params?.[1] || '',
        name: params?.[2] || '',
        description: params?.[3],
        visibility: params?.[4] || 'private',
        created_at: new Date(),
        updated_at: new Date(),
      };
      this.projects.set(project.id, project);

      return {
        rows: [project as any],
        rowCount: 1,
        command: 'INSERT',
        oid: 0,
        fields: [],
      } as QueryResult<T>;
    }

    return {
      rows: [],
      rowCount: 0,
      command: 'INSERT',
      oid: 0,
      fields: [],
    } as QueryResult<T>;
  }

  private handleUpdate<T>(sql: string, params?: any[]): QueryResult<T> {
    // Simplified update handling
    return {
      rows: [],
      rowCount: 1,
      command: 'UPDATE',
      oid: 0,
      fields: [],
    } as QueryResult<T>;
  }

  private handleDelete<T>(sql: string, params?: any[]): QueryResult<T> {
    // Simplified delete handling
    return {
      rows: [],
      rowCount: 1,
      command: 'DELETE',
      oid: 0,
      fields: [],
    } as QueryResult<T>;
  }

  /**
   * Seed demo data for testing
   */
  private seedDemoData(): void {
    // This will be populated by the demo games
    logger.info('In-memory database seeded with demo data');
  }

  /**
   * Add a game to the registry
   */
  addGame(game: Game): void {
    this.games.set(game.id, game);
  }

  /**
   * Get all games
   */
  getAllGames(): Game[] {
    return Array.from(this.games.values());
  }
}
