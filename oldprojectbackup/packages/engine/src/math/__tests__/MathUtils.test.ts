import { MathUtils } from '../MathUtils';

describe('MathUtils', () => {
  describe('constants', () => {
    it('should have correct constants', () => {
      expect(MathUtils.PI).toBe(Math.PI);
      expect(MathUtils.TWO_PI).toBe(Math.PI * 2);
      expect(MathUtils.HALF_PI).toBe(Math.PI / 2);
      expect(MathUtils.DEG2RAD).toBeCloseTo(0.0174533, 5);
      expect(MathUtils.RAD2DEG).toBeCloseTo(57.2958, 4);
    });
  });

  describe('degToRad', () => {
    it('should convert degrees to radians', () => {
      expect(MathUtils.degToRad(180)).toBeCloseTo(Math.PI, 5);
      expect(MathUtils.degToRad(90)).toBeCloseTo(Math.PI / 2, 5);
      expect(MathUtils.degToRad(0)).toBe(0);
    });
  });

  describe('radToDeg', () => {
    it('should convert radians to degrees', () => {
      expect(MathUtils.radToDeg(Math.PI)).toBeCloseTo(180, 4);
      expect(MathUtils.radToDeg(Math.PI / 2)).toBeCloseTo(90, 4);
      expect(MathUtils.radToDeg(0)).toBe(0);
    });
  });

  describe('clamp', () => {
    it('should clamp values within range', () => {
      expect(MathUtils.clamp(5, 0, 10)).toBe(5);
      expect(MathUtils.clamp(-5, 0, 10)).toBe(0);
      expect(MathUtils.clamp(15, 0, 10)).toBe(10);
    });
  });

  describe('lerp', () => {
    it('should interpolate between values', () => {
      expect(MathUtils.lerp(0, 10, 0)).toBe(0);
      expect(MathUtils.lerp(0, 10, 0.5)).toBe(5);
      expect(MathUtils.lerp(0, 10, 1)).toBe(10);
    });
  });

  describe('inverseLerp', () => {
    it('should find interpolation factor', () => {
      expect(MathUtils.inverseLerp(0, 10, 0)).toBe(0);
      expect(MathUtils.inverseLerp(0, 10, 5)).toBe(0.5);
      expect(MathUtils.inverseLerp(0, 10, 10)).toBe(1);
    });
  });

  describe('remap', () => {
    it('should remap values from one range to another', () => {
      expect(MathUtils.remap(5, 0, 10, 0, 100)).toBe(50);
      expect(MathUtils.remap(0, 0, 10, 0, 100)).toBe(0);
      expect(MathUtils.remap(10, 0, 10, 0, 100)).toBe(100);
    });
  });

  describe('approximately', () => {
    it('should check if values are approximately equal', () => {
      expect(MathUtils.approximately(1.0, 1.0000001)).toBe(true);
      expect(MathUtils.approximately(1.0, 1.1)).toBe(false);
    });
  });

  describe('sign', () => {
    it('should return sign of number', () => {
      expect(MathUtils.sign(5)).toBe(1);
      expect(MathUtils.sign(-5)).toBe(-1);
      expect(MathUtils.sign(0)).toBe(0);
    });
  });

  describe('snap', () => {
    it('should snap to nearest step', () => {
      expect(MathUtils.snap(7, 5)).toBe(5);
      expect(MathUtils.snap(8, 5)).toBe(10);
      expect(MathUtils.snap(12.5, 5)).toBe(15);
    });
  });

  describe('deltaAngle', () => {
    it('should calculate shortest angular difference', () => {
      const result = MathUtils.deltaAngle(0, Math.PI);
      expect(Math.abs(result)).toBeCloseTo(Math.PI, 5);
    });
  });

  describe('smoothStep', () => {
    it('should smoothly interpolate', () => {
      expect(MathUtils.smoothStep(0, 10, 0)).toBe(0);
      expect(MathUtils.smoothStep(0, 10, 0.5)).toBe(5);
      expect(MathUtils.smoothStep(0, 10, 1)).toBe(10);
    });
  });

  describe('randomRange', () => {
    it('should generate random number in range', () => {
      for (let i = 0; i < 100; i++) {
        const result = MathUtils.randomRange(5, 10);
        expect(result).toBeGreaterThanOrEqual(5);
        expect(result).toBeLessThan(10);
      }
    });
  });

  describe('randomInt', () => {
    it('should generate random integer in range', () => {
      for (let i = 0; i < 100; i++) {
        const result = MathUtils.randomInt(5, 10);
        expect(result).toBeGreaterThanOrEqual(5);
        expect(result).toBeLessThan(10);
        expect(Number.isInteger(result)).toBe(true);
      }
    });
  });

  describe('isPowerOfTwo', () => {
    it('should check if number is power of two', () => {
      expect(MathUtils.isPowerOfTwo(1)).toBe(true);
      expect(MathUtils.isPowerOfTwo(2)).toBe(true);
      expect(MathUtils.isPowerOfTwo(4)).toBe(true);
      expect(MathUtils.isPowerOfTwo(8)).toBe(true);
      expect(MathUtils.isPowerOfTwo(3)).toBe(false);
      expect(MathUtils.isPowerOfTwo(5)).toBe(false);
    });
  });

  describe('nextPowerOfTwo', () => {
    it('should find next power of two', () => {
      expect(MathUtils.nextPowerOfTwo(3)).toBe(4);
      expect(MathUtils.nextPowerOfTwo(5)).toBe(8);
      expect(MathUtils.nextPowerOfTwo(8)).toBe(8);
      expect(MathUtils.nextPowerOfTwo(9)).toBe(16);
    });
  });
});
