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
 * Graphics Module
 *
 * Exports graphics-related classes and interfaces
 */

export type { IRenderer } from './Renderer';
export { WebGLRenderer } from './WebGLRenderer';
export { RenderPipeline } from './RenderPipeline';
export { Camera, CameraProjectionType } from './Camera';
export { Shader } from './Shader';
export type { UniformValue } from './Shader';
export { ShaderLibrary } from './ShaderLibrary';
export type { ShaderSource } from './ShaderLibrary';
export { Texture, TextureWrap, TextureFilter, TextureFormat } from './Texture';
export { TextureLoader } from './TextureLoader';
export type { TextureLoadOptions } from './TextureLoader';
export { TextureCache } from './TextureCache';
export {
  Material,
  createStandardMaterial,
  createUnlitMaterial,
} from './Material';
export type { MaterialPropertyValue } from './Material';
export { MaterialLibrary } from './MaterialLibrary';
export { Mesh } from './Mesh';
export { MeshRenderer } from './MeshRenderer';
export {
  createCube,
  createPlane,
  createSphere,
  createCylinder,
  createCone,
} from './Primitives';
export {
  OrbitCameraController,
  FirstPersonCameraController,
  FlyCameraController,
} from './CameraController';
export type { ICameraController } from './CameraController';
export { Light, LightType } from './Light';
export { DirectionalLight } from './DirectionalLight';
export { PointLight } from './PointLight';
export { SpotLight } from './SpotLight';
export { LightingSystem } from './LightingSystem';
export * from './materials';
export * from './shaders';
export * from './postprocessing';
