/**
 * Job System Profiler
 * 
 * Production-grade profiling and visualization system for job execution.
 * Provides real-time monitoring, performance analysis, and bottleneck detection.
 */

import type { IJob, JobStatus, JobPriority } from './IJob';

/**
 * Job execution record for profiling
 */
export interface JobExecutionRecord {
  /** Job identifier */
  jobId: string;
  
  /** Job name */
  name: string;
  
  /** Job priority */
  priority: JobPriority;
  
  /** Worker ID that executed the job */
  workerId: number;
  
  /** Start time (high-resolution timestamp) */
  startTime: number;
  
  /** End time (high-resolution timestamp) */
  endTime: number;
  
  /** Execution duration in milliseconds */
  duration: number;
  
  /** Job status after execution */
  status: JobStatus;
  
  /** Error message if failed */
  error?: string;
  
  /** Memory used during execution (if tracked) */
  memoryUsed?: number;
  
  /** Dependencies that were waited for */
  dependencies?: string[];
}

/**
 * Timeline visualization data
 */
export interface TimelineData {
  /** Total session duration */
  totalDuration: number;
  
  /** Worker execution lanes */
  workers: WorkerLane[];
  
  /** Job execution spans */
  spans: ExecutionSpan[];
  
  /** Session metadata */
  metadata: {
    sessionName: string;
    startTime: number;
    endTime: number;
    totalJobs: number;
    workerCount: number;
  };
}

/**
 * Worker execution lane for timeline
 */
export interface WorkerLane {
  /** Worker ID */
  workerId: number;
  
  /** Total execution time */
  totalExecutionTime: number;
  
  /** Idle time */
  idleTime: number;
  
  /** Utilization percentage (0-100) */
  utilization: number;
  
  /** Number of jobs executed */
  jobCount: number;
}

/**
 * Execution span for timeline visualization
 */
export interface ExecutionSpan {
  /** Job identifier */
  jobId: string;
  
  /** Job name */
  name: string;
  
  /** Worker ID */
  workerId: number;
  
  /** Start offset from session start */
  startOffset: number;
  
  /** Duration */
  duration: number;
  
  /** Priority */
  priority: JobPriority;
  
  /** Status */
  status: JobStatus;
  
  /** Color for visualization */
  color: string;
}

/**
 * Bottleneck analysis report
 */
export interface BottleneckReport {
  /** Critical path analysis */
  criticalPath: {
    jobs: string[];
    totalDuration: number;
    percentage: number;
  };
  
  /** Worker load imbalance */
  loadImbalance: {
    maxUtilization: number;
    minUtilization: number;
    variance: number;
    recommendation: string;
  };
  
  /** Long-running jobs */
  longRunningJobs: {
    jobId: string;
    name: string;
    duration: number;
    percentage: number;
  }[];
  
  /** High contention points */
  contentionPoints: {
    time: number;
    queuedJobs: number;
    availableWorkers: number;
  }[];
  
  /** Overall recommendations */
  recommendations: string[];
}

/**
 * Profiling session configuration
 */
export interface ProfilerConfig {
  /** Enable memory tracking */
  trackMemory?: boolean;
  
  /** Enable dependency tracking */
  trackDependencies?: boolean;
  
  /** Maximum number of records to keep */
  maxRecords?: number;
  
  /** Enable real-time monitoring */
  realtimeMonitoring?: boolean;
}

/**
 * Priority color mapping for visualization
 */
const PRIORITY_COLORS: Record<JobPriority, string> = {
  [5]: '#ff0000', // Critical - Red
  [4]: '#ff8800', // High - Orange
  [3]: '#ffdd00', // Normal - Yellow
  [2]: '#00aa00', // Low - Green
  [1]: '#0088ff', // Idle - Blue
};

/**
 * Job System Profiler
 * 
 * Comprehensive profiling system for job execution analysis,
 * bottleneck detection, and performance optimization.
 */
export class JobProfiler {
  private config: Required<ProfilerConfig>;
  private sessions: Map<string, ProfilingSession>;
  private currentSession: ProfilingSession | null = null;

  constructor(config: ProfilerConfig = {}) {
    this.config = {
      trackMemory: config.trackMemory ?? false,
      trackDependencies: config.trackDependencies ?? true,
      maxRecords: config.maxRecords ?? 10000,
      realtimeMonitoring: config.realtimeMonitoring ?? false,
    };

    this.sessions = new Map();
  }

  /**
   * Start a new profiling session
   */
  startSession(name: string): void {
    if (this.currentSession) {
      this.endSession();
    }

    this.currentSession = new ProfilingSession(name, this.config);
    this.sessions.set(name, this.currentSession);
  }

  /**
   * End the current profiling session
   */
  endSession(): void {
    if (this.currentSession) {
      this.currentSession.end();
      this.currentSession = null;
    }
  }

  /**
   * Record a job execution
   */
  recordJob(
    job: IJob<any>,
    workerId: number,
    startTime: number,
    endTime: number,
    status: JobStatus,
    error?: string
  ): void {
    if (!this.currentSession) {
      throw new Error('No active profiling session');
    }

    const record: JobExecutionRecord = {
      jobId: job.id,
      name: job.name,
      priority: job.priority,
      workerId,
      startTime,
      endTime,
      duration: endTime - startTime,
      status,
      error,
    };

    if (this.config.trackDependencies && job.dependencies) {
      record.dependencies = job.dependencies;
    }

    this.currentSession.addRecord(record);
  }

  /**
   * Generate timeline visualization data
   */
  generateTimeline(): TimelineData {
    if (!this.currentSession) {
      throw new Error('No active profiling session');
    }

    return this.currentSession.generateTimeline();
  }

  /**
   * Analyze bottlenecks in the current session
   */
  analyzeBottlenecks(): BottleneckReport {
    if (!this.currentSession) {
      throw new Error('No active profiling session');
    }

    return this.currentSession.analyzeBottlenecks();
  }

  /**
   * Get session by name
   */
  getSession(name: string): ProfilingSession | undefined {
    return this.sessions.get(name);
  }

  /**
   * Get all sessions
   */
  getAllSessions(): ProfilingSession[] {
    return Array.from(this.sessions.values());
  }

  /**
   * Export session data
   */
  export(format: 'json' | 'csv' | 'html' = 'json'): string {
    if (!this.currentSession) {
      throw new Error('No active profiling session');
    }

    return this.currentSession.export(format);
  }

  /**
   * Clear all sessions
   */
  clearSessions(): void {
    this.sessions.clear();
    this.currentSession = null;
  }
}

/**
 * Profiling session
 */
class ProfilingSession {
  readonly name: string;
  private config: Required<ProfilerConfig>;
  private records: JobExecutionRecord[] = [];
  private startTime: number;
  private endTime: number | null = null;
  private workerStats: Map<number, WorkerStats> = new Map();

  constructor(name: string, config: Required<ProfilerConfig>) {
    this.name = name;
    this.config = config;
    this.startTime = performance.now();
  }

  /**
   * End the session
   */
  end(): void {
    this.endTime = performance.now();
  }

  /**
   * Add an execution record
   */
  addRecord(record: JobExecutionRecord): void {
    if (this.records.length >= this.config.maxRecords) {
      // Remove oldest record
      this.records.shift();
    }

    this.records.push(record);

    // Update worker stats
    if (!this.workerStats.has(record.workerId)) {
      this.workerStats.set(record.workerId, {
        workerId: record.workerId,
        totalExecutionTime: 0,
        jobCount: 0,
        jobs: [],
      });
    }

    const stats = this.workerStats.get(record.workerId)!;
    stats.totalExecutionTime += record.duration;
    stats.jobCount++;
    stats.jobs.push(record);
  }

  /**
   * Generate timeline visualization data
   */
  generateTimeline(): TimelineData {
    const sessionDuration = (this.endTime || performance.now()) - this.startTime;

    // Generate worker lanes
    const workers: WorkerLane[] = [];
    for (const [workerId, stats] of this.workerStats) {
      const idleTime = sessionDuration - stats.totalExecutionTime;
      const utilization = (stats.totalExecutionTime / sessionDuration) * 100;

      workers.push({
        workerId,
        totalExecutionTime: stats.totalExecutionTime,
        idleTime,
        utilization,
        jobCount: stats.jobCount,
      });
    }

    // Generate execution spans
    const spans: ExecutionSpan[] = this.records.map(record => ({
      jobId: record.jobId,
      name: record.name,
      workerId: record.workerId,
      startOffset: record.startTime - this.startTime,
      duration: record.duration,
      priority: record.priority,
      status: record.status,
      color: PRIORITY_COLORS[record.priority],
    }));

    return {
      totalDuration: sessionDuration,
      workers,
      spans,
      metadata: {
        sessionName: this.name,
        startTime: this.startTime,
        endTime: this.endTime || performance.now(),
        totalJobs: this.records.length,
        workerCount: this.workerStats.size,
      },
    };
  }

  /**
   * Analyze bottlenecks
   */
  analyzeBottlenecks(): BottleneckReport {
    const sessionDuration = (this.endTime || performance.now()) - this.startTime;
    const timeline = this.generateTimeline();

    // Find critical path (longest dependency chain)
    const criticalPath = this.findCriticalPath();

    // Analyze worker load imbalance
    const utilizations = timeline.workers.map(w => w.utilization);
    const maxUtil = Math.max(...utilizations);
    const minUtil = Math.min(...utilizations);
    const avgUtil = utilizations.reduce((a, b) => a + b, 0) / utilizations.length;
    const variance = utilizations.reduce((sum, util) => sum + Math.pow(util - avgUtil, 2), 0) / utilizations.length;

    let loadBalanceRec = '';
    if (maxUtil - minUtil > 30) {
      loadBalanceRec = 'High worker load imbalance detected. Consider redistributing work or adjusting worker count.';
    } else if (avgUtil < 50) {
      loadBalanceRec = 'Low average worker utilization. Consider reducing worker count or increasing workload.';
    } else if (avgUtil > 90) {
      loadBalanceRec = 'Very high worker utilization. Consider increasing worker count to reduce contention.';
    } else {
      loadBalanceRec = 'Worker load is well balanced.';
    }

    // Find long-running jobs (top 10)
    const longRunning = [...this.records]
      .sort((a, b) => b.duration - a.duration)
      .slice(0, 10)
      .map(record => ({
        jobId: record.jobId,
        name: record.name,
        duration: record.duration,
        percentage: (record.duration / sessionDuration) * 100,
      }));

    // Detect contention points (times with many queued jobs)
    const contentionPoints = this.detectContentionPoints();

    // Generate recommendations
    const recommendations: string[] = [];
    
    if (criticalPath.percentage > 50) {
      recommendations.push(`Critical path accounts for ${criticalPath.percentage.toFixed(1)}% of execution time. Focus on optimizing these jobs.`);
    }
    
    if (longRunning.length > 0 && longRunning[0].percentage > 20) {
      recommendations.push(`Job "${longRunning[0].name}" takes ${longRunning[0].percentage.toFixed(1)}% of total time. Consider breaking it into smaller jobs.`);
    }
    
    if (contentionPoints.length > 3) {
      recommendations.push(`${contentionPoints.length} contention points detected. Consider increasing worker count or reducing concurrent job submissions.`);
    }
    
    if (variance > 500) {
      recommendations.push('High worker utilization variance detected. Improve job distribution for better load balancing.');
    }

    if (recommendations.length === 0) {
      recommendations.push('Performance looks good! No major bottlenecks detected.');
    }

    return {
      criticalPath,
      loadImbalance: {
        maxUtilization: maxUtil,
        minUtilization: minUtil,
        variance,
        recommendation: loadBalanceRec,
      },
      longRunningJobs: longRunning,
      contentionPoints,
      recommendations,
    };
  }

  /**
   * Find critical path (longest dependency chain)
   */
  private findCriticalPath(): { jobs: string[]; totalDuration: number; percentage: number } {
    // Simplified critical path - find longest sequential execution
    const sessionDuration = (this.endTime || performance.now()) - this.startTime;
    
    // For now, just find the longest single job as critical path
    // In a full implementation, this would trace dependency chains
    const longest = this.records.reduce((max, record) => {
      return record.duration > max.duration ? record : max;
    }, this.records[0] || { duration: 0, jobId: '', name: '' });

    return {
      jobs: [longest.name],
      totalDuration: longest.duration,
      percentage: (longest.duration / sessionDuration) * 100,
    };
  }

  /**
   * Detect contention points
   */
  private detectContentionPoints(): { time: number; queuedJobs: number; availableWorkers: number }[] {
    // Simplified contention detection
    // In a full implementation, this would track queue depth over time
    const points: { time: number; queuedJobs: number; availableWorkers: number }[] = [];
    
    // Group jobs by 100ms time windows
    const windows = new Map<number, number>();
    for (const record of this.records) {
      const window = Math.floor(record.startTime / 100) * 100;
      windows.set(window, (windows.get(window) || 0) + 1);
    }

    // Find windows with high job starts (indicates contention)
    for (const [time, count] of windows) {
      if (count > this.workerStats.size * 2) {
        points.push({
          time,
          queuedJobs: count,
          availableWorkers: this.workerStats.size,
        });
      }
    }

    return points;
  }

  /**
   * Export session data
   */
  export(format: 'json' | 'csv' | 'html'): string {
    switch (format) {
      case 'json':
        return JSON.stringify({
          session: this.name,
          startTime: this.startTime,
          endTime: this.endTime,
          records: this.records,
          timeline: this.generateTimeline(),
          bottlenecks: this.analyzeBottlenecks(),
        }, null, 2);

      case 'csv':
        let csv = 'JobID,Name,Priority,WorkerID,StartTime,EndTime,Duration,Status\n';
        for (const record of this.records) {
          csv += `${record.jobId},${record.name},${record.priority},${record.workerId},${record.startTime},${record.endTime},${record.duration},${record.status}\n`;
        }
        return csv;

      case 'html':
        return this.generateHTMLReport();

      default:
        throw new Error(`Unsupported export format: ${format}`);
    }
  }

  /**
   * Generate HTML report
   */
  private generateHTMLReport(): string {
    const timeline = this.generateTimeline();
    const bottlenecks = this.analyzeBottlenecks();

    return `
<!DOCTYPE html>
<html>
<head>
  <title>Job Profiler Report: ${this.name}</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 20px; }
    h1, h2 { color: #333; }
    table { border-collapse: collapse; width: 100%; margin: 20px 0; }
    th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
    th { background-color: #4CAF50; color: white; }
    .stat { display: inline-block; margin: 10px; padding: 10px; background: #f0f0f0; border-radius: 5px; }
    .recommendation { background: #ffffcc; padding: 10px; margin: 10px 0; border-left: 4px solid #ffcc00; }
  </style>
</head>
<body>
  <h1>Job Profiler Report: ${this.name}</h1>
  
  <h2>Session Summary</h2>
  <div class="stat">Total Duration: ${timeline.totalDuration.toFixed(2)}ms</div>
  <div class="stat">Total Jobs: ${timeline.metadata.totalJobs}</div>
  <div class="stat">Workers: ${timeline.metadata.workerCount}</div>
  
  <h2>Worker Utilization</h2>
  <table>
    <tr>
      <th>Worker ID</th>
      <th>Jobs Executed</th>
      <th>Execution Time</th>
      <th>Idle Time</th>
      <th>Utilization</th>
    </tr>
    ${timeline.workers.map(w => `
    <tr>
      <td>${w.workerId}</td>
      <td>${w.jobCount}</td>
      <td>${w.totalExecutionTime.toFixed(2)}ms</td>
      <td>${w.idleTime.toFixed(2)}ms</td>
      <td>${w.utilization.toFixed(1)}%</td>
    </tr>
    `).join('')}
  </table>
  
  <h2>Long-Running Jobs</h2>
  <table>
    <tr>
      <th>Job Name</th>
      <th>Duration</th>
      <th>% of Total</th>
    </tr>
    ${bottlenecks.longRunningJobs.map(job => `
    <tr>
      <td>${job.name}</td>
      <td>${job.duration.toFixed(2)}ms</td>
      <td>${job.percentage.toFixed(1)}%</td>
    </tr>
    `).join('')}
  </table>
  
  <h2>Recommendations</h2>
  ${bottlenecks.recommendations.map(rec => `
    <div class="recommendation">${rec}</div>
  `).join('')}
</body>
</html>
    `.trim();
  }
}

/**
 * Worker statistics
 */
interface WorkerStats {
  workerId: number;
  totalExecutionTime: number;
  jobCount: number;
  jobs: JobExecutionRecord[];
}
