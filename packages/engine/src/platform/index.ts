/**
 * @fileoverview Platform Abstraction Layer - Module Exports
 * @module @nova-engine/platform
 *
 * Provides platform abstraction for Nova Engine across web, desktop, and mobile.
 *
 * Phase 1.1 of the autonomous development guide.
 */

export * from './IPlatform';
export { WebPlatform } from './WebPlatform';

// Platform detection and initialization helpers
export { getPlatform, initializePlatform } from './PlatformManager';
