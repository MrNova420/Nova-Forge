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
 * PBR (Physically-Based Rendering) Material
 * Implements metallic-roughness workflow for realistic rendering
 */

import { Texture } from '../Texture';
import { Vector3 } from '../../math/Vector3';

export interface PBRMaterialProperties {
  albedo?: Vector3;
  albedoMap?: Texture | null;
  metallic?: number;
  roughness?: number;
  metallicRoughnessMap?: Texture | null;
  normalMap?: Texture | null;
  normalScale?: number;
  occlusionMap?: Texture | null;
  occlusionStrength?: number;
  emissive?: Vector3;
  emissiveMap?: Texture | null;
  emissiveIntensity?: number;
  opacity?: number;
  alphaMode?: 'OPAQUE' | 'MASK' | 'BLEND';
  alphaCutoff?: number;
  environmentIntensity?: number;
}

export class PBRMaterial {
  private _albedo: Vector3;
  private _albedoMap: Texture | null;
  private _metallic: number;
  private _roughness: number;
  private _metallicRoughnessMap: Texture | null;
  private _normalMap: Texture | null;
  private _normalScale: number;
  private _occlusionMap: Texture | null;
  private _occlusionStrength: number;
  private _emissive: Vector3;
  private _emissiveMap: Texture | null;
  private _emissiveIntensity: number;
  private _opacity: number;
  private _alphaMode: 'OPAQUE' | 'MASK' | 'BLEND';
  private _alphaCutoff: number;
  private _environmentIntensity: number;

  constructor(properties: PBRMaterialProperties = {}) {
    
    this._albedo = properties.albedo || new Vector3(1, 1, 1);
    this._albedoMap = properties.albedoMap || null;
    this._metallic = properties.metallic !== undefined ? properties.metallic : 0.0;
    this._roughness = properties.roughness !== undefined ? properties.roughness : 0.5;
    this._metallicRoughnessMap = properties.metallicRoughnessMap || null;
    this._normalMap = properties.normalMap || null;
    this._normalScale = properties.normalScale !== undefined ? properties.normalScale : 1.0;
    this._occlusionMap = properties.occlusionMap || null;
    this._occlusionStrength = properties.occlusionStrength !== undefined ? properties.occlusionStrength : 1.0;
    this._emissive = properties.emissive || new Vector3(0, 0, 0);
    this._emissiveMap = properties.emissiveMap || null;
    this._emissiveIntensity = properties.emissiveIntensity !== undefined ? properties.emissiveIntensity : 1.0;
    this._opacity = properties.opacity !== undefined ? properties.opacity : 1.0;
    this._alphaMode = properties.alphaMode || 'OPAQUE';
    this._alphaCutoff = properties.alphaCutoff !== undefined ? properties.alphaCutoff : 0.5;
    this._environmentIntensity = properties.environmentIntensity !== undefined ? properties.environmentIntensity : 1.0;
    
    this.updateShaderProperties();
  }

  private updateShaderProperties(): void {
    // Properties are updated through setters
    // This method is called after initialization
  }

  get albedo(): Vector3 { return this._albedo; }
  set albedo(value: Vector3) { this._albedo = value; }
  
  get albedoMap(): Texture | null { return this._albedoMap; }
  set albedoMap(value: Texture | null) { this._albedoMap = value; }
  
  get metallic(): number { return this._metallic; }
  set metallic(value: number) { this._metallic = Math.max(0, Math.min(1, value)); }
  
  get roughness(): number { return this._roughness; }
  set roughness(value: number) { this._roughness = Math.max(0, Math.min(1, value)); }
  
  get metallicRoughnessMap(): Texture | null { return this._metallicRoughnessMap; }
  set metallicRoughnessMap(value: Texture | null) { this._metallicRoughnessMap = value; }
  
  get normalMap(): Texture | null { return this._normalMap; }
  set normalMap(value: Texture | null) { this._normalMap = value; }
  
  get normalScale(): number { return this._normalScale; }
  set normalScale(value: number) { this._normalScale = value; }
  
  get occlusionMap(): Texture | null { return this._occlusionMap; }
  set occlusionMap(value: Texture | null) { this._occlusionMap = value; }
  
  get occlusionStrength(): number { return this._occlusionStrength; }
  set occlusionStrength(value: number) { this._occlusionStrength = Math.max(0, Math.min(1, value)); }
  
  get emissive(): Vector3 { return this._emissive; }
  set emissive(value: Vector3) { this._emissive = value; }
  
  get emissiveMap(): Texture | null { return this._emissiveMap; }
  set emissiveMap(value: Texture | null) { this._emissiveMap = value; }
  
  get emissiveIntensity(): number { return this._emissiveIntensity; }
  set emissiveIntensity(value: number) { this._emissiveIntensity = Math.max(0, value); }
  
  get opacity(): number { return this._opacity; }
  set opacity(value: number) { this._opacity = Math.max(0, Math.min(1, value)); }
  
  get alphaMode(): 'OPAQUE' | 'MASK' | 'BLEND' { return this._alphaMode; }
  set alphaMode(value: 'OPAQUE' | 'MASK' | 'BLEND') { this._alphaMode = value; }
  
  get alphaCutoff(): number { return this._alphaCutoff; }
  set alphaCutoff(value: number) { this._alphaCutoff = Math.max(0, Math.min(1, value)); }
  
  get environmentIntensity(): number { return this._environmentIntensity; }
  set environmentIntensity(value: number) { this._environmentIntensity = Math.max(0, value); }
}

export function createPBRMaterial(properties: PBRMaterialProperties = {}): PBRMaterial {
  return new PBRMaterial(properties);
}
