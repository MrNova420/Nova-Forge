# Nova Engine - Engine Architecture

This document explains the engine architecture and how different packages work together.

## 🎯 Engine Overview

Nova Engine consists of two main engine packages:

### 1. Core Engine (`@nova-engine/engine`)

**Location**: `packages/engine/`  
**Purpose**: Core game engine runtime with full features  
**Used By**: Unified Platform, Desktop Editor, Production Games

The core engine is the main, full-featured game engine that includes:

- Complete ECS (Entity Component System)
- Advanced rendering pipeline
- Physics simulation (Ammo.js)
- Audio system
- Asset management
- Input handling
- Scene management

**Key Features**:

- ✅ Full feature set
- ✅ Production-ready
- ✅ Desktop and web compatible
- ✅ Optimized performance
- ✅ Complete API

### 2. Web Engine (`@nova-engine/engine-web`)

**Location**: `packages/engine-web/`  
**Purpose**: Lightweight web-specific runtime  
**Used By**: Web launcher, embedded games, lightweight deployments

The web engine is a streamlined version optimized for web deployment:

- Focused on web platform
- Lighter bundle size
- Uses Three.js for rendering
- Uses Cannon.js for physics
- Minimal dependencies

**Key Features**:

- ✅ Lightweight (~smaller bundle)
- ✅ Web-optimized
- ✅ Quick load times
- ✅ Embedded game support

## 🏗️ How Unified Platform Uses Core Engine

The unified platform (`packages/unified-platform/`) uses the **core engine** (`@nova-engine/engine`):

```typescript
// packages/unified-platform/package.json
{
  "dependencies": {
    "@nova-engine/engine": "*",  // ← Core engine
    "@nova-engine/editor": "*",
    // ... other deps
  }
}
```

### Why Core Engine for Unified Platform?

1. **Full Feature Set**: Unified platform needs complete engine capabilities
2. **Editor Integration**: The editor requires full engine API access
3. **Development Tools**: Core engine includes debugging and profiling
4. **Consistency**: Same engine for development and production
5. **Performance**: Core engine is optimized for all scenarios

## 📦 Package Dependencies

```
Unified Platform
    ↓ uses
Core Engine (@nova-engine/engine)
    ↓ provides
Complete Game Runtime
```

```
Web Launcher
    ↓ uses
Web Engine (@nova-engine/engine-web)
    ↓ provides
Lightweight Runtime
```

## 🔄 Engine Features Comparison

| Feature             | Core Engine              | Web Engine           |
| ------------------- | ------------------------ | -------------------- |
| **ECS System**      | ✅ Full                  | ✅ Basic             |
| **Rendering**       | ✅ Custom + Three.js     | ✅ Three.js          |
| **Physics**         | ✅ Ammo.js (full)        | ✅ Cannon.js (basic) |
| **Audio**           | ✅ Advanced              | ✅ Basic             |
| **Asset Pipeline**  | ✅ Complete              | ✅ Simple            |
| **Debugging Tools** | ✅ Full suite            | ⚠️ Limited           |
| **Editor Support**  | ✅ Yes                   | ❌ No                |
| **Bundle Size**     | ~2MB                     | ~500KB               |
| **Load Time**       | Medium                   | Fast                 |
| **Use Case**        | Development & Production | Web deployment       |

## 🎮 Usage Examples

### Using Core Engine (Unified Platform)

```typescript
// In unified platform
import { Engine, Scene, Entity, Vector3 } from '@nova-engine/engine';

// Full engine with all features
const engine = new Engine({
  canvas: document.getElementById('canvas'),
  physics: true,
  audio: true,
  debug: true,
});

const scene = new Scene();
const entity = new Entity();
entity.addComponent('Transform', { position: new Vector3(0, 0, 0) });
scene.addEntity(entity);

engine.loadScene(scene);
engine.start();
```

### Using Web Engine (Web Launcher)

```typescript
// In web launcher
import { WebEngine } from '@nova-engine/engine-web';

// Lightweight engine for quick loading
const engine = new WebEngine({
  canvas: document.getElementById('canvas'),
});

engine.loadGame('game-url');
engine.start();
```

## 🔧 Configuration

### Core Engine Configuration

```typescript
// packages/engine/tsconfig.json
{
  "compilerOptions": {
    "target": "ES2020",
    "module": "ESNext",
    // Full TypeScript features
  }
}
```

### Web Engine Configuration

```typescript
// packages/engine-web/tsconfig.json
{
  "compilerOptions": {
    "target": "ES2020",
    "module": "ESNext",
    // Web-optimized settings
  }
}
```

## 🚀 Build Process

### Core Engine Build

```bash
npm run build:engine
# Outputs:
# - dist/index.js (CommonJS)
# - dist/index.mjs (ES Module)
# - dist/index.d.ts (TypeScript definitions)
```

### Web Engine Build

```bash
cd packages/engine-web && npm run build
# Outputs:
# - dist/index.js (Optimized web bundle)
```

## 📊 Performance Characteristics

### Core Engine

- **Startup Time**: ~500ms
- **Memory Usage**: ~100-200MB
- **Bundle Size**: ~2MB (minified)
- **Target**: Desktop, Full Web Apps

### Web Engine

- **Startup Time**: ~200ms
- **Memory Usage**: ~50-100MB
- **Bundle Size**: ~500KB (minified)
- **Target**: Embedded Games, Mobile Web

## 🔐 Security Considerations

### Core Engine

- Full file system access (desktop)
- Network access for multiplayer
- Complete API access

### Web Engine

- Sandboxed browser environment
- Limited API surface
- Secure by default

## 🛠️ Development Workflow

### Working with Core Engine

```bash
# Make changes to core engine
cd packages/engine/src

# Build engine
npm run build -w @nova-engine/engine

# Test in unified platform
npm run dev -w @nova-engine/unified-platform
```

### Engine Hot Reload

The development environment supports hot module replacement:

```bash
# Start with hot reload
npm start
# Changes to engine automatically rebuild and reload
```

## 📝 Best Practices

### When to Use Core Engine

✅ Unified platform development  
✅ Desktop applications  
✅ Full-featured games  
✅ Editor development  
✅ Advanced features needed

### When to Use Web Engine

✅ Lightweight web games  
✅ Mobile web deployment  
✅ Embedded game widgets  
✅ Quick prototypes  
✅ Bundle size critical

## 🔗 Related Documentation

- [Project Organization](PROJECT_ORGANIZATION.md)
- [Architecture Overview](ARCHITECTURE.md)
- [Development Guide](DEVELOPMENT_GUIDE.md)
- [Component Breakdown](COMPONENT_BREAKDOWN.md)

## 🎯 Future Improvements

### Planned Core Engine Features

- Advanced lighting system
- Real-time GI
- Advanced physics materials
- Networking layer

### Planned Web Engine Features

- WebGPU support
- Better mobile optimization
- Progressive web app features
- Service worker integration

## 📞 Support

For questions about engine architecture:

- Review this document
- Check [ARCHITECTURE.md](ARCHITECTURE.md)
- See example usage in `packages/unified-platform/`
