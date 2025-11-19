/**
 * Copyright (c) 2025 Kayden Shawn Massengill (WeNova Interactive)
 * Licensed under the Nova Engine License - see LICENSE file
 */

/**
 * Job System Types and Interfaces
 *
 * The job system provides a flexible, high-performance task execution
 * framework for parallel and asynchronous work.
 */

/**
 * Job priority levels
 * Higher values = higher priority
 */
export enum JobPriority {
  CRITICAL = 1000, // Must execute immediately
  HIGH = 100, // Important, execute soon
  NORMAL = 50, // Default priority
  LOW = 10, // Execute when possible
  BACKGROUND = 1, // Lowest priority, can be delayed
}

/**
 * Job execution status
 */
export enum JobStatus {
  PENDING = 'pending', // Waiting to execute
  RUNNING = 'running', // Currently executing
  COMPLETED = 'completed', // Successfully completed
  FAILED = 'failed', // Failed with error
  CANCELLED = 'cancelled', // Cancelled before execution
}

/**
 * Job execution context
 */
export interface JobContext {
  /**
   * Job ID
   */
  readonly id: string;

  /**
   * Worker/thread ID executing the job
   */
  readonly workerId: number;

  /**
   * Start time of job execution
   */
  readonly startTime: number;

  /**
   * Cancellation token
   */
  readonly cancelled: boolean;

  /**
   * Progress reporting (0.0 to 1.0)
   */
  progress: number;
}

/**
 * Job execution result
 */
export interface JobResult<T = any> {
  /**
   * Job ID
   */
  id: string;

  /**
   * Execution status
   */
  status: JobStatus;

  /**
   * Result data (if successful)
   */
  result?: T;

  /**
   * Error (if failed)
   */
  error?: Error;

  /**
   * Execution time in milliseconds
   */
  executionTime: number;

  /**
   * Worker that executed the job
   */
  workerId: number;
}

/**
 * Base job interface
 */
export interface IJob<T = any> {
  /**
   * Unique job identifier
   */
  readonly id: string;

  /**
   * Job name for debugging
   */
  readonly name: string;

  /**
   * Job priority
   */
  readonly priority: JobPriority;

  /**
   * Job dependencies (must complete before this job)
   */
  readonly dependencies?: string[];

  /**
   * Execute the job
   * @param context Execution context
   * @returns Job result
   */
  execute(context: JobContext): Promise<T> | T;

  /**
   * Optional cleanup after execution
   */
  cleanup?(): void;

  /**
   * Check if job can be cancelled
   */
  readonly cancellable?: boolean;
}

/**
 * Parallel for-loop job
 * Executes a function for each element in parallel
 */
export interface IParallelForJob<T> extends IJob<T[]> {
  /**
   * Number of iterations
   */
  readonly count: number;

  /**
   * Function to execute for each index
   */
  readonly fn: (index: number, context: JobContext) => Promise<T> | T;

  /**
   * Chunk size for distribution
   */
  readonly chunkSize?: number;
}

/**
 * Async loading job
 * Loads a resource asynchronously
 */
export interface IAsyncLoadJob<T> extends IJob<T> {
  /**
   * Resource URL or path
   */
  readonly url: string;

  /**
   * Load timeout in milliseconds
   */
  readonly timeout?: number;

  /**
   * Retry count on failure
   */
  readonly retries?: number;
}

/**
 * Update job
 * Executes regularly as part of game loop
 */
export interface IUpdateJob extends IJob<void> {
  /**
   * Delta time since last update (ms)
   */
  deltaTime: number;

  /**
   * Total elapsed time (ms)
   */
  totalTime: number;
}

/**
 * Job configuration options
 */
export interface JobOptions {
  /**
   * Job name
   */
  name?: string;

  /**
   * Job priority
   */
  priority?: JobPriority;

  /**
   * Dependencies
   */
  dependencies?: string[];

  /**
   * Is cancellable
   */
  cancellable?: boolean;

  /**
   * Timeout in milliseconds
   */
  timeout?: number;
}

/**
 * Job scheduler configuration
 */
export interface JobSchedulerConfig {
  /**
   * Number of worker threads
   * Default: CPU core count - 1
   */
  workerCount?: number;

  /**
   * Maximum queue size
   * Default: 1000
   */
  maxQueueSize?: number;

  /**
   * Enable job profiling
   * Default: false
   */
  enableProfiling?: boolean;

  /**
   * Job timeout in milliseconds
   * Default: 30000 (30 seconds)
   */
  defaultTimeout?: number;

  /**
   * Auto-start scheduler
   * Default: true
   */
  autoStart?: boolean;
}

/**
 * Job scheduler statistics
 */
export interface JobSchedulerStats {
  /**
   * Total jobs executed
   */
  totalExecuted: number;

  /**
   * Total jobs failed
   */
  totalFailed: number;

  /**
   * Total jobs cancelled
   */
  totalCancelled: number;

  /**
   * Jobs currently pending
   */
  pending: number;

  /**
   * Jobs currently running
   */
  running: number;

  /**
   * Average execution time (ms)
   */
  avgExecutionTime: number;

  /**
   * Peak queue size
   */
  peakQueueSize: number;

  /**
   * Worker utilization (0.0 to 1.0)
   */
  workerUtilization: number;
}

/**
 * Job event types
 */
export enum JobEventType {
  JOB_QUEUED = 'job_queued',
  JOB_STARTED = 'job_started',
  JOB_COMPLETED = 'job_completed',
  JOB_FAILED = 'job_failed',
  JOB_CANCELLED = 'job_cancelled',
  JOB_PROGRESS = 'job_progress',
}

/**
 * Job event data
 */
export interface JobEvent {
  /**
   * Event type
   */
  type: JobEventType;

  /**
   * Job ID
   */
  jobId: string;

  /**
   * Event timestamp
   */
  timestamp: number;

  /**
   * Additional event data
   */
  data?: any;
}

/**
 * Job event listener
 */
export type JobEventListener = (event: JobEvent) => void;
