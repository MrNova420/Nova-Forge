/**
 * @fileoverview Async Load Job
 * @module @nova-engine/jobs
 *
 * Job for asynchronous asset loading.
 * Loads resources from URLs or file paths.
 *
 * Following the autonomous development guide Phase 1.2.3
 */

import type { JobConfig, JobContext } from './IJob';
import { Job } from './Job';

/**
 * Load result with metadata
 */
export interface LoadResult<T = unknown> {
  /** Loaded data */
  data: T;
  /** URL or path that was loaded */
  source: string;
  /** Size in bytes */
  size: number;
  /** Load time in milliseconds */
  loadTime: number;
}

/**
 * Async Load Job
 *
 * Asynchronously loads data from a URL or file path.
 * Supports progress tracking and cancellation.
 *
 * @example
 * ```typescript
 * const job = new AsyncLoadJob({
 *   name: 'Load Texture',
 *   url: '/assets/textures/wall.png',
 *   loader: async (url) => await fetch(url).then(r => r.blob()),
 * });
 * ```
 */
export class AsyncLoadJob<T = unknown> extends Job<LoadResult<T>> {
  private url: string;
  private loader: (url: string) => Promise<T>;
  private onProgressCallback?: (loaded: number, total: number) => void;

  constructor(
    config: JobConfig & {
      url: string;
      loader: (url: string) => Promise<T>;
      onProgress?: (loaded: number, total: number) => void;
    }
  ) {
    super(config);
    this.url = config.url;
    this.loader = config.loader;
    this.onProgressCallback = config.onProgress;
  }

  protected async run(_context: JobContext): Promise<LoadResult<T>> {
    const startTime = Date.now();

    try {
      // Simulate progress tracking
      if (this.onProgressCallback) {
        this.onProgressCallback(0, 100);
      }

      const data = await this.loader(this.url);

      if (this.onProgressCallback) {
        this.onProgressCallback(100, 100);
      }

      const loadTime = Date.now() - startTime;

      // Estimate size (would be more accurate with real fetch)
      const size = this.estimateSize(data);

      return {
        data,
        source: this.url,
        size,
        loadTime,
      };
    } catch (error) {
      throw new Error(`Failed to load ${this.url}: ${(error as Error).message}`);
    }
  }

  private estimateSize(data: unknown): number {
    if (data instanceof ArrayBuffer) {
      return data.byteLength;
    }
    if (data instanceof Blob) {
      return data.size;
    }
    if (typeof data === 'string') {
      return data.length * 2; // UTF-16
    }
    // Rough estimate for objects
    return JSON.stringify(data).length * 2;
  }

  /**
   * Set progress callback
   */
  onProgress(callback: (loaded: number, total: number) => void): void {
    this.onProgressCallback = callback;
  }
}
