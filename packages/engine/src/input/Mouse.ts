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
 * Mouse
 *
 * Manages mouse input state and events.
 */

import { Vector2 } from '../math/Vector2';

/**
 * Mouse button codes
 */
export enum MouseButton {
  Left = 0,
  Middle = 1,
  Right = 2,
  Back = 3,
  Forward = 4,
}

/**
 * Mouse input manager
 */
export class Mouse {
  private _position: Vector2 = new Vector2(0, 0);
  private _previousPosition: Vector2 = new Vector2(0, 0);
  private _delta: Vector2 = new Vector2(0, 0);
  private _scroll: Vector2 = new Vector2(0, 0);

  private _buttonsDown: Set<number> = new Set();
  private _buttonsPressed: Set<number> = new Set();
  private _buttonsReleased: Set<number> = new Set();

  private _enabled: boolean = true;
  private _locked: boolean = false;
  private _element: HTMLElement | null = null;

  /**
   * Initialize mouse input
   * @param element - DOM element to listen to (defaults to window)
   */
  initialize(element: HTMLElement = document.body): void {
    this._element = element;

    element.addEventListener('mousedown', this.handleMouseDown);
    element.addEventListener('mouseup', this.handleMouseUp);
    element.addEventListener('mousemove', this.handleMouseMove);
    element.addEventListener('wheel', this.handleWheel);
    element.addEventListener('contextmenu', this.handleContextMenu);

    document.addEventListener(
      'pointerlockchange',
      this.handlePointerLockChange
    );
    window.addEventListener('blur', this.handleBlur);
  }

  /**
   * Cleanup mouse input
   */
  destroy(): void {
    if (this._element) {
      this._element.removeEventListener('mousedown', this.handleMouseDown);
      this._element.removeEventListener('mouseup', this.handleMouseUp);
      this._element.removeEventListener('mousemove', this.handleMouseMove);
      this._element.removeEventListener('wheel', this.handleWheel);
      this._element.removeEventListener('contextmenu', this.handleContextMenu);
    }

    document.removeEventListener(
      'pointerlockchange',
      this.handlePointerLockChange
    );
    window.removeEventListener('blur', this.handleBlur);

    if (this._locked) {
      document.exitPointerLock();
    }

    this._buttonsDown.clear();
    this._buttonsPressed.clear();
    this._buttonsReleased.clear();
  }

  /**
   * Update mouse state (call once per frame)
   */
  update(): void {
    // Clear frame-specific state
    this._buttonsPressed.clear();
    this._buttonsReleased.clear();
    this._scroll.set(0, 0);

    // Update delta
    this._delta.set(
      this._position.x - this._previousPosition.x,
      this._position.y - this._previousPosition.y
    );
    this._previousPosition.copy(this._position);
  }

  /**
   * Check if a mouse button is currently held down
   * @param button - Mouse button code
   */
  isButtonDown(button: MouseButton): boolean {
    if (!this._enabled) return false;
    return this._buttonsDown.has(button);
  }

  /**
   * Check if a mouse button was pressed this frame
   * @param button - Mouse button code
   */
  isButtonPressed(button: MouseButton): boolean {
    if (!this._enabled) return false;
    return this._buttonsPressed.has(button);
  }

  /**
   * Check if a mouse button was released this frame
   * @param button - Mouse button code
   */
  isButtonReleased(button: MouseButton): boolean {
    if (!this._enabled) return false;
    return this._buttonsReleased.has(button);
  }

  /**
   * Get current mouse position
   */
  get position(): Vector2 {
    return this._position.clone();
  }

  /**
   * Get mouse position delta since last frame
   */
  get delta(): Vector2 {
    return this._delta.clone();
  }

  /**
   * Get mouse scroll delta this frame
   */
  get scroll(): Vector2 {
    return this._scroll.clone();
  }

  /**
   * Get mouse X position
   */
  get x(): number {
    return this._position.x;
  }

  /**
   * Get mouse Y position
   */
  get y(): number {
    return this._position.y;
  }

  /**
   * Request pointer lock
   */
  async lock(): Promise<void> {
    if (!this._element) {
      throw new Error('Mouse not initialized');
    }

    try {
      await this._element.requestPointerLock();
    } catch (error) {
      throw new Error(`Failed to lock pointer: ${error}`);
    }
  }

  /**
   * Exit pointer lock
   */
  unlock(): void {
    if (this._locked) {
      document.exitPointerLock();
    }
  }

  /**
   * Check if pointer is locked
   */
  get locked(): boolean {
    return this._locked;
  }

  /**
   * Enable or disable mouse input
   * @param enabled - Whether mouse input should be enabled
   */
  setEnabled(enabled: boolean): void {
    this._enabled = enabled;
    if (!enabled) {
      this._buttonsDown.clear();
      this._buttonsPressed.clear();
      this._buttonsReleased.clear();
    }
  }

  /**
   * Check if mouse input is enabled
   */
  get enabled(): boolean {
    return this._enabled;
  }

  /**
   * Handle mouse down events
   */
  private handleMouseDown = (event: MouseEvent): void => {
    if (!this._enabled) return;

    const button = event.button;

    if (!this._buttonsDown.has(button)) {
      this._buttonsPressed.add(button);
      this._buttonsDown.add(button);
    }
  };

  /**
   * Handle mouse up events
   */
  private handleMouseUp = (event: MouseEvent): void => {
    if (!this._enabled) return;

    const button = event.button;

    if (this._buttonsDown.has(button)) {
      this._buttonsDown.delete(button);
      this._buttonsReleased.add(button);
    }
  };

  /**
   * Handle mouse move events
   */
  private handleMouseMove = (event: MouseEvent): void => {
    if (!this._enabled) return;

    if (this._locked) {
      // In pointer lock mode, use movement delta
      this._delta.set(event.movementX, event.movementY);
    } else {
      // Normal mode, use client position
      const rect = this._element?.getBoundingClientRect();
      if (rect) {
        this._position.set(event.clientX - rect.left, event.clientY - rect.top);
      } else {
        this._position.set(event.clientX, event.clientY);
      }
    }
  };

  /**
   * Handle wheel events
   */
  private handleWheel = (event: WheelEvent): void => {
    if (!this._enabled) return;

    event.preventDefault();
    this._scroll.set(event.deltaX, event.deltaY);
  };

  /**
   * Handle context menu (right-click menu)
   */
  private handleContextMenu = (event: Event): void => {
    event.preventDefault();
  };

  /**
   * Handle pointer lock change
   */
  private handlePointerLockChange = (): void => {
    this._locked = document.pointerLockElement === this._element;
  };

  /**
   * Handle window blur (clear all button states)
   */
  private handleBlur = (): void => {
    this._buttonsDown.clear();
    this._buttonsPressed.clear();
    this._buttonsReleased.clear();
  };
}
