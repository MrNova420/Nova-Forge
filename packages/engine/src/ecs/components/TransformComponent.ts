import { Component, ComponentRegistry } from '../Component';
import { Vector3 } from '../../math/Vector3';
import { Quaternion } from '../../math/Quaternion';
import { Transform } from '../../math/Transform';

/**
 * Transform component that represents an entity's position, rotation, and scale in 3D space
 */
export class TransformComponent extends Component {
  /**
   * Local transform
   */
  public transform: Transform;

  /**
   * Parent transform component (for hierarchies)
   */
  public parent: TransformComponent | null = null;

  /**
   * Child transform components
   */
  public children: TransformComponent[] = [];

  /**
   * Creates a new TransformComponent
   * @param position - Initial position
   * @param rotation - Initial rotation
   * @param scale - Initial scale
   */
  constructor(
    position: Vector3 = Vector3.zero(),
    rotation: Quaternion = Quaternion.identity(),
    scale: Vector3 = Vector3.one()
  ) {
    super();
    this.transform = new Transform(position, rotation, scale);
  }

  /**
   * Gets the world transform (combining parent transforms)
   */
  getWorldTransform(): Transform {
    if (!this.parent) {
      return this.transform.clone();
    }

    const parentWorld = this.parent.getWorldTransform();
    return parentWorld.combine(this.transform);
  }

  /**
   * Sets the parent transform
   * @param parent - Parent transform component
   */
  setParent(parent: TransformComponent | null): void {
    // Remove from old parent
    if (this.parent) {
      const index = this.parent.children.indexOf(this);
      if (index !== -1) {
        this.parent.children.splice(index, 1);
      }
    }

    // Set new parent
    this.parent = parent;

    // Add to new parent
    if (parent) {
      parent.children.push(this);
    }
  }

  /**
   * Convenience getters/setters for transform properties
   */

  get position(): Vector3 {
    return this.transform.position;
  }

  set position(value: Vector3) {
    this.transform.position = value;
  }

  get rotation(): Quaternion {
    return this.transform.rotation;
  }

  set rotation(value: Quaternion) {
    this.transform.rotation = value;
  }

  get scale(): Vector3 {
    return this.transform.scale;
  }

  set scale(value: Vector3) {
    this.transform.scale = value;
  }

  /**
   * Direction vectors in world space
   */

  forward(): Vector3 {
    return this.getWorldTransform().forward();
  }

  up(): Vector3 {
    return this.getWorldTransform().up();
  }

  right(): Vector3 {
    return this.getWorldTransform().right();
  }
}

// Register the component
ComponentRegistry.register(TransformComponent, 'TransformComponent');
