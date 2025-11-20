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
 * GLTFLoader
 *
 * Loader for glTF 2.0 (GL Transmission Format) 3D model files.
 * Supports both .gltf (JSON + external files) and .glb (binary) formats.
 */

import { AssetLoader } from '../AssetLoader';
import { Mesh } from '../../graphics/Mesh';

/**
 * GLTF model data container
 */
export interface GLTFData {
  meshes: Mesh[];
  scenes: GLTFScene[];
  nodes: GLTFNode[];
  textures?: any[]; // Will be implemented with texture system
  materials?: any[]; // Will be implemented with material system
}

/**
 * GLTF scene
 */
export interface GLTFScene {
  name?: string;
  nodes: number[]; // Indices into nodes array
}

/**
 * GLTF node (transform + mesh/camera/light reference)
 */
export interface GLTFNode {
  name?: string;
  mesh?: number;
  children?: number[];
  matrix?: number[]; // 4x4 transform matrix
  translation?: [number, number, number];
  rotation?: [number, number, number, number]; // Quaternion
  scale?: [number, number, number];
}

/**
 * GLTF accessor types
 */
enum ComponentType {
  BYTE = 5120,
  UNSIGNED_BYTE = 5121,
  SHORT = 5122,
  UNSIGNED_SHORT = 5123,
  UNSIGNED_INT = 5125,
  FLOAT = 5126,
}

/**
 * GLTF loader for glTF 2.0 files
 */
export class GLTFLoader implements AssetLoader<GLTFData> {
  private _gl: WebGL2RenderingContext | null = null;

  /**
   * Set WebGL context (must be called before loading)
   * @param gl - WebGL context
   */
  setContext(gl: WebGL2RenderingContext): void {
    this._gl = gl;
  }

  /**
   * Load a GLTF/GLB file
   * @param path - GLTF/GLB file URL
   * @returns Promise that resolves to GLTF data
   */
  async load(path: string): Promise<GLTFData> {
    if (!this._gl) {
      throw new Error(
        'WebGL context not set. Call setContext() before loading GLTF files.'
      );
    }

    try {
      const isGLB = path.toLowerCase().endsWith('.glb');

      if (isGLB) {
        return await this.loadGLB(path);
      } else {
        return await this.loadGLTF(path);
      }
    } catch (error) {
      throw new Error(`Failed to load GLTF from ${path}: ${error}`);
    }
  }

  /**
   * Load a .gltf file (JSON format)
   */
  private async loadGLTF(path: string): Promise<GLTFData> {
    // Fetch the JSON
    const response = await fetch(path);
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }

    const gltf = await response.json();
    const basePath = path.substring(0, path.lastIndexOf('/') + 1);

    // Load buffers
    const buffers = await this.loadBuffers(gltf, basePath);

    // Parse and create meshes
    return this.parseGLTF(gltf, buffers);
  }

  /**
   * Load a .glb file (binary format)
   */
  private async loadGLB(path: string): Promise<GLTFData> {
    const response = await fetch(path);
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }

    const arrayBuffer = await response.arrayBuffer();
    const dataView = new DataView(arrayBuffer);

    // Read GLB header
    const magic = dataView.getUint32(0, true);
    if (magic !== 0x46546c67) {
      // 'glTF' in ASCII
      throw new Error('Invalid GLB file: incorrect magic number');
    }

    const version = dataView.getUint32(4, true);
    if (version !== 2) {
      throw new Error(`Unsupported GLB version: ${version}`);
    }

    // Read JSON chunk
    const jsonChunkLength = dataView.getUint32(12, true);
    const jsonChunkType = dataView.getUint32(16, true);
    if (jsonChunkType !== 0x4e4f534a) {
      // 'JSON' in ASCII
      throw new Error('Invalid GLB file: JSON chunk not found');
    }

    const jsonData = new Uint8Array(arrayBuffer, 20, jsonChunkLength);
    const gltf = JSON.parse(new TextDecoder().decode(jsonData));

    // Read binary chunk if present
    let binaryBuffer: ArrayBuffer | null = null;
    const binaryChunkStart = 20 + jsonChunkLength;
    if (binaryChunkStart < arrayBuffer.byteLength) {
      const binaryChunkLength = dataView.getUint32(binaryChunkStart, true);
      const binaryChunkType = dataView.getUint32(binaryChunkStart + 4, true);
      if (binaryChunkType === 0x004e4942) {
        // 'BIN' in ASCII
        binaryBuffer = arrayBuffer.slice(
          binaryChunkStart + 8,
          binaryChunkStart + 8 + binaryChunkLength
        );
      }
    }

    // Parse with the embedded binary buffer
    const buffers = binaryBuffer ? [binaryBuffer] : [];
    return this.parseGLTF(gltf, buffers);
  }

  /**
   * Load external buffers referenced in GLTF
   */
  private async loadBuffers(
    gltf: any,
    basePath: string
  ): Promise<ArrayBuffer[]> {
    if (!gltf.buffers) {
      return [];
    }

    const promises = gltf.buffers.map(async (buffer: any) => {
      if (buffer.uri) {
        // External file
        const response = await fetch(basePath + buffer.uri);
        if (!response.ok) {
          throw new Error(`Failed to load buffer: ${buffer.uri}`);
        }
        return response.arrayBuffer();
      }
      throw new Error('Buffer has no URI');
    });

    return Promise.all(promises);
  }

  /**
   * Parse GLTF JSON and create engine objects
   */
  private parseGLTF(gltf: any, buffers: ArrayBuffer[]): GLTFData {
    const meshes: Mesh[] = [];

    // Parse meshes
    if (gltf.meshes) {
      for (let i = 0; i < gltf.meshes.length; i++) {
        const gltfMesh = gltf.meshes[i];
        const mesh = this.parseMesh(gltfMesh, gltf, buffers);
        if (mesh) {
          meshes.push(mesh);
        }
      }
    }

    // Parse scenes
    const scenes: GLTFScene[] = [];
    if (gltf.scenes) {
      for (const scene of gltf.scenes) {
        scenes.push({
          name: scene.name,
          nodes: scene.nodes || [],
        });
      }
    }

    // Parse nodes
    const nodes: GLTFNode[] = [];
    if (gltf.nodes) {
      for (const node of gltf.nodes) {
        nodes.push({
          name: node.name,
          mesh: node.mesh,
          children: node.children,
          matrix: node.matrix,
          translation: node.translation,
          rotation: node.rotation,
          scale: node.scale,
        });
      }
    }

    return {
      meshes,
      scenes,
      nodes,
    };
  }

  /**
   * Parse a single mesh from GLTF
   */
  private parseMesh(
    gltfMesh: any,
    gltf: any,
    buffers: ArrayBuffer[]
  ): Mesh | null {
    if (!this._gl) {
      return null;
    }

    // GLTF meshes can have multiple primitives - we'll take the first one
    const primitive = gltfMesh.primitives[0];
    if (!primitive) {
      return null;
    }

    // Get attributes
    const positionAccessor = this.getAccessor(
      primitive.attributes.POSITION,
      gltf,
      buffers
    );
    const normalAccessor = primitive.attributes.NORMAL
      ? this.getAccessor(primitive.attributes.NORMAL, gltf, buffers)
      : null;
    const uvAccessor = primitive.attributes.TEXCOORD_0
      ? this.getAccessor(primitive.attributes.TEXCOORD_0, gltf, buffers)
      : null;
    const indicesAccessor = primitive.indices
      ? this.getAccessor(primitive.indices, gltf, buffers)
      : null;

    if (!positionAccessor) {
      return null;
    }

    // Create mesh
    const vertices = new Float32Array(positionAccessor.data);
    const indices = indicesAccessor
      ? this.createIndices(indicesAccessor)
      : this.generateIndices(positionAccessor.count);

    const mesh = new Mesh(
      this._gl,
      vertices,
      indices,
      gltfMesh.name || 'GLTF Mesh'
    );

    // Add normals if available
    if (normalAccessor) {
      mesh.setNormals(new Float32Array(normalAccessor.data));
    }

    // Add UVs if available
    if (uvAccessor) {
      mesh.setUVs(new Float32Array(uvAccessor.data));
    }

    return mesh;
  }

  /**
   * Get accessor data from GLTF
   */
  private getAccessor(
    accessorIndex: number,
    gltf: any,
    buffers: ArrayBuffer[]
  ): { data: number[]; count: number } | null {
    const accessor = gltf.accessors[accessorIndex];
    if (!accessor) {
      return null;
    }

    const bufferView = gltf.bufferViews[accessor.bufferView];
    const buffer = buffers[bufferView.buffer];

    if (!buffer) {
      throw new Error('Buffer not found');
    }

    const byteOffset =
      (bufferView.byteOffset || 0) + (accessor.byteOffset || 0);
    const componentCount = this.getComponentCount(accessor.type);
    const componentSize = this.getComponentSize(accessor.componentType);
    const elementSize = componentCount * componentSize;
    const count = accessor.count;

    // Extract data
    const data: number[] = [];
    const dataView = new DataView(buffer);

    for (let i = 0; i < count; i++) {
      const elementOffset = byteOffset + i * elementSize;
      for (let j = 0; j < componentCount; j++) {
        const offset = elementOffset + j * componentSize;
        const value = this.readComponent(
          dataView,
          offset,
          accessor.componentType
        );
        data.push(value);
      }
    }

    return { data, count };
  }

  /**
   * Get number of components for accessor type
   */
  private getComponentCount(type: string): number {
    switch (type) {
      case 'SCALAR':
        return 1;
      case 'VEC2':
        return 2;
      case 'VEC3':
        return 3;
      case 'VEC4':
        return 4;
      case 'MAT2':
        return 4;
      case 'MAT3':
        return 9;
      case 'MAT4':
        return 16;
      default:
        throw new Error(`Unknown accessor type: ${type}`);
    }
  }

  /**
   * Get component size in bytes
   */
  private getComponentSize(componentType: number): number {
    switch (componentType) {
      case ComponentType.BYTE:
      case ComponentType.UNSIGNED_BYTE:
        return 1;
      case ComponentType.SHORT:
      case ComponentType.UNSIGNED_SHORT:
        return 2;
      case ComponentType.UNSIGNED_INT:
      case ComponentType.FLOAT:
        return 4;
      default:
        throw new Error(`Unknown component type: ${componentType}`);
    }
  }

  /**
   * Read a component value from DataView
   */
  private readComponent(
    dataView: DataView,
    offset: number,
    componentType: number
  ): number {
    switch (componentType) {
      case ComponentType.BYTE:
        return dataView.getInt8(offset);
      case ComponentType.UNSIGNED_BYTE:
        return dataView.getUint8(offset);
      case ComponentType.SHORT:
        return dataView.getInt16(offset, true);
      case ComponentType.UNSIGNED_SHORT:
        return dataView.getUint16(offset, true);
      case ComponentType.UNSIGNED_INT:
        return dataView.getUint32(offset, true);
      case ComponentType.FLOAT:
        return dataView.getFloat32(offset, true);
      default:
        throw new Error(`Unknown component type: ${componentType}`);
    }
  }

  /**
   * Create indices array from accessor data
   */
  private createIndices(accessor: {
    data: number[];
    count: number;
  }): Uint16Array | Uint32Array {
    const maxIndex = Math.max(...accessor.data);
    if (maxIndex > 65535) {
      return new Uint32Array(accessor.data);
    } else {
      return new Uint16Array(accessor.data);
    }
  }

  /**
   * Generate sequential indices if none provided
   */
  private generateIndices(count: number): Uint16Array | Uint32Array {
    const indices = new Array(count);
    for (let i = 0; i < count; i++) {
      indices[i] = i;
    }
    return count > 65535 ? new Uint32Array(indices) : new Uint16Array(indices);
  }

  /**
   * Check if this loader supports the given file
   * @param path - File path
   * @returns True if supported
   */
  supports(path: string): boolean {
    const ext = path.toLowerCase();
    return ext.endsWith('.gltf') || ext.endsWith('.glb');
  }
}
