/**
 * @fileoverview Linear Allocator
 * @module @nova-engine/memory
 *
 * Fast linear allocator for frame-scoped allocations.
 * Allocates memory sequentially and resets in bulk.
 *
 * Following the autonomous development guide Phase 1.3.2
 */

import type {
  IAllocator,
  AllocatorConfig,
  AllocationFlags,
  MemoryStats,
} from './IAllocator';

/**
 * Linear Allocator
 *
 * Extremely fast allocator that bumps a pointer forward.
 * No individual frees - only bulk reset.
 * Perfect for per-frame allocations.
 *
 * Characteristics:
 * - O(1) allocation
 * - No fragmentation
 * - Cannot free individual allocations
 * - Must reset entire allocator
 *
 * @example
 * ```typescript
 * const allocator = new LinearAllocator({ name: 'Frame', capacity: 1024 * 1024 });
 *
 * // Frame start
 * const buffer = allocator.allocate(256);
 * // Use buffer...
 *
 * // Frame end - reset all
 * allocator.reset();
 * ```
 */
export class LinearAllocator implements IAllocator {
  private name: string;
  private capacity: number;
  private memory: ArrayBuffer;
  private offset = 0;
  private trackAllocations: boolean;
  private allocations: Map<ArrayBuffer, number> = new Map();
  private stats: MemoryStats;

  constructor(config: AllocatorConfig) {
    this.name = config.name;
    this.capacity = config.capacity || 1024 * 1024; // 1MB default
    this.trackAllocations = config.trackAllocations || false;
    this.memory = new ArrayBuffer(this.capacity);

    this.stats = {
      totalAllocated: 0,
      totalFreed: 0,
      currentUsage: 0,
      peakUsage: 0,
      allocationCount: 0,
      deallocationCount: 0,
      activeAllocations: 0,
      fragmentationPercent: 0,
    };
  }

  allocate(
    size: number,
    alignment = 8,
    flags: AllocationFlags = 0
  ): ArrayBuffer {
    // Align offset
    const alignedOffset = this.alignUp(this.offset, alignment);

    // Check capacity
    if (alignedOffset + size > this.capacity) {
      throw new Error(
        `LinearAllocator '${this.name}' out of memory: requested ${size} bytes, available ${this.capacity - alignedOffset}`
      );
    }

    // Create view into main buffer
    const buffer = this.memory.slice(alignedOffset, alignedOffset + size);

    // Zero-initialize if requested
    if (flags & 1) {
      // AllocationFlags.ZERO_INIT
      new Uint8Array(buffer).fill(0);
    }

    // Update offset
    this.offset = alignedOffset + size;

    // Track allocation
    if (this.trackAllocations) {
      this.allocations.set(buffer, size);
    }

    // Update stats
    this.stats.totalAllocated += size;
    this.stats.currentUsage = this.offset;
    this.stats.peakUsage = Math.max(this.stats.peakUsage, this.offset);
    this.stats.allocationCount++;
    this.stats.activeAllocations++;

    return buffer;
  }

  free(_buffer: ArrayBuffer): void {
    // Linear allocator doesn't support individual frees
    // Use reset() instead
    console.warn(`LinearAllocator '${this.name}' does not support individual frees. Use reset() instead.`);
  }

  reallocate(_buffer: ArrayBuffer, _newSize: number): ArrayBuffer {
    throw new Error(`LinearAllocator '${this.name}' does not support reallocation`);
  }

  reset(): void {
    this.offset = 0;
    this.stats.totalFreed += this.stats.currentUsage;
    this.stats.currentUsage = 0;
    this.stats.activeAllocations = 0;
    this.allocations.clear();
  }

  getTotalAllocated(): number {
    return this.stats.totalAllocated;
  }

  getCurrentUsage(): number {
    return this.stats.currentUsage;
  }

  getStats(): MemoryStats {
    return { ...this.stats };
  }

  owns(buffer: ArrayBuffer): boolean {
    return this.allocations.has(buffer);
  }

  getName(): string {
    return this.name;
  }

  /**
   * Get available space
   */
  getAvailable(): number {
    return this.capacity - this.offset;
  }

  /**
   * Get capacity utilization (0-1)
   */
  getUtilization(): number {
    return this.offset / this.capacity;
  }

  private alignUp(value: number, alignment: number): number {
    return Math.ceil(value / alignment) * alignment;
  }
}
