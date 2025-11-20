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
 * PointLight
 *
 * A light that emits in all directions from a single point.
 * Similar to a light bulb - has position and range/attenuation.
 */

import { Light, LightType } from './Light';

/**
 * Point light component (omni-directional light)
 */
export class PointLight extends Light {
  private _range: number;
  private _attenuation: { constant: number; linear: number; quadratic: number };

  constructor() {
    super(LightType.Point);
    this._range = 10;
    this._attenuation = {
      constant: 1.0,
      linear: 0.09,
      quadratic: 0.032,
    };
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
