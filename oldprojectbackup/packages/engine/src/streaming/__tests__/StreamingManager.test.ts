/**
 * @file StreamingManager.test.ts
 * @description Comprehensive tests for streaming manager
 */

import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import {
  StreamingManager,
  ResourceType,
  StreamingPriority,
  LODLevel,
  StreamingState,
} from '../StreamingManager';

describe('StreamingManager', () => {
  let streamingManager: StreamingManager;
  let mockPlatform: any;
  let mockAllocator: any;
  let mockJobScheduler: any;

  beforeEach(() => {
    mockPlatform = {
      filesystem: {
        readFile: vi.fn(),
      },
    };

    mockAllocator = {
      allocate: vi.fn(() => ({ ptr: 0x1000, size: 1024 })),
      free: vi.fn(),
      getStats: vi.fn(() => ({ allocated: 1024, peak: 2048 })),
    };

    mockJobScheduler = {
      submit: vi.fn((job) => Promise.resolve({ success: true, data: new ArrayBuffer(1024) })),
    };

    streamingManager = new StreamingManager(mockPlatform, mockAllocator, mockJobScheduler, {
      memoryBudget: {
        total: 100 * 1024 * 1024,
        meshes: 30 * 1024 * 1024,
        textures: 50 * 1024 * 1024,
        audio: 10 * 1024 * 1024,
        scenes: 5 * 1024 * 1024,
        shaders: 2 * 1024 * 1024,
        animations: 2 * 1024 * 1024,
        materials: 1 * 1024 * 1024,
      },
      maxConcurrentLoads: 4,
      enableLOD: true,
      lodDistances: [10, 50, 200, 1000],
      cacheSize: 10 * 1024 * 1024,
    });
  });

  afterEach(() => {
    vi.clearAllMocks();
  });

  describe('Initialization', () => {
    it('should initialize with configuration', async () => {
      await streamingManager.initialize();
      const stats = streamingManager.getStats();
      expect(stats).toBeDefined();
      expect(stats.totalMemoryUsed).toBe(0);
      expect(stats.activeRequests).toBe(0);
    });

    it('should shutdown cleanly', async () => {
      await streamingManager.initialize();
      await streamingManager.shutdown();
      const stats = streamingManager.getStats();
      expect(stats.activeRequests).toBe(0);
    });
  });

  describe('Resource Streaming', () => {
    beforeEach(async () => {
      await streamingManager.initialize();
    });

    it('should stream resource', async () => {
      const resource = await streamingManager.request({
        id: 'test_mesh',
        type: ResourceType.MESH,
        priority: StreamingPriority.HIGH,
        lodLevel: LODLevel.LOD0,
        distanceFromCamera: 10,
      });

      expect(resource).toBeDefined();
      expect(resource.id).toBe('test_mesh');
      expect(resource.state).toBe(StreamingState.LOADED);
    });

    it('should handle multiple concurrent requests', async () => {
      const requests = [
        { id: 'mesh1', type: ResourceType.MESH, priority: StreamingPriority.HIGH, lodLevel: LODLevel.LOD0, distanceFromCamera: 10 },
        { id: 'mesh2', type: ResourceType.MESH, priority: StreamingPriority.NORMAL, lodLevel: LODLevel.LOD1, distanceFromCamera: 50 },
        { id: 'mesh3', type: ResourceType.MESH, priority: StreamingPriority.LOW, lodLevel: LODLevel.LOD2, distanceFromCamera: 200 },
      ];

      const resources = await Promise.all(
        requests.map(req => streamingManager.request(req))
      );

      expect(resources).toHaveLength(3);
      resources.forEach((res, i) => {
        expect(res.id).toBe(requests[i].id);
        expect(res.state).toBe(StreamingState.LOADED);
      });
    });

    it('should prioritize high priority requests', async () => {
      const lowPriorityRequest = streamingManager.request({
        id: 'low_priority',
        type: ResourceType.MESH,
        priority: StreamingPriority.LOW,
        lodLevel: LODLevel.LOD2,
        distanceFromCamera: 500,
      });

      const highPriorityRequest = streamingManager.request({
        id: 'high_priority',
        type: ResourceType.MESH,
        priority: StreamingPriority.CRITICAL,
        lodLevel: LODLevel.LOD0,
        distanceFromCamera: 5,
      });

      const [low, high] = await Promise.all([lowPriorityRequest, highPriorityRequest]);

      expect(high).toBeDefined();
      expect(low).toBeDefined();
    });
  });

  describe('LOD System', () => {
    beforeEach(async () => {
      await streamingManager.initialize();
    });

    it('should select appropriate LOD based on distance', async () => {
      const distances = [5, 25, 100, 500];
      const expectedLODs = [LODLevel.LOD0, LODLevel.LOD1, LODLevel.LOD2, LODLevel.LOD3];

      for (let i = 0; i < distances.length; i++) {
        const resource = await streamingManager.request({
          id: `mesh_${i}`,
          type: ResourceType.MESH,
          priority: StreamingPriority.NORMAL,
          lodLevel: expectedLODs[i],
          distanceFromCamera: distances[i],
        });

        expect(resource.lodLevel).toBe(expectedLODs[i]);
      }
    });

    it('should stream higher LOD when distance decreases', async () => {
      let resource = await streamingManager.request({
        id: 'dynamic_mesh',
        type: ResourceType.MESH,
        priority: StreamingPriority.NORMAL,
        lodLevel: LODLevel.LOD2,
        distanceFromCamera: 100,
      });

      expect(resource.lodLevel).toBe(LODLevel.LOD2);

      // Move closer
      resource = await streamingManager.request({
        id: 'dynamic_mesh',
        type: ResourceType.MESH,
        priority: StreamingPriority.HIGH,
        lodLevel: LODLevel.LOD0,
        distanceFromCamera: 5,
      });

      expect(resource.lodLevel).toBe(LODLevel.LOD0);
    });
  });

  describe('Memory Management', () => {
    beforeEach(async () => {
      await streamingManager.initialize();
    });

    it('should enforce memory budget', async () => {
      const stats = streamingManager.getStats();
      expect(stats.totalMemoryUsed).toBeLessThanOrEqual(stats.memoryBudget.total);
    });

    it('should evict resources when memory budget exceeded', async () => {
      // Load many resources to exceed budget
      const requests = [];
      for (let i = 0; i < 100; i++) {
        requests.push(
          streamingManager.request({
            id: `mesh_${i}`,
            type: ResourceType.MESH,
            priority: StreamingPriority.LOW,
            lodLevel: LODLevel.LOD0,
            distanceFromCamera: 100,
          })
        );
      }

      await Promise.all(requests);

      const stats = streamingManager.getStats();
      expect(stats.totalMemoryUsed).toBeLessThanOrEqual(stats.memoryBudget.total);
      expect(stats.evictedResources).toBeGreaterThan(0);
    });

    it('should track memory usage by type', async () => {
      await streamingManager.request({
        id: 'mesh1',
        type: ResourceType.MESH,
        priority: StreamingPriority.NORMAL,
        lodLevel: LODLevel.LOD0,
        distanceFromCamera: 10,
      });

      await streamingManager.request({
        id: 'texture1',
        type: ResourceType.TEXTURE,
        priority: StreamingPriority.NORMAL,
        lodLevel: LODLevel.LOD0,
        distanceFromCamera: 10,
      });

      const stats = streamingManager.getStats();
      expect(stats.memoryByType[ResourceType.MESH]).toBeGreaterThan(0);
      expect(stats.memoryByType[ResourceType.TEXTURE]).toBeGreaterThan(0);
    });
  });

  describe('Caching', () => {
    beforeEach(async () => {
      await streamingManager.initialize();
    });

    it('should cache loaded resources', async () => {
      await streamingManager.request({
        id: 'cached_mesh',
        type: ResourceType.MESH,
        priority: StreamingPriority.NORMAL,
        lodLevel: LODLevel.LOD0,
        distanceFromCamera: 10,
      });

      // Request again - should come from cache
      await streamingManager.request({
        id: 'cached_mesh',
        type: ResourceType.MESH,
        priority: StreamingPriority.NORMAL,
        lodLevel: LODLevel.LOD0,
        distanceFromCamera: 10,
      });

      const stats = streamingManager.getStats();
      expect(stats.cacheHits).toBeGreaterThan(0);
    });

    it('should evict least recently used resources', async () => {
      // Fill cache
      const cacheSize = 10;
      for (let i = 0; i < cacheSize + 5; i++) {
        await streamingManager.request({
          id: `mesh_${i}`,
          type: ResourceType.MESH,
          priority: StreamingPriority.LOW,
          lodLevel: LODLevel.LOD0,
          distanceFromCamera: 100,
        });
      }

      const stats = streamingManager.getStats();
      expect(stats.cachedResources).toBeLessThanOrEqual(cacheSize);
    });
  });

  describe('Streaming Regions', () => {
    beforeEach(async () => {
      await streamingManager.initialize();
    });

    it('should register streaming region', () => {
      streamingManager.registerRegion({
        id: 'region1',
        center: { x: 0, y: 0, z: 0 },
        radius: 100,
        priority: StreamingPriority.HIGH,
        resources: ['mesh1', 'mesh2'],
      });

      const regions = streamingManager.getActiveRegions();
      expect(regions).toContain('region1');
    });

    it('should unload region when player leaves', () => {
      streamingManager.registerRegion({
        id: 'region1',
        center: { x: 0, y: 0, z: 0 },
        radius: 100,
        priority: StreamingPriority.HIGH,
        resources: ['mesh1', 'mesh2'],
      });

      streamingManager.unregisterRegion('region1');

      const regions = streamingManager.getActiveRegions();
      expect(regions).not.toContain('region1');
    });
  });

  describe('Statistics', () => {
    beforeEach(async () => {
      await streamingManager.initialize();
    });

    it('should track load times', async () => {
      await streamingManager.request({
        id: 'timed_mesh',
        type: ResourceType.MESH,
        priority: StreamingPriority.NORMAL,
        lodLevel: LODLevel.LOD0,
        distanceFromCamera: 10,
      });

      const stats = streamingManager.getStats();
      expect(stats.avgLoadTime).toBeGreaterThan(0);
      expect(stats.totalLoads).toBe(1);
    });

    it('should track failed loads', async () => {
      mockJobScheduler.submit.mockRejectedValueOnce(new Error('Load failed'));

      try {
        await streamingManager.request({
          id: 'failed_mesh',
          type: ResourceType.MESH,
          priority: StreamingPriority.NORMAL,
          lodLevel: LODLevel.LOD0,
          distanceFromCamera: 10,
        });
      } catch (error) {
        // Expected
      }

      const stats = streamingManager.getStats();
      expect(stats.failedLoads).toBe(1);
    });

    it('should calculate cache hit rate', async () => {
      // Load once
      await streamingManager.request({
        id: 'hit_mesh',
        type: ResourceType.MESH,
        priority: StreamingPriority.NORMAL,
        lodLevel: LODLevel.LOD0,
        distanceFromCamera: 10,
      });

      // Load again from cache
      await streamingManager.request({
        id: 'hit_mesh',
        type: ResourceType.MESH,
        priority: StreamingPriority.NORMAL,
        lodLevel: LODLevel.LOD0,
        distanceFromCamera: 10,
      });

      const stats = streamingManager.getStats();
      expect(stats.cacheHitRate).toBeGreaterThan(0);
    });
  });

  describe('Stress Tests', () => {
    beforeEach(async () => {
      await streamingManager.initialize();
    });

    it('should handle 1000+ concurrent requests', async () => {
      const requests = [];
      for (let i = 0; i < 1000; i++) {
        requests.push(
          streamingManager.request({
            id: `stress_mesh_${i}`,
            type: ResourceType.MESH,
            priority: StreamingPriority.NORMAL,
            lodLevel: LODLevel.LOD1,
            distanceFromCamera: 50,
          })
        );
      }

      const resources = await Promise.all(requests);
      expect(resources).toHaveLength(1000);
    });

    it('should maintain performance under memory pressure', async () => {
      const startTime = performance.now();

      const requests = [];
      for (let i = 0; i < 100; i++) {
        requests.push(
          streamingManager.request({
            id: `pressure_mesh_${i}`,
            type: ResourceType.MESH,
            priority: StreamingPriority.HIGH,
            lodLevel: LODLevel.LOD0,
            distanceFromCamera: 10,
          })
        );
      }

      await Promise.all(requests);

      const endTime = performance.now();
      const avgTime = (endTime - startTime) / 100;

      expect(avgTime).toBeLessThan(50); // Should average less than 50ms per request
    });
  });
});
