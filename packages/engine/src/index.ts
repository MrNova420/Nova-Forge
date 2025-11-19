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
 * Nova Engine - Core game engine runtime
 * @packageDocumentation
 */

export const ENGINE_VERSION = '0.1.0';

/**
 * Initialize the Nova Engine
 */
export function initializeEngine(): void {
  // Engine initialization logic will be implemented here
}

// Export math library
export * from './math';

// Export core systems
export * from './core';

// Export ECS architecture
export * from './ecs';

// Export graphics systems
export * from './graphics';

// Export scene management
export * from './scene';

// Export asset management
export * from './assets';

// Export input system
export * from './input';

// Export audio system
export * from './audio';

// Export physics system
export * from './physics';

// Export particle system
export * from './particles';

// Export animation system
export * from './animation';

// Export profiling system
export * from './profiling';

// Export platform abstraction layer
export * from './platform';

// Export job system
export * from './jobs';
