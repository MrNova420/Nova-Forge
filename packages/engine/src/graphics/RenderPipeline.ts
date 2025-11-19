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
 * RenderPipeline
 *
 * Manages the rendering pipeline operations including clear, draw calls,
 * and state management for efficient rendering.
 */

/**
 * Render pipeline for managing rendering operations
 */
export class RenderPipeline {
  private _gl: WebGL2RenderingContext;
  private _clearColor: [number, number, number, number];
  private _clearDepth: number;

  /**
   * Create a new render pipeline
   * @param gl - WebGL 2.0 rendering context
   */
  constructor(gl: WebGL2RenderingContext) {
    this._gl = gl;
    this._clearColor = [0.1, 0.1, 0.1, 1.0];
    this._clearDepth = 1.0;
  }

  /**
   * Clear the framebuffer
   * @param color - Clear color buffer (default: true)
   * @param depth - Clear depth buffer (default: true)
   * @param stencil - Clear stencil buffer (default: false)
   */
  clear(
    color: boolean = true,
    depth: boolean = true,
    stencil: boolean = false
  ): void {
    let mask = 0;

    if (color) {
      mask |= this._gl.COLOR_BUFFER_BIT;
    }

    if (depth) {
      mask |= this._gl.DEPTH_BUFFER_BIT;
    }

    if (stencil) {
      mask |= this._gl.STENCIL_BUFFER_BIT;
    }

    if (mask !== 0) {
      this._gl.clear(mask);
    }
  }

  /**
   * Set clear color
   * @param r - Red component (0-1)
   * @param g - Green component (0-1)
   * @param b - Blue component (0-1)
   * @param a - Alpha component (0-1)
   */
  setClearColor(r: number, g: number, b: number, a: number): void {
    this._clearColor = [r, g, b, a];
    this._gl.clearColor(r, g, b, a);
  }

  /**
   * Set clear depth value
   * @param depth - Depth value (0-1)
   */
  setClearDepth(depth: number): void {
    this._clearDepth = depth;
    this._gl.clearDepth(depth);
  }

  /**
   * Get current clear color
   */
  get clearColor(): [number, number, number, number] {
    return [...this._clearColor];
  }

  /**
   * Get current clear depth
   */
  get clearDepth(): number {
    return this._clearDepth;
  }

  /**
   * Enable depth testing
   */
  enableDepthTest(): void {
    this._gl.enable(this._gl.DEPTH_TEST);
  }

  /**
   * Disable depth testing
   */
  disableDepthTest(): void {
    this._gl.disable(this._gl.DEPTH_TEST);
  }

  /**
   * Enable blending
   */
  enableBlending(): void {
    this._gl.enable(this._gl.BLEND);
  }

  /**
   * Disable blending
   */
  disableBlending(): void {
    this._gl.disable(this._gl.BLEND);
  }

  /**
   * Set blend function
   * @param src - Source blend factor
   * @param dst - Destination blend factor
   */
  setBlendFunc(src: number, dst: number): void {
    this._gl.blendFunc(src, dst);
  }

  /**
   * Enable face culling
   */
  enableCulling(): void {
    this._gl.enable(this._gl.CULL_FACE);
  }

  /**
   * Disable face culling
   */
  disableCulling(): void {
    this._gl.disable(this._gl.CULL_FACE);
  }

  /**
   * Set cull face mode
   * @param mode - Cull face mode (FRONT, BACK, FRONT_AND_BACK)
   */
  setCullFace(mode: number): void {
    this._gl.cullFace(mode);
  }

  /**
   * Set viewport
   * @param x - X position
   * @param y - Y position
   * @param width - Width
   * @param height - Height
   */
  setViewport(x: number, y: number, width: number, height: number): void {
    this._gl.viewport(x, y, width, height);
  }

  /**
   * Clean up pipeline resources
   */
  destroy(): void {
    // Pipeline doesn't own any GPU resources currently
    // This is here for future cleanup if needed
  }
}
