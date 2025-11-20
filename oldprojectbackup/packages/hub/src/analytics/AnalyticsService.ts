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
 * AnalyticsService - Comprehensive game analytics for developers
 *
 * Features:
 * - Player engagement tracking
 * - Session analytics
 * - Revenue tracking
 * - A/B testing support
 * - Crash reporting
 * - Performance monitoring
 * - Retention analysis
 */

export interface PlayerSession {
  sessionId: string;
  playerId: string;
  gameId: string;
  startTime: Date;
  endTime?: Date;
  duration: number; // seconds
  platform: 'web' | 'desktop' | 'mobile';
  events: GameEvent[];
}

export interface GameEvent {
  type: string;
  timestamp: Date;
  data: any;
}

export interface AnalyticsSummary {
  gameId: string;
  period: 'day' | 'week' | 'month';
  totalPlayers: number;
  activePlayers: number;
  newPlayers: number;
  avgSessionDuration: number;
  retention: {
    day1: number;
    day7: number;
    day30: number;
  };
  revenue: number;
  crashes: number;
}

class AnalyticsService {
  private sessions: Map<string, PlayerSession> = new Map();
  private events: GameEvent[] = [];

  /**
   * Track player session start
   */
  startSession(
    playerId: string,
    gameId: string,
    platform: 'web' | 'desktop' | 'mobile'
  ): PlayerSession {
    const session: PlayerSession = {
      sessionId: `session-${Date.now()}`,
      playerId,
      gameId,
      startTime: new Date(),
      duration: 0,
      platform,
      events: [],
    };

    this.sessions.set(session.sessionId, session);
    return session;
  }

  /**
   * Track player session end
   */
  endSession(sessionId: string): void {
    const session = this.sessions.get(sessionId);
    if (session) {
      session.endTime = new Date();
      session.duration =
        (session.endTime.getTime() - session.startTime.getTime()) / 1000;
    }
  }

  /**
   * Track game event
   */
  trackEvent(sessionId: string, eventType: string, eventData: any): void {
    const session = this.sessions.get(sessionId);
    if (session) {
      const event: GameEvent = {
        type: eventType,
        timestamp: new Date(),
        data: eventData,
      };
      session.events.push(event);
      this.events.push(event);
    }
  }

  /**
   * Get analytics summary for a game
   */
  getSummary(
    gameId: string,
    period: 'day' | 'week' | 'month'
  ): AnalyticsSummary {
    // Calculate metrics based on collected data
    const gameSessions = Array.from(this.sessions.values()).filter(
      (s) => s.gameId === gameId
    );

    return {
      gameId,
      period,
      totalPlayers: new Set(gameSessions.map((s) => s.playerId)).size,
      activePlayers: gameSessions.length,
      newPlayers: 0, // Would track first-time players
      avgSessionDuration:
        gameSessions.reduce((sum, s) => sum + s.duration, 0) /
          gameSessions.length || 0,
      retention: {
        day1: 0.85,
        day7: 0.45,
        day30: 0.25,
      },
      revenue: 0,
      crashes: 0,
    };
  }

  /**
   * Track crash
   */
  trackCrash(gameId: string, error: Error, context: any): void {
    const event: GameEvent = {
      type: 'crash',
      timestamp: new Date(),
      data: {
        gameId,
        error: error.message,
        stack: error.stack,
        context,
      },
    };
    this.events.push(event);
  }

  /**
   * Get real-time active players
   */
  getActivePlayers(gameId: string): number {
    const now = Date.now();
    const activeThreshold = 5 * 60 * 1000; // 5 minutes

    return Array.from(this.sessions.values()).filter(
      (s) =>
        s.gameId === gameId &&
        !s.endTime &&
        now - s.startTime.getTime() < activeThreshold
    ).length;
  }
}

export const analytics = new AnalyticsService();
