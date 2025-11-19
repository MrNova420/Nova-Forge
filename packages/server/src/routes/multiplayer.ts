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
 * Multiplayer API Routes
 *
 * Endpoints for multiplayer features:
 * - GET /api/multiplayer/lobbies - List lobbies
 * - POST /api/multiplayer/lobbies - Create lobby
 * - GET /api/multiplayer/lobbies/:id - Get lobby details
 * - POST /api/multiplayer/lobbies/:id/join - Join lobby
 * - POST /api/multiplayer/lobbies/:id/leave - Leave lobby
 * - DELETE /api/multiplayer/lobbies/:id - Delete lobby
 * - POST /api/multiplayer/matchmaking/start - Start matchmaking
 * - POST /api/multiplayer/matchmaking/cancel - Cancel matchmaking
 * - GET /api/multiplayer/matches - Get match history
 */

import { Router, Request, Response } from 'express';
import { authMiddleware } from '../middleware/auth';
import { pool } from '../config/database';

const router = Router();

// Get all lobbies
router.get('/lobbies', authMiddleware, async (req: Request, res: Response) => {
  try {
    const game = req.query.game as string;
    const region = req.query.region as string;

    let query = `
      SELECT l.*, u.username as host_name, COUNT(lp.user_id) as current_players
      FROM lobbies l
      JOIN users u ON l.host_id = u.id
      LEFT JOIN lobby_players lp ON l.id = lp.lobby_id
      WHERE l.status = 'open'
    `;

    const params: any[] = [];
    let paramIndex = 1;

    if (game) {
      params.push(game);
      query += ` AND l.game_id = $${paramIndex}`;
      paramIndex++;
    }

    if (region) {
      params.push(region);
      query += ` AND l.region = $${paramIndex}`;
      paramIndex++;
    }

    query += ` GROUP BY l.id, u.username ORDER BY l.created_at DESC`;

    const result = await pool.query(query, params);
    res.json(result.rows);
  } catch (error) {
    console.error('Error fetching lobbies:', error);
    res.status(500).json({ error: 'Failed to fetch lobbies' });
  }
});

// Create lobby (authenticated)
router.post('/lobbies', authMiddleware, async (req: any, res: Response) => {
  try {
    const { game_id, name, max_players, region, game_mode, is_private } =
      req.body;
    const host_id = req.user.userId;

    const result = await pool.query(
      `INSERT INTO lobbies (game_id, host_id, name, max_players, region, game_mode, is_private, status)
       VALUES ($1, $2, $3, $4, $5, $6, $7, 'open')
       RETURNING *`,
      [
        game_id,
        host_id,
        name,
        max_players,
        region,
        game_mode,
        is_private || false,
      ]
    );

    // Add host as first player
    await pool.query(
      'INSERT INTO lobby_players (lobby_id, user_id) VALUES ($1, $2)',
      [result.rows[0].id, host_id]
    );

    res.status(201).json(result.rows[0]);
  } catch (error) {
    console.error('Error creating lobby:', error);
    res.status(500).json({ error: 'Failed to create lobby' });
  }
});

// Get lobby details
router.get(
  '/lobbies/:id',
  authMiddleware,
  async (req: Request, res: Response) => {
    try {
      const { id } = req.params;

      const lobby = await pool.query(
        `SELECT l.*, u.username as host_name
       FROM lobbies l
       JOIN users u ON l.host_id = u.id
       WHERE l.id = $1`,
        [id]
      );

      if (lobby.rows.length === 0) {
        return res.status(404).json({ error: 'Lobby not found' });
      }

      // Get players in lobby
      const players = await pool.query(
        `SELECT u.id, u.username, u.avatar, u.level, lp.joined_at
       FROM lobby_players lp
       JOIN users u ON lp.user_id = u.id
       WHERE lp.lobby_id = $1
       ORDER BY lp.joined_at`,
        [id]
      );

      return res.json({
        ...lobby.rows[0],
        players: players.rows,
      });
    } catch (error) {
      console.error('Error fetching lobby details:', error);
      return res.status(500).json({ error: 'Failed to fetch lobby details' });
    }
  }
);

// Join lobby (authenticated)
router.post(
  '/lobbies/:id/join',
  authMiddleware,
  async (req: any, res: Response) => {
    try {
      const { id } = req.params;
      const user_id = req.user.userId;

      // Check if lobby exists and is open
      const lobby = await pool.query(
        `SELECT l.*, COUNT(lp.user_id) as current_players
       FROM lobbies l
       LEFT JOIN lobby_players lp ON l.id = lp.lobby_id
       WHERE l.id = $1 AND l.status = 'open'
       GROUP BY l.id`,
        [id]
      );

      if (lobby.rows.length === 0) {
        return res.status(404).json({ error: 'Lobby not found or closed' });
      }

      if (lobby.rows[0].current_players >= lobby.rows[0].max_players) {
        return res.status(400).json({ error: 'Lobby is full' });
      }

      // Check if user is already in lobby
      const existing = await pool.query(
        'SELECT * FROM lobby_players WHERE lobby_id = $1 AND user_id = $2',
        [id, user_id]
      );

      if (existing.rows.length > 0) {
        return res.status(400).json({ error: 'Already in lobby' });
      }

      await pool.query(
        'INSERT INTO lobby_players (lobby_id, user_id) VALUES ($1, $2)',
        [id, user_id]
      );

      return res.json({ message: 'Joined lobby successfully' });
    } catch (error) {
      console.error('Error joining lobby:', error);
      return res.status(500).json({ error: 'Failed to join lobby' });
    }
  }
);

// Leave lobby (authenticated)
router.post(
  '/lobbies/:id/leave',
  authMiddleware,
  async (req: any, res: Response) => {
    try {
      const { id } = req.params;
      const user_id = req.user.userId;

      await pool.query(
        'DELETE FROM lobby_players WHERE lobby_id = $1 AND user_id = $2',
        [id, user_id]
      );

      // Check if lobby is now empty
      const remaining = await pool.query(
        'SELECT COUNT(*) FROM lobby_players WHERE lobby_id = $1',
        [id]
      );

      if (parseInt(remaining.rows[0].count) === 0) {
        // Delete empty lobby
        await pool.query('DELETE FROM lobbies WHERE id = $1', [id]);
      }

      res.json({ message: 'Left lobby successfully' });
    } catch (error) {
      console.error('Error leaving lobby:', error);
      res.status(500).json({ error: 'Failed to leave lobby' });
    }
  }
);

// Delete lobby (authenticated, host only)
router.delete(
  '/lobbies/:id',
  authMiddleware,
  async (req: any, res: Response) => {
    try {
      const { id } = req.params;
      const user_id = req.user.userId;

      const result = await pool.query(
        'DELETE FROM lobbies WHERE id = $1 AND host_id = $2 RETURNING *',
        [id, user_id]
      );

      if (result.rows.length === 0) {
        return res
          .status(403)
          .json({ error: 'Not authorized to delete this lobby' });
      }

      return res.json({ message: 'Lobby deleted successfully' });
    } catch (error) {
      console.error('Error deleting lobby:', error);
      return res.status(500).json({ error: 'Failed to delete lobby' });
    }
  }
);

// Start matchmaking (authenticated)
router.post(
  '/matchmaking/start',
  authMiddleware,
  async (req: any, res: Response) => {
    try {
      const { game_id, region, game_mode, skill_range } = req.body;
      const user_id = req.user.userId;

      // Add to matchmaking queue
      await pool.query(
        `INSERT INTO matchmaking_queue (user_id, game_id, region, game_mode, skill_range)
       VALUES ($1, $2, $3, $4, $5)
       ON CONFLICT (user_id) DO UPDATE 
       SET game_id = $2, region = $3, game_mode = $4, skill_range = $5, created_at = CURRENT_TIMESTAMP`,
        [user_id, game_id, region, game_mode, skill_range || 100]
      );

      res.json({ message: 'Matchmaking started', status: 'searching' });

      // TODO: Implement actual matchmaking logic via WebSocket
    } catch (error) {
      console.error('Error starting matchmaking:', error);
      res.status(500).json({ error: 'Failed to start matchmaking' });
    }
  }
);

// Cancel matchmaking (authenticated)
router.post(
  '/matchmaking/cancel',
  authMiddleware,
  async (req: any, res: Response) => {
    try {
      const user_id = req.user.userId;

      await pool.query('DELETE FROM matchmaking_queue WHERE user_id = $1', [
        user_id,
      ]);

      res.json({ message: 'Matchmaking cancelled' });
    } catch (error) {
      console.error('Error cancelling matchmaking:', error);
      res.status(500).json({ error: 'Failed to cancel matchmaking' });
    }
  }
);

// Get match history (authenticated)
router.get('/matches', authMiddleware, async (req: any, res: Response) => {
  try {
    const user_id = req.user.userId;
    const limit = parseInt(req.query.limit as string) || 50;

    const result = await pool.query(
      `SELECT m.*, g.title as game_title, g.thumbnail as game_thumbnail
       FROM matches m
       JOIN games g ON m.game_id = g.id
       WHERE m.id IN (
         SELECT match_id FROM match_players WHERE user_id = $1
       )
       ORDER BY m.created_at DESC
       LIMIT $2`,
      [user_id, limit]
    );

    res.json(result.rows);
  } catch (error) {
    console.error('Error fetching match history:', error);
    res.status(500).json({ error: 'Failed to fetch match history' });
  }
});

export default router;
