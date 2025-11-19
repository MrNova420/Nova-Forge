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
 * SpotLight
 *
 * A light that emits in a cone from a point.
 * Similar to a flashlight or stage light - has position, direction, and cone angle.
 */

import { Light, LightType } from './Light';
import { Vector3 } from '../math/Vector3';

/**
 * Spot light component (cone light)
 */
export class SpotLight extends Light {
  private _direction: Vector3;
  private _range: number;
  private _innerConeAngle: number;
  private _outerConeAngle: number;
  private _attenuation: { constant: number; linear: number; quadratic: number };

  constructor() {
    super(LightType.Spot);
    this._direction = new Vector3(0, -1, 0); // Default: pointing down
    this._range = 10;
    this._innerConeAngle = Math.PI / 8; // 22.5 degrees
    this._outerConeAngle = Math.PI / 6; // 30 degrees
    this._attenuation = {
      constant: 1.0,
      linear: 0.09,
      quadratic: 0.032,
    };
  }

  /**
   * Set light direction
   * @param direction - Direction vector (will be normalized)
   */
  setDirection(direction: Vector3): void {
    this._direction = direction.clone().normalize();
  }

  /**
   * Get light direction
   */
  get direction(): Vector3 {
    return this._direction.clone();
  }

  /**
   * Set light range
   * @param range - Maximum distance light can reach
   */
  setRange(range: number): void {
    this._range = range;
  }

  /**
   * Get light range
   */
  get range(): number {
    return this._range;
  }

  /**
   * Set cone angles
   * @param innerAngle - Inner cone angle in radians (full intensity)
   * @param outerAngle - Outer cone angle in radians (falloff to zero)
   */
  setConeAngles(innerAngle: number, outerAngle: number): void {
    this._innerConeAngle = innerAngle;
    this._outerConeAngle = outerAngle;
  }

  /**
   * Get inner cone angle
   */
  get innerConeAngle(): number {
    return this._innerConeAngle;
  }

  /**
   * Get outer cone angle
   */
  get outerConeAngle(): number {
    return this._outerConeAngle;
  }

  /**
   * Set attenuation parameters
   * @param constant - Constant attenuation factor
   * @param linear - Linear attenuation factor
   * @param quadratic - Quadratic attenuation factor
   */
  setAttenuation(constant: number, linear: number, quadratic: number): void {
    this._attenuation = { constant, linear, quadratic };
  }

  /**
   * Get attenuation parameters
   */
  get attenuation(): { constant: number; linear: number; quadratic: number } {
    return { ...this._attenuation };
  }
}
