/**
 * NOVA ENGINE - Proprietary Software
 * 
 * Copyright (c) 2025 Kayden Shawn Massengill. All Rights Reserved.
 * Operating as: WeNova Interactive
 * 
 * This software is proprietary and confidential. Unauthorized copying,
 * modification, distribution, or use of this software, via any medium,
 * is strictly prohibited without prior written permission.
 * 
 * See LICENSE file in the root directory for full license terms.
 */

/**
 * Application state
 */
export enum ApplicationState {
  Uninitialized = 'uninitialized',
  Initializing = 'initializing',
  Running = 'running',
  Paused = 'paused',
  Stopped = 'stopped',
}

/**
 * Application lifecycle event handlers
 */
export interface ApplicationHandlers {
  /**
   * Called when the application starts
   */
  onStart?: () => void | Promise<void>;

  /**
   * Called on each frame update
   */
  onUpdate?: (deltaTime: number) => void;

  /**
   * Called when the application is paused
   */
  onPause?: () => void;

  /**
   * Called when the application resumes from pause
   */
  onResume?: () => void;

  /**
   * Called when the application is stopped
   */
  onStop?: () => void;

  /**
   * Called when an error occurs
   */
  onError?: (error: Error) => void;
}

/**
 * Manages application lifecycle and state
 */
export class Application {
  private _state: ApplicationState = ApplicationState.Uninitialized;
  private _handlers: ApplicationHandlers = {};
  private _canvas: HTMLCanvasElement | null = null;
  private _context: WebGL2RenderingContext | WebGLRenderingContext | null =
    null;

  /**
   * Creates a new application
   * @param handlers - Lifecycle event handlers
   */
  constructor(handlers: ApplicationHandlers = {}) {
    this._handlers = handlers;
    this._setupWindowEvents();
  }

  /**
   * Current application state
   */
  get state(): ApplicationState {
    return this._state;
  }

  /**
   * Canvas element
   */
  get canvas(): HTMLCanvasElement | null {
    return this._canvas;
  }

  /**
   * Rendering context
   */
  get context(): WebGL2RenderingContext | WebGLRenderingContext | null {
    return this._context;
  }

  /**
   * Initializes the application
   * @param canvas - Canvas element to render to
   */
  async initialize(canvas: HTMLCanvasElement): Promise<void> {
    if (this._state !== ApplicationState.Uninitialized) {
      throw new Error(`Cannot initialize application in state: ${this._state}`);
    }

    this._state = ApplicationState.Initializing;

    try {
      this._canvas = canvas;

      // Get rendering context (try WebGL2 first)
      let context: WebGL2RenderingContext | WebGLRenderingContext | null =
        canvas.getContext('webgl2');
      if (!context) {
        context = canvas.getContext('webgl');
      }

      if (!context) {
        throw new Error('WebGL is not supported');
      }

      this._context = context;
      this._state = ApplicationState.Running;
    } catch (error) {
      this._state = ApplicationState.Stopped;
      if (error instanceof Error) {
        this._handleError(error);
      }
      throw error;
    }
  }

  /**
   * Starts the application
   */
  async start(): Promise<void> {
    if (this._state === ApplicationState.Uninitialized) {
      throw new Error('Application must be initialized before starting');
    }

    if (this._state === ApplicationState.Running) {
      console.warn('Application is already running');
      return;
    }

    try {
      this._state = ApplicationState.Running;

      if (this._handlers.onStart) {
        await this._handlers.onStart();
      }
    } catch (error) {
      if (error instanceof Error) {
        this._handleError(error);
      }
      throw error;
    }
  }

  /**
   * Updates the application
   * @param deltaTime - Time elapsed since last update
   * @internal
   */
  update(deltaTime: number): void {
    if (this._state !== ApplicationState.Running) {
      return;
    }

    try {
      if (this._handlers.onUpdate) {
        this._handlers.onUpdate(deltaTime);
      }
    } catch (error) {
      if (error instanceof Error) {
        this._handleError(error);
      }
    }
  }

  /**
   * Pauses the application
   */
  pause(): void {
    if (this._state !== ApplicationState.Running) {
      return;
    }

    this._state = ApplicationState.Paused;

    if (this._handlers.onPause) {
      this._handlers.onPause();
    }
  }

  /**
   * Resumes the application from pause
   */
  resume(): void {
    if (this._state !== ApplicationState.Paused) {
      return;
    }

    this._state = ApplicationState.Running;

    if (this._handlers.onResume) {
      this._handlers.onResume();
    }
  }

  /**
   * Stops the application
   */
  stop(): void {
    if (this._state === ApplicationState.Stopped) {
      return;
    }

    this._state = ApplicationState.Stopped;

    if (this._handlers.onStop) {
      this._handlers.onStop();
    }

    this._cleanup();
  }

  /**
   * Sets up window event listeners
   */
  private _setupWindowEvents(): void {
    if (typeof window === 'undefined') {
      return;
    }

    // Handle visibility change
    document.addEventListener('visibilitychange', () => {
      if (document.hidden) {
        if (this._state === ApplicationState.Running) {
          this.pause();
        }
      } else {
        if (this._state === ApplicationState.Paused) {
          this.resume();
        }
      }
    });

    // Handle page unload
    window.addEventListener('beforeunload', () => {
      this.stop();
    });
  }

  /**
   * Handles errors
   */
  private _handleError(error: Error): void {
    console.error('Application error:', error);

    if (this._handlers.onError) {
      this._handlers.onError(error);
    }
  }

  /**
   * Cleans up resources
   */
  private _cleanup(): void {
    this._canvas = null;
    this._context = null;
  }
}
