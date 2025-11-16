/**
 * @fileoverview Animation system for updating all animators
 */

import { System } from '../ecs/System';
import { World } from '../ecs/World';
import { Animator } from './Animator';

/**
 * Animation system - updates all Animator components
 */
export class AnimationSystem extends System {
  /** Enable/disable animation updates */
  public enabled: boolean = true;

  constructor() {
    super();
    this.priority = 100; // Run after physics but before rendering
  }

  /**
   * Update all animators
   */
  public update(world: World, deltaTime: number): void {
    if (!this.enabled) return;

    // Get all entities
    const entities = world.getAllEntities();

    for (const entity of entities) {
      const animator = world.getComponent<Animator>(entity, Animator);
      if (animator) {
        animator.update(deltaTime);
      }
    }
  }
}
