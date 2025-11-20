/**
 * @fileoverview Platform Abstraction Layer - Module Exports
 * @module @nova-engine/platform
 *
 * Provides platform abstraction for Nova Engine across web, desktop, and mobile.
 *
 * Phase 1.1 of the autonomous development guide - Complete platform implementations.
 */

export * from './IPlatform';
export { WebPlatform } from './WebPlatform';
export { DesktopPlatform } from './DesktopPlatform';
export { MobilePlatform } from './MobilePlatform';

// Platform detection and initialization helpers
export {
  getPlatform,
  initializePlatform,
  shutdownPlatform,
  resetPlatform,
} from './PlatformManager';
