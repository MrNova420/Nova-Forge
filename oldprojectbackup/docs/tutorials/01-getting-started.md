# Getting Started with Nova Engine

Welcome to Nova Engine! This tutorial will guide you through creating your first game engine project.

## Prerequisites

- Modern web browser with WebGL 2 support
- Basic knowledge of JavaScript/TypeScript
- Text editor or IDE (VS Code recommended)
- Node.js 18+ and npm 9+

## Installation

### Option 1: Using npm (Recommended)

```bash
# Install Nova Engine
npm install @nova-engine/engine @nova-engine/editor

# Or clone the repository
git clone https://github.com/MrNova420/Nova-Engine-editor.git
cd Nova-Engine-editor
npm install
```

### Option 2: CDN (Quick Start)

```html
<script type="module">
  import * as Nova from 'https://cdn.jsdelivr.net/npm/@nova-engine/engine';
  // Your code here
</script>
```

## Your First Project

### 1. Create Project Structure

```bash
mkdir my-nova-game
cd my-nova-game
npm init -y
npm install @nova-engine/engine
```

### 2. Create HTML File

Create `index.html`:

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
        width: 100vw;
        height: 100vh;
        display: block;
      }
    </style>
  </head>
  <body>
    <canvas id="gameCanvas"></canvas>
    <script type="module" src="game.js"></script>
  </body>
</html>
```

### 3. Create Game Script

Create `game.js`:

```javascript
import {
  Engine,
  Scene,
  Entity,
  Camera,
  MeshRenderer,
  Material,
} from '@nova-engine/engine';

// Initialize engine
const canvas = document.getElementById('gameCanvas');
const engine = new Engine(canvas);

await engine.initialize();

// Create scene
const scene = new Scene('MainScene');
engine.setActiveScene(scene);

// Create camera
const cameraEntity = scene.createEntity('Camera');
const camera = cameraEntity.addComponent(Camera);
camera.setPerspective(Math.PI / 4, canvas.width / canvas.height, 0.1, 1000);
cameraEntity.transform.position.set(0, 2, 5);
cameraEntity.transform.lookAt({ x: 0, y: 0, z: 0 });

// Create cube
const cubeEntity = scene.createEntity('Cube');
const meshRenderer = cubeEntity.addComponent(MeshRenderer);
meshRenderer.mesh = engine.assetManager.createPrimitive('cube');
meshRenderer.material = new Material(engine.shaderLibrary.get('standard'));
meshRenderer.material.setProperty('baseColor', {
  r: 0.3,
  g: 0.6,
  b: 1.0,
  a: 1.0,
});

// Start engine
engine.start();

// Update loop
engine.onUpdate((deltaTime) => {
  // Rotate cube
  cubeEntity.transform.rotation.y += deltaTime;
});

console.log('🎮 Game started!');
```

### 4. Run Your Game

```bash
# Install a local server
npm install -g http-server

# Run server
http-server

# Open browser to http://localhost:8080
```

## Understanding the Code

### Engine Initialization

```javascript
const engine = new Engine(canvas);
await engine.initialize();
```

The Engine class is the core of Nova Engine. It manages:

- Rendering pipeline
- Scene management
- Asset loading
- Input handling
- Game loop

### Scene Creation

```javascript
const scene = new Scene('MainScene');
engine.setActiveScene(scene);
```

Scenes contain all game objects (entities). You can have multiple scenes and switch between them.

### Entity-Component System

```javascript
const entity = scene.createEntity('MyEntity');
const component = entity.addComponent(ComponentType);
```

Nova Engine uses ECS (Entity-Component-System) architecture:

- **Entity**: Container for components
- **Component**: Data and behavior
- **System**: Processes components

### Transform Hierarchy

```javascript
entity.transform.position.set(0, 2, 5);
entity.transform.rotation.y = Math.PI / 4;
entity.transform.scale.set(2, 2, 2);
```

Every entity has a Transform component for position, rotation, and scale.

## Next Steps

Now that you have a basic project running, try these tutorials:

1. **[Working with Components](./02-components.md)** - Learn about built-in components
2. **[Materials and Shaders](./03-materials.md)** - Customize object appearance
3. **[Physics Integration](./04-physics.md)** - Add realistic physics
4. **[Input Handling](./05-input.md)** - Respond to user input
5. **[Asset Management](./06-assets.md)** - Load models and textures

## Common Issues

### WebGL 2 Not Supported

If you see this error, your browser doesn't support WebGL 2:

```javascript
if (!canvas.getContext('webgl2')) {
  alert('WebGL 2 is not supported. Please update your browser.');
}
```

### Module Import Errors

Make sure you're using a local server. Opening HTML files directly (`file://`) won't work with ES modules.

### Performance Issues

For better performance:

- Enable production build (`npm run build`)
- Use object pooling for frequently created/destroyed objects
- Batch similar render calls
- Use LOD (Level of Detail) for complex models

## Resources

- [API Documentation](../api/index.html)
- [Examples](../../examples/)
- [GitHub Repository](https://github.com/MrNova420/Nova-Engine-editor)
- [Community Discord](#) (coming soon)

## What's Next?

Explore the [Component System Tutorial](./02-components.md) to learn about all available components and how to create custom ones!

---

**Happy Game Development! 🎮✨**
