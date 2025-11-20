# Asset Loaders

This directory contains specialized asset loaders for different file formats. These loaders integrate with the AssetManager to provide a unified asset loading pipeline.

## Available Loaders

### TextureAssetLoader

Loads image files and creates Texture objects for the graphics system.

**Supported Formats:**

- PNG (.png)
- JPEG (.jpg, .jpeg)
- GIF (.gif)
- WebP (.webp)
- BMP (.bmp)

**Usage:**

```typescript
import { TextureAssetLoader } from '@nova-engine/engine';

const loader = new TextureAssetLoader();
loader.setContext(gl); // Set WebGL context

const texture = await loader.load('path/to/texture.png');
```

### AudioLoader

Loads audio files and decodes them into AudioBuffer objects using the Web Audio API.

**Supported Formats:**

- MP3 (.mp3)
- WAV (.wav)
- OGG (.ogg)
- M4A (.m4a)
- AAC (.aac)
- FLAC (.flac)
- WebM (.webm)

**Usage:**

```typescript
import { AudioLoader } from '@nova-engine/engine';

const loader = new AudioLoader();
const audioData = await loader.load('path/to/sound.mp3');

// Access audio buffer
const buffer = audioData.buffer;
console.log(`Duration: ${audioData.duration}s`);
console.log(`Sample rate: ${audioData.sampleRate}Hz`);
```

**Lifecycle Methods:**

- `suspend()` - Suspend audio context to save resources
- `resume()` - Resume audio context
- `close()` - Close and cleanup audio context

### OBJLoader

Loads Wavefront OBJ 3D model files and creates Mesh objects.

**Supported Formats:**

- OBJ (.obj)

**Features:**

- Parses vertex positions, normals, and UVs
- Supports triangles and quads (automatically triangulated)
- Generates flat normals if not provided
- Vertex deduplication for efficiency

**Usage:**

```typescript
import { OBJLoader } from '@nova-engine/engine';

const loader = new OBJLoader();
loader.setContext(gl); // Set WebGL context

const objData = await loader.load('path/to/model.obj');

// Access meshes
for (const mesh of objData.meshes) {
  // Use mesh in scene
}

console.log(`Vertices: ${objData.vertexCount}`);
console.log(`Faces: ${objData.faceCount}`);
```

### GLTFLoader

Loads glTF 2.0 (GL Transmission Format) 3D model files.

**Supported Formats:**

- glTF JSON (.gltf) + external buffers
- glTF Binary (.glb)

**Features:**

- Parses meshes, nodes, and scenes
- Supports vertex positions, normals, and UVs
- Handles both JSON and binary formats
- Extracts scene hierarchy and transforms

**Usage:**

```typescript
import { GLTFLoader } from '@nova-engine/engine';

const loader = new GLTFLoader();
loader.setContext(gl); // Set WebGL context

const gltfData = await loader.load('path/to/model.gltf');

// Access meshes
for (const mesh of gltfData.meshes) {
  // Use mesh in scene
}

// Access scene hierarchy
for (const scene of gltfData.scenes) {
  console.log(`Scene: ${scene.name}`);
  for (const nodeIndex of scene.nodes) {
    const node = gltfData.nodes[nodeIndex];
    console.log(`Node: ${node.name}`);
  }
}
```

## Integration with AssetManager

All loaders implement the `AssetLoader<T>` interface and can be registered with the AssetManager:

```typescript
import {
  AssetManager,
  AssetType,
  TextureAssetLoader,
  AudioLoader,
  OBJLoader,
  GLTFLoader,
} from '@nova-engine/engine';

const assetManager = new AssetManager();

// Register loaders
const textureLoader = new TextureAssetLoader();
textureLoader.setContext(gl);
assetManager.registerLoader(AssetType.Texture, textureLoader);

const audioLoader = new AudioLoader();
assetManager.registerLoader(AssetType.Audio, audioLoader);

const objLoader = new OBJLoader();
objLoader.setContext(gl);
assetManager.registerLoader(AssetType.Model, objLoader);

const gltfLoader = new GLTFLoader();
gltfLoader.setContext(gl);
assetManager.registerLoader(AssetType.Model, gltfLoader);

// Load assets
const texture = await assetManager.load('texture.png', AssetType.Texture);
const audio = await assetManager.load('sound.mp3', AssetType.Audio);
const model = await assetManager.load('model.gltf', AssetType.Model);
```

## Creating Custom Loaders

To create a custom loader, implement the `AssetLoader<T>` interface:

```typescript
import { AssetLoader } from '@nova-engine/engine';

export class CustomLoader implements AssetLoader<MyAssetType> {
  async load(path: string): Promise<MyAssetType> {
    // Fetch and parse the asset
    const response = await fetch(path);
    const data = await response.text();

    // Process data and return asset
    return this.parseData(data);
  }

  supports(path: string): boolean {
    return path.endsWith('.custom');
  }

  private parseData(data: string): MyAssetType {
    // Custom parsing logic
  }
}
```

## Error Handling

All loaders throw descriptive errors when loading fails:

```typescript
try {
  const model = await loader.load('model.gltf');
} catch (error) {
  console.error('Failed to load model:', error);
}
```

Common error scenarios:

- Network errors (file not found, network issues)
- Parse errors (invalid file format)
- WebGL context not set (for graphics loaders)
- Unsupported file format

## Performance Considerations

1. **Caching**: Use AssetManager's built-in caching to avoid reloading the same assets
2. **Preloading**: Use `preload()` methods to load assets before they're needed
3. **WebGL Context**: Set the WebGL context once at initialization, not per load
4. **Audio Context**: Reuse the same AudioContext across multiple audio loads
5. **Large Files**: Consider loading large models in chunks or using compressed formats

## Future Enhancements

Planned features for future releases:

- FBX loader
- Draco mesh compression support
- KTX2/Basis texture support
- Animation and skeletal data loading
- Material and PBR texture loading
- Progressive loading for large assets
- Worker-based loading for better performance
