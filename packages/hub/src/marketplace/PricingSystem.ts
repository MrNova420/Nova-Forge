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
 * Pricing System - Game monetization and pricing management
 * Supports free, paid, and premium game models
 */

export enum GamePriceModel {
  FREE = 'free',
  PAID = 'paid',
  FREE_TO_PLAY = 'free_to_play', // Free with IAP
  SUBSCRIPTION = 'subscription',
  DONATION = 'donation', // Pay what you want
}

export enum Currency {
  USD = 'USD',
  EUR = 'EUR',
  GBP = 'GBP',
  JPY = 'JPY',
  CAD = 'CAD',
  AUD = 'AUD',
}

export interface GamePrice {
  gameId: string;
  priceModel: GamePriceModel;
  basePrice: number; // in cents
  currency: Currency;
  discountPercent: number;
  discountEndDate?: Date;
  minPrice?: number; // For donation model
  iapEnabled: boolean;
  subscriptionTierId?: string;
}

export interface PriceHistory {
  gameId: string;
  oldPrice: number;
  newPrice: number;
  changedAt: Date;
  reason: string;
}

export interface RegionalPricing {
  gameId: string;
  countryCode: string;
  price: number;
  currency: Currency;
  autoConverted: boolean;
}

/**
 * Manages game pricing, discounts, and monetization models
 */
export class PricingSystem {
  private prices: Map<string, GamePrice> = new Map();
  private priceHistory: Map<string, PriceHistory[]> = new Map();
  private regionalPrices: Map<string, Map<string, RegionalPricing>> = new Map();

  // Currency conversion rates (relative to USD)
  private conversionRates: Map<Currency, number> = new Map([
    [Currency.USD, 1.0],
    [Currency.EUR, 0.92],
    [Currency.GBP, 0.79],
    [Currency.JPY, 149.5],
    [Currency.CAD, 1.36],
    [Currency.AUD, 1.53],
  ]);

  /**
   * Set price for a game
   */
  setPrice(gameId: string, price: Omit<GamePrice, 'gameId'>): void {
    const oldPrice = this.prices.get(gameId);

    const newPrice: GamePrice = {
      gameId,
      ...price,
    };

    this.prices.set(gameId, newPrice);

    // Track price history
    if (oldPrice && oldPrice.basePrice !== newPrice.basePrice) {
      this.addPriceHistory(
        gameId,
        oldPrice.basePrice,
        newPrice.basePrice,
        'Manual price change'
      );
    }
  }

  /**
   * Get price for a game
   */
  getPrice(
    gameId: string,
    countryCode?: string
  ): GamePrice | RegionalPricing | null {
    // Check for regional pricing first
    if (countryCode) {
      const regional = this.getRegionalPrice(gameId, countryCode);
      if (regional) return regional;
    }

    return this.prices.get(gameId) || null;
  }

  /**
   * Get effective price after discount
   */
  getEffectivePrice(gameId: string): number {
    const price = this.prices.get(gameId);
    if (!price) return 0;

    if (
      price.priceModel === GamePriceModel.FREE ||
      price.priceModel === GamePriceModel.FREE_TO_PLAY
    ) {
      return 0;
    }

    // Check if discount is still valid
    if (
      price.discountPercent > 0 &&
      (!price.discountEndDate || price.discountEndDate > new Date())
    ) {
      return Math.floor(price.basePrice * (1 - price.discountPercent / 100));
    }

    return price.basePrice;
  }

  /**
   * Apply discount to a game
   */
  applyDiscount(
    gameId: string,
    discountPercent: number,
    durationDays?: number
  ): void {
    const price = this.prices.get(gameId);
    if (!price) return;

    price.discountPercent = Math.max(0, Math.min(100, discountPercent));

    if (durationDays) {
      const endDate = new Date();
      endDate.setDate(endDate.getDate() + durationDays);
      price.discountEndDate = endDate;
    } else {
      price.discountEndDate = undefined;
    }

    this.prices.set(gameId, price);
  }

  /**
   * Remove discount from a game
   */
  removeDiscount(gameId: string): void {
    const price = this.prices.get(gameId);
    if (!price) return;

    price.discountPercent = 0;
    price.discountEndDate = undefined;
    this.prices.set(gameId, price);
  }

  /**
   * Set regional pricing for specific countries
   */
  setRegionalPrice(regionalPrice: RegionalPricing): void {
    let countryPrices = this.regionalPrices.get(regionalPrice.gameId);
    if (!countryPrices) {
      countryPrices = new Map();
      this.regionalPrices.set(regionalPrice.gameId, countryPrices);
    }

    countryPrices.set(regionalPrice.countryCode, regionalPrice);
  }

  /**
   * Get regional price for a specific country
   */
  getRegionalPrice(
    gameId: string,
    countryCode: string
  ): RegionalPricing | null {
    const countryPrices = this.regionalPrices.get(gameId);
    return countryPrices?.get(countryCode) || null;
  }

  /**
   * Auto-generate regional prices based on base price
   */
  generateRegionalPrices(gameId: string, baseCurrency: Currency): void {
    const basePrice = this.prices.get(gameId);
    if (!basePrice) return;

    const baseRate = this.conversionRates.get(baseCurrency) || 1.0;
    const basePriceUSD = basePrice.basePrice / baseRate;

    // Generate prices for all supported currencies
    this.conversionRates.forEach((rate, currency) => {
      if (currency === baseCurrency) return;

      const convertedPrice = Math.round(basePriceUSD * rate);

      this.setRegionalPrice({
        gameId,
        countryCode: currency, // Using currency as country code for simplicity
        price: convertedPrice,
        currency,
        autoConverted: true,
      });
    });
  }

  /**
   * Add entry to price history
   */
  private addPriceHistory(
    gameId: string,
    oldPrice: number,
    newPrice: number,
    reason: string
  ): void {
    let history = this.priceHistory.get(gameId);
    if (!history) {
      history = [];
      this.priceHistory.set(gameId, history);
    }

    history.push({
      gameId,
      oldPrice,
      newPrice,
      changedAt: new Date(),
      reason,
    });
  }

  /**
   * Get price history for a game
   */
  getPriceHistory(gameId: string): PriceHistory[] {
    return this.priceHistory.get(gameId) || [];
  }

  /**
   * Check if game is on sale
   */
  isOnSale(gameId: string): boolean {
    const price = this.prices.get(gameId);
    if (!price) return false;

    if (price.discountPercent === 0) return false;
    if (!price.discountEndDate) return true;

    return price.discountEndDate > new Date();
  }

  /**
   * Get all games on sale
   */
  getGamesOnSale(): GamePrice[] {
    const onSale: GamePrice[] = [];

    this.prices.forEach((price) => {
      if (this.isOnSale(price.gameId)) {
        onSale.push(price);
      }
    });

    return onSale.sort((a, b) => b.discountPercent - a.discountPercent);
  }

  /**
   * Convert price between currencies
   */
  convertPrice(
    amount: number,
    fromCurrency: Currency,
    toCurrency: Currency
  ): number {
    const fromRate = this.conversionRates.get(fromCurrency) || 1.0;
    const toRate = this.conversionRates.get(toCurrency) || 1.0;

    const usdAmount = amount / fromRate;
    return Math.round(usdAmount * toRate);
  }

  /**
   * Format price for display
   */
  formatPrice(amount: number, currency: Currency): string {
    const symbols: Record<Currency, string> = {
      [Currency.USD]: '$',
      [Currency.EUR]: '€',
      [Currency.GBP]: '£',
      [Currency.JPY]: '¥',
      [Currency.CAD]: 'CA$',
      [Currency.AUD]: 'A$',
    };

    const symbol = symbols[currency];

    // JPY doesn't use decimal places
    if (currency === Currency.JPY) {
      return `${symbol}${amount}`;
    }

    const dollars = Math.floor(amount / 100);
    const cents = amount % 100;
    return `${symbol}${dollars}.${cents.toString().padStart(2, '0')}`;
  }

  /**
   * Clear all pricing data
   */
  clear(): void {
    this.prices.clear();
    this.priceHistory.clear();
    this.regionalPrices.clear();
  }
}
