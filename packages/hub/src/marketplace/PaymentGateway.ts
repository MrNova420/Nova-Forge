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
 * Payment Gateway - Process payments for game purchases
 */
export enum PaymentProvider { STRIPE = 'stripe', PAYPAL = 'paypal', CREDIT_CARD = 'credit_card', WALLET = 'wallet' }
export enum PaymentStatus { PENDING = 'pending', PROCESSING = 'processing', COMPLETED = 'completed', FAILED = 'failed', REFUNDED = 'refunded', CANCELLED = 'cancelled' }
export interface PaymentTransaction { id: string; userId: string; gameId: string; amount: number; status: PaymentStatus; createdAt: Date; }
export class PaymentGateway {
  private transactions: Map<string, PaymentTransaction> = new Map();
  async processPayment(userId: string, gameId: string, amount: number): Promise<PaymentTransaction> {
    const tx: PaymentTransaction = { id: Date.now().toString(), userId, gameId, amount, status: PaymentStatus.COMPLETED, createdAt: new Date() };
    this.transactions.set(tx.id, tx);
    return tx;
  }
  getTransaction(id: string): PaymentTransaction | null { return this.transactions.get(id) || null; }
  getUserTransactions(userId: string): PaymentTransaction[] {
    return Array.from(this.transactions.values()).filter(tx => tx.userId === userId);
  }
}
