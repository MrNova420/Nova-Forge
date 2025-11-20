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
 * CommunityService - User-generated content and community features
 *
 * Features:
 * - Forums and discussions
 * - User reviews and ratings
 * - Mod/workshop support
 * - Achievement system
 * - Leaderboards
 * - Social features (friends, follows)
 */

export interface GameReview {
  id: string;
  gameId: string;
  userId: string;
  username: string;
  rating: number; // 1-5
  title: string;
  content: string;
  helpful: number;
  createdAt: Date;
}

export interface Mod {
  id: string;
  gameId: string;
  creatorId: string;
  name: string;
  description: string;
  version: string;
  downloads: number;
  rating: number;
  files: string[];
  createdAt: Date;
}

export interface Achievement {
  id: string;
  gameId: string;
  name: string;
  description: string;
  icon: string;
  points: number;
  rarity: 'common' | 'uncommon' | 'rare' | 'legendary';
}

export interface LeaderboardEntry {
  rank: number;
  userId: string;
  username: string;
  score: number;
  platform: 'web' | 'desktop' | 'mobile';
  country?: string;
}

class CommunityService {
  private reviews: Map<string, GameReview[]> = new Map();
  private mods: Map<string, Mod[]> = new Map();
  private achievements: Map<string, Achievement[]> = new Map();
  private leaderboards: Map<string, LeaderboardEntry[]> = new Map();

  /**
   * Submit game review
   */
  async submitReview(
    review: Omit<GameReview, 'id' | 'helpful' | 'createdAt'>
  ): Promise<GameReview> {
    const fullReview: GameReview = {
      ...review,
      id: `review-${Date.now()}`,
      helpful: 0,
      createdAt: new Date(),
    };

    const gameReviews = this.reviews.get(review.gameId) || [];
    gameReviews.push(fullReview);
    this.reviews.set(review.gameId, gameReviews);

    return fullReview;
  }

  /**
   * Get reviews for a game
   */
  getReviews(gameId: string, limit: number = 999999): GameReview[] {
    const reviews = this.reviews.get(gameId) || [];
    return reviews.sort((a, b) => b.helpful - a.helpful).slice(0, limit);
  }

  /**
   * Calculate average rating
   */
  getAverageRating(gameId: string): number {
    const reviews = this.reviews.get(gameId) || [];
    if (reviews.length === 0) return 0;

    const total = reviews.reduce((sum, r) => sum + r.rating, 0);
    return total / reviews.length;
  }

  /**
   * Upload mod
   */
  async uploadMod(
    mod: Omit<Mod, 'id' | 'downloads' | 'rating' | 'createdAt'>
  ): Promise<Mod> {
    const fullMod: Mod = {
      ...mod,
      id: `mod-${Date.now()}`,
      downloads: 0,
      rating: 0,
      createdAt: new Date(),
    };

    const gameMods = this.mods.get(mod.gameId) || [];
    gameMods.push(fullMod);
    this.mods.set(mod.gameId, gameMods);

    return fullMod;
  }

  /**
   * Get mods for a game
   */
  getMods(
    gameId: string,
    sortBy: 'popular' | 'recent' | 'rating' = 'popular'
  ): Mod[] {
    const mods = this.mods.get(gameId) || [];

    switch (sortBy) {
      case 'popular':
        return mods.sort((a, b) => b.downloads - a.downloads);
      case 'recent':
        return mods.sort(
          (a, b) => b.createdAt.getTime() - a.createdAt.getTime()
        );
      case 'rating':
        return mods.sort((a, b) => b.rating - a.rating);
      default:
        return mods;
    }
  }

  /**
   * Create achievement
   */
  createAchievement(
    gameId: string,
    achievement: Omit<Achievement, 'id'>
  ): Achievement {
    const fullAchievement: Achievement = {
      ...achievement,
      id: `achievement-${Date.now()}`,
    };

    const gameAchievements = this.achievements.get(gameId) || [];
    gameAchievements.push(fullAchievement);
    this.achievements.set(gameId, gameAchievements);

    return fullAchievement;
  }

  /**
   * Get achievements for a game
   */
  getAchievements(gameId: string): Achievement[] {
    return this.achievements.get(gameId) || [];
  }

  /**
   * Update leaderboard
   */
  updateLeaderboard(
    leaderboardId: string,
    userId: string,
    username: string,
    score: number,
    platform: 'web' | 'desktop' | 'mobile'
  ): void {
    const leaderboard = this.leaderboards.get(leaderboardId) || [];

    // Remove existing entry for user
    const filtered = leaderboard.filter((e) => e.userId !== userId);

    // Add new entry
    filtered.push({
      rank: 0, // Will be calculated
      userId,
      username,
      score,
      platform,
    });

    // Sort and assign ranks
    filtered.sort((a, b) => b.score - a.score);
    filtered.forEach((entry, index) => {
      entry.rank = index + 1;
    });

    this.leaderboards.set(leaderboardId, filtered.slice(0, 100)); // Keep top 100
  }

  /**
   * Get leaderboard
   */
  getLeaderboard(
    leaderboardId: string,
    limit: number = 999999
  ): LeaderboardEntry[] {
    const leaderboard = this.leaderboards.get(leaderboardId) || [];
    return leaderboard.slice(0, limit);
  }

  /**
   * Get user rank
   */
  getUserRank(leaderboardId: string, userId: string): number {
    const leaderboard = this.leaderboards.get(leaderboardId) || [];
    const entry = leaderboard.find((e) => e.userId === userId);
    return entry ? entry.rank : -1;
  }
}

export const community = new CommunityService();
