/**
 * GameEntity - Enhanced entity with convenient component management
 * This provides a game-friendly API for working with entities
 */

import { Entity } from '../ecs/Entity';
import type { World } from '../ecs/World';

/**
 * Component data that can be attached to an entity
 */
export interface ComponentData {
  [key: string]: any;
}

/**
 * Simple component wrapper that provides the API demo games expect
 */
class SimpleComponent {
  public type: string;
  private data: any;

  constructor(type: string, config: any = {}) {
    this.type = type;
    this.data = config;

    // Copy all config properties to this object for easy access
    Object.assign(this, config);
  }

  // Provide setPosition method for Transform components
  setPosition(x: number, y: number, z: number): void {
    if (!this.data.position) {
      this.data.position = { x: 0, y: 0, z: 0 };
    }
    this.data.position.x = x;
    this.data.position.y = y;
    this.data.position.z = z;
  }

  setRotation(x: number, y: number, z: number): void {
    if (!this.data.rotation) {
      this.data.rotation = { x: 0, y: 0, z: 0 };
    }
    this.data.rotation.x = x;
    this.data.rotation.y = y;
    this.data.rotation.z = z;
  }

  setScale(x: number, y: number, z: number): void {
    if (!this.data.scale) {
      this.data.scale = { x: 1, y: 1, z: 1 };
    }
    this.data.scale.x = x;
    this.data.scale.y = y;
    this.data.scale.z = z;
  }

  // Get the underlying data
  getData(): any {
    return this.data;
  }
}

/**
 * GameEntity wraps the core Entity with convenient methods
 * that match what game developers expect
 */
export class GameEntity {
  private _entity: Entity;
  private _world: World | null = null;
  private _components: Map<string, SimpleComponent> = new Map();

  constructor(entity: Entity, world?: World) {
    this._entity = entity;
    this._world = world || null;

    // Every entity automatically has a Transform component
    this.addComponent('Transform', {
      position: { x: 0, y: 0, z: 0 },
      rotation: { x: 0, y: 0, z: 0 },
      scale: { x: 1, y: 1, z: 1 },
    });
  }

  /**
   * Gets the underlying entity
   */
  get entity(): Entity {
    return this._entity;
  }

  /**
   * Gets the entity ID
   */
  get id(): number {
    return this._entity.id;
  }

  /**
   * Gets the entity name
   */
  get name(): string {
    return this._entity.name;
  }

  /**
   * Sets the entity name
   */
  set name(value: string) {
    this._entity.name = value;
  }

  /**
   * Gets whether the entity is active
   */
  get active(): boolean {
    return this._entity.active;
  }

  /**
   * Sets whether the entity is active
   */
  set active(value: boolean) {
    this._entity.active = value;
  }

  /**
   * Adds a component to this entity
   * @param type - Component type name (e.g., 'Camera', 'Transform')
   * @param data - Component data
   * @returns The component
   */
  addComponent(type: string, data: ComponentData = {}): any {
    const component = new SimpleComponent(type, data);
    this._components.set(type, component);
    return component;
  }

  /**
   * Gets a component from this entity
   * @param type - Component type name
   * @returns The component or undefined
   */
  getComponent(type: string): any {
    return this._components.get(type);
  }

  /**
   * Checks if this entity has a component
   * @param type - Component type name
   * @returns True if the component exists
   */
  hasComponent(type: string): boolean {
    return this._components.has(type);
  }

  /**
   * Removes a component from this entity
   * @param type - Component type name
   */
  removeComponent(type: string): void {
    this._components.delete(type);
  }

  /**
   * Gets all components on this entity
   * @returns Array of components
   */
  getAllComponents(): any[] {
    return Array.from(this._components.values());
  }

  /**
   * Destroys this entity
   */
  destroy(): void {
    this._components.clear();

    // Destroy in world if available
    if (this._world) {
      this._world.destroyEntity(this._entity);
    }
  }

  /**
   * Returns a string representation of this entity
   */
  toString(): string {
    return this._entity.toString();
  }
}
