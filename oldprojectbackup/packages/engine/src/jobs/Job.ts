/**
 * @fileoverview Base Job Implementation
 * @module @nova-engine/jobs
 *
 * Base class for all job implementations.
 * Provides common functionality for job lifecycle management.
 *
 * Following the autonomous development guide Phase 1.2.3
 */

import {
  type IJob,
  type JobConfig,
  type JobContext,
  type JobPriority,
  type JobResult,
  JobStatus,
} from './IJob';

/**
 * Abstract base class for jobs
 *
 * Provides common implementation for job lifecycle.
 * Subclasses must implement the run() method.
 */
export abstract class Job<T = unknown> implements IJob<T> {
  readonly id: string;
  readonly name: string;
  readonly priority: JobPriority;
  status: JobStatus;
  dependencies: string[];
  cancellable: boolean;

  protected cancelled = false;
  protected onCompleteCallback?: (result: JobResult<T>) => void;

  constructor(config: JobConfig) {
    this.id = `job-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`;
    this.name = config.name;
    this.priority = config.priority || 2; // Default to NORMAL
    this.dependencies = config.dependencies || [];
    this.cancellable = config.cancellable !== false;
    this.status = JobStatus.PENDING;
  }

  /**
   * Execute the job
   */
  async execute(context: JobContext): Promise<JobResult<T>> {
    if (this.cancelled) {
      return {
        success: false,
        error: new Error('Job was cancelled'),
        executionTime: 0,
      };
    }

    const startTime = Date.now();
    this.status = JobStatus.RUNNING;

    try {
      const data = await this.run(context);
      const executionTime = Date.now() - startTime;

      this.status = JobStatus.COMPLETED;
      return {
        success: true,
        data,
        executionTime,
      };
    } catch (error) {
      const executionTime = Date.now() - startTime;
      this.status = JobStatus.FAILED;

      return {
        success: false,
        error: error as Error,
        executionTime,
      };
    }
  }

  /**
   * Cancel the job
   */
  cancel(): boolean {
    if (!this.cancellable) {
      return false;
    }

    if (this.status === JobStatus.PENDING || this.status === JobStatus.RUNNING) {
      this.cancelled = true;
      this.status = JobStatus.CANCELLED;
      return true;
    }

    return false;
  }

  /**
   * Set completion callback (internal use)
   */
  onComplete?: (result: JobResult<T>) => void;

  /**
   * Run the job - must be implemented by subclasses
   */
  protected abstract run(context: JobContext): Promise<T>;
}
