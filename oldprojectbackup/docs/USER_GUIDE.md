# Nova Engine - User Guide

> **Complete guide to building games with Nova Engine**

---

## Table of Contents

1. [Introduction](#introduction)
2. [Getting Started](#getting-started)
3. [Core Concepts](#core-concepts)
4. [Creating Your First Game](#creating-your-first-game)
5. [Engine Systems](#engine-systems)
6. [Editor Usage](#editor-usage)
7. [Building and Deploying](#building-and-deploying)
8. [Advanced Topics](#advanced-topics)
9. [Troubleshooting](#troubleshooting)
10. [API Reference](#api-reference)

---

## Introduction

Nova Engine is a complete web-based game development platform that enables you to create 2D and 3D games that run in any modern browser. Built with TypeScript and WebGL, it provides professional-grade tools while remaining accessible to developers of all skill levels.

### Key Features

- **Modern Architecture**: Entity-Component-System (ECS) for flexible game logic
- **Advanced Graphics**: WebGL 2.0 rendering with PBR materials and post-processing
- **Physics**: Integrated Ammo.js (Bullet Physics) for realistic simulations
- **Audio**: 3D positional audio with effects
- **Animation**: Keyframe animation with state machines and blending
- **Input**: Unified input system supporting keyboard, mouse, touch, and gamepad
- **Editor**: Professional visual editor with real-time preview
- **Web-First**: Optimized for web deployment with offline support

---

## Getting Started

### Prerequisites

- **Node.js** 18.0.0 or higher
- **npm** 9.0.0 or higher
- Modern web browser (Chrome, Firefox, Safari, Edge)

### Installation

```bash
# Clone the repository
git clone https://github.com/MrNova420/Nova-Engine-editor.git
cd Nova-Engine-editor

# Install dependencies
npm install

# Build all packages
npm run build

# Run tests
npm test
```

### Quick Start

```typescript
import { Engine, Scene, Camera, Mesh, Primitives } from '@nova-engine/engine';

// Initialize engine
const engine = new Engine();
await engine.initialize(document.getElementById('game-canvas'));

// Create scene
const scene = new Scene('main');

// Add camera
const camera = new Camera();
camera.setPerspective(75, 16 / 9, 0.1, 1000);
camera.transform.position.z = 5;

// Add a cube
const cubeMesh = Primitives.createCube();
const cube = scene.createEntity('cube');
scene.addComponent(cube, new MeshRenderer(cubeMesh));

// Start engine
engine.start();
```

---

## Core Concepts

### Entity-Component-System (ECS)

Nova Engine uses an ECS architecture for game logic:

- **Entities**: Game objects (players, enemies, items)
- **Components**: Data containers (position, health, sprite)
- **Systems**: Logic that operates on components (movement, rendering, AI)

#### Example

```typescript
// Create entity
const player = world.createEntity('player');

// Add components
world.addComponent(player, new TransformComponent());
world.addComponent(player, new MeshRenderer(playerMesh));
world.addComponent(player, new RigidBody(BodyType.Dynamic));

// Systems automatically process entities with matching components
```

### Scene Graph

Entities can have parent-child relationships:

```typescript
const parent = scene.createEntity('parent');
const child = scene.createEntity('child');

// Create hierarchy
scene.setParent(child, parent);

// Child inherits parent's transform
```

### Asset Management

Load and manage game assets efficiently:

```typescript
const assetManager = new AssetManager();

// Load assets
const texture = await assetManager.load<Texture>(
  'texture.png',
  AssetType.Texture
);
const model = await assetManager.load<GLTFData>('model.gltf', AssetType.Model);

// Assets are cached automatically
```

---

## Creating Your First Game

### Step 1: Set Up the Project

Create a new HTML file:

```html
<!DOCTYPE html>
<html>
  <head>
    <title>My Nova Game</title>
    <style>
      body {
        margin: 0;
        overflow: hidden;
      }
      canvas {
        display: block;
      }
    </style>
  </head>
  <body>
    <canvas id="game-canvas"></canvas>
    <script type="module" src="game.js"></script>
  </body>
</html>
```

### Step 2: Initialize the Engine

```typescript
import { Engine, Scene } from '@nova-engine/engine';

const engine = new Engine();
const canvas = document.getElementById('game-canvas');

await engine.initialize(canvas);

const scene = new Scene('game');
engine.setActiveScene(scene);
```

### Step 3: Add Game Objects

```typescript
// Create player
const player = scene.createEntity('player');
const playerMesh = Primitives.createCube();
scene.addComponent(player, new MeshRenderer(playerMesh));
scene.addComponent(player, new RigidBody(BodyType.Dynamic));

// Create ground
const ground = scene.createEntity('ground');
const groundMesh = Primitives.createPlane(10, 10);
scene.addComponent(ground, new MeshRenderer(groundMesh));
scene.addComponent(ground, new RigidBody(BodyType.Static));
```

### Step 4: Add Game Logic

```typescript
// Create custom system
class PlayerControlSystem extends System {
  update(world: World, deltaTime: number): void {
    const entities = world.query({ all: [TransformComponent, RigidBody] });

    for (const entity of entities) {
      const rb = world.getComponent(entity, RigidBody);

      if (input.isKeyDown('w')) {
        rb.applyForce(new Vector3(0, 0, -10));
      }
      if (input.isKeyDown('s')) {
        rb.applyForce(new Vector3(0, 0, 10));
      }
    }
  }
}

// Add system to world
world.addSystem(new PlayerControlSystem());
```

### Step 5: Start the Game

```typescript
engine.start();
```

---

## Engine Systems

### Graphics System

#### Rendering

```typescript
const renderer = new WebGLRenderer();
await renderer.initialize(canvas);

// Set up camera
const camera = new Camera();
camera.setPerspective(75, canvas.width / canvas.height, 0.1, 1000);

// Render scene
renderer.render(scene, camera);
```

#### Materials

```typescript
// PBR Material
const material = new PBRMaterial();
material.albedo = new Vector3(1, 0, 0); // Red
material.metallic = 0.5;
material.roughness = 0.3;

// Apply texture
const texture = await assetManager.load('albedo.png', AssetType.Texture);
material.albedoMap = texture;
```

#### Lighting

```typescript
// Directional light (sun)
const sun = scene.createEntity('sun');
const dirLight = new DirectionalLight();
dirLight.color = new Vector3(1, 1, 0.9);
dirLight.intensity = 1.0;
scene.addComponent(sun, dirLight);

// Point light
const lamp = scene.createEntity('lamp');
const pointLight = new PointLight();
pointLight.color = new Vector3(1, 0.8, 0.6);
pointLight.range = 10;
scene.addComponent(lamp, pointLight);
```

### Physics System

```typescript
// Create physics world
const physicsWorld = new PhysicsWorld();
physicsWorld.setGravity(new Vector3(0, -9.81, 0));

// Add rigid body
const rb = new RigidBody(BodyType.Dynamic);
rb.mass = 1.0;
rb.friction = 0.5;
rb.restitution = 0.3;

// Add collider
const collider = new BoxCollider(1, 1, 1);
entity.addComponent(collider);

// Apply forces
rb.applyForce(new Vector3(0, 10, 0));
rb.applyImpulse(new Vector3(5, 0, 0));
```

### Audio System

```typescript
// Initialize audio
const audioSystem = new AudioSystem();
await audioSystem.initialize();

// Load sound
const sound = await assetManager.load('sound.mp3', AssetType.Audio);

// Create audio source
const source = new AudioSource(sound);
source.volume = 0.8;
source.loop = true;
source.play();

// 3D audio
source.spatial = true;
source.position = new Vector3(5, 0, 0);
```

### Animation System

```typescript
// Create animation clip
const walkClip = new AnimationClip('walk', 1.0);
const track = new AnimationTrack('position.y', AnimationPropertyType.Number);
track.addKeyframe(0, 0);
track.addKeyframe(0.5, 1);
track.addKeyframe(1.0, 0);
walkClip.addTrack(track);

// Create animator
const animator = new Animator();
animator.addClip('walk', walkClip);
animator.play('walk');

// Animation state machine
const stateMachine = new AnimationStateMachine();
stateMachine.addState('idle', idleClip);
stateMachine.addState('walk', walkClip);
stateMachine.addTransition('idle', 'walk', 0.2);
```

### Input System

```typescript
const input = new InputManager();

// Keyboard
if (input.isKeyDown('space')) {
  player.jump();
}

// Mouse
const mousePos = input.getMousePosition();
if (input.isMouseButtonPressed(MouseButton.Left)) {
  shoot(mousePos);
}

// Gamepad
if (input.isGamepadButtonDown(0, GamepadButton.A)) {
  player.jump();
}

// Action mapping
input.mapAction('jump', 'space');
input.mapAction('jump', GamepadButton.A);
if (input.isActionPressed('jump')) {
  player.jump();
}
```

---

## Editor Usage

### Opening the Editor

```bash
cd packages/editor
npm run dev
```

The editor will open at `http://localhost:3000`

### Editor Interface

- **Viewport**: 3D view of your scene
- **Hierarchy**: Tree view of all entities
- **Inspector**: Properties of selected entity
- **Asset Browser**: Project assets
- **Console**: Debug output

### Creating Entities

1. Right-click in Hierarchy
2. Select "Create Entity"
3. Choose type (Empty, Cube, Sphere, etc.)

### Adding Components

1. Select entity
2. Click "Add Component" in Inspector
3. Choose component type
4. Configure properties

### Transform Tools

- **Q**: Select tool
- **W**: Move tool
- **E**: Rotate tool
- **R**: Scale tool

### Play Mode

Click the Play button to test your game in the editor.

---

## Building and Deploying

### Web Build

```bash
# Build for web
npm run build

# Output in packages/editor/dist
```

### Deploying to Web

Upload the `dist` folder to any web server:

```bash
# Using a simple server
npx serve dist

# Or deploy to GitHub Pages, Netlify, Vercel, etc.
```

### Desktop Build

```bash
# Build for desktop (Electron/Tauri)
npm run build:desktop

# Platform-specific installers will be created
```

### Optimization

The build system automatically:

- Minifies code
- Tree-shakes unused code
- Code splits into chunks
- Compresses assets
- Generates service worker for offline support

---

## Advanced Topics

### Custom Components

```typescript
class HealthComponent extends Component {
  public currentHealth: number = 100;
  public maxHealth: number = 100;

  public takeDamage(amount: number): void {
    this.currentHealth = Math.max(0, this.currentHealth - amount);
  }

  public heal(amount: number): void {
    this.currentHealth = Math.min(this.maxHealth, this.currentHealth + amount);
  }
}

// Register component
ComponentRegistry.register(HealthComponent);
```

### Custom Systems

```typescript
class HealthSystem extends System {
  public priority: number = 100;

  update(world: World, deltaTime: number): void {
    const entities = world.query({ all: [HealthComponent] });

    for (const entity of entities) {
      const health = world.getComponent(entity, HealthComponent);

      if (health.currentHealth <= 0) {
        world.destroyEntity(entity);
      }
    }
  }
}
```

### Performance Profiling

```typescript
import { profiler } from '@nova-engine/engine';

// Enable profiler
profiler.enable();

// Profile code
profiler.begin('updatePhysics');
physicsWorld.step(deltaTime);
profiler.end('updatePhysics');

// Get report
console.log(profiler.getReport());
```

### Memory Management

```typescript
// Unload assets
assetManager.unload('texture.png');

// Clear unused assets
assetManager.clearUnused();

// Monitor memory
profiler.takeMemorySnapshot();
const memory = profiler.getCurrentMemoryUsage();
console.log(`Memory: ${memory.usedHeapSize / 1024 / 1024} MB`);
```

---

## Troubleshooting

### Common Issues

#### Black Screen

- Check if WebGL is enabled in your browser
- Verify canvas element exists
- Check console for errors

#### Assets Not Loading

- Check asset paths are correct
- Verify files exist in the project
- Check network tab in browser DevTools

#### Low FPS

- Enable profiler to identify bottlenecks
- Reduce polygon count
- Use LOD (Level of Detail)
- Optimize shaders

#### Physics Not Working

- Verify Ammo.js is loaded
- Check collision shapes are added
- Ensure rigid body is dynamic (not static)

---

## API Reference

### Core Classes

- `Engine`: Main engine class
- `Scene`: Scene container
- `World`: ECS world
- `Entity`: Game object
- `Component`: Data container
- `System`: Logic processor

### Graphics

- `WebGLRenderer`: WebGL renderer
- `Camera`: Camera component
- `Material`: Material base class
- `PBRMaterial`: PBR material
- `Mesh`: Mesh data
- `Light`: Light base class

### Physics

- `PhysicsWorld`: Physics simulation
- `RigidBody`: Physics body
- `Collider`: Collision shape
- `Constraint`: Physics joint

### Audio

- `AudioSystem`: Audio manager
- `AudioSource`: Audio playback
- `AudioListener`: Audio receiver

### Animation

- `AnimationClip`: Animation data
- `Animator`: Animation player
- `AnimationStateMachine`: State machine

### Input

- `InputManager`: Input handler
- `Keyboard`: Keyboard input
- `Mouse`: Mouse input
- `Gamepad`: Gamepad input

For complete API documentation, see the [TypeDoc generated docs](./docs/api/index.html).

---

## Legal & Licensing

Before distributing games created with Nova Engine, please review the legal documentation:

- **[LEGAL.md](../LEGAL.md)** - ⭐ **START HERE** - Complete legal documentation overview
- **[LICENSE](../LICENSE)** - Main proprietary license summary
- **[EULA](../legal/NovaEngine_EULA.pdf)** - End User License Agreement
- **[Developer License](../legal/NovaEngine_DeveloperLicense.pdf)** - Revenue sharing and commercial terms
- **[Runtime License](../legal/NovaEngine_RuntimeLicense.pdf)** - Game distribution terms
- **[Privacy Policy](../legal/NovaEngine_PrivacyPolicy.pdf)** - Data protection guidelines

**Key Points:**
- **Nova Engine is proprietary closed-source software** (not open source)
- **Free to use commercially until $250,000 USD/year revenue**
- **2% royalty on revenue above $250,000** (resets each year)
- **You own the games and content you create**
- WeNova Interactive owns Nova Engine
- Review ALL legal documents before publishing commercial games

See [LEGAL.md](../LEGAL.md) for a detailed explanation of all licenses and when they apply.

---

## Next Steps

1. Try the [sample projects](./examples/)
2. Read the [architecture documentation](./ARCHITECTURE.md)
3. Join the [community Discord](https://discord.gg/novaengine)
4. Contribute on [GitHub](https://github.com/MrNova420/Nova-Engine-editor)

---

**Happy game development with Nova Engine!** 🎮✨
