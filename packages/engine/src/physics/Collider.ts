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
 * Collider Shapes
 * Various collision shapes for physics bodies
 */

import Ammo from 'ammojs-typed';
import { Vector3 } from '../math/Vector3';

/**
 * Base collider class
 */
export abstract class Collider {
  protected shape: Ammo.btCollisionShape | null = null;

  /**
   * Get the Ammo.js collision shape
   */
  public getShape(): Ammo.btCollisionShape | null {
    return this.shape;
  }

  /**
   * Create the collision shape
   */
  public abstract createShape(ammo: typeof Ammo): void;

  /**
   * Destroy the collision shape
   */
  public destroy(ammo: typeof Ammo): void {
    if (this.shape) {
      ammo.destroy(this.shape);
      this.shape = null;
    }
  }
}

/**
 * Box collider
 */
export class BoxCollider extends Collider {
  private halfExtents: Vector3;

  /**
   * Create a box collider
   * @param halfExtents - Half extents of the box (width/2, height/2, depth/2)
   */
  constructor(halfExtents: Vector3 = new Vector3(0.5, 0.5, 0.5)) {
    super();
    this.halfExtents = halfExtents;
  }

  public createShape(ammo: typeof Ammo): void {
    const btHalfExtents = new ammo.btVector3(
      this.halfExtents.x,
      this.halfExtents.y,
      this.halfExtents.z
    );
    this.shape = new ammo.btBoxShape(btHalfExtents);
    ammo.destroy(btHalfExtents);
  }

  /**
   * Set half extents
   */
  public setHalfExtents(halfExtents: Vector3): void {
    this.halfExtents = halfExtents;
  }

  /**
   * Get half extents
   */
  public getHalfExtents(): Vector3 {
    return this.halfExtents.clone();
  }
}

/**
 * Sphere collider
 */
export class SphereCollider extends Collider {
  private radius: number;

  /**
   * Create a sphere collider
   * @param radius - Radius of the sphere
   */
  constructor(radius: number = 0.5) {
    super();
    this.radius = radius;
  }

  public createShape(ammo: typeof Ammo): void {
    this.shape = new ammo.btSphereShape(this.radius);
  }

  /**
   * Set radius
   */
  public setRadius(radius: number): void {
    this.radius = radius;
  }

  /**
   * Get radius
   */
  public getRadius(): number {
    return this.radius;
  }
}

/**
 * Capsule collider (cylinder with hemispherical ends)
 */
export class CapsuleCollider extends Collider {
  private radius: number;
  private height: number;
  private axis: number; // 0=X, 1=Y, 2=Z

  /**
   * Create a capsule collider
   * @param radius - Radius of the capsule
   * @param height - Height of the cylindrical part
   * @param axis - Axis along which the capsule is aligned (0=X, 1=Y, 2=Z)
   */
  constructor(radius: number = 0.5, height: number = 1.0, axis: number = 1) {
    super();
    this.radius = radius;
    this.height = height;
    this.axis = axis;
  }

  public createShape(ammo: typeof Ammo): void {
    // Ammo capsule is aligned on Y axis by default
    if (this.axis === 0) {
      this.shape = new ammo.btCapsuleShapeX(this.radius, this.height);
    } else if (this.axis === 2) {
      this.shape = new ammo.btCapsuleShapeZ(this.radius, this.height);
    } else {
      this.shape = new ammo.btCapsuleShape(this.radius, this.height);
    }
  }

  /**
   * Set radius
   */
  public setRadius(radius: number): void {
    this.radius = radius;
  }

  /**
   * Get radius
   */
  public getRadius(): number {
    return this.radius;
  }

  /**
   * Set height
   */
  public setHeight(height: number): void {
    this.height = height;
  }

  /**
   * Get height
   */
  public getHeight(): number {
    return this.height;
  }
}

/**
 * Cylinder collider
 */
export class CylinderCollider extends Collider {
  private halfExtents: Vector3;
  private axis: number; // 0=X, 1=Y, 2=Z

  /**
   * Create a cylinder collider
   * @param halfExtents - Half extents (radius, halfHeight, radius for Y-axis)
   * @param axis - Axis along which the cylinder is aligned (0=X, 1=Y, 2=Z)
   */
  constructor(halfExtents: Vector3 = new Vector3(0.5, 1.0, 0.5), axis: number = 1) {
    super();
    this.halfExtents = halfExtents;
    this.axis = axis;
  }

  public createShape(ammo: typeof Ammo): void {
    const btHalfExtents = new ammo.btVector3(
      this.halfExtents.x,
      this.halfExtents.y,
      this.halfExtents.z
    );

    if (this.axis === 0) {
      this.shape = new ammo.btCylinderShapeX(btHalfExtents);
    } else if (this.axis === 2) {
      this.shape = new ammo.btCylinderShapeZ(btHalfExtents);
    } else {
      this.shape = new ammo.btCylinderShape(btHalfExtents);
    }

    ammo.destroy(btHalfExtents);
  }

  /**
   * Set half extents
   */
  public setHalfExtents(halfExtents: Vector3): void {
    this.halfExtents = halfExtents;
  }

  /**
   * Get half extents
   */
  public getHalfExtents(): Vector3 {
    return this.halfExtents.clone();
  }
}

/**
 * Cone collider
 */
export class ConeCollider extends Collider {
  private radius: number;
  private height: number;
  private axis: number; // 0=X, 1=Y, 2=Z

  /**
   * Create a cone collider
   * @param radius - Radius of the cone base
   * @param height - Height of the cone
   * @param axis - Axis along which the cone points (0=X, 1=Y, 2=Z)
   */
  constructor(radius: number = 0.5, height: number = 1.0, axis: number = 1) {
    super();
    this.radius = radius;
    this.height = height;
    this.axis = axis;
  }

  public createShape(ammo: typeof Ammo): void {
    if (this.axis === 0) {
      this.shape = new ammo.btConeShapeX(this.radius, this.height);
    } else if (this.axis === 2) {
      this.shape = new ammo.btConeShapeZ(this.radius, this.height);
    } else {
      this.shape = new ammo.btConeShape(this.radius, this.height);
    }
  }

  /**
   * Set radius
   */
  public setRadius(radius: number): void {
    this.radius = radius;
  }

  /**
   * Get radius
   */
  public getRadius(): number {
    return this.radius;
  }

  /**
   * Set height
   */
  public setHeight(height: number): void {
    this.height = height;
  }

  /**
   * Get height
   */
  public getHeight(): number {
    return this.height;
  }
}

/**
 * Plane collider (infinite static plane)
 */
export class PlaneCollider extends Collider {
  private normal: Vector3;
  private planeConstant: number;

  /**
   * Create a plane collider
   * @param normal - Normal vector of the plane
   * @param planeConstant - Distance from origin along normal
   */
  constructor(normal: Vector3 = new Vector3(0, 1, 0), planeConstant: number = 0) {
    super();
    this.normal = normal.normalize();
    this.planeConstant = planeConstant;
  }

  public createShape(ammo: typeof Ammo): void {
    const btNormal = new ammo.btVector3(this.normal.x, this.normal.y, this.normal.z);
    this.shape = new ammo.btStaticPlaneShape(btNormal, this.planeConstant);
    ammo.destroy(btNormal);
  }

  /**
   * Set normal
   */
  public setNormal(normal: Vector3): void {
    this.normal = normal.normalize();
  }

  /**
   * Get normal
   */
  public getNormal(): Vector3 {
    return this.normal.clone();
  }

  /**
   * Set plane constant
   */
  public setPlaneConstant(constant: number): void {
    this.planeConstant = constant;
  }

  /**
   * Get plane constant
   */
  public getPlaneConstant(): number {
    return this.planeConstant;
  }
}

/**
 * Convex hull collider (from arbitrary points)
 */
export class ConvexHullCollider extends Collider {
  private points: Vector3[];

  /**
   * Create a convex hull collider
   * @param points - Array of points defining the convex hull
   */
  constructor(points: Vector3[] = []) {
    super();
    this.points = points;
  }

  public createShape(ammo: typeof Ammo): void {
    this.shape = new ammo.btConvexHullShape();

    for (const point of this.points) {
      const btPoint = new ammo.btVector3(point.x, point.y, point.z);
      (this.shape as Ammo.btConvexHullShape).addPoint(btPoint, true);
      ammo.destroy(btPoint);
    }
  }

  /**
   * Set points
   */
  public setPoints(points: Vector3[]): void {
    this.points = points;
  }

  /**
   * Get points
   */
  public getPoints(): Vector3[] {
    return [...this.points];
  }

  /**
   * Add a point
   */
  public addPoint(point: Vector3): void {
    this.points.push(point);
  }
}

/**
 * Compound collider (multiple shapes combined)
 */
export class CompoundCollider extends Collider {
  private children: Array<{
    collider: Collider;
    position: Vector3;
    rotation: { x: number; y: number; z: number; w: number };
  }> = [];

  /**
   * Create a compound collider
   */
  constructor() {
    super();
  }

  public createShape(ammo: typeof Ammo): void {
    this.shape = new ammo.btCompoundShape();

    for (const child of this.children) {
      // Create child shape if not already created
      if (!child.collider.getShape()) {
        child.collider.createShape(ammo);
      }

      const childShape = child.collider.getShape();
      if (!childShape) continue;

      // Create transform for child
      const transform = new ammo.btTransform();
      transform.setIdentity();
      transform.setOrigin(
        new ammo.btVector3(child.position.x, child.position.y, child.position.z)
      );
      transform.setRotation(
        new ammo.btQuaternion(
          child.rotation.x,
          child.rotation.y,
          child.rotation.z,
          child.rotation.w
        )
      );

      (this.shape as Ammo.btCompoundShape).addChildShape(transform, childShape);

      ammo.destroy(transform);
    }
  }

  /**
   * Add a child collider
   * @param collider - Child collider
   * @param position - Local position
   * @param rotation - Local rotation (quaternion)
   */
  public addChild(
    collider: Collider,
    position: Vector3 = new Vector3(),
    rotation: { x: number; y: number; z: number; w: number } = { x: 0, y: 0, z: 0, w: 1 }
  ): void {
    this.children.push({ collider, position, rotation });
  }

  /**
   * Remove a child collider
   */
  public removeChild(collider: Collider): void {
    const index = this.children.findIndex((child) => child.collider === collider);
    if (index !== -1) {
      this.children.splice(index, 1);
    }
  }

  /**
   * Get all children
   */
  public getChildren(): Array<{
    collider: Collider;
    position: Vector3;
    rotation: { x: number; y: number; z: number; w: number };
  }> {
    return [...this.children];
  }

  public override destroy(ammo: typeof Ammo): void {
    // Destroy children first
    for (const child of this.children) {
      child.collider.destroy(ammo);
    }
    this.children = [];

    // Destroy compound shape
    super.destroy(ammo);
  }
}

/**
 * Triangle mesh collider (for complex static geometry)
 */
export class TriangleMeshCollider extends Collider {
  private vertices: Float32Array;
  private indices: Uint32Array;

  /**
   * Create a triangle mesh collider
   * @param vertices - Vertex positions (x,y,z,x,y,z,...)
   * @param indices - Triangle indices
   */
  constructor(vertices: Float32Array = new Float32Array(), indices: Uint32Array = new Uint32Array()) {
    super();
    this.vertices = vertices;
    this.indices = indices;
  }

  public createShape(ammo: typeof Ammo): void {
    const triangleMesh = new ammo.btTriangleMesh(true, false);

    // Add triangles
    for (let i = 0; i < this.indices.length; i += 3) {
      const i0 = this.indices[i]! * 3;
      const i1 = this.indices[i + 1]! * 3;
      const i2 = this.indices[i + 2]! * 3;

      const v0 = new ammo.btVector3(
        this.vertices[i0] || 0,
        this.vertices[i0 + 1] || 0,
        this.vertices[i0 + 2] || 0
      );
      const v1 = new ammo.btVector3(
        this.vertices[i1] || 0,
        this.vertices[i1 + 1] || 0,
        this.vertices[i1 + 2] || 0
      );
      const v2 = new ammo.btVector3(
        this.vertices[i2] || 0,
        this.vertices[i2 + 1] || 0,
        this.vertices[i2 + 2] || 0
      );

      triangleMesh.addTriangle(v0, v1, v2, true);

      ammo.destroy(v0);
      ammo.destroy(v1);
      ammo.destroy(v2);
    }

    // Create BVH triangle mesh shape for better performance
    this.shape = new ammo.btBvhTriangleMeshShape(triangleMesh, true, true);
  }

  /**
   * Set mesh data
   */
  public setMeshData(vertices: Float32Array, indices: Uint32Array): void {
    this.vertices = vertices;
    this.indices = indices;
  }
}
