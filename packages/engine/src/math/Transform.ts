import { Vector3 } from './Vector3';
import { Quaternion } from './Quaternion';
import { Matrix4 } from './Matrix4';

/**
 * Represents a 3D transformation with position, rotation, and scale
 */
export class Transform {
  /**
   * Creates a new Transform
   * @param position - Position vector
   * @param rotation - Rotation quaternion
   * @param scale - Scale vector
   */
  constructor(
    public position: Vector3 = Vector3.zero(),
    public rotation: Quaternion = Quaternion.identity(),
    public scale: Vector3 = Vector3.one()
  ) {}

  /**
   * Creates an identity transform
   */
  static identity(): Transform {
    return new Transform();
  }

  /**
   * Converts this transform to a transformation matrix
   * @returns A new Matrix4 representing the transform
   */
  toMatrix(): Matrix4 {
    const scaleMatrix = Matrix4.scaling(this.scale);
    const rotationMatrix = this.rotation.toMatrix4();
    const translationMatrix = Matrix4.translation(this.position);

    return translationMatrix.multiply(rotationMatrix).multiply(scaleMatrix);
  }

  /**
   * Transforms a point by this transform
   * @param point - The point to transform
   * @returns A new transformed point
   */
  transformPoint(point: Vector3): Vector3 {
    // Scale
    let result = new Vector3(
      point.x * this.scale.x,
      point.y * this.scale.y,
      point.z * this.scale.z
    );

    // Rotate
    result = this.rotation.rotateVector(result);

    // Translate
    result = result.add(this.position);

    return result;
  }

  /**
   * Transforms a direction by this transform (ignores position)
   * @param direction - The direction to transform
   * @returns A new transformed direction
   */
  transformDirection(direction: Vector3): Vector3 {
    return this.rotation.rotateVector(direction);
  }

  /**
   * Combines this transform with another transform
   * @param other - The other transform
   * @returns A new combined transform
   */
  combine(other: Transform): Transform {
    const newPosition = this.transformPoint(other.position);
    const newRotation = this.rotation.multiply(other.rotation);
    const newScale = new Vector3(
      this.scale.x * other.scale.x,
      this.scale.y * other.scale.y,
      this.scale.z * other.scale.z
    );

    return new Transform(newPosition, newRotation, newScale);
  }

  /**
   * Calculates the inverse of this transform
   * @returns A new inverse transform
   */
  inverse(): Transform {
    const invRotation = this.rotation.inverse();
    const invScale = new Vector3(
      1 / this.scale.x,
      1 / this.scale.y,
      1 / this.scale.z
    );

    const invPosition = invRotation
      .rotateVector(this.position.multiply(-1))
      .multiply(invScale.x);

    return new Transform(invPosition, invRotation, invScale);
  }

  /**
   * Linearly interpolates between this transform and another
   * @param other - The target transform
   * @param t - The interpolation factor (0-1)
   * @returns A new interpolated transform
   */
  lerp(other: Transform, t: number): Transform {
    return new Transform(
      this.position.lerp(other.position, t),
      this.rotation.slerp(other.rotation, t),
      this.scale.lerp(other.scale, t)
    );
  }

  /**
   * Gets the forward direction vector
   * @returns Forward vector
   */
  forward(): Vector3 {
    return this.rotation.rotateVector(Vector3.forward());
  }

  /**
   * Gets the up direction vector
   * @returns Up vector
   */
  up(): Vector3 {
    return this.rotation.rotateVector(Vector3.up());
  }

  /**
   * Gets the right direction vector
   * @returns Right vector
   */
  right(): Vector3 {
    return this.rotation.rotateVector(Vector3.right());
  }

  /**
   * Creates a copy of this transform
   * @returns A new transform with the same values
   */
  clone(): Transform {
    return new Transform(
      this.position.clone(),
      this.rotation.clone(),
      this.scale.clone()
    );
  }

  /**
   * Checks if this transform equals another transform
   * @param other - The other transform
   * @param epsilon - Tolerance for floating point comparison
   * @returns True if transforms are equal
   */
  equals(other: Transform, epsilon: number = 0): boolean {
    return (
      this.position.equals(other.position, epsilon) &&
      this.rotation.equals(other.rotation, epsilon) &&
      this.scale.equals(other.scale, epsilon)
    );
  }

  /**
   * Converts this transform to a string
   * @returns String representation
   */
  toString(): string {
    return `Transform(pos: ${this.position.toString()}, rot: ${this.rotation.toString()}, scale: ${this.scale.toString()})`;
  }
}
