/**
 * Represents a 3D vector with x, y, and z components
 */
export class Vector3 {
  /**
   * Creates a new Vector3
   * @param x - The x component
   * @param y - The y component
   * @param z - The z component
   */
  constructor(
    public x: number = 0,
    public y: number = 0,
    public z: number = 0
  ) {}

  /**
   * Creates a new Vector3 with all components set to zero
   */
  static zero(): Vector3 {
    return new Vector3(0, 0, 0);
  }

  /**
   * Creates a new Vector3 with all components set to one
   */
  static one(): Vector3 {
    return new Vector3(1, 1, 1);
  }

  /**
   * Creates a unit vector pointing up (0, 1, 0)
   */
  static up(): Vector3 {
    return new Vector3(0, 1, 0);
  }

  /**
   * Creates a unit vector pointing down (0, -1, 0)
   */
  static down(): Vector3 {
    return new Vector3(0, -1, 0);
  }

  /**
   * Creates a unit vector pointing left (-1, 0, 0)
   */
  static left(): Vector3 {
    return new Vector3(-1, 0, 0);
  }

  /**
   * Creates a unit vector pointing right (1, 0, 0)
   */
  static right(): Vector3 {
    return new Vector3(1, 0, 0);
  }

  /**
   * Creates a unit vector pointing forward (0, 0, 1)
   */
  static forward(): Vector3 {
    return new Vector3(0, 0, 1);
  }

  /**
   * Creates a unit vector pointing backward (0, 0, -1)
   */
  static backward(): Vector3 {
    return new Vector3(0, 0, -1);
  }

  /**
   * Adds another vector to this vector
   * @param v - The vector to add
   * @returns A new vector representing the sum
   */
  add(v: Vector3): Vector3 {
    return new Vector3(this.x + v.x, this.y + v.y, this.z + v.z);
  }

  /**
   * Subtracts another vector from this vector
   * @param v - The vector to subtract
   * @returns A new vector representing the difference
   */
  subtract(v: Vector3): Vector3 {
    return new Vector3(this.x - v.x, this.y - v.y, this.z - v.z);
  }

  /**
   * Multiplies this vector by a scalar
   * @param scalar - The scalar value
   * @returns A new scaled vector
   */
  multiply(scalar: number): Vector3 {
    return new Vector3(this.x * scalar, this.y * scalar, this.z * scalar);
  }

  /**
   * Divides this vector by a scalar
   * @param scalar - The scalar value
   * @returns A new divided vector
   */
  divide(scalar: number): Vector3 {
    if (scalar === 0) {
      throw new Error('Cannot divide by zero');
    }
    return new Vector3(this.x / scalar, this.y / scalar, this.z / scalar);
  }

  /**
   * Calculates the dot product with another vector
   * @param v - The other vector
   * @returns The dot product
   */
  dot(v: Vector3): number {
    return this.x * v.x + this.y * v.y + this.z * v.z;
  }

  /**
   * Calculates the cross product with another vector
   * @param v - The other vector
   * @returns A new vector perpendicular to both vectors
   */
  cross(v: Vector3): Vector3 {
    return new Vector3(
      this.y * v.z - this.z * v.y,
      this.z * v.x - this.x * v.z,
      this.x * v.y - this.y * v.x
    );
  }

  /**
   * Calculates the length (magnitude) of this vector
   * @returns The length
   */
  length(): number {
    return Math.sqrt(this.x * this.x + this.y * this.y + this.z * this.z);
  }

  /**
   * Calculates the squared length of this vector
   * @returns The squared length
   */
  lengthSquared(): number {
    return this.x * this.x + this.y * this.y + this.z * this.z;
  }

  /**
   * Returns a normalized copy of this vector
   * @returns A new normalized vector
   */
  normalize(): Vector3 {
    const len = this.length();
    if (len === 0) {
      return Vector3.zero();
    }
    return this.divide(len);
  }

  /**
   * Calculates the distance to another vector
   * @param v - The other vector
   * @returns The distance
   */
  distance(v: Vector3): number {
    return this.subtract(v).length();
  }

  /**
   * Calculates the squared distance to another vector
   * @param v - The other vector
   * @returns The squared distance
   */
  distanceSquared(v: Vector3): number {
    return this.subtract(v).lengthSquared();
  }

  /**
   * Linearly interpolates between this vector and another
   * @param v - The target vector
   * @param t - The interpolation factor (0-1)
   * @returns A new interpolated vector
   */
  lerp(v: Vector3, t: number): Vector3 {
    return new Vector3(
      this.x + (v.x - this.x) * t,
      this.y + (v.y - this.y) * t,
      this.z + (v.z - this.z) * t
    );
  }

  /**
   * Creates a copy of this vector
   * @returns A new vector with the same components
   */
  clone(): Vector3 {
    return new Vector3(this.x, this.y, this.z);
  }

  /**
   * Checks if this vector equals another vector
   * @param v - The other vector
   * @param epsilon - Optional tolerance for floating point comparison
   * @returns True if vectors are equal
   */
  equals(v: Vector3, epsilon: number = 0): boolean {
    return (
      Math.abs(this.x - v.x) <= epsilon &&
      Math.abs(this.y - v.y) <= epsilon &&
      Math.abs(this.z - v.z) <= epsilon
    );
  }

  /**
   * Converts this vector to a string
   * @returns String representation
   */
  toString(): string {
    return `Vector3(${this.x}, ${this.y}, ${this.z})`;
  }

  /**
   * Converts this vector to an array
   * @returns Array of components [x, y, z]
   */
  toArray(): [number, number, number] {
    return [this.x, this.y, this.z];
  }
}
