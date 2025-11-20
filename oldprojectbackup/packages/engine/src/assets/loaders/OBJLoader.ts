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
 * OBJLoader
 *
 * Loader for Wavefront OBJ 3D model files.
 * Parses OBJ text format and creates Mesh objects.
 */

import { AssetLoader } from '../AssetLoader';
import { Mesh } from '../../graphics/Mesh';
import { Vector3 } from '../../math/Vector3';

/**
 * OBJ model data container
 */
export interface OBJData {
  meshes: Mesh[];
  vertexCount: number;
  faceCount: number;
}

/**
 * Parsed OBJ data before mesh creation
 */
interface ParsedOBJData {
  positions: number[];
  normals: number[];
  uvs: number[];
  indices: number[];
  hasNormals: boolean;
  hasUVs: boolean;
}

/**
 * OBJ file loader
 */
export class OBJLoader implements AssetLoader<OBJData> {
  private _gl: WebGL2RenderingContext | null = null;

  /**
   * Set WebGL context (must be called before loading)
   * @param gl - WebGL context
   */
  setContext(gl: WebGL2RenderingContext): void {
    this._gl = gl;
  }

  /**
   * Load an OBJ file
   * @param path - OBJ file URL
   * @returns Promise that resolves to OBJ data with meshes
   */
  async load(path: string): Promise<OBJData> {
    if (!this._gl) {
      throw new Error(
        'WebGL context not set. Call setContext() before loading OBJ files.'
      );
    }

    try {
      // Fetch the OBJ file
      const response = await fetch(path);
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      const text = await response.text();
      const parsedData = this.parseOBJ(text);
      const mesh = this.createMesh(parsedData, path);

      return {
        meshes: [mesh],
        vertexCount: parsedData.positions.length / 3,
        faceCount: parsedData.indices.length / 3,
      };
    } catch (error) {
      throw new Error(`Failed to load OBJ from ${path}: ${error}`);
    }
  }

  /**
   * Parse OBJ text format
   * @param text - OBJ file content
   * @returns Parsed vertex data
   */
  private parseOBJ(text: string): ParsedOBJData {
    const positions: number[] = [];
    const normals: number[] = [];
    const uvs: number[] = [];
    const indices: number[] = [];

    // Temporary storage for raw data
    const rawPositions: Vector3[] = [];
    const rawNormals: Vector3[] = [];
    const rawUVs: [number, number][] = [];

    // Vertex cache for deduplication
    const vertexCache = new Map<string, number>();
    let vertexIndex = 0;

    const lines = text.split('\n');

    for (const line of lines) {
      const trimmed = line.trim();
      if (!trimmed || trimmed.startsWith('#')) {
        continue; // Skip empty lines and comments
      }

      const parts = trimmed.split(/\s+/);
      const type = parts[0];

      switch (type) {
        case 'v': {
          // Vertex position
          if (parts.length < 4) break;
          const x = parseFloat(parts[1]!);
          const y = parseFloat(parts[2]!);
          const z = parseFloat(parts[3]!);
          rawPositions.push(new Vector3(x, y, z));
          break;
        }

        case 'vn': {
          // Vertex normal
          if (parts.length < 4) break;
          const x = parseFloat(parts[1]!);
          const y = parseFloat(parts[2]!);
          const z = parseFloat(parts[3]!);
          rawNormals.push(new Vector3(x, y, z));
          break;
        }

        case 'vt': {
          // Texture coordinate
          if (parts.length < 3) break;
          const u = parseFloat(parts[1]!);
          const v = parseFloat(parts[2]!);
          rawUVs.push([u, v]);
          break;
        }

        case 'f': {
          // Face (triangle or quad)
          const vertices = parts.slice(1);

          // Convert to triangles if quad
          if (vertices.length === 3) {
            // Triangle
            for (const vertex of vertices) {
              const idx = this.parseVertex(
                vertex,
                rawPositions,
                rawNormals,
                rawUVs,
                positions,
                normals,
                uvs,
                vertexCache,
                vertexIndex
              );
              indices.push(idx);
              if (idx === vertexIndex) {
                vertexIndex++;
              }
            }
          } else if (vertices.length === 4) {
            // Quad - split into two triangles
            const quadIndices: number[] = [];
            for (const vertex of vertices) {
              const idx = this.parseVertex(
                vertex,
                rawPositions,
                rawNormals,
                rawUVs,
                positions,
                normals,
                uvs,
                vertexCache,
                vertexIndex
              );
              quadIndices.push(idx);
              if (idx === vertexIndex) {
                vertexIndex++;
              }
            }
            // First triangle: 0, 1, 2
            indices.push(quadIndices[0]!, quadIndices[1]!, quadIndices[2]!);
            // Second triangle: 0, 2, 3
            indices.push(quadIndices[0]!, quadIndices[2]!, quadIndices[3]!);
          }
          break;
        }
      }
    }

    return {
      positions,
      normals,
      uvs,
      indices,
      hasNormals: rawNormals.length > 0,
      hasUVs: rawUVs.length > 0,
    };
  }

  /**
   * Parse a vertex definition (e.g., "1/1/1" or "1//1")
   */
  private parseVertex(
    vertex: string,
    rawPositions: Vector3[],
    rawNormals: Vector3[],
    rawUVs: [number, number][],
    positions: number[],
    normals: number[],
    uvs: number[],
    vertexCache: Map<string, number>,
    nextIndex: number
  ): number {
    // Check cache
    const cached = vertexCache.get(vertex);
    if (cached !== undefined) {
      return cached;
    }

    // Parse vertex indices
    const parts = vertex.split('/');
    const posIndex = parseInt(parts[0]!) - 1; // OBJ indices are 1-based
    const uvIndex = parts[1] ? parseInt(parts[1]) - 1 : -1;
    const normalIndex = parts[2] ? parseInt(parts[2]) - 1 : -1;

    // Add position
    const pos = rawPositions[posIndex];
    if (!pos) {
      throw new Error(`Invalid position index: ${posIndex}`);
    }
    positions.push(pos.x, pos.y, pos.z);

    // Add normal if available
    if (normalIndex >= 0 && rawNormals[normalIndex]) {
      const normal = rawNormals[normalIndex];
      normals.push(normal.x, normal.y, normal.z);
    }

    // Add UV if available
    if (uvIndex >= 0 && rawUVs[uvIndex]) {
      const uv = rawUVs[uvIndex];
      uvs.push(uv[0], uv[1]);
    }

    // Cache and return
    vertexCache.set(vertex, nextIndex);
    return nextIndex;
  }

  /**
   * Create a mesh from parsed OBJ data
   */
  private createMesh(data: ParsedOBJData, name: string): Mesh {
    if (!this._gl) {
      throw new Error('WebGL context not set');
    }

    // Create mesh with positions and indices
    const vertices = new Float32Array(data.positions);
    const indices =
      vertices.length / 3 > 65535
        ? new Uint32Array(data.indices)
        : new Uint16Array(data.indices);

    const mesh = new Mesh(this._gl, vertices, indices, name);

    // Add normals if available
    if (data.hasNormals && data.normals.length > 0) {
      mesh.setNormals(new Float32Array(data.normals));
    } else {
      // Generate flat normals if not provided
      const generatedNormals = this.generateFlatNormals(vertices, indices);
      mesh.setNormals(generatedNormals);
    }

    // Add UVs if available
    if (data.hasUVs && data.uvs.length > 0) {
      mesh.setUVs(new Float32Array(data.uvs));
    }

    return mesh;
  }

  /**
   * Generate flat normals for a mesh (per-face normals)
   */
  private generateFlatNormals(
    vertices: Float32Array,
    indices: Uint16Array | Uint32Array
  ): Float32Array {
    const normals = new Float32Array(vertices.length);

    // Calculate normal for each triangle
    for (let i = 0; i < indices.length; i += 3) {
      const i0 = indices[i]! * 3;
      const i1 = indices[i + 1]! * 3;
      const i2 = indices[i + 2]! * 3;

      // Get vertices
      const v0 = new Vector3(vertices[i0], vertices[i0 + 1], vertices[i0 + 2]);
      const v1 = new Vector3(vertices[i1], vertices[i1 + 1], vertices[i1 + 2]);
      const v2 = new Vector3(vertices[i2], vertices[i2 + 1], vertices[i2 + 2]);

      // Calculate edges
      const edge1 = v1.subtract(v0);
      const edge2 = v2.subtract(v0);

      // Calculate normal (cross product)
      const normal = edge1.cross(edge2).normalize();

      // Assign same normal to all three vertices of the triangle
      normals[i0] = normal.x;
      normals[i0 + 1] = normal.y;
      normals[i0 + 2] = normal.z;

      normals[i1] = normal.x;
      normals[i1 + 1] = normal.y;
      normals[i1 + 2] = normal.z;

      normals[i2] = normal.x;
      normals[i2 + 1] = normal.y;
      normals[i2 + 2] = normal.z;
    }

    return normals;
  }

  /**
   * Check if this loader supports the given file
   * @param path - File path
   * @returns True if supported
   */
  supports(path: string): boolean {
    return path.toLowerCase().endsWith('.obj');
  }
}
