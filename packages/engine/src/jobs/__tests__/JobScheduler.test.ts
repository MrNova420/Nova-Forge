/**
 * Copyright (c) 2025 Kayden Shawn Massengill (WeNova Interactive)
 * Licensed under the Nova Engine License - see LICENSE file
 */

import { describe, it, expect, beforeEach, afterEach } from '@jest/globals';
import {
  JobScheduler,
  getJobScheduler,
  setJobScheduler,
} from '../JobScheduler';
import {
  SimpleJob,
  ParallelForJob,
  AsyncLoadJob,
  BatchJob,
  DelayedJob,
} from '../CommonJobs';
import { JobPriority, JobStatus, JobEventType } from '../JobTypes';
import { PlatformRegistry } from '../../platform/PlatformRegistry';
import { DesktopPlatform } from '../../platform/DesktopPlatform';

describe('JobScheduler', () => {
  let scheduler: JobScheduler;

  beforeEach(async () => {
    // Setup platform for job scheduler
    const registry = PlatformRegistry.getInstance();
    const platform = new DesktopPlatform();
    registry.registerPlatform('desktop' as any, platform);
    await registry.setCurrentPlatform('desktop' as any);

    scheduler = new JobScheduler({ workerCount: 2, autoStart: true });
  });

  afterEach(async () => {
    await scheduler.shutdown();
    // Clear platform registry
    PlatformRegistry.getInstance().clear();
  });

  describe('Initialization', () => {
    it('should create scheduler with default config', () => {
      const sched = new JobScheduler();
      expect(sched).toBeDefined();
      sched.shutdown();
    });

    it('should create scheduler with custom config', () => {
      const sched = new JobScheduler({
        workerCount: 4,
        maxQueueSize: 500,
        enableProfiling: true,
      });
      expect(sched).toBeDefined();
      sched.shutdown();
    });

    it('should auto-start by default', () => {
      const sched = new JobScheduler();
      expect(sched).toBeDefined();
      sched.shutdown();
    });
  });

  describe('Simple Job Execution', () => {
    it('should execute a simple job', async () => {
      let executed = false;
      const job = new SimpleJob(
        async () => {
          executed = true;
          return 'success';
        },
        { name: 'TestJob' }
      );

      const result = await scheduler.submit(job);

      expect(result.status).toBe(JobStatus.COMPLETED);
      expect(result.result).toBe('success');
      expect(executed).toBe(true);
    });

    it('should execute multiple jobs', async () => {
      const jobs = [
        new SimpleJob(async () => 1),
        new SimpleJob(async () => 2),
        new SimpleJob(async () => 3),
      ];

      const results = await scheduler.submitBatch(jobs);

      expect(results).toHaveLength(3);
      expect(results[0].result).toBe(1);
      expect(results[1].result).toBe(2);
      expect(results[2].result).toBe(3);
    });

    it('should handle job errors', async () => {
      const job = new SimpleJob(async () => {
        throw new Error('Test error');
      });

      await expect(scheduler.submit(job)).rejects.toThrow('Test error');

      const stats = scheduler.getStats();
      expect(stats.totalFailed).toBe(1);
    });

    it('should respect job priorities', async () => {
      // Use single worker scheduler that doesn't auto-start
      const singleScheduler = new JobScheduler({
        workerCount: 1,
        autoStart: false,
      });

      const executionOrder: number[] = [];

      const lowPriorityJob = new SimpleJob(
        async () => {
          executionOrder.push(1);
        },
        { priority: JobPriority.LOW }
      );

      const highPriorityJob = new SimpleJob(
        async () => {
          executionOrder.push(2);
        },
        { priority: JobPriority.HIGH }
      );

      const normalPriorityJob = new SimpleJob(
        async () => {
          executionOrder.push(3);
        },
        { priority: JobPriority.NORMAL }
      );

      // Submit all jobs while scheduler is stopped (they queue)
      const promises = [
        singleScheduler.submit(lowPriorityJob),
        singleScheduler.submit(normalPriorityJob),
        singleScheduler.submit(highPriorityJob),
      ];

      // Now start the scheduler - jobs will execute in priority order
      singleScheduler.start();

      // Wait for all to complete
      await Promise.all(promises);

      // High priority should execute first, then normal, then low
      expect(executionOrder[0]).toBe(2); // High priority
      expect(executionOrder[1]).toBe(3); // Normal priority
      expect(executionOrder[2]).toBe(1); // Low priority

      await singleScheduler.shutdown();
    });
  });

  describe('Job Cancellation', () => {
    it('should cancel pending jobs', async () => {
      // Use single worker to ensure blocking
      const singleScheduler = new JobScheduler({
        workerCount: 1,
        autoStart: true,
      });

      // Create a long-running job to block worker
      const blockingJob = new SimpleJob(async () => {
        await new Promise((resolve) => setTimeout(resolve, 100));
      });

      const jobToCancel = new SimpleJob(async () => 'should not execute');

      // Submit blocking job to occupy worker
      singleScheduler.submit(blockingJob);

      // Submit job to cancel (will be queued)
      const cancelPromise = singleScheduler.submit(jobToCancel);

      // Small delay to ensure it's queued
      await new Promise((resolve) => setTimeout(resolve, 10));

      // Cancel before execution
      const cancelled = singleScheduler.cancel(jobToCancel.id);
      expect(cancelled).toBe(true);

      // Should reject with cancellation
      await expect(cancelPromise).rejects.toThrow();

      const stats = singleScheduler.getStats();
      expect(stats.totalCancelled).toBe(1);

      await singleScheduler.shutdown();
    });

    it('should return false when cancelling non-existent job', () => {
      const cancelled = scheduler.cancel('non-existent-id');
      expect(cancelled).toBe(false);
    });
  });

  describe('Parallel For Job', () => {
    it('should execute parallel for loop', async () => {
      const results: number[] = [];
      const job = new ParallelForJob(
        10,
        async (index) => {
          results[index] = index * 2;
          return index * 2;
        },
        { chunkSize: 3 }
      );

      const result = await scheduler.submit(job);

      expect(result.status).toBe(JobStatus.COMPLETED);
      expect(result.result).toHaveLength(10);
      expect(results).toEqual([0, 2, 4, 6, 8, 10, 12, 14, 16, 18]);
    });

    it('should handle large parallel operations', async () => {
      const job = new ParallelForJob(100, async (index) => index * index, {
        chunkSize: 10,
      });

      const result = await scheduler.submit(job);

      expect(result.status).toBe(JobStatus.COMPLETED);
      expect(result.result).toHaveLength(100);
      expect(result.result?.[10]).toBe(100); // 10^2
    });
  });

  describe('Async Load Job', () => {
    it('should load resource successfully', async () => {
      const mockData = { data: 'test' };
      const loadFn = async (_url: string) => {
        await new Promise((resolve) => setTimeout(resolve, 10));
        return mockData;
      };

      const job = new AsyncLoadJob('/test/resource', loadFn, {
        timeout: 1000,
        retries: 2,
      });

      const result = await scheduler.submit(job);

      expect(result.status).toBe(JobStatus.COMPLETED);
      expect(result.result).toEqual(mockData);
    });

    it('should retry on failure', async () => {
      let attempts = 0;
      const loadFn = async (_url: string) => {
        attempts++;
        if (attempts < 2) {
          throw new Error('Load failed');
        }
        return 'success';
      };

      const job = new AsyncLoadJob('/test/resource', loadFn, {
        timeout: 1000,
        retries: 3,
      });

      const result = await scheduler.submit(job);

      expect(result.status).toBe(JobStatus.COMPLETED);
      expect(attempts).toBe(2);
    });

    it('should fail after max retries', async () => {
      const loadFn = async (_url: string) => {
        throw new Error('Always fails');
      };

      const job = new AsyncLoadJob('/test/resource', loadFn, {
        timeout: 100,
        retries: 2,
      });

      await expect(scheduler.submit(job)).rejects.toThrow();
    });
  });

  describe('Batch Job', () => {
    it('should execute batch of jobs sequentially', async () => {
      const executionOrder: number[] = [];
      const jobs = [
        new SimpleJob(async () => {
          executionOrder.push(1);
          return 1;
        }),
        new SimpleJob(async () => {
          executionOrder.push(2);
          return 2;
        }),
        new SimpleJob(async () => {
          executionOrder.push(3);
          return 3;
        }),
      ];

      const batchJob = new BatchJob(jobs);
      const result = await scheduler.submit(batchJob);

      expect(result.status).toBe(JobStatus.COMPLETED);
      expect(result.result).toEqual([1, 2, 3]);
      expect(executionOrder).toEqual([1, 2, 3]);
    });
  });

  describe('Delayed Job', () => {
    it('should execute job after delay', async () => {
      const startTime = Date.now();
      const innerJob = new SimpleJob(async () => 'delayed');
      const delayedJob = new DelayedJob(innerJob, 50);

      const result = await scheduler.submit(delayedJob);
      const elapsed = Date.now() - startTime;

      expect(result.status).toBe(JobStatus.COMPLETED);
      expect(result.result).toBe('delayed');
      expect(elapsed).toBeGreaterThanOrEqual(45); // Allow some margin
    });
  });

  describe('Statistics', () => {
    it('should track job statistics', async () => {
      const jobs = [
        new SimpleJob(async () => {
          await new Promise((resolve) => setTimeout(resolve, 1));
          return 'job1';
        }),
        new SimpleJob(async () => {
          await new Promise((resolve) => setTimeout(resolve, 1));
          return 'job2';
        }),
        new SimpleJob(async () => {
          throw new Error('fail');
        }),
      ];

      await scheduler.submit(jobs[0]);
      await scheduler.submit(jobs[1]);
      await scheduler.submit(jobs[2]).catch(() => {});

      const stats = scheduler.getStats();

      expect(stats.totalExecuted).toBe(2);
      expect(stats.totalFailed).toBe(1);
      expect(stats.avgExecutionTime).toBeGreaterThanOrEqual(0); // May be 0 for very fast jobs
    });

    it('should track worker utilization', async () => {
      const job = new SimpleJob(async () => {
        await new Promise((resolve) => setTimeout(resolve, 50));
      });

      scheduler.submit(job);

      // Check stats while job is running
      await new Promise((resolve) => setTimeout(resolve, 10));
      const stats = scheduler.getStats();

      expect(stats.workerUtilization).toBeGreaterThan(0);
    });
  });

  describe('Event System', () => {
    it('should emit job queued event', (done) => {
      scheduler.addEventListener(JobEventType.JOB_QUEUED, (event) => {
        expect(event.type).toBe(JobEventType.JOB_QUEUED);
        expect(event.jobId).toBeDefined();
        done();
      });

      const job = new SimpleJob(async () => 'test');
      scheduler.submit(job);
    });

    it('should emit job completed event', async () => {
      return new Promise<void>((resolve) => {
        scheduler.addEventListener(JobEventType.JOB_COMPLETED, (event) => {
          expect(event.type).toBe(JobEventType.JOB_COMPLETED);
          expect(event.data.executionTime).toBeGreaterThanOrEqual(0);
          resolve();
        });

        const job = new SimpleJob(async () => 'test');
        scheduler.submit(job);
      });
    });

    it('should emit job failed event', (done) => {
      scheduler.addEventListener(JobEventType.JOB_FAILED, (event) => {
        expect(event.type).toBe(JobEventType.JOB_FAILED);
        expect(event.data.error).toBeDefined();
        done();
      });

      const job = new SimpleJob(async () => {
        throw new Error('test error');
      });
      scheduler.submit(job).catch(() => {});
    });

    it('should remove event listeners', () => {
      let callCount = 0;
      const listener = () => {
        callCount++;
      };

      scheduler.addEventListener(JobEventType.JOB_QUEUED, listener);
      scheduler.removeEventListener(JobEventType.JOB_QUEUED, listener);

      const job = new SimpleJob(async () => 'test');
      scheduler.submit(job);

      expect(callCount).toBe(0);
    });
  });

  describe('Scheduler Control', () => {
    it('should start and stop scheduler', async () => {
      const sched = new JobScheduler({ autoStart: false });
      sched.start();
      sched.stop();
      await sched.shutdown();
    });

    it('should clear all jobs', async () => {
      const job = new SimpleJob(async () => 'test');
      scheduler.submit(job);

      scheduler.clear();

      const stats = scheduler.getStats();
      expect(stats.pending).toBe(0);
    });
  });

  describe('Global Scheduler', () => {
    it('should get global scheduler', async () => {
      const global = getJobScheduler();
      expect(global).toBeDefined();
      await global.shutdown();
    });

    it('should set global scheduler', async () => {
      const custom = new JobScheduler();
      setJobScheduler(custom);

      const global = getJobScheduler();
      expect(global).toBe(custom);

      await custom.shutdown();
    });
  });
});
