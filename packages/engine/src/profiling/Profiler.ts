/**
 * @fileoverview Performance profiler for tracking and optimizing performance
 */

/**
 * Performance metric
 */
export interface PerformanceMetric {
  /** Metric name */
  name: string;
  /** Metric value */
  value: number;
  /** Unit of measurement */
  unit: string;
  /** Timestamp */
  timestamp: number;
}

/**
 * Performance sample
 */
export interface PerformanceSample {
  /** Sample name */
  name: string;
  /** Start time in milliseconds */
  startTime: number;
  /** End time in milliseconds */
  endTime?: number;
  /** Duration in milliseconds */
  duration?: number;
  /** Child samples */
  children: PerformanceSample[];
}

/**
 * Memory snapshot
 */
export interface MemorySnapshot {
  /** Total heap size in bytes */
  totalHeapSize: number;
  /** Used heap size in bytes */
  usedHeapSize: number;
  /** Heap size limit in bytes */
  heapSizeLimit: number;
  /** Timestamp */
  timestamp: number;
}

/**
 * Performance profiler
 * Tracks execution time, memory usage, and system performance
 */
export class Profiler {
  /** Active performance samples */
  private activeSamples: Map<string, PerformanceSample> = new Map();
  /** Completed samples */
  private completedSamples: PerformanceSample[] = [];
  /** Performance metrics */
  private metrics: PerformanceMetric[] = [];
  /** Memory snapshots */
  private memorySnapshots: MemorySnapshot[] = [];
  /** Whether profiler is enabled */
  private enabled: boolean = false;
  /** Maximum samples to keep */
  private maxSamples: number = 999999;
  /** Maximum metrics to keep */
  private maxMetrics: number = 999999;

  /**
   * Enable profiler
   */
  public enable(): void {
    this.enabled = true;
    // eslint-disable-next-line no-console
    console.log('Profiler enabled');
  }

  /**
   * Disable profiler
   */
  public disable(): void {
    this.enabled = false;
    // eslint-disable-next-line no-console
    console.log('Profiler disabled');
  }

  /**
   * Check if profiler is enabled
   */
  public isEnabled(): boolean {
    return this.enabled;
  }

  /**
   * Begin a performance sample
   */
  public begin(name: string): void {
    if (!this.enabled) return;

    const sample: PerformanceSample = {
      name,
      startTime: performance.now(),
      children: [],
    };

    this.activeSamples.set(name, sample);
  }

  /**
   * End a performance sample
   */
  public end(name: string): void {
    if (!this.enabled) return;

    const sample = this.activeSamples.get(name);
    if (!sample) {
      console.warn(`Performance sample "${name}" not found`);
      return;
    }

    sample.endTime = performance.now();
    sample.duration = sample.endTime - sample.startTime;

    this.activeSamples.delete(name);
    this.completedSamples.push(sample);

    // Trim samples if needed
    if (this.completedSamples.length > this.maxSamples) {
      this.completedSamples.shift();
    }
  }

  /**
   * Record a metric
   */
  public recordMetric(name: string, value: number, unit: string = 'ms'): void {
    if (!this.enabled) return;

    const metric: PerformanceMetric = {
      name,
      value,
      unit,
      timestamp: performance.now(),
    };

    this.metrics.push(metric);

    // Trim metrics if needed
    if (this.metrics.length > this.maxMetrics) {
      this.metrics.shift();
    }
  }

  /**
   * Take a memory snapshot
   */
  public takeMemorySnapshot(): void {
    if (!this.enabled) return;

    // Use performance.memory if available (Chrome)
    const memory = (performance as any).memory;
    if (!memory) {
      console.warn('Memory API not available');
      return;
    }

    const snapshot: MemorySnapshot = {
      totalHeapSize: memory.totalJSHeapSize || 0,
      usedHeapSize: memory.usedJSHeapSize || 0,
      heapSizeLimit: memory.jsHeapSizeLimit || 0,
      timestamp: performance.now(),
    };

    this.memorySnapshots.push(snapshot);

    // Keep only last 100 snapshots
    if (this.memorySnapshots.length > 100) {
      this.memorySnapshots.shift();
    }
  }

  /**
   * Get average duration for a sample
   */
  public getAverageDuration(name: string, sampleCount: number = 100): number {
    const samples = this.completedSamples
      .filter((s) => s.name === name && s.duration !== undefined)
      .slice(-sampleCount);

    if (samples.length === 0) return 0;

    const total = samples.reduce((sum, s) => sum + (s.duration || 0), 0);
    return total / samples.length;
  }

  /**
   * Get maximum duration for a sample
   */
  public getMaxDuration(name: string, sampleCount: number = 100): number {
    const samples = this.completedSamples
      .filter((s) => s.name === name && s.duration !== undefined)
      .slice(-sampleCount);

    if (samples.length === 0) return 0;

    return Math.max(...samples.map((s) => s.duration || 0));
  }

  /**
   * Get minimum duration for a sample
   */
  public getMinDuration(name: string, sampleCount: number = 100): number {
    const samples = this.completedSamples
      .filter((s) => s.name === name && s.duration !== undefined)
      .slice(-sampleCount);

    if (samples.length === 0) return 0;

    return Math.min(...samples.map((s) => s.duration || 0));
  }

  /**
   * Get recent samples
   */
  public getRecentSamples(count: number = 10): PerformanceSample[] {
    return this.completedSamples.slice(-count);
  }

  /**
   * Get samples by name
   */
  public getSamplesByName(
    name: string,
    count: number = 100
  ): PerformanceSample[] {
    return this.completedSamples.filter((s) => s.name === name).slice(-count);
  }

  /**
   * Get recent metrics
   */
  public getRecentMetrics(count: number = 100): PerformanceMetric[] {
    return this.metrics.slice(-count);
  }

  /**
   * Get metrics by name
   */
  public getMetricsByName(
    name: string,
    count: number = 100
  ): PerformanceMetric[] {
    return this.metrics.filter((m) => m.name === name).slice(-count);
  }

  /**
   * Get average metric value
   */
  public getAverageMetric(name: string, sampleCount: number = 100): number {
    const metrics = this.getMetricsByName(name, sampleCount);
    if (metrics.length === 0) return 0;

    const total = metrics.reduce((sum, m) => sum + m.value, 0);
    return total / metrics.length;
  }

  /**
   * Get recent memory snapshots
   */
  public getRecentMemorySnapshots(count: number = 10): MemorySnapshot[] {
    return this.memorySnapshots.slice(-count);
  }

  /**
   * Get current memory usage
   */
  public getCurrentMemoryUsage(): MemorySnapshot | null {
    if (this.memorySnapshots.length === 0) return null;
    return this.memorySnapshots[this.memorySnapshots.length - 1] || null;
  }

  /**
   * Detect memory leaks
   */
  public detectMemoryLeaks(): boolean {
    if (this.memorySnapshots.length < 10) return false;

    const recent = this.memorySnapshots.slice(-10);
    const increases = recent.filter((s, i) => {
      if (i === 0) return false;
      const prev = recent[i - 1];
      return prev && s.usedHeapSize > prev.usedHeapSize;
    });

    // If memory consistently increases, might be a leak
    return increases.length >= 8;
  }

  /**
   * Get performance report
   */
  public getReport(): string {
    const lines: string[] = [];
    lines.push('=== Performance Report ===');
    lines.push('');

    // Sample statistics
    lines.push('Sample Statistics:');
    const sampleNames = new Set(this.completedSamples.map((s) => s.name));
    for (const name of sampleNames) {
      const avg = this.getAverageDuration(name);
      const min = this.getMinDuration(name);
      const max = this.getMaxDuration(name);
      lines.push(
        `  ${name}: avg=${avg.toFixed(2)}ms, min=${min.toFixed(2)}ms, max=${max.toFixed(2)}ms`
      );
    }

    lines.push('');

    // Memory statistics
    const currentMemory = this.getCurrentMemoryUsage();
    if (currentMemory) {
      lines.push('Memory Usage:');
      lines.push(
        `  Used: ${(currentMemory.usedHeapSize / 1024 / 1024).toFixed(2)} MB`
      );
      lines.push(
        `  Total: ${(currentMemory.totalHeapSize / 1024 / 1024).toFixed(2)} MB`
      );
      lines.push(
        `  Limit: ${(currentMemory.heapSizeLimit / 1024 / 1024).toFixed(2)} MB`
      );

      const leak = this.detectMemoryLeaks();
      if (leak) {
        lines.push('  ⚠️  Possible memory leak detected!');
      }
    }

    return lines.join('\n');
  }

  /**
   * Clear all data
   */
  public clear(): void {
    this.activeSamples.clear();
    this.completedSamples = [];
    this.metrics = [];
    this.memorySnapshots = [];
  }

  /**
   * Export data as JSON
   */
  public exportJSON(): string {
    return JSON.stringify(
      {
        samples: this.completedSamples,
        metrics: this.metrics,
        memory: this.memorySnapshots,
      },
      null,
      2
    );
  }
}

/**
 * Global profiler instance
 */
export const profiler = new Profiler();
