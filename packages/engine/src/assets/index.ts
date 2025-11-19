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
 * Assets Module
 */

export { AssetManager } from './AssetManager';
export type { AssetLoadOptions } from './AssetManager';
export { AssetCache } from './AssetCache';
export { AssetRegistry } from './AssetRegistry';
export {
  AssetType,
  TextLoader,
  JSONLoader,
  BinaryLoader,
  ImageLoader,
} from './AssetLoader';
export type { AssetLoader } from './AssetLoader';

// Specialized asset loaders
export {
  TextureAssetLoader,
  AudioLoader,
  OBJLoader,
  GLTFLoader,
} from './loaders';
export type {
  AudioData,
  OBJData,
  GLTFData,
  GLTFScene,
  GLTFNode,
} from './loaders';

// Asset optimization
export { AssetOptimizer, LoadPriority, LoadStrategy } from './AssetOptimizer';
export type { AssetMetadata } from './AssetOptimizer';
