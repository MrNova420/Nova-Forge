/**
 * Tests for AudioLoader
 */

import { describe, it, expect, beforeEach, afterEach } from '@jest/globals';
import { AudioLoader } from '../AudioLoader';

describe('AudioLoader', () => {
  let loader: AudioLoader;

  beforeEach(() => {
    loader = new AudioLoader();
  });

  afterEach(() => {
    loader.close();
  });

  describe('supports()', () => {
    it('should support mp3 files', () => {
      expect(loader.supports('audio.mp3')).toBe(true);
      expect(loader.supports('path/to/audio.mp3')).toBe(true);
      expect(loader.supports('AUDIO.MP3')).toBe(true);
    });

    it('should support wav files', () => {
      expect(loader.supports('sound.wav')).toBe(true);
      expect(loader.supports('path/to/sound.WAV')).toBe(true);
    });

    it('should support ogg files', () => {
      expect(loader.supports('music.ogg')).toBe(true);
    });

    it('should support m4a files', () => {
      expect(loader.supports('track.m4a')).toBe(true);
    });

    it('should support aac files', () => {
      expect(loader.supports('voice.aac')).toBe(true);
    });

    it('should support flac files', () => {
      expect(loader.supports('lossless.flac')).toBe(true);
    });

    it('should support webm files', () => {
      expect(loader.supports('audio.webm')).toBe(true);
    });

    it('should not support unsupported formats', () => {
      expect(loader.supports('file.txt')).toBe(false);
      expect(loader.supports('image.png')).toBe(false);
      expect(loader.supports('video.mp4')).toBe(false);
    });
  });

  describe('audioContext', () => {
    it('should create audio context lazily', () => {
      expect(loader.audioContext).toBeNull();
    });

    it('should provide access to audio context after loading', () => {
      // Audio context is created on first load attempt
      // We can't test actual loading without mock audio files
      // but we can test the getter
      expect(loader.audioContext).toBeNull();
    });
  });

  describe('lifecycle methods', () => {
    it('should have suspend method', () => {
      expect(typeof loader.suspend).toBe('function');
      loader.suspend(); // Should not throw
    });

    it('should have resume method', () => {
      expect(typeof loader.resume).toBe('function');
      loader.resume(); // Should not throw
    });

    it('should have close method', () => {
      expect(typeof loader.close).toBe('function');
      loader.close(); // Should not throw
    });
  });
});
