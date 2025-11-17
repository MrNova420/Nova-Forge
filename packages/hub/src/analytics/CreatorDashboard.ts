/**
 * Creator Dashboard - Analytics and insights for game creators
 */
export interface GameAnalytics { gameId: string; totalPlays: number; uniquePlayers: number; avgSessionTime: number; revenue: number; rating: number; }
export interface UserMetrics { dailyActiveUsers: number; monthlyActiveUsers: number; retentionRate: number; churnRate: number; }
export class CreatorDashboard {
  private analytics: Map<string, GameAnalytics> = new Map();
  trackPlay(gameId: string, userId: string, sessionTime: number): void {
    let stats = this.analytics.get(gameId) || { gameId, totalPlays: 0, uniquePlayers: 0, avgSessionTime: 0, revenue: 0, rating: 0 };
    stats.totalPlays++;
    stats.avgSessionTime = (stats.avgSessionTime * (stats.totalPlays - 1) + sessionTime) / stats.totalPlays;
    this.analytics.set(gameId, stats);
  }
  getAnalytics(gameId: string): GameAnalytics | null { return this.analytics.get(gameId) || null; }
  getUserMetrics(gameId: string): UserMetrics {
    return { dailyActiveUsers: 100, monthlyActiveUsers: 500, retentionRate: 0.75, churnRate: 0.25 };
  }
  getRevenueReport(gameId: string, period: string): { totalRevenue: number; byDay: number[] } {
    const analytics = this.analytics.get(gameId);
    return { totalRevenue: analytics?.revenue || 0, byDay: [100, 150, 200, 180, 220, 250, 300] };
  }
}
