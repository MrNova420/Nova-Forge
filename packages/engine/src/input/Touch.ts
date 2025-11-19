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
 * Touch
 *
 * Manages touch input for mobile and tablet devices.
 */

import { Vector2 } from '../math/Vector2';

/**
 * Touch point data
 */
export interface TouchPoint {
  id: number;
  position: Vector2;
  previousPosition: Vector2;
  startPosition: Vector2;
  delta: Vector2;
  force: number;
  radius: Vector2;
  age: number; // Frames since touch started
}

/**
 * Touch input manager
 */
export class Touch {
  private _touches: Map<number, TouchPoint> = new Map();
  private _touchesStarted: Set<number> = new Set();
  private _touchesEnded: Set<number> = new Set();
  private _enabled: boolean = true;
  private _element: HTMLElement | null = null;

  /**
   * Initialize touch input
   * @param element - DOM element to listen to (defaults to window)
   */
  initialize(element: HTMLElement = document.body): void {
    this._element = element;

    element.addEventListener('touchstart', this.handleTouchStart, {
      passive: false,
    });
    element.addEventListener('touchmove', this.handleTouchMove, {
      passive: false,
    });
    element.addEventListener('touchend', this.handleTouchEnd);
    element.addEventListener('touchcancel', this.handleTouchCancel);
  }

  /**
   * Cleanup touch input
   */
  destroy(): void {
    if (this._element) {
      this._element.removeEventListener('touchstart', this.handleTouchStart);
      this._element.removeEventListener('touchmove', this.handleTouchMove);
      this._element.removeEventListener('touchend', this.handleTouchEnd);
      this._element.removeEventListener('touchcancel', this.handleTouchCancel);
    }

    this._touches.clear();
    this._touchesStarted.clear();
    this._touchesEnded.clear();
  }

  /**
   * Update touch state (call once per frame)
   */
  update(): void {
    // Clear frame-specific state
    this._touchesStarted.clear();
    this._touchesEnded.clear();

    // Update touch ages and deltas
    for (const touch of this._touches.values()) {
      touch.age++;
      touch.delta.set(
        touch.position.x - touch.previousPosition.x,
        touch.position.y - touch.previousPosition.y
      );
      touch.previousPosition.copy(touch.position);
    }
  }

  /**
   * Get all active touches
   */
  getTouches(): TouchPoint[] {
    return Array.from(this._touches.values());
  }

  /**
   * Get a specific touch by ID
   * @param id - Touch identifier
   */
  getTouch(id: number): TouchPoint | undefined {
    return this._touches.get(id);
  }

  /**
   * Get the first active touch (useful for single-touch interactions)
   */
  getPrimaryTouch(): TouchPoint | undefined {
    return this._touches.values().next().value;
  }

  /**
   * Get number of active touches
   */
  get touchCount(): number {
    return this._touches.size;
  }

  /**
   * Check if any touch is active
   */
  get isTouching(): boolean {
    return this._touches.size > 0;
  }

  /**
   * Get touches that started this frame
   */
  getTouchesStarted(): number[] {
    return Array.from(this._touchesStarted);
  }

  /**
   * Get touches that ended this frame
   */
  getTouchesEnded(): number[] {
    return Array.from(this._touchesEnded);
  }

  /**
   * Check if a touch started this frame
   * @param id - Touch identifier
   */
  isTouchStarted(id: number): boolean {
    if (!this._enabled) return false;
    return this._touchesStarted.has(id);
  }

  /**
   * Check if a touch ended this frame
   * @param id - Touch identifier
   */
  isTouchEnded(id: number): boolean {
    if (!this._enabled) return false;
    return this._touchesEnded.has(id);
  }

  /**
   * Enable or disable touch input
   * @param enabled - Whether touch input should be enabled
   */
  setEnabled(enabled: boolean): void {
    this._enabled = enabled;
    if (!enabled) {
      this._touches.clear();
      this._touchesStarted.clear();
      this._touchesEnded.clear();
    }
  }

  /**
   * Check if touch input is enabled
   */
  get enabled(): boolean {
    return this._enabled;
  }

  /**
   * Convert browser touch to element-relative coordinates
   */
  private getTouchPosition(touch: globalThis.Touch): Vector2 {
    const rect = this._element?.getBoundingClientRect();
    if (rect) {
      return new Vector2(touch.clientX - rect.left, touch.clientY - rect.top);
    }
    return new Vector2(touch.clientX, touch.clientY);
  }

  /**
   * Handle touch start events
   */
  private handleTouchStart = (event: TouchEvent): void => {
    if (!this._enabled) return;

    event.preventDefault();

    for (let i = 0; i < event.changedTouches.length; i++) {
      const touch = event.changedTouches[i];
      if (!touch) continue;

      const position = this.getTouchPosition(touch);

      const touchPoint: TouchPoint = {
        id: touch.identifier,
        position: position.clone(),
        previousPosition: position.clone(),
        startPosition: position.clone(),
        delta: new Vector2(0, 0),
        force: touch.force || 1.0,
        radius: new Vector2(touch.radiusX || 1, touch.radiusY || 1),
        age: 0,
      };

      this._touches.set(touch.identifier, touchPoint);
      this._touchesStarted.add(touch.identifier);
    }
  };

  /**
   * Handle touch move events
   */
  private handleTouchMove = (event: TouchEvent): void => {
    if (!this._enabled) return;

    event.preventDefault();

    for (let i = 0; i < event.changedTouches.length; i++) {
      const touch = event.changedTouches[i];
      if (!touch) continue;

      const touchPoint = this._touches.get(touch.identifier);
      if (touchPoint) {
        const position = this.getTouchPosition(touch);
        touchPoint.previousPosition.copy(touchPoint.position);
        touchPoint.position.copy(position);
        touchPoint.force = touch.force || 1.0;
        touchPoint.radius.set(touch.radiusX || 1, touch.radiusY || 1);
      }
    }
  };

  /**
   * Handle touch end events
   */
  private handleTouchEnd = (event: TouchEvent): void => {
    if (!this._enabled) return;

    for (let i = 0; i < event.changedTouches.length; i++) {
      const touch = event.changedTouches[i];
      if (!touch) continue;

      this._touches.delete(touch.identifier);
      this._touchesEnded.add(touch.identifier);
    }
  };

  /**
   * Handle touch cancel events
   */
  private handleTouchCancel = (event: TouchEvent): void => {
    this.handleTouchEnd(event);
  };
}
