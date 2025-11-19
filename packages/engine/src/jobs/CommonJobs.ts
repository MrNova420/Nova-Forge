/**
 * Copyright (c) 2025 Kayden Shawn Massengill (WeNova Interactive)
 * Licensed under the Nova Engine License - see LICENSE file
 */

import {
  IJob,
  IParallelForJob,
  IAsyncLoadJob,
  JobContext,
  JobPriority,
  JobOptions,
} from './JobTypes';

/**
 * Generate a unique job ID
 */
function generateJobId(name: string): string {
  return `${name}_${Date.now()}_${Math.random().toString(36).substring(2, 9)}`;
}

/**
 * Simple job implementation
 */
export class SimpleJob<T = any> implements IJob<T> {
  public readonly id: string;
  public readonly name: string;
  public readonly priority: JobPriority;
  public readonly dependencies?: string[];
  public readonly cancellable?: boolean;

  private fn: (context: JobContext) => Promise<T> | T;
  private cleanupFn?: () => void;

  constructor(
    fn: (context: JobContext) => Promise<T> | T,
    options: JobOptions = {}
  ) {
    this.name = options.name || 'SimpleJob';
    this.id = generateJobId(this.name);
    this.priority = options.priority ?? JobPriority.NORMAL;
    this.dependencies = options.dependencies;
    this.cancellable = options.cancellable ?? true;
    this.fn = fn;
  }

  async execute(context: JobContext): Promise<T> {
    return await this.fn(context);
  }

  cleanup(): void {
    this.cleanupFn?.();
  }

  setCleanup(fn: () => void): void {
    this.cleanupFn = fn;
  }
}

/**
 * Parallel for-loop job
 * Splits iterations across workers
 */
export class ParallelForJob<T = any> implements IParallelForJob<T> {
  public readonly id: string;
  public readonly name: string;
  public readonly priority: JobPriority;
  public readonly dependencies?: string[];
  public readonly cancellable = true;
  public readonly count: number;
  public readonly fn: (index: number, context: JobContext) => Promise<T> | T;
  public readonly chunkSize: number;

  constructor(
    count: number,
    fn: (index: number, context: JobContext) => Promise<T> | T,
    options: JobOptions & { chunkSize?: number } = {}
  ) {
    this.name = options.name || 'ParallelFor';
    this.id = generateJobId(this.name);
    this.count = count;
    this.fn = fn;
    this.priority = options.priority ?? JobPriority.NORMAL;
    this.dependencies = options.dependencies;
    this.chunkSize = options.chunkSize ?? Math.ceil(count / 4);
  }

  async execute(context: JobContext): Promise<T[]> {
    const results: T[] = [];
    const promises: Promise<void>[] = [];

    // Split work into chunks
    for (let i = 0; i < this.count; i += this.chunkSize) {
      const start = i;
      const end = Math.min(i + this.chunkSize, this.count);

      promises.push(
        (async () => {
          for (let j = start; j < end; j++) {
            if (context.cancelled) break;
            results[j] = await this.fn(j, context);

            // Update progress
            context.progress = (j + 1) / this.count;
          }
        })()
      );
    }

    await Promise.all(promises);
    return results;
  }
}

/**
 * Async loading job
 * Loads a resource with retry logic
 */
export class AsyncLoadJob<T = any> implements IAsyncLoadJob<T> {
  public readonly id: string;
  public readonly name: string;
  public readonly priority: JobPriority;
  public readonly dependencies?: string[];
  public readonly cancellable = true;
  public readonly url: string;
  public readonly timeout: number;
  public readonly retries: number;

  private loadFn: (url: string) => Promise<T>;

  constructor(
    url: string,
    loadFn: (url: string) => Promise<T>,
    options: JobOptions & { timeout?: number; retries?: number } = {}
  ) {
    this.name = options.name || 'AsyncLoad';
    this.id = generateJobId(this.name);
    this.url = url;
    this.loadFn = loadFn;
    this.priority = options.priority ?? JobPriority.HIGH;
    this.dependencies = options.dependencies;
    this.timeout = options.timeout ?? 30000;
    this.retries = options.retries ?? 3;
  }

  async execute(context: JobContext): Promise<T> {
    let lastError: Error | null = null;
    let attempt = 0;

    while (attempt < this.retries) {
      if (context.cancelled) {
        throw new Error('Job cancelled');
      }

      try {
        context.progress = attempt / this.retries;

        // Create timeout promise
        const timeoutPromise = new Promise<never>((_, reject) => {
          setTimeout(() => reject(new Error('Load timeout')), this.timeout);
        });

        // Race load against timeout
        const result = await Promise.race([
          this.loadFn(this.url),
          timeoutPromise,
        ]);

        context.progress = 1.0;
        return result;
      } catch (error) {
        lastError = error as Error;
        attempt++;

        if (attempt < this.retries) {
          // Exponential backoff
          await new Promise((resolve) =>
            setTimeout(resolve, Math.pow(2, attempt) * 1000)
          );
        }
      }
    }

    throw lastError || new Error('Failed to load resource');
  }
}

/**
 * Batch job - executes multiple jobs sequentially
 */
export class BatchJob<T = any> implements IJob<T[]> {
  public readonly id: string;
  public readonly name: string;
  public readonly priority: JobPriority;
  public readonly dependencies?: string[];
  public readonly cancellable = true;

  private jobs: IJob<T>[];

  constructor(jobs: IJob<T>[], options: JobOptions = {}) {
    this.name = options.name || 'Batch';
    this.id = generateJobId(this.name);
    this.jobs = jobs;
    this.priority = options.priority ?? JobPriority.NORMAL;
    this.dependencies = options.dependencies;
  }

  async execute(context: JobContext): Promise<T[]> {
    const results: T[] = [];

    for (let i = 0; i < this.jobs.length; i++) {
      if (context.cancelled) break;

      const job = this.jobs[i];
      const result = await job.execute(context);
      results.push(result);

      // Update progress
      context.progress = (i + 1) / this.jobs.length;
    }

    return results;
  }

  cleanup(): void {
    this.jobs.forEach((job) => job.cleanup?.());
  }
}

/**
 * Delayed job - executes after a delay
 */
export class DelayedJob<T = any> implements IJob<T> {
  public readonly id: string;
  public readonly name: string;
  public readonly priority: JobPriority;
  public readonly dependencies?: string[];
  public readonly cancellable = true;

  private innerJob: IJob<T>;
  private delayMs: number;

  constructor(job: IJob<T>, delayMs: number, options: JobOptions = {}) {
    this.name = options.name || `Delayed_${job.name}`;
    this.id = generateJobId(this.name);
    this.innerJob = job;
    this.delayMs = delayMs;
    this.priority = options.priority ?? job.priority;
    this.dependencies = options.dependencies;
  }

  async execute(context: JobContext): Promise<T> {
    // Wait for delay
    await new Promise((resolve) => setTimeout(resolve, this.delayMs));

    if (context.cancelled) {
      throw new Error('Job cancelled');
    }

    return await this.innerJob.execute(context);
  }

  cleanup(): void {
    this.innerJob.cleanup?.();
  }
}

/**
 * Repeating job - executes multiple times
 */
export class RepeatingJob<T = any> implements IJob<T[]> {
  public readonly id: string;
  public readonly name: string;
  public readonly priority: JobPriority;
  public readonly dependencies?: string[];
  public readonly cancellable = true;

  private innerJob: IJob<T>;
  private repeatCount: number;
  private delayBetween: number;

  constructor(
    job: IJob<T>,
    repeatCount: number,
    delayBetween: number = 0,
    options: JobOptions = {}
  ) {
    this.name = options.name || `Repeating_${job.name}`;
    this.id = generateJobId(this.name);
    this.innerJob = job;
    this.repeatCount = repeatCount;
    this.delayBetween = delayBetween;
    this.priority = options.priority ?? job.priority;
    this.dependencies = options.dependencies;
  }

  async execute(context: JobContext): Promise<T[]> {
    const results: T[] = [];

    for (let i = 0; i < this.repeatCount; i++) {
      if (context.cancelled) break;

      const result = await this.innerJob.execute(context);
      results.push(result);

      context.progress = (i + 1) / this.repeatCount;

      // Delay between repeats
      if (i < this.repeatCount - 1 && this.delayBetween > 0) {
        await new Promise((resolve) => setTimeout(resolve, this.delayBetween));
      }
    }

    return results;
  }

  cleanup(): void {
    this.innerJob.cleanup?.();
  }
}
