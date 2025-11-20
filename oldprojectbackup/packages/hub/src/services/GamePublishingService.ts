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
 * Game Publishing Service
 * Handles publishing games from local builds to public distribution
 */

import { DatabaseService } from '../database/DatabaseService';
import { RedisService } from '../database/RedisService';
import { AssetStorageService } from '../storage/AssetStorageService';
import { BuildQueueService } from './BuildQueueService';
import * as path from 'path';
import * as fs from 'fs/promises';

export interface PublishedGame {
  id: string;
  projectId: string;
  buildId: string;
  name: string;
  description: string;
  version: string;
  author: string;
  authorId: string;
  tags: string[];
  category: string;
  thumbnail?: string;
  screenshots: string[];
  playUrl: string;
  downloadUrls: {
    web?: string;
    desktop?: string;
    mobile?: string;
  };
  visibility: 'public' | 'unlisted';
  downloads: number;
  plays: number;
  rating: number;
  ratingCount: number;
  publishedAt: Date;
  updatedAt: Date;
}

export interface PublishGameData {
  projectId: string;
  buildId: string;
  name: string;
  description: string;
  version: string;
  authorId: string;
  tags?: string[];
  category: string;
  thumbnail?: Buffer;
  screenshots?: Buffer[];
  visibility?: 'public' | 'unlisted';
}

export class GamePublishingService {
  private db = DatabaseService.getInstance();
  private redis = RedisService.getInstance();
  private assetService = new AssetStorageService();
  private buildService = new BuildQueueService();
  private cdnBasePath: string;

  constructor() {
    this.cdnBasePath = process.env.CDN_PATH || './cdn';
  }

  /**
   * Publish a game to the public registry
   */
  async publishGame(data: PublishGameData): Promise<PublishedGame> {
    // Verify build exists and is successful
    const build = await this.buildService.getBuild(data.buildId);
    if (!build || build.status !== 'success') {
      throw new Error('Build not found or not successful');
    }

    // Create CDN URLs for the game
    const playUrl = await this.createPlayUrl(data.projectId, data.buildId);
    const downloadUrls = await this.createDownloadUrls(
      data.projectId,
      data.buildId
    );

    // Upload thumbnail and screenshots to CDN
    let thumbnailUrl: string | undefined;
    if (data.thumbnail) {
      thumbnailUrl = await this.uploadThumbnail(data.projectId, data.thumbnail);
    }

    const screenshotUrls: string[] = [];
    if (data.screenshots) {
      for (const screenshot of data.screenshots) {
        const url = await this.uploadScreenshot(data.projectId, screenshot);
        screenshotUrls.push(url);
      }
    }

    // Get author info
    const authorResult = await this.db.query<{ username: string }>(
      'SELECT username FROM users WHERE id = $1',
      [data.authorId]
    );
    const author = authorResult.rows[0]?.username || 'Unknown';

    // Insert or update published game
    const result = await this.db.query<any>(
      `INSERT INTO published_games (
        project_id, build_id, name, description, version, author, author_id,
        tags, category, thumbnail, screenshots, play_url, download_urls,
        visibility, published_at
      ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, NOW())
      ON CONFLICT (project_id) 
      DO UPDATE SET
        build_id = $2, name = $3, description = $4, version = $5,
        tags = $8, category = $9, thumbnail = $10, screenshots = $11,
        play_url = $12, download_urls = $13, visibility = $14,
        updated_at = NOW()
      RETURNING *`,
      [
        data.projectId,
        data.buildId,
        data.name,
        data.description,
        data.version,
        author,
        data.authorId,
        JSON.stringify(data.tags || []),
        data.category,
        thumbnailUrl,
        JSON.stringify(screenshotUrls),
        playUrl,
        JSON.stringify(downloadUrls),
        data.visibility || 'public',
      ]
    );

    const published: PublishedGame = {
      id: result.rows[0].id,
      projectId: result.rows[0].project_id,
      buildId: result.rows[0].build_id,
      name: result.rows[0].name,
      description: result.rows[0].description,
      version: result.rows[0].version,
      author: result.rows[0].author,
      authorId: result.rows[0].author_id,
      tags: JSON.parse(result.rows[0].tags),
      category: result.rows[0].category,
      thumbnail: result.rows[0].thumbnail,
      screenshots: JSON.parse(result.rows[0].screenshots),
      playUrl: result.rows[0].play_url,
      downloadUrls: JSON.parse(result.rows[0].download_urls),
      visibility: result.rows[0].visibility,
      downloads: result.rows[0].downloads || 0,
      plays: result.rows[0].plays || 0,
      rating: result.rows[0].rating || 0,
      ratingCount: result.rows[0].rating_count || 0,
      publishedAt: result.rows[0].published_at,
      updatedAt: result.rows[0].updated_at,
    };

    // Cache the published game
    await this.cachePublishedGame(published);

    // Clear the games list cache to show new game
    await this.clearGamesListCache();

    return published;
  }

  /**
   * Get published game by ID
   */
  async getPublishedGame(gameId: string): Promise<PublishedGame | null> {
    // Check cache first
    const cached = await this.redis.getJson<PublishedGame>(
      `published_game:${gameId}`
    );
    if (cached) {
      return cached;
    }

    const result = await this.db.query<any>(
      'SELECT * FROM published_games WHERE id = $1',
      [gameId]
    );

    if (result.rows.length === 0) {
      return null;
    }

    const row = result.rows[0];
    const game: PublishedGame = {
      id: row.id,
      projectId: row.project_id,
      buildId: row.build_id,
      name: row.name,
      description: row.description,
      version: row.version,
      author: row.author,
      authorId: row.author_id,
      tags: JSON.parse(row.tags),
      category: row.category,
      thumbnail: row.thumbnail,
      screenshots: JSON.parse(row.screenshots),
      playUrl: row.play_url,
      downloadUrls: JSON.parse(row.download_urls),
      visibility: row.visibility,
      downloads: row.downloads || 0,
      plays: row.plays || 0,
      rating: row.rating || 0,
      ratingCount: row.rating_count || 0,
      publishedAt: row.published_at,
      updatedAt: row.updated_at,
    };

    await this.cachePublishedGame(game);
    return game;
  }

  /**
   * Get all published games
   */
  async getPublishedGames(
    options: {
      category?: string;
      tags?: string[];
      search?: string;
      sortBy?: 'newest' | 'popular' | 'rating';
      limit?: number;
      offset?: number;
    } = {}
  ): Promise<PublishedGame[]> {
    const {
      category,
      tags,
      search,
      sortBy = 'newest',
      limit = 999999,
      offset = 0,
    } = options;

    let query = `SELECT * FROM published_games WHERE visibility = 'public'`;
    const params: any[] = [];
    let paramIndex = 1;

    if (category) {
      query += ` AND category = $${paramIndex++}`;
      params.push(category);
    }

    if (tags && tags.length > 0) {
      query += ` AND tags @> $${paramIndex++}`;
      params.push(JSON.stringify(tags));
    }

    if (search) {
      query += ` AND (name ILIKE $${paramIndex++} OR description ILIKE $${paramIndex++})`;
      params.push(`%${search}%`, `%${search}%`);
    }

    // Sort
    switch (sortBy) {
      case 'popular':
        query += ' ORDER BY plays DESC, downloads DESC';
        break;
      case 'rating':
        query += ' ORDER BY rating DESC, rating_count DESC';
        break;
      default:
        query += ' ORDER BY published_at DESC';
    }

    query += ` LIMIT $${paramIndex++} OFFSET $${paramIndex++}`;
    params.push(limit, offset);

    const result = await this.db.query<any>(query, params);

    return result.rows.map((row) => ({
      id: row.id,
      projectId: row.project_id,
      buildId: row.build_id,
      name: row.name,
      description: row.description,
      version: row.version,
      author: row.author,
      authorId: row.author_id,
      tags: JSON.parse(row.tags),
      category: row.category,
      thumbnail: row.thumbnail,
      screenshots: JSON.parse(row.screenshots),
      playUrl: row.play_url,
      downloadUrls: JSON.parse(row.download_urls),
      visibility: row.visibility,
      downloads: row.downloads || 0,
      plays: row.plays || 0,
      rating: row.rating || 0,
      ratingCount: row.rating_count || 0,
      publishedAt: row.published_at,
      updatedAt: row.updated_at,
    }));
  }

  /**
   * Increment play count
   */
  async incrementPlayCount(gameId: string): Promise<void> {
    await this.db.query(
      'UPDATE published_games SET plays = plays + 1 WHERE id = $1',
      [gameId]
    );
    await this.redis.del(`published_game:${gameId}`);
  }

  /**
   * Increment download count
   */
  async incrementDownloadCount(gameId: string): Promise<void> {
    await this.db.query(
      'UPDATE published_games SET downloads = downloads + 1 WHERE id = $1',
      [gameId]
    );
    await this.redis.del(`published_game:${gameId}`);
  }

  /**
   * Rate game
   */
  async rateGame(
    gameId: string,
    userId: string,
    rating: number
  ): Promise<void> {
    if (rating < 1 || rating > 5) {
      throw new Error('Rating must be between 1 and 5');
    }

    // Insert or update rating
    await this.db.query(
      `INSERT INTO game_ratings (game_id, user_id, rating)
       VALUES ($1, $2, $3)
       ON CONFLICT (game_id, user_id)
       DO UPDATE SET rating = $3, updated_at = NOW()`,
      [gameId, userId, rating]
    );

    // Recalculate average rating
    const result = await this.db.query<{ avg: string; count: string }>(
      'SELECT AVG(rating) as avg, COUNT(*) as count FROM game_ratings WHERE game_id = $1',
      [gameId]
    );

    const avgRating = parseFloat(result.rows[0].avg);
    const ratingCount = parseInt(result.rows[0].count, 10);

    await this.db.query(
      'UPDATE published_games SET rating = $1, rating_count = $2 WHERE id = $3',
      [avgRating, ratingCount, gameId]
    );

    await this.redis.del(`published_game:${gameId}`);
  }

  /**
   * Unpublish game
   */
  async unpublishGame(gameId: string): Promise<boolean> {
    const result = await this.db.query(
      'DELETE FROM published_games WHERE id = $1',
      [gameId]
    );

    if (result.rowCount && result.rowCount > 0) {
      await this.redis.del(`published_game:${gameId}`);
      await this.clearGamesListCache();
      return true;
    }

    return false;
  }

  /**
   * Create play URL for the game
   */
  private async createPlayUrl(
    projectId: string,
    _buildId: string
  ): Promise<string> {
    const baseUrl = process.env.PLAY_BASE_URL || 'https://play.novaengine.io';
    return `${baseUrl}/game/${projectId}`;
  }

  /**
   * Create download URLs
   */
  private async createDownloadUrls(
    projectId: string,
    buildId: string
  ): Promise<{ web?: string; desktop?: string; mobile?: string }> {
    const baseUrl = process.env.CDN_BASE_URL || 'https://cdn.novaengine.io';
    return {
      web: `${baseUrl}/builds/${projectId}/${buildId}/web.zip`,
      desktop: `${baseUrl}/builds/${projectId}/${buildId}/desktop.zip`,
      mobile: `${baseUrl}/builds/${projectId}/${buildId}/mobile.zip`,
    };
  }

  /**
   * Upload thumbnail
   */
  private async uploadThumbnail(
    projectId: string,
    buffer: Buffer
  ): Promise<string> {
    const filename = `thumbnail-${Date.now()}.png`;
    const cdnPath = path.join(
      this.cdnBasePath,
      'thumbnails',
      projectId,
      filename
    );

    await fs.mkdir(path.dirname(cdnPath), { recursive: true });
    await fs.writeFile(cdnPath, buffer);

    const baseUrl = process.env.CDN_BASE_URL || 'https://cdn.novaengine.io';
    return `${baseUrl}/thumbnails/${projectId}/${filename}`;
  }

  /**
   * Upload screenshot
   */
  private async uploadScreenshot(
    projectId: string,
    buffer: Buffer
  ): Promise<string> {
    const filename = `screenshot-${Date.now()}.png`;
    const cdnPath = path.join(
      this.cdnBasePath,
      'screenshots',
      projectId,
      filename
    );

    await fs.mkdir(path.dirname(cdnPath), { recursive: true });
    await fs.writeFile(cdnPath, buffer);

    const baseUrl = process.env.CDN_BASE_URL || 'https://cdn.novaengine.io';
    return `${baseUrl}/screenshots/${projectId}/${filename}`;
  }

  /**
   * Cache published game
   */
  private async cachePublishedGame(game: PublishedGame): Promise<void> {
    await this.redis.setJson(`published_game:${game.id}`, game, 600); // 10 minutes
  }

  /**
   * Clear games list cache
   */
  private async clearGamesListCache(): Promise<void> {
    // Clear various list caches
    await this.redis.del('games:newest');
    await this.redis.del('games:popular');
    await this.redis.del('games:rated');
  }
}
