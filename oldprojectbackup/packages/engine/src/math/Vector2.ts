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
 * Represents a 2D vector with x and y components
 */
export class Vector2 {
  /**
   * Creates a new Vector2
   * @param x - The x component
   * @param y - The y component
   */
  constructor(
    public x: number = 0,
    public y: number = 0
  ) {}

  /**
   * Creates a new Vector2 with all components set to zero
   */
  static zero(): Vector2 {
    return new Vector2(0, 0);
  }

  /**
   * Creates a new Vector2 with all components set to one
   */
  static one(): Vector2 {
    return new Vector2(1, 1);
  }

  /**
   * Adds another vector to this vector
   * @param v - The vector to add
   * @returns A new vector representing the sum
   */
  add(v: Vector2): Vector2 {
    return new Vector2(this.x + v.x, this.y + v.y);
  }

  /**
   * Subtracts another vector from this vector
   * @param v - The vector to subtract
   * @returns A new vector representing the difference
   */
  subtract(v: Vector2): Vector2 {
    return new Vector2(this.x - v.x, this.y - v.y);
  }

  /**
   * Multiplies this vector by a scalar
   * @param scalar - The scalar value
   * @returns A new scaled vector
   */
  multiply(scalar: number): Vector2 {
    return new Vector2(this.x * scalar, this.y * scalar);
  }

  /**
   * Divides this vector by a scalar
   * @param scalar - The scalar value
   * @returns A new divided vector
   */
  divide(scalar: number): Vector2 {
    if (scalar === 0) {
      throw new Error('Cannot divide by zero');
    }
    return new Vector2(this.x / scalar, this.y / scalar);
  }

  /**
   * Calculates the dot product with another vector
   * @param v - The other vector
   * @returns The dot product
   */
  dot(v: Vector2): number {
    return this.x * v.x + this.y * v.y;
  }

  /**
   * Calculates the length (magnitude) of this vector
   * @returns The length
   */
  length(): number {
    return Math.sqrt(this.x * this.x + this.y * this.y);
  }

  /**
   * Calculates the squared length of this vector
   * @returns The squared length
   */
  lengthSquared(): number {
    return this.x * this.x + this.y * this.y;
  }

  /**
   * Returns a normalized copy of this vector
   * @returns A new normalized vector
   */
  normalize(): Vector2 {
    const len = this.length();
    if (len === 0) {
      return Vector2.zero();
    }
    return this.divide(len);
  }

  /**
   * Calculates the distance to another vector
   * @param v - The other vector
   * @returns The distance
   */
  distance(v: Vector2): number {
    return this.subtract(v).length();
  }

  /**
   * Calculates the squared distance to another vector
   * @param v - The other vector
   * @returns The squared distance
   */
  distanceSquared(v: Vector2): number {
    return this.subtract(v).lengthSquared();
  }

  /**
   * Linearly interpolates between this vector and another
   * @param v - The target vector
   * @param t - The interpolation factor (0-1)
   * @returns A new interpolated vector
   */
  lerp(v: Vector2, t: number): Vector2 {
    return new Vector2(
      this.x + (v.x - this.x) * t,
      this.y + (v.y - this.y) * t
    );
  }

  /**
   * Creates a copy of this vector
   * @returns A new vector with the same components
   */
  clone(): Vector2 {
    return new Vector2(this.x, this.y);
  }

  /**
   * Sets the components of this vector
   * @param x - The x component
   * @param y - The y component
   * @returns This vector for chaining
   */
  set(x: number, y: number): Vector2 {
    this.x = x;
    this.y = y;
    return this;
  }

  /**
   * Copies the components from another vector to this vector
   * @param v - The vector to copy from
   * @returns This vector for chaining
   */
  copy(v: Vector2): Vector2 {
    this.x = v.x;
    this.y = v.y;
    return this;
  }

  /**
   * Checks if this vector equals another vector
   * @param v - The other vector
   * @param epsilon - Optional tolerance for floating point comparison
   * @returns True if vectors are equal
   */
  equals(v: Vector2, epsilon: number = 0): boolean {
    return (
      Math.abs(this.x - v.x) <= epsilon && Math.abs(this.y - v.y) <= epsilon
    );
  }

  /**
   * Converts this vector to a string
   * @returns String representation
   */
  toString(): string {
    return `Vector2(${this.x}, ${this.y})`;
  }

  /**
   * Converts this vector to an array
   * @returns Array of components [x, y]
   */
  toArray(): [number, number] {
    return [this.x, this.y];
  }
}
