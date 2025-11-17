/**
 * Post-Processing Stack
 * Manages and applies a stack of post-processing effects to the rendered scene
 */

import { Texture } from '../Texture';

/**
 * Base interface for all post-processing effects
 */
export interface PostProcessingEffect {
  /**
   * The name of the effect
   */
  readonly name: string;

  /**
   * Whether the effect is currently enabled
   */
  enabled: boolean;

  /**
   * Apply the effect to the input texture
   * @param gl - WebGL context
   * @param inputTexture - Input texture to process
   * @param outputFramebuffer - Output framebuffer (null for screen)
   * @param width - Output width
   * @param height - Output height
   */
  apply(
    gl: WebGL2RenderingContext,
    inputTexture: Texture,
    outputFramebuffer: WebGLFramebuffer | null,
    width: number,
    height: number
  ): void;

  /**
   * Resize the effect's internal buffers
   * @param gl - WebGL context
   * @param width - New width
   * @param height - New height
   */
  resize(gl: WebGL2RenderingContext, width: number, height: number): void;

  /**
   * Clean up resources
   * @param gl - WebGL context
   */
  dispose(gl: WebGL2RenderingContext): void;
}

/**
 * Framebuffer for intermediate rendering
 */
class RenderTarget {
  public framebuffer: WebGLFramebuffer;
  public texture: Texture;
  public width: number;
  public height: number;

  constructor(
    gl: WebGL2RenderingContext,
    width: number,
    height: number,
    internalFormat: number = gl.RGBA16F
  ) {
    this.width = width;
    this.height = height;

    // Create framebuffer
    const fb = gl.createFramebuffer();
    if (!fb) throw new Error('Failed to create framebuffer');
    this.framebuffer = fb;

    // Create texture
    this.texture = new Texture(gl);
    gl.bindTexture(gl.TEXTURE_2D, this.texture.glTexture);
    gl.texImage2D(
      gl.TEXTURE_2D,
      0,
      internalFormat,
      width,
      height,
      0,
      gl.RGBA,
      gl.FLOAT,
      null
    );
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

    // Attach texture to framebuffer
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.framebuffer);
    gl.framebufferTexture2D(
      gl.FRAMEBUFFER,
      gl.COLOR_ATTACHMENT0,
      gl.TEXTURE_2D,
      this.texture.glTexture,
      0
    );

    // Check framebuffer status
    const status = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
    if (status !== gl.FRAMEBUFFER_COMPLETE) {
      throw new Error(`Framebuffer incomplete: ${status}`);
    }

    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
  }

  public resize(gl: WebGL2RenderingContext, width: number, height: number): void {
    this.width = width;
    this.height = height;

    gl.bindTexture(gl.TEXTURE_2D, this.texture.glTexture);
    gl.texImage2D(
      gl.TEXTURE_2D,
      0,
      gl.RGBA16F,
      width,
      height,
      0,
      gl.RGBA,
      gl.FLOAT,
      null
    );
  }

  public dispose(gl: WebGL2RenderingContext): void {
    gl.deleteFramebuffer(this.framebuffer);
  }
}

/**
 * Post-Processing Stack
 * Manages multiple post-processing effects and applies them in sequence
 */
export class PostProcessingStack {
  private effects: PostProcessingEffect[] = [];
  private renderTargets: RenderTarget[] = [];
  private width: number = 0;
  private height: number = 0;
  private quadVAO: WebGLVertexArrayObject | null = null;
  private quadVBO: WebGLBuffer | null = null;

  /**
   * Initialize the post-processing stack
   * @param gl - WebGL context
   * @param width - Initial width
   * @param height - Initial height
   */
  public initialize(
    gl: WebGL2RenderingContext,
    width: number,
    height: number
  ): void {
    this.width = width;
    this.height = height;

    // Create full-screen quad for rendering effects
    this.createQuad(gl);

    // Create render targets for ping-pong rendering
    this.renderTargets.push(new RenderTarget(gl, width, height));
    this.renderTargets.push(new RenderTarget(gl, width, height));
  }

  /**
   * Add a post-processing effect to the stack
   * @param effect - Effect to add
   */
  public addEffect(effect: PostProcessingEffect): void {
    this.effects.push(effect);
  }

  /**
   * Remove a post-processing effect from the stack
   * @param effect - Effect to remove
   */
  public removeEffect(effect: PostProcessingEffect): void {
    const index = this.effects.indexOf(effect);
    if (index !== -1) {
      this.effects.splice(index, 1);
    }
  }

  /**
   * Get an effect by name
   * @param name - Name of the effect
   * @returns The effect or undefined
   */
  public getEffect(name: string): PostProcessingEffect | undefined {
    return this.effects.find((effect) => effect.name === name);
  }

  /**
   * Apply all enabled effects in the stack
   * @param gl - WebGL context
   * @param inputTexture - Input scene texture
   */
  public apply(gl: WebGL2RenderingContext, inputTexture: Texture): void {
    const enabledEffects = this.effects.filter((effect) => effect.enabled);
    if (enabledEffects.length === 0) {
      // No effects enabled, just render input to screen
      this.renderToScreen(gl, inputTexture);
      return;
    }

    let currentInput = inputTexture;
    let targetIndex = 0;

    // Apply each effect in sequence
    for (let i = 0; i < enabledEffects.length; i++) {
      const effect = enabledEffects[i];
      if (!effect) continue;

      const isLastEffect = i === enabledEffects.length - 1;
      const renderTarget = this.renderTargets[targetIndex];
      if (!renderTarget) continue;

      const outputFB = isLastEffect ? null : renderTarget.framebuffer;

      effect.apply(gl, currentInput, outputFB, this.width, this.height);

      // Ping-pong between render targets
      if (!isLastEffect) {
        currentInput = renderTarget.texture;
        targetIndex = 1 - targetIndex;
      }
    }
  }

  /**
   * Resize all render targets and effects
   * @param gl - WebGL context
   * @param width - New width
   * @param height - New height
   */
  public resize(gl: WebGL2RenderingContext, width: number, height: number): void {
    this.width = width;
    this.height = height;

    // Resize render targets
    for (const rt of this.renderTargets) {
      rt.resize(gl, width, height);
    }

    // Resize effects
    for (const effect of this.effects) {
      effect.resize(gl, width, height);
    }
  }

  /**
   * Clean up all resources
   * @param gl - WebGL context
   */
  public dispose(gl: WebGL2RenderingContext): void {
    // Dispose effects
    for (const effect of this.effects) {
      effect.dispose(gl);
    }
    this.effects = [];

    // Dispose render targets
    for (const rt of this.renderTargets) {
      if (rt.framebuffer) {
        gl.deleteFramebuffer(rt.framebuffer);
      }
      // Note: Texture doesn't have a dispose method
    }
    this.renderTargets = [];

    // Dispose quad
    if (this.quadVAO) gl.deleteVertexArray(this.quadVAO);
    if (this.quadVBO) gl.deleteBuffer(this.quadVBO);
  }

  /**
   * Create full-screen quad for rendering
   */
  private createQuad(gl: WebGL2RenderingContext): void {
    // Full-screen quad vertices (position + UV)
    const vertices = new Float32Array([
      -1, -1, 0, 0,
      1, -1, 1, 0,
      -1, 1, 0, 1,
      1, 1, 1, 1,
    ]);

    this.quadVBO = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.quadVBO);
    gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

    this.quadVAO = gl.createVertexArray();
    gl.bindVertexArray(this.quadVAO);

    // Position attribute
    gl.enableVertexAttribArray(0);
    gl.vertexAttribPointer(0, 2, gl.FLOAT, false, 16, 0);

    // UV attribute
    gl.enableVertexAttribArray(1);
    gl.vertexAttribPointer(1, 2, gl.FLOAT, false, 16, 8);

    gl.bindVertexArray(null);
  }

  /**
   * Render a texture to the screen using a simple blit shader
   */
  private renderToScreen(gl: WebGL2RenderingContext, texture: Texture): void {
    // Create simple pass-through shader if needed
    const vertexShader = `#version 300 es
      layout(location = 0) in vec2 aPosition;
      layout(location = 1) in vec2 aTexCoord;
      out vec2 vTexCoord;
      void main() {
        vTexCoord = aTexCoord;
        gl_Position = vec4(aPosition, 0.0, 1.0);
      }
    `;
    
    const fragmentShader = `#version 300 es
      precision highp float;
      in vec2 vTexCoord;
      uniform sampler2D uTexture;
      out vec4 fragColor;
      void main() {
        fragColor = texture(uTexture, vTexCoord);
      }
    `;
    
    // Compile shaders (simplified - in real implementation would cache this)
    const vs = gl.createShader(gl.VERTEX_SHADER);
    const fs = gl.createShader(gl.FRAGMENT_SHADER);
    
    if (!vs || !fs) return;
    
    gl.shaderSource(vs, vertexShader);
    gl.shaderSource(fs, fragmentShader);
    gl.compileShader(vs);
    gl.compileShader(fs);
    
    const program = gl.createProgram();
    if (!program) return;
    
    gl.attachShader(program, vs);
    gl.attachShader(program, fs);
    gl.linkProgram(program);
    
    // Use program and render
    gl.useProgram(program);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    gl.viewport(0, 0, this.width, this.height);
    
    // Bind texture
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, texture.glTexture);
    gl.uniform1i(gl.getUniformLocation(program, 'uTexture'), 0);
    
    // Draw quad
    this.bindQuad(gl);
    this.drawQuad(gl);
    
    // Cleanup
    gl.deleteShader(vs);
    gl.deleteShader(fs);
    gl.deleteProgram(program);
  }

  /**
   * Bind the full-screen quad for rendering
   */
  public bindQuad(gl: WebGL2RenderingContext): void {
    if (this.quadVAO) {
      gl.bindVertexArray(this.quadVAO);
    }
  }

  /**
   * Draw the full-screen quad
   */
  public drawQuad(gl: WebGL2RenderingContext): void {
    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
  }
}
