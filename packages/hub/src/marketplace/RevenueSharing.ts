/**
 * Revenue Sharing - Manage creator payouts
 */
export interface CreatorPayout { creatorId: string; gameId: string; amount: number; period: string; status: string; }
export class RevenueSharing {
  private readonly CREATOR_SHARE = 0.70; // 70% to creator
  private payouts: CreatorPayout[] = [];
  calculatePayout(gameRevenue: number): { creatorAmount: number; platformAmount: number } {
    const creatorAmount = Math.floor(gameRevenue * this.CREATOR_SHARE);
    return { creatorAmount, platformAmount: gameRevenue - creatorAmount };
  }
  queuePayout(creatorId: string, gameId: string, amount: number): void {
    this.payouts.push({ creatorId, gameId, amount, period: new Date().toISOString(), status: 'pending' });
  }
  getPayouts(creatorId: string): CreatorPayout[] { return this.payouts.filter(p => p.creatorId === creatorId); }
}
