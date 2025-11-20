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
 * Version Control Service
 * Git-like versioning for projects
 */

import { DatabaseService } from '../database/DatabaseService';
// import { v4 as uuidv4 } from 'uuid';  // Unused import

export interface ProjectVersion {
  id: string;
  projectId: string;
  versionNumber: number;
  commitMessage?: string;
  createdBy: string;
  createdAt: Date;
  snapshotData?: any;
}

export interface CreateVersionData {
  projectId: string;
  commitMessage?: string;
  createdBy: string;
  snapshotData?: any;
}

export class VersionControlService {
  private db = DatabaseService.getInstance();

  /**
   * Create a new version
   */
  async createVersion(data: CreateVersionData): Promise<ProjectVersion> {
    // Get next version number
    const versionNumber = await this.getNextVersionNumber(data.projectId);

    const result = await this.db.query<ProjectVersion>(
      `INSERT INTO project_versions (project_id, version_number, commit_message, created_by, snapshot_data)
       VALUES ($1, $2, $3, $4, $5)
       RETURNING id, project_id as "projectId", version_number as "versionNumber",
                 commit_message as "commitMessage", created_by as "createdBy",
                 created_at as "createdAt", snapshot_data as "snapshotData"`,
      [
        data.projectId,
        versionNumber,
        data.commitMessage,
        data.createdBy,
        JSON.stringify(data.snapshotData),
      ]
    );

    return result.rows[0];
  }

  /**
   * Get version by ID
   */
  async getVersion(versionId: string): Promise<ProjectVersion | null> {
    const result = await this.db.query<ProjectVersion>(
      `SELECT id, project_id as "projectId", version_number as "versionNumber",
              commit_message as "commitMessage", created_by as "createdBy",
              created_at as "createdAt", snapshot_data as "snapshotData"
       FROM project_versions WHERE id = $1`,
      [versionId]
    );

    return result.rows.length > 0 ? result.rows[0] : null;
  }

  /**
   * Get all versions for a project
   */
  async getProjectVersions(
    projectId: string,
    limit = 999999,
    offset = 0
  ): Promise<ProjectVersion[]> {
    const result = await this.db.query<ProjectVersion>(
      `SELECT id, project_id as "projectId", version_number as "versionNumber",
              commit_message as "commitMessage", created_by as "createdBy",
              created_at as "createdAt", snapshot_data as "snapshotData"
       FROM project_versions
       WHERE project_id = $1
       ORDER BY version_number DESC
       LIMIT $2 OFFSET $3`,
      [projectId, limit, offset]
    );

    return result.rows;
  }

  /**
   * Get specific version number
   */
  async getVersionByNumber(
    projectId: string,
    versionNumber: number
  ): Promise<ProjectVersion | null> {
    const result = await this.db.query<ProjectVersion>(
      `SELECT id, project_id as "projectId", version_number as "versionNumber",
              commit_message as "commitMessage", created_by as "createdBy",
              created_at as "createdAt", snapshot_data as "snapshotData"
       FROM project_versions
       WHERE project_id = $1 AND version_number = $2`,
      [projectId, versionNumber]
    );

    return result.rows.length > 0 ? result.rows[0] : null;
  }

  /**
   * Get latest version
   */
  async getLatestVersion(projectId: string): Promise<ProjectVersion | null> {
    const result = await this.db.query<ProjectVersion>(
      `SELECT id, project_id as "projectId", version_number as "versionNumber",
              commit_message as "commitMessage", created_by as "createdBy",
              created_at as "createdAt", snapshot_data as "snapshotData"
       FROM project_versions
       WHERE project_id = $1
       ORDER BY version_number DESC
       LIMIT 1`,
      [projectId]
    );

    return result.rows.length > 0 ? result.rows[0] : null;
  }

  /**
   * Get version count
   */
  async getVersionCount(projectId: string): Promise<number> {
    const result = await this.db.query<{ count: string }>(
      'SELECT COUNT(*) as count FROM project_versions WHERE project_id = $1',
      [projectId]
    );

    return parseInt(result.rows[0].count, 10);
  }

  /**
   * Compare two versions
   */
  async compareVersions(
    versionId1: string,
    versionId2: string
  ): Promise<{
    version1: ProjectVersion;
    version2: ProjectVersion;
    diff: any;
  }> {
    const [v1, v2] = await Promise.all([
      this.getVersion(versionId1),
      this.getVersion(versionId2),
    ]);

    if (!v1 || !v2) {
      throw new Error('One or both versions not found');
    }

    // Simple diff calculation (can be enhanced with proper diff algorithm)
    const diff = {
      versionNumberDiff: Math.abs(v1.versionNumber - v2.versionNumber),
      older: v1.versionNumber < v2.versionNumber ? v1 : v2,
      newer: v1.versionNumber > v2.versionNumber ? v1 : v2,
    };

    return { version1: v1, version2: v2, diff };
  }

  /**
   * Restore project to specific version
   */
  async restoreVersion(
    projectId: string,
    versionId: string,
    userId: string
  ): Promise<ProjectVersion> {
    const version = await this.getVersion(versionId);
    if (!version) {
      throw new Error('Version not found');
    }

    if (version.projectId !== projectId) {
      throw new Error('Version does not belong to this project');
    }

    // Create new version with the restored data
    return this.createVersion({
      projectId,
      commitMessage: `Restored from version ${version.versionNumber}`,
      createdBy: userId,
      snapshotData: version.snapshotData,
    });
  }

  /**
   * Delete version
   */
  async deleteVersion(versionId: string): Promise<boolean> {
    const result = await this.db.query(
      'DELETE FROM project_versions WHERE id = $1',
      [versionId]
    );

    return result.rowCount !== null && result.rowCount > 0;
  }

  /**
   * Get next version number for a project
   */
  private async getNextVersionNumber(projectId: string): Promise<number> {
    const result = await this.db.query<{ max: number | null }>(
      'SELECT MAX(version_number) as max FROM project_versions WHERE project_id = $1',
      [projectId]
    );

    const maxVersion = result.rows[0].max;
    return maxVersion === null ? 1 : maxVersion + 1;
  }

  /**
   * Get version history summary
   */
  async getVersionHistory(projectId: string): Promise<{
    totalVersions: number;
    firstVersion: ProjectVersion | null;
    latestVersion: ProjectVersion | null;
    recentVersions: ProjectVersion[];
  }> {
    const [count, recent] = await Promise.all([
      this.getVersionCount(projectId),
      this.getProjectVersions(projectId, 5, 0),
    ]);

    return {
      totalVersions: count,
      firstVersion: recent.length > 0 ? recent[recent.length - 1] : null,
      latestVersion: recent.length > 0 ? recent[0] : null,
      recentVersions: recent,
    };
  }
}
