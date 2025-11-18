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
 * Mock component class for game development
 * This allows games to add components with simple data objects
 */
class MockComponent {
  public type: string;
  public data: ComponentData;
  public entity: Entity | null = null;

  constructor(type: string, data: ComponentData = {}) {
    this.type = type;
    this.data = data;
  }

  onAttach?(): void {}
  onDetach?(): void {}
}

/**
 * GameEntity wraps the core Entity with convenient methods
 * that match what game developers expect
 */
export class GameEntity {
  private _entity: Entity;
  private _world: World | null = null;
  private _components: Map<string, MockComponent> = new Map();

  constructor(entity: Entity, world?: World) {
    this._entity = entity;
    this._world = world || null;
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
  addComponent(type: string, data: ComponentData = {}): MockComponent {
    const component = new MockComponent(type, data);
    component.entity = this._entity;
    this._components.set(type, component);
    return component;
  }

  /**
   * Gets a component from this entity
   * @param type - Component type name
   * @returns The component or undefined
   */
  getComponent(type: string): MockComponent | undefined {
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
    const component = this._components.get(type);
    if (component && component.onDetach) {
      component.onDetach();
    }
    this._components.delete(type);
  }

  /**
   * Gets all components on this entity
   * @returns Array of components
   */
  getAllComponents(): MockComponent[] {
    return Array.from(this._components.values());
  }

  /**
   * Destroys this entity
   */
  destroy(): void {
    // Call onDetach for all components
    for (const component of this._components.values()) {
      if (component.onDetach) {
        component.onDetach();
      }
    }
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
