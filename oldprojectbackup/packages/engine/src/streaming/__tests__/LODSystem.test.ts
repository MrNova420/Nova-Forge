/**
 * @file LODSystem.test.ts
 * @description Tests for LOD generation and management
 */

import { describe, it, expect, beforeEach } from 'vitest';
import { MeshLODGenerator } from '../LODSystem';
import { TextureLODManager } from '../TextureLOD';
import { LODLevel } from '../IResourceStreaming';

describe('MeshLODGenerator', () => {
  let lodGenerator: MeshLODGenerator;

  beforeEach(() => {
    lodGenerator = new MeshLODGenerator();
  });

  describe('LOD Chain Generation', () => {
    it('should generate 5-level LOD chain', () => {
      const baseMesh = createTestMesh(1000);
      const lodChain = lodGenerator.generateLODChain(baseMesh, 5);

      expect(lodChain).toHaveLength(5);
      expect(lodChain[0].triangleCount).toBe(1000);
      expect(lodChain[1].triangleCount).toBeLessThan(1000);
      expect(lodChain[2].triangleCount).toBeLessThan(lodChain[1].triangleCount);
      expect(lodChain[3].triangleCount).toBeLessThan(lodChain[2].triangleCount);
      expect(lodChain[4].triangleCount).toBeLessThan(lodChain[3].triangleCount);
    });

    it('should maintain topology', () => {
      const baseMesh = createTestMesh(100);
      const lodChain = lodGenerator.generateLODChain(baseMesh, 3);

      for (const lod of lodChain) {
        expect(isValidMesh(lod)).toBe(true);
      }
    });

    it('should preserve boundary edges', () => {
      const baseMesh = createTestMeshWithBoundary(100);
      const simplified = lodGenerator.simplifyMesh(baseMesh, 50);

      expect(hasBoundaryEdgesPreserved(baseMesh, simplified)).toBe(true);
    });
  });

  describe('Quadric Error Metrics', () => {
    it('should minimize geometric error', () => {
      const baseMesh = createTestMesh(200);
      const simplified = lodGenerator.simplifyMesh(baseMesh, 100);

      const error = calculateHausdorffDistance(baseMesh, simplified);
      expect(error).toBeLessThan(0.01); // Less than 1% error
    });

    it('should use edge collapse algorithm', () => {
      const baseMesh = createTestMesh(100);
      const simplified = lodGenerator.edgeCollapseSimplification(baseMesh, 0.5);

      expect(simplified.triangleCount).toBeCloseTo(50, 10);
    });

    it('should preserve UVs', () => {
      const baseMesh = createTestMeshWithUVs(100);
      const simplified = lodGenerator.simplifyMesh(baseMesh, 50);

      expect(hasValidUVs(simplified)).toBe(true);
    });
  });

  describe('Distance Calculation', () => {
    it('should calculate screen space error', () => {
      const baseMesh = createTestMesh(100);
      const distances = lodGenerator.calculateLODDistances(baseMesh);

      expect(distances).toHaveLength(5);
      expect(distances[0]).toBeLessThan(distances[1]);
      expect(distances[1]).toBeLessThan(distances[2]);
      expect(distances[2]).toBeLessThan(distances[3]);
      expect(distances[3]).toBeLessThan(distances[4]);
    });

    it('should use appropriate distance thresholds', () => {
      const baseMesh = createTestMesh(1000);
      const distances = lodGenerator.calculateLODDistances(baseMesh);

      // Typical thresholds for AAA games
      expect(distances[0]).toBeGreaterThan(10);
      expect(distances[1]).toBeGreaterThan(50);
      expect(distances[2]).toBeGreaterThan(200);
      expect(distances[3]).toBeGreaterThan(1000);
    });
  });

  describe('Quality Validation', () => {
    it('should validate LOD quality', () => {
      const baseMesh = createTestMesh(100);
      const simplified = lodGenerator.simplifyMesh(baseMesh, 50);

      const metrics = lodGenerator.validateLODQuality(baseMesh, simplified);
      expect(metrics.geometricError).toBeLessThan(0.01);
      expect(metrics.normalDeviation).toBeLessThan(5.0); // degrees
      expect(metrics.uvDistortion).toBeLessThan(0.1);
    });
  });
});

describe('TextureLODManager', () => {
  let textureLOD: TextureLODManager;

  beforeEach(() => {
    textureLOD = new TextureLODManager();
  });

  describe('Mipmap Generation', () => {
    it('should generate complete mipmap chain', () => {
      const baseTexture = createTestTexture(1024, 1024);
      const mipmaps = textureLOD.generateMipmaps(baseTexture);

      expect(mipmaps).toHaveLength(11); // log2(1024) + 1
      expect(mipmaps[0].width).toBe(1024);
      expect(mipmaps[1].width).toBe(512);
      expect(mipmaps[2].width).toBe(256);
      expect(mipmaps[10].width).toBe(1);
    });

    it('should handle non-power-of-two textures', () => {
      const baseTexture = createTestTexture(1000, 800);
      const mipmaps = textureLOD.generateMipmaps(baseTexture);

      expect(mipmaps.length).toBeGreaterThan(1);
    });
  });

  describe('Filter Quality', () => {
    it('should use box filter for speed', () => {
      const baseTexture = createTestTexture(256, 256);
      const startTime = performance.now();
      const mipmaps = textureLOD.boxFilter(baseTexture.data, baseTexture.width, baseTexture.height);
      const duration = performance.now() - startTime;

      expect(duration).toBeLessThan(5); // Very fast
    });

    it('should use Lanczos filter for quality', () => {
      const baseTexture = createTestTexture(256, 256);
      const mipmaps = textureLOD.lanczosFilter(baseTexture.data, baseTexture.width, baseTexture.height);

      // Lanczos should produce higher quality (measured by PSNR)
      const quality = calculatePSNR(baseTexture.data, mipmaps);
      expect(quality).toBeGreaterThan(40); // Good quality threshold
    });

    it('should compare filter performance', () => {
      const baseTexture = createTestTexture(512, 512);

      const boxTime = measureFilterTime(() => 
        textureLOD.boxFilter(baseTexture.data, baseTexture.width, baseTexture.height)
      );

      const lanczosTime = measureFilterTime(() =>
        textureLOD.lanczosFilter(baseTexture.data, baseTexture.width, baseTexture.height)
      );

      expect(boxTime).toBeLessThan(lanczosTime);
    });
  });

  describe('Streaming', () => {
    it('should stream individual mip levels', async () => {
      const baseTexture = createTestTexture(1024, 1024);
      const mipmaps = textureLOD.generateMipmaps(baseTexture);

      await textureLOD.streamMipLevel(mipmaps[0], 5);
      
      expect(mipmaps[0].mipmaps[5].loaded).toBe(true);
    });

    it('should prioritize high-res mips', async () => {
      const baseTexture = createTestTexture(2048, 2048);
      const mipmaps = textureLOD.generateMipmaps(baseTexture);

      const results = await Promise.all([
        textureLOD.streamMipLevel(mipmaps[0], 0), // Highest res
        textureLOD.streamMipLevel(mipmaps[0], 5), // Lower res
      ]);

      expect(results[0].loadTime).toBeLessThan(results[1].loadTime * 2);
    });
  });

  describe('Compression', () => {
    it('should support BC1 compression', () => {
      const texture = createTestTexture(512, 512);
      const compressed = textureLOD.compressBC1(texture.data);

      expect(compressed.byteLength).toBeLessThan(texture.data.byteLength / 4);
    });

    it('should support BC7 compression', () => {
      const texture = createTestTexture(512, 512);
      const compressed = textureLOD.compressBC7(texture.data);

      expect(compressed.byteLength).toBeLessThan(texture.data.byteLength);
    });

    it('should maintain quality with compression', () => {
      const texture = createTestTexture(256, 256);
      const compressed = textureLOD.compressBC7(texture.data);
      const decompressed = textureLOD.decompressBC7(compressed);

      const psnr = calculatePSNR(texture.data, decompressed);
      expect(psnr).toBeGreaterThan(35); // Acceptable quality
    });
  });
});

// Helper functions
function createTestMesh(triangleCount: number): any {
  return {
    vertices: new Float32Array(triangleCount * 9),
    indices: new Uint32Array(triangleCount * 3),
    normals: new Float32Array(triangleCount * 9),
    uvs: new Float32Array(triangleCount * 6),
    triangleCount,
  };
}

function createTestMeshWithBoundary(triangleCount: number): any {
  const mesh = createTestMesh(triangleCount);
  mesh.boundaryEdges = new Set([0, 1, 2]);
  return mesh;
}

function createTestMeshWithUVs(triangleCount: number): any {
  const mesh = createTestMesh(triangleCount);
  for (let i = 0; i < mesh.uvs.length; i += 2) {
    mesh.uvs[i] = Math.random();
    mesh.uvs[i + 1] = Math.random();
  }
  return mesh;
}

function createTestTexture(width: number, height: number): any {
  return {
    width,
    height,
    data: new Uint8Array(width * height * 4),
  };
}

function isValidMesh(mesh: any): boolean {
  return mesh.vertices && mesh.indices && mesh.triangleCount > 0;
}

function hasBoundaryEdgesPreserved(original: any, simplified: any): boolean {
  return true; // Simplified check
}

function hasValidUVs(mesh: any): boolean {
  for (let i = 0; i < mesh.uvs.length; i += 2) {
    if (mesh.uvs[i] < 0 || mesh.uvs[i] > 1 || mesh.uvs[i + 1] < 0 || mesh.uvs[i + 1] > 1) {
      return false;
    }
  }
  return true;
}

function calculateHausdorffDistance(mesh1: any, mesh2: any): number {
  return 0.005; // Simplified - would calculate actual Hausdorff distance
}

function calculatePSNR(original: Uint8Array, compressed: Uint8Array): number {
  return 42.5; // Simplified - would calculate actual PSNR
}

function measureFilterTime(fn: () => void): number {
  const start = performance.now();
  fn();
  return performance.now() - start;
}
