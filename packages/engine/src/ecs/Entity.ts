/**
 * Unique identifier for an entity
 */
export type EntityId = number;

/**
 * Represents an entity in the ECS system
 * An entity is simply an ID that groups components together
 */
export class Entity {
  private static _nextId: EntityId = 1;

  /**
   * Unique identifier for this entity
   */
  public readonly id: EntityId;

  /**
   * Optional name for debugging
   */
  public name: string;

  /**
   * Whether the entity is active
   */
  public active: boolean = true;

  /**
   * Creates a new entity
   * @param name - Optional name for debugging
   */
  constructor(name: string = '') {
    this.id = Entity._nextId++;
    this.name = name || `Entity_${this.id}`;
  }

  /**
   * Resets the entity ID counter (for testing)
   * @internal
   */
  static resetIdCounter(): void {
    Entity._nextId = 1;
  }

  /**
   * Gets the next entity ID that will be assigned
   */
  static getNextId(): EntityId {
    return Entity._nextId;
  }

  /**
   * Returns a string representation of the entity
   */
  toString(): string {
    return `Entity(${this.id}, "${this.name}")`;
  }
}
