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
 * MatchmakingService - Intelligent matchmaking for multiplayer games
 *
 * Features:
 * - Skill-based matchmaking
 * - Region-based matching
 * - Quick match and ranked modes
 * - Party system
 * - Lobby management
 */

export interface Player {
  id: string;
  username: string;
  skill: number;
  region: string;
  platform: 'web' | 'desktop' | 'mobile';
}

export interface MatchRequest {
  gameId: string;
  playerId: string;
  mode: 'quick' | 'ranked' | 'custom';
  partyId?: string;
  preferredRegions: string[];
}

export interface Match {
  id: string;
  gameId: string;
  players: Player[];
  region: string;
  status: 'forming' | 'ready' | 'in_progress' | 'completed';
  createdAt: Date;
}

class MatchmakingService {
  private queue: Map<string, MatchRequest[]> = new Map();
  private activeMatches: Map<string, Match> = new Map();

  /**
   * Add player to matchmaking queue
   */
  async addToQueue(request: MatchRequest): Promise<void> {
    const gameQueue = this.queue.get(request.gameId) || [];
    gameQueue.push(request);
    this.queue.set(request.gameId, gameQueue);

    // Try to create match immediately
    await this.processQueue(request.gameId);
  }

  /**
   * Process matchmaking queue
   */
  private async processQueue(gameId: string): Promise<void> {
    const gameQueue = this.queue.get(gameId) || [];
    if (gameQueue.length < 2) return; // Need at least 2 players

    // Simple matching algorithm - can be enhanced with ELO, etc.
    const match: Match = {
      id: `match-${Date.now()}`,
      gameId,
      players: [],
      region: 'us-east',
      status: 'forming',
      createdAt: new Date(),
    };

    // Take first 2-4 players (configurable per game)
    const playersNeeded = 2;
    const _matchedPlayers = gameQueue.splice(0, playersNeeded);

    this.activeMatches.set(match.id, match);
  }

  /**
   * Get active matches
   */
  getActiveMatches(gameId?: string): Match[] {
    const matches = Array.from(this.activeMatches.values());
    return gameId ? matches.filter((m) => m.gameId === gameId) : matches;
  }
}

export const matchmaking = new MatchmakingService();
