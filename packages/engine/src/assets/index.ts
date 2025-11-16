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
