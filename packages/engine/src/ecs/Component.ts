import { Entity } from './Entity';

/**
 * Base interface for all components
 * Components hold data but contain no logic
 */
export interface IComponent {
  /**
   * The entity this component is attached to
   */
  entity: Entity | null;

  /**
   * Whether this component is enabled
   */
  enabled: boolean;

  /**
   * Called when the component is added to an entity
   */
  onAttach?(): void;

  /**
   * Called when the component is removed from an entity
   */
  onDetach?(): void;
}

/**
 * Component constructor type
 */
export type ComponentConstructor<T extends IComponent = IComponent> = new (
  ...args: any[]
) => T;

/**
 * Type name for a component class
 */
export type ComponentType = string;

/**
 * Registry for component types
 */
export class ComponentRegistry {
  private static _types: Map<ComponentConstructor, ComponentType> = new Map();
  private static _constructors: Map<ComponentType, ComponentConstructor> =
    new Map();

  /**
   * Registers a component type
   * @param constructor - Component constructor
   * @param typeName - Unique type name
   */
  static register<T extends IComponent>(
    constructor: ComponentConstructor<T>,
    typeName?: string
  ): void {
    const name = typeName || constructor.name;

    if (this._types.has(constructor)) {
      console.warn(`Component ${name} is already registered`);
      return;
    }

    this._types.set(constructor, name);
    this._constructors.set(name, constructor);
  }

  /**
   * Gets the type name for a component constructor
   * @param constructor - Component constructor
   * @returns Type name
   */
  static getTypeName(constructor: ComponentConstructor): ComponentType {
    const typeName = this._types.get(constructor);
    if (!typeName) {
      throw new Error(`Component ${constructor.name} is not registered`);
    }
    return typeName;
  }

  /**
   * Gets the constructor for a component type name
   * @param typeName - Type name
   * @returns Component constructor
   */
  static getConstructor(typeName: ComponentType): ComponentConstructor {
    const constructor = this._constructors.get(typeName);
    if (!constructor) {
      throw new Error(`Component type "${typeName}" is not registered`);
    }
    return constructor;
  }

  /**
   * Checks if a component type is registered
   * @param constructor - Component constructor
   * @returns True if registered
   */
  static isRegistered(constructor: ComponentConstructor): boolean {
    return this._types.has(constructor);
  }

  /**
   * Clears all registered component types (for testing)
   * @internal
   */
  static clear(): void {
    this._types.clear();
    this._constructors.clear();
  }
}

/**
 * Base class for components
 */
export abstract class Component implements IComponent {
  public entity: Entity | null = null;
  public enabled: boolean = true;

  /**
   * Called when the component is added to an entity
   */
  onAttach?(): void;

  /**
   * Called when the component is removed from an entity
   */
  onDetach?(): void;
}
