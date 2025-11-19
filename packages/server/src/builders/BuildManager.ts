/**
 * @fileoverview Build manager for orchestrating multiple build types
 */

import { BuildConfig, BuildResult, BuildTarget } from './BuildConfig';
import { WebBuilder } from './WebBuilder';
import { DesktopBuilder, DesktopBuildConfig } from './DesktopBuilder';
import { MobileBuilder, MobileBuildConfig } from './MobileBuilder';
import { ServerBuilder, ServerBuildConfig } from './ServerBuilder';

/**
 * Build request
 */
export interface BuildRequest {
  /** Project path */
  projectPath: string;
  /** Build configuration */
  config:
    | BuildConfig
    | DesktopBuildConfig
    | MobileBuildConfig
    | ServerBuildConfig;
}

/**
 * Build job status
 */
export enum BuildStatus {
  /** Build is queued */
  Queued = 'queued',
  /** Build is running */
  Running = 'running',
  /** Build completed successfully */
  Completed = 'completed',
  /** Build failed */
  Failed = 'failed',
  /** Build was cancelled */
  Cancelled = 'cancelled',
}

/**
 * Build job
 */
export interface BuildJob {
  /** Unique job ID */
  id: string;
  /** Build request */
  request: BuildRequest;
  /** Current status */
  status: BuildStatus;
  /** Build result (when completed) */
  result?: BuildResult;
  /** Start time */
  startTime?: number;
  /** End time */
  endTime?: number;
  /** Progress percentage (0-100) */
  progress: number;
}

/**
 * Build manager
 * Orchestrates build jobs, manages queue, and coordinates builders
 */
export class BuildManager {
  /** Active build jobs */
  private jobs: Map<string, BuildJob> = new Map();
  /** Build queue */
  private queue: BuildJob[] = [];
  /** Currently running job */
  private currentJob?: BuildJob;
  // Note: maxConcurrentBuilds is reserved for future use when parallel builds are implemented
  /** Next job ID */
  private nextJobId: number = 1;

  /**
   * Queue a build job
   */
  public queueBuild(request: BuildRequest): string {
    const jobId = `build-${this.nextJobId++}`;

    const job: BuildJob = {
      id: jobId,
      request,
      status: BuildStatus.Queued,
      progress: 0,
    };

    this.jobs.set(jobId, job);
    this.queue.push(job);

    // Start processing queue
    this.processQueue();

    return jobId;
  }

  /**
   * Process the build queue
   */
  private async processQueue(): Promise<void> {
    // Check if we can start a new build
    if (this.currentJob || this.queue.length === 0) {
      return;
    }

    // Get next job from queue
    const job = this.queue.shift();
    if (!job) return;

    this.currentJob = job;
    job.status = BuildStatus.Running;
    job.startTime = Date.now();

    try {
      // Execute build
      const result = await this.executeBuild(job);

      // Update job
      job.result = result;
      job.status = result.success ? BuildStatus.Completed : BuildStatus.Failed;
      job.progress = 100;
      job.endTime = Date.now();

      // eslint-disable-next-line no-console
      console.log(
        `Build ${job.id} ${job.status} in ${job.endTime - job.startTime!}ms`
      );
    } catch (error) {
      job.status = BuildStatus.Failed;
      job.result = {
        success: false,
        errors: [error instanceof Error ? error.message : String(error)],
      };
      job.endTime = Date.now();

      console.error(`Build ${job.id} failed:`, error);
    } finally {
      this.currentJob = undefined;

      // Process next job
      setTimeout(() => this.processQueue(), 0);
    }
  }

  /**
   * Execute a build
   */
  private async executeBuild(job: BuildJob): Promise<BuildResult> {
    const { request } = job;
    const { projectPath, config } = request;

    // eslint-disable-next-line no-console
    console.log(
      `Starting build ${job.id} for target ${config.target} at ${projectPath}`
    );

    // Select appropriate builder
    switch (config.target) {
      case BuildTarget.Web:
        return await this.buildWeb(projectPath, config);

      case BuildTarget.Desktop:
        return await this.buildDesktop(
          projectPath,
          config as DesktopBuildConfig
        );

      case BuildTarget.Mobile:
        return await this.buildMobile(projectPath, config as MobileBuildConfig);

      case BuildTarget.Server:
        return await this.buildServer(projectPath, config as ServerBuildConfig);

      default:
        throw new Error(`Unknown build target: ${config.target}`);
    }
  }

  /**
   * Build for web
   */
  private async buildWeb(
    projectPath: string,
    config: BuildConfig
  ): Promise<BuildResult> {
    const builder = new WebBuilder(config);
    return await builder.build(projectPath);
  }

  /**
   * Build for desktop
   */
  private async buildDesktop(
    projectPath: string,
    config: DesktopBuildConfig
  ): Promise<BuildResult> {
    const builder = new DesktopBuilder(config);
    return await builder.build(projectPath);
  }

  /**
   * Build for mobile
   */
  private async buildMobile(
    projectPath: string,
    config: MobileBuildConfig
  ): Promise<BuildResult> {
    const builder = new MobileBuilder(config);
    return await builder.build(projectPath);
  }

  /**
   * Build for server
   */
  private async buildServer(
    projectPath: string,
    config: ServerBuildConfig
  ): Promise<BuildResult> {
    const builder = new ServerBuilder(config);
    return await builder.build(projectPath);
  }

  /**
   * Get build job status
   */
  public getJob(jobId: string): BuildJob | undefined {
    return this.jobs.get(jobId);
  }

  /**
   * Get all jobs
   */
  public getAllJobs(): BuildJob[] {
    return Array.from(this.jobs.values());
  }

  /**
   * Get queue status
   */
  public getQueueStatus(): {
    queued: number;
    running: number;
    completed: number;
    failed: number;
  } {
    const jobs = Array.from(this.jobs.values());
    return {
      queued: jobs.filter((j) => j.status === BuildStatus.Queued).length,
      running: jobs.filter((j) => j.status === BuildStatus.Running).length,
      completed: jobs.filter((j) => j.status === BuildStatus.Completed).length,
      failed: jobs.filter((j) => j.status === BuildStatus.Failed).length,
    };
  }

  /**
   * Cancel a build
   */
  public cancelBuild(jobId: string): boolean {
    const job = this.jobs.get(jobId);
    if (!job) return false;

    if (job.status === BuildStatus.Queued) {
      // Remove from queue
      this.queue = this.queue.filter((j) => j.id !== jobId);
      job.status = BuildStatus.Cancelled;
      return true;
    }

    // Cannot cancel running builds for now
    return false;
  }

  /**
   * Clear completed jobs
   */
  public clearCompleted(): number {
    const completed = Array.from(this.jobs.values()).filter(
      (j) =>
        j.status === BuildStatus.Completed ||
        j.status === BuildStatus.Failed ||
        j.status === BuildStatus.Cancelled
    );

    for (const job of completed) {
      this.jobs.delete(job.id);
    }

    return completed.length;
  }
}
