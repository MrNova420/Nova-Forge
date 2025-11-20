/**
 * @fileoverview Parallel For Job
 * @module @nova-engine/jobs
 *
 * Job for parallel iteration over data arrays.
 * Splits data into batches and processes them in parallel.
 *
 * Following the autonomous development guide Phase 1.2.3
 */

import type { JobContext, ParallelJobConfig } from './IJob';
import { Job } from './Job';

/**
 * Parallel For Job
 *
 * Processes an array of items in parallel by splitting into batches.
 * Useful for CPU-intensive operations on large datasets.
 *
 * @example
 * ```typescript
 * const job = new ParallelForJob({
 *   name: 'Process Particles',
 *   items: particles,
 *   batchSize: 100,
 *   process: (particle) => particle.update(deltaTime),
 * });
 * ```
 */
export class ParallelForJob<T, R = void> extends Job<R[]> {
  private items: T[];
  private processFunc: (item: T, index: number) => R;
  private batchSize: number;
  private maxWorkers: number;

  constructor(
    config: ParallelJobConfig<T> & {
      process: (item: T, index: number) => R;
    }
  ) {
    super(config);
    this.items = config.items;
    this.processFunc = config.process;
    this.batchSize = config.batchSize || Math.ceil(this.items.length / 4);
    this.maxWorkers = config.maxWorkers || 4;
  }

  protected async run(_context: JobContext): Promise<R[]> {
    const results: R[] = new Array(this.items.length);

    // Split items into batches
    const batches: T[][] = [];
    for (let i = 0; i < this.items.length; i += this.batchSize) {
      batches.push(this.items.slice(i, i + this.batchSize));
    }

    // Process batches in parallel (limited by maxWorkers)
    const processBatch = async (
      batch: T[],
      startIndex: number
    ): Promise<void> => {
      for (let i = 0; i < batch.length; i++) {
        if (this.cancelled) break;
        results[startIndex + i] = this.processFunc(batch[i], startIndex + i);
      }
    };

    // Process batches with worker limit
    const workers: Promise<void>[] = [];
    for (let i = 0; i < batches.length; i++) {
      if (workers.length >= this.maxWorkers) {
        await Promise.race(workers);
        workers.splice(
          workers.findIndex((p) => p === undefined),
          1
        );
      }

      const startIndex = i * this.batchSize;
      workers.push(processBatch(batches[i], startIndex));
    }

    await Promise.all(workers);

    return results;
  }
}
