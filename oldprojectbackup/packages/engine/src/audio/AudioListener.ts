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
 * AudioListener
 *
 * Represents the listener's position and orientation in 3D space.
 * Typically corresponds to the camera or player position.
 */

import { Vector3 } from '../math/Vector3';

/**
 * Audio listener for 3D audio
 */
export class AudioListener {
  private _context: AudioContext;

  private _position: Vector3 = new Vector3(0, 0, 0);
  private _forward: Vector3 = new Vector3(0, 0, -1);
  private _up: Vector3 = new Vector3(0, 1, 0);
  private _velocity: Vector3 = new Vector3(0, 0, 0);

  /**
   * Create an audio listener
   * @param context - Audio context
   */
  constructor(context: AudioContext) {
    this._context = context;
  }

  /**
   * Update listener state (call once per frame)
   */
  update(): void {
    const listener = this._context.listener;

    // Update position
    if (listener.positionX) {
      // Modern API
      listener.positionX.value = this._position.x;
      listener.positionY.value = this._position.y;
      listener.positionZ.value = this._position.z;
    } else if (listener.setPosition) {
      // Legacy API
      listener.setPosition(
        this._position.x,
        this._position.y,
        this._position.z
      );
    }

    // Update orientation
    if (listener.forwardX) {
      // Modern API
      listener.forwardX.value = this._forward.x;
      listener.forwardY.value = this._forward.y;
      listener.forwardZ.value = this._forward.z;
      listener.upX.value = this._up.x;
      listener.upY.value = this._up.y;
      listener.upZ.value = this._up.z;
    } else if (listener.setOrientation) {
      // Legacy API
      listener.setOrientation(
        this._forward.x,
        this._forward.y,
        this._forward.z,
        this._up.x,
        this._up.y,
        this._up.z
      );
    }

    // Update velocity (for doppler effect) - legacy API removed in modern browsers
    // Note: Velocity/Doppler effect is deprecated in Web Audio API
  }

  /**
   * Set listener position
   * @param position - Position vector
   */
  setPosition(position: Vector3): void {
    this._position.copy(position);
  }

  /**
   * Set listener forward direction
   * @param forward - Forward vector (should be normalized)
   */
  setForward(forward: Vector3): void {
    this._forward.copy(forward).normalize();
  }

  /**
   * Set listener up direction
   * @param up - Up vector (should be normalized)
   */
  setUp(up: Vector3): void {
    this._up.copy(up).normalize();
  }

  /**
   * Set listener velocity for doppler effect
   * @param velocity - Velocity vector
   */
  setVelocity(velocity: Vector3): void {
    this._velocity.copy(velocity);
  }

  /**
   * Set listener orientation from look-at
   * @param target - Target position to look at
   * @param up - Up vector (defaults to Y-up)
   */
  lookAt(target: Vector3, up: Vector3 = new Vector3(0, 1, 0)): void {
    this._forward = target.subtract(this._position).normalize();
    this._up.copy(up).normalize();
  }

  /**
   * Get current position
   */
  get position(): Vector3 {
    return this._position.clone();
  }

  /**
   * Get forward direction
   */
  get forward(): Vector3 {
    return this._forward.clone();
  }

  /**
   * Get up direction
   */
  get up(): Vector3 {
    return this._up.clone();
  }

  /**
   * Get velocity
   */
  get velocity(): Vector3 {
    return this._velocity.clone();
  }
}
