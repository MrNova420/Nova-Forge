/**
 * @fileoverview Tests for build configuration
 */

import { describe, it, expect } from '@jest/globals';
import {
  BuildTarget,
  BuildMode,
  OptimizationLevel,
  DEFAULT_BUILD_CONFIG,
} from '../BuildConfig';

describe('BuildConfig', () => {
  it('should have correct default configuration', () => {
    expect(DEFAULT_BUILD_CONFIG.target).toBe(BuildTarget.Web);
    expect(DEFAULT_BUILD_CONFIG.mode).toBe(BuildMode.Production);
    expect(DEFAULT_BUILD_CONFIG.optimization).toBe(OptimizationLevel.Standard);
    expect(DEFAULT_BUILD_CONFIG.minify).toBe(true);
    expect(DEFAULT_BUILD_CONFIG.codeSplitting).toBe(true);
    expect(DEFAULT_BUILD_CONFIG.treeShaking).toBe(true);
  });

  it('should have correct build targets', () => {
    expect(BuildTarget.Web).toBe('web');
    expect(BuildTarget.Desktop).toBe('desktop');
    expect(BuildTarget.Mobile).toBe('mobile');
    expect(BuildTarget.Server).toBe('server');
  });

  it('should have correct build modes', () => {
    expect(BuildMode.Development).toBe('development');
    expect(BuildMode.Production).toBe('production');
  });

  it('should have correct optimization levels', () => {
    expect(OptimizationLevel.None).toBe(0);
    expect(OptimizationLevel.Basic).toBe(1);
    expect(OptimizationLevel.Standard).toBe(2);
    expect(OptimizationLevel.Aggressive).toBe(3);
  });
});
