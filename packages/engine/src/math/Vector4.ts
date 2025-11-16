/**
 * Represents a 4D vector with x, y, z, and w components
 */
export class Vector4 {
  /**
   * Creates a new Vector4
   * @param x - The x component
   * @param y - The y component
   * @param z - The z component
   * @param w - The w component
   */
  constructor(
    public x: number = 0,
    public y: number = 0,
    public z: number = 0,
    public w: number = 0
  ) {}

  /**
   * Creates a new Vector4 with all components set to zero
   */
  static zero(): Vector4 {
    return new Vector4(0, 0, 0, 0);
  }

  /**
   * Creates a new Vector4 with all components set to one
   */
  static one(): Vector4 {
    return new Vector4(1, 1, 1, 1);
  }

  /**
   * Adds another vector to this vector
   * @param v - The vector to add
   * @returns A new vector representing the sum
   */
  add(v: Vector4): Vector4 {
    return new Vector4(this.x + v.x, this.y + v.y, this.z + v.z, this.w + v.w);
  }

  /**
   * Subtracts another vector from this vector
   * @param v - The vector to subtract
   * @returns A new vector representing the difference
   */
  subtract(v: Vector4): Vector4 {
    return new Vector4(this.x - v.x, this.y - v.y, this.z - v.z, this.w - v.w);
  }

  /**
   * Multiplies this vector by a scalar
   * @param scalar - The scalar value
   * @returns A new scaled vector
   */
  multiply(scalar: number): Vector4 {
    return new Vector4(
      this.x * scalar,
      this.y * scalar,
      this.z * scalar,
      this.w * scalar
    );
  }

  /**
   * Divides this vector by a scalar
   * @param scalar - The scalar value
   * @returns A new divided vector
   */
  divide(scalar: number): Vector4 {
    if (scalar === 0) {
      throw new Error('Cannot divide by zero');
    }
    return new Vector4(
      this.x / scalar,
      this.y / scalar,
      this.z / scalar,
      this.w / scalar
    );
  }

  /**
   * Calculates the dot product with another vector
   * @param v - The other vector
   * @returns The dot product
   */
  dot(v: Vector4): number {
    return this.x * v.x + this.y * v.y + this.z * v.z + this.w * v.w;
  }

  /**
   * Calculates the length (magnitude) of this vector
   * @returns The length
   */
  length(): number {
    return Math.sqrt(
      this.x * this.x + this.y * this.y + this.z * this.z + this.w * this.w
    );
  }

  /**
   * Calculates the squared length of this vector
   * @returns The squared length
   */
  lengthSquared(): number {
    return (
      this.x * this.x + this.y * this.y + this.z * this.z + this.w * this.w
    );
  }

  /**
   * Returns a normalized copy of this vector
   * @returns A new normalized vector
   */
  normalize(): Vector4 {
    const len = this.length();
    if (len === 0) {
      return Vector4.zero();
    }
    return this.divide(len);
  }

  /**
   * Linearly interpolates between this vector and another
   * @param v - The target vector
   * @param t - The interpolation factor (0-1)
   * @returns A new interpolated vector
   */
  lerp(v: Vector4, t: number): Vector4 {
    return new Vector4(
      this.x + (v.x - this.x) * t,
      this.y + (v.y - this.y) * t,
      this.z + (v.z - this.z) * t,
      this.w + (v.w - this.w) * t
    );
  }

  /**
   * Creates a copy of this vector
   * @returns A new vector with the same components
   */
  clone(): Vector4 {
    return new Vector4(this.x, this.y, this.z, this.w);
  }

  /**
   * Checks if this vector equals another vector
   * @param v - The other vector
   * @param epsilon - Optional tolerance for floating point comparison
   * @returns True if vectors are equal
   */
  equals(v: Vector4, epsilon: number = 0): boolean {
    return (
      Math.abs(this.x - v.x) <= epsilon &&
      Math.abs(this.y - v.y) <= epsilon &&
      Math.abs(this.z - v.z) <= epsilon &&
      Math.abs(this.w - v.w) <= epsilon
    );
  }

  /**
   * Converts this vector to a string
   * @returns String representation
   */
  toString(): string {
    return `Vector4(${this.x}, ${this.y}, ${this.z}, ${this.w})`;
  }

  /**
   * Converts this vector to an array
   * @returns Array of components [x, y, z, w]
   */
  toArray(): [number, number, number, number] {
    return [this.x, this.y, this.z, this.w];
  }
}
