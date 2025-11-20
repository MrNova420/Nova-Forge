/**
 * @file SpatialStreamingManager.ts
 * @description Advanced spatial management for resource streaming
 * Uses spatial hashing for O(1) region queries and predictive loading
 */

import { StreamingRegion, StreamingPriority } from './IResourceStreaming';

export interface Vector3 {
  x: number;
  y: number;
  z: number;
}

export interface BoundingBox {
  min: Vector3;
  max: Vector3;
}

export interface Frustum {
  planes: Plane[];
}

export interface Plane {
  normal: Vector3;
  distance: number;
}

/**
 * Spatial hash grid for efficient spatial queries
 */
export class SpatialStreamingManager {
  private grid: Map<string, StreamingRegion[]> = new Map();
  private cellSize: number;
  private activeRegions: Set<string> = new Set();
  private predictiveLoadDistance: number = 50.0;

  constructor(cellSize: number = 100) {
    this.cellSize = cellSize;
  }

  /**
   * Add region to spatial grid
   */
  addRegion(region: StreamingRegion): void {
    const cells = this.getRegionCells(region.bounds);
    
    for (const cell of cells) {
      if (!this.grid.has(cell)) {
        this.grid.set(cell, []);
      }
      this.grid.get(cell)!.push(region);
    }
  }

  /**
   * Remove region from spatial grid
   */
  removeRegion(region: StreamingRegion): void {
    const cells = this.getRegionCells(region.bounds);
    
    for (const cell of cells) {
      const regions = this.grid.get(cell);
      if (regions) {
        const index = regions.indexOf(region);
        if (index !== -1) {
          regions.splice(index, 1);
        }
        if (regions.length === 0) {
          this.grid.delete(cell);
        }
      }
    }
  }

  /**
   * Find regions near position (O(1) per cell)
   */
  findNearbyRegions(position: Vector3, radius: number): StreamingRegion[] {
    const cellRadius = Math.ceil(radius / this.cellSize);
    const centerCell = this.positionToCell(position);
    const regions = new Set<StreamingRegion>();

    // Check surrounding cells
    for (let dx = -cellRadius; dx <= cellRadius; dx++) {
      for (let dy = -cellRadius; dy <= cellRadius; dy++) {
        for (let dz = -cellRadius; dz <= cellRadius; dz++) {
          const cellKey = `${centerCell.x + dx},${centerCell.y + dy},${centerCell.z + dz}`;
          const cellRegions = this.grid.get(cellKey);
          
          if (cellRegions) {
            for (const region of cellRegions) {
              // Check actual distance
              const distance = this.distanceToRegion(position, region.bounds);
              if (distance <= radius) {
                regions.add(region);
              }
            }
          }
        }
      }
    }

    return Array.from(regions);
  }

  /**
   * Update regions based on camera position and view distance
   */
  updateRegions(cameraPos: Vector3, viewDistance: number): StreamingRegion[] {
    const nearbyRegions = this.findNearbyRegions(cameraPos, viewDistance);
    
    // Sort by distance for priority
    nearbyRegions.sort((a, b) => {
      const distA = this.distanceToRegion(cameraPos, a.bounds);
      const distB = this.distanceToRegion(cameraPos, b.bounds);
      return distA - distB;
    });

    // Mark as active
    for (const region of nearbyRegions) {
      this.activeRegions.add(region.id);
    }

    return nearbyRegions;
  }

  /**
   * Find regions within camera frustum
   */
  findVisibleRegions(frustum: Frustum): StreamingRegion[] {
    const visibleRegions: StreamingRegion[] = [];

    // Check all regions (could be optimized with frustum-grid intersection)
    for (const cellRegions of this.grid.values()) {
      for (const region of cellRegions) {
        if (this.isRegionInFrustum(region.bounds, frustum)) {
          visibleRegions.push(region);
        }
      }
    }

    return visibleRegions;
  }

  /**
   * Update regions considering occlusion
   */
  updateOccludedRegions(frustum: Frustum, occluders: BoundingBox[]): StreamingRegion[] {
    const visibleRegions = this.findVisibleRegions(frustum);
    
    // Filter out occluded regions
    return visibleRegions.filter(region => {
      return !this.isRegionOccluded(region.bounds, occluders);
    });
  }

  /**
   * Predictive loading based on player movement
   */
  predictAndPreload(
    playerPos: Vector3,
    velocity: Vector3,
    predictionTime: number = 2.0
  ): StreamingRegion[] {
    // Predict future position
    const futurePos: Vector3 = {
      x: playerPos.x + velocity.x * predictionTime,
      y: playerPos.y + velocity.y * predictionTime,
      z: playerPos.z + velocity.z * predictionTime,
    };

    // Find regions along path
    const pathRegions = this.findRegionsAlongPath(playerPos, futurePos);
    
    // Return regions to preload
    return pathRegions.filter(region => !this.activeRegions.has(region.id));
  }

  /**
   * Find regions along movement path
   */
  findRegionsAlongPath(start: Vector3, end: Vector3): StreamingRegion[] {
    const regions = new Set<StreamingRegion>();
    const steps = 10;
    
    for (let i = 0; i <= steps; i++) {
      const t = i / steps;
      const pos: Vector3 = {
        x: start.x + (end.x - start.x) * t,
        y: start.y + (end.y - start.y) * t,
        z: start.z + (end.z - start.z) * t,
      };
      
      const nearbyRegions = this.findNearbyRegions(pos, this.predictiveLoadDistance);
      for (const region of nearbyRegions) {
        regions.add(region);
      }
    }

    return Array.from(regions);
  }

  /**
   * Get streaming priority based on distance and visibility
   */
  calculatePriority(region: StreamingRegion, cameraPos: Vector3, isVisible: boolean): StreamingPriority {
    const distance = this.distanceToRegion(cameraPos, region.bounds);
    
    if (isVisible) {
      if (distance < 20) return StreamingPriority.CRITICAL;
      if (distance < 50) return StreamingPriority.HIGH;
      return StreamingPriority.NORMAL;
    } else {
      if (distance < 100) return StreamingPriority.LOW;
      return StreamingPriority.BACKGROUND;
    }
  }

  /**
   * Convert position to grid cell coordinates
   */
  private positionToCell(position: Vector3): { x: number; y: number; z: number } {
    return {
      x: Math.floor(position.x / this.cellSize),
      y: Math.floor(position.y / this.cellSize),
      z: Math.floor(position.z / this.cellSize),
    };
  }

  /**
   * Get all grid cells that overlap with a bounding box
   */
  private getRegionCells(bounds: BoundingBox): string[] {
    const minCell = this.positionToCell(bounds.min);
    const maxCell = this.positionToCell(bounds.max);
    const cells: string[] = [];

    for (let x = minCell.x; x <= maxCell.x; x++) {
      for (let y = minCell.y; y <= maxCell.y; y++) {
        for (let z = minCell.z; z <= maxCell.z; z++) {
          cells.push(`${x},${y},${z}`);
        }
      }
    }

    return cells;
  }

  /**
   * Calculate distance from point to bounding box
   */
  private distanceToRegion(point: Vector3, bounds: BoundingBox): number {
    // Calculate center of bounding box
    const center: Vector3 = {
      x: (bounds.min.x + bounds.max.x) / 2,
      y: (bounds.min.y + bounds.max.y) / 2,
      z: (bounds.min.z + bounds.max.z) / 2,
    };

    // Calculate distance
    const dx = center.x - point.x;
    const dy = center.y - point.y;
    const dz = center.z - point.z;

    return Math.sqrt(dx * dx + dy * dy + dz * dz);
  }

  /**
   * Check if bounding box is within frustum
   */
  private isRegionInFrustum(bounds: BoundingBox, frustum: Frustum): boolean {
    // Test bounding box against all frustum planes
    for (const plane of frustum.planes) {
      if (this.isBoxOutsidePlane(bounds, plane)) {
        return false;
      }
    }
    return true;
  }

  /**
   * Check if bounding box is outside a plane
   */
  private isBoxOutsidePlane(bounds: BoundingBox, plane: Plane): boolean {
    // Get positive vertex (farthest from plane)
    const p: Vector3 = {
      x: plane.normal.x >= 0 ? bounds.max.x : bounds.min.x,
      y: plane.normal.y >= 0 ? bounds.max.y : bounds.min.y,
      z: plane.normal.z >= 0 ? bounds.max.z : bounds.min.z,
    };

    // Check if positive vertex is outside plane
    const distance = this.dotProduct(plane.normal, p) + plane.distance;
    return distance < 0;
  }

  /**
   * Check if region is occluded by any occluder
   */
  private isRegionOccluded(bounds: BoundingBox, occluders: BoundingBox[]): boolean {
    // Simplified occlusion check - would be more sophisticated in production
    for (const occluder of occluders) {
      if (this.isBoxOccludedByBox(bounds, occluder)) {
        return true;
      }
    }
    return false;
  }

  /**
   * Check if one box is occluded by another
   */
  private isBoxOccludedByBox(bounds: BoundingBox, occluder: BoundingBox): boolean {
    // Very simplified - would need proper occlusion algorithm
    // Check if occluder is in front and larger
    return false; // Placeholder
  }

  /**
   * Vector dot product
   */
  private dotProduct(a: Vector3, b: Vector3): number {
    return a.x * b.x + a.y * b.y + a.z * b.z;
  }

  /**
   * Get statistics
   */
  getStats(): {
    totalRegions: number;
    activeRegions: number;
    gridCells: number;
    avgRegionsPerCell: number;
  } {
    let totalRegions = 0;
    for (const regions of this.grid.values()) {
      totalRegions += regions.length;
    }

    return {
      totalRegions: totalRegions,
      activeRegions: this.activeRegions.size,
      gridCells: this.grid.size,
      avgRegionsPerCell: this.grid.size > 0 ? totalRegions / this.grid.size : 0,
    };
  }

  /**
   * Clear all regions
   */
  clear(): void {
    this.grid.clear();
    this.activeRegions.clear();
  }
}

/**
 * Streaming cache with LRU eviction and predictive preloading
 */
export class StreamingCacheOptimizer {
  private cache: Map<string, CacheEntry> = new Map();
  private accessOrder: string[] = [];
  private maxCacheSize: number;
  private currentCacheSize: number = 0;

  constructor(maxCacheSizeMB: number = 512) {
    this.maxCacheSize = maxCacheSizeMB * 1024 * 1024;
  }

  /**
   * Add item to cache
   */
  add(id: string, data: any, size: number): void {
    // Check if we need to evict
    while (this.currentCacheSize + size > this.maxCacheSize && this.accessOrder.length > 0) {
      this.evictLRU();
    }

    this.cache.set(id, {
      data,
      size,
      accessTime: Date.now(),
      accessCount: 0,
    });

    this.currentCacheSize += size;
    this.updateAccessOrder(id);
  }

  /**
   * Get item from cache
   */
  get(id: string): any | null {
    const entry = this.cache.get(id);
    if (!entry) {
      return null;
    }

    entry.accessTime = Date.now();
    entry.accessCount++;
    this.updateAccessOrder(id);

    return entry.data;
  }

  /**
   * Check if item is in cache
   */
  has(id: string): boolean {
    return this.cache.has(id);
  }

  /**
   * Remove item from cache
   */
  remove(id: string): void {
    const entry = this.cache.get(id);
    if (!entry) return;

    this.currentCacheSize -= entry.size;
    this.cache.delete(id);
    
    const index = this.accessOrder.indexOf(id);
    if (index !== -1) {
      this.accessOrder.splice(index, 1);
    }
  }

  /**
   * Predictive preloading based on player velocity
   */
  predictAndPreload(
    playerPos: Vector3,
    velocity: Vector3,
    getResourceIdsAtPosition: (pos: Vector3) => string[]
  ): string[] {
    // Predict positions along movement path
    const predictions: Vector3[] = [];
    const predictionSteps = 5;
    const timeStep = 0.5; // 0.5 seconds per step

    for (let i = 1; i <= predictionSteps; i++) {
      predictions.push({
        x: playerPos.x + velocity.x * timeStep * i,
        y: playerPos.y + velocity.y * timeStep * i,
        z: playerPos.z + velocity.z * timeStep * i,
      });
    }

    // Get resources at predicted positions
    const resourcesToPreload: string[] = [];
    for (const pos of predictions) {
      const resources = getResourceIdsAtPosition(pos);
      for (const resourceId of resources) {
        if (!this.has(resourceId) && !resourcesToPreload.includes(resourceId)) {
          resourcesToPreload.push(resourceId);
        }
      }
    }

    return resourcesToPreload;
  }

  /**
   * Optimize cache size based on usage patterns
   */
  optimizeCacheSize(): void {
    // Analyze cache hit rate and adjust size if needed
    const stats = this.getStats();
    
    if (stats.hitRate < 0.7 && this.maxCacheSize < 1024 * 1024 * 1024) {
      // Low hit rate, try to increase cache size
      this.maxCacheSize = Math.min(this.maxCacheSize * 1.2, 1024 * 1024 * 1024);
    } else if (stats.hitRate > 0.95 && this.currentCacheSize < this.maxCacheSize * 0.5) {
      // Very high hit rate and not using much cache, can reduce
      this.maxCacheSize = Math.max(this.maxCacheSize * 0.9, 256 * 1024 * 1024);
    }
  }

  /**
   * Handle memory pressure
   */
  handleMemoryPressure(severity: number): void {
    // severity: 0-1, where 1 is critical
    const targetSize = this.maxCacheSize * (1 - severity * 0.5);
    
    while (this.currentCacheSize > targetSize && this.accessOrder.length > 0) {
      this.evictLRU();
    }
  }

  /**
   * Evict least recently used item
   */
  private evictLRU(): void {
    if (this.accessOrder.length === 0) return;
    
    const lruId = this.accessOrder[0];
    this.remove(lruId);
  }

  /**
   * Update access order (move to end)
   */
  private updateAccessOrder(id: string): void {
    const index = this.accessOrder.indexOf(id);
    if (index !== -1) {
      this.accessOrder.splice(index, 1);
    }
    this.accessOrder.push(id);
  }

  /**
   * Get cache statistics
   */
  getStats(): {
    size: number;
    maxSize: number;
    utilization: number;
    entries: number;
    hitRate: number;
  } {
    let totalAccesses = 0;
    let totalHits = 0;

    for (const entry of this.cache.values()) {
      totalAccesses += entry.accessCount;
      if (entry.accessCount > 0) {
        totalHits += entry.accessCount;
      }
    }

    return {
      size: this.currentCacheSize,
      maxSize: this.maxCacheSize,
      utilization: this.currentCacheSize / this.maxCacheSize,
      entries: this.cache.size,
      hitRate: totalAccesses > 0 ? totalHits / totalAccesses : 0,
    };
  }

  /**
   * Clear cache
   */
  clear(): void {
    this.cache.clear();
    this.accessOrder = [];
    this.currentCacheSize = 0;
  }
}

interface CacheEntry {
  data: any;
  size: number;
  accessTime: number;
  accessCount: number;
}
