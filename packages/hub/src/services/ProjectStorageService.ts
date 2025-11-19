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
 * Project Storage Service
 * Handles CRUD operations for game projects
 */

import { DatabaseService } from '../database/DatabaseService';
import { RedisService } from '../database/RedisService';
// import { v4 as uuidv4 } from 'uuid';  // Unused import

export interface Project {
  id: string;
  ownerId: string;
  name: string;
  description?: string;
  visibility: 'public' | 'private' | 'unlisted';
  createdAt: Date;
  updatedAt: Date;
  lastBuildAt?: Date;
  sizeBytes: number;
  assetCount: number;
}

export interface CreateProjectData {
  ownerId: string;
  name: string;
  description?: string;
  visibility?: 'public' | 'private' | 'unlisted';
}

export interface UpdateProjectData {
  name?: string;
  description?: string;
  visibility?: 'public' | 'private' | 'unlisted';
}

export class ProjectStorageService {
  private db = DatabaseService.getInstance();
  private redis = RedisService.getInstance();

  /**
   * Create a new project
   */
  async createProject(data: CreateProjectData): Promise<Project> {
    const result = await this.db.query<Project>(
      `INSERT INTO projects (owner_id, name, description, visibility)
       VALUES ($1, $2, $3, $4)
       RETURNING id, owner_id as "ownerId", name, description, visibility,
                 created_at as "createdAt", updated_at as "updatedAt",
                 last_build_at as "lastBuildAt", size_bytes as "sizeBytes",
                 asset_count as "assetCount"`,
      [data.ownerId, data.name, data.description, data.visibility || 'private']
    );

    const project = result.rows[0];
    await this.cacheProject(project);
    return project;
  }

  /**
   * Get project by ID
   */
  async getProject(projectId: string): Promise<Project | null> {
    // Check cache first
    const cached = await this.redis.getJson<Project>(`project:${projectId}`);
    if (cached) {
      return cached;
    }

    // Query database
    const result = await this.db.query<Project>(
      `SELECT id, owner_id as "ownerId", name, description, visibility,
              created_at as "createdAt", updated_at as "updatedAt",
              last_build_at as "lastBuildAt", size_bytes as "sizeBytes",
              asset_count as "assetCount"
       FROM projects WHERE id = $1`,
      [projectId]
    );

    if (result.rows.length === 0) {
      return null;
    }

    const project = result.rows[0];
    await this.cacheProject(project);
    return project;
  }

  /**
   * Get all projects for a user
   */
  async getUserProjects(
    userId: string,
    limit = 999999,
    offset = 0
  ): Promise<Project[]> {
    const result = await this.db.query<Project>(
      `SELECT id, owner_id as "ownerId", name, description, visibility,
              created_at as "createdAt", updated_at as "updatedAt",
              last_build_at as "lastBuildAt", size_bytes as "sizeBytes",
              asset_count as "assetCount"
       FROM projects
       WHERE owner_id = $1
       ORDER BY updated_at DESC
       LIMIT $2 OFFSET $3`,
      [userId, limit, offset]
    );

    return result.rows;
  }

  /**
   * Get public projects
   */
  async getPublicProjects(limit = 999999, offset = 0): Promise<Project[]> {
    const result = await this.db.query<Project>(
      `SELECT id, owner_id as "ownerId", name, description, visibility,
              created_at as "createdAt", updated_at as "updatedAt",
              last_build_at as "lastBuildAt", size_bytes as "sizeBytes",
              asset_count as "assetCount"
       FROM projects
       WHERE visibility = 'public'
       ORDER BY updated_at DESC
       LIMIT $1 OFFSET $2`,
      [limit, offset]
    );

    return result.rows;
  }

  /**
   * Update project
   */
  async updateProject(
    projectId: string,
    updates: UpdateProjectData
  ): Promise<Project | null> {
    const sets: string[] = [];
    const values: any[] = [];
    let paramIndex = 1;

    if (updates.name !== undefined) {
      sets.push(`name = $${paramIndex++}`);
      values.push(updates.name);
    }

    if (updates.description !== undefined) {
      sets.push(`description = $${paramIndex++}`);
      values.push(updates.description);
    }

    if (updates.visibility !== undefined) {
      sets.push(`visibility = $${paramIndex++}`);
      values.push(updates.visibility);
    }

    if (sets.length === 0) {
      return this.getProject(projectId);
    }

    sets.push(`updated_at = NOW()`);
    values.push(projectId);

    const result = await this.db.query<Project>(
      `UPDATE projects SET ${sets.join(', ')}
       WHERE id = $${paramIndex}
       RETURNING id, owner_id as "ownerId", name, description, visibility,
                 created_at as "createdAt", updated_at as "updatedAt",
                 last_build_at as "lastBuildAt", size_bytes as "sizeBytes",
                 asset_count as "assetCount"`,
      values
    );

    if (result.rows.length === 0) {
      return null;
    }

    const project = result.rows[0];
    await this.invalidateProjectCache(projectId);
    await this.cacheProject(project);
    return project;
  }

  /**
   * Delete project
   */
  async deleteProject(projectId: string): Promise<boolean> {
    const result = await this.db.query('DELETE FROM projects WHERE id = $1', [
      projectId,
    ]);

    if (result.rowCount && result.rowCount > 0) {
      await this.invalidateProjectCache(projectId);
      return true;
    }

    return false;
  }

  /**
   * Update project statistics
   */
  async updateProjectStats(
    projectId: string,
    sizeBytes: number,
    assetCount: number
  ): Promise<void> {
    await this.db.query(
      `UPDATE projects SET size_bytes = $1, asset_count = $2, updated_at = NOW()
       WHERE id = $3`,
      [sizeBytes, assetCount, projectId]
    );

    await this.invalidateProjectCache(projectId);
  }

  /**
   * Update last build time
   */
  async updateLastBuildAt(projectId: string): Promise<void> {
    await this.db.query(
      `UPDATE projects SET last_build_at = NOW(), updated_at = NOW()
       WHERE id = $1`,
      [projectId]
    );

    await this.invalidateProjectCache(projectId);
  }

  /**
   * Check if user owns project
   */
  async isProjectOwner(projectId: string, userId: string): Promise<boolean> {
    const result = await this.db.query(
      'SELECT 1 FROM projects WHERE id = $1 AND owner_id = $2',
      [projectId, userId]
    );

    return result.rows.length > 0;
  }

  /**
   * Search projects
   */
  async searchProjects(query: string, limit = 999999): Promise<Project[]> {
    const result = await this.db.query<Project>(
      `SELECT id, owner_id as "ownerId", name, description, visibility,
              created_at as "createdAt", updated_at as "updatedAt",
              last_build_at as "lastBuildAt", size_bytes as "sizeBytes",
              asset_count as "assetCount"
       FROM projects
       WHERE visibility = 'public'
         AND (name ILIKE $1 OR description ILIKE $1)
       ORDER BY updated_at DESC
       LIMIT $2`,
      [`%${query}%`, limit]
    );

    return result.rows;
  }

  /**
   * Cache project
   */
  private async cacheProject(project: Project): Promise<void> {
    await this.redis.setJson(`project:${project.id}`, project, 300); // 5 minutes
  }

  /**
   * Invalidate project cache
   */
  private async invalidateProjectCache(projectId: string): Promise<void> {
    await this.redis.del(`project:${projectId}`);
  }
}
