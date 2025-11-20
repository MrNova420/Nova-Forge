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
 * @fileoverview Profiling module exports
 */

export { Profiler, profiler } from './Profiler';
export { PerformanceMonitor, performanceMonitor } from './PerformanceMonitor';
export type {
  PerformanceMetrics,
  PerformanceBudget,
  PerformanceAlert,
} from './PerformanceMonitor';
export type {
  PerformanceMetric,
  PerformanceSample,
  MemorySnapshot,
} from './Profiler';
