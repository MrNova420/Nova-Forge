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
 * Fast Approximate Anti-Aliasing (FXAA)
 * GPU-based anti-aliasing technique for smoothing jagged edges
 */

import { PostProcessingEffect } from './PostProcessingStack';
import { Texture } from '../Texture';
import { Shader } from '../Shader';

/**
 * FXAA quality presets
 */
export enum FXAAQuality {
  /**
   * Low quality, fastest
   */
  LOW = 'LOW',

  /**
   * Medium quality, balanced
   */
  MEDIUM = 'MEDIUM',

  /**
   * High quality, slower
   */
  HIGH = 'HIGH',

  /**
   * Ultra quality, slowest
   */
  ULTRA = 'ULTRA',
}

/**
 * FXAA effect for fast anti-aliasing
 */
export class FXAA implements PostProcessingEffect {
  public readonly name = 'FXAA';
  public enabled = true;

  private quality: FXAAQuality = FXAAQuality.MEDIUM;
  private edgeThreshold: number = 0.125; // Edge detection threshold
  private edgeThresholdMin: number = 0.0625; // Minimum edge threshold

  private shader: Shader | null = null;
  private width: number = 0;
  private height: number = 0;

  /**
   * Create a new FXAA effect
   * @param quality - Quality preset
   */
  constructor(quality: FXAAQuality = FXAAQuality.MEDIUM) {
    this.quality = quality;
  }

  /**
   * Set the quality preset
   */
  public setQuality(quality: FXAAQuality): void {
    this.quality = quality;
  }

  /**
   * Set the edge detection threshold
   */
  public setEdgeThreshold(threshold: number): void {
    this.edgeThreshold = Math.max(0.0, Math.min(1.0, threshold));
  }

  /**
   * Set the minimum edge threshold
   */
  public setEdgeThresholdMin(threshold: number): void {
    this.edgeThresholdMin = Math.max(0.0, Math.min(1.0, threshold));
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
    this.shader!.setUniform('uTexelSize', [1.0 / width, 1.0 / height]);
    this.shader!.setUniformInt('uQuality', this.getQualityValue());
    this.shader!.setUniform('uEdgeThreshold', this.edgeThreshold);
    this.shader!.setUniform('uEdgeThresholdMin', this.edgeThresholdMin);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, inputTexture.glTexture);

    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

    this.shader!;
  }

  public resize(_gl: WebGL2RenderingContext, width: number, height: number): void {
    this.width = width;
    this.height = height;
    // No buffers to resize for FXAA
  }

  private getQualityValue(): number {
    switch (this.quality) {
      case FXAAQuality.LOW:
        return 0.0;
      case FXAAQuality.MEDIUM:
        return 1.0;
      case FXAAQuality.HIGH:
        return 2.0;
      case FXAAQuality.ULTRA:
        return 3.0;
      default:
        return 1.0;
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
      uniform vec2 uTexelSize;
      uniform float uQuality;
      uniform float uEdgeThreshold;
      uniform float uEdgeThresholdMin;

      // Luminance calculation
      float getLuminance(vec3 color) {
        return dot(color, vec3(0.299, 0.587, 0.114));
      }

      void main() {
        // Sample center and neighbors
        vec3 colorCenter = texture(uTexture, vUV).rgb;
        float lumCenter = getLuminance(colorCenter);
        
        // Sample neighbors for edge detection
        float lumN = getLuminance(textureOffset(uTexture, vUV, ivec2(0, 1)).rgb);
        float lumS = getLuminance(textureOffset(uTexture, vUV, ivec2(0, -1)).rgb);
        float lumE = getLuminance(textureOffset(uTexture, vUV, ivec2(1, 0)).rgb);
        float lumW = getLuminance(textureOffset(uTexture, vUV, ivec2(-1, 0)).rgb);
        
        // Find min and max luminance
        float lumMin = min(lumCenter, min(min(lumN, lumS), min(lumE, lumW)));
        float lumMax = max(lumCenter, max(max(lumN, lumS), max(lumE, lumW)));
        
        // Edge detection
        float lumRange = lumMax - lumMin;
        
        // Early exit if no edge detected
        if (lumRange < max(uEdgeThresholdMin, lumMax * uEdgeThreshold)) {
          fragColor = vec4(colorCenter, 1.0);
          return;
        }
        
        // Sample diagonal neighbors
        float lumNE = getLuminance(textureOffset(uTexture, vUV, ivec2(1, 1)).rgb);
        float lumNW = getLuminance(textureOffset(uTexture, vUV, ivec2(-1, 1)).rgb);
        float lumSE = getLuminance(textureOffset(uTexture, vUV, ivec2(1, -1)).rgb);
        float lumSW = getLuminance(textureOffset(uTexture, vUV, ivec2(-1, -1)).rgb);
        
        // Combine samples for edge direction
        float lumNS = lumN + lumS;
        float lumEW = lumE + lumW;
        float lumNESW = lumNE + lumSW;
        float lumNWSE = lumNW + lumSE;
        
        float edgeHorizontal = abs(-2.0 * lumCenter + lumNS) + 
                               abs(-2.0 * lumE + lumNESW) + 
                               abs(-2.0 * lumW + lumNWSE);
        
        float edgeVertical = abs(-2.0 * lumCenter + lumEW) + 
                            abs(-2.0 * lumN + lumNWSE) + 
                            abs(-2.0 * lumS + lumNESW);
        
        bool isHorizontal = edgeHorizontal >= edgeVertical;
        
        // Select step direction based on edge orientation
        float stepLength = isHorizontal ? uTexelSize.y : uTexelSize.x;
        float lum1 = isHorizontal ? lumN : lumE;
        float lum2 = isHorizontal ? lumS : lumW;
        float gradient1 = abs(lum1 - lumCenter);
        float gradient2 = abs(lum2 - lumCenter);
        
        // Choose blend direction
        bool is1Steepest = gradient1 >= gradient2;
        float gradientScaled = 0.25 * max(gradient1, gradient2);
        
        // Blend factor
        float blendFactor = max(gradient1, gradient2) / lumRange;
        blendFactor = smoothstep(0.0, 1.0, blendFactor);
        
        // Calculate blend UV offset
        vec2 blendUV = vUV;
        if (isHorizontal) {
          blendUV.y += stepLength * (is1Steepest ? 0.5 : -0.5) * blendFactor;
        } else {
          blendUV.x += stepLength * (is1Steepest ? 0.5 : -0.5) * blendFactor;
        }
        
        // Sample blended color
        vec3 colorBlended = texture(uTexture, blendUV).rgb;
        
        // Quality-based additional sampling
        if (uQuality >= 1.0) {
          // Medium/High/Ultra quality adds sub-pixel anti-aliasing
          vec3 colorNE = texture(uTexture, vUV + vec2(uTexelSize.x, uTexelSize.y) * 0.5).rgb;
          vec3 colorNW = texture(uTexture, vUV + vec2(-uTexelSize.x, uTexelSize.y) * 0.5).rgb;
          vec3 colorSE = texture(uTexture, vUV + vec2(uTexelSize.x, -uTexelSize.y) * 0.5).rgb;
          vec3 colorSW = texture(uTexture, vUV + vec2(-uTexelSize.x, -uTexelSize.y) * 0.5).rgb;
          colorBlended = (colorBlended + colorNE + colorNW + colorSE + colorSW) / 5.0;
        }
        
        fragColor = vec4(colorBlended, 1.0);
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
