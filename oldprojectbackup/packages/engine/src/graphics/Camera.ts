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
 * Camera
 *
 * Represents a camera in 3D space with perspective or orthographic projection.
 * Used to define the viewpoint for rendering.
 */

import { Matrix4 } from '../math/Matrix4';
import { Vector3 } from '../math/Vector3';

/**
 * Camera projection type
 */
export enum CameraProjectionType {
  Perspective = 'perspective',
  Orthographic = 'orthographic',
}

/**
 * Camera class for rendering
 */
export class Camera {
  private _projectionMatrix: Matrix4;
  private _viewMatrix: Matrix4;
  private _position: Vector3;
  private _target: Vector3;
  private _up: Vector3;
  private _projectionType: CameraProjectionType;

  // Perspective parameters
  private _fov: number;
  private _aspect: number;
  private _near: number;
  private _far: number;

  // Orthographic parameters
  private _left: number;
  private _right: number;
  private _bottom: number;
  private _top: number;

  /**
   * Create a new camera
   */
  constructor() {
    this._projectionMatrix = Matrix4.identity();
    this._viewMatrix = Matrix4.identity();
    this._position = new Vector3(0, 0, 5);
    this._target = new Vector3(0, 0, 0);
    this._up = new Vector3(0, 1, 0);
    this._projectionType = CameraProjectionType.Perspective;

    // Default perspective settings
    this._fov = Math.PI / 4; // 45 degrees
    this._aspect = 16 / 9;
    this._near = 0.1;
    this._far = 1000;

    // Default orthographic settings
    this._left = -10;
    this._right = 10;
    this._bottom = -10;
    this._top = 10;

    this.updateProjectionMatrix();
    this.updateViewMatrix();
  }

  /**
   * Set perspective projection
   * @param fov - Field of view in radians
   * @param aspect - Aspect ratio (width / height)
   * @param near - Near clipping plane
   * @param far - Far clipping plane
   */
  setPerspective(fov: number, aspect: number, near: number, far: number): void {
    this._projectionType = CameraProjectionType.Perspective;
    this._fov = fov;
    this._aspect = aspect;
    this._near = near;
    this._far = far;
    this.updateProjectionMatrix();
  }

  /**
   * Set orthographic projection
   * @param left - Left plane
   * @param right - Right plane
   * @param bottom - Bottom plane
   * @param top - Top plane
   * @param near - Near clipping plane
   * @param far - Far clipping plane
   */
  setOrthographic(
    left: number,
    right: number,
    bottom: number,
    top: number,
    near: number,
    far: number
  ): void {
    this._projectionType = CameraProjectionType.Orthographic;
    this._left = left;
    this._right = right;
    this._bottom = bottom;
    this._top = top;
    this._near = near;
    this._far = far;
    this.updateProjectionMatrix();
  }

  /**
   * Look at a target position
   * @param target - Target position to look at
   * @param up - Up vector (optional)
   */
  lookAt(target: Vector3, up?: Vector3): void {
    this._target = target.clone();
    if (up) {
      this._up = up.clone();
    }
    this.updateViewMatrix();
  }

  /**
   * Set camera position
   * @param position - New position
   */
  setPosition(position: Vector3): void {
    this._position = position.clone();
    this.updateViewMatrix();
  }

  /**
   * Get camera position
   */
  get position(): Vector3 {
    return this._position.clone();
  }

  /**
   * Get projection matrix
   */
  get projectionMatrix(): Matrix4 {
    return this._projectionMatrix;
  }

  /**
   * Get view matrix
   */
  get viewMatrix(): Matrix4 {
    return this._viewMatrix;
  }

  /**
   * Get projection type
   */
  get projectionType(): CameraProjectionType {
    return this._projectionType;
  }

  /**
   * Update projection matrix based on current settings
   */
  private updateProjectionMatrix(): void {
    if (this._projectionType === CameraProjectionType.Perspective) {
      this._projectionMatrix = Matrix4.perspective(
        this._fov,
        this._aspect,
        this._near,
        this._far
      );
    } else {
      this._projectionMatrix = Matrix4.orthographic(
        this._left,
        this._right,
        this._bottom,
        this._top,
        this._near,
        this._far
      );
    }
  }

  /**
   * Update view matrix based on position, target, and up vector
   */
  private updateViewMatrix(): void {
    this._viewMatrix = Matrix4.lookAt(this._position, this._target, this._up);
  }

  /**
   * Update aspect ratio (useful for window resize)
   * @param aspect - New aspect ratio
   */
  updateAspect(aspect: number): void {
    if (this._projectionType === CameraProjectionType.Perspective) {
      this._aspect = aspect;
      this.updateProjectionMatrix();
    }
  }
}
