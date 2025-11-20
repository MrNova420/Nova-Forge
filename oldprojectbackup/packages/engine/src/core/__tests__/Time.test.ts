import { Time } from '../Time';

describe('Time', () => {
  let time: Time;

  beforeEach(() => {
    time = new Time();
  });

  describe('initialization', () => {
    it('should start with zero values', () => {
      expect(time.deltaTime).toBe(0);
      expect(time.totalTime).toBe(0);
      expect(time.frameCount).toBe(0);
      expect(time.timeScale).toBe(1.0);
    });
  });

  describe('update', () => {
    it('should calculate delta time correctly', () => {
      time.update(1000);
      time.update(1016); // 16ms later = 60 FPS

      expect(time.deltaTime).toBeCloseTo(0.016, 3);
      expect(time.frameCount).toBe(1);
    });

    it('should accumulate total time', () => {
      time.update(1000);
      time.update(1016);
      time.update(1032);

      expect(time.totalTime).toBeCloseTo(0.032, 3);
      expect(time.frameCount).toBe(2);
    });

    it('should respect time scale', () => {
      time.timeScale = 0.5;
      time.update(1000);
      time.update(1100); // 100ms

      expect(time.deltaTime).toBeCloseTo(0.05, 3); // 0.1 * 0.5
      expect(time.totalTime).toBeCloseTo(0.05, 3);
    });

    it('should cap delta time to max value', () => {
      time.maxDeltaTime = 0.05; // 50ms max
      time.update(1000);
      time.update(1200); // 200ms jump

      expect(time.deltaTime).toBe(0.05);
    });
  });

  describe('fps', () => {
    it('should calculate FPS correctly', () => {
      time.update(1000);
      time.update(1016); // ~62.5 FPS

      expect(time.fps).toBeCloseTo(62.5, 0);
    });

    it('should return 0 FPS when deltaTime is 0', () => {
      expect(time.fps).toBe(0);
    });
  });

  describe('timeScale', () => {
    it('should not allow negative time scale', () => {
      time.timeScale = -1;
      expect(time.timeScale).toBe(0);
    });

    it('should affect delta time but not unscaled delta time', () => {
      time.timeScale = 2.0;
      time.update(1000);
      time.update(1050);

      expect(time.unscaledDeltaTime).toBeCloseTo(0.05, 3);
      expect(time.deltaTime).toBeCloseTo(0.1, 3);
    });
  });

  describe('reset', () => {
    it('should reset all time values', () => {
      time.update(1000);
      time.update(1016);
      time.reset();

      expect(time.deltaTime).toBe(0);
      expect(time.totalTime).toBe(0);
      expect(time.frameCount).toBe(0);
    });
  });
});
