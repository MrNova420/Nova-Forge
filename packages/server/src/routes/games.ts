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
 * Games API Routes
 *
 * Endpoints for game management:
 * - GET /api/games - List all games
 * - GET /api/games/:id - Get game details
 * - POST /api/games - Create new game
 * - PUT /api/games/:id - Update game
 * - DELETE /api/games/:id - Delete game
 * - POST /api/games/:id/play - Track game play
 */

import { Router, Request, Response } from 'express';
import { authMiddleware } from '../middleware/auth';
import { pool } from '../config/database';

const router = Router();

// Get all games (paginated)
router.get('/', async (req: Request, res: Response) => {
  try {
    const page = parseInt(req.query.page as string) || 1;
    const limit = parseInt(req.query.limit as string) || 20;
    const offset = (page - 1) * limit;
    const category = req.query.category as string;

    let query = `
      SELECT g.*, u.username as creator_name,
        (SELECT COUNT(*) FROM game_plays WHERE game_id = g.id) as play_count,
        (SELECT AVG(rating) FROM game_ratings WHERE game_id = g.id) as avg_rating
      FROM games g
      JOIN users u ON g.creator_id = u.id
      WHERE g.published = true
    `;

    if (category) {
      query += ` AND g.category = $3`;
    }

    query += ` ORDER BY g.created_at DESC LIMIT $1 OFFSET $2`;

    const values = category ? [limit, offset, category] : [limit, offset];
    const result = await pool.query(query, values);

    // Get total count
    const countQuery = category
      ? 'SELECT COUNT(*) FROM games WHERE published = true AND category = $1'
      : 'SELECT COUNT(*) FROM games WHERE published = true';
    const countValues = category ? [category] : [];
    const countResult = await pool.query(countQuery, countValues);

    res.json({
      games: result.rows,
      total: parseInt(countResult.rows[0].count),
      page,
      limit,
    });
  } catch (error) {
    console.error('Error fetching games:', error);
    res.status(500).json({ error: 'Failed to fetch games' });
  }
});

// Get game by ID
router.get('/:id', async (req: Request, res: Response) => {
  try {
    const { id } = req.params;

    const result = await pool.query(
      `SELECT g.*, u.username as creator_name,
        (SELECT COUNT(*) FROM game_plays WHERE game_id = g.id) as play_count,
        (SELECT AVG(rating) FROM game_ratings WHERE game_id = g.id) as avg_rating
      FROM games g
      JOIN users u ON g.creator_id = u.id
      WHERE g.id = $1`,
      [id]
    );

    if (result.rows.length === 0) {
      return res.status(404).json({ error: 'Game not found' });
    }

    return res.json(result.rows[0]);
  } catch (error) {
    console.error('Error fetching game:', error);
    return res.status(500).json({ error: 'Failed to fetch game' });
  }
});

// Create new game (authenticated)
router.post('/', authMiddleware, async (req: any, res: Response) => {
  try {
    const { title, description, category, thumbnail, published } = req.body;
    const creator_id = req.user.userId;

    const result = await pool.query(
      `INSERT INTO games (title, description, category, thumbnail, creator_id, published)
       VALUES ($1, $2, $3, $4, $5, $6)
       RETURNING *`,
      [title, description, category, thumbnail, creator_id, published || false]
    );

    res.status(201).json(result.rows[0]);
  } catch (error) {
    console.error('Error creating game:', error);
    res.status(500).json({ error: 'Failed to create game' });
  }
});

// Update game (authenticated, creator only)
router.put('/:id', authMiddleware, async (req: any, res: Response) => {
  try {
    const { id } = req.params;
    const { title, description, category, thumbnail, published } = req.body;
    const creator_id = req.user.userId;

    // Check if user is the creator
    const checkResult = await pool.query(
      'SELECT * FROM games WHERE id = $1 AND creator_id = $2',
      [id, creator_id]
    );

    if (checkResult.rows.length === 0) {
      return res
        .status(403)
        .json({ error: 'Not authorized to update this game' });
    }

    const result = await pool.query(
      `UPDATE games 
       SET title = $1, description = $2, category = $3, thumbnail = $4, published = $5, updated_at = CURRENT_TIMESTAMP
       WHERE id = $6 AND creator_id = $7
       RETURNING *`,
      [title, description, category, thumbnail, published, id, creator_id]
    );

    return res.json(result.rows[0]);
  } catch (error) {
    console.error('Error updating game:', error);
    return res.status(500).json({ error: 'Failed to update game' });
  }
});

// Delete game (authenticated, creator only)
router.delete('/:id', authMiddleware, async (req: any, res: Response) => {
  try {
    const { id } = req.params;
    const creator_id = req.user.userId;

    const result = await pool.query(
      'DELETE FROM games WHERE id = $1 AND creator_id = $2 RETURNING *',
      [id, creator_id]
    );

    if (result.rows.length === 0) {
      return res
        .status(403)
        .json({ error: 'Not authorized to delete this game' });
    }

    return res.json({ message: 'Game deleted successfully' });
  } catch (error) {
    console.error('Error deleting game:', error);
    return res.status(500).json({ error: 'Failed to delete game' });
  }
});

// Track game play (authenticated)
router.post('/:id/play', authMiddleware, async (req: any, res: Response) => {
  try {
    const { id } = req.params;
    const user_id = req.user.userId;

    await pool.query(
      'INSERT INTO game_plays (game_id, user_id) VALUES ($1, $2)',
      [id, user_id]
    );

    res.json({ message: 'Game play tracked' });
  } catch (error) {
    console.error('Error tracking game play:', error);
    res.status(500).json({ error: 'Failed to track game play' });
  }
});

// Rate game (authenticated)
router.post('/:id/rate', authMiddleware, async (req: any, res: Response) => {
  try {
    const { id } = req.params;
    const { rating } = req.body;
    const user_id = req.user.userId;

    if (rating < 1 || rating > 5) {
      return res.status(400).json({ error: 'Rating must be between 1 and 5' });
    }

    await pool.query(
      `INSERT INTO game_ratings (game_id, user_id, rating)
       VALUES ($1, $2, $3)
       ON CONFLICT (game_id, user_id) 
       DO UPDATE SET rating = $3, updated_at = CURRENT_TIMESTAMP`,
      [id, user_id, rating]
    );

    return res.json({ message: 'Game rated successfully' });
  } catch (error) {
    console.error('Error rating game:', error);
    return res.status(500).json({ error: 'Failed to rate game' });
  }
});

export default router;
