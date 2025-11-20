/**
 * @file AssetStreamingIntegration.ts
 * @description Comprehensive asset streaming integration layer
 * Connects streaming manager with all asset types for seamless loading
 */

import { IStreamingManager, StreamingRequest, StreamedResource, StreamingPriority, ResourceType, LODLevel } from './IResourceStreaming';
import { IAllocator } from '../memory/IAllocator';
import { JobScheduler } from '../jobs/JobScheduler';
import { AsyncLoadJob } from '../jobs/AsyncLoadJob';
import { ParallelForJob } from '../jobs/ParallelForJob';

/**
 * Mesh data structure
 */
export interface Mesh {
  vertices: Float32Array;
  indices: Uint32Array;
  normals?: Float32Array;
  uvs?: Float32Array;
  tangents?: Float32Array;
  bounds: BoundingBox;
  lodLevel: LODLevel;
}

/**
 * Texture data structure
 */
export interface Texture {
  width: number;
  height: number;
  data: Uint8Array;
  format: TextureFormat;
  mipmaps: Uint8Array[];
  compressed: boolean;
}

/**
 * Audio buffer structure
 */
export interface AudioBuffer {
  samples: Float32Array;
  sampleRate: number;
  channels: number;
  duration: number;
}

/**
 * Scene data structure
 */
export interface SceneData {
  entities: Entity[];
  lights: Light[];
  cameras: Camera[];
  metadata: Record<string, any>;
}

export interface Entity {
  id: string;
  position: Vector3;
  rotation: Quaternion;
  scale: Vector3;
  meshId?: string;
  materialId?: string;
}

export interface Light {
  type: 'point' | 'directional' | 'spot';
  position: Vector3;
  color: Vector3;
  intensity: number;
}

export interface Camera {
  position: Vector3;
  target: Vector3;
  fov: number;
}

export interface Vector3 {
  x: number;
  y: number;
  z: number;
}

export interface Quaternion {
  x: number;
  y: number;
  z: number;
  w: number;
}

export interface BoundingBox {
  min: Vector3;
  max: Vector3;
}

export enum TextureFormat {
  RGBA8,
  RGB8,
  RGBA16F,
  RGB16F,
  BC1,
  BC3,
  BC5,
  BC7,
  ETC2,
  ASTC,
}

/**
 * Mesh streaming system - handles mesh loading with LOD selection
 */
export class MeshStreamingSystem {
  private loadedMeshes: Map<string, Mesh> = new Map();
  private meshLODs: Map<string, Map<LODLevel, string>> = new Map();
  private memoryUsage: number = 0;
  private maxMemoryBudget: number;

  constructor(
    private streamingManager: IStreamingManager,
    private allocator: IAllocator,
    private jobScheduler: JobScheduler,
    memoryBudgetMB: number = 128
  ) {
    this.maxMemoryBudget = memoryBudgetMB * 1024 * 1024;
  }

  /**
   * Stream a mesh with automatic LOD selection based on distance
   */
  async streamMesh(meshId: string, distance: number): Promise<Mesh> {
    const lod = this.selectLOD(distance);
    const lodMeshId = this.getLODMeshId(meshId, lod);

    // Check if already loaded
    if (this.loadedMeshes.has(lodMeshId)) {
      return this.loadedMeshes.get(lodMeshId)!;
    }

    // Request streaming
    const resource = await this.streamingManager.request({
      id: lodMeshId,
      type: ResourceType.MESH,
      priority: this.calculatePriority(distance),
      lodLevel: lod,
      distanceFromCamera: distance,
      onProgress: (loaded, total) => {
        // Progress callback
      },
    });

    const mesh = await this.loadMeshData(resource);
    this.loadedMeshes.set(lodMeshId, mesh);
    this.memoryUsage += this.calculateMeshSize(mesh);

    return mesh;
  }

  /**
   * Batch stream multiple meshes
   */
  async streamMeshBatch(meshIds: string[], distances: number[]): Promise<Map<string, Mesh>> {
    if (meshIds.length !== distances.length) {
      throw new Error('Mesh IDs and distances arrays must have same length');
    }

    const jobs = meshIds.map((meshId, i) => {
      const lod = this.selectLOD(distances[i]);
      const lodMeshId = this.getLODMeshId(meshId, lod);

      return new AsyncLoadJob({
        name: `Load Mesh ${lodMeshId}`,
        url: `/assets/meshes/${lodMeshId}.mesh`,
        loader: async (url: string) => {
          return await this.streamMesh(meshId, distances[i]);
        },
        priority: this.calculatePriority(distances[i]),
      });
    });

    const results = await Promise.all(
      jobs.map(job => this.jobScheduler.submit(job))
    );

    const meshMap = new Map<string, Mesh>();
    meshIds.forEach((meshId, i) => {
      if (results[i].success) {
        meshMap.set(meshId, results[i].data as Mesh);
      }
    });

    return meshMap;
  }

  /**
   * Unload distant meshes to free memory
   */
  unloadDistantMeshes(cameraPos: Vector3, maxDistance: number): void {
    const toUnload: string[] = [];

    for (const [meshId, mesh] of this.loadedMeshes) {
      const distance = this.calculateDistance(mesh.bounds, cameraPos);
      if (distance > maxDistance) {
        toUnload.push(meshId);
      }
    }

    for (const meshId of toUnload) {
      this.unloadMesh(meshId);
    }
  }

  /**
   * Enforce memory budget by unloading low-priority meshes
   */
  private enforceMeshMemoryBudget(): void {
    if (this.memoryUsage <= this.maxMemoryBudget) {
      return;
    }

    // Sort meshes by priority (distance-based)
    const meshesWithPriority = Array.from(this.loadedMeshes.entries()).map(([id, mesh]) => ({
      id,
      mesh,
      priority: this.calculateMeshPriority(mesh),
    }));

    meshesWithPriority.sort((a, b) => a.priority - b.priority);

    // Unload until under budget
    for (const { id } of meshesWithPriority) {
      if (this.memoryUsage <= this.maxMemoryBudget * 0.9) {
        break;
      }
      this.unloadMesh(id);
    }
  }

  private unloadMesh(meshId: string): void {
    const mesh = this.loadedMeshes.get(meshId);
    if (!mesh) return;

    this.memoryUsage -= this.calculateMeshSize(mesh);
    this.loadedMeshes.delete(meshId);
    this.streamingManager.unload(meshId);
  }

  private selectLOD(distance: number): LODLevel {
    if (distance < 10) return LODLevel.LOD0;
    if (distance < 30) return LODLevel.LOD1;
    if (distance < 60) return LODLevel.LOD2;
    if (distance < 100) return LODLevel.LOD3;
    return LODLevel.LOD4;
  }

  private getLODMeshId(baseMeshId: string, lod: LODLevel): string {
    return `${baseMeshId}_lod${lod}`;
  }

  private calculatePriority(distance: number): StreamingPriority {
    if (distance < 10) return StreamingPriority.CRITICAL;
    if (distance < 30) return StreamingPriority.HIGH;
    if (distance < 60) return StreamingPriority.NORMAL;
    if (distance < 100) return StreamingPriority.LOW;
    return StreamingPriority.BACKGROUND;
  }

  private calculateMeshSize(mesh: Mesh): number {
    let size = 0;
    size += mesh.vertices.byteLength;
    size += mesh.indices.byteLength;
    if (mesh.normals) size += mesh.normals.byteLength;
    if (mesh.uvs) size += mesh.uvs.byteLength;
    if (mesh.tangents) size += mesh.tangents.byteLength;
    return size;
  }

  private calculateMeshPriority(mesh: Mesh): number {
    // Lower number = higher priority
    // This is a placeholder - should be based on actual camera distance
    return 0;
  }

  private calculateDistance(bounds: BoundingBox, point: Vector3): number {
    const center = {
      x: (bounds.min.x + bounds.max.x) / 2,
      y: (bounds.min.y + bounds.max.y) / 2,
      z: (bounds.min.z + bounds.max.z) / 2,
    };

    const dx = center.x - point.x;
    const dy = center.y - point.y;
    const dz = center.z - point.z;

    return Math.sqrt(dx * dx + dy * dy + dz * dz);
  }

  private async loadMeshData(resource: StreamedResource): Promise<Mesh> {
    // Placeholder - would actually load from resource.data
    return {
      vertices: new Float32Array(0),
      indices: new Uint32Array(0),
      bounds: { min: { x: 0, y: 0, z: 0 }, max: { x: 0, y: 0, z: 0 } },
      lodLevel: LODLevel.LOD0,
    };
  }

  getStats(): { loaded: number; memoryUsage: number; memoryBudget: number } {
    return {
      loaded: this.loadedMeshes.size,
      memoryUsage: this.memoryUsage,
      memoryBudget: this.maxMemoryBudget,
    };
  }
}

/**
 * Texture streaming system - handles texture loading with mipmap selection
 */
export class TextureStreamingSystem {
  private loadedTextures: Map<string, Texture> = new Map();
  private textureMipmaps: Map<string, Map<number, string>> = new Map();
  private memoryUsage: number = 0;
  private maxMemoryBudget: number;

  constructor(
    private streamingManager: IStreamingManager,
    private allocator: IAllocator,
    private jobScheduler: JobScheduler,
    memoryBudgetMB: number = 256
  ) {
    this.maxMemoryBudget = memoryBudgetMB * 1024 * 1024;
  }

  /**
   * Stream texture with automatic mipmap selection
   */
  async streamTexture(textureId: string, distance: number): Promise<Texture> {
    const mipLevel = this.selectMipLevel(distance);
    
    // Check if already loaded
    if (this.loadedTextures.has(textureId)) {
      const texture = this.loadedTextures.get(textureId)!;
      if (texture.mipmaps.length > mipLevel) {
        return texture;
      }
    }

    const resource = await this.streamingManager.request({
      id: textureId,
      type: ResourceType.TEXTURE,
      priority: this.calculatePriority(distance),
      lodLevel: this.mipLevelToLOD(mipLevel),
      distanceFromCamera: distance,
    });

    const texture = await this.loadTextureData(resource);
    this.loadedTextures.set(textureId, texture);
    this.memoryUsage += this.calculateTextureSize(texture);

    return texture;
  }

  /**
   * Progressive texture loading - load from low to high resolution
   */
  async streamTextureProgressive(
    textureId: string,
    onProgress?: (loaded: number, total: number) => void
  ): Promise<Texture> {
    const maxMipLevels = 8;
    
    // Load lowest quality first
    let texture = await this.streamTexture(textureId, 1000);
    if (onProgress) onProgress(1, maxMipLevels);

    // Progressively load higher quality mipmaps
    for (let mip = maxMipLevels - 2; mip >= 0; mip--) {
      await this.streamMipLevel(textureId, mip);
      if (onProgress) onProgress(maxMipLevels - mip, maxMipLevels);
    }

    return this.loadedTextures.get(textureId)!;
  }

  /**
   * Stream individual mipmap level
   */
  async streamMipLevel(textureId: string, mipLevel: number): Promise<void> {
    const texture = this.loadedTextures.get(textureId);
    if (!texture) {
      throw new Error(`Texture not loaded: ${textureId}`);
    }

    // Stream the specific mip level
    const mipResource = await this.streamingManager.request({
      id: `${textureId}_mip${mipLevel}`,
      type: ResourceType.TEXTURE,
      priority: StreamingPriority.NORMAL,
      lodLevel: this.mipLevelToLOD(mipLevel),
      distanceFromCamera: 0,
    });

    // Update texture with new mip level data
    // Placeholder implementation
  }

  /**
   * Virtual texturing - stream texture tiles for mega textures
   */
  async streamVirtualTexture(vtId: string, region: { x: number; y: number; width: number; height: number }): Promise<Uint8Array> {
    const tileSize = 256;
    const tiles = this.calculateRequiredTiles(region, tileSize);

    const tileJobs = tiles.map(tile => {
      return new AsyncLoadJob({
        name: `Load VT Tile ${tile.x},${tile.y}`,
        url: `/assets/vt/${vtId}/${tile.x}_${tile.y}.tile`,
        loader: async (url: string) => {
          // Load tile data
          return new Uint8Array(tileSize * tileSize * 4);
        },
        priority: StreamingPriority.HIGH,
      });
    });

    const results = await Promise.all(
      tileJobs.map(job => this.jobScheduler.submit(job))
    );

    // Assemble tiles into complete texture region
    return this.assembleTiles(results.map(r => r.data as Uint8Array), region, tileSize);
  }

  /**
   * Unload low-priority textures under memory pressure
   */
  private evictLowPriorityTextures(): void {
    if (this.memoryUsage <= this.maxMemoryBudget) {
      return;
    }

    const texturesWithPriority = Array.from(this.loadedTextures.entries()).map(([id, texture]) => ({
      id,
      texture,
      priority: this.calculateTexturePriority(texture),
    }));

    texturesWithPriority.sort((a, b) => a.priority - b.priority);

    for (const { id } of texturesWithPriority) {
      if (this.memoryUsage <= this.maxMemoryBudget * 0.9) {
        break;
      }
      this.unloadTexture(id);
    }
  }

  private unloadTexture(textureId: string): void {
    const texture = this.loadedTextures.get(textureId);
    if (!texture) return;

    this.memoryUsage -= this.calculateTextureSize(texture);
    this.loadedTextures.delete(textureId);
    this.streamingManager.unload(textureId);
  }

  private selectMipLevel(distance: number): number {
    // Screen space error calculation - simplified
    const screenSpaceSize = 1024 / distance;
    return Math.max(0, Math.min(7, Math.floor(Math.log2(1024 / screenSpaceSize))));
  }

  private mipLevelToLOD(mipLevel: number): LODLevel {
    if (mipLevel <= 1) return LODLevel.LOD0;
    if (mipLevel <= 3) return LODLevel.LOD1;
    if (mipLevel <= 5) return LODLevel.LOD2;
    if (mipLevel <= 7) return LODLevel.LOD3;
    return LODLevel.LOD4;
  }

  private calculatePriority(distance: number): StreamingPriority {
    if (distance < 10) return StreamingPriority.CRITICAL;
    if (distance < 30) return StreamingPriority.HIGH;
    if (distance < 60) return StreamingPriority.NORMAL;
    if (distance < 100) return StreamingPriority.LOW;
    return StreamingPriority.BACKGROUND;
  }

  private calculateTextureSize(texture: Texture): number {
    let size = texture.data.byteLength;
    for (const mip of texture.mipmaps) {
      size += mip.byteLength;
    }
    return size;
  }

  private calculateTexturePriority(texture: Texture): number {
    return 0; // Placeholder
  }

  private calculateRequiredTiles(region: { x: number; y: number; width: number; height: number }, tileSize: number): Array<{ x: number; y: number }> {
    const tiles: Array<{ x: number; y: number }> = [];
    const startX = Math.floor(region.x / tileSize);
    const startY = Math.floor(region.y / tileSize);
    const endX = Math.ceil((region.x + region.width) / tileSize);
    const endY = Math.ceil((region.y + region.height) / tileSize);

    for (let y = startY; y < endY; y++) {
      for (let x = startX; x < endX; x++) {
        tiles.push({ x, y });
      }
    }

    return tiles;
  }

  private assembleTiles(tiles: Uint8Array[], region: { x: number; y: number; width: number; height: number }, tileSize: number): Uint8Array {
    // Placeholder - would actually assemble tiles
    return new Uint8Array(region.width * region.height * 4);
  }

  private async loadTextureData(resource: StreamedResource): Promise<Texture> {
    // Placeholder
    return {
      width: 1024,
      height: 1024,
      data: new Uint8Array(1024 * 1024 * 4),
      format: TextureFormat.RGBA8,
      mipmaps: [],
      compressed: false,
    };
  }

  getStats(): { loaded: number; memoryUsage: number; memoryBudget: number } {
    return {
      loaded: this.loadedTextures.size,
      memoryUsage: this.memoryUsage,
      memoryBudget: this.maxMemoryBudget,
    };
  }
}

/**
 * Audio streaming system - handles audio loading with quality selection
 */
export class AudioStreamingSystem {
  private loadedAudio: Map<string, AudioBuffer> = new Map();
  private streamingAudio: Map<string, AudioStream> = new Map();
  private memoryUsage: number = 0;
  private maxMemoryBudget: number;

  constructor(
    private streamingManager: IStreamingManager,
    private allocator: IAllocator,
    private jobScheduler: JobScheduler,
    memoryBudgetMB: number = 64
  ) {
    this.maxMemoryBudget = memoryBudgetMB * 1024 * 1024;
  }

  /**
   * Stream audio file
   */
  async streamAudio(audioId: string): Promise<AudioBuffer> {
    if (this.loadedAudio.has(audioId)) {
      return this.loadedAudio.get(audioId)!;
    }

    const resource = await this.streamingManager.request({
      id: audioId,
      type: ResourceType.AUDIO,
      priority: StreamingPriority.NORMAL,
      lodLevel: LODLevel.LOD0,
      distanceFromCamera: 0,
    });

    const audio = await this.loadAudioData(resource);
    this.loadedAudio.set(audioId, audio);
    this.memoryUsage += this.calculateAudioSize(audio);

    return audio;
  }

  /**
   * Stream audio in realtime for music/ambience
   */
  streamAudioRealtime(audioId: string, onChunk: (chunk: Float32Array) => void): AudioStream {
    const stream = new AudioStream();

    // Stream in chunks
    this.streamChunks(audioId, 64 * 1024, (chunk) => {
      stream.pushChunk(chunk);
      onChunk(chunk);
    });

    this.streamingAudio.set(audioId, stream);
    return stream;
  }

  /**
   * Stream 3D audio with distance-based quality
   */
  async streamAudio3D(audioId: string, distance: number): Promise<AudioBuffer> {
    const quality = this.selectAudioQuality(distance);
    const qualityId = `${audioId}_${quality}`;

    if (this.loadedAudio.has(qualityId)) {
      return this.loadedAudio.get(qualityId)!;
    }

    const resource = await this.streamingManager.request({
      id: qualityId,
      type: ResourceType.AUDIO,
      priority: this.calculatePriority(distance),
      lodLevel: this.qualityToLOD(quality),
      distanceFromCamera: distance,
    });

    const audio = await this.loadAudioData(resource);
    this.loadedAudio.set(qualityId, audio);
    this.memoryUsage += this.calculateAudioSize(audio);

    return audio;
  }

  private async streamChunks(audioId: string, chunkSize: number, onChunk: (chunk: Float32Array) => void): Promise<void> {
    // Placeholder - would stream chunks from server
    const totalChunks = 10;
    for (let i = 0; i < totalChunks; i++) {
      const chunk = new Float32Array(chunkSize / 4);
      onChunk(chunk);
      await new Promise(resolve => setTimeout(resolve, 100));
    }
  }

  private selectAudioQuality(distance: number): 'high' | 'medium' | 'low' {
    if (distance < 10) return 'high';
    if (distance < 50) return 'medium';
    return 'low';
  }

  private qualityToLOD(quality: 'high' | 'medium' | 'low'): LODLevel {
    switch (quality) {
      case 'high': return LODLevel.LOD0;
      case 'medium': return LODLevel.LOD2;
      case 'low': return LODLevel.LOD4;
    }
  }

  private calculatePriority(distance: number): StreamingPriority {
    if (distance < 10) return StreamingPriority.HIGH;
    if (distance < 30) return StreamingPriority.NORMAL;
    return StreamingPriority.LOW;
  }

  private calculateAudioSize(audio: AudioBuffer): number {
    return audio.samples.byteLength;
  }

  private async loadAudioData(resource: StreamedResource): Promise<AudioBuffer> {
    // Placeholder
    return {
      samples: new Float32Array(44100 * 2),
      sampleRate: 44100,
      channels: 2,
      duration: 1.0,
    };
  }

  getStats(): { loaded: number; streaming: number; memoryUsage: number; memoryBudget: number } {
    return {
      loaded: this.loadedAudio.size,
      streaming: this.streamingAudio.size,
      memoryUsage: this.memoryUsage,
      memoryBudget: this.maxMemoryBudget,
    };
  }
}

/**
 * Audio stream for realtime playback
 */
export class AudioStream {
  private chunks: Float32Array[] = [];
  private currentChunk: number = 0;

  pushChunk(chunk: Float32Array): void {
    this.chunks.push(chunk);
  }

  getNextChunk(): Float32Array | null {
    if (this.currentChunk >= this.chunks.length) {
      return null;
    }
    return this.chunks[this.currentChunk++];
  }

  reset(): void {
    this.currentChunk = 0;
  }
}

/**
 * Scene streaming system - handles scene region loading
 */
export class SceneStreamingSystem {
  private loadedRegions: Map<string, SceneData> = new Map();
  private activeRegions: Set<string> = new Set();

  constructor(
    private streamingManager: IStreamingManager,
    private meshStreaming: MeshStreamingSystem,
    private textureStreaming: TextureStreamingSystem,
    private allocator: IAllocator,
    private jobScheduler: JobScheduler
  ) {}

  /**
   * Stream scene region based on bounding box
   */
  async streamSceneRegion(region: BoundingBox): Promise<SceneData> {
    const regionId = this.getRegionId(region);

    if (this.loadedRegions.has(regionId)) {
      return this.loadedRegions.get(regionId)!;
    }

    const resource = await this.streamingManager.request({
      id: regionId,
      type: ResourceType.SCENE,
      priority: StreamingPriority.HIGH,
      lodLevel: LODLevel.LOD0,
      distanceFromCamera: 0,
    });

    const sceneData = await this.loadSceneData(resource);

    // Stream associated assets
    await this.streamSceneAssets(sceneData);

    this.loadedRegions.set(regionId, sceneData);
    this.activeRegions.add(regionId);

    return sceneData;
  }

  /**
   * Unload distant scene regions
   */
  unloadSceneRegion(region: BoundingBox): void {
    const regionId = this.getRegionId(region);
    this.loadedRegions.delete(regionId);
    this.activeRegions.delete(regionId);
    this.streamingManager.unload(regionId);
  }

  /**
   * Preload scene regions in background
   */
  async preloadSceneRegions(regions: BoundingBox[]): Promise<void> {
    const jobs = regions.map(region => {
      return new AsyncLoadJob({
        name: `Preload Scene ${this.getRegionId(region)}`,
        url: `/assets/scenes/${this.getRegionId(region)}.scene`,
        loader: async () => this.streamSceneRegion(region),
        priority: StreamingPriority.BACKGROUND,
      });
    });

    await Promise.all(jobs.map(job => this.jobScheduler.submit(job)));
  }

  /**
   * Smooth scene transition with preloading
   */
  async transitionToScene(sceneId: string, fadeTime: number): Promise<void> {
    // Start preloading new scene
    const preloadPromise = this.preloadScene(sceneId);

    // Fade out current scene
    await this.fadeOut(fadeTime / 2);

    // Wait for preload to complete
    await preloadPromise;

    // Unload old scene
    this.unloadCurrentScene();

    // Activate new scene
    await this.activateScene(sceneId);

    // Fade in new scene
    await this.fadeIn(fadeTime / 2);
  }

  private async streamSceneAssets(sceneData: SceneData): Promise<void> {
    // Extract unique mesh IDs
    const meshIds = [...new Set(sceneData.entities.filter(e => e.meshId).map(e => e.meshId!))];
    
    // Stream meshes in parallel
    const meshDistances = meshIds.map(() => 0); // Placeholder distances
    await this.meshStreaming.streamMeshBatch(meshIds, meshDistances);
  }

  private getRegionId(region: BoundingBox): string {
    return `region_${region.min.x}_${region.min.y}_${region.min.z}`;
  }

  private async loadSceneData(resource: StreamedResource): Promise<SceneData> {
    // Placeholder
    return {
      entities: [],
      lights: [],
      cameras: [],
      metadata: {},
    };
  }

  private async preloadScene(sceneId: string): Promise<void> {
    // Placeholder
  }

  private async fadeOut(time: number): Promise<void> {
    // Placeholder
  }

  private async fadeIn(time: number): Promise<void> {
    // Placeholder
  }

  private unloadCurrentScene(): void {
    // Placeholder
  }

  private async activateScene(sceneId: string): Promise<void> {
    // Placeholder
  }

  getStats(): { loadedRegions: number; activeRegions: number } {
    return {
      loadedRegions: this.loadedRegions.size,
      activeRegions: this.activeRegions.size,
    };
  }
}

/**
 * Integrated asset streaming system
 */
export class IntegratedStreamingSystem {
  public readonly mesh: MeshStreamingSystem;
  public readonly texture: TextureStreamingSystem;
  public readonly audio: AudioStreamingSystem;
  public readonly scene: SceneStreamingSystem;

  constructor(
    streamingManager: IStreamingManager,
    allocator: IAllocator,
    jobScheduler: JobScheduler
  ) {
    this.mesh = new MeshStreamingSystem(streamingManager, allocator, jobScheduler);
    this.texture = new TextureStreamingSystem(streamingManager, allocator, jobScheduler);
    this.audio = new AudioStreamingSystem(streamingManager, allocator, jobScheduler);
    this.scene = new SceneStreamingSystem(
      streamingManager,
      this.mesh,
      this.texture,
      allocator,
      jobScheduler
    );
  }

  getAllStats() {
    return {
      mesh: this.mesh.getStats(),
      texture: this.texture.getStats(),
      audio: this.audio.getStats(),
      scene: this.scene.getStats(),
    };
  }
}
