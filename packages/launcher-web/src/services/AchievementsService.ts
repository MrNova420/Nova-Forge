/**
 * Achievements Service - Complete achievement system
 * Phase 8.2: Social Features - Achievements
 */

export interface Achievement {
  id: string;
  gameId: string;
  name: string;
  description: string;
  icon: string;
  rarity: 'common' | 'uncommon' | 'rare' | 'epic' | 'legendary';
  points: number;
  requirements: {
    type: 'progress' | 'count' | 'condition' | 'time' | 'score';
    target: number;
    current?: number;
  };
  hidden: boolean;
  unlockedAt?: Date;
  unlockedBy?: number; // percentage of players who unlocked it
}

export interface PlayerAchievement {
  achievement: Achievement;
  progress: number; // 0-100
  unlocked: boolean;
  unlockedAt?: Date;
}

export interface AchievementNotification {
  achievement: Achievement;
  timestamp: Date;
}

class AchievementsService {
  private achievements: Map<string, Achievement> = new Map();
  private playerAchievements: Map<string, PlayerAchievement> = new Map();
  private notifications: AchievementNotification[] = [];
  private onUnlockCallbacks: ((achievement: Achievement) => void)[] = [];

  /**
   * Get all achievements for a game
   */
  async getGameAchievements(gameId: string): Promise<Achievement[]> {
    try {
      const response = await fetch(`/api/games/${gameId}/achievements`, {
        headers: { Authorization: `Bearer ${this.getToken()}` },
      });

      if (!response.ok) throw new Error('Failed to fetch achievements');

      const achievements: Achievement[] = await response.json();
      achievements.forEach((ach) => {
        this.achievements.set(ach.id, ach);
      });

      return achievements;
    } catch (error) {
      console.error('Error fetching achievements:', error);
      return [];
    }
  }

  /**
   * Get player's achievement progress
   */
  async getPlayerAchievements(gameId: string): Promise<PlayerAchievement[]> {
    try {
      const response = await fetch(
        `/api/games/${gameId}/achievements/progress`,
        {
          headers: { Authorization: `Bearer ${this.getToken()}` },
        }
      );

      if (!response.ok) throw new Error('Failed to fetch progress');

      const progress: PlayerAchievement[] = await response.json();
      progress.forEach((pa) => {
        this.playerAchievements.set(pa.achievement.id, pa);
      });

      return progress;
    } catch (error) {
      console.error('Error fetching player achievements:', error);
      return [];
    }
  }

  /**
   * Update achievement progress
   */
  async updateProgress(
    achievementId: string,
    progress: number,
    metadata?: any
  ): Promise<PlayerAchievement | null> {
    try {
      const response = await fetch(
        `/api/achievements/${achievementId}/progress`,
        {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
            Authorization: `Bearer ${this.getToken()}`,
          },
          body: JSON.stringify({ progress, metadata }),
        }
      );

      if (!response.ok) throw new Error('Failed to update progress');

      const playerAchievement: PlayerAchievement = await response.json();
      this.playerAchievements.set(achievementId, playerAchievement);

      // Check if unlocked
      if (
        playerAchievement.unlocked &&
        !playerAchievement.achievement.unlockedAt
      ) {
        this.handleUnlock(playerAchievement.achievement);
      }

      return playerAchievement;
    } catch (error) {
      console.error('Error updating achievement progress:', error);
      return null;
    }
  }

  /**
   * Unlock achievement
   */
  async unlockAchievement(achievementId: string): Promise<void> {
    try {
      const response = await fetch(
        `/api/achievements/${achievementId}/unlock`,
        {
          method: 'POST',
          headers: { Authorization: `Bearer ${this.getToken()}` },
        }
      );

      if (!response.ok) throw new Error('Failed to unlock achievement');

      const achievement: Achievement = await response.json();
      achievement.unlockedAt = new Date();

      this.achievements.set(achievementId, achievement);

      const playerAch = this.playerAchievements.get(achievementId);
      if (playerAch) {
        playerAch.unlocked = true;
        playerAch.unlockedAt = achievement.unlockedAt;
        playerAch.progress = 100;
      }

      this.handleUnlock(achievement);
    } catch (error) {
      console.error('Error unlocking achievement:', error);
      throw error;
    }
  }

  /**
   * Handle achievement unlock
   */
  private handleUnlock(achievement: Achievement): void {
    // Add notification
    this.notifications.unshift({
      achievement,
      timestamp: new Date(),
    });

    // Limit notifications
    if (this.notifications.length > 10) {
      this.notifications.pop();
    }

    // Call callbacks
    this.onUnlockCallbacks.forEach((callback) => callback(achievement));

    // Show toast notification
    this.showUnlockNotification(achievement);

    // eslint-disable-next-line no-console
    console.log(`🏆 Achievement Unlocked: ${achievement.name}`);
  }

  /**
   * Show achievement unlock notification
   */
  private showUnlockNotification(achievement: Achievement): void {
    // Create notification element
    const notification = document.createElement('div');
    notification.className = 'achievement-notification';
    notification.style.cssText = `
      position: fixed;
      top: 20px;
      right: 20px;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      padding: 20px;
      border-radius: 12px;
      box-shadow: 0 10px 40px rgba(0,0,0,0.3);
      z-index: 10000;
      min-width: 300px;
      animation: slideIn 0.5s ease-out;
    `;

    notification.innerHTML = `
      <div style="display: flex; align-items: center; gap: 15px;">
        <div style="font-size: 48px;">🏆</div>
        <div style="flex: 1;">
          <div style="font-weight: bold; font-size: 16px; margin-bottom: 5px;">Achievement Unlocked!</div>
          <div style="font-size: 14px; opacity: 0.9;">${achievement.name}</div>
          <div style="font-size: 12px; opacity: 0.7; margin-top: 5px;">${achievement.points} points</div>
        </div>
      </div>
    `;

    document.body.appendChild(notification);

    // Remove after 5 seconds
    setTimeout(() => {
      notification.style.animation = 'slideOut 0.5s ease-in';
      setTimeout(() => notification.remove(), 500);
    }, 5000);
  }

  /**
   * Get recent unlocked achievements
   */
  getRecentUnlocks(limit: number = 5): AchievementNotification[] {
    return this.notifications.slice(0, limit);
  }

  /**
   * Get achievement statistics
   */
  async getAchievementStats(gameId: string): Promise<{
    total: number;
    unlocked: number;
    progress: number;
    points: number;
    totalPoints: number;
    rarity: Record<string, number>;
  }> {
    try {
      const response = await fetch(`/api/games/${gameId}/achievements/stats`, {
        headers: { Authorization: `Bearer ${this.getToken()}` },
      });

      if (!response.ok) throw new Error('Failed to fetch stats');

      return await response.json();
    } catch (error) {
      console.error('Error fetching achievement stats:', error);

      // Calculate from local data
      const achievements = Array.from(this.playerAchievements.values());
      const unlocked = achievements.filter((a) => a.unlocked).length;

      return {
        total: achievements.length,
        unlocked,
        progress:
          achievements.length > 0 ? (unlocked / achievements.length) * 100 : 0,
        points: achievements
          .filter((a) => a.unlocked)
          .reduce((sum, a) => sum + a.achievement.points, 0),
        totalPoints: achievements.reduce(
          (sum, a) => sum + a.achievement.points,
          0
        ),
        rarity: {
          common: achievements.filter((a) => a.achievement.rarity === 'common')
            .length,
          uncommon: achievements.filter(
            (a) => a.achievement.rarity === 'uncommon'
          ).length,
          rare: achievements.filter((a) => a.achievement.rarity === 'rare')
            .length,
          epic: achievements.filter((a) => a.achievement.rarity === 'epic')
            .length,
          legendary: achievements.filter(
            (a) => a.achievement.rarity === 'legendary'
          ).length,
        },
      };
    }
  }

  /**
   * Get leaderboard for achievement points
   */
  async getAchievementLeaderboard(
    gameId: string,
    limit: number = 999999
  ): Promise<
    {
      rank: number;
      username: string;
      points: number;
      achievementsUnlocked: number;
    }[]
  > {
    try {
      const response = await fetch(
        `/api/games/${gameId}/achievements/leaderboard?limit=${limit}`,
        {
          headers: { Authorization: `Bearer ${this.getToken()}` },
        }
      );

      if (!response.ok) throw new Error('Failed to fetch leaderboard');

      return await response.json();
    } catch (error) {
      console.error('Error fetching leaderboard:', error);
      return [];
    }
  }

  /**
   * Subscribe to achievement unlocks
   */
  onUnlock(callback: (achievement: Achievement) => void): () => void {
    this.onUnlockCallbacks.push(callback);

    // Return unsubscribe function
    return () => {
      const index = this.onUnlockCallbacks.indexOf(callback);
      if (index > -1) {
        this.onUnlockCallbacks.splice(index, 1);
      }
    };
  }

  /**
   * Get rarity color
   */
  getRarityColor(rarity: Achievement['rarity']): string {
    const colors = {
      common: '#95a5a6',
      uncommon: '#3498db',
      rare: '#9b59b6',
      epic: '#e74c3c',
      legendary: '#f39c12',
    };
    return colors[rarity];
  }

  /**
   * Get rarity icon
   */
  getRarityIcon(rarity: Achievement['rarity']): string {
    const icons = {
      common: '⚪',
      uncommon: '🔵',
      rare: '🟣',
      epic: '🔴',
      legendary: '🟡',
    };
    return icons[rarity];
  }

  /**
   * Calculate completion percentage
   */
  getCompletionPercentage(): number {
    const achievements = Array.from(this.playerAchievements.values());
    if (achievements.length === 0) return 0;

    const totalProgress = achievements.reduce(
      (sum, pa) => sum + pa.progress,
      0
    );
    return totalProgress / achievements.length;
  }

  /**
   * Get achievements by rarity
   */
  getAchievementsByRarity(rarity: Achievement['rarity']): PlayerAchievement[] {
    return Array.from(this.playerAchievements.values()).filter(
      (pa) => pa.achievement.rarity === rarity
    );
  }

  /**
   * Check if achievement should be unlocked based on progress
   */
  checkAutoUnlock(achievementId: string): Promise<void> {
    const pa = this.playerAchievements.get(achievementId);
    if (pa && pa.progress >= 100 && !pa.unlocked) {
      return this.unlockAchievement(achievementId);
    }
    return Promise.resolve();
  }

  /**
   * Get auth token
   */
  private getToken(): string {
    return localStorage.getItem('auth_token') || '';
  }
}

// Add CSS animation
const style = document.createElement('style');
style.textContent = `
  @keyframes slideIn {
    from {
      transform: translateX(400px);
      opacity: 0;
    }
    to {
      transform: translateX(0);
      opacity: 1;
    }
  }

  @keyframes slideOut {
    from {
      transform: translateX(0);
      opacity: 1;
    }
    to {
      transform: translateX(400px);
      opacity: 0;
    }
  }
`;
document.head.appendChild(style);

export const achievementsService = new AchievementsService();
