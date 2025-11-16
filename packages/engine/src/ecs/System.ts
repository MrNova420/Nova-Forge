import { World } from './World';

/**
 * Base interface for all systems
 * Systems contain logic that operates on entities with specific components
 */
export interface ISystem {
  /**
   * System priority (lower values execute first)
   */
  priority: number;

  /**
   * Whether this system is enabled
   */
  enabled: boolean;

  /**
   * Reference to the world this system belongs to
   */
  world: World | null;

  /**
   * Called when the system is added to the world
   */
  onInit?(): void;

  /**
   * Called every frame
   * @param deltaTime - Time elapsed since last update
   */
  onUpdate?(deltaTime: number): void;

  /**
   * Called at a fixed timestep
   * @param deltaTime - Fixed timestep duration
   */
  onFixedUpdate?(deltaTime: number): void;

  /**
   * Called when the system is removed from the world
   */
  onDestroy?(): void;
}

/**
 * Base class for systems
 */
export abstract class System implements ISystem {
  public priority: number = 0;
  public enabled: boolean = true;
  public world: World | null = null;

  /**
   * Creates a new system
   * @param priority - Execution priority (lower = earlier)
   */
  constructor(priority: number = 0) {
    this.priority = priority;
  }

  /**
   * Called when the system is added to the world
   */
  onInit?(): void;

  /**
   * Called every frame
   * @param deltaTime - Time elapsed since last update
   */
  onUpdate?(deltaTime: number): void;

  /**
   * Called at a fixed timestep
   * @param deltaTime - Fixed timestep duration
   */
  onFixedUpdate?(deltaTime: number): void;

  /**
   * Called when the system is removed from the world
   */
  onDestroy?(): void;
}
