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
 * Callback function for the game loop
 */
export type LoopCallback = (deltaTime: number) => void;

/**
 * Options for configuring the game loop
 */
export interface LoopOptions {
  /**
   * Target frames per second (0 = unlimited)
   */
  targetFPS?: number;

  /**
   * Whether to use a fixed timestep
   */
  fixedTimestep?: boolean;

  /**
   * Fixed timestep duration in seconds
   */
  fixedTimestepDuration?: number;

  /**
   * Maximum number of fixed updates per frame
   */
  maxFixedUpdates?: number;
}

/**
 * Manages the main game loop
 */
export class Loop {
  private _isRunning: boolean = false;
  private _animationFrameId: number | null = null;
  private _callback: LoopCallback | null = null;
  private _targetFPS: number = 0;
  private _targetFrameTime: number = 0;
  private _lastFrameTime: number = 0;
  private _accumulator: number = 0;

  private _fixedTimestep: boolean = false;
  private _fixedTimestepDuration: number = 1 / 60; // 60 FPS by default
  private _maxFixedUpdates: number = 1000; // Allow catching up on slow frames

  /**
   * Creates a new game loop
   * @param options - Configuration options
   */
  constructor(options: LoopOptions = {}) {
    this._targetFPS = options.targetFPS ?? 0;
    this._targetFrameTime = this._targetFPS > 0 ? 1000 / this._targetFPS : 0;
    this._fixedTimestep = options.fixedTimestep ?? false;
    this._fixedTimestepDuration = options.fixedTimestepDuration ?? 1 / 60;
    this._maxFixedUpdates = options.maxFixedUpdates ?? 5;
  }

  /**
   * Whether the loop is currently running
   */
  get isRunning(): boolean {
    return this._isRunning;
  }

  /**
   * Target frames per second
   */
  get targetFPS(): number {
    return this._targetFPS;
  }

  set targetFPS(value: number) {
    this._targetFPS = Math.max(0, value);
    this._targetFrameTime = this._targetFPS > 0 ? 1000 / this._targetFPS : 0;
  }

  /**
   * Whether using fixed timestep
   */
  get fixedTimestep(): boolean {
    return this._fixedTimestep;
  }

  set fixedTimestep(value: boolean) {
    this._fixedTimestep = value;
  }

  /**
   * Fixed timestep duration in seconds
   */
  get fixedTimestepDuration(): number {
    return this._fixedTimestepDuration;
  }

  set fixedTimestepDuration(value: number) {
    this._fixedTimestepDuration = Math.max(0.001, value);
  }

  /**
   * Starts the game loop
   * @param callback - Function to call on each frame
   */
  start(callback: LoopCallback): void {
    if (this._isRunning) {
      console.warn('Loop is already running');
      return;
    }

    this._callback = callback;
    this._isRunning = true;
    this._lastFrameTime = performance.now();
    this._accumulator = 0;

    this._loop(this._lastFrameTime);
  }

  /**
   * Stops the game loop
   */
  stop(): void {
    if (!this._isRunning) {
      return;
    }

    this._isRunning = false;
    if (this._animationFrameId !== null) {
      cancelAnimationFrame(this._animationFrameId);
      this._animationFrameId = null;
    }
    this._callback = null;
  }

  /**
   * Internal loop function
   * @param currentTime - Current time from requestAnimationFrame
   */
  private _loop(currentTime: number): void {
    if (!this._isRunning || !this._callback) {
      return;
    }

    // Schedule next frame
    this._animationFrameId = requestAnimationFrame((time) => this._loop(time));

    // Calculate delta time
    const deltaTimeMs = currentTime - this._lastFrameTime;

    // FPS limiting
    if (this._targetFrameTime > 0 && deltaTimeMs < this._targetFrameTime) {
      return;
    }

    const deltaTime = deltaTimeMs / 1000;
    this._lastFrameTime = currentTime;

    // Fixed timestep mode
    if (this._fixedTimestep) {
      this._accumulator += deltaTime;

      let updates = 0;
      while (
        this._accumulator >= this._fixedTimestepDuration &&
        updates < this._maxFixedUpdates
      ) {
        this._callback(this._fixedTimestepDuration);
        this._accumulator -= this._fixedTimestepDuration;
        updates++;
      }

      // Prevent spiral of death
      if (this._accumulator > this._fixedTimestepDuration) {
        this._accumulator = 0;
      }
    } else {
      // Variable timestep mode
      this._callback(deltaTime);
    }
  }
}
