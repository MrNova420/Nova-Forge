import { Entity, EntityId } from './Entity';
import {
  IComponent,
  ComponentConstructor,
  ComponentType,
  ComponentRegistry,
} from './Component';
import { ISystem } from './System';

/**
 * Query for entities with specific components
 */
export interface ComponentQuery {
  /**
   * Component types that must be present (AND condition)
   */
  all?: ComponentConstructor[];

  /**
   * Component types where at least one must be present (OR condition)
   */
  any?: ComponentConstructor[];

  /**
   * Component types that must not be present (NOT condition)
   */
  none?: ComponentConstructor[];
}

/**
 * World manages all entities, components, and systems
 */
export class World {
  private _entities: Map<EntityId, Entity> = new Map();
  private _components: Map<EntityId, Map<ComponentType, IComponent>> =
    new Map();
  private _systems: ISystem[] = [];
  private _entitiesToDestroy: Set<EntityId> = new Set();

  /**
   * Creates a new entity
   * @param name - Optional name for debugging
   * @returns The created entity
   */
  createEntity(name?: string): Entity {
    const entity = new Entity(name);
    this._entities.set(entity.id, entity);
    this._components.set(entity.id, new Map());
    return entity;
  }

  /**
   * Destroys an entity and all its components
   * @param entity - Entity to destroy
   */
  destroyEntity(entity: Entity): void {
    this._entitiesToDestroy.add(entity.id);
  }

  /**
   * Immediately destroys an entity (called during cleanup)
   * @param entityId - Entity ID to destroy
   */
  private _destroyEntityImmediate(entityId: EntityId): void {
    const components = this._components.get(entityId);
    if (components) {
      // Call onDetach for all components
      for (const component of components.values()) {
        if (component.onDetach) {
          component.onDetach();
        }
        component.entity = null;
      }
      this._components.delete(entityId);
    }

    this._entities.delete(entityId);
  }

  /**
   * Gets an entity by ID
   * @param entityId - Entity ID
   * @returns Entity or undefined
   */
  getEntity(entityId: EntityId): Entity | undefined {
    return this._entities.get(entityId);
  }

  /**
   * Gets all entities
   * @returns Array of all entities
   */
  getAllEntities(): Entity[] {
    return Array.from(this._entities.values());
  }

  /**
   * Adds a component to an entity
   * @param entity - Entity to add component to
   * @param component - Component instance
   */
  addComponent<T extends IComponent>(entity: Entity, component: T): T {
    const components = this._components.get(entity.id);
    if (!components) {
      throw new Error(`Entity ${entity.id} does not exist`);
    }

    const typeName = ComponentRegistry.getTypeName(
      component.constructor as ComponentConstructor
    );

    if (components.has(typeName)) {
      console.warn(
        `Entity ${entity.id} already has component ${typeName}, replacing`
      );
      const existing = components.get(typeName);
      if (existing?.onDetach) {
        existing.onDetach();
      }
    }

    component.entity = entity;
    components.set(typeName, component);

    if (component.onAttach) {
      component.onAttach();
    }

    return component;
  }

  /**
   * Removes a component from an entity
   * @param entity - Entity to remove component from
   * @param componentType - Component constructor
   */
  removeComponent<T extends IComponent>(
    entity: Entity,
    componentType: ComponentConstructor<T>
  ): void {
    const components = this._components.get(entity.id);
    if (!components) {
      return;
    }

    const typeName = ComponentRegistry.getTypeName(componentType);
    const component = components.get(typeName);

    if (component) {
      if (component.onDetach) {
        component.onDetach();
      }
      component.entity = null;
      components.delete(typeName);
    }
  }

  /**
   * Gets a component from an entity
   * @param entity - Entity to get component from
   * @param componentType - Component constructor
   * @returns Component or undefined
   */
  getComponent<T extends IComponent>(
    entity: Entity,
    componentType: ComponentConstructor<T>
  ): T | undefined {
    const components = this._components.get(entity.id);
    if (!components) {
      return undefined;
    }

    const typeName = ComponentRegistry.getTypeName(componentType);
    return components.get(typeName) as T | undefined;
  }

  /**
   * Checks if an entity has a component
   * @param entity - Entity to check
   * @param componentType - Component constructor
   * @returns True if entity has the component
   */
  hasComponent<T extends IComponent>(
    entity: Entity,
    componentType: ComponentConstructor<T>
  ): boolean {
    const components = this._components.get(entity.id);
    if (!components) {
      return false;
    }

    const typeName = ComponentRegistry.getTypeName(componentType);
    return components.has(typeName);
  }

  /**
   * Gets all components of an entity
   * @param entity - Entity to get components from
   * @returns Array of components
   */
  getComponents(entity: Entity): IComponent[] {
    const components = this._components.get(entity.id);
    return components ? Array.from(components.values()) : [];
  }

  /**
   * Queries entities with specific components
   * @param query - Component query
   * @returns Array of matching entities
   */
  query(query: ComponentQuery): Entity[] {
    const result: Entity[] = [];

    for (const entity of this._entities.values()) {
      if (!entity.active) {
        continue;
      }

      // Check 'all' condition
      if (query.all) {
        const hasAll = query.all.every((type) =>
          this.hasComponent(entity, type)
        );
        if (!hasAll) {
          continue;
        }
      }

      // Check 'any' condition
      if (query.any) {
        const hasAny = query.any.some((type) =>
          this.hasComponent(entity, type)
        );
        if (!hasAny) {
          continue;
        }
      }

      // Check 'none' condition
      if (query.none) {
        const hasNone = query.none.some((type) =>
          this.hasComponent(entity, type)
        );
        if (hasNone) {
          continue;
        }
      }

      result.push(entity);
    }

    return result;
  }

  /**
   * Adds a system to the world
   * @param system - System to add
   */
  addSystem(system: ISystem): void {
    if (this._systems.includes(system)) {
      console.warn('System already added to world');
      return;
    }

    system.world = this;
    this._systems.push(system);

    // Sort systems by priority
    this._systems.sort((a, b) => a.priority - b.priority);

    if (system.onInit) {
      system.onInit();
    }
  }

  /**
   * Removes a system from the world
   * @param system - System to remove
   */
  removeSystem(system: ISystem): void {
    const index = this._systems.indexOf(system);
    if (index === -1) {
      return;
    }

    if (system.onDestroy) {
      system.onDestroy();
    }

    system.world = null;
    this._systems.splice(index, 1);
  }

  /**
   * Gets all systems
   * @returns Array of systems
   */
  getSystems(): ISystem[] {
    return [...this._systems];
  }

  /**
   * Updates all systems
   * @param deltaTime - Time elapsed since last update
   */
  update(deltaTime: number): void {
    // Update systems
    for (const system of this._systems) {
      if (system.enabled && system.onUpdate) {
        system.onUpdate(deltaTime);
      }
    }

    // Clean up destroyed entities
    this._cleanupDestroyedEntities();
  }

  /**
   * Fixed update for all systems
   * @param deltaTime - Fixed timestep duration
   */
  fixedUpdate(deltaTime: number): void {
    for (const system of this._systems) {
      if (system.enabled && system.onFixedUpdate) {
        system.onFixedUpdate(deltaTime);
      }
    }
  }

  /**
   * Cleans up entities marked for destruction
   */
  private _cleanupDestroyedEntities(): void {
    for (const entityId of this._entitiesToDestroy) {
      this._destroyEntityImmediate(entityId);
    }
    this._entitiesToDestroy.clear();
  }

  /**
   * Clears the world (removes all entities and systems)
   */
  clear(): void {
    // Destroy all systems
    for (const system of [...this._systems]) {
      this.removeSystem(system);
    }

    // Destroy all entities
    for (const entityId of this._entities.keys()) {
      this._destroyEntityImmediate(entityId);
    }

    this._entities.clear();
    this._components.clear();
    this._systems = [];
    this._entitiesToDestroy.clear();
  }
}
