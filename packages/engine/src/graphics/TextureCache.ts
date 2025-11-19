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
 * TextureCache
 *
 * Caches loaded textures to avoid redundant loading and improve performance.
 */

import { Texture } from './Texture';
import { TextureLoader, TextureLoadOptions } from './TextureLoader';

/**
 * Texture cache for managing loaded textures
 */
export class TextureCache {
  private _loader: TextureLoader;
  private _cache: Map<string, Texture> = new Map();
  private _loadingPromises: Map<string, Promise<Texture>> = new Map();

  /**
   * Create a new texture cache
   * @param gl - WebGL context
   */
  constructor(gl: WebGL2RenderingContext) {
    this._loader = new TextureLoader(gl);
  }

  /**
   * Get texture from cache or load if not cached
   * @param url - Texture URL
   * @param options - Loading options
   * @returns Promise that resolves to the texture
   */
  async get(url: string, options?: TextureLoadOptions): Promise<Texture> {
    // Check if already cached
    if (this._cache.has(url)) {
      return this._cache.get(url)!;
    }

    // Check if currently loading
    if (this._loadingPromises.has(url)) {
      return this._loadingPromises.get(url)!;
    }

    // Start loading
    const promise = this._loader.load(url, url, options).then((texture) => {
      this._cache.set(url, texture);
      this._loadingPromises.delete(url);
      return texture;
    });

    this._loadingPromises.set(url, promise);
    return promise;
  }

  /**
   * Load multiple textures
   * @param urls - Array of texture URLs
   * @param options - Loading options
   * @returns Promise that resolves to array of textures
   */
  async getMultiple(
    urls: string[],
    options?: TextureLoadOptions
  ): Promise<Texture[]> {
    const promises = urls.map((url) => this.get(url, options));
    return Promise.all(promises);
  }

  /**
   * Add texture to cache manually
   * @param key - Cache key
   * @param texture - Texture to cache
   */
  add(key: string, texture: Texture): void {
    this._cache.set(key, texture);
  }

  /**
   * Check if texture is cached
   * @param key - Cache key
   */
  has(key: string): boolean {
    return this._cache.has(key);
  }

  /**
   * Remove texture from cache
   * @param key - Cache key
   * @param destroy - Whether to destroy the texture (default: true)
   */
  remove(key: string, destroy: boolean = true): void {
    const texture = this._cache.get(key);
    if (texture) {
      if (destroy) {
        texture.destroy();
      }
      this._cache.delete(key);
    }
  }

  /**
   * Clear all cached textures
   * @param destroy - Whether to destroy textures (default: true)
   */
  clear(destroy: boolean = true): void {
    if (destroy) {
      for (const texture of this._cache.values()) {
        texture.destroy();
      }
    }
    this._cache.clear();
    this._loadingPromises.clear();
  }

  /**
   * Get number of cached textures
   */
  get count(): number {
    return this._cache.size;
  }

  /**
   * Get all cache keys
   */
  getKeys(): string[] {
    return Array.from(this._cache.keys());
  }

  /**
   * Get all cached textures
   */
  getAll(): Texture[] {
    return Array.from(this._cache.values());
  }

  /**
   * Get the texture loader
   */
  get loader(): TextureLoader {
    return this._loader;
  }

  /**
   * Preload textures
   * @param urls - Array of texture URLs to preload
   * @param options - Loading options
   * @returns Promise that resolves when all textures are loaded
   */
  async preload(urls: string[], options?: TextureLoadOptions): Promise<void> {
    await this.getMultiple(urls, options);
  }

  /**
   * Get cache statistics
   */
  getStats(): {
    cached: number;
    loading: number;
    totalMemoryEstimate: number;
  } {
    let totalMemory = 0;
    for (const texture of this._cache.values()) {
      // Rough estimate: width * height * 4 bytes per pixel (RGBA)
      totalMemory += texture.width * texture.height * 4;
    }

    return {
      cached: this._cache.size,
      loading: this._loadingPromises.size,
      totalMemoryEstimate: totalMemory,
    };
  }
}
