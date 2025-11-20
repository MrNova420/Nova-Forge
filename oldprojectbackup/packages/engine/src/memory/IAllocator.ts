/**
 * @fileoverview Memory Allocator Interfaces
 * @module @nova-engine/memory
 *
 * Custom memory allocation system for performance and control.
 * Enables efficient memory management with specialized allocators.
 *
 * Following the autonomous development guide Phase 1.3.1
 * Research insights from:
 * - Unreal Engine's Memory Allocator (FMalloc)
 * - Unity's Native Memory Allocator
 * - Godot's Memory Pool
 */

/**
 * Allocation policy
 */
export enum AllocationPolicy {
  /** First-fit allocation */
  FIRST_FIT,
  /** Best-fit allocation */
  BEST_FIT,
  /** Worst-fit allocation */
  WORST_FIT,
  /** Next-fit allocation */
  NEXT_FIT,
}

/**
 * Allocation flags
 */
export enum AllocationFlags {
  /** No special flags */
  NONE = 0,
  /** Zero-initialize memory */
  ZERO_INIT = 1 << 0,
  /** Align to cache line */
  CACHE_ALIGNED = 1 << 1,
  /** Track for leak detection */
  TRACK = 1 << 2,
  /** Allow reallocation */
  REALLOCATABLE = 1 << 3,
}

/**
 * Memory statistics
 */
export interface MemoryStats {
  /** Total bytes allocated */
  totalAllocated: number;
  /** Total bytes freed */
  totalFreed: number;
  /** Current bytes in use */
  currentUsage: number;
  /** Peak bytes usage */
  peakUsage: number;
  /** Number of allocations */
  allocationCount: number;
  /** Number of deallocations */
  deallocationCount: number;
  /** Number of active allocations */
  activeAllocations: number;
  /** Fragmentation percentage (0-100) */
  fragmentationPercent: number;
}

/**
 * Allocation metadata
 */
export interface AllocationInfo {
  /** Allocation address */
  address: number;
  /** Allocation size */
  size: number;
  /** Alignment */
  alignment: number;
  /** Allocation timestamp */
  timestamp: number;
  /** Call stack (if tracking enabled) */
  callStack?: string;
  /** Label for debugging */
  label?: string;
}

/**
 * Base allocator interface
 *
 * All memory allocators must implement this interface.
 */
export interface IAllocator {
  /**
   * Allocate memory
   * @param size Size in bytes
   * @param alignment Alignment requirement (power of 2)
   * @param flags Allocation flags
   * @returns ArrayBuffer or throws on failure
   */
  allocate(
    size: number,
    alignment?: number,
    flags?: AllocationFlags
  ): ArrayBuffer;

  /**
   * Free allocated memory
   * @param buffer Buffer to free
   */
  free(buffer: ArrayBuffer): void;

  /**
   * Reallocate memory (resize existing allocation)
   * @param buffer Existing buffer
   * @param newSize New size in bytes
   * @returns New ArrayBuffer
   */
  reallocate(buffer: ArrayBuffer, newSize: number): ArrayBuffer;

  /**
   * Get total allocated memory
   */
  getTotalAllocated(): number;

  /**
   * Get current memory usage
   */
  getCurrentUsage(): number;

  /**
   * Get memory statistics
   */
  getStats(): MemoryStats;

  /**
   * Reset allocator (free all memory)
   */
  reset(): void;

  /**
   * Check if allocator owns this buffer
   */
  owns(buffer: ArrayBuffer): boolean;

  /**
   * Get allocator name
   */
  getName(): string;
}

/**
 * Allocator configuration
 */
export interface AllocatorConfig {
  /** Allocator name for debugging */
  name: string;
  /** Initial capacity in bytes */
  capacity?: number;
  /** Whether to track allocations */
  trackAllocations?: boolean;
  /** Whether to enable leak detection */
  detectLeaks?: boolean;
  /** Default alignment */
  defaultAlignment?: number;
}
