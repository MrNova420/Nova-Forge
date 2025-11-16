import { Vector3 } from '../Vector3';

describe('Vector3', () => {
  describe('constructor', () => {
    it('should create a zero vector by default', () => {
      const v = new Vector3();
      expect(v.x).toBe(0);
      expect(v.y).toBe(0);
      expect(v.z).toBe(0);
    });

    it('should create a vector with given values', () => {
      const v = new Vector3(1, 2, 3);
      expect(v.x).toBe(1);
      expect(v.y).toBe(2);
      expect(v.z).toBe(3);
    });
  });

  describe('static constructors', () => {
    it('should create zero vector', () => {
      const v = Vector3.zero();
      expect(v.x).toBe(0);
      expect(v.y).toBe(0);
      expect(v.z).toBe(0);
    });

    it('should create one vector', () => {
      const v = Vector3.one();
      expect(v.x).toBe(1);
      expect(v.y).toBe(1);
      expect(v.z).toBe(1);
    });

    it('should create direction vectors', () => {
      expect(Vector3.up()).toEqual(new Vector3(0, 1, 0));
      expect(Vector3.down()).toEqual(new Vector3(0, -1, 0));
      expect(Vector3.left()).toEqual(new Vector3(-1, 0, 0));
      expect(Vector3.right()).toEqual(new Vector3(1, 0, 0));
      expect(Vector3.forward()).toEqual(new Vector3(0, 0, 1));
      expect(Vector3.backward()).toEqual(new Vector3(0, 0, -1));
    });
  });

  describe('add', () => {
    it('should add two vectors', () => {
      const v1 = new Vector3(1, 2, 3);
      const v2 = new Vector3(4, 5, 6);
      const result = v1.add(v2);
      expect(result.x).toBe(5);
      expect(result.y).toBe(7);
      expect(result.z).toBe(9);
    });
  });

  describe('subtract', () => {
    it('should subtract two vectors', () => {
      const v1 = new Vector3(5, 7, 9);
      const v2 = new Vector3(1, 2, 3);
      const result = v1.subtract(v2);
      expect(result.x).toBe(4);
      expect(result.y).toBe(5);
      expect(result.z).toBe(6);
    });
  });

  describe('multiply', () => {
    it('should multiply by scalar', () => {
      const v = new Vector3(1, 2, 3);
      const result = v.multiply(2);
      expect(result.x).toBe(2);
      expect(result.y).toBe(4);
      expect(result.z).toBe(6);
    });
  });

  describe('divide', () => {
    it('should divide by scalar', () => {
      const v = new Vector3(4, 6, 8);
      const result = v.divide(2);
      expect(result.x).toBe(2);
      expect(result.y).toBe(3);
      expect(result.z).toBe(4);
    });

    it('should throw error when dividing by zero', () => {
      const v = new Vector3(1, 2, 3);
      expect(() => v.divide(0)).toThrow('Cannot divide by zero');
    });
  });

  describe('dot', () => {
    it('should calculate dot product', () => {
      const v1 = new Vector3(1, 2, 3);
      const v2 = new Vector3(4, 5, 6);
      const result = v1.dot(v2);
      expect(result).toBe(32); // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
    });
  });

  describe('cross', () => {
    it('should calculate cross product', () => {
      const v1 = new Vector3(1, 0, 0);
      const v2 = new Vector3(0, 1, 0);
      const result = v1.cross(v2);
      expect(result.x).toBe(0);
      expect(result.y).toBe(0);
      expect(result.z).toBe(1);
    });
  });

  describe('length', () => {
    it('should calculate length', () => {
      const v = new Vector3(3, 4, 0);
      expect(v.length()).toBe(5);
    });

    it('should calculate length for 3D vector', () => {
      const v = new Vector3(1, 2, 2);
      expect(v.length()).toBe(3);
    });
  });

  describe('lengthSquared', () => {
    it('should calculate squared length', () => {
      const v = new Vector3(3, 4, 0);
      expect(v.lengthSquared()).toBe(25);
    });
  });

  describe('normalize', () => {
    it('should normalize vector', () => {
      const v = new Vector3(3, 4, 0);
      const result = v.normalize();
      expect(result.length()).toBeCloseTo(1, 5);
      expect(result.x).toBeCloseTo(0.6, 5);
      expect(result.y).toBeCloseTo(0.8, 5);
    });

    it('should return zero vector when normalizing zero vector', () => {
      const v = Vector3.zero();
      const result = v.normalize();
      expect(result.equals(Vector3.zero())).toBe(true);
    });
  });

  describe('distance', () => {
    it('should calculate distance between vectors', () => {
      const v1 = new Vector3(0, 0, 0);
      const v2 = new Vector3(3, 4, 0);
      expect(v1.distance(v2)).toBe(5);
    });
  });

  describe('lerp', () => {
    it('should interpolate between vectors', () => {
      const v1 = new Vector3(0, 0, 0);
      const v2 = new Vector3(10, 10, 10);
      const result = v1.lerp(v2, 0.5);
      expect(result.x).toBe(5);
      expect(result.y).toBe(5);
      expect(result.z).toBe(5);
    });
  });

  describe('clone', () => {
    it('should create a copy', () => {
      const v1 = new Vector3(1, 2, 3);
      const v2 = v1.clone();
      expect(v2).not.toBe(v1);
      expect(v2.equals(v1)).toBe(true);
    });
  });

  describe('equals', () => {
    it('should check equality', () => {
      const v1 = new Vector3(1, 2, 3);
      const v2 = new Vector3(1, 2, 3);
      const v3 = new Vector3(1, 2, 4);
      expect(v1.equals(v2)).toBe(true);
      expect(v1.equals(v3)).toBe(false);
    });

    it('should check equality with epsilon', () => {
      const v1 = new Vector3(1, 2, 3);
      const v2 = new Vector3(1.0001, 2.0001, 3.0001);
      expect(v1.equals(v2, 0.001)).toBe(true);
    });
  });

  describe('toArray', () => {
    it('should convert to array', () => {
      const v = new Vector3(1, 2, 3);
      const arr = v.toArray();
      expect(arr).toEqual([1, 2, 3]);
    });
  });
});
