/**
 * Copyright (c) 2025 Kayden Shawn Massengill (WeNova Interactive)
 * Licensed under the Nova Engine License - see LICENSE file
 */

import { getPlatform } from '../platform';
import {
  IJob,
  JobContext,
  JobResult,
  JobStatus,
  JobPriority,
  JobSchedulerConfig,
  JobSchedulerStats,
  JobEvent,
  JobEventType,
  JobEventListener,
} from './JobTypes';

/**
 * Priority queue for job management
 */
class PriorityQueue<T extends { priority: number }> {
  private items: T[] = [];

  enqueue(item: T): void {
    // Insert in priority order (higher priority first)
    let inserted = false;
    for (let i = 0; i < this.items.length; i++) {
      if (item.priority > this.items[i].priority) {
        this.items.splice(i, 0, item);
        inserted = true;
        break;
      }
    }
    if (!inserted) {
      this.items.push(item);
    }
  }

  dequeue(): T | undefined {
    return this.items.shift();
  }

  peek(): T | undefined {
    return this.items[0];
  }

  get length(): number {
    return this.items.length;
  }

  clear(): void {
    this.items = [];
  }

  toArray(): T[] {
    return [...this.items];
  }
}

/**
 * Job wrapper with metadata
 */
interface JobWrapper<T = any> {
  job: IJob<T>;
  priority: number;
  queuedTime: number;
  attempts: number;
  timeout?: number;
}

/**
 * Worker state
 */
interface WorkerState {
  id: number;
  busy: boolean;
  currentJob?: string;
  jobsExecuted: number;
}

/**
 * Job Scheduler
 *
 * Manages job execution across multiple workers/threads
 */
export class JobScheduler {
  private config: Required<JobSchedulerConfig>;
  private queue: PriorityQueue<JobWrapper>;
  private jobs: Map<string, JobWrapper>;
  private results: Map<string, JobResult>;
  private workers: WorkerState[];
  private running: boolean;
  private stats: JobSchedulerStats;
  private eventListeners: Map<JobEventType, Set<JobEventListener>>;
  private executionTimes: number[];
  private peakQueueSize: number;

  constructor(config: JobSchedulerConfig = {}) {
    // Initialize configuration with defaults
    this.config = {
      workerCount:
        config.workerCount ??
        Math.max(1, getPlatform().threading.getCoreCount() - 1),
      maxQueueSize: config.maxQueueSize ?? 1000,
      enableProfiling: config.enableProfiling ?? false,
      defaultTimeout: config.defaultTimeout ?? 30000,
      autoStart: config.autoStart ?? true,
    };

    this.queue = new PriorityQueue();
    this.jobs = new Map();
    this.results = new Map();
    this.workers = [];
    this.running = false;
    this.executionTimes = [];
    this.peakQueueSize = 0;

    // Initialize statistics
    this.stats = {
      totalExecuted: 0,
      totalFailed: 0,
      totalCancelled: 0,
      pending: 0,
      running: 0,
      avgExecutionTime: 0,
      peakQueueSize: 0,
      workerUtilization: 0,
    };

    // Initialize event listeners
    this.eventListeners = new Map();
    Object.values(JobEventType).forEach((type) => {
      this.eventListeners.set(type as JobEventType, new Set());
    });

    // Initialize workers
    for (let i = 0; i < this.config.workerCount; i++) {
      this.workers.push({
        id: i,
        busy: false,
        jobsExecuted: 0,
      });
    }

    if (this.config.autoStart) {
      this.start();
    }
  }

  /**
   * Start the scheduler
   */
  start(): void {
    if (this.running) return;
    this.running = true;
    this.processQueue();
  }

  /**
   * Stop the scheduler
   */
  stop(): void {
    this.running = false;
  }

  /**
   * Submit a job for execution
   * @param job Job to execute
   * @param timeout Optional timeout override
   * @returns Promise that resolves with job result
   */
  async submit<T>(job: IJob<T>, timeout?: number): Promise<JobResult<T>> {
    // Check queue size limit
    if (this.queue.length >= this.config.maxQueueSize) {
      throw new Error(`Job queue is full (max: ${this.config.maxQueueSize})`);
    }

    // Create job wrapper
    const wrapper: JobWrapper<T> = {
      job,
      priority: job.priority ?? JobPriority.NORMAL,
      queuedTime: Date.now(),
      attempts: 0,
      timeout: timeout ?? this.config.defaultTimeout,
    };

    // Store job
    this.jobs.set(job.id, wrapper);

    // Add to queue
    this.queue.enqueue(wrapper);
    this.stats.pending++;

    // Update peak queue size
    if (this.queue.length > this.peakQueueSize) {
      this.peakQueueSize = this.queue.length;
      this.stats.peakQueueSize = this.peakQueueSize;
    }

    // Emit queued event
    this.emitEvent({
      type: JobEventType.JOB_QUEUED,
      jobId: job.id,
      timestamp: Date.now(),
    });

    // Trigger queue processing
    if (this.running) {
      this.processQueue();
    }

    // Wait for result
    return new Promise((resolve, reject) => {
      const checkResult = () => {
        const result = this.results.get(job.id);
        if (result) {
          // Clean up
          this.jobs.delete(job.id);
          this.results.delete(job.id);

          if (result.status === JobStatus.COMPLETED) {
            resolve(result);
          } else {
            reject(result.error || new Error(`Job ${job.id} failed`));
          }
        } else {
          // Check again after delay
          setTimeout(checkResult, 10);
        }
      };
      checkResult();
    });
  }

  /**
   * Submit multiple jobs
   */
  async submitBatch<T>(jobs: IJob<T>[]): Promise<JobResult<T>[]> {
    return Promise.all(jobs.map((job) => this.submit(job)));
  }

  /**
   * Cancel a job
   * @param jobId Job ID to cancel
   * @returns true if cancelled, false if not found or already executing
   */
  cancel(jobId: string): boolean {
    const wrapper = this.jobs.get(jobId);
    if (!wrapper) return false;

    // Remove from queue
    const queueItems = this.queue.toArray();
    this.queue.clear();
    queueItems.forEach((item) => {
      if (item.job.id !== jobId) {
        this.queue.enqueue(item);
      }
    });

    // Mark as cancelled
    this.results.set(jobId, {
      id: jobId,
      status: JobStatus.CANCELLED,
      executionTime: 0,
      workerId: -1,
    });

    this.stats.totalCancelled++;
    this.stats.pending = Math.max(0, this.stats.pending - 1);

    this.emitEvent({
      type: JobEventType.JOB_CANCELLED,
      jobId,
      timestamp: Date.now(),
    });

    return true;
  }

  /**
   * Get job result
   */
  getResult<T>(jobId: string): JobResult<T> | undefined {
    return this.results.get(jobId) as JobResult<T> | undefined;
  }

  /**
   * Get scheduler statistics
   */
  getStats(): JobSchedulerStats {
    // Update worker utilization
    const busyWorkers = this.workers.filter((w) => w.busy).length;
    this.stats.workerUtilization = busyWorkers / this.workers.length;
    this.stats.running = busyWorkers;
    this.stats.pending = this.queue.length;

    return { ...this.stats };
  }

  /**
   * Add event listener
   */
  addEventListener(type: JobEventType, listener: JobEventListener): void {
    this.eventListeners.get(type)?.add(listener);
  }

  /**
   * Remove event listener
   */
  removeEventListener(type: JobEventType, listener: JobEventListener): void {
    this.eventListeners.get(type)?.delete(listener);
  }

  /**
   * Clear all jobs and reset scheduler
   */
  clear(): void {
    this.queue.clear();
    this.jobs.clear();
    this.results.clear();
    this.stats.pending = 0;
    this.stats.running = 0;
  }

  /**
   * Shutdown the scheduler
   */
  async shutdown(): Promise<void> {
    this.stop();
    this.clear();
  }

  /**
   * Process the job queue
   */
  private async processQueue(): Promise<void> {
    if (!this.running) return;

    // Find available workers
    const availableWorkers = this.workers.filter((w) => !w.busy);
    if (availableWorkers.length === 0) return;

    // Dequeue jobs for available workers
    for (const worker of availableWorkers) {
      const wrapper = this.queue.dequeue();
      if (!wrapper) break;

      this.stats.pending = Math.max(0, this.stats.pending - 1);
      worker.busy = true;
      worker.currentJob = wrapper.job.id;

      // Execute job asynchronously
      this.executeJob(wrapper, worker);
    }

    // Continue processing if there are more jobs
    if (this.queue.length > 0) {
      setTimeout(() => this.processQueue(), 0);
    }
  }

  /**
   * Execute a job
   */
  private async executeJob<T>(
    wrapper: JobWrapper<T>,
    worker: WorkerState
  ): Promise<void> {
    const { job } = wrapper;
    const startTime = Date.now();

    // Create execution context
    const context: JobContext = {
      id: job.id,
      workerId: worker.id,
      startTime,
      cancelled: false,
      progress: 0,
    };

    // Emit started event
    this.emitEvent({
      type: JobEventType.JOB_STARTED,
      jobId: job.id,
      timestamp: startTime,
      data: { workerId: worker.id },
    });

    try {
      // Execute with timeout
      const timeoutPromise = new Promise<never>((_, reject) => {
        if (wrapper.timeout) {
          setTimeout(() => reject(new Error('Job timeout')), wrapper.timeout);
        }
      });

      const result = await Promise.race([job.execute(context), timeoutPromise]);

      const executionTime = Date.now() - startTime;

      // Store result
      this.results.set(job.id, {
        id: job.id,
        status: JobStatus.COMPLETED,
        result,
        executionTime,
        workerId: worker.id,
      });

      // Update statistics
      this.stats.totalExecuted++;
      worker.jobsExecuted++;
      this.executionTimes.push(executionTime);

      // Calculate rolling average (last 100 jobs)
      if (this.executionTimes.length > 100) {
        this.executionTimes.shift();
      }
      this.stats.avgExecutionTime =
        this.executionTimes.reduce((sum, time) => sum + time, 0) /
        this.executionTimes.length;

      // Emit completed event
      this.emitEvent({
        type: JobEventType.JOB_COMPLETED,
        jobId: job.id,
        timestamp: Date.now(),
        data: { executionTime, workerId: worker.id },
      });

      // Cleanup
      job.cleanup?.();
    } catch (error) {
      const executionTime = Date.now() - startTime;

      // Store error result
      this.results.set(job.id, {
        id: job.id,
        status: JobStatus.FAILED,
        error: error as Error,
        executionTime,
        workerId: worker.id,
      });

      this.stats.totalFailed++;

      // Emit failed event
      this.emitEvent({
        type: JobEventType.JOB_FAILED,
        jobId: job.id,
        timestamp: Date.now(),
        data: { error: (error as Error).message, workerId: worker.id },
      });
    } finally {
      // Mark worker as available
      worker.busy = false;
      worker.currentJob = undefined;

      // Continue processing queue
      this.processQueue();
    }
  }

  /**
   * Emit an event
   */
  private emitEvent(event: JobEvent): void {
    const listeners = this.eventListeners.get(event.type);
    if (listeners) {
      listeners.forEach((listener) => {
        try {
          listener(event);
        } catch (error) {
          console.error('Error in job event listener:', error);
        }
      });
    }
  }
}

/**
 * Global job scheduler instance
 */
let globalScheduler: JobScheduler | null = null;

/**
 * Get the global job scheduler
 */
export function getJobScheduler(): JobScheduler {
  if (!globalScheduler) {
    globalScheduler = new JobScheduler();
  }
  return globalScheduler;
}

/**
 * Set the global job scheduler
 */
export function setJobScheduler(scheduler: JobScheduler): void {
  globalScheduler = scheduler;
}
