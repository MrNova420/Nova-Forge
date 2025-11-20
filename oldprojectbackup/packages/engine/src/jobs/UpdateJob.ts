/**
 * @fileoverview Update Job
 * @module @nova-engine/jobs
 *
 * Job for frame update logic.
 * Executes game logic, physics, animation updates.
 *
 * Following the autonomous development guide Phase 1.2.3
 */

import type { JobConfig, JobContext } from './IJob';
import { Job } from './Job';

/**
 * Update Job
 *
 * Executes update logic for a single frame.
 * Typically runs game logic, physics, animations, etc.
 *
 * @example
 * ```typescript
 * const job = new UpdateJob({
 *   name: 'Update Physics',
 *   update: (deltaTime) => physicsWorld.step(deltaTime),
 *   deltaTime: 0.016,
 * });
 * ```
 */
export class UpdateJob<T = void> extends Job<T> {
  private updateFunc: (deltaTime: number, context: JobContext) => T | Promise<T>;
  private deltaTime: number;

  constructor(
    config: JobConfig & {
      update: (deltaTime: number, context: JobContext) => T | Promise<T>;
      deltaTime: number;
    }
  ) {
    super(config);
    this.updateFunc = config.update;
    this.deltaTime = config.deltaTime;
  }

  protected async run(context: JobContext): Promise<T> {
    return await Promise.resolve(this.updateFunc(this.deltaTime, context));
  }

  /**
   * Update delta time for next execution
   */
  setDeltaTime(deltaTime: number): void {
    this.deltaTime = deltaTime;
  }
}

/**
 * @fileoverview Render Job
 * @module @nova-engine/jobs
 *
 * Job for rendering operations.
 * Executes rendering commands on GPU.
 */

/**
 * Render Job
 *
 * Executes rendering operations for a frame.
 * Should run on main thread with GPU access.
 *
 * @example
 * ```typescript
 * const job = new RenderJob({
 *   name: 'Render Scene',
 *   render: (context) => renderer.render(scene, camera),
 * });
 * ```
 */
export class RenderJob<T = void> extends Job<T> {
  private renderFunc: (context: JobContext) => T | Promise<T>;

  constructor(
    config: JobConfig & {
      render: (context: JobContext) => T | Promise<T>;
    }
  ) {
    super(config);
    this.renderFunc = config.render;
  }

  protected async run(context: JobContext): Promise<T> {
    return await Promise.resolve(this.renderFunc(context));
  }
}
