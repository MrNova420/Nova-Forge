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

import { Vector2 } from './Vector2';

/**
 * Represents a 3x3 matrix stored in column-major order
 */
export class Matrix3 {
  /**
   * Matrix elements stored in column-major order
   */
  public elements: Float32Array;

  /**
   * Creates a new Matrix3
   * @param elements - Optional array of 9 elements
   */
  constructor(elements?: number[]) {
    this.elements = new Float32Array(9);
    if (elements) {
      this.elements.set(elements);
    } else {
      this.identity();
    }
  }

  /**
   * Creates an identity matrix
   */
  static identity(): Matrix3 {
    return new Matrix3();
  }

  /**
   * Sets this matrix to the identity matrix
   * @returns This matrix for chaining
   */
  identity(): Matrix3 {
    const e = this.elements;
    e[0]! = 1;
    e[1]! = 0;
    e[2]! = 0;
    e[3]! = 0;
    e[4]! = 1;
    e[5]! = 0;
    e[6]! = 0;
    e[7]! = 0;
    e[8]! = 1;
    return this;
  }

  /**
   * Multiplies this matrix by another matrix
   * @param m - The matrix to multiply by
   * @returns A new matrix representing the product
   */
  multiply(m: Matrix3): Matrix3 {
    const a = this.elements;
    const b = m.elements;
    const result = new Matrix3();
    const r = result.elements;

    const a00 = a[0]!,
      a01 = a[1]!,
      a02 = a[2]!;
    const a10 = a[3]!,
      a11 = a[4]!,
      a12 = a[5]!;
    const a20 = a[6]!,
      a21 = a[7]!,
      a22 = a[8]!;

    const b00 = b[0]!,
      b01 = b[1]!,
      b02 = b[2]!;
    const b10 = b[3]!,
      b11 = b[4]!,
      b12 = b[5]!;
    const b20 = b[6]!,
      b21 = b[7]!,
      b22 = b[8]!;

    r[0]! = a00 * b00 + a10 * b01 + a20 * b02;
    r[1]! = a01 * b00 + a11 * b01 + a21 * b02;
    r[2]! = a02 * b00 + a12 * b01 + a22 * b02;

    r[3]! = a00 * b10 + a10 * b11 + a20 * b12;
    r[4]! = a01 * b10 + a11 * b11 + a21 * b12;
    r[5]! = a02 * b10 + a12 * b11 + a22 * b12;

    r[6]! = a00 * b20 + a10 * b21 + a20 * b22;
    r[7]! = a01 * b20 + a11 * b21 + a21 * b22;
    r[8]! = a02 * b20 + a12 * b21 + a22 * b22;

    return result;
  }

  /**
   * Creates a translation matrix (2D)
   * @param v - Translation vector
   * @returns A new translation matrix
   */
  static translation(v: Vector2): Matrix3 {
    const m = new Matrix3();
    m.elements[6] = v.x;
    m.elements[7] = v.y;
    return m;
  }

  /**
   * Creates a rotation matrix (2D)
   * @param angle - Rotation angle in radians
   * @returns A new rotation matrix
   */
  static rotation(angle: number): Matrix3 {
    const c = Math.cos(angle);
    const s = Math.sin(angle);
    const m = new Matrix3();
    m.elements[0] = c;
    m.elements[1] = s;
    m.elements[3] = -s;
    m.elements[4] = c;
    return m;
  }

  /**
   * Creates a scaling matrix (2D)
   * @param v - Scale vector
   * @returns A new scaling matrix
   */
  static scaling(v: Vector2): Matrix3 {
    const m = new Matrix3();
    m.elements[0] = v.x;
    m.elements[4] = v.y;
    return m;
  }

  /**
   * Inverts this matrix
   * @returns A new inverted matrix, or null if not invertible
   */
  invert(): Matrix3 | null {
    const e = this.elements;
    const n11 = e[0]!,
      n12 = e[3]!,
      n13 = e[6]!;
    const n21 = e[1]!,
      n22 = e[4]!,
      n23 = e[7]!;
    const n31 = e[2]!,
      n32 = e[5]!,
      n33 = e[8]!;

    const t11 = n33 * n22 - n32 * n23;
    const t12 = n32 * n13 - n33 * n12;
    const t13 = n23 * n12 - n22 * n13;

    const det = n11 * t11 + n21 * t12 + n31 * t13;

    if (det === 0) {
      return null;
    }

    const detInv = 1 / det;
    const result = new Matrix3();
    const r = result.elements;

    r[0]! = t11 * detInv;
    r[1]! = (n31 * n23 - n33 * n21) * detInv;
    r[2]! = (n32 * n21 - n31 * n22) * detInv;

    r[3]! = t12 * detInv;
    r[4]! = (n33 * n11 - n31 * n13) * detInv;
    r[5]! = (n31 * n12 - n32 * n11) * detInv;

    r[6]! = t13 * detInv;
    r[7]! = (n21 * n13 - n23 * n11) * detInv;
    r[8]! = (n22 * n11 - n21 * n12) * detInv;

    return result;
  }

  /**
   * Transposes this matrix
   * @returns A new transposed matrix
   */
  transpose(): Matrix3 {
    const e = this.elements;
    const result = new Matrix3();
    const r = result.elements;

    r[0]! = e[0]!;
    r[1]! = e[3]!;
    r[2]! = e[6]!;
    r[3]! = e[1]!;
    r[4]! = e[4]!;
    r[5]! = e[7]!;
    r[6]! = e[2]!;
    r[7]! = e[5]!;
    r[8]! = e[8]!;

    return result;
  }

  /**
   * Calculates the determinant of this matrix
   * @returns The determinant
   */
  determinant(): number {
    const e = this.elements;
    const a = e[0]!,
      b = e[1]!,
      c = e[2]!;
    const d = e[3]!,
      e2 = e[4]!,
      f = e[5]!;
    const g = e[6]!,
      h = e[7]!,
      i = e[8]!;

    return (
      a * e2 * i - a * f * h - b * d * i + b * f * g + c * d * h - c * e2 * g
    );
  }

  /**
   * Creates a copy of this matrix
   * @returns A new matrix with the same elements
   */
  clone(): Matrix3 {
    return new Matrix3(Array.from(this.elements));
  }

  /**
   * Checks if this matrix equals another matrix
   * @param m - The other matrix
   * @param epsilon - Tolerance for floating point comparison
   * @returns True if matrices are equal
   */
  equals(m: Matrix3, epsilon: number = 0): boolean {
    const a = this.elements;
    const b = m.elements;
    for (let i = 0; i < 9; i++) {
      if (Math.abs(a[i]! - b[i]!) > epsilon) {
        return false;
      }
    }
    return true;
  }
}
