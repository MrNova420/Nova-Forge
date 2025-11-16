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
