/**
 * @fileoverview Memory Management - Module Exports
 * @module @nova-engine/memory
 *
 * AAA-grade custom memory allocation system for Nova Engine.
 * Phase 1.3 of the autonomous development guide.
 *
 * Production-ready allocators with industry-leading features:
 * - Linear Allocator: Ultra-fast frame allocations
 * - Pool Allocator: Fixed-size object pooling
 * - Stack Allocator: Scoped hierarchical allocations with markers
 * - General Allocator: Advanced general-purpose with defragmentation
 */

export * from './IAllocator';
export { LinearAllocator } from './LinearAllocator';
export { PoolAllocator } from './PoolAllocator';
export { StackAllocator } from './StackAllocator';
export { GeneralAllocator, type MemoryCategory } from './GeneralAllocator';
