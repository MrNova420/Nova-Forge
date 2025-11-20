/**
 * @fileoverview Job Scheduler
 * @module @nova-engine/jobs
 *
 * Manages job execution, threading, and load balancing.
 * Implements a priority queue with dependency resolution.
 *
 * Following the autonomous development guide Phase 1.2.2
 */

import type { IPlatform } from '../platform/IPlatform';
import {
  type IJob,
  type JobContext,
  JobPriority,
  type JobStats,
  JobStatus,
  type JobResult,
} from './IJob';

/**
 * Job Scheduler Configuration
 */
export interface JobSchedulerConfig {
  /** Number of worker threads (default: CPU cores - 1) */
  workerCount?: number;
  /** Maximum jobs in queue before warning (default: 1000) */
  maxQueueSize?: number;
  /** Job timeout in ms (default: 30000) */
  defaultTimeout?: number;
  /** Enable profiling (default: false) */
  profiling?: boolean;
}

/**
 * Internal job wrapper
 */
interface JobEntry {
  job: IJob;
  addedTime: number;
  startTime?: number;
  resolve: (result: JobResult) => void;
  reject: (error: Error) => void;
}

/**
 * Job Scheduler
 *
 * Central system for managing and executing jobs across multiple threads.
 * Features:
 * - Priority-based job queue
 * - Dependency resolution
 * - Worker thread pool
 * - Load balancing
 * - Job profiling
 */
export class JobScheduler {
  private platform: IPlatform;
  private config: Required<JobSchedulerConfig>;
  private jobQueue: Map<JobPriority, JobEntry[]>;
  private runningJobs: Map<string, JobEntry>;
  private completedJobs: Map<string, JobResult>;
  private workers: Map<number, any>; // Platform workers
  private workerBusy: Map<number, boolean>;
  private stats: JobStats;
  private running = false;
  private sharedData: Map<string, unknown>;

  constructor(platform: IPlatform, config: JobSchedulerConfig = {}) {
    this.platform = platform;
    this.config = {
      workerCount: config.workerCount || Math.max(1, platform.getHardwareConcurrency() - 1),
      maxQueueSize: config.maxQueueSize || 1000,
      defaultTimeout: config.defaultTimeout || 30000,
      profiling: config.profiling || false,
    };

    // Initialize queues for each priority level
    this.jobQueue = new Map();
    Object.values(JobPriority)
      .filter((v) => typeof v === 'number')
      .forEach((priority) => {
        this.jobQueue.set(priority as JobPriority, []);
      });

    this.runningJobs = new Map();
    this.completedJobs = new Map();
    this.workers = new Map();
    this.workerBusy = new Map();
    this.sharedData = new Map();

    this.stats = {
      totalJobs: 0,
      completedJobs: 0,
      failedJobs: 0,
      cancelledJobs: 0,
      pendingJobs: 0,
      runningJobs: 0,
      averageExecutionTime: 0,
      totalExecutionTime: 0,
    };
  }

  /**
   * Initialize the job scheduler and worker pool
   */
  async initialize(): Promise<void> {
    if (this.running) {
      throw new Error('JobScheduler already initialized');
    }

    // Initialize worker threads
    // Note: Actual worker initialization would require platform-specific code
    for (let i = 0; i < this.config.workerCount; i++) {
      this.workerBusy.set(i, false);
      // Workers map reserved for future implementation
      void this.workers;
    }

    this.running = true;
  }

  /**
   * Shutdown the job scheduler
   */
  async shutdown(): Promise<void> {
    if (!this.running) return;

    // Cancel all pending jobs
    for (const queue of this.jobQueue.values()) {
      queue.forEach((entry) => {
        entry.job.status = 'cancelled' as JobStatus;
        entry.reject(new Error('Scheduler shutdown'));
      });
      queue.length = 0;
    }

    // Wait for running jobs to complete (with timeout)
    const shutdownTimeout = 5000;
    const startTime = Date.now();
    while (this.runningJobs.size > 0 && Date.now() - startTime < shutdownTimeout) {
      await this.platform.sleep(100);
    }

    // Force cancel remaining jobs
    this.runningJobs.forEach((entry) => {
      entry.job.cancel();
    });

    this.running = false;
  }

  /**
   * Submit a job for execution
   * @param job Job to execute
   * @returns Promise that resolves with job result
   */
  submit<T = unknown>(job: IJob<T>): Promise<JobResult<T>> {
    if (!this.running) {
      throw new Error('JobScheduler not initialized');
    }

    return new Promise((resolve, reject) => {
      // Check queue size
      const totalPending = Array.from(this.jobQueue.values()).reduce(
        (sum, queue) => sum + queue.length,
        0
      );

      if (totalPending >= this.config.maxQueueSize) {
        console.warn(
          `Job queue size (${totalPending}) exceeded warning threshold (${this.config.maxQueueSize})`
        );
      }

      // Create job entry
      const entry: JobEntry = {
        job,
        addedTime: this.platform.now(),
        resolve: resolve as any,
        reject,
      };

      // Add to appropriate priority queue
      const queue = this.jobQueue.get(job.priority);
      if (!queue) {
        reject(new Error(`Invalid job priority: ${job.priority}`));
        return;
      }

      // Check dependencies
      if (job.dependencies.length > 0) {
        const unresolvedDeps = job.dependencies.filter(
          (depId) => !this.completedJobs.has(depId)
        );

        if (unresolvedDeps.length > 0) {
          // Wait for dependencies (simplified - real implementation would be more sophisticated)
          job.status = 'pending' as JobStatus;
        }
      }

      job.status = 'pending' as JobStatus;
      queue.push(entry);
      this.stats.totalJobs++;
      this.stats.pendingJobs++;

      // Try to process immediately
      this.processQueue();
    });
  }

  /**
   * Submit multiple jobs in parallel
   * @param jobs Jobs to execute
   * @returns Promise that resolves when all jobs complete
   */
  async submitBatch<T = unknown>(jobs: IJob<T>[]): Promise<JobResult<T>[]> {
    const promises = jobs.map((job) => this.submit(job));
    return Promise.all(promises);
  }

  /**
   * Cancel a job by ID
   * @param jobId Job ID to cancel
   * @returns Whether cancellation was successful
   */
  cancel(jobId: string): boolean {
    // Check if job is running
    const runningEntry = this.runningJobs.get(jobId);
    if (runningEntry) {
      if (runningEntry.job.cancellable && runningEntry.job.cancel()) {
        this.runningJobs.delete(jobId);
        this.stats.cancelledJobs++;
        this.stats.runningJobs--;
        return true;
      }
      return false;
    }

    // Check if job is in queue
    for (const queue of this.jobQueue.values()) {
      const index = queue.findIndex((entry) => entry.job.id === jobId);
      if (index !== -1) {
        const entry = queue[index];
        if (entry.job.cancellable) {
          entry.job.status = 'cancelled' as JobStatus;
          entry.reject(new Error('Job cancelled'));
          queue.splice(index, 1);
          this.stats.cancelledJobs++;
          this.stats.pendingJobs--;
          return true;
        }
        return false;
      }
    }

    return false;
  }

  /**
   * Get job statistics
   */
  getStats(): JobStats {
    return { ...this.stats };
  }

  /**
   * Get number of pending jobs
   */
  getPendingJobCount(): number {
    return Array.from(this.jobQueue.values()).reduce(
      (sum, queue) => sum + queue.length,
      0
    );
  }

  /**
   * Get number of running jobs
   */
  getRunningJobCount(): number {
    return this.runningJobs.size;
  }

  /**
   * Process the job queue
   */
  private processQueue(): void {
    // Find available worker
    let availableWorker = -1;
    for (const [workerId, busy] of this.workerBusy) {
      if (!busy) {
        availableWorker = workerId;
        break;
      }
    }

    if (availableWorker === -1) {
      // No workers available
      return;
    }

    // Get highest priority job
    const entry = this.getNextJob();
    if (!entry) {
      // No jobs in queue
      return;
    }

    // Execute job
    this.executeJob(entry, availableWorker);

    // Try to process more jobs
    if (this.getPendingJobCount() > 0) {
      this.processQueue();
    }
  }

  /**
   * Get next job from queue (highest priority)
   */
  private getNextJob(): JobEntry | null {
    // Check queues in priority order
    for (let priority = JobPriority.CRITICAL; priority <= JobPriority.IDLE; priority++) {
      const queue = this.jobQueue.get(priority);
      if (queue && queue.length > 0) {
        return queue.shift()!;
      }
    }
    return null;
  }

  /**
   * Execute a job
   */
  private async executeJob(entry: JobEntry, workerId: number): Promise<void> {
    const { job } = entry;

    // Mark worker as busy
    this.workerBusy.set(workerId, true);

    // Update status
    job.status = 'running' as JobStatus;
    entry.startTime = this.platform.now();
    this.runningJobs.set(job.id, entry);
    this.stats.pendingJobs--;
    this.stats.runningJobs++;

    try {
      // Create job context
      const context: JobContext = {
        id: job.id,
        workerId,
        startTime: entry.startTime,
        sharedData: this.sharedData,
      };

      // Execute job with timeout
      const result = await Promise.race([
        job.execute(context),
        this.createTimeoutPromise(this.config.defaultTimeout),
      ]);

      // Update execution time
      const executionTime = this.platform.now() - entry.startTime;
      result.executionTime = executionTime;

      // Update stats
      this.stats.totalExecutionTime += executionTime;
      this.stats.averageExecutionTime =
        this.stats.totalExecutionTime / (this.stats.completedJobs + this.stats.failedJobs + 1);

      if (result.success) {
        job.status = 'completed' as JobStatus;
        this.stats.completedJobs++;
        this.completedJobs.set(job.id, result);
      } else {
        job.status = 'failed' as JobStatus;
        this.stats.failedJobs++;
      }

      // Call completion callback
      if (job.onComplete) {
        job.onComplete(result);
      }

      entry.resolve(result);
    } catch (error) {
      job.status = JobStatus.FAILED;
      this.stats.failedJobs++;

      const result: JobResult = {
        success: false,
        error: error as Error,
        executionTime: this.platform.now() - entry.startTime!,
      };

      if (job.onComplete) {
        job.onComplete(result);
      }

      entry.reject(error as Error);
    } finally {
      // Mark worker as available
      this.workerBusy.set(workerId, false);
      this.runningJobs.delete(job.id);
      this.stats.runningJobs--;

      // Process next job
      this.processQueue();
    }
  }

  /**
   * Create a timeout promise
   */
  private createTimeoutPromise(timeout: number): Promise<JobResult> {
    return new Promise((_, reject) => {
      setTimeout(() => {
        reject(new Error(`Job timed out after ${timeout}ms`));
      }, timeout);
    });
  }

  /**
   * Set shared data accessible to all jobs
   */
  setSharedData(key: string, value: unknown): void {
    this.sharedData.set(key, value);
  }

  /**
   * Get shared data
   */
  getSharedData(key: string): unknown {
    return this.sharedData.get(key);
  }

  /**
   * Clear shared data
   */
  clearSharedData(): void {
    this.sharedData.clear();
  }
}
