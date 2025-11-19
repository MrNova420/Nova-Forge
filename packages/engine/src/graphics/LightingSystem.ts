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
 * LightingSystem
 *
 * System for managing and updating lights in the scene.
 * Collects all lights and prepares them for rendering.
 */

import { System } from '../ecs/System';
import { DirectionalLight } from './DirectionalLight';
import { PointLight } from './PointLight';
import { SpotLight } from './SpotLight';

/**
 * Lighting system for managing scene lights
 */
export class LightingSystem extends System {
  private _directionalLights: DirectionalLight[] = [];
  private _pointLights: PointLight[] = [];
  private _spotLights: SpotLight[] = [];
  private _maxDirectionalLights: number = 100; // Increased from 4
  private _maxPointLights: number = 1000; // Increased from 8
  private _maxSpotLights: number = 1000; // Increased from 8
  private _ambientColor: [number, number, number] = [0.2, 0.2, 0.2];

  /**
   * Update lighting system
   * @param _deltaTime - Time since last frame
   */
  update(_deltaTime: number): void {
    // Clear previous frame's lights
    this._directionalLights = [];
    this._pointLights = [];
    this._spotLights = [];

    if (!this.world) return;

    // Query all entities with DirectionalLight components
    const dirLightEntities = this.world.query({ all: [DirectionalLight] });
    for (const entity of dirLightEntities) {
      const light = this.world.getComponent(entity, DirectionalLight);
      if (
        light &&
        light.isEnabled &&
        this._directionalLights.length < this._maxDirectionalLights
      ) {
        this._directionalLights.push(light);
      }
    }

    // Query all entities with PointLight components
    const pointLightEntities = this.world.query({ all: [PointLight] });
    for (const entity of pointLightEntities) {
      const light = this.world.getComponent(entity, PointLight);
      if (
        light &&
        light.isEnabled &&
        this._pointLights.length < this._maxPointLights
      ) {
        this._pointLights.push(light);
      }
    }

    // Query all entities with SpotLight components
    const spotLightEntities = this.world.query({ all: [SpotLight] });
    for (const entity of spotLightEntities) {
      const light = this.world.getComponent(entity, SpotLight);
      if (
        light &&
        light.isEnabled &&
        this._spotLights.length < this._maxSpotLights
      ) {
        this._spotLights.push(light);
      }
    }
  }

  /**
   * Get all directional lights
   */
  getDirectionalLights(): DirectionalLight[] {
    return this._directionalLights;
  }

  /**
   * Get all point lights
   */
  getPointLights(): PointLight[] {
    return this._pointLights;
  }

  /**
   * Get all spot lights
   */
  getSpotLights(): SpotLight[] {
    return this._spotLights;
  }

  /**
   * Set ambient light color
   * @param r - Red component (0-1)
   * @param g - Green component (0-1)
   * @param b - Blue component (0-1)
   */
  setAmbientColor(r: number, g: number, b: number): void {
    this._ambientColor = [r, g, b];
  }

  /**
   * Get ambient light color
   */
  getAmbientColor(): [number, number, number] {
    return [...this._ambientColor];
  }

  /**
   * Set maximum number of each light type
   */
  setMaxLights(directional: number, point: number, spot: number): void {
    this._maxDirectionalLights = directional;
    this._maxPointLights = point;
    this._maxSpotLights = spot;
  }

  /**
   * Get light count by type
   */
  getLightCounts(): {
    directional: number;
    point: number;
    spot: number;
  } {
    return {
      directional: this._directionalLights.length,
      point: this._pointLights.length,
      spot: this._spotLights.length,
    };
  }
}
