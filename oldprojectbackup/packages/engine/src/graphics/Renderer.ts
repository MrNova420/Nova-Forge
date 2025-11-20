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
 * Renderer Interface
 *
 * Base interface for all renderer implementations.
 * Defines the contract that all renderers must follow.
 */

import { Scene } from '../scene/Scene';
import { Camera } from './Camera';

/**
 * Base renderer interface
 */
export interface IRenderer {
  /**
   * Initialize the renderer with a canvas
   * @param canvas - The HTML canvas element to render to
   */
  initialize(canvas: HTMLCanvasElement): Promise<void>;

  /**
   * Render a scene from a camera's perspective
   * @param scene - The scene to render
   * @param camera - The camera to render from
   */
  render(scene: Scene, camera: Camera): void;

  /**
   * Resize the rendering viewport
   * @param width - New width in pixels
   * @param height - New height in pixels
   */
  resize(width: number, height: number): void;

  /**
   * Clean up and destroy the renderer
   */
  destroy(): void;

  /**
   * Get the underlying rendering context
   */
  getContext(): any;
}
