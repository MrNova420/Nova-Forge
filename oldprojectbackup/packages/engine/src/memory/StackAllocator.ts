/**
 * @fileoverview Stack Allocator - Production Grade
 * @module @nova-engine/memory
 *
 * Advanced stack-based allocator with markers and scoped allocations.
 * Industry-grade implementation with comprehensive error handling and diagnostics.
 *
 * Following the autonomous development guide Phase 1.3.2
 * Inspired by Unreal Engine's FMemStack and Unity's TempAllocator
 */

import {
  type IAllocator,
  type AllocatorConfig,
  AllocationFlags,
  type MemoryStats,
  type AllocationInfo,
} from './IAllocator';

/**
 * Stack marker for nested allocation scopes
 */
export interface StackMarker {
  /** Marker ID */
  id: string;
  /** Stack offset at marker creation */
  offset: number;
  /** Timestamp when marker was created */
  timestamp: number;
  /** Number of allocations since marker */
  allocationCount: number;
}

/**
 * Stack Allocator - Production Grade
 *
 * LIFO (Last-In-First-Out) allocator with marker support.
 * Provides stack-based memory allocation with scoped lifetime management.
 *
 * Features:
 * - O(1) allocation and deallocation (in correct order)
 * - Marker-based scoped allocation
 * - Nested scope support
 * - Comprehensive leak detection
 * - Detailed allocation tracking
 * - Guard page protection (debug mode)
 * - Memory statistics and profiling
 * - Double-free detection
 * - Buffer overflow detection
 *
 * Perfect for:
 * - Hierarchical allocations (scene graphs, UI trees)
 * - Temporary calculations
 * - String building
 * - Recursive algorithms
 *
 * @example
 * ```typescript
 * const allocator = new StackAllocator({
 *   name: 'Stack',
 *   capacity: 1024 * 1024,
 *   trackAllocations: true,
 *   detectLeaks: true,
 * });
 *
 * // Create marker
 * const marker = allocator.pushMarker('Scene Load');
 *
 * // Allocate within scope
 * const buffer1 = allocator.allocate(256);
 * const buffer2 = allocator.allocate(512);
 *
 * // Pop back to marker (frees buffer1 and buffer2)
 * allocator.popMarker(marker);
 * ```
 */
export class StackAllocator implements IAllocator {
  private name: string;
  private capacity: number;
  private memory: ArrayBuffer;
  private offset = 0;
  private markers: StackMarker[] = [];
  private trackAllocations: boolean;
  private detectLeaks: boolean;
  private allocations: Map<ArrayBuffer, AllocationInfo> = new Map();
  private freedBuffers: Set<ArrayBuffer> = new Set();
  private stats: MemoryStats;
  private defaultAlignment: number;
  private enableGuardPages: boolean;
  private guardPageSize = 16; // bytes
  private guardPattern = 0xDEADBEEF;

  constructor(config: AllocatorConfig & { enableGuardPages?: boolean }) {
    this.name = config.name;
    this.capacity = config.capacity || 1024 * 1024 * 4; // 4MB default
    this.trackAllocations = config.trackAllocations ?? true;
    this.detectLeaks = config.detectLeaks ?? true;
    this.defaultAlignment = config.defaultAlignment || 16;
    this.enableGuardPages = config.enableGuardPages ?? false;

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

    if (this.enableGuardPages) {
      console.log(`StackAllocator '${this.name}': Guard pages enabled (${this.guardPageSize} bytes)`);
    }
  }

  allocate(
    size: number,
    alignment?: number,
    flags: AllocationFlags = 0
  ): ArrayBuffer {
    // Validate input
    if (size <= 0) {
      throw new Error(`StackAllocator '${this.name}': Invalid allocation size ${size}`);
    }

    const align = alignment || this.defaultAlignment;
    if (!this.isPowerOfTwo(align)) {
      throw new Error(`StackAllocator '${this.name}': Alignment must be power of 2, got ${align}`);
    }

    // Calculate aligned offset
    let alignedOffset = this.alignUp(this.offset, align);

    // Add guard page before allocation (if enabled)
    if (this.enableGuardPages) {
      alignedOffset += this.guardPageSize;
    }

    // Calculate total size including guard pages
    const totalSize = this.enableGuardPages
      ? size + this.guardPageSize * 2
      : size;

    // Check capacity
    if (alignedOffset + totalSize > this.capacity) {
      throw new Error(
        `StackAllocator '${this.name}' out of memory:\n` +
        `  Requested: ${size} bytes (${totalSize} with guards)\n` +
        `  Available: ${this.capacity - alignedOffset} bytes\n` +
        `  Current usage: ${this.offset}/${this.capacity} (${(this.offset / this.capacity * 100).toFixed(1)}%)\n` +
        `  Peak usage: ${this.stats.peakUsage} bytes\n` +
        `  Active allocations: ${this.stats.activeAllocations}`
      );
    }

    // Write guard page before (if enabled)
    if (this.enableGuardPages) {
      this.writeGuardPage(alignedOffset - this.guardPageSize);
    }

    // Create buffer view
    const buffer = this.memory.slice(alignedOffset, alignedOffset + size);

    // Zero-initialize if requested
    if (flags & AllocationFlags.ZERO_INIT) {
      new Uint8Array(buffer).fill(0);
    }

    // Write guard page after (if enabled)
    if (this.enableGuardPages) {
      this.writeGuardPage(alignedOffset + size);
    }

    // Update offset
    this.offset = alignedOffset + totalSize;

    // Track allocation
    if (this.trackAllocations) {
      const info: AllocationInfo = {
        address: alignedOffset,
        size,
        alignment: align,
        timestamp: performance.now(),
        callStack: this.detectLeaks ? this.captureCallStack() : undefined,
      };
      this.allocations.set(buffer, info);
    }

    // Update marker allocation count
    if (this.markers.length > 0) {
      this.markers[this.markers.length - 1].allocationCount++;
    }

    // Update statistics
    this.stats.totalAllocated += size;
    this.stats.currentUsage = this.offset;
    this.stats.peakUsage = Math.max(this.stats.peakUsage, this.offset);
    this.stats.allocationCount++;
    this.stats.activeAllocations++;

    return buffer;
  }

  free(buffer: ArrayBuffer): void {
    // Validate buffer
    if (!this.allocations.has(buffer)) {
      throw new Error(`StackAllocator '${this.name}': Attempting to free unknown buffer`);
    }

    // Check for double-free
    if (this.freedBuffers.has(buffer)) {
      throw new Error(`StackAllocator '${this.name}': Double-free detected!`);
    }

    const info = this.allocations.get(buffer)!;

    // Verify guard pages (if enabled)
    if (this.enableGuardPages) {
      this.verifyGuardPages(info.address, info.size);
    }

    // Stack allocator requires LIFO freeing
    const expectedOffset = this.enableGuardPages
      ? info.address + info.size + this.guardPageSize
      : info.address + info.size;

    if (expectedOffset !== this.offset) {
      console.warn(
        `StackAllocator '${this.name}': Out-of-order free detected!\n` +
        `  Expected offset: ${expectedOffset}\n` +
        `  Current offset: ${this.offset}\n` +
        `  This may indicate incorrect deallocation order.`
      );
    }

    // Roll back offset
    this.offset = this.enableGuardPages
      ? info.address - this.guardPageSize
      : info.address;

    // Mark as freed
    this.freedBuffers.add(buffer);
    this.allocations.delete(buffer);

    // Update stats
    this.stats.totalFreed += info.size;
    this.stats.currentUsage = this.offset;
    this.stats.deallocationCount++;
    this.stats.activeAllocations--;
  }

  reallocate(buffer: ArrayBuffer, newSize: number): ArrayBuffer {
    const info = this.allocations.get(buffer);
    if (!info) {
      throw new Error(`StackAllocator '${this.name}': Cannot reallocate unknown buffer`);
    }

    // Check if this is the top allocation
    const expectedOffset = this.enableGuardPages
      ? info.address + info.size + this.guardPageSize
      : info.address + info.size;

    if (expectedOffset === this.offset) {
      // Top allocation - can extend in place
      const sizeDiff = newSize - info.size;
      const newOffset = this.offset + sizeDiff;

      if (newOffset > this.capacity) {
        throw new Error(
          `StackAllocator '${this.name}': Cannot extend allocation beyond capacity`
        );
      }

      // Update guard page if growing
      if (this.enableGuardPages && newSize > info.size) {
        this.writeGuardPage(info.address + newSize);
      }

      this.offset = newOffset;
      info.size = newSize;
      this.stats.currentUsage = this.offset;

      return this.memory.slice(info.address, info.address + newSize);
    } else {
      // Not top allocation - need to allocate new and copy
      const newBuffer = this.allocate(newSize, info.alignment);
      const copySize = Math.min(info.size, newSize);
      new Uint8Array(newBuffer).set(new Uint8Array(buffer, 0, copySize));
      return newBuffer;
    }
  }

  /**
   * Push a marker for scoped allocations
   */
  pushMarker(label?: string): StackMarker {
    const marker: StackMarker = {
      id: `marker-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`,
      offset: this.offset,
      timestamp: performance.now(),
      allocationCount: 0,
    };

    this.markers.push(marker);

    if (label && this.trackAllocations) {
      console.log(`StackAllocator '${this.name}': Pushed marker '${label}' at offset ${this.offset}`);
    }

    return marker;
  }

  /**
   * Pop back to a marker (frees all allocations since marker)
   */
  popMarker(marker: StackMarker): void {
    const index = this.markers.findIndex(m => m.id === marker.id);
    if (index === -1) {
      throw new Error(`StackAllocator '${this.name}': Invalid or already popped marker`);
    }

    // Check for out-of-order pop
    if (index !== this.markers.length - 1) {
      throw new Error(
        `StackAllocator '${this.name}': Out-of-order marker pop!\n` +
        `  Attempting to pop marker at index ${index}, but stack has ${this.markers.length} markers.\n` +
        `  Markers must be popped in LIFO order.`
      );
    }

    // Calculate freed memory
    const freedBytes = this.offset - marker.offset;

    // Roll back to marker
    this.offset = marker.offset;

    // Remove allocations after marker
    for (const [buffer, info] of this.allocations) {
      if (info.address >= marker.offset) {
        this.allocations.delete(buffer);
        this.stats.activeAllocations--;
      }
    }

    this.markers.pop();

    // Update stats
    this.stats.totalFreed += freedBytes;
    this.stats.currentUsage = this.offset;

    if (this.trackAllocations) {
      const elapsedMs = performance.now() - marker.timestamp;
      console.log(
        `StackAllocator '${this.name}': Popped marker (freed ${freedBytes} bytes, ${marker.allocationCount} allocations, ${elapsedMs.toFixed(2)}ms)`
      );
    }
  }

  /**
   * Get current marker (for savepoint pattern)
   */
  getCurrentMarker(): StackMarker {
    return this.pushMarker();
  }

  /**
   * Reset to marker without popping (for rollback)
   */
  rewindToMarker(marker: StackMarker): void {
    if (marker.offset > this.offset) {
      throw new Error(`StackAllocator '${this.name}': Cannot rewind to future marker`);
    }

    const freedBytes = this.offset - marker.offset;
    this.offset = marker.offset;
    this.stats.totalFreed += freedBytes;
    this.stats.currentUsage = this.offset;

    // Clean up allocations
    for (const [buffer, info] of this.allocations) {
      if (info.address >= marker.offset) {
        this.allocations.delete(buffer);
        this.stats.activeAllocations--;
      }
    }
  }

  reset(): void {
    // Check for leaks
    if (this.detectLeaks && this.allocations.size > 0) {
      console.error(
        `StackAllocator '${this.name}': Memory leak detected!\n` +
        `  ${this.allocations.size} allocations not freed before reset\n` +
        `  Total leaked: ${Array.from(this.allocations.values()).reduce((sum, info) => sum + info.size, 0)} bytes`
      );

      if (this.trackAllocations) {
        console.log('Leaked allocations:');
        for (const [_, info] of this.allocations) {
          console.log(`  - ${info.size} bytes at offset ${info.address}`);
          if (info.callStack) {
            console.log(`    Call stack: ${info.callStack}`);
          }
        }
      }
    }

    this.offset = 0;
    this.markers = [];
    this.allocations.clear();
    this.freedBuffers.clear();
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

  owns(buffer: ArrayBuffer): boolean {
    return this.allocations.has(buffer);
  }

  getName(): string {
    return this.name;
  }

  /**
   * Get detailed allocation report
   */
  getDetailedReport(): string {
    let report = `\n=== StackAllocator '${this.name}' Report ===\n\n`;
    
    report += `Memory:\n`;
    report += `  Capacity: ${this.formatBytes(this.capacity)}\n`;
    report += `  Current usage: ${this.formatBytes(this.offset)} (${(this.offset / this.capacity * 100).toFixed(1)}%)\n`;
    report += `  Peak usage: ${this.formatBytes(this.stats.peakUsage)} (${(this.stats.peakUsage / this.capacity * 100).toFixed(1)}%)\n`;
    report += `  Available: ${this.formatBytes(this.capacity - this.offset)}\n\n`;

    report += `Statistics:\n`;
    report += `  Total allocated: ${this.formatBytes(this.stats.totalAllocated)}\n`;
    report += `  Total freed: ${this.formatBytes(this.stats.totalFreed)}\n`;
    report += `  Allocation count: ${this.stats.allocationCount}\n`;
    report += `  Deallocation count: ${this.stats.deallocationCount}\n`;
    report += `  Active allocations: ${this.stats.activeAllocations}\n`;
    report += `  Markers active: ${this.markers.length}\n\n`;

    if (this.allocations.size > 0) {
      report += `Active Allocations:\n`;
      for (const [_, info] of this.allocations) {
        report += `  - ${this.formatBytes(info.size)} at offset ${info.address} (align: ${info.alignment})\n`;
      }
    }

    return report;
  }

  // Private helper methods

  private writeGuardPage(offset: number): void {
    const view = new DataView(this.memory);
    for (let i = 0; i < this.guardPageSize; i += 4) {
      view.setUint32(offset + i, this.guardPattern, true);
    }
  }

  private verifyGuardPages(address: number, size: number): void {
    const view = new DataView(this.memory);

    // Check front guard
    for (let i = 0; i < this.guardPageSize; i += 4) {
      const value = view.getUint32(address - this.guardPageSize + i, true);
      if (value !== this.guardPattern) {
        throw new Error(
          `StackAllocator '${this.name}': Buffer underrun detected!\n` +
          `  Allocation at ${address}, size ${size}\n` +
          `  Front guard page corrupted at byte ${i}`
        );
      }
    }

    // Check back guard
    for (let i = 0; i < this.guardPageSize; i += 4) {
      const value = view.getUint32(address + size + i, true);
      if (value !== this.guardPattern) {
        throw new Error(
          `StackAllocator '${this.name}': Buffer overrun detected!\n` +
          `  Allocation at ${address}, size ${size}\n` +
          `  Back guard page corrupted at byte ${i}`
        );
      }
    }
  }

  private alignUp(value: number, alignment: number): number {
    return Math.ceil(value / alignment) * alignment;
  }

  private isPowerOfTwo(n: number): boolean {
    return n > 0 && (n & (n - 1)) === 0;
  }

  private captureCallStack(): string {
    const stack = new Error().stack || '';
    return stack.split('\n').slice(3, 8).join('\n');
  }

  private formatBytes(bytes: number): string {
    if (bytes < 1024) return `${bytes} B`;
    if (bytes < 1024 * 1024) return `${(bytes / 1024).toFixed(2)} KB`;
    return `${(bytes / (1024 * 1024)).toFixed(2)} MB`;
  }
}
