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

/**
 * Represents a 4x4 matrix stored in column-major order
 */
export class Matrix4 {
  /**
   * Matrix elements stored in column-major order
   */
  public elements: Float32Array;

  /**
   * Creates a new Matrix4
   * @param elements - Optional array of 16 elements
   */
  constructor(elements?: number[]) {
    this.elements = new Float32Array(16);
    if (elements) {
      this.elements.set(elements);
    } else {
      this.identity();
    }
  }

  /**
   * Creates an identity matrix
   */
  static identity(): Matrix4 {
    return new Matrix4();
  }

  /**
   * Sets this matrix to the identity matrix
   * @returns This matrix for chaining
   */
  identity(): Matrix4 {
    const e = this.elements;
    e[0]! = 1;
    e[1]! = 0;
    e[2]! = 0;
    e[3]! = 0;
    e[4]! = 0;
    e[5]! = 1;
    e[6]! = 0;
    e[7]! = 0;
    e[8]! = 0;
    e[9]! = 0;
    e[10]! = 1;
    e[11]! = 0;
    e[12]! = 0;
    e[13]! = 0;
    e[14]! = 0;
    e[15]! = 1;
    return this;
  }

  /**
   * Multiplies this matrix by another matrix
   * @param m - The matrix to multiply by
   * @returns A new matrix representing the product
   */
  multiply(m: Matrix4): Matrix4 {
    const a = this.elements;
    const b = m.elements;
    const result = new Matrix4();
    const r = result.elements;

    const a00 = a[0]!,
      a01 = a[1]!,
      a02 = a[2]!,
      a03 = a[3]!;
    const a10 = a[4]!,
      a11 = a[5]!,
      a12 = a[6]!,
      a13 = a[7]!;
    const a20 = a[8]!,
      a21 = a[9]!,
      a22 = a[10]!,
      a23 = a[11]!;
    const a30 = a[12]!,
      a31 = a[13]!,
      a32 = a[14]!,
      a33 = a[15]!;

    const b00 = b[0]!,
      b01 = b[1]!,
      b02 = b[2]!,
      b03 = b[3]!;
    const b10 = b[4]!,
      b11 = b[5]!,
      b12 = b[6]!,
      b13 = b[7]!;
    const b20 = b[8]!,
      b21 = b[9]!,
      b22 = b[10]!,
      b23 = b[11]!;
    const b30 = b[12]!,
      b31 = b[13]!,
      b32 = b[14]!,
      b33 = b[15]!;

    r[0]! = a00 * b00 + a10 * b01 + a20 * b02 + a30 * b03;
    r[1]! = a01 * b00 + a11 * b01 + a21 * b02 + a31 * b03;
    r[2]! = a02 * b00 + a12 * b01 + a22 * b02 + a32 * b03;
    r[3]! = a03 * b00 + a13 * b01 + a23 * b02 + a33 * b03;

    r[4]! = a00 * b10 + a10 * b11 + a20 * b12 + a30 * b13;
    r[5]! = a01 * b10 + a11 * b11 + a21 * b12 + a31 * b13;
    r[6]! = a02 * b10 + a12 * b11 + a22 * b12 + a32 * b13;
    r[7]! = a03 * b10 + a13 * b11 + a23 * b12 + a33 * b13;

    r[8]! = a00 * b20 + a10 * b21 + a20 * b22 + a30 * b23;
    r[9]! = a01 * b20 + a11 * b21 + a21 * b22 + a31 * b23;
    r[10]! = a02 * b20 + a12 * b21 + a22 * b22 + a32 * b23;
    r[11]! = a03 * b20 + a13 * b21 + a23 * b22 + a33 * b23;

    r[12]! = a00 * b30 + a10 * b31 + a20 * b32 + a30 * b33;
    r[13]! = a01 * b30 + a11 * b31 + a21 * b32 + a31 * b33;
    r[14]! = a02 * b30 + a12 * b31 + a22 * b32 + a32 * b33;
    r[15]! = a03 * b30 + a13 * b31 + a23 * b32 + a33 * b33;

    return result;
  }

  /**
   * Creates a translation matrix
   * @param v - Translation vector
   * @returns A new translation matrix
   */
  static translation(v: Vector3): Matrix4 {
    const m = new Matrix4();
    m.elements[12] = v.x;
    m.elements[13] = v.y;
    m.elements[14] = v.z;
    return m;
  }

  /**
   * Translates this matrix by a vector
   * @param v - Translation vector
   * @returns A new translated matrix
   */
  translate(v: Vector3): Matrix4 {
    return this.multiply(Matrix4.translation(v));
  }

  /**
   * Creates a rotation matrix around an axis
   * @param angle - Rotation angle in radians
   * @param axis - Rotation axis (must be normalized)
   * @returns A new rotation matrix
   */
  static rotation(angle: number, axis: Vector3): Matrix4 {
    const m = new Matrix4();
    const c = Math.cos(angle);
    const s = Math.sin(angle);
    const t = 1 - c;
    const x = axis.x;
    const y = axis.y;
    const z = axis.z;

    m.elements[0] = t * x * x + c;
    m.elements[1] = t * x * y + s * z;
    m.elements[2] = t * x * z - s * y;
    m.elements[3] = 0;

    m.elements[4] = t * x * y - s * z;
    m.elements[5] = t * y * y + c;
    m.elements[6] = t * y * z + s * x;
    m.elements[7] = 0;

    m.elements[8] = t * x * z + s * y;
    m.elements[9] = t * y * z - s * x;
    m.elements[10] = t * z * z + c;
    m.elements[11] = 0;

    m.elements[12] = 0;
    m.elements[13] = 0;
    m.elements[14] = 0;
    m.elements[15] = 1;

    return m;
  }

  /**
   * Rotates this matrix around an axis
   * @param angle - Rotation angle in radians
   * @param axis - Rotation axis (must be normalized)
   * @returns A new rotated matrix
   */
  rotate(angle: number, axis: Vector3): Matrix4 {
    return this.multiply(Matrix4.rotation(angle, axis));
  }

  /**
   * Creates a scaling matrix
   * @param v - Scale vector
   * @returns A new scaling matrix
   */
  static scaling(v: Vector3): Matrix4 {
    const m = new Matrix4();
    m.elements[0] = v.x;
    m.elements[5] = v.y;
    m.elements[10] = v.z;
    return m;
  }

  /**
   * Scales this matrix by a vector
   * @param v - Scale vector
   * @returns A new scaled matrix
   */
  scale(v: Vector3): Matrix4 {
    return this.multiply(Matrix4.scaling(v));
  }

  /**
   * Inverts this matrix
   * @returns A new inverted matrix, or null if not invertible
   */
  invert(): Matrix4 | null {
    const e = this.elements;
    const result = new Matrix4();
    const r = result.elements;

    const n11 = e[0]!,
      n12 = e[4]!,
      n13 = e[8]!,
      n14 = e[12]!;
    const n21 = e[1]!,
      n22 = e[5]!,
      n23 = e[9]!,
      n24 = e[13]!;
    const n31 = e[2]!,
      n32 = e[6]!,
      n33 = e[10]!,
      n34 = e[14]!;
    const n41 = e[3]!,
      n42 = e[7]!,
      n43 = e[11]!,
      n44 = e[15]!;

    r[0]! =
      n23 * n34 * n42 -
      n24 * n33 * n42 +
      n24 * n32 * n43 -
      n22 * n34 * n43 -
      n23 * n32 * n44 +
      n22 * n33 * n44;
    r[4]! =
      n14 * n33 * n42 -
      n13 * n34 * n42 -
      n14 * n32 * n43 +
      n12 * n34 * n43 +
      n13 * n32 * n44 -
      n12 * n33 * n44;
    r[8]! =
      n13 * n24 * n42 -
      n14 * n23 * n42 +
      n14 * n22 * n43 -
      n12 * n24 * n43 -
      n13 * n22 * n44 +
      n12 * n23 * n44;
    r[12]! =
      n14 * n23 * n32 -
      n13 * n24 * n32 -
      n14 * n22 * n33 +
      n12 * n24 * n33 +
      n13 * n22 * n34 -
      n12 * n23 * n34;

    const det = n11 * r[0]! + n21 * r[4]! + n31 * r[8]! + n41 * r[12]!;

    if (det === 0) {
      return null;
    }

    const detInv = 1 / det;

    r[0]! *= detInv;
    r[4]! *= detInv;
    r[8]! *= detInv;
    r[12]! *= detInv;

    r[1]! =
      (n24 * n33 * n41 -
        n23 * n34 * n41 -
        n24 * n31 * n43 +
        n21 * n34 * n43 +
        n23 * n31 * n44 -
        n21 * n33 * n44) *
      detInv;
    r[5]! =
      (n13 * n34 * n41 -
        n14 * n33 * n41 +
        n14 * n31 * n43 -
        n11 * n34 * n43 -
        n13 * n31 * n44 +
        n11 * n33 * n44) *
      detInv;
    r[9]! =
      (n14 * n23 * n41 -
        n13 * n24 * n41 -
        n14 * n21 * n43 +
        n11 * n24 * n43 +
        n13 * n21 * n44 -
        n11 * n23 * n44) *
      detInv;
    r[13]! =
      (n13 * n24 * n31 -
        n14 * n23 * n31 +
        n14 * n21 * n33 -
        n11 * n24 * n33 -
        n13 * n21 * n34 +
        n11 * n23 * n34) *
      detInv;

    r[2]! =
      (n22 * n34 * n41 -
        n24 * n32 * n41 +
        n24 * n31 * n42 -
        n21 * n34 * n42 -
        n22 * n31 * n44 +
        n21 * n32 * n44) *
      detInv;
    r[6]! =
      (n14 * n32 * n41 -
        n12 * n34 * n41 -
        n14 * n31 * n42 +
        n11 * n34 * n42 +
        n12 * n31 * n44 -
        n11 * n32 * n44) *
      detInv;
    r[10]! =
      (n12 * n24 * n41 -
        n14 * n22 * n41 +
        n14 * n21 * n42 -
        n11 * n24 * n42 -
        n12 * n21 * n44 +
        n11 * n22 * n44) *
      detInv;
    r[14]! =
      (n14 * n22 * n31 -
        n12 * n24 * n31 -
        n14 * n21 * n32 +
        n11 * n24 * n32 +
        n12 * n21 * n34 -
        n11 * n22 * n34) *
      detInv;

    r[3]! =
      (n23 * n32 * n41 -
        n22 * n33 * n41 -
        n23 * n31 * n42 +
        n21 * n33 * n42 +
        n22 * n31 * n43 -
        n21 * n32 * n43) *
      detInv;
    r[7]! =
      (n12 * n33 * n41 -
        n13 * n32 * n41 +
        n13 * n31 * n42 -
        n11 * n33 * n42 -
        n12 * n31 * n43 +
        n11 * n32 * n43) *
      detInv;
    r[11]! =
      (n13 * n22 * n41 -
        n12 * n23 * n41 -
        n13 * n21 * n42 +
        n11 * n23 * n42 +
        n12 * n21 * n43 -
        n11 * n22 * n43) *
      detInv;
    r[15]! =
      (n12 * n23 * n31 -
        n13 * n22 * n31 +
        n13 * n21 * n32 -
        n11 * n23 * n32 -
        n12 * n21 * n33 +
        n11 * n22 * n33) *
      detInv;

    return result;
  }

  /**
   * Transposes this matrix
   * @returns A new transposed matrix
   */
  transpose(): Matrix4 {
    const e = this.elements;
    const result = new Matrix4();
    const r = result.elements;

    r[0]! = e[0]!;
    r[1]! = e[4]!;
    r[2]! = e[8]!;
    r[3]! = e[12]!;
    r[4]! = e[1]!;
    r[5]! = e[5]!;
    r[6]! = e[9]!;
    r[7]! = e[13]!;
    r[8]! = e[2]!;
    r[9]! = e[6]!;
    r[10]! = e[10]!;
    r[11]! = e[14]!;
    r[12]! = e[3]!;
    r[13]! = e[7]!;
    r[14]! = e[11]!;
    r[15]! = e[15]!;

    return result;
  }

  /**
   * Creates a perspective projection matrix
   * @param fov - Field of view in radians
   * @param aspect - Aspect ratio (width / height)
   * @param near - Near clipping plane
   * @param far - Far clipping plane
   * @returns A new perspective matrix
   */
  static perspective(
    fov: number,
    aspect: number,
    near: number,
    far: number
  ): Matrix4 {
    const m = new Matrix4();
    const f = 1.0 / Math.tan(fov / 2);
    const rangeInv = 1 / (near - far);

    m.elements[0] = f / aspect;
    m.elements[5] = f;
    m.elements[10] = (near + far) * rangeInv;
    m.elements[11] = -1;
    m.elements[14] = near * far * rangeInv * 2;
    m.elements[15] = 0;

    return m;
  }

  /**
   * Creates an orthographic projection matrix
   * @param left - Left plane
   * @param right - Right plane
   * @param bottom - Bottom plane
   * @param top - Top plane
   * @param near - Near plane
   * @param far - Far plane
   * @returns A new orthographic matrix
   */
  static orthographic(
    left: number,
    right: number,
    bottom: number,
    top: number,
    near: number,
    far: number
  ): Matrix4 {
    const m = new Matrix4();
    const w = right - left;
    const h = top - bottom;
    const d = far - near;

    m.elements[0] = 2 / w;
    m.elements[5] = 2 / h;
    m.elements[10] = -2 / d;
    m.elements[12] = -(right + left) / w;
    m.elements[13] = -(top + bottom) / h;
    m.elements[14] = -(far + near) / d;

    return m;
  }

  /**
   * Creates a look-at view matrix
   * @param eye - Eye position
   * @param target - Target position
   * @param up - Up vector
   * @returns A new look-at matrix
   */
  static lookAt(eye: Vector3, target: Vector3, up: Vector3): Matrix4 {
    const z = eye.subtract(target).normalize();
    const x = up.cross(z).normalize();
    const y = z.cross(x);

    const m = new Matrix4();
    m.elements[0] = x.x;
    m.elements[4] = x.y;
    m.elements[8] = x.z;
    m.elements[1] = y.x;
    m.elements[5] = y.y;
    m.elements[9] = y.z;
    m.elements[2] = z.x;
    m.elements[6] = z.y;
    m.elements[10] = z.z;
    m.elements[12] = -x.dot(eye);
    m.elements[13] = -y.dot(eye);
    m.elements[14] = -z.dot(eye);

    return m;
  }

  /**
   * Creates a copy of this matrix
   * @returns A new matrix with the same elements
   */
  clone(): Matrix4 {
    return new Matrix4(Array.from(this.elements));
  }

  /**
   * Checks if this matrix equals another matrix
   * @param m - The other matrix
   * @param epsilon - Tolerance for floating point comparison
   * @returns True if matrices are equal
   */
  equals(m: Matrix4, epsilon: number = 0): boolean {
    const a = this.elements;
    const b = m.elements;
    for (let i = 0; i < 16; i++) {
      if (Math.abs(a[i]! - b[i]!) > epsilon) {
        return false;
      }
    }
    return true;
  }
}
