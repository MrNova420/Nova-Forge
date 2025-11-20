/**
 * Post-Processing Tests
 */

import { PostProcessingStack } from '../PostProcessingStack';
import { Bloom } from '../Bloom';
import { SSAO } from '../SSAO';
import { ToneMapping, ToneMappingOperator } from '../ToneMapping';
import { FXAA, FXAAQuality } from '../FXAA';

describe('PostProcessingStack', () => {
  it('should create a post-processing stack', () => {
    const stack = new PostProcessingStack();
    expect(stack).toBeDefined();
  });

  it('should add effects to the stack', () => {
    const stack = new PostProcessingStack();
    const bloom = new Bloom();
    stack.addEffect(bloom);
    expect(stack.getEffect('Bloom')).toBe(bloom);
  });

  it('should remove effects from the stack', () => {
    const stack = new PostProcessingStack();
    const bloom = new Bloom();
    stack.addEffect(bloom);
    stack.removeEffect(bloom);
    expect(stack.getEffect('Bloom')).toBeUndefined();
  });
});

describe('Bloom', () => {
  it('should create a bloom effect', () => {
    const bloom = new Bloom();
    expect(bloom).toBeDefined();
    expect(bloom.name).toBe('Bloom');
    expect(bloom.enabled).toBe(true);
  });

  it('should set threshold', () => {
    const bloom = new Bloom();
    bloom.setThreshold(0.5);
    expect(bloom).toBeDefined();
  });

  it('should set intensity', () => {
    const bloom = new Bloom();
    bloom.setIntensity(2.0);
    expect(bloom).toBeDefined();
  });

  it('should set blur iterations', () => {
    const bloom = new Bloom();
    bloom.setBlurIterations(10);
    expect(bloom).toBeDefined();
  });
});

describe('SSAO', () => {
  it('should create an SSAO effect', () => {
    const ssao = new SSAO();
    expect(ssao).toBeDefined();
    expect(ssao.name).toBe('SSAO');
    expect(ssao.enabled).toBe(true);
  });

  it('should set radius', () => {
    const ssao = new SSAO();
    ssao.setRadius(1.0);
    expect(ssao).toBeDefined();
  });

  it('should set intensity', () => {
    const ssao = new SSAO();
    ssao.setIntensity(1.5);
    expect(ssao).toBeDefined();
  });

  it('should set bias', () => {
    const ssao = new SSAO();
    ssao.setBias(0.01);
    expect(ssao).toBeDefined();
  });
});

describe('ToneMapping', () => {
  it('should create a tone mapping effect', () => {
    const toneMapping = new ToneMapping();
    expect(toneMapping).toBeDefined();
    expect(toneMapping.name).toBe('ToneMapping');
    expect(toneMapping.enabled).toBe(true);
  });

  it('should set operator', () => {
    const toneMapping = new ToneMapping();
    toneMapping.setOperator(ToneMappingOperator.REINHARD);
    expect(toneMapping).toBeDefined();
  });

  it('should set exposure', () => {
    const toneMapping = new ToneMapping();
    toneMapping.setExposure(1.5);
    expect(toneMapping).toBeDefined();
  });

  it('should set gamma', () => {
    const toneMapping = new ToneMapping();
    toneMapping.setGamma(2.4);
    expect(toneMapping).toBeDefined();
  });

  it('should set white point', () => {
    const toneMapping = new ToneMapping();
    toneMapping.setWhitePoint(15.0);
    expect(toneMapping).toBeDefined();
  });

  it('should support multiple operators', () => {
    const operators = [
      ToneMappingOperator.REINHARD,
      ToneMappingOperator.REINHARD_LUMINANCE,
      ToneMappingOperator.ACES,
      ToneMappingOperator.UNCHARTED2,
      ToneMappingOperator.EXPOSURE,
      ToneMappingOperator.LINEAR,
    ];

    operators.forEach((op) => {
      const toneMapping = new ToneMapping(op);
      toneMapping.setOperator(op);
      expect(toneMapping).toBeDefined();
    });
  });
});

describe('FXAA', () => {
  it('should create an FXAA effect', () => {
    const fxaa = new FXAA();
    expect(fxaa).toBeDefined();
    expect(fxaa.name).toBe('FXAA');
    expect(fxaa.enabled).toBe(true);
  });

  it('should set quality', () => {
    const fxaa = new FXAA();
    fxaa.setQuality(FXAAQuality.HIGH);
    expect(fxaa).toBeDefined();
  });

  it('should set edge threshold', () => {
    const fxaa = new FXAA();
    fxaa.setEdgeThreshold(0.15);
    expect(fxaa).toBeDefined();
  });

  it('should set edge threshold min', () => {
    const fxaa = new FXAA();
    fxaa.setEdgeThresholdMin(0.05);
    expect(fxaa).toBeDefined();
  });

  it('should support multiple quality levels', () => {
    const qualities = [
      FXAAQuality.LOW,
      FXAAQuality.MEDIUM,
      FXAAQuality.HIGH,
      FXAAQuality.ULTRA,
    ];

    qualities.forEach((quality) => {
      const fxaa = new FXAA(quality);
      fxaa.setQuality(quality);
      expect(fxaa).toBeDefined();
    });
  });
});
