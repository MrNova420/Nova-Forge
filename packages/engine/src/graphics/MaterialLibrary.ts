/**
 * MaterialLibrary
 *
 * Manages a collection of materials for centralized access and organization.
 */

import { Material } from './Material';

/**
 * Material library for managing multiple materials
 */
export class MaterialLibrary {
  private _materials: Map<string, Material> = new Map();

  /**
   * Add a material to the library
   * @param material - Material to add
   * @returns True if added, false if name already exists
   */
  add(material: Material): boolean {
    if (this._materials.has(material.name)) {
      console.warn(`Material "${material.name}" already exists in library`);
      return false;
    }

    this._materials.set(material.name, material);
    return true;
  }

  /**
   * Get a material from the library
   * @param name - Material name
   * @returns The material or undefined if not found
   */
  get(name: string): Material | undefined {
    return this._materials.get(name);
  }

  /**
   * Check if material exists
   * @param name - Material name
   */
  has(name: string): boolean {
    return this._materials.has(name);
  }

  /**
   * Remove a material from the library
   * @param name - Material name
   * @returns True if removed, false if not found
   */
  remove(name: string): boolean {
    return this._materials.delete(name);
  }

  /**
   * Get all material names
   */
  getNames(): string[] {
    return Array.from(this._materials.keys());
  }

  /**
   * Get all materials
   */
  getAll(): Material[] {
    return Array.from(this._materials.values());
  }

  /**
   * Get number of materials
   */
  get count(): number {
    return this._materials.size;
  }

  /**
   * Clear all materials from library
   */
  clear(): void {
    this._materials.clear();
  }

  /**
   * Clone a material in the library
   * @param name - Name of material to clone
   * @param newName - Name for the cloned material
   * @returns The cloned material or undefined if source not found
   */
  clone(name: string, newName: string): Material | undefined {
    const source = this._materials.get(name);
    if (!source) {
      console.error(`Cannot clone material "${name}": not found`);
      return undefined;
    }

    const cloned = source.clone(newName);
    this.add(cloned);
    return cloned;
  }
}
