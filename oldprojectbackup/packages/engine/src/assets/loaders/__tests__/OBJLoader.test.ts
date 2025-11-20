/**
 * Tests for OBJLoader
 */

import { describe, it, expect, beforeEach } from '@jest/globals';
import { OBJLoader } from '../OBJLoader';

describe('OBJLoader', () => {
  let loader: OBJLoader;

  beforeEach(() => {
    loader = new OBJLoader();
  });

  describe('supports()', () => {
    it('should support obj files', () => {
      expect(loader.supports('model.obj')).toBe(true);
      expect(loader.supports('path/to/model.obj')).toBe(true);
      expect(loader.supports('MODEL.OBJ')).toBe(true);
    });

    it('should not support unsupported formats', () => {
      expect(loader.supports('model.gltf')).toBe(false);
      expect(loader.supports('model.glb')).toBe(false);
      expect(loader.supports('file.txt')).toBe(false);
      expect(loader.supports('image.png')).toBe(false);
    });
  });

  describe('setContext()', () => {
    it('should have setContext method', () => {
      expect(typeof loader.setContext).toBe('function');
    });

    it('should throw error when loading without context', async () => {
      await expect(loader.load('model.obj')).rejects.toThrow(
        'WebGL context not set'
      );
    });
  });

  describe('load()', () => {
    it('should have load method', () => {
      expect(typeof loader.load).toBe('function');
    });

    it('should return a promise that rejects without context', async () => {
      await expect(loader.load('model.obj')).rejects.toThrow();
    });
  });
});
