/**
 * Asset Storage Service
 * Handles asset upload, download, and management
 */

import { DatabaseService } from '../database/DatabaseService';
import { createHash } from 'crypto';
import * as path from 'path';
import * as fs from 'fs/promises';

export interface Asset {
  id: string;
  projectId: string;
  path: string;
  type: string;
  sizeBytes: number;
  hash: string;
  storageUrl: string;
  createdAt: Date;
  updatedAt: Date;
}

export interface UploadAssetData {
  projectId: string;
  path: string;
  type: string;
  buffer: Buffer;
}

export class AssetStorageService {
  private db = DatabaseService.getInstance();
  private storageBasePath: string;

  constructor() {
    this.storageBasePath = process.env.STORAGE_PATH || './storage';
  }

  /**
   * Upload asset
   */
  async uploadAsset(data: UploadAssetData): Promise<Asset> {
    // Calculate hash
    const hash = this.calculateHash(data.buffer);

    // Generate storage path
    const storageUrl = await this.saveToStorage(
      data.projectId,
      data.path,
      data.buffer
    );

    // Save to database
    const result = await this.db.query<Asset>(
      `INSERT INTO assets (project_id, path, type, size_bytes, hash, storage_url)
       VALUES ($1, $2, $3, $4, $5, $6)
       ON CONFLICT (project_id, path)
       DO UPDATE SET type = $3, size_bytes = $4, hash = $5, storage_url = $6, updated_at = NOW()
       RETURNING id, project_id as "projectId", path, type, size_bytes as "sizeBytes",
                 hash, storage_url as "storageUrl", created_at as "createdAt",
                 updated_at as "updatedAt"`,
      [
        data.projectId,
        data.path,
        data.type,
        data.buffer.length,
        hash,
        storageUrl,
      ]
    );

    // Update project statistics
    await this.updateProjectStats(data.projectId);

    return result.rows[0];
  }

  /**
   * Get asset by ID
   */
  async getAsset(assetId: string): Promise<Asset | null> {
    const result = await this.db.query<Asset>(
      `SELECT id, project_id as "projectId", path, type, size_bytes as "sizeBytes",
              hash, storage_url as "storageUrl", created_at as "createdAt",
              updated_at as "updatedAt"
       FROM assets WHERE id = $1`,
      [assetId]
    );

    return result.rows.length > 0 ? result.rows[0] : null;
  }

  /**
   * Get all assets for a project
   */
  async getProjectAssets(
    projectId: string,
    limit = 999999,
    offset = 0
  ): Promise<Asset[]> {
    const result = await this.db.query<Asset>(
      `SELECT id, project_id as "projectId", path, type, size_bytes as "sizeBytes",
              hash, storage_url as "storageUrl", created_at as "createdAt",
              updated_at as "updatedAt"
       FROM assets
       WHERE project_id = $1
       ORDER BY path ASC
       LIMIT $2 OFFSET $3`,
      [projectId, limit, offset]
    );

    return result.rows;
  }

  /**
   * Get asset by path
   */
  async getAssetByPath(
    projectId: string,
    assetPath: string
  ): Promise<Asset | null> {
    const result = await this.db.query<Asset>(
      `SELECT id, project_id as "projectId", path, type, size_bytes as "sizeBytes",
              hash, storage_url as "storageUrl", created_at as "createdAt",
              updated_at as "updatedAt"
       FROM assets
       WHERE project_id = $1 AND path = $2`,
      [projectId, assetPath]
    );

    return result.rows.length > 0 ? result.rows[0] : null;
  }

  /**
   * Download asset
   */
  async downloadAsset(assetId: string): Promise<Buffer> {
    const asset = await this.getAsset(assetId);
    if (!asset) {
      throw new Error('Asset not found');
    }

    return this.loadFromStorage(asset.storageUrl);
  }

  /**
   * Delete asset
   */
  async deleteAsset(assetId: string): Promise<boolean> {
    const asset = await this.getAsset(assetId);
    if (!asset) {
      return false;
    }

    // Delete from storage
    await this.deleteFromStorage(asset.storageUrl);

    // Delete from database
    const result = await this.db.query('DELETE FROM assets WHERE id = $1', [
      assetId,
    ]);

    if (result.rowCount && result.rowCount > 0) {
      // Update project statistics
      await this.updateProjectStats(asset.projectId);
      return true;
    }

    return false;
  }

  /**
   * Delete all assets for a project
   */
  async deleteProjectAssets(projectId: string): Promise<number> {
    const assets = await this.getProjectAssets(projectId);

    // Delete from storage
    await Promise.all(
      assets.map((asset) => this.deleteFromStorage(asset.storageUrl))
    );

    // Delete from database
    const result = await this.db.query(
      'DELETE FROM assets WHERE project_id = $1',
      [projectId]
    );

    return result.rowCount || 0;
  }

  /**
   * Get assets by type
   */
  async getAssetsByType(projectId: string, type: string): Promise<Asset[]> {
    const result = await this.db.query<Asset>(
      `SELECT id, project_id as "projectId", path, type, size_bytes as "sizeBytes",
              hash, storage_url as "storageUrl", created_at as "createdAt",
              updated_at as "updatedAt"
       FROM assets
       WHERE project_id = $1 AND type = $2
       ORDER BY path ASC`,
      [projectId, type]
    );

    return result.rows;
  }

  /**
   * Search assets
   */
  async searchAssets(projectId: string, query: string): Promise<Asset[]> {
    const result = await this.db.query<Asset>(
      `SELECT id, project_id as "projectId", path, type, size_bytes as "sizeBytes",
              hash, storage_url as "storageUrl", created_at as "createdAt",
              updated_at as "updatedAt"
       FROM assets
       WHERE project_id = $1 AND path ILIKE $2
       ORDER BY path ASC`,
      [projectId, `%${query}%`]
    );

    return result.rows;
  }

  /**
   * Get project storage stats
   */
  async getProjectStorageStats(projectId: string): Promise<{
    totalSize: number;
    assetCount: number;
    byType: Record<string, { count: number; size: number }>;
  }> {
    const result = await this.db.query<{
      type: string;
      count: string;
      total_size: string;
    }>(
      `SELECT type, COUNT(*) as count, SUM(size_bytes) as total_size
       FROM assets
       WHERE project_id = $1
       GROUP BY type`,
      [projectId]
    );

    const byType: Record<string, { count: number; size: number }> = {};
    let totalSize = 0;
    let assetCount = 0;

    for (const row of result.rows) {
      const count = parseInt(row.count, 10);
      const size = parseInt(row.total_size, 10);
      byType[row.type] = { count, size };
      totalSize += size;
      assetCount += count;
    }

    return { totalSize, assetCount, byType };
  }

  /**
   * Calculate file hash
   */
  private calculateHash(buffer: Buffer): string {
    return createHash('sha256').update(buffer).digest('hex');
  }

  /**
   * Save to storage (filesystem - can be replaced with S3/cloud storage)
   */
  private async saveToStorage(
    projectId: string,
    assetPath: string,
    buffer: Buffer
  ): Promise<string> {
    const storagePath = path.join(this.storageBasePath, projectId, assetPath);
    const storageDir = path.dirname(storagePath);

    // Ensure directory exists
    await fs.mkdir(storageDir, { recursive: true });

    // Write file
    await fs.writeFile(storagePath, buffer);

    return storagePath;
  }

  /**
   * Load from storage
   */
  private async loadFromStorage(storageUrl: string): Promise<Buffer> {
    return fs.readFile(storageUrl);
  }

  /**
   * Delete from storage
   */
  private async deleteFromStorage(storageUrl: string): Promise<void> {
    try {
      await fs.unlink(storageUrl);
    } catch (_error) {
      // Ignore if file doesn't exist
    }
  }

  /**
   * Update project statistics
   */
  private async updateProjectStats(projectId: string): Promise<void> {
    const stats = await this.getProjectStorageStats(projectId);

    await this.db.query(
      `UPDATE projects SET size_bytes = $1, asset_count = $2, updated_at = NOW()
       WHERE id = $3`,
      [stats.totalSize, stats.assetCount, projectId]
    );
  }
}
