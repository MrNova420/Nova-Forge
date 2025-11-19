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

import { Vector3 } from './Vector3';
import { Matrix4 } from './Matrix4';

/**
 * Represents a quaternion for 3D rotations
 */
export class Quaternion {
  /**
   * Creates a new Quaternion
   * @param x - The x component
   * @param y - The y component
   * @param z - The z component
   * @param w - The w component
   */
  constructor(
    public x: number = 0,
    public y: number = 0,
    public z: number = 0,
    public w: number = 1
  ) {}

  /**
   * Creates an identity quaternion
   */
  static identity(): Quaternion {
    return new Quaternion(0, 0, 0, 1);
  }

  /**
   * Creates a quaternion from axis-angle representation
   * @param axis - The rotation axis (must be normalized)
   * @param angle - The rotation angle in radians
   * @returns A new quaternion
   */
  static fromAxisAngle(axis: Vector3, angle: number): Quaternion {
    const halfAngle = angle / 2;
    const s = Math.sin(halfAngle);
    return new Quaternion(
      axis.x * s,
      axis.y * s,
      axis.z * s,
      Math.cos(halfAngle)
    );
  }

  /**
   * Creates a quaternion from Euler angles (in radians)
   * @param x - Rotation around X axis
   * @param y - Rotation around Y axis
   * @param z - Rotation around Z axis
   * @returns A new quaternion
   */
  static fromEuler(x: number, y: number, z: number): Quaternion {
    const cx = Math.cos(x / 2);
    const cy = Math.cos(y / 2);
    const cz = Math.cos(z / 2);
    const sx = Math.sin(x / 2);
    const sy = Math.sin(y / 2);
    const sz = Math.sin(z / 2);

    return new Quaternion(
      sx * cy * cz + cx * sy * sz,
      cx * sy * cz - sx * cy * sz,
      cx * cy * sz + sx * sy * cz,
      cx * cy * cz - sx * sy * sz
    );
  }

  /**
   * Multiplies this quaternion by another
   * @param q - The quaternion to multiply by
   * @returns A new quaternion representing the product
   */
  multiply(q: Quaternion): Quaternion {
    const ax = this.x,
      ay = this.y,
      az = this.z,
      aw = this.w;
    const bx = q.x,
      by = q.y,
      bz = q.z,
      bw = q.w;

    return new Quaternion(
      ax * bw + aw * bx + ay * bz - az * by,
      ay * bw + aw * by + az * bx - ax * bz,
      az * bw + aw * bz + ax * by - ay * bx,
      aw * bw - ax * bx - ay * by - az * bz
    );
  }

  /**
   * Calculates the length (magnitude) of this quaternion
   * @returns The length
   */
  length(): number {
    return Math.sqrt(
      this.x * this.x + this.y * this.y + this.z * this.z + this.w * this.w
    );
  }

  /**
   * Returns a normalized copy of this quaternion
   * @returns A new normalized quaternion
   */
  normalize(): Quaternion {
    const len = this.length();
    if (len === 0) {
      return Quaternion.identity();
    }
    return new Quaternion(
      this.x / len,
      this.y / len,
      this.z / len,
      this.w / len
    );
  }

  /**
   * Returns the conjugate of this quaternion
   * @returns A new conjugate quaternion
   */
  conjugate(): Quaternion {
    return new Quaternion(-this.x, -this.y, -this.z, this.w);
  }

  /**
   * Returns the inverse of this quaternion
   * @returns A new inverse quaternion
   */
  inverse(): Quaternion {
    const lengthSq =
      this.x * this.x + this.y * this.y + this.z * this.z + this.w * this.w;
    if (lengthSq === 0) {
      return Quaternion.identity();
    }
    const invLengthSq = 1 / lengthSq;
    return new Quaternion(
      -this.x * invLengthSq,
      -this.y * invLengthSq,
      -this.z * invLengthSq,
      this.w * invLengthSq
    );
  }

  /**
   * Calculates the dot product with another quaternion
   * @param q - The other quaternion
   * @returns The dot product
   */
  dot(q: Quaternion): number {
    return this.x * q.x + this.y * q.y + this.z * q.z + this.w * q.w;
  }

  /**
   * Spherically interpolates between this quaternion and another
   * @param q - The target quaternion
   * @param t - The interpolation factor (0-1)
   * @returns A new interpolated quaternion
   */
  slerp(q: Quaternion, t: number): Quaternion {
    let dot = this.dot(q);

    // If the dot product is negative, slerp won't take the shorter path
    let qTemp = q;
    if (dot < 0) {
      dot = -dot;
      qTemp = new Quaternion(-q.x, -q.y, -q.z, -q.w);
    }

    // If quaternions are very close, use linear interpolation
    if (dot > 0.9995) {
      return new Quaternion(
        this.x + t * (qTemp.x - this.x),
        this.y + t * (qTemp.y - this.y),
        this.z + t * (qTemp.z - this.z),
        this.w + t * (qTemp.w - this.w)
      ).normalize();
    }

    const theta = Math.acos(dot);
    const sinTheta = Math.sin(theta);
    const a = Math.sin((1 - t) * theta) / sinTheta;
    const b = Math.sin(t * theta) / sinTheta;

    return new Quaternion(
      a * this.x + b * qTemp.x,
      a * this.y + b * qTemp.y,
      a * this.z + b * qTemp.z,
      a * this.w + b * qTemp.w
    );
  }

  /**
   * Converts this quaternion to a rotation matrix
   * @returns A new Matrix4 representing the rotation
   */
  toMatrix4(): Matrix4 {
    const x = this.x,
      y = this.y,
      z = this.z,
      w = this.w;
    const x2 = x + x,
      y2 = y + y,
      z2 = z + z;
    const xx = x * x2,
      xy = x * y2,
      xz = x * z2;
    const yy = y * y2,
      yz = y * z2,
      zz = z * z2;
    const wx = w * x2,
      wy = w * y2,
      wz = w * z2;

    const m = Matrix4.identity();
    m.elements[0] = 1 - (yy + zz);
    m.elements[1] = xy + wz;
    m.elements[2] = xz - wy;
    m.elements[4] = xy - wz;
    m.elements[5] = 1 - (xx + zz);
    m.elements[6] = yz + wx;
    m.elements[8] = xz + wy;
    m.elements[9] = yz - wx;
    m.elements[10] = 1 - (xx + yy);

    return m;
  }

  /**
   * Rotates a vector by this quaternion
   * @param v - The vector to rotate
   * @returns A new rotated vector
   */
  rotateVector(v: Vector3): Vector3 {
    const qv = new Quaternion(v.x, v.y, v.z, 0);
    const result = this.multiply(qv).multiply(this.conjugate());
    return new Vector3(result.x, result.y, result.z);
  }

  /**
   * Creates a copy of this quaternion
   * @returns A new quaternion with the same components
   */
  clone(): Quaternion {
    return new Quaternion(this.x, this.y, this.z, this.w);
  }

  /**
   * Checks if this quaternion equals another quaternion
   * @param q - The other quaternion
   * @param epsilon - Tolerance for floating point comparison
   * @returns True if quaternions are equal
   */
  equals(q: Quaternion, epsilon: number = 0): boolean {
    return (
      Math.abs(this.x - q.x) <= epsilon &&
      Math.abs(this.y - q.y) <= epsilon &&
      Math.abs(this.z - q.z) <= epsilon &&
      Math.abs(this.w - q.w) <= epsilon
    );
  }

  /**
   * Converts this quaternion to a string
   * @returns String representation
   */
  toString(): string {
    return `Quaternion(${this.x}, ${this.y}, ${this.z}, ${this.w})`;
  }
}
