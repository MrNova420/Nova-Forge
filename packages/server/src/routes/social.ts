/**
 * Social API Routes
 *
 * Endpoints for social features:
 * - GET /api/social/profile/:id - Get user profile
 * - PUT /api/social/profile - Update own profile
 * - GET /api/social/friends - Get friends list
 * - POST /api/social/friends/:id - Add friend
 * - DELETE /api/social/friends/:id - Remove friend
 * - GET /api/social/achievements - Get achievements
 * - POST /api/social/achievements/:id/unlock - Unlock achievement
 * - GET /api/social/leaderboard - Get leaderboard
 * - GET /api/social/activity - Get activity feed
 */

import { Router, Request, Response } from 'express';
import { authMiddleware } from '../middleware/auth';
import { pool } from '../config/database';

const router = Router();

// Get user profile
router.get('/profile/:id', async (req: Request, res: Response) => {
  try {
    const { id } = req.params;

    const result = await pool.query(
      `SELECT id, username, email, level, experience, avatar, title, bio, 
              games_created, games_played, achievements_unlocked, created_at
       FROM users WHERE id = $1`,
      [id]
    );

    if (result.rows.length === 0) {
      return res.status(404).json({ error: 'User not found' });
    }

    return res.json(result.rows[0]);
  } catch (error) {
    console.error('Error fetching profile:', error);
    return res.status(500).json({ error: 'Failed to fetch profile' });
  }
});

// Update own profile (authenticated)
router.put('/profile', authMiddleware, async (req: any, res: Response) => {
  try {
    const user_id = req.user.userId;
    const { avatar, title, bio } = req.body;

    const result = await pool.query(
      `UPDATE users 
       SET avatar = $1, title = $2, bio = $3, updated_at = CURRENT_TIMESTAMP
       WHERE id = $4
       RETURNING id, username, email, level, experience, avatar, title, bio`,
      [avatar, title, bio, user_id]
    );

    res.json(result.rows[0]);
  } catch (error) {
    console.error('Error updating profile:', error);
    res.status(500).json({ error: 'Failed to update profile' });
  }
});

// Get friends list (authenticated)
router.get('/friends', authMiddleware, async (req: any, res: Response) => {
  try {
    const user_id = req.user.userId;

    const result = await pool.query(
      `SELECT u.id, u.username, u.avatar, u.level, u.title, f.status, f.created_at as friends_since
       FROM friendships f
       JOIN users u ON (f.user_id = $1 AND f.friend_id = u.id) OR (f.friend_id = $1 AND f.user_id = u.id)
       WHERE (f.user_id = $1 OR f.friend_id = $1) AND f.status = 'accepted'
       ORDER BY u.username`,
      [user_id]
    );

    res.json(result.rows);
  } catch (error) {
    console.error('Error fetching friends:', error);
    res.status(500).json({ error: 'Failed to fetch friends' });
  }
});

// Send friend request (authenticated)
router.post('/friends/:id', authMiddleware, async (req: any, res: Response) => {
  try {
    const user_id = req.user.userId;
    const friend_id = parseInt(req.params.id);

    if (user_id === friend_id) {
      return res.status(400).json({ error: 'Cannot add yourself as friend' });
    }

    // Check if friendship already exists
    const existing = await pool.query(
      `SELECT * FROM friendships 
       WHERE (user_id = $1 AND friend_id = $2) OR (user_id = $2 AND friend_id = $1)`,
      [user_id, friend_id]
    );

    if (existing.rows.length > 0) {
      return res
        .status(400)
        .json({ error: 'Friendship already exists or pending' });
    }

    await pool.query(
      `INSERT INTO friendships (user_id, friend_id, status)
       VALUES ($1, $2, 'pending')`,
      [user_id, friend_id]
    );

    return res.json({ message: 'Friend request sent' });
  } catch (error) {
    console.error('Error sending friend request:', error);
    return res.status(500).json({ error: 'Failed to send friend request' });
  }
});

// Remove friend (authenticated)
router.delete(
  '/friends/:id',
  authMiddleware,
  async (req: any, res: Response) => {
    try {
      const user_id = req.user.userId;
      const friend_id = parseInt(req.params.id);

      await pool.query(
        `DELETE FROM friendships 
       WHERE (user_id = $1 AND friend_id = $2) OR (user_id = $2 AND friend_id = $1)`,
        [user_id, friend_id]
      );

      res.json({ message: 'Friend removed' });
    } catch (error) {
      console.error('Error removing friend:', error);
      res.status(500).json({ error: 'Failed to remove friend' });
    }
  }
);

// Get all achievements with user progress (authenticated)
router.get('/achievements', authMiddleware, async (req: any, res: Response) => {
  try {
    const user_id = req.user.userId;

    const result = await pool.query(
      `SELECT a.*, ua.unlocked_at, ua.progress,
              CASE WHEN ua.user_id IS NOT NULL THEN true ELSE false END as unlocked
       FROM achievements a
       LEFT JOIN user_achievements ua ON a.id = ua.achievement_id AND ua.user_id = $1
       ORDER BY a.rarity DESC, a.name`,
      [user_id]
    );

    res.json(result.rows);
  } catch (error) {
    console.error('Error fetching achievements:', error);
    res.status(500).json({ error: 'Failed to fetch achievements' });
  }
});

// Unlock achievement (authenticated)
router.post(
  '/achievements/:id/unlock',
  authMiddleware,
  async (req: any, res: Response) => {
    try {
      const user_id = req.user.userId;
      const achievement_id = parseInt(req.params.id);

      // Check if already unlocked
      const existing = await pool.query(
        'SELECT * FROM user_achievements WHERE user_id = $1 AND achievement_id = $2',
        [user_id, achievement_id]
      );

      if (existing.rows.length > 0) {
        return res.status(400).json({ error: 'Achievement already unlocked' });
      }

      await pool.query(
        `INSERT INTO user_achievements (user_id, achievement_id, progress, unlocked_at)
       VALUES ($1, $2, 100, CURRENT_TIMESTAMP)`,
        [user_id, achievement_id]
      );

      // Update user achievements count
      await pool.query(
        'UPDATE users SET achievements_unlocked = achievements_unlocked + 1 WHERE id = $1',
        [user_id]
      );

      return res.json({ message: 'Achievement unlocked!' });
    } catch (error) {
      console.error('Error unlocking achievement:', error);
      return res.status(500).json({ error: 'Failed to unlock achievement' });
    }
  }
);

// Get leaderboard
router.get('/leaderboard', async (req: Request, res: Response) => {
  try {
    const limit = parseInt(req.query.limit as string) || 100;
    const timeframe = (req.query.timeframe as string) || 'all_time';

    let query = `
      SELECT id, username, level, experience, games_created, games_played, achievements_unlocked,
             (level * 1000 + experience) as score
      FROM users
    `;

    if (timeframe === 'this_month') {
      query += ` WHERE created_at >= DATE_TRUNC('month', CURRENT_DATE)`;
    } else if (timeframe === 'this_week') {
      query += ` WHERE created_at >= DATE_TRUNC('week', CURRENT_DATE)`;
    }

    query += ` ORDER BY score DESC LIMIT $1`;

    const result = await pool.query(query, [limit]);

    // Add rank to each entry
    const leaderboard = result.rows.map((user, index) => ({
      ...user,
      rank: index + 1,
    }));

    res.json(leaderboard);
  } catch (error) {
    console.error('Error fetching leaderboard:', error);
    res.status(500).json({ error: 'Failed to fetch leaderboard' });
  }
});

// Get activity feed (authenticated)
router.get('/activity', authMiddleware, async (req: any, res: Response) => {
  try {
    const user_id = req.user.userId;
    const limit = parseInt(req.query.limit as string) || 50;

    // Get activities from friends
    const result = await pool.query(
      `SELECT a.*, u.username, u.avatar
       FROM activities a
       JOIN users u ON a.user_id = u.id
       WHERE a.user_id IN (
         SELECT CASE 
           WHEN user_id = $1 THEN friend_id 
           ELSE user_id 
         END
         FROM friendships 
         WHERE (user_id = $1 OR friend_id = $1) AND status = 'accepted'
       ) OR a.user_id = $1
       ORDER BY a.created_at DESC
       LIMIT $2`,
      [user_id, limit]
    );

    res.json(result.rows);
  } catch (error) {
    console.error('Error fetching activity feed:', error);
    res.status(500).json({ error: 'Failed to fetch activity feed' });
  }
});

export default router;
