/**
 * Asset Optimizer
 * Provides utilities for optimizing assets for web deployment
 */

export interface OptimizationOptions {
  maxTextureSize?: number;
  compressTextures?: boolean;
  quality?: number;
  generateMipmaps?: boolean;
  targetFileSize?: number;
}

export interface AssetManifest {
  version: string;
  assets: AssetEntry[];
  totalSize: number;
  generatedAt: string;
}

export interface AssetEntry {
  path: string;
  type: string;
  size: number;
  hash: string;
  optimized: boolean;
  originalSize?: number;
  compressionRatio?: number;
}

export class AssetOptimizer {
  private options: Required<OptimizationOptions>;

  constructor(options: OptimizationOptions = {}) {
    this.options = {
      maxTextureSize: options.maxTextureSize || 2048,
      compressTextures: options.compressTextures ?? true,
      quality: options.quality || 85,
      generateMipmaps: options.generateMipmaps ?? true,
      targetFileSize: options.targetFileSize || 5 * 1024 * 1024,
    };
  }

  calculateHash(content: ArrayBuffer): string {
    const bytes = new Uint8Array(content);
    let hash = 0;
    for (let i = 0; i < bytes.length; i++) {
      const byte = bytes[i];
      if (byte !== undefined) {
        hash = (hash << 5) - hash + byte;
        hash |= 0;
      }
    }
    return Math.abs(hash).toString(36);
  }

  getOptimalTextureSize(
    width: number,
    height: number
  ): { width: number; height: number } {
    const max = this.options.maxTextureSize;
    if (width <= max && height <= max) {
      return { width, height };
    }

    const aspectRatio = width / height;
    let newWidth = Math.min(width, max);
    let newHeight = Math.min(height, max);

    if (width > height) {
      newWidth = max;
      newHeight = Math.floor(max / aspectRatio);
    } else {
      newHeight = max;
      newWidth = Math.floor(max * aspectRatio);
    }

    return {
      width: Math.pow(2, Math.ceil(Math.log2(newWidth))),
      height: Math.pow(2, Math.ceil(Math.log2(newHeight))),
    };
  }

  needsOptimization(size: number, type: string): boolean {
    const thresholds: Record<string, number> = {
      'image/png': 500 * 1024,
      'image/jpeg': 500 * 1024,
      'audio/wav': 1024 * 1024,
      'model/obj': 1024 * 1024,
    };
    return size > (thresholds[type] || 1024 * 1024);
  }
}

export class BundleSizeAnalyzer {
  analyzeBundleSize(manifest: AssetManifest) {
    const byCategory = {
      code: 0,
      textures: 0,
      models: 0,
      audio: 0,
      other: 0,
    };

    for (const asset of manifest.assets) {
      if (asset.type.startsWith('image/')) byCategory.textures += asset.size;
      else if (asset.type.startsWith('model/')) byCategory.models += asset.size;
      else if (asset.type.startsWith('audio/')) byCategory.audio += asset.size;
      else if (asset.type.includes('javascript') || asset.type.includes('css'))
        byCategory.code += asset.size;
      else byCategory.other += asset.size;
    }

    return {
      totalSize: manifest.totalSize,
      byCategory,
      assetCount: manifest.assets.length,
    };
  }

  getRecommendations(analysis: any): string[] {
    const recommendations: string[] = [];
    if (analysis.totalSize > 10 * 1024 * 1024) {
      recommendations.push('Bundle exceeds 10MB. Consider lazy loading.');
    }
    if (analysis.byCategory.code > 1024 * 1024) {
      recommendations.push('Code bundle >1MB. Enable code splitting.');
    }
    return recommendations;
  }
}
