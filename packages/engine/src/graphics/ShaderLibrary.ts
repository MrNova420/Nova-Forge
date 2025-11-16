/**
 * ShaderLibrary
 *
 * Manages a collection of shaders with caching and hot reload support.
 * Provides centralized shader management for the engine.
 */

import { Shader } from './Shader';

/**
 * Shader source code interface
 */
export interface ShaderSource {
  vertex: string;
  fragment: string;
}

/**
 * Shader library for managing multiple shaders
 */
export class ShaderLibrary {
  private _gl: WebGL2RenderingContext;
  private _shaders: Map<string, Shader> = new Map();
  private _sources: Map<string, ShaderSource> = new Map();

  /**
   * Create a new shader library
   * @param gl - WebGL context
   */
  constructor(gl: WebGL2RenderingContext) {
    this._gl = gl;
  }

  /**
   * Add a shader to the library
   * @param name - Shader name
   * @param vertexSource - Vertex shader source
   * @param fragmentSource - Fragment shader source
   * @returns The compiled shader or null if compilation failed
   */
  add(
    name: string,
    vertexSource: string,
    fragmentSource: string
  ): Shader | null {
    // Check if shader already exists
    if (this._shaders.has(name)) {
      console.warn(`Shader "${name}" already exists in library. Replacing...`);
      this.remove(name);
    }

    // Store sources for hot reload
    this._sources.set(name, {
      vertex: vertexSource,
      fragment: fragmentSource,
    });

    // Create and compile shader
    const shader = new Shader(this._gl, name);
    const success = shader.compile(vertexSource, fragmentSource);

    if (!success) {
      console.error(`Failed to compile shader: ${name}`);
      return null;
    }

    this._shaders.set(name, shader);
    return shader;
  }

  /**
   * Get a shader from the library
   * @param name - Shader name
   * @returns The shader or null if not found
   */
  get(name: string): Shader | null {
    return this._shaders.get(name) || null;
  }

  /**
   * Check if shader exists in library
   * @param name - Shader name
   */
  has(name: string): boolean {
    return this._shaders.has(name);
  }

  /**
   * Remove a shader from the library
   * @param name - Shader name
   */
  remove(name: string): void {
    const shader = this._shaders.get(name);
    if (shader) {
      shader.destroy();
      this._shaders.delete(name);
      this._sources.delete(name);
    }
  }

  /**
   * Reload a shader (hot reload support)
   * @param name - Shader name
   * @param vertexSource - New vertex shader source (optional)
   * @param fragmentSource - New fragment shader source (optional)
   * @returns True if reload was successful
   */
  reload(
    name: string,
    vertexSource?: string,
    fragmentSource?: string
  ): boolean {
    const source = this._sources.get(name);
    if (!source) {
      console.error(`Cannot reload shader "${name}": source not found`);
      return false;
    }

    // Use new sources if provided, otherwise use stored sources
    const vertex = vertexSource || source.vertex;
    const fragment = fragmentSource || source.fragment;

    // Remove old shader
    this.remove(name);

    // Add new shader
    const shader = this.add(name, vertex, fragment);
    return shader !== null;
  }

  /**
   * Load shader from URLs (async)
   * @param name - Shader name
   * @param vertexUrl - URL to vertex shader file
   * @param fragmentUrl - URL to fragment shader file
   */
  async loadFromUrls(
    name: string,
    vertexUrl: string,
    fragmentUrl: string
  ): Promise<Shader | null> {
    try {
      const [vertexResponse, fragmentResponse] = await Promise.all([
        fetch(vertexUrl),
        fetch(fragmentUrl),
      ]);

      if (!vertexResponse.ok || !fragmentResponse.ok) {
        console.error(`Failed to load shader files for "${name}"`);
        return null;
      }

      const vertexSource = await vertexResponse.text();
      const fragmentSource = await fragmentResponse.text();

      return this.add(name, vertexSource, fragmentSource);
    } catch (error) {
      console.error(`Error loading shader "${name}":`, error);
      return null;
    }
  }

  /**
   * Get all shader names
   */
  getNames(): string[] {
    return Array.from(this._shaders.keys());
  }

  /**
   * Get number of shaders in library
   */
  get count(): number {
    return this._shaders.size;
  }

  /**
   * Clear all shaders from library
   */
  clear(): void {
    for (const shader of this._shaders.values()) {
      shader.destroy();
    }
    this._shaders.clear();
    this._sources.clear();
  }

  /**
   * Destroy shader library and all shaders
   */
  destroy(): void {
    this.clear();
  }
}
