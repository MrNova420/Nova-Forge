/**
 * @fileoverview Build configuration types and interfaces
 */

/**
 * Target platforms for builds
 */
export enum BuildTarget {
  /** Web/Browser build (HTML5 + WebGL) */
  Web = 'web',
  /** Desktop build (Electron/Tauri) */
  Desktop = 'desktop',
  /** Mobile build (Capacitor/React Native) */
  Mobile = 'mobile',
  /** Server build (headless for multiplayer) */
  Server = 'server',
}

/**
 * Build modes
 */
export enum BuildMode {
  /** Development build with debugging */
  Development = 'development',
  /** Production build with optimizations */
  Production = 'production',
}

/**
 * Optimization level
 */
export enum OptimizationLevel {
  /** No optimization */
  None = 0,
  /** Basic optimization */
  Basic = 1,
  /** Standard optimization */
  Standard = 2,
  /** Aggressive optimization */
  Aggressive = 3,
}

/**
 * Build configuration options
 */
export interface BuildConfig {
  /** Target platform */
  target: BuildTarget;
  /** Build mode */
  mode: BuildMode;
  /** Optimization level */
  optimization: OptimizationLevel;
  /** Enable source maps */
  sourceMaps: boolean;
  /** Enable minification */
  minify: boolean;
  /** Enable code splitting */
  codeSplitting: boolean;
  /** Enable tree shaking */
  treeShaking: boolean;
  /** Output directory */
  outDir: string;
  /** Asset compression */
  compressAssets: boolean;
  /** Generate service worker */
  serviceWorker?: boolean;
  /** Custom defines/environment variables */
  defines?: Record<string, string>;
}

/**
 * Build result
 */
export interface BuildResult {
  /** Whether build was successful */
  success: boolean;
  /** Build output path */
  outputPath?: string;
  /** Build artifacts */
  artifacts?: BuildArtifact[];
  /** Build errors */
  errors?: string[];
  /** Build warnings */
  warnings?: string[];
  /** Build statistics */
  stats?: BuildStats;
}

/**
 * Build artifact
 */
export interface BuildArtifact {
  /** Artifact type */
  type: 'html' | 'js' | 'css' | 'asset' | 'wasm' | 'map';
  /** File path */
  path: string;
  /** File size in bytes */
  size: number;
  /** Gzipped size */
  gzipSize?: number;
}

/**
 * Build statistics
 */
export interface BuildStats {
  /** Build duration in milliseconds */
  duration: number;
  /** Total output size in bytes */
  totalSize: number;
  /** Number of artifacts */
  artifactCount: number;
  /** Chunk information */
  chunks?: ChunkInfo[];
}

/**
 * Chunk information
 */
export interface ChunkInfo {
  /** Chunk name */
  name: string;
  /** Chunk size */
  size: number;
  /** Modules in chunk */
  modules?: string[];
}

/**
 * Default build configuration
 */
export const DEFAULT_BUILD_CONFIG: BuildConfig = {
  target: BuildTarget.Web,
  mode: BuildMode.Production,
  optimization: OptimizationLevel.Standard,
  sourceMaps: false,
  minify: true,
  codeSplitting: true,
  treeShaking: true,
  outDir: 'dist',
  compressAssets: true,
  serviceWorker: true,
  defines: {},
};
