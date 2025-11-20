# Materials and Shaders

Learn how to create stunning visuals with Nova Engine's material system and custom shaders.

## Material Basics

Materials define how objects appear when rendered. They combine shaders, textures, and properties.

### Creating a Basic Material

```javascript
import { Material } from '@nova-engine/engine';

// Get shader from library
const shader = engine.shaderLibrary.get('standard');

// Create material
const material = new Material(shader);

// Set properties
material.setProperty('baseColor', { r: 1, g: 0, b: 0, a: 1 });
material.setProperty('metallic', 0.5);
material.setProperty('roughness', 0.3);

// Apply to mesh
meshRenderer.material = material;
```

## Standard Material (PBR)

Nova Engine uses Physically Based Rendering (PBR) for realistic materials.

### PBR Properties

```javascript
import { PBRMaterial } from '@nova-engine/engine';

const material = new PBRMaterial();

// Base color (albedo)
material.baseColor = { r: 0.8, g: 0.2, b: 0.2, a: 1.0 };

// Metallic (0 = dielectric, 1 = metal)
material.metallic = 0.0;

// Roughness (0 = smooth, 1 = rough)
material.roughness = 0.5;

// Emissive (glowing)
material.emissive = { r: 0, g: 0, b: 0 };
material.emissiveIntensity = 1.0;

// Ambient occlusion
material.aoStrength = 1.0;

// Normal mapping strength
material.normalScale = 1.0;

// Alpha mode
material.alphaMode = 'Opaque'; // 'Opaque', 'Mask', 'Blend'
material.alphaCutoff = 0.5; // For 'Mask' mode
```

### Texture Mapping

```javascript
// Load textures
const albedoTex = await engine.assetManager.load(
  'textures/brick_albedo.png',
  'texture'
);
const normalTex = await engine.assetManager.load(
  'textures/brick_normal.png',
  'texture'
);
const roughTex = await engine.assetManager.load(
  'textures/brick_rough.png',
  'texture'
);
const aoTex = await engine.assetManager.load(
  'textures/brick_ao.png',
  'texture'
);

// Apply textures
material.setTexture('baseColorMap', albedoTex);
material.setTexture('normalMap', normalTex);
material.setTexture('roughnessMap', roughTex);
material.setTexture('aoMap', aoTex);

// Metallic-roughness combined texture
const metalRoughTex = await engine.assetManager.load(
  'textures/metalRough.png',
  'texture'
);
material.setTexture('metallicRoughnessMap', metalRoughTex);
```

## Material Presets

### Glass Material

```javascript
const glass = new PBRMaterial();
glass.baseColor = { r: 0.9, g: 0.95, b: 1.0, a: 0.1 };
glass.metallic = 0.0;
glass.roughness = 0.0;
glass.alphaMode = 'Blend';
```

### Metal Material

```javascript
const metal = new PBRMaterial();
metal.baseColor = { r: 0.8, g: 0.8, b: 0.8, a: 1.0 };
metal.metallic = 1.0;
metal.roughness = 0.2;
```

### Plastic Material

```javascript
const plastic = new PBRMaterial();
plastic.baseColor = { r: 1.0, g: 0.0, b: 0.0, a: 1.0 };
plastic.metallic = 0.0;
plastic.roughness = 0.6;
```

### Emissive Material (Glow)

```javascript
const glow = new PBRMaterial();
glow.baseColor = { r: 0.1, g: 0.1, b: 0.1, a: 1.0 };
glow.emissive = { r: 0.0, g: 1.0, b: 0.5 };
glow.emissiveIntensity = 2.0;
```

## Custom Shaders

### Vertex and Fragment Shaders

```javascript
// Vertex shader
const vertexShader = `#version 300 es
precision highp float;

in vec3 aPosition;
in vec3 aNormal;
in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vNormal;
out vec2 vUV;

void main() {
  vNormal = mat3(uModel) * aNormal;
  vUV = aUV;
  gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
}
`;

// Fragment shader
const fragmentShader = `#version 300 es
precision highp float;

in vec3 vNormal;
in vec2 vUV;

uniform vec4 uColor;
uniform sampler2D uTexture;

out vec4 fragColor;

void main() {
  vec3 normal = normalize(vNormal);
  vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
  float diffuse = max(dot(normal, lightDir), 0.0);
  
  vec4 texColor = texture(uTexture, vUV);
  vec3 color = texColor.rgb * uColor.rgb * diffuse;
  
  fragColor = vec4(color, texColor.a * uColor.a);
}
`;

// Create shader
const shader = engine.shaderLibrary.create(
  'myShader',
  vertexShader,
  fragmentShader
);

// Create material with custom shader
const material = new Material(shader);
```

### Animated Shader Example

```javascript
const animatedShader = `#version 300 es
precision highp float;

in vec2 vUV;
uniform float uTime;
uniform vec4 uColor;

out vec4 fragColor;

void main() {
  // Animated wave pattern
  float wave = sin(vUV.x * 10.0 + uTime) * 0.5 + 0.5;
  vec3 color = uColor.rgb * wave;
  fragColor = vec4(color, uColor.a);
}
`;

// Update time uniform in game loop
engine.onUpdate((deltaTime) => {
  material.setProperty('uTime', engine.time.total);
});
```

### Toon Shader (Cel Shading)

```javascript
const toonFragment = `#version 300 es
precision highp float;

in vec3 vNormal;
uniform vec3 uLightDir;
uniform vec4 uColor;

out vec4 fragColor;

void main() {
  vec3 normal = normalize(vNormal);
  float NdotL = dot(normal, uLightDir);
  
  // Quantize lighting into bands
  float intensity;
  if (NdotL > 0.95) intensity = 1.0;
  else if (NdotL > 0.5) intensity = 0.7;
  else if (NdotL > 0.05) intensity = 0.4;
  else intensity = 0.2;
  
  vec3 color = uColor.rgb * intensity;
  fragColor = vec4(color, uColor.a);
}
`;
```

## Material Library

Create reusable materials:

```javascript
class MaterialLibrary {
  constructor() {
    this.materials = new Map();
  }

  add(name, material) {
    this.materials.set(name, material);
    return material;
  }

  get(name) {
    return this.materials.get(name);
  }

  createWood() {
    const material = new PBRMaterial();
    material.baseColor = { r: 0.6, g: 0.3, b: 0.1, a: 1.0 };
    material.metallic = 0.0;
    material.roughness = 0.8;
    return this.add('wood', material);
  }

  createStone() {
    const material = new PBRMaterial();
    material.baseColor = { r: 0.5, g: 0.5, b: 0.5, a: 1.0 };
    material.metallic = 0.0;
    material.roughness = 0.9;
    return this.add('stone', material);
  }

  createGold() {
    const material = new PBRMaterial();
    material.baseColor = { r: 1.0, g: 0.8, b: 0.2, a: 1.0 };
    material.metallic = 1.0;
    material.roughness = 0.2;
    return this.add('gold', material);
  }
}

// Usage
const matLib = new MaterialLibrary();
const woodMat = matLib.createWood();
meshRenderer.material = woodMat;
```

## Advanced Techniques

### Multi-Pass Rendering

```javascript
// First pass: Render to texture
const renderTarget = engine.createRenderTarget(1024, 1024);
engine.renderer.setRenderTarget(renderTarget);
engine.renderer.render(scene, camera);

// Second pass: Use rendered texture
const material = new Material(postProcessShader);
material.setTexture('uScreenTexture', renderTarget.texture);
```

### Material Instancing

```javascript
// Share material between objects for better performance
const sharedMaterial = new PBRMaterial();

for (let i = 0; i < 100; i++) {
  const entity = scene.createEntity(`Cube${i}`);
  const renderer = entity.addComponent(MeshRenderer);
  renderer.mesh = cubeMesh;
  renderer.material = sharedMaterial; // Same material instance
}
```

### Dynamic Material Properties

```javascript
class PulsatingMaterial extends Component {
  onAttach() {
    this._renderer = this.entity.getComponent(MeshRenderer);
    this._time = 0;
  }

  update(deltaTime) {
    this._time += deltaTime;
    const intensity = Math.sin(this._time * 2) * 0.5 + 0.5;

    this._renderer.material.setProperty('emissiveIntensity', intensity * 3);
  }
}
```

## Shader Debugging

### Visualizing Normals

```javascript
const normalDebugShader = `#version 300 es
precision highp float;

in vec3 vNormal;
out vec4 fragColor;

void main() {
  vec3 normal = normalize(vNormal);
  vec3 color = normal * 0.5 + 0.5; // Remap from [-1,1] to [0,1]
  fragColor = vec4(color, 1.0);
}
`;
```

### Visualizing UVs

```javascript
const uvDebugShader = `#version 300 es
precision highp float;

in vec2 vUV;
out vec4 fragColor;

void main() {
  fragColor = vec4(vUV.x, vUV.y, 0.0, 1.0);
}
`;
```

## Performance Tips

1. **Batch Similar Materials**: Objects with the same material render faster
2. **Use Texture Atlases**: Combine multiple textures into one
3. **Minimize Shader Complexity**: Simpler shaders = better performance
4. **Cache Uniform Locations**: Don't look them up every frame
5. **Use Mipmaps**: Essential for textured objects at varying distances

## Next Steps

- [Physics Integration](./04-physics.md) - Add realistic physics
- [Post-Processing Effects](./06-post-processing.md) - Screen effects
- [Lighting Techniques](./07-lighting.md) - Advanced lighting

---

**Create beautiful visuals with materials! 🎨✨**
