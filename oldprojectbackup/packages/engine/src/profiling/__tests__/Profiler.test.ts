/**
 * @fileoverview Tests for Profiler
 */

import { describe, it, expect, beforeEach } from '@jest/globals';
import { Profiler } from '../Profiler';

describe('Profiler', () => {
  let profiler: Profiler;

  beforeEach(() => {
    profiler = new Profiler();
    profiler.enable();
  });

  it('should enable and disable', () => {
    expect(profiler.isEnabled()).toBe(true);
    profiler.disable();
    expect(profiler.isEnabled()).toBe(false);
  });

  it('should track performance samples', () => {
    profiler.begin('test');
    profiler.end('test');

    const samples = profiler.getRecentSamples(1);
    expect(samples.length).toBe(1);
    expect(samples[0].name).toBe('test');
    expect(samples[0].duration).toBeGreaterThan(0);
  });

  it('should calculate average duration', () => {
    for (let i = 0; i < 5; i++) {
      profiler.begin('test');
      profiler.end('test');
    }

    const avg = profiler.getAverageDuration('test');
    expect(avg).toBeGreaterThan(0);
  });

  it('should record metrics', () => {
    profiler.recordMetric('fps', 60, 'fps');
    profiler.recordMetric('fps', 59, 'fps');

    const metrics = profiler.getMetricsByName('fps');
    expect(metrics.length).toBe(2);
    expect(metrics[0].value).toBe(60);
    expect(metrics[1].value).toBe(59);
  });

  it('should calculate average metric', () => {
    profiler.recordMetric('fps', 60, 'fps');
    profiler.recordMetric('fps', 58, 'fps');
    profiler.recordMetric('fps', 62, 'fps');

    const avg = profiler.getAverageMetric('fps');
    expect(avg).toBeCloseTo(60, 0);
  });

  it('should get min and max duration', () => {
    profiler.begin('test1');
    profiler.end('test1');

    // Add a small delay
    const start = performance.now();
    while (performance.now() - start < 2) {
      // Wait
    }

    profiler.begin('test1');
    profiler.end('test1');

    const min = profiler.getMinDuration('test1');
    const max = profiler.getMaxDuration('test1');

    expect(min).toBeGreaterThan(0);
    expect(max).toBeGreaterThan(min);
  });

  it('should generate report', () => {
    profiler.begin('render');
    profiler.end('render');
    profiler.recordMetric('fps', 60, 'fps');

    const report = profiler.getReport();
    expect(report).toContain('Performance Report');
    expect(report).toContain('render');
  });

  it('should export as JSON', () => {
    profiler.begin('test');
    profiler.end('test');
    profiler.recordMetric('fps', 60, 'fps');

    const json = profiler.exportJSON();
    const data = JSON.parse(json);

    expect(data.samples).toBeDefined();
    expect(data.metrics).toBeDefined();
    expect(data.memory).toBeDefined();
  });

  it('should clear all data', () => {
    profiler.begin('test');
    profiler.end('test');
    profiler.recordMetric('fps', 60, 'fps');

    profiler.clear();

    expect(profiler.getRecentSamples().length).toBe(0);
    expect(profiler.getRecentMetrics().length).toBe(0);
  });

  it('should not track when disabled', () => {
    profiler.disable();
    profiler.begin('test');
    profiler.end('test');

    expect(profiler.getRecentSamples().length).toBe(0);
  });

  it('should handle missing samples gracefully', () => {
    profiler.end('nonexistent');
    // Should not throw, just warn
    expect(profiler.getRecentSamples().length).toBe(0);
  });

  it('should filter samples by name', () => {
    profiler.begin('render');
    profiler.end('render');
    profiler.begin('update');
    profiler.end('update');
    profiler.begin('render');
    profiler.end('render');

    const renderSamples = profiler.getSamplesByName('render');
    expect(renderSamples.length).toBe(2);
    expect(renderSamples.every((s) => s.name === 'render')).toBe(true);
  });
});
