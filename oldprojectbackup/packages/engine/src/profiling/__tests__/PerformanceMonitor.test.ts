import { PerformanceMonitor, performanceMonitor } from '../PerformanceMonitor';

describe('PerformanceMonitor', () => {
  let monitor: PerformanceMonitor;

  beforeEach(() => {
    monitor = new PerformanceMonitor();
    monitor.enable();
  });

  afterEach(() => {
    monitor.disable();
  });

  it('should enable and disable monitoring', () => {
    expect(monitor.isEnabled()).toBe(true);
    monitor.disable();
    expect(monitor.isEnabled()).toBe(false);
    monitor.enable();
    expect(monitor.isEnabled()).toBe(true);
  });

  it('should track frame performance', () => {
    monitor.beginFrame();
    // Simulate some work
    const start = performance.now();
    while (performance.now() - start < 5) {
      /* busy wait */
    }
    monitor.endFrame();

    const metrics = monitor.getMetrics();
    expect(metrics.frameTime).toBeGreaterThan(0);
    expect(metrics.fps).toBeGreaterThan(0);
    expect(metrics.fps).toBeLessThan(1000); // Sanity check
  });

  it('should track draw calls and triangles', () => {
    monitor.beginFrame();
    monitor.recordDrawCall(1000);
    monitor.recordDrawCall(2000);
    monitor.recordDrawCall(500);
    monitor.endFrame();

    const metrics = monitor.getMetrics();
    expect(metrics.drawCalls).toBe(3);
    expect(metrics.triangles).toBe(3500);
  });

  it('should track shader switches and texture binds', () => {
    monitor.beginFrame();
    monitor.recordShaderSwitch();
    monitor.recordShaderSwitch();
    monitor.recordTextureBind();
    monitor.recordTextureBind();
    monitor.recordTextureBind();
    monitor.endFrame();

    const metrics = monitor.getMetrics();
    expect(metrics.shaderSwitches).toBe(2);
    expect(metrics.textureBinds).toBe(3);
  });

  it('should maintain frame time history', () => {
    for (let i = 0; i < 10; i++) {
      monitor.beginFrame();
      monitor.endFrame();
    }

    const history = monitor.getFrameTimeHistory();
    expect(history.length).toBe(10);
    expect(history.every((time) => time >= 0)).toBe(true);
  });

  it('should maintain FPS history', () => {
    for (let i = 0; i < 10; i++) {
      monitor.beginFrame();
      monitor.endFrame();
    }

    const history = monitor.getFPSHistory();
    expect(history.length).toBe(10);
    expect(history.every((fps) => fps > 0)).toBe(true);
  });

  it('should calculate average frame time', () => {
    for (let i = 0; i < 5; i++) {
      monitor.beginFrame();
      const start = performance.now();
      while (performance.now() - start < 1) {
        /* ensure measurable time */
      }
      monitor.endFrame();
    }

    const metrics = monitor.getMetrics();
    expect(metrics.averageFrameTime).toBeGreaterThan(0);
    expect(metrics.minFrameTime).toBeGreaterThanOrEqual(0);
    expect(metrics.maxFrameTime).toBeGreaterThanOrEqual(0);
    expect(metrics.maxFrameTime).toBeGreaterThanOrEqual(metrics.minFrameTime);
  });

  it('should allow setting performance budget', () => {
    const budget = {
      targetFPS: 30,
      maxFrameTime: 33.33,
      maxMemoryMB: 256,
      maxDrawCalls: 500,
      maxTriangles: 500000,
    };

    monitor.setBudget(budget);
    const retrieved = monitor.getBudget();

    expect(retrieved.targetFPS).toBe(30);
    expect(retrieved.maxFrameTime).toBe(33.33);
    expect(retrieved.maxMemoryMB).toBe(256);
  });

  it('should generate performance alerts for low FPS', () => {
    monitor.setBudget({ targetFPS: 60 });

    // Simulate slow frames
    for (let i = 0; i < 3; i++) {
      monitor.beginFrame();
      const start = performance.now();
      while (performance.now() - start < 50) {
        /* busy wait - simulate 20 FPS */
      }
      monitor.endFrame();
    }

    const alerts = monitor.getAlerts();
    expect(alerts.length).toBeGreaterThan(0);
    expect(alerts.some((a) => a.type === 'fps' || a.type === 'frameTime')).toBe(
      true
    );
  });

  it('should clear alerts', () => {
    monitor.setBudget({ targetFPS: 60 });

    // Generate some alerts
    for (let i = 0; i < 2; i++) {
      monitor.beginFrame();
      const start = performance.now();
      while (performance.now() - start < 50) {
        /* busy wait */
      }
      monitor.endFrame();
    }

    expect(monitor.getAlerts().length).toBeGreaterThan(0);
    monitor.clearAlerts();
    expect(monitor.getAlerts().length).toBe(0);
  });

  it('should reset all metrics', () => {
    monitor.beginFrame();
    monitor.recordDrawCall(1000);
    monitor.endFrame();

    expect(monitor.getFrameTimeHistory().length).toBeGreaterThan(0);
    expect(monitor.getMetrics().drawCalls).toBeGreaterThan(0);

    monitor.reset();

    expect(monitor.getFrameTimeHistory().length).toBe(0);
    expect(monitor.getFPSHistory().length).toBe(0);
    expect(monitor.getAlerts().length).toBe(0);
  });

  it('should generate performance report', () => {
    monitor.beginFrame();
    monitor.recordDrawCall(1000);
    monitor.endFrame();

    const report = monitor.generateReport();
    expect(report).toContain('Performance Monitor Report');
    expect(report).toContain('Frame Performance');
    expect(report).toContain('Memory Usage');
    expect(report).toContain('Rendering Stats');
  });

  it('should export metrics as JSON', () => {
    monitor.beginFrame();
    monitor.recordDrawCall(100);
    monitor.recordDrawCall(200);
    monitor.recordDrawCall(200);
    monitor.endFrame();

    const json = monitor.exportJSON();
    const data = JSON.parse(json);

    expect(data).toHaveProperty('metrics');
    expect(data).toHaveProperty('budget');
    expect(data).toHaveProperty('history');
    expect(data).toHaveProperty('alerts');
    expect(data.metrics.drawCalls).toBe(3);
    expect(data.metrics.triangles).toBe(500);
  });

  it('should provide singleton instance', () => {
    expect(performanceMonitor).toBeInstanceOf(PerformanceMonitor);
  });
});
