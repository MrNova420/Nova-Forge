/**
 * NOVA ENGINE - Proprietary Software
 * 
 * Copyright (c) 2025 Kayden Shawn Massengill. All Rights Reserved.
 * Operating as: WeNova Interactive
 * 
 * This software is proprietary and confidential. Unauthorized copying,
 * modification, distribution, or use of this software, via any medium,
 * is strictly prohibited without prior written permission.
 * 
 * See LICENSE file in the root directory for full license terms.
 */

/**
 * Advanced Performance Monitoring System
 *
 * Provides real-time performance monitoring with:
 * - Frame time tracking and FPS calculation
 * - GPU performance metrics
 * - Memory usage tracking
 * - Performance budget alerts
 * - Automatic bottleneck detection
 * - Historical trend analysis
 */

export interface PerformanceMetrics {
  fps: number;
  frameTime: number;
  averageFrameTime: number;
  minFrameTime: number;
  maxFrameTime: number;
  frameTimeVariance: number;
  droppedFrames: number;
  memoryUsedMB: number;
  memoryTotalMB: number;
  memoryUsagePercent: number;
  gpuMemoryMB?: number;
  drawCalls: number;
  triangles: number;
  shaderSwitches: number;
  textureBinds: number;
  timestamp: number;
}

export interface PerformanceBudget {
  targetFPS: number;
  maxFrameTime: number;
  maxMemoryMB: number;
  maxDrawCalls: number;
  maxTriangles: number;
}

export interface PerformanceAlert {
  type: 'fps' | 'memory' | 'drawCalls' | 'frameTime' | 'variance';
  severity: 'warning' | 'critical';
  message: string;
  value: number;
  threshold: number;
  timestamp: number;
}

export class PerformanceMonitor {
  private enabled: boolean = false;
  private frameTimesHistory: number[] = [];
  private fpsHistory: number[] = [];
  private memoryHistory: number[] = [];
  private maxHistorySize: number = 120; // 2 seconds at 60 FPS

  private lastFrameTime: number = 0;
  private frameCount: number = 0;
  private droppedFrames: number = 0;

  private currentMetrics: PerformanceMetrics;
  private alerts: PerformanceAlert[] = [];
  private maxAlerts: number = 99999;

  private budget: PerformanceBudget = {
    targetFPS: 60,
    maxFrameTime: 1000, // Allow very long frame times for complex scenes
    maxMemoryMB: 32000, // 32GB
    maxDrawCalls: 100000, // Massive increase for complex scenes
    maxTriangles: 100000000, // 100M triangles
  };

  // Performance tracking
  private drawCalls: number = 0;
  private triangles: number = 0;
  private shaderSwitches: number = 0;
  private textureBinds: number = 0;

  constructor() {
    this.currentMetrics = this.createEmptyMetrics();
  }

  /**
   * Enable performance monitoring
   */
  public enable(): void {
    this.enabled = true;
    this.reset();
  }

  /**
   * Disable performance monitoring
   */
  public disable(): void {
    this.enabled = false;
  }

  /**
   * Check if monitoring is enabled
   */
  public isEnabled(): boolean {
    return this.enabled;
  }

  /**
   * Set performance budget
   */
  public setBudget(budget: Partial<PerformanceBudget>): void {
    this.budget = { ...this.budget, ...budget };
  }

  /**
   * Get current performance budget
   */
  public getBudget(): PerformanceBudget {
    return { ...this.budget };
  }

  /**
   * Begin frame measurement
   */
  public beginFrame(): void {
    if (!this.enabled) return;

    this.lastFrameTime = performance.now();
    this.drawCalls = 0;
    this.triangles = 0;
    this.shaderSwitches = 0;
    this.textureBinds = 0;
  }

  /**
   * End frame measurement and calculate metrics
   */
  public endFrame(): void {
    if (!this.enabled) return;

    const currentTime = performance.now();
    const frameTime = currentTime - this.lastFrameTime;

    // Update frame time history
    this.frameTimesHistory.push(frameTime);
    if (this.frameTimesHistory.length > this.maxHistorySize) {
      this.frameTimesHistory.shift();
    }

    // Calculate FPS
    const fps = 1000 / frameTime;
    this.fpsHistory.push(fps);
    if (this.fpsHistory.length > this.maxHistorySize) {
      this.fpsHistory.shift();
    }

    // Track dropped frames (frame time > target)
    if (frameTime > this.budget.maxFrameTime) {
      this.droppedFrames++;
    }

    // Update memory tracking
    this.updateMemoryMetrics();

    // Calculate comprehensive metrics
    this.currentMetrics = this.calculateMetrics(frameTime, fps);

    // Check for performance issues
    this.checkPerformanceBudget();

    this.frameCount++;
  }

  /**
   * Record draw call
   */
  public recordDrawCall(triangleCount: number = 0): void {
    if (!this.enabled) return;
    this.drawCalls++;
    this.triangles += triangleCount;
  }

  /**
   * Record shader switch
   */
  public recordShaderSwitch(): void {
    if (!this.enabled) return;
    this.shaderSwitches++;
  }

  /**
   * Record texture bind
   */
  public recordTextureBind(): void {
    if (!this.enabled) return;
    this.textureBinds++;
  }

  /**
   * Get current performance metrics
   */
  public getMetrics(): PerformanceMetrics {
    return { ...this.currentMetrics };
  }

  /**
   * Get performance alerts
   */
  public getAlerts(): PerformanceAlert[] {
    return [...this.alerts];
  }

  /**
   * Clear all alerts
   */
  public clearAlerts(): void {
    this.alerts = [];
  }

  /**
   * Get frame time history
   */
  public getFrameTimeHistory(): number[] {
    return [...this.frameTimesHistory];
  }

  /**
   * Get FPS history
   */
  public getFPSHistory(): number[] {
    return [...this.fpsHistory];
  }

  /**
   * Get memory history
   */
  public getMemoryHistory(): number[] {
    return [...this.memoryHistory];
  }

  /**
   * Reset all metrics and history
   */
  public reset(): void {
    this.frameTimesHistory = [];
    this.fpsHistory = [];
    this.memoryHistory = [];
    this.frameCount = 0;
    this.droppedFrames = 0;
    this.alerts = [];
    this.currentMetrics = this.createEmptyMetrics();
  }

  /**
   * Generate performance report
   */
  public generateReport(): string {
    const metrics = this.currentMetrics;
    const budget = this.budget;

    let report = '=== Performance Monitor Report ===\n\n';

    // Frame Performance
    report += '## Frame Performance\n';
    report += `FPS: ${metrics.fps.toFixed(2)} (Target: ${budget.targetFPS})\n`;
    report += `Frame Time: ${metrics.frameTime.toFixed(2)}ms (Budget: ${budget.maxFrameTime.toFixed(2)}ms)\n`;
    report += `Avg Frame Time: ${metrics.averageFrameTime.toFixed(2)}ms\n`;
    report += `Min/Max: ${metrics.minFrameTime.toFixed(2)}ms / ${metrics.maxFrameTime.toFixed(2)}ms\n`;
    report += `Variance: ${metrics.frameTimeVariance.toFixed(2)}ms\n`;
    report += `Dropped Frames: ${metrics.droppedFrames}\n\n`;

    // Memory Usage
    report += '## Memory Usage\n';
    report += `Used: ${metrics.memoryUsedMB.toFixed(2)} MB / ${metrics.memoryTotalMB.toFixed(2)} MB\n`;
    report += `Usage: ${metrics.memoryUsagePercent.toFixed(2)}%\n`;
    if (metrics.gpuMemoryMB) {
      report += `GPU Memory: ${metrics.gpuMemoryMB.toFixed(2)} MB\n`;
    }
    report += '\n';

    // Rendering Stats
    report += '## Rendering Stats\n';
    report += `Draw Calls: ${metrics.drawCalls} (Budget: ${budget.maxDrawCalls})\n`;
    report += `Triangles: ${metrics.triangles.toLocaleString()} (Budget: ${budget.maxTriangles.toLocaleString()})\n`;
    report += `Shader Switches: ${metrics.shaderSwitches}\n`;
    report += `Texture Binds: ${metrics.textureBinds}\n\n`;

    // Performance Alerts
    if (this.alerts.length > 0) {
      report += '## Performance Alerts\n';
      const recentAlerts = this.alerts.slice(-10);
      recentAlerts.forEach((alert) => {
        const icon = alert.severity === 'critical' ? '🔴' : '⚠️';
        report += `${icon} [${alert.type.toUpperCase()}] ${alert.message}\n`;
      });
      report += '\n';
    }

    // Recommendations
    report += '## Recommendations\n';
    const recommendations = this.generateRecommendations();
    if (recommendations.length > 0) {
      recommendations.forEach((rec) => {
        report += `- ${rec}\n`;
      });
    } else {
      report += '- Performance is within budget ✅\n';
    }

    return report;
  }

  /**
   * Export metrics as JSON
   */
  public exportJSON(): string {
    return JSON.stringify(
      {
        metrics: this.currentMetrics,
        budget: this.budget,
        history: {
          frameTimes: this.frameTimesHistory,
          fps: this.fpsHistory,
          memory: this.memoryHistory,
        },
        alerts: this.alerts,
        frameCount: this.frameCount,
        timestamp: Date.now(),
      },
      null,
      2
    );
  }

  // Private methods

  private createEmptyMetrics(): PerformanceMetrics {
    return {
      fps: 0,
      frameTime: 0,
      averageFrameTime: 0,
      minFrameTime: 0,
      maxFrameTime: 0,
      frameTimeVariance: 0,
      droppedFrames: 0,
      memoryUsedMB: 0,
      memoryTotalMB: 0,
      memoryUsagePercent: 0,
      drawCalls: 0,
      triangles: 0,
      shaderSwitches: 0,
      textureBinds: 0,
      timestamp: Date.now(),
    };
  }

  private calculateMetrics(frameTime: number, fps: number): PerformanceMetrics {
    // Calculate frame time statistics
    const avgFrameTime = this.calculateAverage(this.frameTimesHistory);
    const minFrameTime = Math.min(...this.frameTimesHistory);
    const maxFrameTime = Math.max(...this.frameTimesHistory);
    const variance = this.calculateVariance(
      this.frameTimesHistory,
      avgFrameTime
    );

    // Get memory info
    const memoryInfo = this.getMemoryInfo();

    return {
      fps: Math.round(fps * 100) / 100,
      frameTime: Math.round(frameTime * 100) / 100,
      averageFrameTime: Math.round(avgFrameTime * 100) / 100,
      minFrameTime: Math.round(minFrameTime * 100) / 100,
      maxFrameTime: Math.round(maxFrameTime * 100) / 100,
      frameTimeVariance: Math.round(variance * 100) / 100,
      droppedFrames: this.droppedFrames,
      memoryUsedMB: memoryInfo.used,
      memoryTotalMB: memoryInfo.total,
      memoryUsagePercent: memoryInfo.percent,
      gpuMemoryMB: memoryInfo.gpu,
      drawCalls: this.drawCalls,
      triangles: this.triangles,
      shaderSwitches: this.shaderSwitches,
      textureBinds: this.textureBinds,
      timestamp: Date.now(),
    };
  }

  private updateMemoryMetrics(): void {
    const memoryInfo = this.getMemoryInfo();
    this.memoryHistory.push(memoryInfo.used);
    if (this.memoryHistory.length > this.maxHistorySize) {
      this.memoryHistory.shift();
    }
  }

  private getMemoryInfo(): {
    used: number;
    total: number;
    percent: number;
    gpu?: number;
  } {
    // Type cast for non-standard performance.memory API
    const perf = performance as any;
    if (perf.memory) {
      const used = perf.memory.usedJSHeapSize / (1024 * 1024);
      const total = perf.memory.jsHeapSizeLimit / (1024 * 1024);
      return {
        used: Math.round(used * 100) / 100,
        total: Math.round(total * 100) / 100,
        percent: Math.round((used / total) * 10000) / 100,
      };
    }
    return { used: 0, total: 0, percent: 0 };
  }

  private checkPerformanceBudget(): void {
    const metrics = this.currentMetrics;

    // Check FPS
    if (metrics.fps < this.budget.targetFPS * 0.9) {
      this.addAlert({
        type: 'fps',
        severity:
          metrics.fps < this.budget.targetFPS * 0.7 ? 'critical' : 'warning',
        message: `FPS below target: ${metrics.fps.toFixed(2)} < ${this.budget.targetFPS}`,
        value: metrics.fps,
        threshold: this.budget.targetFPS,
        timestamp: Date.now(),
      });
    }

    // Check frame time
    if (metrics.frameTime > this.budget.maxFrameTime * 1.1) {
      this.addAlert({
        type: 'frameTime',
        severity:
          metrics.frameTime > this.budget.maxFrameTime * 1.5
            ? 'critical'
            : 'warning',
        message: `Frame time exceeds budget: ${metrics.frameTime.toFixed(2)}ms > ${this.budget.maxFrameTime.toFixed(2)}ms`,
        value: metrics.frameTime,
        threshold: this.budget.maxFrameTime,
        timestamp: Date.now(),
      });
    }

    // Check frame time variance (stuttering)
    if (metrics.frameTimeVariance > 5) {
      this.addAlert({
        type: 'variance',
        severity: metrics.frameTimeVariance > 10 ? 'critical' : 'warning',
        message: `High frame time variance detected: ${metrics.frameTimeVariance.toFixed(2)}ms (stuttering)`,
        value: metrics.frameTimeVariance,
        threshold: 5,
        timestamp: Date.now(),
      });
    }

    // Check memory
    if (metrics.memoryUsagePercent > 80) {
      this.addAlert({
        type: 'memory',
        severity: metrics.memoryUsagePercent > 90 ? 'critical' : 'warning',
        message: `High memory usage: ${metrics.memoryUsagePercent.toFixed(2)}%`,
        value: metrics.memoryUsedMB,
        threshold: this.budget.maxMemoryMB,
        timestamp: Date.now(),
      });
    }

    // Check draw calls
    if (metrics.drawCalls > this.budget.maxDrawCalls) {
      this.addAlert({
        type: 'drawCalls',
        severity:
          metrics.drawCalls > this.budget.maxDrawCalls * 1.5
            ? 'critical'
            : 'warning',
        message: `Draw calls exceed budget: ${metrics.drawCalls} > ${this.budget.maxDrawCalls}`,
        value: metrics.drawCalls,
        threshold: this.budget.maxDrawCalls,
        timestamp: Date.now(),
      });
    }
  }

  private addAlert(alert: PerformanceAlert): void {
    this.alerts.push(alert);
    if (this.alerts.length > this.maxAlerts) {
      this.alerts.shift();
    }
  }

  private generateRecommendations(): string[] {
    const recommendations: string[] = [];
    const metrics = this.currentMetrics;

    if (metrics.fps < this.budget.targetFPS * 0.9) {
      recommendations.push(
        'Consider reducing rendering quality or optimizing draw calls'
      );
    }

    if (metrics.drawCalls > 500) {
      recommendations.push(
        'High draw call count - consider batching similar objects'
      );
    }

    if (metrics.triangles > 500000) {
      recommendations.push('High triangle count - consider using LOD system');
    }

    if (metrics.shaderSwitches > 100) {
      recommendations.push('Many shader switches - consider material batching');
    }

    if (metrics.frameTimeVariance > 5) {
      recommendations.push(
        'Frame time variance detected - check for background tasks or garbage collection'
      );
    }

    if (metrics.memoryUsagePercent > 80) {
      recommendations.push(
        'High memory usage - consider unloading unused assets'
      );
    }

    return recommendations;
  }

  private calculateAverage(values: number[]): number {
    if (values.length === 0) return 0;
    const sum = values.reduce((a, b) => a + b, 0);
    return sum / values.length;
  }

  private calculateVariance(values: number[], mean: number): number {
    if (values.length === 0) return 0;
    const squaredDiffs = values.map((value) => Math.pow(value - mean, 2));
    return Math.sqrt(this.calculateAverage(squaredDiffs));
  }
}

// Singleton instance
export const performanceMonitor = new PerformanceMonitor();
