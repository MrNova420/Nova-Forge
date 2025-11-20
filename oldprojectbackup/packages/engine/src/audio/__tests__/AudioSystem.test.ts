/**
 * Tests for AudioSystem
 */

import { describe, it, expect } from '@jest/globals';
import { AudioSystem } from '../AudioSystem';

describe('AudioSystem', () => {
  it('should create audio system instance', () => {
    const audioSystem = new AudioSystem();
    expect(audioSystem).toBeDefined();
  });

  it('should have required methods', () => {
    const audioSystem = new AudioSystem();
    expect(typeof audioSystem.initialize).toBe('function');
    expect(typeof audioSystem.update).toBe('function');
    expect(typeof audioSystem.suspend).toBe('function');
    expect(typeof audioSystem.resume).toBe('function');
    expect(typeof audioSystem.destroy).toBe('function');
    expect(typeof audioSystem.createSource).toBe('function');
    expect(typeof audioSystem.setMasterVolume).toBe('function');
    expect(typeof audioSystem.mute).toBe('function');
    expect(typeof audioSystem.unmute).toBe('function');
  });

  it('should have properties', () => {
    const audioSystem = new AudioSystem();
    expect(audioSystem.masterVolume).toBeDefined();
    expect(audioSystem.isMuted).toBeDefined();
    expect(audioSystem.activeSourceCount).toBeDefined();
  });

  it('should start with default master volume', () => {
    const audioSystem = new AudioSystem();
    expect(audioSystem.masterVolume).toBe(1.0);
  });

  it('should start unmuted', () => {
    const audioSystem = new AudioSystem();
    expect(audioSystem.isMuted).toBe(false);
  });

  it('should have getSources method', () => {
    const audioSystem = new AudioSystem();
    expect(typeof audioSystem.getSources).toBe('function');
  });
});
