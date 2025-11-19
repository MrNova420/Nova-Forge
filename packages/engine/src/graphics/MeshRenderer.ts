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
 * MeshRenderer
 *
 * Component for rendering a mesh with a material.
 * Integrates with the ECS system.
 */

import { Component } from '../ecs/Component';
import { Mesh } from './Mesh';
import { Material } from './Material';

/**
 * Mesh renderer component
 */
export class MeshRenderer extends Component {
  private _mesh: Mesh | null = null;
  private _material: Material | null = null;
  private _visible: boolean = true;
  private _castShadows: boolean = true;
  private _receiveShadows: boolean = true;

  /**
   * Set the mesh to render
   * @param mesh - Mesh to render
   */
  setMesh(mesh: Mesh): void {
    this._mesh = mesh;
  }

  /**
   * Get the mesh
   */
  get mesh(): Mesh | null {
    return this._mesh;
  }

  /**
   * Set the material
   * @param material - Material to use
   */
  setMaterial(material: Material): void {
    this._material = material;
  }

  /**
   * Get the material
   */
  get material(): Material | null {
    return this._material;
  }

  /**
   * Set visibility
   * @param visible - Whether the mesh should be visible
   */
  setVisible(visible: boolean): void {
    this._visible = visible;
  }

  /**
   * Get visibility
   */
  get visible(): boolean {
    return this._visible;
  }

  /**
   * Set whether this mesh casts shadows
   * @param cast - Whether to cast shadows
   */
  setCastShadows(cast: boolean): void {
    this._castShadows = cast;
  }

  /**
   * Get whether this mesh casts shadows
   */
  get castShadows(): boolean {
    return this._castShadows;
  }

  /**
   * Set whether this mesh receives shadows
   * @param receive - Whether to receive shadows
   */
  setReceiveShadows(receive: boolean): void {
    this._receiveShadows = receive;
  }

  /**
   * Get whether this mesh receives shadows
   */
  get receiveShadows(): boolean {
    return this._receiveShadows;
  }

  /**
   * Check if renderer is ready to render
   */
  get isReady(): boolean {
    return this._mesh !== null && this._material !== null && this._visible;
  }
}
