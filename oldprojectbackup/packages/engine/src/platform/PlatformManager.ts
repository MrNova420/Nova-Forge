/**
 * @fileoverview Platform Manager - Auto-detection and initialization
 * @module @nova-engine/platform
 *
 * Automatically detects the current platform and provides the appropriate implementation.
 * Supports Web, Desktop (Electron/Tauri), and Mobile (React Native/Capacitor) platforms.
 */

import type { IPlatform } from './IPlatform';
import { WebPlatform } from './WebPlatform';
import { DesktopPlatform } from './DesktopPlatform';
import { MobilePlatform } from './MobilePlatform';

let currentPlatform: IPlatform | null = null;

/**
 * Detect if running in mobile environment
 */
function isMobile(): boolean {
  const ua = (globalThis as any).navigator?.userAgent || '';
  return /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(
    ua
  );
}

/**
 * Detect if running in desktop native environment
 */
function isDesktop(): boolean {
  // Check for Electron
  if (
    typeof (globalThis as any).process !== 'undefined' &&
    (globalThis as any).process.versions?.electron
  ) {
    return true;
  }

  // Check for Node.js (but not in browser with polyfills)
  if (
    typeof (globalThis as any).process !== 'undefined' &&
    (globalThis as any).process.versions?.node &&
    typeof window === 'undefined'
  ) {
    return true;
  }

  return false;
}

/**
 * Detect current platform and return appropriate implementation
 */
export function getPlatform(): IPlatform {
  if (currentPlatform) {
    return currentPlatform;
  }

  // Detect platform
  if (isDesktop()) {
    // Desktop platform (Electron/Tauri/Node.js)
    currentPlatform = new DesktopPlatform();
  } else if (isMobile()) {
    // Mobile platform (React Native/Capacitor/Mobile browser)
    currentPlatform = new MobilePlatform();
  } else if (typeof window !== 'undefined' && typeof document !== 'undefined') {
    // Web platform (browser)
    currentPlatform = new WebPlatform();
  } else {
    // Unknown platform - default to web
    console.warn('Unknown platform detected, defaulting to WebPlatform');
    currentPlatform = new WebPlatform();
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
