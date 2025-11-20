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
 * Keyboard
 *
 * Manages keyboard input state and events.
 */

/**
 * Keyboard key codes
 */
export enum KeyCode {
  // Letters
  A = 'KeyA',
  B = 'KeyB',
  C = 'KeyC',
  D = 'KeyD',
  E = 'KeyE',
  F = 'KeyF',
  G = 'KeyG',
  H = 'KeyH',
  I = 'KeyI',
  J = 'KeyJ',
  K = 'KeyK',
  L = 'KeyL',
  M = 'KeyM',
  N = 'KeyN',
  O = 'KeyO',
  P = 'KeyP',
  Q = 'KeyQ',
  R = 'KeyR',
  S = 'KeyS',
  T = 'KeyT',
  U = 'KeyU',
  V = 'KeyV',
  W = 'KeyW',
  X = 'KeyX',
  Y = 'KeyY',
  Z = 'KeyZ',

  // Numbers
  Digit0 = 'Digit0',
  Digit1 = 'Digit1',
  Digit2 = 'Digit2',
  Digit3 = 'Digit3',
  Digit4 = 'Digit4',
  Digit5 = 'Digit5',
  Digit6 = 'Digit6',
  Digit7 = 'Digit7',
  Digit8 = 'Digit8',
  Digit9 = 'Digit9',

  // Function keys
  F1 = 'F1',
  F2 = 'F2',
  F3 = 'F3',
  F4 = 'F4',
  F5 = 'F5',
  F6 = 'F6',
  F7 = 'F7',
  F8 = 'F8',
  F9 = 'F9',
  F10 = 'F10',
  F11 = 'F11',
  F12 = 'F12',

  // Control keys
  Enter = 'Enter',
  Escape = 'Escape',
  Backspace = 'Backspace',
  Tab = 'Tab',
  Space = 'Space',
  Shift = 'ShiftLeft',
  ShiftRight = 'ShiftRight',
  Control = 'ControlLeft',
  ControlRight = 'ControlRight',
  Alt = 'AltLeft',
  AltRight = 'AltRight',
  Meta = 'MetaLeft',
  MetaRight = 'MetaRight',

  // Arrow keys
  ArrowLeft = 'ArrowLeft',
  ArrowRight = 'ArrowRight',
  ArrowUp = 'ArrowUp',
  ArrowDown = 'ArrowDown',

  // Special keys
  CapsLock = 'CapsLock',
  Delete = 'Delete',
  End = 'End',
  Home = 'Home',
  Insert = 'Insert',
  PageDown = 'PageDown',
  PageUp = 'PageUp',
}

/**
 * Keyboard input manager
 */
export class Keyboard {
  private _keysDown: Set<string> = new Set();
  private _keysPressed: Set<string> = new Set();
  private _keysReleased: Set<string> = new Set();
  private _enabled: boolean = true;

  /**
   * Initialize keyboard input
   */
  initialize(): void {
    window.addEventListener('keydown', this.handleKeyDown);
    window.addEventListener('keyup', this.handleKeyUp);
    window.addEventListener('blur', this.handleBlur);
  }

  /**
   * Cleanup keyboard input
   */
  destroy(): void {
    window.removeEventListener('keydown', this.handleKeyDown);
    window.removeEventListener('keyup', this.handleKeyUp);
    window.removeEventListener('blur', this.handleBlur);
    this._keysDown.clear();
    this._keysPressed.clear();
    this._keysReleased.clear();
  }

  /**
   * Update keyboard state (call once per frame)
   */
  update(): void {
    // Clear frame-specific state
    this._keysPressed.clear();
    this._keysReleased.clear();
  }

  /**
   * Check if a key is currently held down
   * @param key - Key code or string
   */
  isKeyDown(key: KeyCode | string): boolean {
    if (!this._enabled) return false;
    return this._keysDown.has(key as string);
  }

  /**
   * Check if a key was pressed this frame
   * @param key - Key code or string
   */
  isKeyPressed(key: KeyCode | string): boolean {
    if (!this._enabled) return false;
    return this._keysPressed.has(key as string);
  }

  /**
   * Check if a key was released this frame
   * @param key - Key code or string
   */
  isKeyReleased(key: KeyCode | string): boolean {
    if (!this._enabled) return false;
    return this._keysReleased.has(key as string);
  }

  /**
   * Check if any key is currently down
   */
  isAnyKeyDown(): boolean {
    return this._keysDown.size > 0;
  }

  /**
   * Get all keys currently down
   */
  getKeysDown(): string[] {
    return Array.from(this._keysDown);
  }

  /**
   * Enable or disable keyboard input
   * @param enabled - Whether keyboard input should be enabled
   */
  setEnabled(enabled: boolean): void {
    this._enabled = enabled;
    if (!enabled) {
      this._keysDown.clear();
      this._keysPressed.clear();
      this._keysReleased.clear();
    }
  }

  /**
   * Check if keyboard input is enabled
   */
  get enabled(): boolean {
    return this._enabled;
  }

  /**
   * Handle keydown events
   */
  private handleKeyDown = (event: KeyboardEvent): void => {
    if (!this._enabled) return;

    const key = event.code;

    // Track pressed state (only on first press, not repeat)
    if (!this._keysDown.has(key)) {
      this._keysPressed.add(key);
      this._keysDown.add(key);
    }
  };

  /**
   * Handle keyup events
   */
  private handleKeyUp = (event: KeyboardEvent): void => {
    if (!this._enabled) return;

    const key = event.code;

    if (this._keysDown.has(key)) {
      this._keysDown.delete(key);
      this._keysReleased.add(key);
    }
  };

  /**
   * Handle window blur (clear all key states)
   */
  private handleBlur = (): void => {
    this._keysDown.clear();
    this._keysPressed.clear();
    this._keysReleased.clear();
  };
}
