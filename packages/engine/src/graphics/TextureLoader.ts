/**
 * TextureLoader
 *
 * Handles asynchronous loading of textures from URLs or data sources.
 */

import { Texture, TextureFormat, TextureWrap, TextureFilter } from './Texture';

/**
 * Texture loading options
 */
export interface TextureLoadOptions {
  format?: TextureFormat;
  wrapS?: TextureWrap;
  wrapT?: TextureWrap;
  minFilter?: TextureFilter;
  magFilter?: TextureFilter;
  generateMipmaps?: boolean;
  flipY?: boolean;
}

/**
 * Texture loader for loading textures asynchronously
 */
export class TextureLoader {
  private _gl: WebGL2RenderingContext;

  /**
   * Create a new texture loader
   * @param gl - WebGL context
   */
  constructor(gl: WebGL2RenderingContext) {
    this._gl = gl;
  }

  /**
   * Load texture from URL
   * @param url - Image URL
   * @param name - Texture name (optional, defaults to URL)
   * @param options - Loading options
   * @returns Promise that resolves to the loaded texture
   */
  async load(
    url: string,
    name?: string,
    options?: TextureLoadOptions
  ): Promise<Texture> {
    return new Promise((resolve, reject) => {
      const image = new Image();

      image.onload = () => {
        try {
          const texture = this.createTextureFromImage(
            image,
            name || url,
            options
          );
          resolve(texture);
        } catch (error) {
          reject(error);
        }
      };

      image.onerror = () => {
        reject(new Error(`Failed to load texture from URL: ${url}`));
      };

      // Enable CORS if loading from different origin
      image.crossOrigin = 'anonymous';
      image.src = url;
    });
  }

  /**
   * Load multiple textures
   * @param urls - Array of image URLs
   * @param options - Loading options
   * @returns Promise that resolves to array of loaded textures
   */
  async loadMultiple(
    urls: string[],
    options?: TextureLoadOptions
  ): Promise<Texture[]> {
    const promises = urls.map((url) => this.load(url, undefined, options));
    return Promise.all(promises);
  }

  /**
   * Create texture from HTML image element
   * @param image - HTML image element
   * @param name - Texture name
   * @param options - Loading options
   * @returns Created texture
   */
  createTextureFromImage(
    image: HTMLImageElement,
    name: string,
    options?: TextureLoadOptions
  ): Texture {
    const texture = new Texture(this._gl, name);

    // Apply options
    if (options) {
      if (options.format !== undefined) {
        texture.setFormat(options.format);
      }
      if (options.wrapS !== undefined || options.wrapT !== undefined) {
        texture.setWrap(
          options.wrapS || TextureWrap.Repeat,
          options.wrapT || options.wrapS
        );
      }
      if (options.minFilter !== undefined || options.magFilter !== undefined) {
        texture.setFilter(
          options.minFilter || TextureFilter.LinearMipmapLinear,
          options.magFilter
        );
      }
      if (options.generateMipmaps !== undefined) {
        texture.setGenerateMipmaps(options.generateMipmaps);
      }
    }

    // Upload image data
    texture.fromImage(image);

    return texture;
  }

  /**
   * Create a solid color texture
   * @param color - RGBA color (0-255)
   * @param name - Texture name
   * @returns Created texture
   */
  createSolidColor(
    color: [number, number, number, number],
    name: string = 'Solid Color'
  ): Texture {
    const texture = new Texture(this._gl, name);
    texture.setGenerateMipmaps(false);
    texture.setWrap(TextureWrap.ClampToEdge);
    texture.setFilter(TextureFilter.Nearest);

    const data = new Uint8Array(color);
    texture.fromData(1, 1, data);

    return texture;
  }

  /**
   * Create a checkerboard texture (useful for debugging)
   * @param size - Size of each checker square
   * @param count - Number of checkers per side
   * @param color1 - First color
   * @param color2 - Second color
   * @param name - Texture name
   * @returns Created texture
   */
  createCheckerboard(
    size: number = 8,
    count: number = 8,
    color1: [number, number, number, number] = [255, 255, 255, 255],
    color2: [number, number, number, number] = [0, 0, 0, 255],
    name: string = 'Checkerboard'
  ): Texture {
    const width = size * count;
    const height = size * count;
    const data = new Uint8Array(width * height * 4);

    for (let y = 0; y < height; y++) {
      for (let x = 0; x < width; x++) {
        const checkerX = Math.floor(x / size);
        const checkerY = Math.floor(y / size);
        const isWhite = (checkerX + checkerY) % 2 === 0;
        const color = isWhite ? color1 : color2;

        const index = (y * width + x) * 4;
        data[index] = color[0];
        data[index + 1] = color[1];
        data[index + 2] = color[2];
        data[index + 3] = color[3];
      }
    }

    const texture = new Texture(this._gl, name);
    texture.setWrap(TextureWrap.Repeat);
    texture.fromData(width, height, data);

    return texture;
  }
}
