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
 * Tone Mapping Post-Processing Effect
 * Maps HDR colors to displayable LDR range
 */

import { PostProcessingEffect } from './PostProcessingStack';
import { Texture } from '../Texture';
import { Shader } from '../Shader';

/**
 * Tone mapping operators
 */
export enum ToneMappingOperator {
  /**
   * Simple Reinhard tone mapping
   */
  REINHARD = 'REINHARD',

  /**
   * Reinhard with luminance-based adaptation
   */
  REINHARD_LUMINANCE = 'REINHARD_LUMINANCE',

  /**
   * Filmic ACES tone mapping (Academy Color Encoding System)
   */
  ACES = 'ACES',

  /**
   * Uncharted 2 filmic tone mapping
   */
  UNCHARTED2 = 'UNCHARTED2',

  /**
   * Simple exposure-based tone mapping
   */
  EXPOSURE = 'EXPOSURE',

  /**
   * No tone mapping, just gamma correction
   */
  LINEAR = 'LINEAR',
}

/**
 * Tone Mapping effect for HDR to LDR conversion
 */
export class ToneMapping implements PostProcessingEffect {
  public readonly name = 'ToneMapping';
  public enabled = true;

  private operator: ToneMappingOperator = ToneMappingOperator.ACES;
  private exposure: number = 1.0;
  private gamma: number = 2.2;
  private whitePoint: number = 11.2;

  private shader: Shader | null = null;
  private width: number = 0;
  private height: number = 0;

  /**
   * Create a new ToneMapping effect
   * @param operator - Tone mapping operator to use
   * @param exposure - Exposure adjustment
   * @param gamma - Gamma correction value
   */
  constructor(
    operator: ToneMappingOperator = ToneMappingOperator.ACES,
    exposure: number = 1.0,
    gamma: number = 2.2
  ) {
    this.operator = operator;
    this.exposure = exposure;
    this.gamma = gamma;
  }

  /**
   * Set the tone mapping operator
   */
  public setOperator(operator: ToneMappingOperator): void {
    this.operator = operator;
  }

  /**
   * Set the exposure value
   */
  public setExposure(exposure: number): void {
    this.exposure = Math.max(0, exposure);
  }

  /**
   * Set the gamma correction value
   */
  public setGamma(gamma: number): void {
    this.gamma = Math.max(0.1, gamma);
  }

  /**
   * Set the white point for Reinhard tone mapping
   */
  public setWhitePoint(whitePoint: number): void {
    this.whitePoint = Math.max(1.0, whitePoint);
  }

  public apply(
    gl: WebGL2RenderingContext,
    inputTexture: Texture,
    outputFramebuffer: WebGLFramebuffer | null,
    width: number,
    height: number
  ): void {
    if (!this.shader) {
      this.initializeShader(gl);
    }

    if (this.width !== width || this.height !== height) {
      this.resize(gl, width, height);
    }

    gl.bindFramebuffer(gl.FRAMEBUFFER, outputFramebuffer);
    gl.viewport(0, 0, width, height);

    this.shader!.use();
    this.shader!.setUniformInt('uTexture', 0);
    this.shader!.setUniformInt('uOperator', this.getOperatorIndex());
    this.shader!.setUniform('uExposure', this.exposure);
    this.shader!.setUniform('uGamma', this.gamma);
    this.shader!.setUniform('uWhitePoint', this.whitePoint);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, inputTexture.glTexture);

    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

    this.shader!;
  }

  public resize(_gl: WebGL2RenderingContext, width: number, height: number): void {
    this.width = width;
    this.height = height;
    // No buffers to resize for tone mapping
  }

  private getOperatorIndex(): number {
    switch (this.operator) {
      case ToneMappingOperator.REINHARD:
        return 0;
      case ToneMappingOperator.REINHARD_LUMINANCE:
        return 1;
      case ToneMappingOperator.ACES:
        return 2;
      case ToneMappingOperator.UNCHARTED2:
        return 3;
      case ToneMappingOperator.EXPOSURE:
        return 4;
      case ToneMappingOperator.LINEAR:
        return 5;
      default:
        return 2;
    }
  }

  private initializeShader(gl: WebGL2RenderingContext): void {
    const vertexShader = `#version 300 es
      layout(location = 0) in vec2 aPosition;
      layout(location = 1) in vec2 aUV;
      out vec2 vUV;
      void main() {
        vUV = aUV;
        gl_Position = vec4(aPosition, 0.0, 1.0);
      }
    `;

    const fragmentShader = `#version 300 es
      precision highp float;
      in vec2 vUV;
      out vec4 fragColor;
      
      uniform sampler2D uTexture;
      uniform int uOperator;
      uniform float uExposure;
      uniform float uGamma;
      uniform float uWhitePoint;

      // Reinhard tone mapping
      vec3 reinhard(vec3 color) {
        return color / (color + vec3(1.0));
      }

      // Reinhard with luminance
      vec3 reinhardLuminance(vec3 color, float whitePoint) {
        float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
        float mappedLuminance = (luminance * (1.0 + luminance / (whitePoint * whitePoint))) / (1.0 + luminance);
        return color * (mappedLuminance / luminance);
      }

      // ACES Filmic tone mapping
      vec3 aces(vec3 x) {
        const float a = 2.51;
        const float b = 0.03;
        const float c = 2.43;
        const float d = 0.59;
        const float e = 0.14;
        return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
      }

      // Uncharted 2 tone mapping
      vec3 uncharted2Tonemap(vec3 x) {
        const float A = 0.15;
        const float B = 0.50;
        const float C = 0.10;
        const float D = 0.20;
        const float E = 0.02;
        const float F = 0.30;
        return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
      }

      vec3 uncharted2(vec3 color) {
        const float W = 11.2;
        color = uncharted2Tonemap(color * 2.0);
        vec3 whiteScale = 1.0 / uncharted2Tonemap(vec3(W));
        return color * whiteScale;
      }

      // Simple exposure
      vec3 exposure(vec3 color, float exp) {
        return vec3(1.0) - exp(-color * exp);
      }

      // Gamma correction
      vec3 gammaCorrect(vec3 color, float gamma) {
        return pow(color, vec3(1.0 / gamma));
      }

      void main() {
        vec3 hdrColor = texture(uTexture, vUV).rgb;
        
        // Apply exposure
        hdrColor *= uExposure;
        
        // Apply tone mapping
        vec3 mappedColor;
        if (uOperator == 0) {
          mappedColor = reinhard(hdrColor);
        } else if (uOperator == 1) {
          mappedColor = reinhardLuminance(hdrColor, uWhitePoint);
        } else if (uOperator == 2) {
          mappedColor = aces(hdrColor);
        } else if (uOperator == 3) {
          mappedColor = uncharted2(hdrColor);
        } else if (uOperator == 4) {
          mappedColor = exposure(hdrColor, uExposure);
        } else {
          mappedColor = hdrColor;
        }
        
        // Gamma correction
        mappedColor = gammaCorrect(mappedColor, uGamma);
        
        fragColor = vec4(mappedColor, 1.0);
      }
    `;

    this.shader = new Shader(gl);
    this.shader.compile(vertexShader, fragmentShader);
  }

  public dispose(_gl: WebGL2RenderingContext): void {
    if (this.shader) {
      this.shader.destroy();
    }
  }
}
