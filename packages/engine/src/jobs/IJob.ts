/**
 * @fileoverview Job System - Core Interfaces
 * @module @nova-engine/jobs
 *
 * Defines the job system for parallel processing and multi-threading.
 * Enables efficient CPU utilization across multiple cores.
 *
 * Following the autonomous development guide Phase 1.2.1
 * Research insights from:
 * - Unreal Engine's Task Graph System
 * - Unity's Job System (ECS Jobs, C# Job System)
 * - Godot's Thread Pool
 */

/**
 * Job priority levels
 */
export enum JobPriority {
  /** Critical jobs that must run immediately */
  CRITICAL = 0,
  /** High priority jobs (rendering, input) */
  HIGH = 1,
  /** Normal priority jobs (game logic) */
  NORMAL = 2,
  /** Low priority jobs (background tasks) */
  LOW = 3,
  /** Lowest priority jobs (cleanup, logging) */
  IDLE = 4,
}

/**
 * Job status
 */
export enum JobStatus {
  /** Job is waiting in queue */
  PENDING = 'pending',
  /** Job is currently executing */
  RUNNING = 'running',
  /** Job completed successfully */
  COMPLETED = 'completed',
  /** Job failed with error */
  FAILED = 'failed',
  /** Job was cancelled */
  CANCELLED = 'cancelled',
}

/**
 * Job execution context
 */
export interface JobContext {
  /** Job ID */
  id: string;
  /** Worker ID executing this job */
  workerId: number;
  /** Job start time */
  startTime: number;
  /** Shared data accessible to all jobs */
  sharedData?: Map<string, unknown>;
}

/**
 * Job result
 */
export interface JobResult<T = unknown> {
  /** Whether job succeeded */
  success: boolean;
  /** Result data if successful */
  data?: T;
  /** Error if failed */
  error?: Error;
  /** Execution time in milliseconds */
  executionTime: number;
}

/**
 * Core Job interface
 *
 * All jobs must implement this interface.
 * Jobs can be executed synchronously or asynchronously.
 */
export interface IJob<T = unknown> {
  /** Unique job identifier */
  readonly id: string;

  /** Job name for debugging */
  readonly name: string;

  /** Job priority */
  readonly priority: JobPriority;

  /** Job status */
  status: JobStatus;

  /** Jobs that must complete before this job can run */
  dependencies: string[];

  /** Whether this job can be cancelled */
  cancellable: boolean;

  /**
   * Execute the job
   * @param context Execution context
   * @returns Job result
   */
  execute(context: JobContext): Promise<JobResult<T>>;

  /**
   * Cancel the job if possible
   * @returns Whether cancellation was successful
   */
  cancel(): boolean;

  /**
   * Called when job completes (success or failure)
   */
  onComplete?(result: JobResult<T>): void;
}

/**
 * Job configuration
 */
export interface JobConfig {
  /** Job name */
  name: string;
  /** Job priority */
  priority?: JobPriority;
  /** Job dependencies */
  dependencies?: string[];
  /** Whether job can be cancelled */
  cancellable?: boolean;
  /** Timeout in milliseconds */
  timeout?: number;
}

/**
 * Parallel job configuration
 */
export interface ParallelJobConfig<T = unknown> extends JobConfig {
  /** Data items to process in parallel */
  items: T[];
  /** Number of items to process per batch */
  batchSize?: number;
  /** Maximum number of parallel workers */
  maxWorkers?: number;
}

/**
 * Job statistics
 */
export interface JobStats {
  /** Total jobs submitted */
  totalJobs: number;
  /** Jobs completed successfully */
  completedJobs: number;
  /** Jobs that failed */
  failedJobs: number;
  /** Jobs cancelled */
  cancelledJobs: number;
  /** Currently pending jobs */
  pendingJobs: number;
  /** Currently running jobs */
  runningJobs: number;
  /** Average execution time (ms) */
  averageExecutionTime: number;
  /** Total execution time (ms) */
  totalExecutionTime: number;
}
