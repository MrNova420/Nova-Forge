/**
 * Material
 *
 * Represents a material that defines how a surface should be rendered.
 * Materials contain a shader and properties (uniforms and textures).
 */

import { Shader, UniformValue } from './Shader';
import { Texture } from './Texture';
import { Vector3 } from '../math/Vector3';
import { Vector4 } from '../math/Vector4';

/**
 * Material property value types
 */
export type MaterialPropertyValue = UniformValue | Texture;

/**
 * Material class for managing rendering properties
 */
export class Material {
  private _shader: Shader;
  private _properties: Map<string, UniformValue> = new Map();
  private _textures: Map<string, { texture: Texture; unit: number }> =
    new Map();
  private _name: string;
  private _nextTextureUnit: number = 0;

  /**
   * Create a new material
   * @param shader - Shader to use for this material
   * @param name - Material name
   */
  constructor(shader: Shader, name: string = 'Unnamed Material') {
    this._shader = shader;
    this._name = name;
  }

  /**
   * Get material name
   */
  get name(): string {
    return this._name;
  }

  /**
   * Set material name
   */
  set name(value: string) {
    this._name = value;
  }

  /**
   * Get the shader
   */
  get shader(): Shader {
    return this._shader;
  }

  /**
   * Set the shader
   */
  set shader(value: Shader) {
    this._shader = value;
  }

  /**
   * Set a material property
   * @param name - Property name (uniform name)
   * @param value - Property value
   */
  setProperty(name: string, value: UniformValue): void {
    this._properties.set(name, value);
  }

  /**
   * Get a material property
   * @param name - Property name
   */
  getProperty(name: string): UniformValue | undefined {
    return this._properties.get(name);
  }

  /**
   * Check if property exists
   * @param name - Property name
   */
  hasProperty(name: string): boolean {
    return this._properties.has(name);
  }

  /**
   * Set a texture
   * @param name - Texture uniform name
   * @param texture - Texture to set
   * @param unit - Texture unit (optional, auto-assigned if not provided)
   */
  setTexture(name: string, texture: Texture, unit?: number): void {
    const textureUnit = unit !== undefined ? unit : this._nextTextureUnit++;
    this._textures.set(name, { texture, unit: textureUnit });
  }

  /**
   * Get a texture
   * @param name - Texture name
   */
  getTexture(name: string): Texture | undefined {
    return this._textures.get(name)?.texture;
  }

  /**
   * Check if texture exists
   * @param name - Texture name
   */
  hasTexture(name: string): boolean {
    return this._textures.has(name);
  }

  /**
   * Bind material (activate shader and set uniforms/textures)
   */
  bind(): void {
    // Use the shader
    this._shader.use();

    // Set all properties as uniforms
    for (const [name, value] of this._properties) {
      this._shader.setUniform(name, value);
    }

    // Bind all textures
    for (const [name, { texture, unit }] of this._textures) {
      texture.bind(unit);
      this._shader.setUniformInt(name, unit);
    }
  }

  /**
   * Unbind material (unbind textures)
   */
  unbind(): void {
    // Unbind all textures
    for (const { texture } of this._textures.values()) {
      texture.unbind();
    }
  }

  /**
   * Clone the material
   * @param newName - Name for the cloned material
   */
  clone(newName?: string): Material {
    const cloned = new Material(
      this._shader,
      newName || `${this._name} (Clone)`
    );

    // Copy properties
    for (const [name, value] of this._properties) {
      cloned.setProperty(name, value);
    }

    // Copy texture references (not cloning the textures themselves)
    for (const [name, { texture, unit }] of this._textures) {
      cloned.setTexture(name, texture, unit);
    }

    return cloned;
  }

  /**
   * Get all property names
   */
  getPropertyNames(): string[] {
    return Array.from(this._properties.keys());
  }

  /**
   * Get all texture names
   */
  getTextureNames(): string[] {
    return Array.from(this._textures.keys());
  }
}

/**
 * Create a standard PBR-style material with common properties
 */
export function createStandardMaterial(
  shader: Shader,
  name: string = 'Standard Material'
): Material {
  const material = new Material(shader, name);

  // Set default PBR properties
  material.setProperty('u_baseColor', new Vector4(1, 1, 1, 1));
  material.setProperty('u_lightDirection', new Vector3(0, -1, 0));
  material.setProperty('u_lightColor', new Vector3(1, 1, 1));
  material.setProperty('u_ambientColor', new Vector3(0.2, 0.2, 0.2));
  material.setProperty('u_cameraPosition', new Vector3(0, 0, 5));

  return material;
}

/**
 * Create an unlit material with a solid color
 */
export function createUnlitMaterial(
  shader: Shader,
  color: Vector4 = new Vector4(1, 1, 1, 1),
  name: string = 'Unlit Material'
): Material {
  const material = new Material(shader, name);
  material.setProperty('u_baseColor', color);
  return material;
}
