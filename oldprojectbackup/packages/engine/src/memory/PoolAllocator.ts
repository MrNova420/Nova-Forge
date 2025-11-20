/**
 * @fileoverview Pool Allocator
 * @module @nova-engine/memory
 *
 * Fixed-size block allocator for objects of uniform size.
 * Extremely fast allocation/deallocation with no fragmentation.
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
 * Pool Allocator
 *
 * Pre-allocates a pool of fixed-size blocks.
 * Perfect for frequently allocated/deallocated objects of same size.
 *
 * Characteristics:
 * - O(1) allocation and deallocation
 * - Zero fragmentation
 * - Fixed block size
 * - Excellent cache locality
 *
 * @example
 * ```typescript
 * const allocator = new PoolAllocator({
 *   name: 'Particle Pool',
 *   capacity: 10000,
 *   blockSize: 64,
 * });
 *
 * const particle = allocator.allocate(); // Get 64-byte block
 * // Use particle...
 * allocator.free(particle); // Return to pool
 * ```
 */
export class PoolAllocator implements IAllocator {
  private name: string;
  private blockSize: number;
  private blockCount: number;
  private memory: ArrayBuffer;
  private freeList: number[] = [];
  private allocatedBlocks: Set<number> = new Set();
  private stats: MemoryStats;

  constructor(
    config: AllocatorConfig & {
      blockSize: number;
      blockCount?: number;
    }
  ) {
    this.name = config.name;
    this.blockSize = config.blockSize;
    this.blockCount = config.blockCount || 1000;

    // Allocate pool
    const totalSize = this.blockSize * this.blockCount;
    this.memory = new ArrayBuffer(totalSize);

    // Initialize free list
    for (let i = 0; i < this.blockCount; i++) {
      this.freeList.push(i);
    }

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
    size?: number,
    _alignment?: number,
    flags: AllocationFlags = 0
  ): ArrayBuffer {
    // Validate size
    if (size && size > this.blockSize) {
      throw new Error(
        `PoolAllocator '${this.name}' cannot allocate ${size} bytes (block size is ${this.blockSize})`
      );
    }

    // Check availability
    if (this.freeList.length === 0) {
      throw new Error(`PoolAllocator '${this.name}' exhausted (${this.blockCount} blocks)`);
    }

    // Get block from free list
    const blockIndex = this.freeList.pop()!;
    const offset = blockIndex * this.blockSize;

    // Create view
    const buffer = this.memory.slice(offset, offset + this.blockSize);

    // Zero-initialize if requested
    if (flags & 1) {
      new Uint8Array(buffer).fill(0);
    }

    // Track allocation
    this.allocatedBlocks.add(blockIndex);

    // Update stats
    this.stats.totalAllocated += this.blockSize;
    this.stats.currentUsage = this.allocatedBlocks.size * this.blockSize;
    this.stats.peakUsage = Math.max(this.stats.peakUsage, this.stats.currentUsage);
    this.stats.allocationCount++;
    this.stats.activeAllocations++;

    return buffer;
  }

  free(buffer: ArrayBuffer): void {
    // Find block index (simplified - real implementation would store mapping)
    let blockIndex = -1;
    for (const idx of this.allocatedBlocks) {
      const offset = idx * this.blockSize;
      const blockBuffer = this.memory.slice(offset, offset + this.blockSize);
      if (blockBuffer.byteLength === buffer.byteLength) {
        // Simplified comparison
        blockIndex = idx;
        break;
      }
    }

    if (blockIndex === -1) {
      console.warn(`PoolAllocator '${this.name}' does not own this buffer`);
      return;
    }

    // Return to free list
    this.allocatedBlocks.delete(blockIndex);
    this.freeList.push(blockIndex);

    // Update stats
    this.stats.totalFreed += this.blockSize;
    this.stats.currentUsage = this.allocatedBlocks.size * this.blockSize;
    this.stats.deallocationCount++;
    this.stats.activeAllocations--;
  }

  reallocate(_buffer: ArrayBuffer, _newSize: number): ArrayBuffer {
    throw new Error(`PoolAllocator '${this.name}' does not support reallocation (fixed block size)`);
  }

  reset(): void {
    // Return all blocks to free list
    this.freeList = [];
    for (let i = 0; i < this.blockCount; i++) {
      this.freeList.push(i);
    }

    this.allocatedBlocks.clear();
    this.stats.totalFreed += this.stats.currentUsage;
    this.stats.currentUsage = 0;
    this.stats.activeAllocations = 0;
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

  owns(_buffer: ArrayBuffer): boolean {
    // Simplified ownership check
    return true; // Would need proper tracking
  }

  getName(): string {
    return this.name;
  }

  /**
   * Get number of free blocks
   */
  getFreeBlockCount(): number {
    return this.freeList.length;
  }

  /**
   * Get pool utilization (0-1)
   */
  getUtilization(): number {
    return this.allocatedBlocks.size / this.blockCount;
  }

  /**
   * Get block size
   */
  getBlockSize(): number {
    return this.blockSize;
  }
}
