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
 * Gamepad
 *
 * Manages gamepad/controller input.
 */

/**
 * Standard gamepad button indices
 */
export enum GamepadButton {
  A = 0, // Cross on PlayStation
  B = 1, // Circle on PlayStation
  X = 2, // Square on PlayStation
  Y = 3, // Triangle on PlayStation
  LeftBumper = 4,
  RightBumper = 5,
  LeftTrigger = 6,
  RightTrigger = 7,
  Back = 8, // Select
  Start = 9,
  LeftStick = 10,
  RightStick = 11,
  DPadUp = 12,
  DPadDown = 13,
  DPadLeft = 14,
  DPadRight = 15,
  Home = 16,
}

/**
 * Standard gamepad axes
 */
export enum GamepadAxis {
  LeftStickX = 0,
  LeftStickY = 1,
  RightStickX = 2,
  RightStickY = 3,
}

/**
 * Gamepad state
 */
interface GamepadState {
  index: number;
  id: string;
  connected: boolean;
  buttons: boolean[];
  buttonsPressed: boolean[];
  buttonsReleased: boolean[];
  axes: number[];
  timestamp: number;
}

/**
 * Gamepad input manager
 */
export class Gamepad {
  private _gamepads: Map<number, GamepadState> = new Map();
  private _enabled: boolean = true;
  private _deadzone: number = 0.1; // Analog stick deadzone

  /**
   * Initialize gamepad input
   */
  initialize(): void {
    window.addEventListener('gamepadconnected', this.handleGamepadConnected);
    window.addEventListener(
      'gamepaddisconnected',
      this.handleGamepadDisconnected
    );

    // Poll for already connected gamepads
    this.pollGamepads();
  }

  /**
   * Cleanup gamepad input
   */
  destroy(): void {
    window.removeEventListener('gamepadconnected', this.handleGamepadConnected);
    window.removeEventListener(
      'gamepaddisconnected',
      this.handleGamepadDisconnected
    );
    this._gamepads.clear();
  }

  /**
   * Update gamepad state (call once per frame)
   */
  update(): void {
    if (!this._enabled) return;

    // Poll gamepad state
    const browserGamepads = navigator.getGamepads();

    for (const browserGamepad of browserGamepads) {
      if (!browserGamepad) continue;

      const state = this._gamepads.get(browserGamepad.index);
      if (!state) continue;

      // Store previous button states
      const previousButtons = [...state.buttons];

      // Update button states
      state.buttons = browserGamepad.buttons.map((button) => button.pressed);

      // Detect pressed/released
      state.buttonsPressed = state.buttons.map(
        (current, i) => current && !previousButtons[i]!
      );
      state.buttonsReleased = state.buttons.map(
        (current, i) => !current && previousButtons[i]!
      );

      // Update axes with deadzone
      state.axes = browserGamepad.axes.map((value) =>
        Math.abs(value) < this._deadzone ? 0 : value
      );

      state.timestamp = browserGamepad.timestamp;
    }
  }

  /**
   * Check if a gamepad is connected
   * @param index - Gamepad index (0-3)
   */
  isConnected(index: number = 0): boolean {
    return this._gamepads.get(index)?.connected ?? false;
  }

  /**
   * Get number of connected gamepads
   */
  get connectedCount(): number {
    return Array.from(this._gamepads.values()).filter((g) => g.connected)
      .length;
  }

  /**
   * Check if a button is currently held down
   * @param button - Button index
   * @param index - Gamepad index (0-3)
   */
  isButtonDown(button: GamepadButton, index: number = 0): boolean {
    if (!this._enabled) return false;
    const state = this._gamepads.get(index);
    return state?.buttons[button] ?? false;
  }

  /**
   * Check if a button was pressed this frame
   * @param button - Button index
   * @param index - Gamepad index (0-3)
   */
  isButtonPressed(button: GamepadButton, index: number = 0): boolean {
    if (!this._enabled) return false;
    const state = this._gamepads.get(index);
    return state?.buttonsPressed[button] ?? false;
  }

  /**
   * Check if a button was released this frame
   * @param button - Button index
   * @param index - Gamepad index (0-3)
   */
  isButtonReleased(button: GamepadButton, index: number = 0): boolean {
    if (!this._enabled) return false;
    const state = this._gamepads.get(index);
    return state?.buttonsReleased[button] ?? false;
  }

  /**
   * Get axis value
   * @param axis - Axis index
   * @param index - Gamepad index (0-3)
   * @returns Value between -1 and 1
   */
  getAxis(axis: GamepadAxis, index: number = 0): number {
    if (!this._enabled) return 0;
    const state = this._gamepads.get(index);
    return state?.axes[axis] ?? 0;
  }

  /**
   * Get left stick X axis
   * @param index - Gamepad index (0-3)
   */
  getLeftStickX(index: number = 0): number {
    return this.getAxis(GamepadAxis.LeftStickX, index);
  }

  /**
   * Get left stick Y axis
   * @param index - Gamepad index (0-3)
   */
  getLeftStickY(index: number = 0): number {
    return this.getAxis(GamepadAxis.LeftStickY, index);
  }

  /**
   * Get right stick X axis
   * @param index - Gamepad index (0-3)
   */
  getRightStickX(index: number = 0): number {
    return this.getAxis(GamepadAxis.RightStickX, index);
  }

  /**
   * Get right stick Y axis
   * @param index - Gamepad index (0-3)
   */
  getRightStickY(index: number = 0): number {
    return this.getAxis(GamepadAxis.RightStickY, index);
  }

  /**
   * Set analog stick deadzone
   * @param deadzone - Deadzone value (0-1)
   */
  setDeadzone(deadzone: number): void {
    this._deadzone = Math.max(0, Math.min(1, deadzone));
  }

  /**
   * Get current deadzone
   */
  get deadzone(): number {
    return this._deadzone;
  }

  /**
   * Enable or disable gamepad input
   * @param enabled - Whether gamepad input should be enabled
   */
  setEnabled(enabled: boolean): void {
    this._enabled = enabled;
  }

  /**
   * Check if gamepad input is enabled
   */
  get enabled(): boolean {
    return this._enabled;
  }

  /**
   * Get all connected gamepad IDs
   */
  getConnectedGamepads(): Array<{ index: number; id: string }> {
    return Array.from(this._gamepads.values())
      .filter((g) => g.connected)
      .map((g) => ({ index: g.index, id: g.id }));
  }

  /**
   * Poll for connected gamepads
   */
  private pollGamepads(): void {
    const browserGamepads = navigator.getGamepads();

    for (const gamepad of browserGamepads) {
      if (gamepad && !this._gamepads.has(gamepad.index)) {
        this.addGamepad(gamepad);
      }
    }
  }

  /**
   * Add a gamepad
   */
  private addGamepad(gamepad: globalThis.Gamepad): void {
    const state: GamepadState = {
      index: gamepad.index,
      id: gamepad.id,
      connected: true,
      buttons: gamepad.buttons.map((button) => button.pressed),
      buttonsPressed: new Array(gamepad.buttons.length).fill(false),
      buttonsReleased: new Array(gamepad.buttons.length).fill(false),
      axes: gamepad.axes.map((value) =>
        Math.abs(value) < this._deadzone ? 0 : value
      ),
      timestamp: gamepad.timestamp,
    };

    this._gamepads.set(gamepad.index, state);
  }

  /**
   * Handle gamepad connected event
   */
  private handleGamepadConnected = (event: GamepadEvent): void => {
    this.addGamepad(event.gamepad);
  };

  /**
   * Handle gamepad disconnected event
   */
  private handleGamepadDisconnected = (event: GamepadEvent): void => {
    const state = this._gamepads.get(event.gamepad.index);
    if (state) {
      state.connected = false;
    }
  };
}
