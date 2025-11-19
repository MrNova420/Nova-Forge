/**
 * NOVA ENGINE - Proprietary Software
 * 
 * Copyright (c) 2025 Kayden Shawn Massengill. All Rights Reserved.
 * Operating as: WeNova Interactive
 * 
 * This software is proprietary and confidential. Unauthorized copying,
 * modification, distribution, or use of this software, via any medium,
 * is strictly prohibited without prior written permission.
 * 
 * See LICENSE file in the root directory for full license terms.
 */

/**
 * InputManager
 *
 * Central manager for all input systems (keyboard, mouse, touch, gamepad).
 * Provides unified access to input state and action mapping.
 */

import { Keyboard, KeyCode } from './Keyboard';
import { Mouse, MouseButton } from './Mouse';
import { Touch, TouchPoint } from './Touch';
import { Gamepad, GamepadButton, GamepadAxis } from './Gamepad';
import { InputMapping, InputType, InputBinding } from './InputMapping';

/**
 * Input manager options
 */
export interface InputManagerOptions {
  element?: HTMLElement;
  enableKeyboard?: boolean;
  enableMouse?: boolean;
  enableTouch?: boolean;
  enableGamepad?: boolean;
}

/**
 * Central input management system
 */
export class InputManager {
  private _keyboard: Keyboard;
  private _mouse: Mouse;
  private _touch: Touch;
  private _gamepad: Gamepad;
  private _mapping: InputMapping;

  /**
   * Create a new input manager
   * @param options - Input manager options
   */
  constructor(options: InputManagerOptions = {}) {
    const {
      element = document.body,
      enableKeyboard = true,
      enableMouse = true,
      enableTouch = true,
      enableGamepad = true,
    } = options;

    // Create input systems
    this._keyboard = new Keyboard();
    this._mouse = new Mouse();
    this._touch = new Touch();
    this._gamepad = new Gamepad();
    this._mapping = new InputMapping();

    // Initialize enabled systems
    if (enableKeyboard) {
      this._keyboard.initialize();
    }

    if (enableMouse) {
      this._mouse.initialize(element);
    }

    if (enableTouch) {
      this._touch.initialize(element);
    }

    if (enableGamepad) {
      this._gamepad.initialize();
    }
  }

  /**
   * Update input state (call once per frame)
   */
  update(): void {
    this._keyboard.update();
    this._mouse.update();
    this._touch.update();
    this._gamepad.update();

    // Update action mappings
    this.updateActionMappings();
  }

  /**
   * Cleanup input manager
   */
  destroy(): void {
    this._keyboard.destroy();
    this._mouse.destroy();
    this._touch.destroy();
    this._gamepad.destroy();
  }

  /**
   * Get keyboard system
   */
  get keyboard(): Keyboard {
    return this._keyboard;
  }

  /**
   * Get mouse system
   */
  get mouse(): Mouse {
    return this._mouse;
  }

  /**
   * Get touch system
   */
  get touch(): Touch {
    return this._touch;
  }

  /**
   * Get gamepad system
   */
  get gamepad(): Gamepad {
    return this._gamepad;
  }

  /**
   * Get input mapping system
   */
  get mapping(): InputMapping {
    return this._mapping;
  }

  /**
   * Map an action to an input
   * @param action - Action name
   * @param binding - Input binding
   */
  mapAction(action: string, binding: InputBinding): void {
    this._mapping.mapAction(action, binding);
  }

  /**
   * Get action value
   * @param action - Action name
   */
  getAction(action: string): number {
    return this._mapping.getActionValue(action);
  }

  /**
   * Check if action was pressed this frame
   * @param action - Action name
   */
  isActionPressed(action: string): boolean {
    return this._mapping.isActionPressed(action);
  }

  /**
   * Check if action was released this frame
   * @param action - Action name
   */
  isActionReleased(action: string): boolean {
    return this._mapping.isActionReleased(action);
  }

  /**
   * Check if action is currently held down
   * @param action - Action name
   */
  isActionDown(action: string): boolean {
    return this._mapping.isActionDown(action);
  }

  /**
   * Update action mapping states based on current input
   */
  private updateActionMappings(): void {
    const actions = this._mapping.getAllActions();

    for (const action of actions) {
      const bindings = this._mapping.getBindings(action);

      let value = 0;
      let pressed = false;
      let released = false;
      let down = false;

      // Check each binding for this action
      for (const binding of bindings) {
        switch (binding.type) {
          case InputType.Keyboard:
            if (binding.key) {
              if (this._keyboard.isKeyPressed(binding.key)) {
                pressed = true;
              }
              if (this._keyboard.isKeyReleased(binding.key)) {
                released = true;
              }
              if (this._keyboard.isKeyDown(binding.key)) {
                down = true;
                value = 1;
              }
            }
            break;

          case InputType.Mouse:
            if (binding.button !== undefined) {
              const mouseButton = binding.button as MouseButton;
              if (this._mouse.isButtonPressed(mouseButton)) {
                pressed = true;
              }
              if (this._mouse.isButtonReleased(mouseButton)) {
                released = true;
              }
              if (this._mouse.isButtonDown(mouseButton)) {
                down = true;
                value = 1;
              }
            }
            break;

          case InputType.Gamepad:
            if (binding.button !== undefined) {
              const gamepadIndex = binding.gamepadIndex ?? 0;
              const gamepadButton = binding.button as GamepadButton;
              if (this._gamepad.isButtonPressed(gamepadButton, gamepadIndex)) {
                pressed = true;
              }
              if (this._gamepad.isButtonReleased(gamepadButton, gamepadIndex)) {
                released = true;
              }
              if (this._gamepad.isButtonDown(gamepadButton, gamepadIndex)) {
                down = true;
                value = 1;
              }
            }
            break;
        }
      }

      this._mapping.updateActionState(action, value, pressed, released, down);
    }
  }

  /**
   * Load a preset input scheme
   * @param preset - Preset name
   */
  loadPreset(preset: 'wasd' | 'arrows' | 'gamepad'): void {
    this._mapping.loadPreset(preset);
  }
}

// Re-export types for convenience
export { KeyCode, MouseButton, GamepadButton, GamepadAxis };
export type { TouchPoint, InputBinding };
