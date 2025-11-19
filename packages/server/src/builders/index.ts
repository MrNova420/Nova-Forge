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
 * @fileoverview Build system exports
 */

export {
  BuildTarget,
  BuildMode,
  OptimizationLevel,
  DEFAULT_BUILD_CONFIG,
} from './BuildConfig';

export type {
  BuildConfig,
  BuildResult,
  BuildArtifact,
  BuildStats,
  ChunkInfo,
} from './BuildConfig';

export { WebBuilder } from './WebBuilder';

export { DesktopBuilder, DesktopPlatform } from './DesktopBuilder';

export type { DesktopBuildConfig } from './DesktopBuilder';

export { MobileBuilder, MobilePlatform } from './MobileBuilder';

export type { MobileBuildConfig } from './MobileBuilder';

export { ServerBuilder, ServerPlatform } from './ServerBuilder';

export type { ServerBuildConfig } from './ServerBuilder';

export { BuildManager, BuildStatus } from './BuildManager';

export type { BuildRequest, BuildJob } from './BuildManager';
