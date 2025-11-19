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
 * Mesh
 *
 * Represents a 3D mesh with vertex data and attributes.
 * Manages WebGL buffer objects for efficient rendering.
 */

/**
 * Mesh class for managing 3D geometry
 */
export class Mesh {
  private _gl: WebGL2RenderingContext;
  private _name: string;

  // Vertex data
  private _vertices: Float32Array;
  private _indices: Uint16Array | Uint32Array;
  private _normals: Float32Array | null = null;
  private _uvs: Float32Array | null = null;
  private _colors: Float32Array | null = null;

  // WebGL buffers
  private _vertexBuffer: WebGLBuffer | null = null;
  private _indexBuffer: WebGLBuffer | null = null;
  private _normalBuffer: WebGLBuffer | null = null;
  private _uvBuffer: WebGLBuffer | null = null;
  private _colorBuffer: WebGLBuffer | null = null;

  // VAO for efficient rendering
  private _vao: WebGLVertexArrayObject | null = null;

  // State
  private _uploaded: boolean = false;
  private _drawMode: number;

  /**
   * Create a new mesh
   * @param gl - WebGL context
   * @param vertices - Vertex positions (x, y, z per vertex)
   * @param indices - Triangle indices
   * @param name - Mesh name
   */
  constructor(
    gl: WebGL2RenderingContext,
    vertices: Float32Array,
    indices: Uint16Array | Uint32Array,
    name: string = 'Unnamed Mesh'
  ) {
    this._gl = gl;
    this._name = name;
    this._vertices = vertices;
    this._indices = indices;
    this._drawMode = gl.TRIANGLES;
  }

  /**
   * Set normals for the mesh
   * @param normals - Normal vectors (x, y, z per vertex)
   */
  setNormals(normals: Float32Array): void {
    this._normals = normals;
    this._uploaded = false; // Mark as needing re-upload
  }

  /**
   * Set UV coordinates for the mesh
   * @param uvs - Texture coordinates (u, v per vertex)
   */
  setUVs(uvs: Float32Array): void {
    this._uvs = uvs;
    this._uploaded = false;
  }

  /**
   * Set vertex colors
   * @param colors - Vertex colors (r, g, b, a per vertex)
   */
  setColors(colors: Float32Array): void {
    this._colors = colors;
    this._uploaded = false;
  }

  /**
   * Set draw mode
   * @param mode - WebGL draw mode (TRIANGLES, LINES, POINTS, etc.)
   */
  setDrawMode(mode: number): void {
    this._drawMode = mode;
  }

  /**
   * Upload mesh data to GPU
   */
  upload(): void {
    const gl = this._gl;

    // Create VAO
    if (!this._vao) {
      this._vao = gl.createVertexArray();
    }
    gl.bindVertexArray(this._vao);

    // Upload vertex positions
    if (!this._vertexBuffer) {
      this._vertexBuffer = gl.createBuffer();
    }
    gl.bindBuffer(gl.ARRAY_BUFFER, this._vertexBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, this._vertices, gl.STATIC_DRAW);
    gl.enableVertexAttribArray(0); // position attribute
    gl.vertexAttribPointer(0, 3, gl.FLOAT, false, 0, 0);

    // Upload normals if present
    if (this._normals) {
      if (!this._normalBuffer) {
        this._normalBuffer = gl.createBuffer();
      }
      gl.bindBuffer(gl.ARRAY_BUFFER, this._normalBuffer);
      gl.bufferData(gl.ARRAY_BUFFER, this._normals, gl.STATIC_DRAW);
      gl.enableVertexAttribArray(1); // normal attribute
      gl.vertexAttribPointer(1, 3, gl.FLOAT, false, 0, 0);
    }

    // Upload UVs if present
    if (this._uvs) {
      if (!this._uvBuffer) {
        this._uvBuffer = gl.createBuffer();
      }
      gl.bindBuffer(gl.ARRAY_BUFFER, this._uvBuffer);
      gl.bufferData(gl.ARRAY_BUFFER, this._uvs, gl.STATIC_DRAW);
      gl.enableVertexAttribArray(2); // uv attribute
      gl.vertexAttribPointer(2, 2, gl.FLOAT, false, 0, 0);
    }

    // Upload colors if present
    if (this._colors) {
      if (!this._colorBuffer) {
        this._colorBuffer = gl.createBuffer();
      }
      gl.bindBuffer(gl.ARRAY_BUFFER, this._colorBuffer);
      gl.bufferData(gl.ARRAY_BUFFER, this._colors, gl.STATIC_DRAW);
      gl.enableVertexAttribArray(3); // color attribute
      gl.vertexAttribPointer(3, 4, gl.FLOAT, false, 0, 0);
    }

    // Upload indices
    if (!this._indexBuffer) {
      this._indexBuffer = gl.createBuffer();
    }
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this._indexBuffer);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, this._indices, gl.STATIC_DRAW);

    // Unbind VAO
    gl.bindVertexArray(null);

    this._uploaded = true;
  }

  /**
   * Draw the mesh
   */
  draw(): void {
    if (!this._uploaded) {
      console.warn(`Mesh "${this._name}" not uploaded. Call upload() first.`);
      return;
    }

    const gl = this._gl;
    gl.bindVertexArray(this._vao);

    const indexType =
      this._indices instanceof Uint32Array
        ? gl.UNSIGNED_INT
        : gl.UNSIGNED_SHORT;

    gl.drawElements(this._drawMode, this._indices.length, indexType, 0);

    gl.bindVertexArray(null);
  }

  /**
   * Get mesh name
   */
  get name(): string {
    return this._name;
  }

  /**
   * Get vertex count
   */
  get vertexCount(): number {
    return this._vertices.length / 3;
  }

  /**
   * Get index count
   */
  get indexCount(): number {
    return this._indices.length;
  }

  /**
   * Get triangle count
   */
  get triangleCount(): number {
    return this._indices.length / 3;
  }

  /**
   * Check if mesh is uploaded
   */
  get isUploaded(): boolean {
    return this._uploaded;
  }

  /**
   * Get vertices
   */
  get vertices(): Float32Array {
    return this._vertices;
  }

  /**
   * Get indices
   */
  get indices(): Uint16Array | Uint32Array {
    return this._indices;
  }

  /**
   * Get normals
   */
  get normals(): Float32Array | null {
    return this._normals;
  }

  /**
   * Get UVs
   */
  get uvs(): Float32Array | null {
    return this._uvs;
  }

  /**
   * Get colors
   */
  get colors(): Float32Array | null {
    return this._colors;
  }

  /**
   * Destroy mesh and free GPU resources
   */
  destroy(): void {
    const gl = this._gl;

    if (this._vertexBuffer) {
      gl.deleteBuffer(this._vertexBuffer);
      this._vertexBuffer = null;
    }

    if (this._indexBuffer) {
      gl.deleteBuffer(this._indexBuffer);
      this._indexBuffer = null;
    }

    if (this._normalBuffer) {
      gl.deleteBuffer(this._normalBuffer);
      this._normalBuffer = null;
    }

    if (this._uvBuffer) {
      gl.deleteBuffer(this._uvBuffer);
      this._uvBuffer = null;
    }

    if (this._colorBuffer) {
      gl.deleteBuffer(this._colorBuffer);
      this._colorBuffer = null;
    }

    if (this._vao) {
      gl.deleteVertexArray(this._vao);
      this._vao = null;
    }

    this._uploaded = false;
  }
}
