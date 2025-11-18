import { Time } from './Time';
import { Loop, LoopOptions } from './Loop';
import {
  Application,
  ApplicationHandlers,
  ApplicationState,
} from './Application';
import { World } from '../ecs/World';
import { Entity } from '../ecs/Entity';
import { GameEntity } from './GameEntity';

/**
 * Configuration options for the engine
 */
export interface EngineConfig {
  /**
   * Canvas element to render to
   */
  canvas: HTMLCanvasElement;

  /**
   * Loop configuration
   */
  loop?: LoopOptions;

  /**
   * Application event handlers
   */
  handlers?: ApplicationHandlers;
}

/**
 * Main engine class that manages all engine systems
 */
export class Engine {
  private _time: Time;
  private _loop: Loop;
  private _application: Application;
  private _world: World;
  private _isInitialized: boolean = false;
  private _canvas: HTMLCanvasElement;

  /**
   * Creates a new engine instance
   * @param config - Engine configuration
   */
  constructor(config: EngineConfig) {
    this._time = new Time();
    this._loop = new Loop(config.loop);
    this._world = new World();
    this._canvas = config.canvas;

    // Wrap application handlers to integrate with engine systems
    const wrappedHandlers: ApplicationHandlers = {
      ...config.handlers,
      onUpdate: (deltaTime: number) => {
        // Update ECS world
        this._world.update(deltaTime);

        if (config.handlers?.onUpdate) {
          config.handlers.onUpdate(deltaTime);
        }
      },
    };

    this._application = new Application(wrappedHandlers);
  }

  /**
   * Time management system
   */
  get time(): Time {
    return this._time;
  }

  /**
   * Game loop system
   */
  get loop(): Loop {
    return this._loop;
  }

  /**
   * Application lifecycle system
   */
  get application(): Application {
    return this._application;
  }

  /**
   * Whether the engine is initialized
   */
  get isInitialized(): boolean {
    return this._isInitialized;
  }

  /**
   * Whether the engine is running
   */
  get isRunning(): boolean {
    return this._loop.isRunning;
  }

  /**
   * Initializes the engine
   */
  async initialize(): Promise<void> {
    if (this._isInitialized) {
      console.warn('Engine is already initialized');
      return;
    }

    try {
      const canvas = (this._application as any)._canvas;
      if (!canvas) {
        throw new Error('Canvas element is required');
      }

      await this._application.initialize(canvas);
      this._isInitialized = true;

      console.log('Engine initialized successfully');
    } catch (error) {
      console.error('Failed to initialize engine:', error);
      throw error;
    }
  }

  /**
   * Starts the engine
   */
  async start(): Promise<void> {
    if (!this._isInitialized) {
      throw new Error('Engine must be initialized before starting');
    }

    if (this._loop.isRunning) {
      console.warn('Engine is already running');
      return;
    }

    try {
      await this._application.start();

      // Start the game loop
      this._loop.start((deltaTime) => {
        this._update(deltaTime);
      });

      console.log('Engine started');
    } catch (error) {
      console.error('Failed to start engine:', error);
      throw error;
    }
  }

  /**
   * Stops the engine
   */
  stop(): void {
    if (!this._loop.isRunning) {
      console.warn('Engine is not running');
      return;
    }

    this._loop.stop();
    this._application.stop();

    console.log('Engine stopped');
  }

  /**
   * Pauses the engine
   */
  pause(): void {
    this._application.pause();
  }

  /**
   * Resumes the engine
   */
  resume(): void {
    this._application.resume();
  }

  /**
   * Destroys the engine and cleans up resources
   */
  destroy(): void {
    this.stop();
    this._isInitialized = false;
    console.log('Engine destroyed');
  }

  /**
   * ECS World accessor
   */
  get world(): World {
    return this._world;
  }

  /**
   * Canvas element
   */
  get canvas(): HTMLCanvasElement {
    return this._canvas;
  }

  /**
   * Creates a new entity in the world
   * @param name - Optional name for the entity
   * @returns The created game entity with convenient methods
   */
  createEntity(name?: string): GameEntity {
    const entity = this._world.createEntity(name);
    return new GameEntity(entity, this._world);
  }

  /**
   * Destroys an entity
   * @param entity - Entity to destroy
   */
  destroyEntity(entity: Entity): void {
    this._world.destroyEntity(entity);
  }

  /**
   * Gets an entity by ID
   * @param id - Entity ID
   * @returns The entity or undefined
   */
  getEntity(id: number): Entity | undefined {
    return this._world.getEntity(id);
  }

  /**
   * Queries entities by components
   * @param query - Component query
   * @returns Array of matching entities
   */
  queryEntities(query: any): Entity[] {
    return this._world.queryEntities(query);
  }

  /**
   * Adds a component to an entity
   * @param entity - Entity to add component to
   * @param componentType - Type of component
   * @param data - Component data
   */
  addComponent(entity: Entity, componentType: string, data?: any): any {
    return this._world.addComponent(entity, componentType as any, data);
  }

  /**
   * Gets a component from an entity
   * @param entity - Entity to get component from
   * @param componentType - Type of component
   */
  getComponent(entity: Entity, componentType: string): any {
    return this._world.getComponent(entity, componentType as any);
  }

  /**
   * Removes a component from an entity
   * @param entity - Entity to remove component from
   * @param componentType - Type of component
   */
  removeComponent(entity: Entity, componentType: string): void {
    this._world.removeComponent(entity, componentType as any);
  }

  /**
   * Render the current frame
   */
  render(): void {
    // Rendering logic will be implemented here
    // For now, just clear the canvas
    const ctx = this._canvas.getContext('2d');
    if (ctx) {
      ctx.fillStyle = '#1a1a1a';
      ctx.fillRect(0, 0, this._canvas.width, this._canvas.height);
    }
  }

  /**
   * Main update loop
   * @param deltaTime - Time elapsed since last update
   */
  private _update(deltaTime: number): void {
    // Update time
    this._time.update(performance.now());

    // Update application
    if (this._application.state === ApplicationState.Running) {
      this._application.update(deltaTime);
    }
  }
}
