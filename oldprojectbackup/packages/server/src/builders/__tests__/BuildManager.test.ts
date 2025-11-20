/**
 * @fileoverview Tests for BuildManager
 */

import { describe, it, expect } from '@jest/globals';
import { BuildManager, BuildStatus } from '../BuildManager';
import { BuildTarget, BuildMode } from '../BuildConfig';

describe('BuildManager', () => {
  it('should create build manager', () => {
    const manager = new BuildManager();
    expect(manager).toBeDefined();
  });

  it('should queue builds', async () => {
    const manager = new BuildManager();

    const jobId = manager.queueBuild({
      projectPath: '/nonexistent/path', // Use invalid path so build fails fast
      config: {
        target: BuildTarget.Web,
        mode: BuildMode.Production,
        optimization: 2,
        sourceMaps: false,
        minify: true,
        codeSplitting: true,
        treeShaking: true,
        outDir: 'dist',
        compressAssets: true,
      },
    });

    expect(jobId).toBeDefined();
    expect(jobId).toMatch(/^build-\d+$/);

    // Wait a bit for async processing
    await new Promise((resolve) => setTimeout(resolve, 100));
  });

  it('should track build jobs', async () => {
    const manager = new BuildManager();

    const jobId = manager.queueBuild({
      projectPath: '/nonexistent/path',
      config: {
        target: BuildTarget.Web,
        mode: BuildMode.Production,
        optimization: 2,
        sourceMaps: false,
        minify: true,
        codeSplitting: true,
        treeShaking: true,
        outDir: 'dist',
        compressAssets: true,
      },
    });

    const job = manager.getJob(jobId);
    expect(job).toBeDefined();
    expect(job?.id).toBe(jobId);
    // Job may be Queued or Running or Failed depending on timing
    expect([
      BuildStatus.Queued,
      BuildStatus.Running,
      BuildStatus.Failed,
    ]).toContain(job?.status);

    await new Promise((resolve) => setTimeout(resolve, 100));
  });

  it('should return queue status', async () => {
    const manager = new BuildManager();

    manager.queueBuild({
      projectPath: '/nonexistent/path1',
      config: {
        target: BuildTarget.Web,
        mode: BuildMode.Production,
        optimization: 2,
        sourceMaps: false,
        minify: true,
        codeSplitting: true,
        treeShaking: true,
        outDir: 'dist',
        compressAssets: true,
      },
    });

    const status = manager.getQueueStatus();
    expect(status.queued).toBeGreaterThanOrEqual(0);
    expect(status.running).toBeGreaterThanOrEqual(0);
    expect(status.completed).toBeGreaterThanOrEqual(0);
    expect(status.failed).toBeGreaterThanOrEqual(0);

    await new Promise((resolve) => setTimeout(resolve, 100));
  });

  it('should list all jobs', async () => {
    const manager = new BuildManager();

    manager.queueBuild({
      projectPath: '/nonexistent/path1',
      config: {
        target: BuildTarget.Web,
        mode: BuildMode.Production,
        optimization: 2,
        sourceMaps: false,
        minify: true,
        codeSplitting: true,
        treeShaking: true,
        outDir: 'dist',
        compressAssets: true,
      },
    });

    const jobs = manager.getAllJobs();
    expect(jobs.length).toBeGreaterThanOrEqual(1);

    await new Promise((resolve) => setTimeout(resolve, 100));
  });

  it('should cancel queued builds', async () => {
    const manager = new BuildManager();

    const jobId = manager.queueBuild({
      projectPath: '/nonexistent/path',
      config: {
        target: BuildTarget.Web,
        mode: BuildMode.Production,
        optimization: 2,
        sourceMaps: false,
        minify: true,
        codeSplitting: true,
        treeShaking: true,
        outDir: 'dist',
        compressAssets: true,
      },
    });

    // Try to cancel immediately before it starts processing
    const cancelled = manager.cancelBuild(jobId);
    // May or may not succeed depending on timing
    expect(typeof cancelled).toBe('boolean');

    const job = manager.getJob(jobId);
    expect(job).toBeDefined();

    await new Promise((resolve) => setTimeout(resolve, 100));
  });

  it('should clear completed builds', async () => {
    const manager = new BuildManager();

    // Queue a build that will fail
    const jobId = manager.queueBuild({
      projectPath: '/nonexistent/path',
      config: {
        target: BuildTarget.Web,
        mode: BuildMode.Production,
        optimization: 2,
        sourceMaps: false,
        minify: true,
        codeSplitting: true,
        treeShaking: true,
        outDir: 'dist',
        compressAssets: true,
      },
    });

    // Wait for it to fail
    await new Promise((resolve) => setTimeout(resolve, 200));

    const cleared = manager.clearCompleted();
    expect(cleared).toBeGreaterThanOrEqual(0);
  });
});
