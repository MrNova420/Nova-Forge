/**
 * @fileoverview Platform Manager - Auto-detection and initialization
 * @module @nova-engine/platform
 *
 * Automatically detects the current platform and provides the appropriate implementation.
 */

import type { IPlatform } from './IPlatform';
import { WebPlatform } from './WebPlatform';

let currentPlatform: IPlatform | null = null;

/**
 * Detect current platform and return appropriate implementation
 */
export function getPlatform(): IPlatform {
  if (currentPlatform) {
    return currentPlatform;
  }

  // Detect platform
  if (typeof window !== 'undefined' && typeof document !== 'undefined') {
    // Web platform
    currentPlatform = new WebPlatform();
  } else if (typeof (globalThis as any).process !== 'undefined') {
    // Node.js/Electron/Desktop platform
    // TODO: Implement DesktopPlatform
    throw new Error('Desktop platform not yet implemented');
  } else {
    // Unknown platform
    throw new Error('Unknown platform - cannot initialize');
  }

  return currentPlatform;
}

/**
 * Initialize the platform with optional configuration
 */
export async function initializePlatform(
  config?: Record<string, unknown>
): Promise<IPlatform> {
  const platform = getPlatform();
  await platform.initialize(config);
  return platform;
}

/**
 * Shutdown the current platform
 */
export async function shutdownPlatform(): Promise<void> {
  if (currentPlatform) {
    await currentPlatform.shutdown();
    currentPlatform = null;
  }
}

/**
 * Reset platform (for testing)
 */
export function resetPlatform(): void {
  currentPlatform = null;
}
