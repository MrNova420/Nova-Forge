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
 * Screen Space Ambient Occlusion (SSAO)
 * Adds realistic ambient shadows based on scene geometry
 */

import { PostProcessingEffect } from './PostProcessingStack';
import { Texture } from '../Texture';
import { Shader } from '../Shader';

/**
 * SSAO effect that calculates ambient occlusion in screen space
 */
export class SSAO implements PostProcessingEffect {
  public readonly name = 'SSAO';
  public enabled = true;

  private radius: number = 0.5; // Sampling radius
  private bias: number = 0.025; // Depth bias to prevent acne
  private intensity: number = 1.0; // Effect intensity
  private sampleCount: number = 128; // High quality samples for AAA graphics

  private ssaoFB: WebGLFramebuffer | null = null;
  private ssaoTexture: Texture | null = null;
  private blurFB: WebGLFramebuffer | null = null;
  private blurTexture: Texture | null = null;
  private noiseTexture: Texture | null = null;

  private ssaoShader: Shader | null = null;
  private blurShader: Shader | null = null;
  private compositeShader: Shader | null = null;

  private samples: Float32Array | null = null;
  private width: number = 0;
  private height: number = 0;

  /**
   * Create a new SSAO effect
   * @param radius - Sampling radius (default: 0.5)
   * @param sampleCount - Number of samples (default: 16)
   */
  constructor(radius: number = 0.5, sampleCount: number = 128) {
    this.radius = radius;
    this.sampleCount = sampleCount;
    this.generateSamples();
  }

  /**
   * Set the sampling radius
   */
  public setRadius(radius: number): void {
    this.radius = Math.max(0.01, radius);
  }

  /**
   * Set the effect intensity
   */
  public setIntensity(intensity: number): void {
    this.intensity = Math.max(0, intensity);
  }

  /**
   * Set the depth bias
   */
  public setBias(bias: number): void {
    this.bias = Math.max(0, bias);
  }

  public apply(
    gl: WebGL2RenderingContext,
    inputTexture: Texture,
    outputFramebuffer: WebGLFramebuffer | null,
    width: number,
    height: number
  ): void {
    if (!this.ssaoShader || !this.blurShader || !this.compositeShader) {
      this.initializeShaders(gl);
    }

    if (!this.noiseTexture) {
      this.generateNoiseTexture(gl);
    }

    if (this.width !== width || this.height !== height) {
      this.resize(gl, width, height);
    }

    // Note: This is a simplified version. A complete SSAO requires:
    // - Depth buffer from the scene
    // - Normal buffer from the scene
    // - Camera projection/view matrices

    // Step 1: Calculate SSAO
    this.calculateSSAO(gl, inputTexture);

    // Step 2: Blur SSAO to reduce noise
    this.blurSSAO(gl);

    // Step 3: Composite with original scene
    this.composite(gl, inputTexture, outputFramebuffer, width, height);
  }

  /**
   * Generate sample kernel for hemisphere sampling
   */
  private generateSamples(): void {
    this.samples = new Float32Array(this.sampleCount * 3);

    for (let i = 0; i < this.sampleCount; i++) {
      // Random point in hemisphere
      let x = Math.random() * 2.0 - 1.0;
      let y = Math.random() * 2.0 - 1.0;
      let z = Math.random();

      // Normalize
      const length = Math.sqrt(x * x + y * y + z * z);
      x /= length;
      y /= length;
      z /= length;

      // Scale with lerp to favor samples closer to origin
      let scale = i / this.sampleCount;
      scale = 0.1 + scale * scale * 0.9;
      x *= scale;
      y *= scale;
      z *= scale;

      this.samples[i * 3] = x;
      this.samples[i * 3 + 1] = y;
      this.samples[i * 3 + 2] = z;
    }
  }

  /**
   * Generate random noise texture for sample rotation
   */
  private generateNoiseTexture(gl: WebGL2RenderingContext): void {
    const noiseSize = 4;
    const noiseData = new Float32Array(noiseSize * noiseSize * 3);

    for (let i = 0; i < noiseSize * noiseSize; i++) {
      // Random rotation vectors
      noiseData[i * 3] = Math.random() * 2.0 - 1.0;
      noiseData[i * 3 + 1] = Math.random() * 2.0 - 1.0;
      noiseData[i * 3 + 2] = 0.0; // Only rotate in XY plane
    }

    this.noiseTexture = new Texture(gl);
    gl.bindTexture(gl.TEXTURE_2D, this.noiseTexture.glTexture);
    gl.texImage2D(
      gl.TEXTURE_2D,
      0,
      gl.RGB16F,
      noiseSize,
      noiseSize,
      0,
      gl.RGB,
      gl.FLOAT,
      noiseData
    );
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);
  }

  /**
   * Calculate SSAO values
   */
  private calculateSSAO(
    gl: WebGL2RenderingContext,
    inputTexture: Texture
  ): void {
    if (!this.ssaoFB || !this.ssaoShader || !this.noiseTexture || !this.samples)
      return;

    gl.bindFramebuffer(gl.FRAMEBUFFER, this.ssaoFB);
    gl.viewport(0, 0, this.width, this.height);

    this.ssaoShader.use();
    this.ssaoShader.setUniformInt('uTexture', 0);
    this.ssaoShader.setUniformInt('uNoise', 1);
    this.ssaoShader.setUniform('uRadius', this.radius);
    this.ssaoShader.setUniform('uBias', this.bias);
    this.ssaoShader.setUniformInt('uSampleCount', this.sampleCount);

    // Note: Would also need to set samples array as uniform
    // this.ssaoShader.setUniform('uSamples', this.samples);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, inputTexture.glTexture);

    gl.activeTexture(gl.TEXTURE1);
    gl.bindTexture(gl.TEXTURE_2D, this.noiseTexture.glTexture);

    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

    this.ssaoShader;
  }

  /**
   * Blur SSAO to reduce noise
   */
  private blurSSAO(gl: WebGL2RenderingContext): void {
    if (!this.blurFB || !this.blurShader || !this.ssaoTexture) return;

    gl.bindFramebuffer(gl.FRAMEBUFFER, this.blurFB);
    gl.viewport(0, 0, this.width, this.height);

    this.blurShader.use();
    this.blurShader.setUniformInt('uTexture', 0);
    this.blurShader.setUniform('uTexelSize', [
      1.0 / this.width,
      1.0 / this.height,
    ]);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, this.ssaoTexture.glTexture);

    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

    this.blurShader;
  }

  /**
   * Composite SSAO with original scene
   */
  private composite(
    gl: WebGL2RenderingContext,
    inputTexture: Texture,
    outputFB: WebGLFramebuffer | null,
    width: number,
    height: number
  ): void {
    if (!this.compositeShader || !this.blurTexture) return;

    gl.bindFramebuffer(gl.FRAMEBUFFER, outputFB);
    gl.viewport(0, 0, width, height);

    this.compositeShader.use();
    this.compositeShader.setUniformInt('uScene', 0);
    this.compositeShader.setUniformInt('uSSAO', 1);
    this.compositeShader.setUniform('uIntensity', this.intensity);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, inputTexture.glTexture);

    gl.activeTexture(gl.TEXTURE1);
    gl.bindTexture(gl.TEXTURE_2D, this.blurTexture.glTexture);

    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

    this.compositeShader;
  }

  public resize(
    gl: WebGL2RenderingContext,
    width: number,
    height: number
  ): void {
    this.width = width;
    this.height = height;

    // Dispose old resources
    if (this.ssaoFB) gl.deleteFramebuffer(this.ssaoFB);
    if (this.blurFB) gl.deleteFramebuffer(this.blurFB);

    // Create SSAO buffer
    this.ssaoFB = gl.createFramebuffer();
    this.ssaoTexture = this.createRenderTexture(gl, width, height, gl.RED);
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.ssaoFB);
    gl.framebufferTexture2D(
      gl.FRAMEBUFFER,
      gl.COLOR_ATTACHMENT0,
      gl.TEXTURE_2D,
      this.ssaoTexture.glTexture,
      0
    );

    // Create blur buffer
    this.blurFB = gl.createFramebuffer();
    this.blurTexture = this.createRenderTexture(gl, width, height, gl.RED);
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.blurFB);
    gl.framebufferTexture2D(
      gl.FRAMEBUFFER,
      gl.COLOR_ATTACHMENT0,
      gl.TEXTURE_2D,
      this.blurTexture.glTexture,
      0
    );

    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
  }

  private createRenderTexture(
    gl: WebGL2RenderingContext,
    width: number,
    height: number,
    format: number
  ): Texture {
    const texture = new Texture(gl);
    gl.bindTexture(gl.TEXTURE_2D, texture.glTexture);
    gl.texImage2D(
      gl.TEXTURE_2D,
      0,
      format,
      width,
      height,
      0,
      format === gl.RED ? gl.RED : gl.RGBA,
      gl.FLOAT,
      null
    );
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    return texture;
  }

  private initializeShaders(gl: WebGL2RenderingContext): void {
    const vertexShader = `#version 300 es
      layout(location = 0) in vec2 aPosition;
      layout(location = 1) in vec2 aUV;
      out vec2 vUV;
      void main() {
        vUV = aUV;
        gl_Position = vec4(aPosition, 0.0, 1.0);
      }
    `;

    // Simplified SSAO shader (requires depth and normals)
    const ssaoFrag = `#version 300 es
      precision highp float;
      in vec2 vUV;
      out float fragColor;
      uniform sampler2D uTexture;
      uniform sampler2D uNoise;
      uniform float uRadius;
      uniform float uBias;
      uniform int uSampleCount;
      void main() {
        // Simplified placeholder - real SSAO needs depth/normal buffers
        vec4 color = texture(uTexture, vUV);
        float occlusion = 1.0;
        fragColor = occlusion;
      }
    `;

    // Blur shader
    const blurFrag = `#version 300 es
      precision highp float;
      in vec2 vUV;
      out float fragColor;
      uniform sampler2D uTexture;
      uniform vec2 uTexelSize;
      void main() {
        float result = 0.0;
        for (int x = -2; x <= 2; x++) {
          for (int y = -2; y <= 2; y++) {
            vec2 offset = vec2(float(x), float(y)) * uTexelSize;
            result += texture(uTexture, vUV + offset).r;
          }
        }
        fragColor = result / 25.0;
      }
    `;

    // Composite shader
    const compositeFrag = `#version 300 es
      precision highp float;
      in vec2 vUV;
      out vec4 fragColor;
      uniform sampler2D uScene;
      uniform sampler2D uSSAO;
      uniform float uIntensity;
      void main() {
        vec4 scene = texture(uScene, vUV);
        float occlusion = texture(uSSAO, vUV).r;
        vec3 darkened = scene.rgb * mix(1.0, occlusion, uIntensity);
        fragColor = vec4(darkened, scene.a);
      }
    `;

    this.ssaoShader = new Shader(gl);
    this.ssaoShader.compile(vertexShader, ssaoFrag);

    this.blurShader = new Shader(gl);
    this.blurShader.compile(vertexShader, blurFrag);

    this.compositeShader = new Shader(gl);
    this.compositeShader.compile(vertexShader, compositeFrag);
  }

  public dispose(gl: WebGL2RenderingContext): void {
    if (this.ssaoFB) gl.deleteFramebuffer(this.ssaoFB);
    if (this.blurFB) gl.deleteFramebuffer(this.blurFB);

    if (this.ssaoShader) this.ssaoShader.destroy();
    if (this.blurShader) this.blurShader.destroy();
    if (this.compositeShader) this.compositeShader.destroy();
  }
}
