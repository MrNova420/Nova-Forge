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
 * Light
 *
 * Base class for all light types in the engine.
 * Provides common properties and methods for lighting.
 */

import { Component } from '../ecs/Component';
import { Vector3 } from '../math/Vector3';

/**
 * Light type enumeration
 */
export enum LightType {
  Directional = 'directional',
  Point = 'point',
  Spot = 'spot',
  Ambient = 'ambient',
}

/**
 * Base light component
 */
export abstract class Light extends Component {
  protected _color: Vector3;
  protected _intensity: number;
  protected _enabled: boolean;
  protected _castShadows: boolean;
  protected _type: LightType;

  constructor(type: LightType) {
    super();
    this._type = type;
    this._color = new Vector3(1, 1, 1); // White light
    this._intensity = 1.0;
    this._enabled = true;
    this._castShadows = false;
  }

  /**
   * Get light type
   */
  get type(): LightType {
    return this._type;
  }

  /**
   * Set light color
   * @param color - RGB color (0-1 range)
   */
  setColor(color: Vector3): void {
    this._color = color.clone();
  }

  /**
   * Get light color
   */
  get color(): Vector3 {
    return this._color.clone();
  }

  /**
   * Set light intensity
   * @param intensity - Light intensity (typically 0-1, but can be higher)
   */
  setIntensity(intensity: number): void {
    this._intensity = intensity;
  }

  /**
   * Get light intensity
   */
  get intensity(): number {
    return this._intensity;
  }

  /**
   * Enable/disable light
   * @param enabled - Whether light should be enabled
   */
  setEnabled(enabled: boolean): void {
    this._enabled = enabled;
  }

  /**
   * Check if light is enabled
   */
  get isEnabled(): boolean {
    return this._enabled;
  }

  /**
   * Enable/disable shadow casting
   * @param cast - Whether light should cast shadows
   */
  setCastShadows(cast: boolean): void {
    this._castShadows = cast;
  }

  /**
   * Check if light casts shadows
   */
  get castShadows(): boolean {
    return this._castShadows;
  }
}
