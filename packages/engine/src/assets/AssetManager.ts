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
 * AssetManager - Central hub for loading and managing assets
 */

import { AssetCache } from './AssetCache';
import { AssetRegistry } from './AssetRegistry';
import { AssetLoader, AssetType } from './AssetLoader';

export interface AssetLoadOptions {
  cache?: boolean;
  priority?: number;
}

export class AssetManager {
  private _cache: AssetCache;
  private _registry: AssetRegistry;
  private _loaders: Map<AssetType, AssetLoader<any>>;
  private _loadingPromises: Map<string, Promise<any>>;

  constructor() {
    this._cache = new AssetCache();
    this._registry = new AssetRegistry();
    this._loaders = new Map();
    this._loadingPromises = new Map();
  }

  registerLoader<T>(type: AssetType, loader: AssetLoader<T>): void {
    this._loaders.set(type, loader);
  }

  async load<T>(
    path: string,
    type: AssetType,
    options: AssetLoadOptions = {}
  ): Promise<T> {
    const { cache = true } = options;

    if (cache) {
      const cached = this._cache.get<T>(path);
      if (cached) {
        this._registry.incrementReferences(path);
        return cached;
      }
    }

    if (this._loadingPromises.has(path)) {
      return this._loadingPromises.get(path) as Promise<T>;
    }

    const loader = this._loaders.get(type);
    if (!loader) {
      throw new Error(`No loader registered for asset type: ${type}`);
    }

    const loadPromise = this._loadAsset<T>(path, type, loader, cache);
    this._loadingPromises.set(path, loadPromise);

    try {
      const asset = await loadPromise;
      return asset;
    } finally {
      this._loadingPromises.delete(path);
    }
  }

  private async _loadAsset<T>(
    path: string,
    type: AssetType,
    loader: AssetLoader<T>,
    cache: boolean
  ): Promise<T> {
    try {
      const asset = await loader.load(path);
      if (cache) {
        this._cache.set(path, asset);
        this._registry.register(path, type);
        this._registry.incrementReferences(path);
      }
      return asset;
    } catch (error) {
      throw new Error(`Failed to load asset '${path}': ${error}`);
    }
  }

  get<T>(path: string): T | null {
    return this._cache.get<T>(path);
  }

  has(path: string): boolean {
    return this._cache.has(path);
  }

  unload(path: string): boolean {
    const refCount = this._registry.decrementReferences(path);
    if (refCount <= 0) {
      this._cache.remove(path);
      this._registry.unregister(path);
      return true;
    }
    return false;
  }

  forceUnload(path: string): void {
    this._cache.remove(path);
    this._registry.unregister(path);
  }

  async preload(assets: Array<[string, AssetType]>): Promise<void> {
    const promises = assets.map(([path, type]) => this.load(path, type));
    await Promise.all(promises);
  }

  async preloadPaths(paths: string[], type: AssetType): Promise<void> {
    const promises = paths.map((path) => this.load(path, type));
    await Promise.all(promises);
  }

  clear(): void {
    this._cache.clear();
    this._registry.clear();
    this._loadingPromises.clear();
  }

  getCacheStats() {
    return {
      cached: this._cache.size,
      loading: this._loadingPromises.size,
      registered: this._registry.getAssetCount(),
    };
  }

  getLoadedAssets(): string[] {
    return this._registry.getAllAssets();
  }

  getRefCount(path: string): number {
    return this._registry.getReferences(path);
  }

  get registry(): AssetRegistry {
    return this._registry;
  }

  get cache(): AssetCache {
    return this._cache;
  }
}
