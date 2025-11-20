/**
 * PBR Material Tests
 */

import { PBRMaterial, createPBRMaterial } from '../PBRMaterial';
import { Vector3 } from '../../../math/Vector3';

describe('PBRMaterial', () => {
  describe('Constructor', () => {
    it('should create with default properties', () => {
      const material = new PBRMaterial();
      
      expect(material.albedo).toEqual(new Vector3(1, 1, 1));
      expect(material.metallic).toBe(0.0);
      expect(material.roughness).toBe(0.5);
      expect(material.opacity).toBe(1.0);
      expect(material.alphaMode).toBe('OPAQUE');
    });

    it('should create with custom properties', () => {
      const albedo = new Vector3(0.8, 0.2, 0.1);
      const emissive = new Vector3(1, 0, 0);
      
      const material = new PBRMaterial({
        albedo,
        metallic: 0.8,
        roughness: 0.3,
        emissive,
        emissiveIntensity: 2.0,
      });
      
      expect(material.albedo).toEqual(albedo);
      expect(material.metallic).toBe(0.8);
      expect(material.roughness).toBe(0.3);
      expect(material.emissive).toEqual(emissive);
      expect(material.emissiveIntensity).toBe(2.0);
    });
  });

  describe('Metallic and Roughness', () => {
    it('should clamp metallic to [0, 1]', () => {
      const material = new PBRMaterial();
      
      material.metallic = 1.5;
      expect(material.metallic).toBe(1.0);
      
      material.metallic = -0.5;
      expect(material.metallic).toBe(0.0);
      
      material.metallic = 0.7;
      expect(material.metallic).toBe(0.7);
    });

    it('should clamp roughness to [0, 1]', () => {
      const material = new PBRMaterial();
      
      material.roughness = 2.0;
      expect(material.roughness).toBe(1.0);
      
      material.roughness = -1.0;
      expect(material.roughness).toBe(0.0);
      
      material.roughness = 0.4;
      expect(material.roughness).toBe(0.4);
    });
  });

  describe('Helper Function', () => {
    it('should create PBR material with helper', () => {
      const material = createPBRMaterial({
        metallic: 1.0,
        roughness: 0.2,
      });
      
      expect(material).toBeInstanceOf(PBRMaterial);
      expect(material.metallic).toBe(1.0);
      expect(material.roughness).toBe(0.2);
    });
  });
});
