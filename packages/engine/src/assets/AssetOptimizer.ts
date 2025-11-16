/**
 * @fileoverview Asset optimization utilities for web platform
 * Provides lazy loading, progressive loading, and asset compression
 */

/**
 * Asset loading priority levels
 */
export enum LoadPriority {
  /** Critical assets needed immediately */
  Critical = 0,
  /** High priority assets */
  High = 1,
  /** Normal priority assets */
  Normal = 2,
  /** Low priority assets that can be deferred */
  Low = 3,
}

/**
 * Asset loading strategy
 */
export enum LoadStrategy {
  /** Load immediately */
  Immediate = 'immediate',
  /** Load when idle */
  Idle = 'idle',
  /** Load when visible */
  Visible = 'visible',
  /** Load on demand */
  OnDemand = 'on-demand',
}

/**
 * Asset metadata for optimization
 */
export interface AssetMetadata {
  /** Asset path */
  path: string;
  /** Asset type */
  type: string;
  /** Loading priority */
  priority: LoadPriority;
  /** Loading strategy */
  strategy: LoadStrategy;
  /** Asset size in bytes */
  size?: number;
  /** Whether asset can be compressed */
  compressible?: boolean;
}

/**
 * Asset optimizer for web platform
 * Handles lazy loading, progressive loading, and compression
 */
export class AssetOptimizer {
  /** Queue of assets to load */
  private loadQueue: AssetMetadata[] = [];
  /** Currently loading assets */
  private loading: Set<string> = new Set();
  /** Observer for visibility-based loading */
  private observer?: IntersectionObserver;
  /** Whether idle loading is active */
  private idleLoadingActive: boolean = false;

  /**
   * Add asset to loading queue
   */
  public queueAsset(metadata: AssetMetadata): void {
    // Don't queue if already loading or loaded
    if (this.loading.has(metadata.path)) {
      return;
    }

    // Add to queue based on priority
    const insertIndex = this.loadQueue.findIndex(
      (item) => item.priority > metadata.priority
    );

    if (insertIndex === -1) {
      this.loadQueue.push(metadata);
    } else {
      this.loadQueue.splice(insertIndex, 0, metadata);
    }

    // Start loading if strategy is immediate
    if (metadata.strategy === LoadStrategy.Immediate) {
      this.processQueue();
    }
  }

  /**
   * Process the loading queue
   */
  private processQueue(): void {
    // Process critical and high priority assets first
    const immediateAssets = this.loadQueue.filter(
      (asset) =>
        asset.strategy === LoadStrategy.Immediate &&
        (asset.priority === LoadPriority.Critical ||
          asset.priority === LoadPriority.High)
    );

    for (const asset of immediateAssets) {
      this.loadAsset(asset);
      this.loadQueue = this.loadQueue.filter((a) => a !== asset);
    }

    // Start idle loading if not active
    if (!this.idleLoadingActive) {
      this.startIdleLoading();
    }
  }

  /**
   * Load an asset
   */
  private async loadAsset(metadata: AssetMetadata): Promise<void> {
    if (this.loading.has(metadata.path)) {
      return;
    }

    this.loading.add(metadata.path);

    try {
      // Load asset based on type
      await this.fetchAsset(metadata);
    } catch (error) {
      console.error(`Failed to load asset: ${metadata.path}`, error);
    } finally {
      this.loading.delete(metadata.path);
    }
  }

  /**
   * Fetch asset with optimization
   */
  private async fetchAsset(metadata: AssetMetadata): Promise<void> {
    // Use fetch with appropriate options
    const response = await fetch(metadata.path, {
      // Use cache for better performance
      cache: 'default',
      // Set priority based on metadata
      priority:
        metadata.priority === LoadPriority.Critical
          ? 'high'
          : metadata.priority === LoadPriority.High
            ? 'high'
            : 'low',
    } as RequestInit);

    if (!response.ok) {
      throw new Error(`HTTP ${response.status}: ${response.statusText}`);
    }

    // Return the response for further processing
    return;
  }

  /**
   * Start idle loading
   */
  private startIdleLoading(): void {
    if (this.idleLoadingActive) {
      return;
    }

    this.idleLoadingActive = true;

    // Use requestIdleCallback if available
    if ('requestIdleCallback' in window) {
      this.scheduleIdleLoad();
    } else {
      // Fallback to setTimeout
      setTimeout(() => this.processIdleQueue(), 100);
    }
  }

  /**
   * Schedule idle loading
   */
  private scheduleIdleLoad(): void {
    (window as any).requestIdleCallback(
      (deadline: IdleDeadline) => {
        // Load assets while we have time
        while (deadline.timeRemaining() > 0 && this.loadQueue.length > 0) {
          const asset = this.loadQueue.find(
            (a) => a.strategy === LoadStrategy.Idle
          );

          if (asset) {
            this.loadAsset(asset);
            this.loadQueue = this.loadQueue.filter((a) => a !== asset);
          } else {
            break;
          }
        }

        // Schedule next idle callback if queue not empty
        if (this.loadQueue.length > 0) {
          this.scheduleIdleLoad();
        } else {
          this.idleLoadingActive = false;
        }
      },
      { timeout: 1000 }
    );
  }

  /**
   * Process idle loading queue
   */
  private processIdleQueue(): void {
    const idleAssets = this.loadQueue.filter(
      (a) => a.strategy === LoadStrategy.Idle
    );

    for (const asset of idleAssets) {
      this.loadAsset(asset);
      this.loadQueue = this.loadQueue.filter((a) => a !== asset);
    }

    this.idleLoadingActive = false;
  }

  /**
   * Enable visibility-based loading
   */
  public enableVisibilityLoading(elements: Element[]): void {
    if (!('IntersectionObserver' in window)) {
      console.warn('IntersectionObserver not available');
      return;
    }

    this.observer = new IntersectionObserver(
      (entries) => {
        for (const entry of entries) {
          if (entry.isIntersecting) {
            const path = entry.target.getAttribute('data-asset-path');
            if (path) {
              const asset = this.loadQueue.find((a) => a.path === path);
              if (asset) {
                this.loadAsset(asset);
                this.loadQueue = this.loadQueue.filter((a) => a !== asset);
              }
            }
          }
        }
      },
      {
        rootMargin: '50px', // Start loading slightly before visible
      }
    );

    elements.forEach((element) => this.observer?.observe(element));
  }

  /**
   * Preload critical assets
   */
  public async preloadCritical(paths: string[]): Promise<void> {
    const promises = paths.map((path) => {
      const metadata: AssetMetadata = {
        path,
        type: this.getAssetType(path),
        priority: LoadPriority.Critical,
        strategy: LoadStrategy.Immediate,
      };
      return this.loadAsset(metadata);
    });

    await Promise.all(promises);
  }

  /**
   * Get asset type from path
   */
  private getAssetType(path: string): string {
    const ext = path.split('.').pop()?.toLowerCase() || '';
    const typeMap: { [key: string]: string } = {
      png: 'image',
      jpg: 'image',
      jpeg: 'image',
      webp: 'image',
      gltf: 'model',
      glb: 'model',
      obj: 'model',
      mp3: 'audio',
      wav: 'audio',
      ogg: 'audio',
      js: 'script',
      wasm: 'wasm',
    };
    return typeMap[ext] || 'unknown';
  }

  /**
   * Clear the loading queue
   */
  public clearQueue(): void {
    this.loadQueue = [];
    this.loading.clear();
  }

  /**
   * Get queue statistics
   */
  public getStats(): { queued: number; loading: number } {
    return {
      queued: this.loadQueue.length,
      loading: this.loading.size,
    };
  }
}
