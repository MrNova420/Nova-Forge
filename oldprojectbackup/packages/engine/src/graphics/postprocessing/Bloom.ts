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
 * Bloom Post-Processing Effect
 * Creates a glow effect around bright areas of the image
 */

import { PostProcessingEffect } from './PostProcessingStack';
import { Texture } from '../Texture';
import { Shader } from '../Shader';

/**
 * Bloom effect that extracts bright areas, blurs them, and adds them back
 */
export class Bloom implements PostProcessingEffect {
  public readonly name = 'Bloom';
  public enabled = true;

  private threshold: number = 1.0; // Brightness threshold
  private intensity: number = 1.0; // Effect intensity
  private blurIterations: number = 5; // Number of blur passes
  private blurRadius: number = 1.0; // Blur spread

  private brightPassFB: WebGLFramebuffer | null = null;
  private brightPassTexture: Texture | null = null;
  private blurFB: WebGLFramebuffer[] = [];
  private blurTextures: Texture[] = [];

  private brightPassShader: Shader | null = null;
  private blurShader: Shader | null = null;
  private compositeShader: Shader | null = null;

  private width: number = 0;
  private height: number = 0;

  /**
   * Create a new Bloom effect
   * @param threshold - Brightness threshold (default: 1.0)
   * @param intensity - Effect intensity (default: 1.0)
   * @param iterations - Number of blur passes (default: 5)
   */
  constructor(threshold: number = 1.0, intensity: number = 1.0, iterations: number = 5) {
    this.threshold = threshold;
    this.intensity = intensity;
    this.blurIterations = iterations;
  }

  /**
   * Set the brightness threshold
   */
  public setThreshold(threshold: number): void {
    this.threshold = Math.max(0, threshold);
  }

  /**
   * Set the effect intensity
   */
  public setIntensity(intensity: number): void {
    this.intensity = Math.max(0, intensity);
  }

  /**
   * Set the number of blur iterations
   */
  public setBlurIterations(iterations: number): void {
    this.blurIterations = Math.max(1, Math.min(10, iterations));
  }

  public apply(
    gl: WebGL2RenderingContext,
    inputTexture: Texture,
    outputFramebuffer: WebGLFramebuffer | null,
    width: number,
    height: number
  ): void {
    if (!this.brightPassShader || !this.blurShader || !this.compositeShader) {
      this.initializeShaders(gl);
    }

    if (this.width !== width || this.height !== height) {
      this.resize(gl, width, height);
    }

    // Step 1: Extract bright areas
    this.extractBrightAreas(gl, inputTexture);

    // Step 2: Blur the bright areas
    this.blurBrightAreas(gl);

    // Step 3: Composite back onto original
    this.composite(gl, inputTexture, outputFramebuffer, width, height);
  }

  /**
   * Extract areas brighter than threshold
   */
  private extractBrightAreas(gl: WebGL2RenderingContext, inputTexture: Texture): void {
    if (!this.brightPassFB || !this.brightPassShader) return;

    gl.bindFramebuffer(gl.FRAMEBUFFER, this.brightPassFB);
    gl.viewport(0, 0, this.width / 2, this.height / 2);

    this.brightPassShader.use();
    this.brightPassShader.setUniformInt('uTexture', 0);
    this.brightPassShader.setUniform('uThreshold', this.threshold);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, inputTexture.glTexture);

    // Draw full-screen quad (assumes quad is bound externally)
    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

    this.brightPassShader;
  }

  /**
   * Blur bright areas with progressive downsampling
   */
  private blurBrightAreas(gl: WebGL2RenderingContext): void {
    if (!this.blurShader || !this.brightPassTexture) return;

    let currentTexture = this.brightPassTexture;
    let currentWidth = this.width / 2;
    let currentHeight = this.height / 2;

    for (let i = 0; i < this.blurIterations; i++) {
      const targetFB = this.blurFB[i];
      const targetTexture = this.blurTextures[i];

      // Downsample
      currentWidth = Math.max(1, Math.floor(currentWidth / 2));
      currentHeight = Math.max(1, Math.floor(currentHeight / 2));

      gl.bindFramebuffer(gl.FRAMEBUFFER, targetFB!);
      gl.viewport(0, 0, currentWidth, currentHeight);

      this.blurShader.use();
      this.blurShader.setUniformInt('uTexture', 0);
      this.blurShader.setUniform('uTexelSize', [1.0 / currentWidth, 1.0 / currentHeight]);
      this.blurShader.setUniform('uRadius', this.blurRadius);

      gl.activeTexture(gl.TEXTURE0);
      gl.bindTexture(gl.TEXTURE_2D, currentTexture.glTexture);

      gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

      this.blurShader;

      currentTexture = targetTexture!;
    }
  }

  /**
   * Composite blurred result back onto original
   */
  private composite(
    gl: WebGL2RenderingContext,
    inputTexture: Texture,
    outputFB: WebGLFramebuffer | null,
    width: number,
    height: number
  ): void {
    if (!this.compositeShader) return;

    gl.bindFramebuffer(gl.FRAMEBUFFER, outputFB);
    gl.viewport(0, 0, width, height);

    this.compositeShader.use();
    this.compositeShader.setUniformInt('uScene', 0);
    this.compositeShader.setUniformInt('uBloom', 1);
    this.compositeShader.setUniform('uIntensity', this.intensity);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, inputTexture.glTexture);

    // Use the last blur result
    const lastBlur = this.blurTextures[this.blurIterations - 1];
    if (lastBlur) {
      gl.activeTexture(gl.TEXTURE1);
      gl.bindTexture(gl.TEXTURE_2D, lastBlur.glTexture);
    }

    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

    this.compositeShader;
  }

  public resize(gl: WebGL2RenderingContext, width: number, height: number): void {
    this.width = width;
    this.height = height;

    // Dispose old resources
    if (this.brightPassFB) {
      gl.deleteFramebuffer(this.brightPassFB);
    }
    if (this.brightPassTexture) {
    }
    for (const fb of this.blurFB) {
      gl.deleteFramebuffer(fb);
    }
    for (const tex of this.blurTextures) {
      tex.destroy();
    }

    this.blurFB = [];
    this.blurTextures = [];

    // Create bright pass buffer (half resolution)
    this.brightPassFB = gl.createFramebuffer();
    this.brightPassTexture = this.createRenderTexture(gl, width / 2, height / 2);
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.brightPassFB);
    gl.framebufferTexture2D(
      gl.FRAMEBUFFER,
      gl.COLOR_ATTACHMENT0,
      gl.TEXTURE_2D,
      this.brightPassTexture.glTexture,
      0
    );

    // Create blur buffers (progressively smaller)
    let blurWidth = width / 2;
    let blurHeight = height / 2;
    for (let i = 0; i < this.blurIterations; i++) {
      blurWidth = Math.max(1, Math.floor(blurWidth / 2));
      blurHeight = Math.max(1, Math.floor(blurHeight / 2));

      const fb = gl.createFramebuffer();
      const texture = this.createRenderTexture(gl, blurWidth, blurHeight);

      gl.bindFramebuffer(gl.FRAMEBUFFER, fb);
      gl.framebufferTexture2D(
        gl.FRAMEBUFFER,
        gl.COLOR_ATTACHMENT0,
        gl.TEXTURE_2D,
        texture.glTexture,
        0
      );

      this.blurFB.push(fb!);
      this.blurTextures.push(texture);
    }

    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
  }

  private createRenderTexture(
    gl: WebGL2RenderingContext,
    width: number,
    height: number
  ): Texture {
    const texture = new Texture(gl);
    gl.bindTexture(gl.TEXTURE_2D, texture.glTexture);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA16F, width, height, 0, gl.RGBA, gl.FLOAT, null);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    return texture;
  }

  private initializeShaders(gl: WebGL2RenderingContext): void {
    // Bright pass shader source
    const brightPassVert = `#version 300 es
      layout(location = 0) in vec2 aPosition;
      layout(location = 1) in vec2 aUV;
      out vec2 vUV;
      void main() {
        vUV = aUV;
        gl_Position = vec4(aPosition, 0.0, 1.0);
      }
    `;

    const brightPassFrag = `#version 300 es
      precision highp float;
      in vec2 vUV;
      out vec4 fragColor;
      uniform sampler2D uTexture;
      uniform float uThreshold;
      void main() {
        vec4 color = texture(uTexture, vUV);
        float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
        if (brightness > uThreshold) {
          fragColor = color;
        } else {
          fragColor = vec4(0.0);
        }
      }
    `;

    // Blur shader source (Gaussian)
    const blurFrag = `#version 300 es
      precision highp float;
      in vec2 vUV;
      out vec4 fragColor;
      uniform sampler2D uTexture;
      uniform vec2 uTexelSize;
      uniform float uRadius;
      void main() {
        vec4 sum = vec4(0.0);
        float weights = 0.0;
        for (float x = -4.0; x <= 4.0; x += 1.0) {
          for (float y = -4.0; y <= 4.0; y += 1.0) {
            vec2 offset = vec2(x, y) * uTexelSize * uRadius;
            float weight = exp(-(x*x + y*y) / 8.0);
            sum += texture(uTexture, vUV + offset) * weight;
            weights += weight;
          }
        }
        fragColor = sum / weights;
      }
    `;

    // Composite shader source
    const compositeFrag = `#version 300 es
      precision highp float;
      in vec2 vUV;
      out vec4 fragColor;
      uniform sampler2D uScene;
      uniform sampler2D uBloom;
      uniform float uIntensity;
      void main() {
        vec4 scene = texture(uScene, vUV);
        vec4 bloom = texture(uBloom, vUV);
        fragColor = scene + bloom * uIntensity;
      }
    `;

    this.brightPassShader = new Shader(gl);
    this.brightPassShader.compile(brightPassVert, brightPassFrag);

    this.blurShader = new Shader(gl);
    this.blurShader.compile(brightPassVert, blurFrag);

    this.compositeShader = new Shader(gl);
    this.compositeShader.compile(brightPassVert, compositeFrag);
  }

  public dispose(gl: WebGL2RenderingContext): void {
    if (this.brightPassFB) gl.deleteFramebuffer(this.brightPassFB);

    for (const fb of this.blurFB) {
      gl.deleteFramebuffer(fb);
    }
    for (const tex of this.blurTextures) {
      tex.destroy();
    }

    if (this.brightPassShader) this.brightPassShader.destroy();
    if (this.blurShader) this.blurShader.destroy();
    if (this.compositeShader) this.compositeShader.destroy();
  }
}
