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
 * Build Queue Service
 * Manages automated build job queue
 */

import { DatabaseService } from '../database/DatabaseService';
import { RedisService } from '../database/RedisService';

export interface Build {
  id: string;
  projectId: string;
  versionId?: string;
  target: 'web' | 'desktop' | 'mobile';
  status: 'queued' | 'building' | 'success' | 'failed';
  startedAt?: Date;
  completedAt?: Date;
  buildUrl?: string;
  errorMessage?: string;
  createdAt: Date;
}

export interface CreateBuildData {
  projectId: string;
  versionId?: string;
  target: 'web' | 'desktop' | 'mobile';
}

export class BuildQueueService {
  private db = DatabaseService.getInstance();
  private redis = RedisService.getInstance();

  /**
   * Queue a new build
   */
  async queueBuild(data: CreateBuildData): Promise<Build> {
    const result = await this.db.query<Build>(
      `INSERT INTO builds (project_id, version_id, target, status)
       VALUES ($1, $2, $3, 'queued')
       RETURNING id, project_id as "projectId", version_id as "versionId",
                 target, status, started_at as "startedAt",
                 completed_at as "completedAt", build_url as "buildUrl",
                 error_message as "errorMessage", created_at as "createdAt"`,
      [data.projectId, data.versionId, data.target]
    );

    const build = result.rows[0];

    // Add to Redis queue
    await this.redis.set(`build:queue:${build.id}`, JSON.stringify(build));

    return build;
  }

  /**
   * Get build by ID
   */
  async getBuild(buildId: string): Promise<Build | null> {
    const result = await this.db.query<Build>(
      `SELECT id, project_id as "projectId", version_id as "versionId",
              target, status, started_at as "startedAt",
              completed_at as "completedAt", build_url as "buildUrl",
              error_message as "errorMessage", created_at as "createdAt"
       FROM builds WHERE id = $1`,
      [buildId]
    );

    return result.rows.length > 0 ? result.rows[0] : null;
  }

  /**
   * Get project builds
   */
  async getProjectBuilds(
    projectId: string,
    limit = 999999,
    offset = 0
  ): Promise<Build[]> {
    const result = await this.db.query<Build>(
      `SELECT id, project_id as "projectId", version_id as "versionId",
              target, status, started_at as "startedAt",
              completed_at as "completedAt", build_url as "buildUrl",
              error_message as "errorMessage", created_at as "createdAt"
       FROM builds
       WHERE project_id = $1
       ORDER BY created_at DESC
       LIMIT $2 OFFSET $3`,
      [projectId, limit, offset]
    );

    return result.rows;
  }

  /**
   * Get queued builds
   */
  async getQueuedBuilds(limit = 999999): Promise<Build[]> {
    const result = await this.db.query<Build>(
      `SELECT id, project_id as "projectId", version_id as "versionId",
              target, status, started_at as "startedAt",
              completed_at as "completedAt", build_url as "buildUrl",
              error_message as "errorMessage", created_at as "createdAt"
       FROM builds
       WHERE status = 'queued'
       ORDER BY created_at ASC
       LIMIT $1`,
      [limit]
    );

    return result.rows;
  }

  /**
   * Start build
   */
  async startBuild(buildId: string): Promise<Build | null> {
    const result = await this.db.query<Build>(
      `UPDATE builds
       SET status = 'building', started_at = NOW()
       WHERE id = $1 AND status = 'queued'
       RETURNING id, project_id as "projectId", version_id as "versionId",
                 target, status, started_at as "startedAt",
                 completed_at as "completedAt", build_url as "buildUrl",
                 error_message as "errorMessage", created_at as "createdAt"`,
      [buildId]
    );

    return result.rows.length > 0 ? result.rows[0] : null;
  }

  /**
   * Complete build
   */
  async completeBuild(
    buildId: string,
    buildUrl: string
  ): Promise<Build | null> {
    const result = await this.db.query<Build>(
      `UPDATE builds
       SET status = 'success', completed_at = NOW(), build_url = $2
       WHERE id = $1
       RETURNING id, project_id as "projectId", version_id as "versionId",
                 target, status, started_at as "startedAt",
                 completed_at as "completedAt", build_url as "buildUrl",
                 error_message as "errorMessage", created_at as "createdAt"`,
      [buildId, buildUrl]
    );

    if (result.rows.length > 0) {
      // Update project's last build time
      const build = result.rows[0];
      await this.db.query(
        'UPDATE projects SET last_build_at = NOW() WHERE id = $1',
        [build.projectId]
      );

      // Remove from queue
      await this.redis.del(`build:queue:${buildId}`);
    }

    return result.rows.length > 0 ? result.rows[0] : null;
  }

  /**
   * Fail build
   */
  async failBuild(
    buildId: string,
    errorMessage: string
  ): Promise<Build | null> {
    const result = await this.db.query<Build>(
      `UPDATE builds
       SET status = 'failed', completed_at = NOW(), error_message = $2
       WHERE id = $1
       RETURNING id, project_id as "projectId", version_id as "versionId",
                 target, status, started_at as "startedAt",
                 completed_at as "completedAt", build_url as "buildUrl",
                 error_message as "errorMessage", created_at as "createdAt"`,
      [buildId, errorMessage]
    );

    if (result.rows.length > 0) {
      // Remove from queue
      await this.redis.del(`build:queue:${buildId}`);
    }

    return result.rows.length > 0 ? result.rows[0] : null;
  }

  /**
   * Cancel build
   */
  async cancelBuild(buildId: string): Promise<boolean> {
    const result = await this.db.query(
      `UPDATE builds SET status = 'failed', error_message = 'Cancelled by user'
       WHERE id = $1 AND status IN ('queued', 'building')`,
      [buildId]
    );

    if (result.rowCount && result.rowCount > 0) {
      await this.redis.del(`build:queue:${buildId}`);
      return true;
    }

    return false;
  }

  /**
   * Get build statistics
   */
  async getBuildStats(projectId?: string): Promise<{
    total: number;
    queued: number;
    building: number;
    success: number;
    failed: number;
  }> {
    const whereClause = projectId ? 'WHERE project_id = $1' : '';
    const params = projectId ? [projectId] : [];

    const result = await this.db.query<{
      status: string;
      count: string;
    }>(
      `SELECT status, COUNT(*) as count
       FROM builds
       ${whereClause}
       GROUP BY status`,
      params
    );

    const stats = {
      total: 0,
      queued: 0,
      building: 0,
      success: 0,
      failed: 0,
    };

    for (const row of result.rows) {
      const count = parseInt(row.count, 10);
      stats.total += count;

      switch (row.status) {
        case 'queued':
          stats.queued = count;
          break;
        case 'building':
          stats.building = count;
          break;
        case 'success':
          stats.success = count;
          break;
        case 'failed':
          stats.failed = count;
          break;
      }
    }

    return stats;
  }
}
