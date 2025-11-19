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
 * Primitives
 *
 * Factory functions for creating basic 3D shapes (primitives).
 * Includes cube, sphere, plane, and cylinder.
 */

import { Mesh } from './Mesh';

/**
 * Create a cube mesh
 * @param gl - WebGL context
 * @param size - Cube size (default: 1)
 * @returns Cube mesh
 */
export function createCube(gl: WebGL2RenderingContext, size: number = 1): Mesh {
  const s = size / 2;

  // Vertices (position)
  const vertices = new Float32Array([
    // Front face
    -s,
    -s,
    s,
    s,
    -s,
    s,
    s,
    s,
    s,
    -s,
    s,
    s,
    // Back face
    -s,
    -s,
    -s,
    -s,
    s,
    -s,
    s,
    s,
    -s,
    s,
    -s,
    -s,
    // Top face
    -s,
    s,
    -s,
    -s,
    s,
    s,
    s,
    s,
    s,
    s,
    s,
    -s,
    // Bottom face
    -s,
    -s,
    -s,
    s,
    -s,
    -s,
    s,
    -s,
    s,
    -s,
    -s,
    s,
    // Right face
    s,
    -s,
    -s,
    s,
    s,
    -s,
    s,
    s,
    s,
    s,
    -s,
    s,
    // Left face
    -s,
    -s,
    -s,
    -s,
    -s,
    s,
    -s,
    s,
    s,
    -s,
    s,
    -s,
  ]);

  // Indices
  const indices = new Uint16Array([
    0,
    1,
    2,
    0,
    2,
    3, // front
    4,
    5,
    6,
    4,
    6,
    7, // back
    8,
    9,
    10,
    8,
    10,
    11, // top
    12,
    13,
    14,
    12,
    14,
    15, // bottom
    16,
    17,
    18,
    16,
    18,
    19, // right
    20,
    21,
    22,
    20,
    22,
    23, // left
  ]);

  // Normals
  const normals = new Float32Array([
    // Front
    0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1,
    // Back
    0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1,
    // Top
    0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0,
    // Bottom
    0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0,
    // Right
    1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0,
    // Left
    -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0,
  ]);

  // UVs
  const uvs = new Float32Array([
    // Front
    0, 0, 1, 0, 1, 1, 0, 1,
    // Back
    1, 0, 1, 1, 0, 1, 0, 0,
    // Top
    0, 1, 0, 0, 1, 0, 1, 1,
    // Bottom
    1, 1, 0, 1, 0, 0, 1, 0,
    // Right
    1, 0, 1, 1, 0, 1, 0, 0,
    // Left
    0, 0, 1, 0, 1, 1, 0, 1,
  ]);

  const mesh = new Mesh(gl, vertices, indices, 'Cube');
  mesh.setNormals(normals);
  mesh.setUVs(uvs);

  return mesh;
}

/**
 * Create a plane mesh
 * @param gl - WebGL context
 * @param width - Plane width (default: 1)
 * @param height - Plane height (default: 1)
 * @param widthSegments - Number of width segments (default: 1)
 * @param heightSegments - Number of height segments (default: 1)
 * @returns Plane mesh
 */
export function createPlane(
  gl: WebGL2RenderingContext,
  width: number = 1,
  height: number = 1,
  widthSegments: number = 1,
  heightSegments: number = 1
): Mesh {
  const w = width / 2;
  const h = height / 2;
  const gridX = widthSegments;
  const gridY = heightSegments;
  const gridX1 = gridX + 1;
  const gridY1 = gridY + 1;

  const segmentWidth = width / gridX;
  const segmentHeight = height / gridY;

  // Build vertices, normals and uvs
  const vertices: number[] = [];
  const normals: number[] = [];
  const uvs: number[] = [];

  for (let iy = 0; iy < gridY1; iy++) {
    const y = iy * segmentHeight - h;
    for (let ix = 0; ix < gridX1; ix++) {
      const x = ix * segmentWidth - w;

      vertices.push(x, y, 0);
      normals.push(0, 0, 1);
      uvs.push(ix / gridX, 1 - iy / gridY);
    }
  }

  // Build indices
  const indices: number[] = [];

  for (let iy = 0; iy < gridY; iy++) {
    for (let ix = 0; ix < gridX; ix++) {
      const a = ix + gridX1 * iy;
      const b = ix + gridX1 * (iy + 1);
      const c = ix + 1 + gridX1 * (iy + 1);
      const d = ix + 1 + gridX1 * iy;

      indices.push(a, b, d);
      indices.push(b, c, d);
    }
  }

  const mesh = new Mesh(
    gl,
    new Float32Array(vertices),
    new Uint16Array(indices),
    'Plane'
  );
  mesh.setNormals(new Float32Array(normals));
  mesh.setUVs(new Float32Array(uvs));

  return mesh;
}

/**
 * Create a sphere mesh
 * @param gl - WebGL context
 * @param radius - Sphere radius (default: 0.5)
 * @param widthSegments - Number of horizontal segments (default: 32)
 * @param heightSegments - Number of vertical segments (default: 16)
 * @returns Sphere mesh
 */
export function createSphere(
  gl: WebGL2RenderingContext,
  radius: number = 0.5,
  widthSegments: number = 32,
  heightSegments: number = 16
): Mesh {
  const vertices: number[] = [];
  const normals: number[] = [];
  const uvs: number[] = [];
  const indices: number[] = [];

  for (let y = 0; y <= heightSegments; y++) {
    const v = y / heightSegments;
    const phi = v * Math.PI;

    for (let x = 0; x <= widthSegments; x++) {
      const u = x / widthSegments;
      const theta = u * Math.PI * 2;

      const sinPhi = Math.sin(phi);
      const cosPhi = Math.cos(phi);
      const sinTheta = Math.sin(theta);
      const cosTheta = Math.cos(theta);

      const px = radius * sinPhi * cosTheta;
      const py = radius * cosPhi;
      const pz = radius * sinPhi * sinTheta;

      vertices.push(px, py, pz);
      normals.push(sinPhi * cosTheta, cosPhi, sinPhi * sinTheta);
      uvs.push(u, 1 - v);
    }
  }

  for (let y = 0; y < heightSegments; y++) {
    for (let x = 0; x < widthSegments; x++) {
      const a = y * (widthSegments + 1) + x;
      const b = a + widthSegments + 1;
      const c = a + widthSegments + 2;
      const d = a + 1;

      indices.push(a, b, d);
      indices.push(b, c, d);
    }
  }

  const mesh = new Mesh(
    gl,
    new Float32Array(vertices),
    new Uint16Array(indices),
    'Sphere'
  );
  mesh.setNormals(new Float32Array(normals));
  mesh.setUVs(new Float32Array(uvs));

  return mesh;
}

/**
 * Create a cylinder mesh
 * @param gl - WebGL context
 * @param radiusTop - Top radius (default: 0.5)
 * @param radiusBottom - Bottom radius (default: 0.5)
 * @param height - Cylinder height (default: 1)
 * @param radialSegments - Number of radial segments (default: 32)
 * @param heightSegments - Number of height segments (default: 1)
 * @returns Cylinder mesh
 */
export function createCylinder(
  gl: WebGL2RenderingContext,
  radiusTop: number = 0.5,
  radiusBottom: number = 0.5,
  height: number = 1,
  radialSegments: number = 32,
  heightSegments: number = 1
): Mesh {
  const vertices: number[] = [];
  const normals: number[] = [];
  const uvs: number[] = [];
  const indices: number[] = [];

  const halfHeight = height / 2;

  // Generate vertices, normals, and uvs
  for (let y = 0; y <= heightSegments; y++) {
    const v = y / heightSegments;
    const radius = v * (radiusBottom - radiusTop) + radiusTop;
    const py = -v * height + halfHeight;

    for (let x = 0; x <= radialSegments; x++) {
      const u = x / radialSegments;
      const theta = u * Math.PI * 2;

      const sinTheta = Math.sin(theta);
      const cosTheta = Math.cos(theta);

      const px = radius * sinTheta;
      const pz = radius * cosTheta;

      vertices.push(px, py, pz);
      normals.push(sinTheta, 0, cosTheta);
      uvs.push(u, 1 - v);
    }
  }

  // Generate indices
  for (let y = 0; y < heightSegments; y++) {
    for (let x = 0; x < radialSegments; x++) {
      const a = y * (radialSegments + 1) + x;
      const b = a + radialSegments + 1;
      const c = a + radialSegments + 2;
      const d = a + 1;

      indices.push(a, b, d);
      indices.push(b, c, d);
    }
  }

  const mesh = new Mesh(
    gl,
    new Float32Array(vertices),
    new Uint16Array(indices),
    'Cylinder'
  );
  mesh.setNormals(new Float32Array(normals));
  mesh.setUVs(new Float32Array(uvs));

  return mesh;
}

/**
 * Create a cone mesh
 * @param gl - WebGL context
 * @param radius - Base radius (default: 0.5)
 * @param height - Cone height (default: 1)
 * @param radialSegments - Number of radial segments (default: 32)
 * @returns Cone mesh
 */
export function createCone(
  gl: WebGL2RenderingContext,
  radius: number = 0.5,
  height: number = 1,
  radialSegments: number = 32
): Mesh {
  return createCylinder(gl, 0, radius, height, radialSegments, 1);
}
