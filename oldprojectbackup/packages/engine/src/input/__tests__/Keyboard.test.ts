/**
 * Tests for Keyboard
 */

import { describe, it, expect } from '@jest/globals';
import { Keyboard, KeyCode } from '../Keyboard';

describe('Keyboard', () => {
  it('should create keyboard instance', () => {
    const keyboard = new Keyboard();
    expect(keyboard).toBeDefined();
  });

  it('should have KeyCode enum', () => {
    expect(KeyCode.A).toBe('KeyA');
    expect(KeyCode.Space).toBe('Space');
    expect(KeyCode.Enter).toBe('Enter');
    expect(KeyCode.ArrowUp).toBe('ArrowUp');
  });

  it('should have required methods', () => {
    const keyboard = new Keyboard();
    expect(typeof keyboard.initialize).toBe('function');
    expect(typeof keyboard.destroy).toBe('function');
    expect(typeof keyboard.update).toBe('function');
    expect(typeof keyboard.isKeyDown).toBe('function');
    expect(typeof keyboard.isKeyPressed).toBe('function');
    expect(typeof keyboard.isKeyReleased).toBe('function');
    expect(typeof keyboard.isAnyKeyDown).toBe('function');
    expect(typeof keyboard.getKeysDown).toBe('function');
    expect(typeof keyboard.setEnabled).toBe('function');
  });

  it('should be enabled by default', () => {
    const keyboard = new Keyboard();
    expect(keyboard.enabled).toBe(true);
  });

  it('should allow disabling', () => {
    const keyboard = new Keyboard();
    keyboard.setEnabled(false);
    expect(keyboard.enabled).toBe(false);
  });
});
