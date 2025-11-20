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
 * @fileoverview Animation mixer for blending multiple animations
 */

import { AnimationClip } from './AnimationClip';

/**
 * Active animation playback info
 */
interface ActiveAnimation {
  /** Animation clip */
  clip: AnimationClip;
  /** Current playback time */
  time: number;
  /** Playback speed multiplier */
  speed: number;
  /** Blend weight (0-1) */
  weight: number;
  /** Whether animation is playing */
  isPlaying: boolean;
  /** Loop mode */
  loop: boolean;
  /** Fade in/out state */
  fadeDirection: number; // -1: fade out, 0: stable, 1: fade in
  /** Fade speed */
  fadeSpeed: number;
}

/**
 * Animation mixer - blends multiple animations simultaneously
 * Allows layered animation playback with weights
 */
export class AnimationMixer {
  /** Currently active animations */
  private activeAnimations: Map<string, ActiveAnimation> = new Map();
  /** Default fade duration */
  public defaultFadeDuration: number = 0.3;

  /**
   * Play an animation with optional fade in
   */
  public play(
    name: string,
    clip: AnimationClip,
    fadeDuration: number = this.defaultFadeDuration,
    loop: boolean = clip.loop,
    weight: number = 1.0,
  ): void {
    const existing = this.activeAnimations.get(name);

    if (existing) {
      // Restart existing animation
      existing.time = 0;
      existing.isPlaying = true;
      existing.loop = loop;
      existing.fadeDirection = fadeDuration > 0 ? 1 : 0;
      existing.fadeSpeed = fadeDuration > 0 ? 1.0 / fadeDuration : 0;
      existing.weight = fadeDuration > 0 ? 0 : weight;
    } else {
      // Create new animation
      const anim: ActiveAnimation = {
        clip,
        time: 0,
        speed: 1.0,
        weight: fadeDuration > 0 ? 0 : weight,
        isPlaying: true,
        loop,
        fadeDirection: fadeDuration > 0 ? 1 : 0,
        fadeSpeed: fadeDuration > 0 ? 1.0 / fadeDuration : 0,
      };
      this.activeAnimations.set(name, anim);
    }
  }

  /**
   * Stop an animation with optional fade out
   */
  public stop(name: string, fadeDuration: number = this.defaultFadeDuration): boolean {
    const anim = this.activeAnimations.get(name);
    if (!anim) return false;

    if (fadeDuration > 0) {
      anim.fadeDirection = -1;
      anim.fadeSpeed = 1.0 / fadeDuration;
    } else {
      this.activeAnimations.delete(name);
    }

    return true;
  }

  /**
   * Pause an animation
   */
  public pause(name: string): boolean {
    const anim = this.activeAnimations.get(name);
    if (!anim) return false;

    anim.isPlaying = false;
    return true;
  }

  /**
   * Resume an animation
   */
  public resume(name: string): boolean {
    const anim = this.activeAnimations.get(name);
    if (!anim) return false;

    anim.isPlaying = true;
    return true;
  }

  /**
   * Set animation weight
   */
  public setWeight(name: string, weight: number): boolean {
    const anim = this.activeAnimations.get(name);
    if (!anim) return false;

    anim.weight = Math.max(0, Math.min(1, weight));
    return true;
  }

  /**
   * Set animation speed
   */
  public setSpeed(name: string, speed: number): boolean {
    const anim = this.activeAnimations.get(name);
    if (!anim) return false;

    anim.speed = speed;
    return true;
  }

  /**
   * Get animation time
   */
  public getTime(name: string): number | null {
    const anim = this.activeAnimations.get(name);
    return anim ? anim.time : null;
  }

  /**
   * Set animation time
   */
  public setTime(name: string, time: number): boolean {
    const anim = this.activeAnimations.get(name);
    if (!anim) return false;

    anim.time = time;
    return true;
  }

  /**
   * Check if animation is playing
   */
  public isPlaying(name: string): boolean {
    const anim = this.activeAnimations.get(name);
    return anim ? anim.isPlaying : false;
  }

  /**
   * Stop all animations
   */
  public stopAll(fadeDuration: number = this.defaultFadeDuration): void {
    const names = Array.from(this.activeAnimations.keys());
    for (const name of names) {
      this.stop(name, fadeDuration);
    }
  }

  /**
   * Update all animations
   */
  public update(deltaTime: number): void {
    const toRemove: string[] = [];

    for (const [name, anim] of this.activeAnimations) {
      if (!anim.isPlaying && anim.fadeDirection === 0) continue;

      // Update time
      if (anim.isPlaying) {
        anim.time += deltaTime * anim.speed;

        // Handle looping
        if (anim.loop && anim.clip.duration > 0) {
          anim.time = anim.time % anim.clip.duration;
        } else if (anim.time >= anim.clip.duration) {
          anim.time = anim.clip.duration;
          anim.isPlaying = false;

          // Start fade out after reaching end
          if (anim.fadeDirection === 0) {
            anim.fadeDirection = -1;
            anim.fadeSpeed = 1.0 / this.defaultFadeDuration;
          }
        }
      }

      // Update weight (fading)
      if (anim.fadeDirection !== 0) {
        anim.weight += anim.fadeDirection * anim.fadeSpeed * deltaTime;

        if (anim.fadeDirection > 0 && anim.weight >= 1.0) {
          // Fade in complete
          anim.weight = 1.0;
          anim.fadeDirection = 0;
        } else if (anim.fadeDirection < 0 && anim.weight <= 0) {
          // Fade out complete
          toRemove.push(name);
        }
      }
    }

    // Remove fully faded out animations
    for (const name of toRemove) {
      this.activeAnimations.delete(name);
    }
  }

  /**
   * Sample all active animations with blending
   */
  public sample(): Map<string, unknown> {
    const result = new Map<string, unknown>();
    const propertyWeights = new Map<string, number>();

    // Sample each active animation
    for (const anim of this.activeAnimations.values()) {
      if (anim.weight <= 0) continue;

      const samples = anim.clip.sample(anim.time);

      for (const [property, value] of samples) {
        const existingValue = result.get(property);
        const existingWeight = propertyWeights.get(property) || 0;

        if (existingValue === undefined) {
          // First value for this property
          result.set(property, value);
          propertyWeights.set(property, anim.weight);
        } else {
          // Blend with existing value
          const blended = this.blendValues(existingValue, value, existingWeight, anim.weight);
          result.set(property, blended);
          propertyWeights.set(property, existingWeight + anim.weight);
        }
      }
    }

    // Normalize weights
    for (const [property, totalWeight] of propertyWeights) {
      if (totalWeight > 1.0) {
        const value = result.get(property);
        if (typeof value === 'number') {
          result.set(property, value / totalWeight);
        }
      }
    }

    return result;
  }

  /**
   * Blend two values
   */
  private blendValues(
    value1: unknown,
    value2: unknown,
    weight1: number,
    weight2: number,
  ): unknown {
    // Simple numeric blending
    if (typeof value1 === 'number' && typeof value2 === 'number') {
      return value1 * weight1 + value2 * weight2;
    }

    // For complex types, return based on higher weight
    return weight1 >= weight2 ? value1 : value2;
  }

  /**
   * Get number of active animations
   */
  public getActiveCount(): number {
    return this.activeAnimations.size;
  }

  /**
   * Clear all animations
   */
  public clear(): void {
    this.activeAnimations.clear();
  }
}
