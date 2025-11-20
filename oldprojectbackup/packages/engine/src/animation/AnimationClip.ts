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
 * @fileoverview Animation clip with keyframe tracks for property animation
 */

import { Vector3 } from '../math/Vector3';
import { Quaternion } from '../math/Quaternion';

/**
 * Interpolation types for animation keyframes
 */
export enum InterpolationType {
  /** Constant/step interpolation - no smoothing */
  STEP = 'step',
  /** Linear interpolation between keyframes */
  LINEAR = 'linear',
  /** Cubic/smooth interpolation for natural motion */
  CUBIC = 'cubic',
}

/**
 * Types of animation properties
 */
export enum AnimationPropertyType {
  /** Number/scalar property */
  NUMBER = 'number',
  /** Vector3 position/scale */
  VECTOR3 = 'vector3',
  /** Quaternion rotation */
  QUATERNION = 'quaternion',
  /** Color RGBA */
  COLOR = 'color',
}

/**
 * Single keyframe with time and value
 */
export interface Keyframe<T = unknown> {
  /** Time in seconds from start of animation */
  time: number;
  /** Value at this keyframe */
  value: T;
  /** Interpolation type to next keyframe */
  interpolation: InterpolationType;
  /** Optional cubic tangent for smooth curves (in) */
  tangentIn?: number;
  /** Optional cubic tangent for smooth curves (out) */
  tangentOut?: number;
}

/**
 * Animation track for a single property
 */
export class AnimationTrack<T = unknown> {
  /** Target property path (e.g., "transform.position.x") */
  public readonly property: string;
  /** Type of property being animated */
  public readonly propertyType: AnimationPropertyType;
  /** Keyframes sorted by time */
  private keyframes: Keyframe<T>[] = [];

  constructor(property: string, propertyType: AnimationPropertyType) {
    this.property = property;
    this.propertyType = propertyType;
  }

  /**
   * Add a keyframe to the track
   */
  public addKeyframe(
    time: number,
    value: T,
    interpolation: InterpolationType = InterpolationType.LINEAR,
  ): void {
    const keyframe: Keyframe<T> = { time, value, interpolation };

    // Insert sorted by time
    const index = this.keyframes.findIndex((kf) => kf.time > time);
    if (index === -1) {
      this.keyframes.push(keyframe);
    } else {
      this.keyframes.splice(index, 0, keyframe);
    }
  }

  /**
   * Remove keyframe at specific time
   */
  public removeKeyframe(time: number): boolean {
    const index = this.keyframes.findIndex((kf) => Math.abs(kf.time - time) < 0.001);
    if (index !== -1) {
      this.keyframes.splice(index, 1);
      return true;
    }
    return false;
  }

  /**
   * Get all keyframes
   */
  public getKeyframes(): Keyframe<T>[] {
    return this.keyframes;
  }

  /**
   * Get keyframe count
   */
  public getKeyframeCount(): number {
    return this.keyframes.length;
  }

  /**
   * Sample the track at a specific time
   */
  public sample(time: number): T | null {
    if (this.keyframes.length === 0) return null;
    if (this.keyframes.length === 1) {
      const first = this.keyframes[0];
      return first ? first.value : null;
    }

    // Find surrounding keyframes
    let prev: Keyframe<T> | undefined;
    let next: Keyframe<T> | undefined;

    for (let i = 0; i < this.keyframes.length; i++) {
      const kf = this.keyframes[i];
      if (kf && kf.time <= time) {
        prev = kf;
      }
      if (kf && kf.time >= time && !next) {
        next = kf;
        break;
      }
    }

    // Before first keyframe
    if (!prev && next) return next.value;

    // After last keyframe
    if (!next && prev) return prev.value;

    // No keyframes
    if (!prev || !next) return null;

    // Exact match
    if (Math.abs(prev.time - time) < 0.001) return prev.value;
    if (Math.abs(next.time - time) < 0.001) return next.value;

    // Interpolate
    const t = (time - prev.time) / (next.time - prev.time);
    return this.interpolate(prev, next, t);
  }

  /**
   * Interpolate between two keyframes
   */
  private interpolate(prev: Keyframe<T>, next: Keyframe<T>, t: number): T {
    switch (prev.interpolation) {
      case InterpolationType.STEP:
        return prev.value;

      case InterpolationType.LINEAR:
        return this.linearInterpolate(prev.value, next.value, t);

      case InterpolationType.CUBIC:
        return this.cubicInterpolate(prev.value, next.value, t);

      default:
        return prev.value;
    }
  }

  /**
   * Linear interpolation based on property type
   */
  private linearInterpolate(a: T, b: T, t: number): T {
    switch (this.propertyType) {
      case AnimationPropertyType.NUMBER:
        return ((a as number) * (1 - t) + (b as number) * t) as T;

      case AnimationPropertyType.VECTOR3: {
        const v1 = a as unknown as Vector3;
        const v2 = b as unknown as Vector3;
        return new Vector3(
          v1.x * (1 - t) + v2.x * t,
          v1.y * (1 - t) + v2.y * t,
          v1.z * (1 - t) + v2.z * t,
        ) as unknown as T;
      }

      case AnimationPropertyType.QUATERNION: {
        const q1 = a as unknown as Quaternion;
        const q2 = b as unknown as Quaternion;
        // Quaternion slerp (simplified linear for now)
        const result = new Quaternion(
          q1.x * (1 - t) + q2.x * t,
          q1.y * (1 - t) + q2.y * t,
          q1.z * (1 - t) + q2.z * t,
          q1.w * (1 - t) + q2.w * t,
        );
        result.normalize();
        return result as unknown as T;
      }

      case AnimationPropertyType.COLOR: {
        const c1 = a as unknown as number[];
        const c2 = b as unknown as number[];
        if (c1 && c2 && c1.length >= 4 && c2.length >= 4) {
          return [
            (c1[0] || 0) * (1 - t) + (c2[0] || 0) * t,
            (c1[1] || 0) * (1 - t) + (c2[1] || 0) * t,
            (c1[2] || 0) * (1 - t) + (c2[2] || 0) * t,
            (c1[3] || 0) * (1 - t) + (c2[3] || 0) * t,
          ] as unknown as T;
        }
        return a;
      }

      default:
        return a;
    }
  }

  /**
   * Cubic interpolation (Hermite spline)
   */
  private cubicInterpolate(a: T, b: T, t: number): T {
    // Smooth step function for cubic interpolation
    const smoothT = t * t * (3 - 2 * t);
    return this.linearInterpolate(a, b, smoothT);
  }
}

/**
 * Animation clip containing multiple property tracks
 */
export class AnimationClip {
  /** Clip name */
  public readonly name: string;
  /** Duration in seconds */
  public duration: number;
  /** Animation tracks */
  private tracks: Map<string, AnimationTrack> = new Map();
  /** Whether animation loops */
  public loop: boolean = false;

  constructor(name: string, duration: number = 0) {
    this.name = name;
    this.duration = duration;
  }

  /**
   * Add a track to the clip
   */
  public addTrack(track: AnimationTrack): void {
    this.tracks.set(track.property, track);

    // Update duration if needed
    const keyframes = track.getKeyframes();
    if (keyframes.length > 0) {
      const lastKeyframe = keyframes[keyframes.length - 1];
      if (lastKeyframe && lastKeyframe.time > this.duration) {
        this.duration = lastKeyframe.time;
      }
    }
  }

  /**
   * Get a track by property path
   */
  public getTrack(property: string): AnimationTrack | undefined {
    return this.tracks.get(property);
  }

  /**
   * Get all tracks
   */
  public getTracks(): AnimationTrack[] {
    return Array.from(this.tracks.values());
  }

  /**
   * Remove a track
   */
  public removeTrack(property: string): boolean {
    return this.tracks.delete(property);
  }

  /**
   * Sample all tracks at a specific time
   */
  public sample(time: number): Map<string, unknown> {
    const result = new Map<string, unknown>();

    // Clamp time to duration if not looping
    let sampleTime = time;
    if (this.loop && this.duration > 0) {
      sampleTime = time % this.duration;
    } else {
      sampleTime = Math.max(0, Math.min(time, this.duration));
    }

    // Sample each track
    for (const [property, track] of this.tracks) {
      const value = track.sample(sampleTime);
      if (value !== null) {
        result.set(property, value);
      }
    }

    return result;
  }

  /**
   * Create a copy of this clip
   */
  public clone(): AnimationClip {
    const clip = new AnimationClip(this.name, this.duration);
    clip.loop = this.loop;

    for (const track of this.tracks.values()) {
      const newTrack = new AnimationTrack(track.property, track.propertyType);
      for (const keyframe of track.getKeyframes()) {
        newTrack.addKeyframe(keyframe.time, keyframe.value, keyframe.interpolation);
      }
      clip.addTrack(newTrack);
    }

    return clip;
  }
}
