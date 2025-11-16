/**
 * Texture
 *
 * Represents a texture that can be applied to materials.
 * This is a placeholder implementation that will be expanded in Task 2.1.4.
 */

/**
 * Texture placeholder class
 * Full implementation in Task 2.1.4
 */
export class Texture {
  private _gl: WebGL2RenderingContext;
  private _texture: WebGLTexture | null = null;
  private _width: number = 0;
  private _height: number = 0;
  private _name: string;

  /**
   * Create a new texture
   * @param gl - WebGL context
   * @param name - Texture name
   */
  constructor(gl: WebGL2RenderingContext, name: string = 'Unnamed Texture') {
    this._gl = gl;
    this._name = name;
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
