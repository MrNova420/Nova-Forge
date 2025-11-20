/**
 * @fileoverview Resource Streaming System - Core Interfaces
 * @module @nova-engine/streaming
 *
 * AAA-Grade resource streaming for large open worlds.
 * Load/unload assets dynamically based on player location and memory budget.
 *
 * Following Phase 1.4 of the autonomous development guide
 * Production-grade implementation rivaling Unreal/Unity streaming
 */

/**
 * Resource priority levels
 */
export enum StreamingPriority {
  /** Critical resources (player immediate vicinity) */
  CRITICAL = 0,
  /** High priority (visible, close proximity) */
  HIGH = 1,
  /** Normal priority (nearby, may become visible) */
  NORMAL = 2,
  /** Low priority (distant, preload) */
  LOW = 3,
  /** Background priority (optional, fill spare capacity) */
  BACKGROUND = 4,
}

/**
 * Resource types that can be streamed
 */
export enum ResourceType {
  MESH = 'mesh',
  TEXTURE = 'texture',
  AUDIO = 'audio',
  SCENE = 'scene',
  SHADER = 'shader',
  ANIMATION = 'animation',
  MATERIAL = 'material',
  PREFAB = 'prefab',
}

/**
 * Streaming state
 */
export enum StreamingState {
  /** Not loaded */
  UNLOADED = 'unloaded',
  /** Load requested */
  REQUESTED = 'requested',
  /** Currently loading */
  LOADING = 'loading',
  /** Fully loaded and ready */
  LOADED = 'loaded',
  /** Unload requested */
  UNLOAD_REQUESTED = 'unload_requested',
  /** Currently unloading */
  UNLOADING = 'unloading',
  /** Load failed */
  FAILED = 'failed',
}

/**
 * LOD (Level of Detail) level
 */
export enum LODLevel {
  /** Highest quality (closest to camera) */
  LOD0 = 0,
  /** High quality */
  LOD1 = 1,
  /** Medium quality */
  LOD2 = 2,
  /** Low quality */
  LOD3 = 3,
  /** Lowest quality (furthest from camera) */
  LOD4 = 4,
}

/**
 * Streaming request
 */
export interface StreamingRequest {
  /** Unique request ID */
  id: string;
  /** Resource URL or path */
  url: string;
  /** Resource type */
  type: ResourceType;
  /** Priority level */
  priority: StreamingPriority;
  /** Required LOD level */
  lodLevel: LODLevel;
  /** Distance from camera/player */
  distance: number;
  /** Timestamp when requested */
  requestTime: number;
  /** Completion callback */
  onComplete?: (resource: StreamedResource) => void;
  /** Error callback */
  onError?: (error: Error) => void;
}

/**
 * Streamed resource
 */
export interface StreamedResource {
  /** Resource ID */
  id: string;
  /** Resource URL */
  url: string;
  /** Resource type */
  type: ResourceType;
  /** Current state */
  state: StreamingState;
  /** Current LOD level */
  lodLevel: LODLevel;
  /** Resource data (type-specific) */
  data: unknown;
  /** Size in bytes */
  size: number;
  /** Last access time */
  lastAccessTime: number;
  /** Reference count */
  refCount: number;
  /** Load time in ms */
  loadTime?: number;
}

/**
 * Streaming region (spatial area for streaming control)
 */
export interface StreamingRegion {
  /** Region ID */
  id: string;
  /** Center position */
  center: { x: number; y: number; z: number };
  /** Radius in world units */
  radius: number;
  /** Priority for resources in this region */
  priority: StreamingPriority;
  /** Resources to load in this region */
  resources: string[];
  /** Whether region is currently active */
  active: boolean;
}

/**
 * Memory budget configuration
 */
export interface MemoryBudget {
  /** Maximum total memory in bytes */
  maxTotal: number;
  /** Mesh memory budget */
  meshBudget: number;
  /** Texture memory budget */
  textureBudget: number;
  /** Audio memory budget */
  audioBudget: number;
  /** Other resources budget */
  otherBudget: number;
  /** Warning threshold (0-1) */
  warningThreshold: number;
  /** Critical threshold (0-1) */
  criticalThreshold: number;
}

/**
 * Streaming statistics
 */
export interface StreamingStats {
  /** Total resources loaded */
  totalLoaded: number;
  /** Total resources unloaded */
  totalUnloaded: number;
  /** Currently loading */
  currentlyLoading: number;
  /** Currently loaded */
  currentlyLoaded: number;
  /** Total memory used (bytes) */
  memoryUsed: number;
  /** Memory by resource type */
  memoryByType: Record<ResourceType, number>;
  /** Average load time (ms) */
  averageLoadTime: number;
  /** Failed loads */
  failedLoads: number;
  /** Cache hit rate (0-1) */
  cacheHitRate: number;
}

/**
 * Streaming manager configuration
 */
export interface StreamingConfig {
  /** Memory budget */
  memoryBudget: MemoryBudget;
  /** Maximum concurrent loads */
  maxConcurrentLoads: number;
  /** Distance-based LOD thresholds */
  lodDistances: number[];
  /** Enable aggressive unloading when memory low */
  aggressiveUnloading: boolean;
  /** Preload distance multiplier */
  preloadMultiplier: number;
  /** Enable streaming cache */
  enableCache: boolean;
  /** Cache size in bytes */
  cacheSize: number;
}

/**
 * Resource streaming manager interface
 */
export interface IStreamingManager {
  /**
   * Initialize streaming system
   */
  initialize(config: StreamingConfig): Promise<void>;

  /**
   * Shutdown streaming system
   */
  shutdown(): Promise<void>;

  /**
   * Request resource to be loaded
   */
  requestLoad(request: StreamingRequest): Promise<StreamedResource>;

  /**
   * Request resource to be unloaded
   */
  requestUnload(resourceId: string): void;

  /**
   * Update streaming (call per frame)
   * @param cameraPosition Current camera position
   * @param deltaTime Time since last update
   */
  update(cameraPosition: { x: number; y: number; z: number }, deltaTime: number): void;

  /**
   * Register streaming region
   */
  registerRegion(region: StreamingRegion): void;

  /**
   * Unregister streaming region
   */
  unregisterRegion(regionId: string): void;

  /**
   * Get resource by ID
   */
  getResource(resourceId: string): StreamedResource | undefined;

  /**
   * Get all loaded resources
   */
  getLoadedResources(): StreamedResource[];

  /**
   * Get streaming statistics
   */
  getStats(): StreamingStats;

  /**
   * Force unload all resources below priority
   */
  forceUnloadBelow(priority: StreamingPriority): void;

  /**
   * Get optimal LOD level for distance
   */
  getLODForDistance(distance: number): LODLevel;

  /**
   * Set memory budget
   */
  setMemoryBudget(budget: MemoryBudget): void;

  /**
   * Get current memory usage
   */
  getMemoryUsage(): number;

  /**
   * Is memory critical
   */
  isMemoryCritical(): boolean;
}
