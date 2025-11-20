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
 * Texture
 *
 * Represents a texture that can be applied to materials.
 * Supports image loading, mipmap generation, and filtering options.
 */

/**
 * Texture wrap modes
 */
export enum TextureWrap {
  Repeat = 'repeat',
  ClampToEdge = 'clamp-to-edge',
  MirroredRepeat = 'mirrored-repeat',
}

/**
 * Texture filter modes
 */
export enum TextureFilter {
  Nearest = 'nearest',
  Linear = 'linear',
  NearestMipmapNearest = 'nearest-mipmap-nearest',
  LinearMipmapNearest = 'linear-mipmap-nearest',
  NearestMipmapLinear = 'nearest-mipmap-linear',
  LinearMipmapLinear = 'linear-mipmap-linear',
}

/**
 * Texture format
 */
export enum TextureFormat {
  RGB = 'rgb',
  RGBA = 'rgba',
  Luminance = 'luminance',
  LuminanceAlpha = 'luminance-alpha',
}

/**
 * Texture class for managing WebGL textures
 */
export class Texture {
  private _gl: WebGL2RenderingContext;
  private _texture: WebGLTexture | null = null;
  private _width: number = 0;
  private _height: number = 0;
  private _name: string;
  private _format: TextureFormat;
  private _wrapS: TextureWrap;
  private _wrapT: TextureWrap;
  private _minFilter: TextureFilter;
  private _magFilter: TextureFilter;
  private _generateMipmaps: boolean;

  /**
   * Create a new texture
   * @param gl - WebGL context
   * @param name - Texture name
   */
  constructor(gl: WebGL2RenderingContext, name: string = 'Unnamed Texture') {
    this._gl = gl;
    this._name = name;
    this._format = TextureFormat.RGBA;
    this._wrapS = TextureWrap.Repeat;
    this._wrapT = TextureWrap.Repeat;
    this._minFilter = TextureFilter.LinearMipmapLinear;
    this._magFilter = TextureFilter.Linear;
    this._generateMipmaps = true;

    // Create WebGL texture
    this._texture = gl.createTexture();
    if (!this._texture) {
      throw new Error('Failed to create WebGL texture');
    }
  }

  /**
   * Create texture from image element
   * @param image - HTML image element
   */
  fromImage(image: HTMLImageElement): void {
    if (!this._texture) return;

    this._width = image.width;
    this._height = image.height;

    const gl = this._gl;
    gl.bindTexture(gl.TEXTURE_2D, this._texture);

    // Upload image data
    gl.texImage2D(
      gl.TEXTURE_2D,
      0,
      this.getInternalFormat(),
      this.getFormat(),
      gl.UNSIGNED_BYTE,
      image
    );

    // Apply texture parameters
    this.applyParameters();

    // Generate mipmaps if needed
    if (this._generateMipmaps && this.isPowerOfTwo()) {
      gl.generateMipmap(gl.TEXTURE_2D);
    }

    gl.bindTexture(gl.TEXTURE_2D, null);
  }

  /**
   * Create texture from raw data
   * @param width - Texture width
   * @param height - Texture height
   * @param data - Pixel data (Uint8Array)
   */
  fromData(
    width: number,
    height: number,
    data: Uint8Array | null = null
  ): void {
    if (!this._texture) return;

    this._width = width;
    this._height = height;

    const gl = this._gl;
    gl.bindTexture(gl.TEXTURE_2D, this._texture);

    // Upload data
    gl.texImage2D(
      gl.TEXTURE_2D,
      0,
      this.getInternalFormat(),
      width,
      height,
      0,
      this.getFormat(),
      gl.UNSIGNED_BYTE,
      data
    );

    // Apply texture parameters
    this.applyParameters();

    // Generate mipmaps if needed
    if (this._generateMipmaps && this.isPowerOfTwo()) {
      gl.generateMipmap(gl.TEXTURE_2D);
    }

    gl.bindTexture(gl.TEXTURE_2D, null);
  }

  /**
   * Apply texture parameters (wrap, filter)
   */
  private applyParameters(): void {
    const gl = this._gl;

    // Set wrap modes
    gl.texParameteri(
      gl.TEXTURE_2D,
      gl.TEXTURE_WRAP_S,
      this.getWrapMode(this._wrapS)
    );
    gl.texParameteri(
      gl.TEXTURE_2D,
      gl.TEXTURE_WRAP_T,
      this.getWrapMode(this._wrapT)
    );

    // Set filter modes
    gl.texParameteri(
      gl.TEXTURE_2D,
      gl.TEXTURE_MIN_FILTER,
      this.getFilterMode(this._minFilter)
    );
    gl.texParameteri(
      gl.TEXTURE_2D,
      gl.TEXTURE_MAG_FILTER,
      this.getFilterMode(this._magFilter)
    );
  }

  /**
   * Get WebGL wrap mode constant
   */
  private getWrapMode(wrap: TextureWrap): number {
    const gl = this._gl;
    switch (wrap) {
      case TextureWrap.Repeat:
        return gl.REPEAT;
      case TextureWrap.ClampToEdge:
        return gl.CLAMP_TO_EDGE;
      case TextureWrap.MirroredRepeat:
        return gl.MIRRORED_REPEAT;
      default:
        return gl.REPEAT;
    }
  }

  /**
   * Get WebGL filter mode constant
   */
  private getFilterMode(filter: TextureFilter): number {
    const gl = this._gl;
    switch (filter) {
      case TextureFilter.Nearest:
        return gl.NEAREST;
      case TextureFilter.Linear:
        return gl.LINEAR;
      case TextureFilter.NearestMipmapNearest:
        return gl.NEAREST_MIPMAP_NEAREST;
      case TextureFilter.LinearMipmapNearest:
        return gl.LINEAR_MIPMAP_NEAREST;
      case TextureFilter.NearestMipmapLinear:
        return gl.NEAREST_MIPMAP_LINEAR;
      case TextureFilter.LinearMipmapLinear:
        return gl.LINEAR_MIPMAP_LINEAR;
      default:
        return gl.LINEAR;
    }
  }

  /**
   * Get WebGL format constant
   */
  private getFormat(): number {
    const gl = this._gl;
    switch (this._format) {
      case TextureFormat.RGB:
        return gl.RGB;
      case TextureFormat.RGBA:
        return gl.RGBA;
      case TextureFormat.Luminance:
        return gl.LUMINANCE;
      case TextureFormat.LuminanceAlpha:
        return gl.LUMINANCE_ALPHA;
      default:
        return gl.RGBA;
    }
  }

  /**
   * Get WebGL internal format constant
   */
  private getInternalFormat(): number {
    const gl = this._gl;
    switch (this._format) {
      case TextureFormat.RGB:
        return gl.RGB;
      case TextureFormat.RGBA:
        return gl.RGBA;
      case TextureFormat.Luminance:
        return gl.LUMINANCE;
      case TextureFormat.LuminanceAlpha:
        return gl.LUMINANCE_ALPHA;
      default:
        return gl.RGBA;
    }
  }

  /**
   * Check if texture dimensions are power of two
   */
  private isPowerOfTwo(): boolean {
    return (
      (this._width & (this._width - 1)) === 0 &&
      (this._height & (this._height - 1)) === 0
    );
  }

  /**
   * Set texture wrap mode
   * @param wrapS - Horizontal wrap mode
   * @param wrapT - Vertical wrap mode (optional, defaults to wrapS)
   */
  setWrap(wrapS: TextureWrap, wrapT?: TextureWrap): void {
    this._wrapS = wrapS;
    this._wrapT = wrapT || wrapS;
  }

  /**
   * Set texture filter mode
   * @param minFilter - Minification filter
   * @param magFilter - Magnification filter (optional, defaults to linear)
   */
  setFilter(minFilter: TextureFilter, magFilter?: TextureFilter): void {
    this._minFilter = minFilter;
    this._magFilter = magFilter || TextureFilter.Linear;
  }

  /**
   * Set whether to generate mipmaps
   * @param generate - Whether to generate mipmaps
   */
  setGenerateMipmaps(generate: boolean): void {
    this._generateMipmaps = generate;
  }

  /**
   * Set texture format
   * @param format - Texture format
   */
  setFormat(format: TextureFormat): void {
    this._format = format;
  }

  /**
   * Get texture name
   */
  get name(): string {
    return this._name;
  }

  /**
   * Get texture width
   */
  get width(): number {
    return this._width;
  }

  /**
   * Get texture height
   */
  get height(): number {
    return this._height;
  }

  /**
   * Get WebGL texture object
   */
  get glTexture(): WebGLTexture | null {
    return this._texture;
  }

  /**
   * Bind texture to a texture unit
   * @param unit - Texture unit (0-31)
   */
  bind(unit: number = 0): void {
    this._gl.activeTexture(this._gl.TEXTURE0 + unit);
    this._gl.bindTexture(this._gl.TEXTURE_2D, this._texture);
  }

  /**
   * Unbind texture
   */
  unbind(): void {
    this._gl.bindTexture(this._gl.TEXTURE_2D, null);
  }

  /**
   * Destroy texture and free resources
   */
  destroy(): void {
    if (this._texture) {
      this._gl.deleteTexture(this._texture);
      this._texture = null;
    }
  }
}
