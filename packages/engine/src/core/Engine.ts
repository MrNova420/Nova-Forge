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
  private _gameEntities: Map<number, GameEntity> = new Map();

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
    const gameEntity = new GameEntity(entity, this._world);
    this._gameEntities.set(entity.id, gameEntity);
    return gameEntity;
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
   * Render the current frame with enhanced visual effects
   */
  render(): void {
    const ctx = this._canvas.getContext('2d');
    if (!ctx) return;

    // Clear the canvas with an animated gradient background
    const time = performance.now() / 1000;
    const gradient = ctx.createLinearGradient(
      0,
      0,
      this._canvas.width,
      this._canvas.height
    );
    const hue1 = (Math.sin(time * 0.5) * 30 + 230) % 360;
    const hue2 = (hue1 + 30) % 360;
    gradient.addColorStop(0, `hsl(${hue1}, 80%, 10%)`);
    gradient.addColorStop(1, `hsl(${hue2}, 70%, 15%)`);
    ctx.fillStyle = gradient;
    ctx.fillRect(0, 0, this._canvas.width, this._canvas.height);

    // Render an animated grid
    ctx.strokeStyle = `rgba(100, 100, 255, ${0.05 + Math.sin(time) * 0.03})`;
    ctx.lineWidth = 1;
    const gridSize = 50;
    const offsetX = (time * 20) % gridSize;
    const offsetY = (time * 15) % gridSize;

    for (let x = -offsetX; x < this._canvas.width; x += gridSize) {
      ctx.beginPath();
      ctx.moveTo(x, 0);
      ctx.lineTo(x, this._canvas.height);
      ctx.stroke();
    }
    for (let y = -offsetY; y < this._canvas.height; y += gridSize) {
      ctx.beginPath();
      ctx.moveTo(0, y);
      ctx.lineTo(this._canvas.width, y);
      ctx.stroke();
    }

    // Render all game entities with enhanced visuals
    this._gameEntities.forEach((gameEntity, index) => {
      const transform = gameEntity.getComponent('Transform');
      if (transform) {
        const data = transform.getData();
        const pos = data.position || { x: 0, y: 0, z: 0 };
        const scale = data.scale || { x: 1, y: 1, z: 1 };

        // Convert 3D position to 2D screen position (isometric-style)
        const screenX = this._canvas.width / 2 + pos.x * 10 - pos.z * 3;
        const screenY = this._canvas.height / 2 - pos.y * 10 - pos.z * 5;

        // Draw entity with glow effect
        const radius = 8 * scale.x;
        const hue = (index * 137.5 + time * 50) % 360;

        // Outer glow
        const glow = ctx.createRadialGradient(
          screenX,
          screenY,
          0,
          screenX,
          screenY,
          radius * 2
        );
        glow.addColorStop(0, `hsla(${hue}, 100%, 60%, 0.8)`);
        glow.addColorStop(1, `hsla(${hue}, 100%, 60%, 0)`);
        ctx.fillStyle = glow;
        ctx.fillRect(
          screenX - radius * 2,
          screenY - radius * 2,
          radius * 4,
          radius * 4
        );

        // Main entity circle
        ctx.fillStyle = `hsl(${hue}, 90%, 60%)`;
        ctx.beginPath();
        ctx.arc(screenX, screenY, radius, 0, Math.PI * 2);
        ctx.fill();

        // Inner highlight
        ctx.fillStyle = 'rgba(255, 255, 255, 0.3)';
        ctx.beginPath();
        ctx.arc(
          screenX - radius * 0.3,
          screenY - radius * 0.3,
          radius * 0.4,
          0,
          Math.PI * 2
        );
        ctx.fill();

        // Draw entity name with shadow
        if (gameEntity.name) {
          ctx.shadowColor = 'rgba(0, 0, 0, 0.8)';
          ctx.shadowBlur = 4;
          ctx.shadowOffsetX = 2;
          ctx.shadowOffsetY = 2;
          ctx.fillStyle = '#ffffff';
          ctx.font = 'bold 13px Arial';
          ctx.fillText(gameEntity.name, screenX + 12, screenY + 5);
          ctx.shadowBlur = 0;
        }
      }
    });

    // Render loading state or game info
    if (this._gameEntities.size === 0) {
      // Animated loading text
      ctx.shadowColor = 'rgba(123, 47, 247, 0.8)';
      ctx.shadowBlur = 20;
      ctx.fillStyle = `rgba(255, 255, 255, ${0.7 + Math.sin(time * 3) * 0.3})`;
      ctx.font = 'bold 32px Arial';
      ctx.textAlign = 'center';
      ctx.fillText(
        'Nova Engine',
        this._canvas.width / 2,
        this._canvas.height / 2 - 20
      );
      ctx.font = '18px Arial';
      ctx.fillStyle = `rgba(200, 200, 255, ${0.5 + Math.sin(time * 4) * 0.3})`;
      ctx.fillText(
        'Initializing Game...',
        this._canvas.width / 2,
        this._canvas.height / 2 + 20
      );
      ctx.shadowBlur = 0;
    }

    // Render HUD with enhanced styling
    ctx.textAlign = 'left';
    ctx.shadowColor = 'rgba(0, 0, 0, 0.8)';
    ctx.shadowBlur = 4;

    // FPS counter with color coding
    const fps = this._time.fps;
    ctx.fillStyle = fps >= 50 ? '#00ff00' : fps >= 30 ? '#ffff00' : '#ff0000';
    ctx.font = 'bold 18px monospace';
    ctx.fillText(`FPS: ${fps.toFixed(1)}`, 12, 32);

    // Entity count
    ctx.fillStyle = '#00ddff';
    ctx.fillText(`Entities: ${this._gameEntities.size}`, 12, 56);

    // Engine watermark
    ctx.fillStyle = 'rgba(123, 47, 247, 0.6)';
    ctx.font = 'bold 14px Arial';
    ctx.textAlign = 'right';
    ctx.fillText(
      'Nova Engine',
      this._canvas.width - 12,
      this._canvas.height - 12
    );

    ctx.shadowBlur = 0;
    ctx.textAlign = 'left';
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
