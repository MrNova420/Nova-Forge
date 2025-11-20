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
 * Common mathematical utility functions
 */
export class MathUtils {
  /**
   * The value of PI
   */
  static readonly PI = Math.PI;

  /**
   * The value of PI * 2
   */
  static readonly TWO_PI = Math.PI * 2;

  /**
   * The value of PI / 2
   */
  static readonly HALF_PI = Math.PI / 2;

  /**
   * Conversion factor from degrees to radians
   */
  static readonly DEG2RAD = Math.PI / 180;

  /**
   * Conversion factor from radians to degrees
   */
  static readonly RAD2DEG = 180 / Math.PI;

  /**
   * Small epsilon value for floating point comparisons
   */
  static readonly EPSILON = 0.000001;

  /**
   * Converts degrees to radians
   * @param degrees - Angle in degrees
   * @returns Angle in radians
   */
  static degToRad(degrees: number): number {
    return degrees * MathUtils.DEG2RAD;
  }

  /**
   * Converts radians to degrees
   * @param radians - Angle in radians
   * @returns Angle in degrees
   */
  static radToDeg(radians: number): number {
    return radians * MathUtils.RAD2DEG;
  }

  /**
   * Clamps a value between min and max
   * @param value - The value to clamp
   * @param min - Minimum value
   * @param max - Maximum value
   * @returns The clamped value
   */
  static clamp(value: number, min: number, max: number): number {
    return Math.max(min, Math.min(max, value));
  }

  /**
   * Linearly interpolates between two values
   * @param a - Start value
   * @param b - End value
   * @param t - Interpolation factor (0-1)
   * @returns The interpolated value
   */
  static lerp(a: number, b: number, t: number): number {
    return a + (b - a) * t;
  }

  /**
   * Inverse linear interpolation
   * @param a - Start value
   * @param b - End value
   * @param value - The value to find the factor for
   * @returns The interpolation factor
   */
  static inverseLerp(a: number, b: number, value: number): number {
    if (a === b) return 0;
    return (value - a) / (b - a);
  }

  /**
   * Remaps a value from one range to another
   * @param value - The value to remap
   * @param fromMin - Source range minimum
   * @param fromMax - Source range maximum
   * @param toMin - Target range minimum
   * @param toMax - Target range maximum
   * @returns The remapped value
   */
  static remap(
    value: number,
    fromMin: number,
    fromMax: number,
    toMin: number,
    toMax: number
  ): number {
    const t = MathUtils.inverseLerp(fromMin, fromMax, value);
    return MathUtils.lerp(toMin, toMax, t);
  }

  /**
   * Checks if two values are approximately equal
   * @param a - First value
   * @param b - Second value
   * @param epsilon - Tolerance for comparison
   * @returns True if values are approximately equal
   */
  static approximately(
    a: number,
    b: number,
    epsilon: number = MathUtils.EPSILON
  ): boolean {
    return Math.abs(a - b) <= epsilon;
  }

  /**
   * Returns the sign of a number
   * @param value - The value
   * @returns -1, 0, or 1
   */
  static sign(value: number): number {
    if (value > 0) return 1;
    if (value < 0) return -1;
    return 0;
  }

  /**
   * Snaps a value to the nearest step
   * @param value - The value to snap
   * @param step - The step size
   * @returns The snapped value
   */
  static snap(value: number, step: number): number {
    return Math.round(value / step) * step;
  }

  /**
   * Calculates the shortest difference between two angles
   * @param current - Current angle in radians
   * @param target - Target angle in radians
   * @returns The shortest angular difference
   */
  static deltaAngle(current: number, target: number): number {
    let delta = (target - current) % MathUtils.TWO_PI;
    if (delta > Math.PI) {
      delta -= MathUtils.TWO_PI;
    } else if (delta < -Math.PI) {
      delta += MathUtils.TWO_PI;
    }
    return delta;
  }

  /**
   * Smoothly interpolates between two values using smooth step
   * @param a - Start value
   * @param b - End value
   * @param t - Interpolation factor (0-1)
   * @returns The interpolated value
   */
  static smoothStep(a: number, b: number, t: number): number {
    t = MathUtils.clamp(t, 0, 1);
    t = t * t * (3 - 2 * t);
    return MathUtils.lerp(a, b, t);
  }

  /**
   * Smoothly interpolates using smoother step (quintic)
   * @param a - Start value
   * @param b - End value
   * @param t - Interpolation factor (0-1)
   * @returns The interpolated value
   */
  static smootherStep(a: number, b: number, t: number): number {
    t = MathUtils.clamp(t, 0, 1);
    t = t * t * t * (t * (t * 6 - 15) + 10);
    return MathUtils.lerp(a, b, t);
  }

  /**
   * Generates a pseudo-random number between 0 and 1
   * @returns A random number
   */
  static random(): number {
    return Math.random();
  }

  /**
   * Generates a random number between min and max
   * @param min - Minimum value
   * @param max - Maximum value
   * @returns A random number in the range
   */
  static randomRange(min: number, max: number): number {
    return min + Math.random() * (max - min);
  }

  /**
   * Generates a random integer between min (inclusive) and max (exclusive)
   * @param min - Minimum value
   * @param max - Maximum value
   * @returns A random integer
   */
  static randomInt(min: number, max: number): number {
    return Math.floor(MathUtils.randomRange(min, max));
  }

  /**
   * Checks if a value is a power of two
   * @param value - The value to check
   * @returns True if value is a power of two
   */
  static isPowerOfTwo(value: number): boolean {
    return value > 0 && (value & (value - 1)) === 0;
  }

  /**
   * Returns the next power of two greater than or equal to the value
   * @param value - The value
   * @returns The next power of two
   */
  static nextPowerOfTwo(value: number): number {
    if (value <= 0) return 1;
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return value + 1;
  }
}
