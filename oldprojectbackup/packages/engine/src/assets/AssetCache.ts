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
 * AssetCache - Caching system for loaded assets
 */

export class AssetCache {
  private _cache: Map<string, any>;
  private _maxSize: number;

  constructor(maxSize: number = 100000) {
    // Increased from 1000 to 100000
    this._cache = new Map();
    this._maxSize = maxSize;
  }

  set<T>(key: string, value: T): void {
    if (this._cache.size >= this._maxSize && !this._cache.has(key)) {
      const firstKey = this._cache.keys().next().value;
      if (firstKey) {
        this._cache.delete(firstKey);
      }
    }
    this._cache.delete(key);
    this._cache.set(key, value);
  }

  get<T>(key: string): T | null {
    const value = this._cache.get(key);
    if (value !== undefined) {
      this._cache.delete(key);
      this._cache.set(key, value);
      return value as T;
    }
    return null;
  }

  has(key: string): boolean {
    return this._cache.has(key);
  }

  remove(key: string): boolean {
    return this._cache.delete(key);
  }

  clear(): void {
    this._cache.clear();
  }

  get size(): number {
    return this._cache.size;
  }

  keys(): string[] {
    return Array.from(this._cache.keys());
  }

  get maxSize(): number {
    return this._maxSize;
  }

  set maxSize(value: number) {
    this._maxSize = value;
    while (this._cache.size > this._maxSize) {
      const firstKey = this._cache.keys().next().value;
      if (firstKey) {
        this._cache.delete(firstKey);
      }
    }
  }
}
