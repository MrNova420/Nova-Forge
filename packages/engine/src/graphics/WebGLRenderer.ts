/**
 * WebGLRenderer
 *
 * WebGL 2.0 implementation of the renderer interface.
 * Handles WebGL context creation, state management, and rendering operations.
 */

import { IRenderer } from './Renderer';
import { Scene } from '../scene/Scene';
import { Camera } from './Camera';
import { RenderPipeline } from './RenderPipeline';

/**
 * WebGL 2.0 Renderer implementation
 */
export class WebGLRenderer implements IRenderer {
  private _canvas: HTMLCanvasElement | null = null;
  private _gl: WebGL2RenderingContext | null = null;
  private _pipeline: RenderPipeline | null = null;
  private _width: number = 0;
  private _height: number = 0;

  /**
   * Initialize the renderer with a canvas
   * @param canvas - The HTML canvas element to render to
   */
  async initialize(canvas: HTMLCanvasElement): Promise<void> {
    this._canvas = canvas;

    // Get WebGL 2.0 context
    const gl = canvas.getContext('webgl2', {
      alpha: true,
      depth: true,
      stencil: false,
      antialias: true,
      premultipliedAlpha: true,
      preserveDrawingBuffer: false,
      powerPreference: 'high-performance',
    });

    if (!gl) {
      throw new Error(
        'WebGL 2.0 is not supported in this browser. Please use a modern browser.'
      );
    }

    this._gl = gl;
    this._width = canvas.width;
    this._height = canvas.height;

    // Initialize render pipeline
    this._pipeline = new RenderPipeline(gl);

    // Set up initial WebGL state
    this.setupWebGLState();

    console.log('WebGL 2.0 Renderer initialized successfully');
    console.log('Vendor:', gl.getParameter(gl.VENDOR));
    console.log('Renderer:', gl.getParameter(gl.RENDERER));
    console.log('Version:', gl.getParameter(gl.VERSION));
  }

  /**
   * Set up initial WebGL state
   */
  private setupWebGLState(): void {
    if (!this._gl) return;

    const gl = this._gl;

    // Enable depth testing
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    // Enable backface culling
    gl.enable(gl.CULL_FACE);
    gl.cullFace(gl.BACK);
    gl.frontFace(gl.CCW);

    // Set clear color (dark gray)
    gl.clearColor(0.1, 0.1, 0.1, 1.0);

    // Set viewport
    gl.viewport(0, 0, this._width, this._height);
  }

  /**
   * Render a scene from a camera's perspective
   * @param scene - The scene to render
   * @param camera - The camera to render from
   */
  render(scene: Scene, camera: Camera): void {
    if (!this._gl || !this._pipeline) {
      console.error('Renderer not initialized');
      return;
    }

    // Clear the screen
    this._pipeline.clear();

    // Get all renderable entities from the scene
    // TODO: Implement proper ECS integration with component queries
    // For now, this is a placeholder that will be connected to the World/ECS system
    const entities = scene.getEntities();
    const renderQueue: Array<{
      entity: any;
      mesh: any;
      material: any;
      transform: any;
      distance: number;
    }> = [];

    // Build render queue
    // Note: This will need to be updated when ECS integration is complete
    // Currently entities don't have getComponent - this needs World integration
    // Placeholder - will be implemented when connecting to World/ComponentManager
    // for (const entity of entities) {
    //   const meshRenderer = world.getComponent(entity, MeshRenderer);
    //   const transform = world.getComponent(entity, Transform);
    //   ... render logic
    // }

    // For now, skip rendering until ECS integration is complete
    // This allows the code to compile
    console.log(
      `Scene has ${entities.length} entities (rendering not yet implemented)`
    );

    // Sort by material to minimize state changes, then by distance
    renderQueue.sort((a, b) => {
      // Opaque objects: front to back
      // Transparent objects: back to front
      const aTransparent = a.material?.transparent || false;
      const bTransparent = b.material?.transparent || false;

      if (aTransparent !== bTransparent) {
        return aTransparent ? 1 : -1;
      }

      if (aTransparent) {
        return b.distance - a.distance; // Back to front
      } else {
        return a.distance - b.distance; // Front to back
      }
    });

    // Get camera matrices
    const viewMatrix = camera.viewMatrix;
    const projectionMatrix = camera.projectionMatrix;

    // Render each object
    const gl = this._gl;
    let currentMaterial: any = null;

    for (const item of renderQueue) {
      // Bind material if changed
      if (item.material !== currentMaterial) {
        this.bindMaterial(item.material);
        currentMaterial = item.material;
      }

      // Set transform matrices
      const modelMatrix = item.transform.getWorldMatrix?.();
      if (modelMatrix && this._currentShader) {
        this.setMatrix4('uModelMatrix', modelMatrix);
        this.setMatrix4('uViewMatrix', viewMatrix);
        this.setMatrix4('uProjectionMatrix', projectionMatrix);
      }

      // Draw mesh
      if (item.mesh && item.mesh.vao) {
        gl.bindVertexArray(item.mesh.vao);
        const indexCount = item.mesh.indexCount || 0;
        if (indexCount > 0) {
          gl.drawElements(gl.TRIANGLES, indexCount, gl.UNSIGNED_SHORT, 0);
        }
        gl.bindVertexArray(null);
      }
    }

    // Check for WebGL errors
    this.checkGLError();
  }

  private _currentShader: any = null;

  private bindMaterial(material: any): void {
    if (!material || !this._gl) return;

    const gl = this._gl;

    // Get or create shader for material
    const shader = this.getOrCreateShader(material);
    if (shader && shader !== this._currentShader) {
      gl.useProgram(shader.program);
      this._currentShader = shader;
    }

    // Set material uniforms
    if (material.color) {
      this.setVector4('uColor', [
        material.color.r || 1,
        material.color.g || 1,
        material.color.b || 1,
        material.color.a || 1,
      ]);
    }

    // Bind textures
    if (material.mainTexture) {
      gl.activeTexture(gl.TEXTURE0);
      gl.bindTexture(gl.TEXTURE_2D, material.mainTexture);
      this.setInt('uMainTexture', 0);
    }
  }

  private shaderCache = new Map<string, any>();

  private getOrCreateShader(material: any): any {
    const shaderKey = material.shaderType || 'default';

    if (!this.shaderCache.has(shaderKey)) {
      // Create basic shader
      const vertexShaderSource = `#version 300 es
        in vec3 aPosition;
        in vec3 aNormal;
        in vec2 aTexCoord;
        
        uniform mat4 uModelMatrix;
        uniform mat4 uViewMatrix;
        uniform mat4 uProjectionMatrix;
        
        out vec3 vNormal;
        out vec2 vTexCoord;
        
        void main() {
          gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aPosition, 1.0);
          vNormal = mat3(uModelMatrix) * aNormal;
          vTexCoord = aTexCoord;
        }
      `;

      const fragmentShaderSource = `#version 300 es
        precision highp float;
        
        in vec3 vNormal;
        in vec2 vTexCoord;
        
        uniform vec4 uColor;
        uniform sampler2D uMainTexture;
        
        out vec4 fragColor;
        
        void main() {
          vec3 normal = normalize(vNormal);
          vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
          float diff = max(dot(normal, lightDir), 0.2);
          
          vec4 texColor = texture(uMainTexture, vTexCoord);
          fragColor = vec4(uColor.rgb * texColor.rgb * diff, uColor.a * texColor.a);
        }
      `;

      const shader = this.compileShader(
        vertexShaderSource,
        fragmentShaderSource
      );
      this.shaderCache.set(shaderKey, shader);
    }

    return this.shaderCache.get(shaderKey);
  }

  private compileShader(vertexSource: string, fragmentSource: string): any {
    if (!this._gl) return null;

    const gl = this._gl;

    // Compile vertex shader
    const vertexShader = gl.createShader(gl.VERTEX_SHADER);
    if (!vertexShader) return null;
    gl.shaderSource(vertexShader, vertexSource);
    gl.compileShader(vertexShader);

    // Compile fragment shader
    const fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
    if (!fragmentShader) return null;
    gl.shaderSource(fragmentShader, fragmentSource);
    gl.compileShader(fragmentShader);

    // Link program
    const program = gl.createProgram();
    if (!program) return null;
    gl.attachShader(program, vertexShader);
    gl.attachShader(program, fragmentShader);
    gl.linkProgram(program);

    // Cleanup shaders
    gl.deleteShader(vertexShader);
    gl.deleteShader(fragmentShader);

    return { program };
  }

  private setMatrix4(name: string, matrix: any): void {
    if (!this._currentShader || !this._gl) return;
    const location = this._gl.getUniformLocation(
      this._currentShader.program,
      name
    );
    if (location) {
      this._gl.uniformMatrix4fv(location, false, matrix);
    }
  }

  private setVector4(name: string, value: number[]): void {
    if (!this._currentShader || !this._gl) return;
    const location = this._gl.getUniformLocation(
      this._currentShader.program,
      name
    );
    if (location) {
      this._gl.uniform4fv(location, value);
    }
  }

  private setInt(name: string, value: number): void {
    if (!this._currentShader || !this._gl) return;
    const location = this._gl.getUniformLocation(
      this._currentShader.program,
      name
    );
    if (location) {
      this._gl.uniform1i(location, value);
    }
  }

  /**
   * Resize the rendering viewport
   * @param width - New width in pixels
   * @param height - New height in pixels
   */
  resize(width: number, height: number): void {
    if (!this._canvas || !this._gl) return;

    this._width = width;
    this._height = height;

    // Update canvas size
    this._canvas.width = width;
    this._canvas.height = height;

    // Update WebGL viewport
    this._gl.viewport(0, 0, width, height);

    console.log(`Renderer resized to ${width}x${height}`);
  }

  /**
   * Clean up and destroy the renderer
   */
  destroy(): void {
    if (this._pipeline) {
      this._pipeline.destroy();
      this._pipeline = null;
    }

    // Lose WebGL context
    if (this._canvas && this._gl) {
      const loseContext = this._gl.getExtension('WEBGL_lose_context');
      if (loseContext) {
        loseContext.loseContext();
      }
    }

    this._gl = null;
    this._canvas = null;

    console.log('WebGL Renderer destroyed');
  }

  /**
   * Get the underlying WebGL context
   */
  getContext(): WebGL2RenderingContext | null {
    return this._gl;
  }

  /**
   * Check for WebGL errors and log them
   */
  private checkGLError(): void {
    if (!this._gl) return;

    const error = this._gl.getError();
    if (error !== this._gl.NO_ERROR) {
      let errorString = 'Unknown error';
      switch (error) {
        case this._gl.INVALID_ENUM:
          errorString = 'INVALID_ENUM';
          break;
        case this._gl.INVALID_VALUE:
          errorString = 'INVALID_VALUE';
          break;
        case this._gl.INVALID_OPERATION:
          errorString = 'INVALID_OPERATION';
          break;
        case this._gl.OUT_OF_MEMORY:
          errorString = 'OUT_OF_MEMORY';
          break;
        case this._gl.INVALID_FRAMEBUFFER_OPERATION:
          errorString = 'INVALID_FRAMEBUFFER_OPERATION';
          break;
      }
      console.error(`WebGL Error: ${errorString} (${error})`);
    }
  }

  /**
   * Get canvas dimensions
   */
  get width(): number {
    return this._width;
  }

  get height(): number {
    return this._height;
  }
}
