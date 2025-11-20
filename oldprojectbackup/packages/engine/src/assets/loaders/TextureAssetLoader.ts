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
 * TextureAssetLoader
 *
 * Asset loader for textures that integrates with the graphics system.
 * Uses TextureLoader to create Texture objects from image files.
 */

import { AssetLoader } from '../AssetLoader';
import { Texture } from '../../graphics/Texture';
import {
  TextureLoader,
  TextureLoadOptions,
} from '../../graphics/TextureLoader';

/**
 * Options for texture loading
 */
export interface TextureAssetLoadOptions extends TextureLoadOptions {
  gl: WebGL2RenderingContext;
}

/**
 * Texture asset loader for the asset management system
 */
export class TextureAssetLoader implements AssetLoader<Texture> {
  private _gl: WebGL2RenderingContext | null = null;
  private _loader: TextureLoader | null = null;

  /**
   * Set WebGL context (must be called before loading)
   * @param gl - WebGL context
   */
  setContext(gl: WebGL2RenderingContext): void {
    this._gl = gl;
    this._loader = new TextureLoader(gl);
  }

  /**
   * Load a texture from a URL
   * @param path - Image URL
   * @returns Promise that resolves to the loaded texture
   */
  async load(path: string): Promise<Texture> {
    if (!this._gl || !this._loader) {
      throw new Error(
        'WebGL context not set. Call setContext() before loading textures.'
      );
    }

    try {
      const texture = await this._loader.load(path);
      return texture;
    } catch (error) {
      throw new Error(`Failed to load texture from ${path}: ${error}`);
    }
  }

  /**
   * Check if this loader supports the given file
   * @param path - File path
   * @returns True if supported
   */
  supports(path: string): boolean {
    const ext = path.toLowerCase();
    return (
      ext.endsWith('.png') ||
      ext.endsWith('.jpg') ||
      ext.endsWith('.jpeg') ||
      ext.endsWith('.gif') ||
      ext.endsWith('.webp') ||
      ext.endsWith('.bmp')
    );
  }
}
