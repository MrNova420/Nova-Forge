/**
 * Shader
 *
 * Manages WebGL shader programs including compilation, linking,
 * and uniform management.
 */

import { Matrix4 } from '../math/Matrix4';
import { Vector2 } from '../math/Vector2';
import { Vector3 } from '../math/Vector3';
import { Vector4 } from '../math/Vector4';

/**
 * Shader uniform value types
 */
export type UniformValue =
  | number
  | Vector2
  | Vector3
  | Vector4
  | Matrix4
  | number[]
  | Float32Array;

/**
 * Shader class for managing WebGL shader programs
 */
export class Shader {
  private _gl: WebGL2RenderingContext;
  private _program: WebGLProgram | null = null;
  private _vertexShader: WebGLShader | null = null;
  private _fragmentShader: WebGLShader | null = null;
  private _uniformLocations: Map<string, WebGLUniformLocation> = new Map();
  private _name: string;

  /**
   * Create a new shader
   * @param gl - WebGL context
   * @param name - Shader name for debugging
   */
  constructor(gl: WebGL2RenderingContext, name: string = 'Unnamed Shader') {
    this._gl = gl;
    this._name = name;
  }

  /**
   * Compile and link shader program
   * @param vertexSource - Vertex shader source code
   * @param fragmentSource - Fragment shader source code
   */
  compile(vertexSource: string, fragmentSource: string): boolean {
    // Compile vertex shader
    this._vertexShader = this.compileShader(
      vertexSource,
      this._gl.VERTEX_SHADER
    );
    if (!this._vertexShader) {
      return false;
    }

    // Compile fragment shader
    this._fragmentShader = this.compileShader(
      fragmentSource,
      this._gl.FRAGMENT_SHADER
    );
    if (!this._fragmentShader) {
      return false;
    }

    // Link program
    return this.linkProgram();
  }

  /**
   * Compile individual shader
   */
  private compileShader(source: string, type: number): WebGLShader | null {
    const shader = this._gl.createShader(type);
    if (!shader) {
      console.error(`Failed to create shader: ${this._name}`);
      return null;
    }

    this._gl.shaderSource(shader, source);
    this._gl.compileShader(shader);

    // Check compilation status
    if (!this._gl.getShaderParameter(shader, this._gl.COMPILE_STATUS)) {
      const info = this._gl.getShaderInfoLog(shader);
      const typeName = type === this._gl.VERTEX_SHADER ? 'vertex' : 'fragment';
      console.error(
        `Shader compilation error (${typeName}) in ${this._name}:`,
        info
      );
      this._gl.deleteShader(shader);
      return null;
    }

    return shader;
  }

  /**
   * Link shader program
   */
  private linkProgram(): boolean {
    if (!this._vertexShader || !this._fragmentShader) {
      return false;
    }

    const program = this._gl.createProgram();
    if (!program) {
      console.error(`Failed to create program: ${this._name}`);
      return false;
    }

    this._gl.attachShader(program, this._vertexShader);
    this._gl.attachShader(program, this._fragmentShader);
    this._gl.linkProgram(program);

    // Check link status
    if (!this._gl.getProgramParameter(program, this._gl.LINK_STATUS)) {
      const info = this._gl.getProgramInfoLog(program);
      console.error(`Shader link error in ${this._name}:`, info);
      this._gl.deleteProgram(program);
      return false;
    }

    this._program = program;

    // Clean up individual shaders (they're now in the program)
    this._gl.deleteShader(this._vertexShader);
    this._gl.deleteShader(this._fragmentShader);
    this._vertexShader = null;
    this._fragmentShader = null;

    console.log(`Shader compiled successfully: ${this._name}`);
    return true;
  }

  /**
   * Use this shader program
   */
  use(): void {
    if (this._program) {
      this._gl.useProgram(this._program);
    }
  }

  /**
   * Get uniform location (cached)
   */
  private getUniformLocation(name: string): WebGLUniformLocation | null {
    if (this._uniformLocations.has(name)) {
      return this._uniformLocations.get(name) || null;
    }

    if (!this._program) {
      return null;
    }

    const location = this._gl.getUniformLocation(this._program, name);
    if (location) {
      this._uniformLocations.set(name, location);
    }
    return location;
  }

  /**
   * Set uniform value
   */
  setUniform(name: string, value: UniformValue): void {
    const location = this.getUniformLocation(name);
    if (!location) {
      return;
    }

    if (typeof value === 'number') {
      this._gl.uniform1f(location, value);
    } else if (value instanceof Vector2) {
      this._gl.uniform2f(location, value.x, value.y);
    } else if (value instanceof Vector3) {
      this._gl.uniform3f(location, value.x, value.y, value.z);
    } else if (value instanceof Vector4) {
      this._gl.uniform4f(location, value.x, value.y, value.z, value.w);
    } else if (value instanceof Matrix4) {
      this._gl.uniformMatrix4fv(location, false, value.elements);
    } else if (Array.isArray(value)) {
      // Handle array of numbers
      if (value.length === 1) {
        this._gl.uniform1f(location, value[0] || 0);
      } else if (value.length === 2) {
        this._gl.uniform2fv(location, value);
      } else if (value.length === 3) {
        this._gl.uniform3fv(location, value);
      } else if (value.length === 4) {
        this._gl.uniform4fv(location, value);
      }
    } else if (value instanceof Float32Array) {
      // Handle typed arrays
      if (value.length === 16) {
        // Matrix4
        this._gl.uniformMatrix4fv(location, false, value);
      }
    }
  }

  /**
   * Set integer uniform
   */
  setUniformInt(name: string, value: number): void {
    const location = this.getUniformLocation(name);
    if (location) {
      this._gl.uniform1i(location, value);
    }
  }

  /**
   * Get shader program
   */
  get program(): WebGLProgram | null {
    return this._program;
  }

  /**
   * Get shader name
   */
  get name(): string {
    return this._name;
  }

  /**
   * Check if shader is compiled
   */
  get isCompiled(): boolean {
    return this._program !== null;
  }

  /**
   * Destroy shader and free resources
   */
  destroy(): void {
    if (this._program) {
      this._gl.deleteProgram(this._program);
      this._program = null;
    }

    if (this._vertexShader) {
      this._gl.deleteShader(this._vertexShader);
      this._vertexShader = null;
    }

    if (this._fragmentShader) {
      this._gl.deleteShader(this._fragmentShader);
      this._fragmentShader = null;
    }

    this._uniformLocations.clear();
  }
}
