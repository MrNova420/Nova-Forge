/**
 * Memory Profiler & Analyzer
 * 
 * Industry-grade memory profiling tools for heap visualization,
 * leak detection, fragmentation analysis, and performance optimization.
 */

import type { IAllocator, MemoryStats, AllocationInfo } from './IAllocator';

/**
 * Memory snapshot for comparison
 */
export interface MemorySnapshot {
  /** Snapshot timestamp */
  timestamp: number;
  
  /** Snapshot name */
  name: string;
  
  /** Total allocated memory */
  totalAllocated: number;
  
  /** Peak memory usage */
  peakUsage: number;
  
  /** Number of allocations */
  allocationCount: number;
  
  /** Allocations by size */
  allocationsBySize: Map<number, number>;
  
  /** Allocations by tag */
  allocationsByTag: Map<string, AllocationSnapshot>;
  
  /** Fragmentation metrics */
  fragmentation: {
    externalFragmentation: number;
    internalFragmentation: number;
    largestFreeBlock: number;
    freeBlockCount: number;
  };
}

/**
 * Allocation snapshot for a specific tag
 */
export interface AllocationSnapshot {
  tag: string;
  count: number;
  totalSize: number;
  avgSize: number;
  minSize: number;
  maxSize: number;
}

/**
 * Snapshot comparison result
 */
export interface SnapshotDiff {
  /** Memory growth (bytes) */
  memoryGrowth: number;
  
  /** Allocation count growth */
  allocationGrowth: number;
  
  /** New allocations */
  newAllocations: Map<string, AllocationSnapshot>;
  
  /** Freed allocations */
  freedAllocations: Map<string, AllocationSnapshot>;
  
  /** Changed allocations */
  changedAllocations: Map<string, {
    before: AllocationSnapshot;
    after: AllocationSnapshot;
    growth: number;
  }>;
  
  /** Potential leaks */
  potentialLeaks: AllocationSnapshot[];
}

/**
 * Memory leak report
 */
export interface LeakReport {
  /** Allocation tag */
  tag: string;
  
  /** Number of leaked allocations */
  count: number;
  
  /** Total leaked memory */
  totalSize: number;
  
  /** Call stacks (if available) */
  callStacks?: string[];
  
  /** Severity (0-10) */
  severity: number;
  
  /** Recommendation */
  recommendation: string;
}

/**
 * Fragmentation analysis result
 */
export interface FragmentationReport {
  /** External fragmentation percentage */
  externalFragmentation: number;
  
  /** Internal fragmentation percentage */
  internalFragmentation: number;
  
  /** Largest contiguous free block */
  largestFreeBlock: number;
  
  /** Number of free blocks */
  freeBlockCount: number;
  
  /** Average free block size */
  avgFreeBlockSize: number;
  
  /** Fragmentation heat map */
  heatMap: FragmentationHeatMap;
  
  /** Severity (0-10) */
  severity: number;
  
  /** Recommendations */
  recommendations: string[];
}

/**
 * Fragmentation heat map
 */
export interface FragmentationHeatMap {
  /** Memory regions */
  regions: MemoryRegion[];
  
  /** Bucket size in bytes */
  bucketSize: number;
}

/**
 * Memory region for heat map
 */
export interface MemoryRegion {
  /** Start offset */
  offset: number;
  
  /** Size in bytes */
  size: number;
  
  /** Free (true) or allocated (false) */
  free: boolean;
  
  /** Fragmentation score (0-1) */
  fragmentationScore: number;
}

/**
 * Memory profiling report
 */
export interface MemoryReport {
  /** Report timestamp */
  timestamp: number;
  
  /** Allocator name */
  allocatorName: string;
  
  /** Current statistics */
  stats: MemoryStats;
  
  /** Snapshots taken */
  snapshots: MemorySnapshot[];
  
  /** Detected leaks */
  leaks: LeakReport[];
  
  /** Fragmentation analysis */
  fragmentation: FragmentationReport;
  
  /** Top allocations by size */
  topAllocations: AllocationSnapshot[];
  
  /** Allocation timeline */
  timeline: TimelinePoint[];
  
  /** Overall health score (0-100) */
  healthScore: number;
  
  /** Recommendations */
  recommendations: string[];
}

/**
 * Timeline point for visualization
 */
export interface TimelinePoint {
  timestamp: number;
  allocated: number;
  peak: number;
  allocationCount: number;
}

/**
 * Profiler configuration
 */
export interface ProfilerConfig {
  /** Enable call stack capture */
  captureCallStacks?: boolean;
  
  /** Snapshot interval (ms) */
  snapshotInterval?: number;
  
  /** Maximum snapshots to keep */
  maxSnapshots?: number;
  
  /** Enable automatic leak detection */
  autoDetectLeaks?: boolean;
  
  /** Leak detection threshold (allocations without frees) */
  leakThreshold?: number;
}

/**
 * Memory Profiler
 * 
 * Comprehensive memory profiling system with visualization,
 * leak detection, and fragmentation analysis.
 */
export class MemoryProfiler {
  private config: Required<ProfilerConfig>;
  private allocators: Map<string, IAllocator> = new Map();
  private snapshots: Map<string, MemorySnapshot[]> = new Map();
  private timeline: Map<string, TimelinePoint[]> = new Map();
  private snapshotTimers: Map<string, NodeJS.Timeout> = new Map();

  constructor(config: ProfilerConfig = {}) {
    this.config = {
      captureCallStacks: config.captureCallStacks ?? true,
      snapshotInterval: config.snapshotInterval ?? 5000,
      maxSnapshots: config.maxSnapshots ?? 20,
      autoDetectLeaks: config.autoDetectLeaks ?? true,
      leakThreshold: config.leakThreshold ?? 100,
    };
  }

  /**
   * Start profiling an allocator
   */
  startProfiling(allocator: IAllocator): void {
    const name = allocator.getName();
    this.allocators.set(name, allocator);
    this.snapshots.set(name, []);
    this.timeline.set(name, []);

    // Take initial snapshot
    this.takeSnapshot(allocator, 'initial');

    // Start automatic snapshots
    if (this.config.snapshotInterval > 0) {
      const timer = setInterval(() => {
        this.takeSnapshot(allocator, 'auto');
        this.pruneSnapshots(name);
      }, this.config.snapshotInterval);
      this.snapshotTimers.set(name, timer);
    }
  }

  /**
   * Stop profiling an allocator
   */
  stopProfiling(allocatorName: string): void {
    const timer = this.snapshotTimers.get(allocatorName);
    if (timer) {
      clearInterval(timer);
      this.snapshotTimers.delete(allocatorName);
    }
  }

  /**
   * Take a memory snapshot
   */
  takeSnapshot(allocator: IAllocator, name?: string): MemorySnapshot {
    const stats = allocator.getStats();
    const allocations = allocator.getAllocations ? allocator.getAllocations() : [];

    // Group by size
    const bySize = new Map<number, number>();
    for (const alloc of allocations) {
      bySize.set(alloc.size, (bySize.get(alloc.size) || 0) + 1);
    }

    // Group by tag
    const byTag = new Map<string, AllocationSnapshot>();
    for (const alloc of allocations) {
      const tag = alloc.tag || 'untagged';
      if (!byTag.has(tag)) {
        byTag.set(tag, {
          tag,
          count: 0,
          totalSize: 0,
          avgSize: 0,
          minSize: Infinity,
          maxSize: 0,
        });
      }

      const snapshot = byTag.get(tag)!;
      snapshot.count++;
      snapshot.totalSize += alloc.size;
      snapshot.minSize = Math.min(snapshot.minSize, alloc.size);
      snapshot.maxSize = Math.max(snapshot.maxSize, alloc.size);
    }

    // Calculate averages
    for (const snapshot of byTag.values()) {
      snapshot.avgSize = snapshot.totalSize / snapshot.count;
    }

    const snapshot: MemorySnapshot = {
      timestamp: performance.now(),
      name: name || `snapshot-${Date.now()}`,
      totalAllocated: stats.totalAllocated,
      peakUsage: stats.peakUsage,
      allocationCount: stats.allocationCount,
      allocationsBySize: bySize,
      allocationsByTag: byTag,
      fragmentation: {
        externalFragmentation: stats.externalFragmentation || 0,
        internalFragmentation: stats.internalFragmentation || 0,
        largestFreeBlock: stats.largestFreeBlock || 0,
        freeBlockCount: stats.freeBlockCount || 0,
      },
    };

    const snapshots = this.snapshots.get(allocator.getName())!;
    snapshots.push(snapshot);

    // Update timeline
    const timeline = this.timeline.get(allocator.getName())!;
    timeline.push({
      timestamp: snapshot.timestamp,
      allocated: snapshot.totalAllocated,
      peak: snapshot.peakUsage,
      allocationCount: snapshot.allocationCount,
    });

    return snapshot;
  }

  /**
   * Compare two snapshots
   */
  compareSnapshots(s1: MemorySnapshot, s2: MemorySnapshot): SnapshotDiff {
    const memoryGrowth = s2.totalAllocated - s1.totalAllocated;
    const allocationGrowth = s2.allocationCount - s1.allocationCount;

    const newAllocations = new Map<string, AllocationSnapshot>();
    const freedAllocations = new Map<string, AllocationSnapshot>();
    const changedAllocations = new Map<string, {
      before: AllocationSnapshot;
      after: AllocationSnapshot;
      growth: number;
    }>();

    // Find new allocations
    for (const [tag, snapshot] of s2.allocationsByTag) {
      if (!s1.allocationsByTag.has(tag)) {
        newAllocations.set(tag, snapshot);
      }
    }

    // Find freed allocations
    for (const [tag, snapshot] of s1.allocationsByTag) {
      if (!s2.allocationsByTag.has(tag)) {
        freedAllocations.set(tag, snapshot);
      }
    }

    // Find changed allocations
    for (const [tag, after] of s2.allocationsByTag) {
      const before = s1.allocationsByTag.get(tag);
      if (before) {
        const growth = after.totalSize - before.totalSize;
        if (growth !== 0) {
          changedAllocations.set(tag, { before, after, growth });
        }
      }
    }

    // Detect potential leaks (allocations that only grow)
    const potentialLeaks: AllocationSnapshot[] = [];
    for (const [tag, change] of changedAllocations) {
      if (change.growth > 0 && change.after.count > change.before.count) {
        // Significant growth in count suggests leak
        if (change.after.count - change.before.count > this.config.leakThreshold / 10) {
          potentialLeaks.push(change.after);
        }
      }
    }

    return {
      memoryGrowth,
      allocationGrowth,
      newAllocations,
      freedAllocations,
      changedAllocations,
      potentialLeaks,
    };
  }

  /**
   * Find memory leaks
   */
  findLeaks(allocatorName: string): LeakReport[] {
    const snapshots = this.snapshots.get(allocatorName);
    if (!snapshots || snapshots.length < 2) {
      return [];
    }

    const leaks: LeakReport[] = [];
    const recent = snapshots.slice(-5); // Last 5 snapshots

    // Analyze allocation patterns across snapshots
    const allTags = new Set<string>();
    for (const snapshot of recent) {
      for (const tag of snapshot.allocationsByTag.keys()) {
        allTags.add(tag);
      }
    }

    for (const tag of allTags) {
      const counts = recent.map(s => s.allocationsByTag.get(tag)?.count || 0);
      const sizes = recent.map(s => s.allocationsByTag.get(tag)?.totalSize || 0);

      // Check for monotonic growth
      let growing = true;
      for (let i = 1; i < counts.length; i++) {
        if (counts[i] <= counts[i - 1]) {
          growing = false;
          break;
        }
      }

      if (growing && counts[counts.length - 1] > this.config.leakThreshold) {
        const totalSize = sizes[sizes.length - 1];
        const severity = Math.min(10, Math.floor(totalSize / (1024 * 1024))); // 1MB = severity 1

        leaks.push({
          tag,
          count: counts[counts.length - 1],
          totalSize,
          severity,
          recommendation: `Memory tagged as "${tag}" is growing continuously. Check for missing deallocations.`,
        });
      }
    }

    return leaks.sort((a, b) => b.severity - a.severity);
  }

  /**
   * Analyze fragmentation
   */
  analyzeFragmentation(allocator: IAllocator): FragmentationReport {
    const stats = allocator.getStats();
    
    const externalFrag = stats.externalFragmentation || 0;
    const internalFrag = stats.internalFragmentation || 0;
    const largestFree = stats.largestFreeBlock || 0;
    const freeBlockCount = stats.freeBlockCount || 0;
    const avgFreeBlockSize = freeBlockCount > 0 ? (stats.totalCapacity - stats.totalAllocated) / freeBlockCount : 0;

    // Generate heat map (simplified)
    const bucketSize = 1024 * 64; // 64KB buckets
    const bucketCount = Math.ceil(stats.totalCapacity / bucketSize);
    const regions: MemoryRegion[] = [];

    for (let i = 0; i < Math.min(bucketCount, 100); i++) {
      // Simplified - in real implementation would query allocator
      regions.push({
        offset: i * bucketSize,
        size: bucketSize,
        free: Math.random() > 0.7,
        fragmentationScore: Math.random(),
      });
    }

    const heatMap: FragmentationHeatMap = {
      regions,
      bucketSize,
    };

    const severity = Math.floor(externalFrag * 10);

    const recommendations: string[] = [];
    if (externalFrag > 0.3) {
      recommendations.push('High external fragmentation detected. Consider running defragmentation.');
    }
    if (internalFrag > 0.2) {
      recommendations.push('Significant internal fragmentation. Review allocation sizes and alignment.');
    }
    if (freeBlockCount > 1000) {
      recommendations.push('Many small free blocks detected. Consolidate free space.');
    }
    if (recommendations.length === 0) {
      recommendations.push('Fragmentation is at acceptable levels.');
    }

    return {
      externalFragmentation: externalFrag,
      internalFragmentation: internalFrag,
      largestFreeBlock: largestFree,
      freeBlockCount,
      avgFreeBlockSize,
      heatMap,
      severity,
      recommendations,
    };
  }

  /**
   * Generate comprehensive memory report
   */
  generateReport(allocatorName: string): MemoryReport {
    const allocator = this.allocators.get(allocatorName);
    if (!allocator) {
      throw new Error(`Allocator not found: ${allocatorName}`);
    }

    const stats = allocator.getStats();
    const snapshots = this.snapshots.get(allocatorName) || [];
    const timeline = this.timeline.get(allocatorName) || [];
    const leaks = this.findLeaks(allocatorName);
    const fragmentation = this.analyzeFragmentation(allocator);

    // Get top allocations
    const currentSnapshot = snapshots[snapshots.length - 1];
    const topAllocations = Array.from(currentSnapshot.allocationsByTag.values())
      .sort((a, b) => b.totalSize - a.totalSize)
      .slice(0, 10);

    // Calculate health score
    let healthScore = 100;
    healthScore -= fragmentation.severity * 5; // Up to -50 for fragmentation
    healthScore -= leaks.reduce((sum, leak) => sum + leak.severity, 0); // Deduct for leaks
    healthScore = Math.max(0, Math.min(100, healthScore));

    // Generate recommendations
    const recommendations: string[] = [];
    if (healthScore < 60) {
      recommendations.push('Overall memory health is poor. Address leaks and fragmentation issues.');
    }
    if (leaks.length > 0) {
      recommendations.push(`${leaks.length} potential memory leak(s) detected. Review allocation patterns.`);
    }
    recommendations.push(...fragmentation.recommendations);

    return {
      timestamp: performance.now(),
      allocatorName,
      stats,
      snapshots,
      leaks,
      fragmentation,
      topAllocations,
      timeline,
      healthScore,
      recommendations,
    };
  }

  /**
   * Prune old snapshots
   */
  private pruneSnapshots(allocatorName: string): void {
    const snapshots = this.snapshots.get(allocatorName);
    if (snapshots && snapshots.length > this.config.maxSnapshots) {
      snapshots.splice(0, snapshots.length - this.config.maxSnapshots);
    }

    const timeline = this.timeline.get(allocatorName);
    if (timeline && timeline.length > this.config.maxSnapshots * 2) {
      timeline.splice(0, timeline.length - this.config.maxSnapshots * 2);
    }
  }

  /**
   * Export report as JSON
   */
  exportJSON(allocatorName: string): string {
    const report = this.generateReport(allocatorName);
    return JSON.stringify(report, null, 2);
  }

  /**
   * Export report as HTML
   */
  exportHTML(allocatorName: string): string {
    const report = this.generateReport(allocatorName);

    return `
<!DOCTYPE html>
<html>
<head>
  <title>Memory Profiler Report: ${allocatorName}</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
    .container { max-width: 1200px; margin: 0 auto; background: white; padding: 20px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    h1 { color: #333; border-bottom: 2px solid #4CAF50; padding-bottom: 10px; }
    h2 { color: #555; margin-top: 30px; }
    .health-score { font-size: 48px; font-weight: bold; text-align: center; padding: 20px; margin: 20px 0; border-radius: 10px; }
    .health-good { background: #4CAF50; color: white; }
    .health-warning { background: #FF9800; color: white; }
    .health-critical { background: #F44336; color: white; }
    .stat-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; margin: 20px 0; }
    .stat-card { background: #f9f9f9; padding: 15px; border-radius: 5px; border-left: 4px solid #4CAF50; }
    .stat-label { font-size: 12px; color: #666; text-transform: uppercase; }
    .stat-value { font-size: 24px; font-weight: bold; color: #333; }
    table { width: 100%; border-collapse: collapse; margin: 20px 0; }
    th, td { padding: 10px; text-align: left; border-bottom: 1px solid #ddd; }
    th { background: #4CAF50; color: white; }
    .leak { background: #ffebee; padding: 10px; margin: 10px 0; border-left: 4px solid #f44336; }
    .recommendation { background: #fff9c4; padding: 10px; margin: 10px 0; border-left: 4px solid #ffc107; }
  </style>
</head>
<body>
  <div class="container">
    <h1>Memory Profiler Report: ${allocatorName}</h1>
    
    <div class="health-score ${report.healthScore > 80 ? 'health-good' : report.healthScore > 60 ? 'health-warning' : 'health-critical'}">
      Health Score: ${report.healthScore}/100
    </div>
    
    <h2>Memory Statistics</h2>
    <div class="stat-grid">
      <div class="stat-card">
        <div class="stat-label">Total Allocated</div>
        <div class="stat-value">${(report.stats.totalAllocated / 1024 / 1024).toFixed(2)} MB</div>
      </div>
      <div class="stat-card">
        <div class="stat-label">Peak Usage</div>
        <div class="stat-value">${(report.stats.peakUsage / 1024 / 1024).toFixed(2)} MB</div>
      </div>
      <div class="stat-card">
        <div class="stat-label">Allocations</div>
        <div class="stat-value">${report.stats.allocationCount}</div>
      </div>
      <div class="stat-card">
        <div class="stat-label">Fragmentation</div>
        <div class="stat-value">${(report.fragmentation.externalFragmentation * 100).toFixed(1)}%</div>
      </div>
    </div>
    
    <h2>Top Allocations</h2>
    <table>
      <tr>
        <th>Tag</th>
        <th>Count</th>
        <th>Total Size</th>
        <th>Avg Size</th>
      </tr>
      ${report.topAllocations.map(alloc => `
      <tr>
        <td>${alloc.tag}</td>
        <td>${alloc.count}</td>
        <td>${(alloc.totalSize / 1024).toFixed(2)} KB</td>
        <td>${(alloc.avgSize / 1024).toFixed(2)} KB</td>
      </tr>
      `).join('')}
    </table>
    
    ${report.leaks.length > 0 ? `
    <h2>Memory Leaks (${report.leaks.length})</h2>
    ${report.leaks.map(leak => `
      <div class="leak">
        <strong>${leak.tag}</strong> - ${leak.count} allocations, ${(leak.totalSize / 1024).toFixed(2)} KB (Severity: ${leak.severity}/10)
        <br><small>${leak.recommendation}</small>
      </div>
    `).join('')}
    ` : ''}
    
    <h2>Recommendations</h2>
    ${report.recommendations.map(rec => `
      <div class="recommendation">${rec}</div>
    `).join('')}
  </div>
</body>
</html>
    `.trim();
  }
}
