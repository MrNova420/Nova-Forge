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
 * Desktop Services - Export all desktop-specific services
 */

export * from './LocalGameStorage';
export * from './LocalSaveData';
export * from './OfflineSync';
export * from './UpdateManager';
export * from './GameRuntime';
export * from './PerformanceMonitor';
export * from './MediaCapture';
export * from './ControllerConfig';

// Initialize all services
import { localGameStorage } from './LocalGameStorage';
import { localSaveData } from './LocalSaveData';
import { offlineSync } from './OfflineSync';
import { updateManager } from './UpdateManager';
import { mediaCapture } from './MediaCapture';
import { controllerConfig } from './ControllerConfig';
import { gameRuntime } from './GameRuntime';

/**
 * Initialize all desktop services
 */
export async function initializeServices(): Promise<void> {
  // eslint-disable-next-line no-console
  console.log('Initializing Nova Launcher Desktop services...');

  try {
    // Initialize in order
    await localGameStorage.initialize();
    await localSaveData.initialize();
    await offlineSync.initialize();
    await updateManager.initialize();
    await mediaCapture.initialize();
    await controllerConfig.initialize();

    // eslint-disable-next-line no-console
    console.log('All services initialized successfully');
  } catch (error) {
    console.error('Failed to initialize services:', error);
    throw error;
  }
}

/**
 * Cleanup all services
 */
export function cleanupServices(): void {
  offlineSync.stopBackgroundSync();
  updateManager.stopUpdateChecks();
  controllerConfig.cleanup();
  gameRuntime.cleanup();
  // eslint-disable-next-line no-console
  console.log('Services cleaned up');
}
