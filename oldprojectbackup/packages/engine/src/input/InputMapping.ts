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
 * InputMapping
 *
 * Maps abstract actions to physical inputs (keyboard, mouse, gamepad).
 * Allows for customizable input schemes and input rebinding.
 */

import { KeyCode } from './Keyboard';
import { MouseButton } from './Mouse';
import { GamepadButton } from './Gamepad';

/**
 * Input binding types
 */
export enum InputType {
  Keyboard = 'keyboard',
  Mouse = 'mouse',
  Gamepad = 'gamepad',
}

/**
 * Input binding
 */
export interface InputBinding {
  type: InputType;
  key?: KeyCode | string;
  button?: MouseButton | GamepadButton;
  gamepadIndex?: number;
}

/**
 * Action state
 */
interface ActionState {
  value: number;
  pressed: boolean;
  released: boolean;
  down: boolean;
}

/**
 * Input mapping system for action-based input
 */
export class InputMapping {
  private _actions: Map<string, InputBinding[]> = new Map();
  private _actionStates: Map<string, ActionState> = new Map();

  /**
   * Map an action to an input
   * @param action - Action name
   * @param binding - Input binding
   */
  mapAction(action: string, binding: InputBinding): void {
    const bindings = this._actions.get(action) || [];
    bindings.push(binding);
    this._actions.set(action, bindings);

    // Initialize action state
    if (!this._actionStates.has(action)) {
      this._actionStates.set(action, {
        value: 0,
        pressed: false,
        released: false,
        down: false,
      });
    }
  }

  /**
   * Remove an action mapping
   * @param action - Action name
   * @param binding - Optional specific binding to remove
   */
  unmapAction(action: string, binding?: InputBinding): void {
    if (!binding) {
      this._actions.delete(action);
      this._actionStates.delete(action);
      return;
    }

    const bindings = this._actions.get(action);
    if (!bindings) return;

    const index = bindings.findIndex(
      (b) =>
        b.type === binding.type &&
        b.key === binding.key &&
        b.button === binding.button &&
        b.gamepadIndex === binding.gamepadIndex
    );

    if (index !== -1) {
      bindings.splice(index, 1);
      if (bindings.length === 0) {
        this._actions.delete(action);
        this._actionStates.delete(action);
      }
    }
  }

  /**
   * Get all bindings for an action
   * @param action - Action name
   */
  getBindings(action: string): InputBinding[] {
    return this._actions.get(action) || [];
  }

  /**
   * Check if an action exists
   * @param action - Action name
   */
  hasAction(action: string): boolean {
    return this._actions.has(action);
  }

  /**
   * Get all action names
   */
  getAllActions(): string[] {
    return Array.from(this._actions.keys());
  }

  /**
   * Update action state (called by InputManager)
   * @param action - Action name
   * @param value - Action value (0-1 for buttons, -1 to 1 for axes)
   * @param pressed - Whether action was pressed this frame
   * @param released - Whether action was released this frame
   * @param down - Whether action is currently held down
   */
  updateActionState(
    action: string,
    value: number,
    pressed: boolean,
    released: boolean,
    down: boolean
  ): void {
    const state = this._actionStates.get(action);
    if (state) {
      state.value = value;
      state.pressed = pressed;
      state.released = released;
      state.down = down;
    }
  }

  /**
   * Get action value (0-1 for buttons, -1 to 1 for axes)
   * @param action - Action name
   */
  getActionValue(action: string): number {
    return this._actionStates.get(action)?.value ?? 0;
  }

  /**
   * Check if action was pressed this frame
   * @param action - Action name
   */
  isActionPressed(action: string): boolean {
    return this._actionStates.get(action)?.pressed ?? false;
  }

  /**
   * Check if action was released this frame
   * @param action - Action name
   */
  isActionReleased(action: string): boolean {
    return this._actionStates.get(action)?.released ?? false;
  }

  /**
   * Check if action is currently held down
   * @param action - Action name
   */
  isActionDown(action: string): boolean {
    return this._actionStates.get(action)?.down ?? false;
  }

  /**
   * Clear all action mappings
   */
  clear(): void {
    this._actions.clear();
    this._actionStates.clear();
  }

  /**
   * Create a preset input scheme
   * @param preset - Preset name
   */
  loadPreset(preset: 'wasd' | 'arrows' | 'gamepad'): void {
    this.clear();

    switch (preset) {
      case 'wasd':
        this.mapAction('moveForward', {
          type: InputType.Keyboard,
          key: KeyCode.W,
        });
        this.mapAction('moveBackward', {
          type: InputType.Keyboard,
          key: KeyCode.S,
        });
        this.mapAction('moveLeft', {
          type: InputType.Keyboard,
          key: KeyCode.A,
        });
        this.mapAction('moveRight', {
          type: InputType.Keyboard,
          key: KeyCode.D,
        });
        this.mapAction('jump', {
          type: InputType.Keyboard,
          key: KeyCode.Space,
        });
        this.mapAction('sprint', {
          type: InputType.Keyboard,
          key: KeyCode.Shift,
        });
        this.mapAction('fire', {
          type: InputType.Mouse,
          button: MouseButton.Left,
        });
        this.mapAction('aim', {
          type: InputType.Mouse,
          button: MouseButton.Right,
        });
        break;

      case 'arrows':
        this.mapAction('moveForward', {
          type: InputType.Keyboard,
          key: KeyCode.ArrowUp,
        });
        this.mapAction('moveBackward', {
          type: InputType.Keyboard,
          key: KeyCode.ArrowDown,
        });
        this.mapAction('moveLeft', {
          type: InputType.Keyboard,
          key: KeyCode.ArrowLeft,
        });
        this.mapAction('moveRight', {
          type: InputType.Keyboard,
          key: KeyCode.ArrowRight,
        });
        this.mapAction('jump', {
          type: InputType.Keyboard,
          key: KeyCode.Space,
        });
        this.mapAction('sprint', {
          type: InputType.Keyboard,
          key: KeyCode.Shift,
        });
        break;

      case 'gamepad':
        this.mapAction('fire', {
          type: InputType.Gamepad,
          button: GamepadButton.A,
        });
        this.mapAction('jump', {
          type: InputType.Gamepad,
          button: GamepadButton.A,
        });
        this.mapAction('aim', {
          type: InputType.Gamepad,
          button: GamepadButton.LeftTrigger,
        });
        this.mapAction('sprint', {
          type: InputType.Gamepad,
          button: GamepadButton.LeftBumper,
        });
        break;
    }
  }
}
