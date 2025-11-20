/**
 * Tests for TextureAssetLoader
 */

import { describe, it, expect, beforeEach } from '@jest/globals';
import { TextureAssetLoader } from '../TextureAssetLoader';

describe('TextureAssetLoader', () => {
  let loader: TextureAssetLoader;

  beforeEach(() => {
    loader = new TextureAssetLoader();
  });

  describe('supports()', () => {
    it('should support png files', () => {
      expect(loader.supports('texture.png')).toBe(true);
      expect(loader.supports('path/to/texture.png')).toBe(true);
      expect(loader.supports('TEXTURE.PNG')).toBe(true);
    });

    it('should support jpg files', () => {
      expect(loader.supports('photo.jpg')).toBe(true);
      expect(loader.supports('photo.jpeg')).toBe(true);
      expect(loader.supports('PHOTO.JPG')).toBe(true);
    });

    it('should support gif files', () => {
      expect(loader.supports('animation.gif')).toBe(true);
    });

    it('should support webp files', () => {
      expect(loader.supports('image.webp')).toBe(true);
    });

    it('should support bmp files', () => {
      expect(loader.supports('bitmap.bmp')).toBe(true);
    });

    it('should not support unsupported formats', () => {
      expect(loader.supports('model.obj')).toBe(false);
      expect(loader.supports('audio.mp3')).toBe(false);
      expect(loader.supports('file.txt')).toBe(false);
    });
  });

  describe('setContext()', () => {
    it('should have setContext method', () => {
      expect(typeof loader.setContext).toBe('function');
    });

    it('should throw error when loading without context', async () => {
      await expect(loader.load('texture.png')).rejects.toThrow(
        'WebGL context not set'
      );
    });
  });

  describe('load()', () => {
    it('should have load method', () => {
      expect(typeof loader.load).toBe('function');
    });

    it('should return a promise that rejects without context', async () => {
      await expect(loader.load('texture.png')).rejects.toThrow();
    });
  });
});
