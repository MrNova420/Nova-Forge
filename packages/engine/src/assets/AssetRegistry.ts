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
 * AssetRegistry - Tracks loaded assets and reference counts
 */

import { AssetType } from './AssetLoader';

interface AssetMetadata {
  path: string;
  type: AssetType;
  references: number;
  loadedAt: number;
}

export class AssetRegistry {
  private _assets: Map<string, AssetMetadata>;

  constructor() {
    this._assets = new Map();
  }

  register(path: string, type: AssetType): void {
    if (!this._assets.has(path)) {
      this._assets.set(path, {
        path,
        type,
        references: 0,
        loadedAt: Date.now(),
      });
    }
  }

  unregister(path: string): boolean {
    return this._assets.delete(path);
  }

  incrementReferences(path: string): number {
    const metadata = this._assets.get(path);
    if (metadata) {
      metadata.references++;
      return metadata.references;
    }
    return 0;
  }

  decrementReferences(path: string): number {
    const metadata = this._assets.get(path);
    if (metadata && metadata.references > 0) {
      metadata.references--;
      return metadata.references;
    }
    return 0;
  }

  getReferences(path: string): number {
    const metadata = this._assets.get(path);
    return metadata ? metadata.references : 0;
  }

  getMetadata(path: string): AssetMetadata | null {
    return this._assets.get(path) || null;
  }

  getAllAssets(): string[] {
    return Array.from(this._assets.keys());
  }

  getAssetsByType(type: AssetType): string[] {
    const assets: string[] = [];
    for (const [path, metadata] of this._assets) {
      if (metadata.type === type) {
        assets.push(path);
      }
    }
    return assets;
  }

  getUnreferencedAssets(): string[] {
    const assets: string[] = [];
    for (const [path, metadata] of this._assets) {
      if (metadata.references === 0) {
        assets.push(path);
      }
    }
    return assets;
  }

  getAssetCount(): number {
    return this._assets.size;
  }

  clear(): void {
    this._assets.clear();
  }

  has(path: string): boolean {
    return this._assets.has(path);
  }
}
