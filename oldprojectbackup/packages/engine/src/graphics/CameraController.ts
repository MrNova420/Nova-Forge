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
 * CameraController
 *
 * Base class and implementations for camera control systems.
 * Includes orbit, first-person, and fly controllers.
 */

import { Camera } from './Camera';
import { Vector3 } from '../math/Vector3';

/**
 * Base camera controller interface
 */
export interface ICameraController {
  update(deltaTime: number): void;
  handleInput(event: any): void;
}

/**
 * Orbit camera controller
 * Rotates around a target point
 */
export class OrbitCameraController implements ICameraController {
  private _camera: Camera;
  private _target: Vector3;
  private _distance: number;
  private _azimuth: number; // Horizontal rotation
  private _elevation: number; // Vertical rotation
  private _minDistance: number;
  private _maxDistance: number;
  private _minElevation: number;
  private _maxElevation: number;
  private _rotationSpeed: number;
  private _zoomSpeed: number;

  constructor(camera: Camera, target: Vector3 = new Vector3(0, 0, 0)) {
    this._camera = camera;
    this._target = target.clone();
    this._distance = 10;
    this._azimuth = 0;
    this._elevation = Math.PI / 6; // 30 degrees
    this._minDistance = 1;
    this._maxDistance = 1000000; // Allow massive view distances
    this._minElevation = -Math.PI / 2 + 0.01;
    this._maxElevation = Math.PI / 2 - 0.01;
    this._rotationSpeed = 0.005;
    this._zoomSpeed = 1;
  }

  /**
   * Rotate camera
   */
  rotate(deltaAzimuth: number, deltaElevation: number): void {
    this._azimuth += deltaAzimuth * this._rotationSpeed;
    this._elevation += deltaElevation * this._rotationSpeed;
    this._elevation = Math.max(
      this._minElevation,
      Math.min(this._maxElevation, this._elevation)
    );
    this.updateCameraPosition();
  }

  /**
   * Zoom camera
   */
  zoom(delta: number): void {
    this._distance -= delta * this._zoomSpeed;
    this._distance = Math.max(
      this._minDistance,
      Math.min(this._maxDistance, this._distance)
    );
    this.updateCameraPosition();
  }

  /**
   * Set target position
   */
  setTarget(target: Vector3): void {
    this._target = target.clone();
    this.updateCameraPosition();
  }

  /**
   * Update camera position based on orbit parameters
   */
  private updateCameraPosition(): void {
    const x =
      this._distance * Math.cos(this._elevation) * Math.sin(this._azimuth);
    const y = this._distance * Math.sin(this._elevation);
    const z =
      this._distance * Math.cos(this._elevation) * Math.cos(this._azimuth);

    const position = new Vector3(
      this._target.x + x,
      this._target.y + y,
      this._target.z + z
    );

    this._camera.setPosition(position);
    this._camera.lookAt(this._target);
  }

  update(_deltaTime: number): void {
    // Orbit controller is event-driven, no per-frame updates needed
  }

  handleInput(_event: any): void {
    // Input handling would be implemented based on the input system
  }
}

/**
 * First-person camera controller
 * WASD movement with mouse look
 */
export class FirstPersonCameraController implements ICameraController {
  private _camera: Camera;
  private _yaw: number;
  private _pitch: number;
  private _moveSpeed: number;
  private _lookSpeed: number;
  private _minPitch: number;
  private _maxPitch: number;

  // Movement state
  private _forward: boolean = false;
  private _backward: boolean = false;
  private _left: boolean = false;
  private _right: boolean = false;
  private _up: boolean = false;
  private _down: boolean = false;

  constructor(camera: Camera) {
    this._camera = camera;
    this._yaw = 0;
    this._pitch = 0;
    this._moveSpeed = 5;
    this._lookSpeed = 0.002;
    this._minPitch = -Math.PI / 2 + 0.01;
    this._maxPitch = Math.PI / 2 - 0.01;
  }

  /**
   * Rotate camera
   */
  rotate(deltaX: number, deltaY: number): void {
    this._yaw -= deltaX * this._lookSpeed;
    this._pitch -= deltaY * this._lookSpeed;
    this._pitch = Math.max(
      this._minPitch,
      Math.min(this._maxPitch, this._pitch)
    );
    this.updateCameraRotation();
  }

  /**
   * Set movement state
   */
  setMovement(direction: string, active: boolean): void {
    switch (direction) {
      case 'forward':
        this._forward = active;
        break;
      case 'backward':
        this._backward = active;
        break;
      case 'left':
        this._left = active;
        break;
      case 'right':
        this._right = active;
        break;
      case 'up':
        this._up = active;
        break;
      case 'down':
        this._down = active;
        break;
    }
  }

  /**
   * Update camera rotation based on yaw and pitch
   */
  private updateCameraRotation(): void {
    const forward = new Vector3(
      Math.sin(this._yaw) * Math.cos(this._pitch),
      Math.sin(this._pitch),
      -Math.cos(this._yaw) * Math.cos(this._pitch)
    );

    const position = this._camera.position;
    const target = position.clone().add(forward);
    this._camera.lookAt(target);
  }

  /**
   * Get forward direction
   */
  private getForwardDirection(): Vector3 {
    return new Vector3(
      Math.sin(this._yaw),
      0,
      -Math.cos(this._yaw)
    ).normalize();
  }

  /**
   * Get right direction
   */
  private getRightDirection(): Vector3 {
    return new Vector3(Math.cos(this._yaw), 0, Math.sin(this._yaw)).normalize();
  }

  update(deltaTime: number): void {
    const position = this._camera.position;
    const moveAmount = this._moveSpeed * deltaTime;

    if (this._forward) {
      position.add(this.getForwardDirection().multiply(moveAmount));
    }
    if (this._backward) {
      position.subtract(this.getForwardDirection().multiply(moveAmount));
    }
    if (this._right) {
      position.add(this.getRightDirection().multiply(moveAmount));
    }
    if (this._left) {
      position.subtract(this.getRightDirection().multiply(moveAmount));
    }
    if (this._up) {
      position.y += moveAmount;
    }
    if (this._down) {
      position.y -= moveAmount;
    }

    this._camera.setPosition(position);
    this.updateCameraRotation();
  }

  handleInput(_event: any): void {
    // Input handling would be implemented based on the input system
  }
}

/**
 * Fly camera controller
 * Free-flight camera with no constraints
 */
export class FlyCameraController implements ICameraController {
  private _camera: Camera;
  private _yaw: number;
  private _pitch: number;
  private _roll: number;
  private _moveSpeed: number;
  private _rotationSpeed: number;
  private _boostMultiplier: number;

  // Movement state
  private _forward: boolean = false;
  private _backward: boolean = false;
  private _left: boolean = false;
  private _right: boolean = false;
  private _up: boolean = false;
  private _down: boolean = false;
  private _boost: boolean = false;

  constructor(camera: Camera) {
    this._camera = camera;
    this._yaw = 0;
    this._pitch = 0;
    this._roll = 0;
    this._moveSpeed = 10;
    this._rotationSpeed = 0.002;
    this._boostMultiplier = 3;
  }

  /**
   * Rotate camera
   */
  rotate(deltaX: number, deltaY: number, deltaZ: number = 0): void {
    this._yaw -= deltaX * this._rotationSpeed;
    this._pitch -= deltaY * this._rotationSpeed;
    this._roll += deltaZ * this._rotationSpeed;
    this.updateCameraRotation();
  }

  /**
   * Set movement state
   */
  setMovement(direction: string, active: boolean): void {
    switch (direction) {
      case 'forward':
        this._forward = active;
        break;
      case 'backward':
        this._backward = active;
        break;
      case 'left':
        this._left = active;
        break;
      case 'right':
        this._right = active;
        break;
      case 'up':
        this._up = active;
        break;
      case 'down':
        this._down = active;
        break;
      case 'boost':
        this._boost = active;
        break;
    }
  }

  /**
   * Update camera rotation
   */
  private updateCameraRotation(): void {
    const forward = new Vector3(
      Math.sin(this._yaw) * Math.cos(this._pitch),
      Math.sin(this._pitch),
      -Math.cos(this._yaw) * Math.cos(this._pitch)
    );

    const position = this._camera.position;
    const target = position.clone().add(forward);
    this._camera.lookAt(target);
  }

  /**
   * Get forward direction
   */
  private getForwardDirection(): Vector3 {
    return new Vector3(
      Math.sin(this._yaw) * Math.cos(this._pitch),
      Math.sin(this._pitch),
      -Math.cos(this._yaw) * Math.cos(this._pitch)
    ).normalize();
  }

  /**
   * Get right direction
   */
  private getRightDirection(): Vector3 {
    return new Vector3(Math.cos(this._yaw), 0, Math.sin(this._yaw)).normalize();
  }

  /**
   * Get up direction
   */
  private getUpDirection(): Vector3 {
    const forward = this.getForwardDirection();
    const right = this.getRightDirection();
    return right.cross(forward).normalize();
  }

  update(deltaTime: number): void {
    const position = this._camera.position;
    let moveAmount = this._moveSpeed * deltaTime;

    if (this._boost) {
      moveAmount *= this._boostMultiplier;
    }

    if (this._forward) {
      position.add(this.getForwardDirection().multiply(moveAmount));
    }
    if (this._backward) {
      position.subtract(this.getForwardDirection().multiply(moveAmount));
    }
    if (this._right) {
      position.add(this.getRightDirection().multiply(moveAmount));
    }
    if (this._left) {
      position.subtract(this.getRightDirection().multiply(moveAmount));
    }
    if (this._up) {
      position.add(this.getUpDirection().multiply(moveAmount));
    }
    if (this._down) {
      position.subtract(this.getUpDirection().multiply(moveAmount));
    }

    this._camera.setPosition(position);
    this.updateCameraRotation();
  }

  handleInput(_event: any): void {
    // Input handling would be implemented based on the input system
  }
}
