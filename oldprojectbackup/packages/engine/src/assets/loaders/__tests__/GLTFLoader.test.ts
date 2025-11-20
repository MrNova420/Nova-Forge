/**
 * Tests for GLTFLoader
 */

import { describe, it, expect, beforeEach } from '@jest/globals';
import { GLTFLoader } from '../GLTFLoader';

describe('GLTFLoader', () => {
  let loader: GLTFLoader;

  beforeEach(() => {
    loader = new GLTFLoader();
  });

  describe('supports()', () => {
    it('should support gltf files', () => {
      expect(loader.supports('model.gltf')).toBe(true);
      expect(loader.supports('path/to/model.gltf')).toBe(true);
      expect(loader.supports('MODEL.GLTF')).toBe(true);
    });

    it('should support glb files', () => {
      expect(loader.supports('model.glb')).toBe(true);
      expect(loader.supports('path/to/model.glb')).toBe(true);
      expect(loader.supports('MODEL.GLB')).toBe(true);
    });

    it('should not support unsupported formats', () => {
      expect(loader.supports('model.obj')).toBe(false);
      expect(loader.supports('file.txt')).toBe(false);
      expect(loader.supports('image.png')).toBe(false);
    });
  });

  describe('setContext()', () => {
    it('should have setContext method', () => {
      expect(typeof loader.setContext).toBe('function');
    });

    it('should throw error when loading without context', async () => {
      await expect(loader.load('model.gltf')).rejects.toThrow(
        'WebGL context not set'
      );
    });
  });

  describe('load()', () => {
    it('should have load method', () => {
      expect(typeof loader.load).toBe('function');
    });

    it('should return a promise that rejects without context', async () => {
      await expect(loader.load('model.gltf')).rejects.toThrow();
    });
  });
});
