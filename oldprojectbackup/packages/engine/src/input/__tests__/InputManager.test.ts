/**
 * Tests for InputManager
 */

import { describe, it, expect } from '@jest/globals';
import {
  InputManager,
  KeyCode,
  MouseButton,
  GamepadButton,
} from '../InputManager';
import { InputType } from '../InputMapping';

// Mock DOM element for testing
const mockElement = {} as HTMLElement;

describe('InputManager', () => {
  it('should create input manager instance', () => {
    const inputManager = new InputManager({
      element: mockElement,
      element: mockElement,
      enableKeyboard: false,
      enableMouse: false,
      enableTouch: false,
      enableGamepad: false,
    });
    expect(inputManager).toBeDefined();
  });

  it('should have all input systems', () => {
    const inputManager = new InputManager({
      element: mockElement,
      element: mockElement,
      enableKeyboard: false,
      enableMouse: false,
      enableTouch: false,
      enableGamepad: false,
    });
    expect(inputManager.keyboard).toBeDefined();
    expect(inputManager.mouse).toBeDefined();
    expect(inputManager.touch).toBeDefined();
    expect(inputManager.gamepad).toBeDefined();
    expect(inputManager.mapping).toBeDefined();
  });

  it('should have required methods', () => {
    const inputManager = new InputManager({
      element: mockElement,
      enableKeyboard: false,
      enableMouse: false,
      enableTouch: false,
      enableGamepad: false,
    });
    expect(typeof inputManager.update).toBe('function');
    expect(typeof inputManager.destroy).toBe('function');
    expect(typeof inputManager.mapAction).toBe('function');
    expect(typeof inputManager.getAction).toBe('function');
    expect(typeof inputManager.isActionPressed).toBe('function');
    expect(typeof inputManager.isActionReleased).toBe('function');
    expect(typeof inputManager.isActionDown).toBe('function');
    expect(typeof inputManager.loadPreset).toBe('function');
  });

  describe('Action Mapping', () => {
    it('should map actions', () => {
      const inputManager = new InputManager({
        element: mockElement,
        enableKeyboard: false,
        enableMouse: false,
        enableTouch: false,
        enableGamepad: false,
      });

      inputManager.mapAction('jump', {
        type: InputType.Keyboard,
        key: KeyCode.Space,
      });

      expect(inputManager.mapping.hasAction('jump')).toBe(true);
    });

    it('should get action value', () => {
      const inputManager = new InputManager({
        element: mockElement,
        enableKeyboard: false,
        enableMouse: false,
        enableTouch: false,
        enableGamepad: false,
      });

      inputManager.mapAction('jump', {
        type: InputType.Keyboard,
        key: KeyCode.Space,
      });

      expect(inputManager.getAction('jump')).toBe(0);
    });
  });

  describe('Presets', () => {
    it('should load WASD preset', () => {
      const inputManager = new InputManager({
        element: mockElement,
        enableKeyboard: false,
        enableMouse: false,
        enableTouch: false,
        enableGamepad: false,
      });

      expect(() => inputManager.loadPreset('wasd')).not.toThrow();
      expect(inputManager.mapping.hasAction('moveForward')).toBe(true);
    });

    it('should load arrows preset', () => {
      const inputManager = new InputManager({
        element: mockElement,
        enableKeyboard: false,
        enableMouse: false,
        enableTouch: false,
        enableGamepad: false,
      });

      expect(() => inputManager.loadPreset('arrows')).not.toThrow();
      expect(inputManager.mapping.hasAction('moveForward')).toBe(true);
    });

    it('should load gamepad preset', () => {
      const inputManager = new InputManager({
        element: mockElement,
        enableKeyboard: false,
        enableMouse: false,
        enableTouch: false,
        enableGamepad: false,
      });

      expect(() => inputManager.loadPreset('gamepad')).not.toThrow();
      expect(inputManager.mapping.hasAction('jump')).toBe(true);
    });
  });

  describe('Enum exports', () => {
    it('should export KeyCode enum', () => {
      expect(KeyCode.A).toBe('KeyA');
      expect(KeyCode.Space).toBe('Space');
    });

    it('should export MouseButton enum', () => {
      expect(MouseButton.Left).toBe(0);
      expect(MouseButton.Right).toBe(2);
    });

    it('should export GamepadButton enum', () => {
      expect(GamepadButton.A).toBe(0);
      expect(GamepadButton.B).toBe(1);
    });
  });
});
