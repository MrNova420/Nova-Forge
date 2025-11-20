# Nova Engine - Platform Abstraction Layer

**Phase 1.1 of Autonomous Development Guide**

## Overview

The Platform Abstraction Layer provides a unified interface for Nova Engine to run across multiple platforms with a single codebase. This abstraction enables seamless cross-platform development for web browsers, native desktop applications, and mobile devices.

## Architecture

### Core Interface: `IPlatform`

The `IPlatform` interface defines the contract that all platform implementations must fulfill. It includes:

- **Lifecycle Management**: `initialize()`, `shutdown()`
- **Platform Information**: `getName()`, `getInfo()`, `getCapabilities()`
- **Window Management**: `createWindow()`, `getWindows()`, `getMainWindow()`
- **File System**: `getFileSystem()` with full CRUD operations
- **Threading**: `createWorker()`, `getHardwareConcurrency()`
- **Timing**: `now()`, `sleep()`
- **Input**: `getInputDevices()`
- **Networking**: `fetch()`, `createWebSocket()`
- **Clipboard**: `readClipboard()`, `writeClipboard()`
- **Dialogs**: `alert()`, `confirm()`, `prompt()`
- **Events**: `addEventListener()`, `removeEventListener()`

## Platform Implementations

### WebPlatform

**Target**: Modern web browsers  
**Lines of Code**: 643

**Features**:
- Canvas-based window management
- IndexedDB for persistent file storage
- Web Workers for multi-threading
- WebGL/WebGPU capability detection
- Full Web API integration (Fetch, WebSocket, Clipboard API)
- Touch and gamepad input support

**Use Cases**:
- Browser-based games and applications
- Progressive Web Apps (PWA)
- WebGL/WebGPU rendering

### DesktopPlatform

**Target**: Electron, Tauri, Node.js  
**Lines of Code**: 621

**Features**:
- Native window management (Electron BrowserWindow)
- Node.js file system (fs module)
- Worker threads for multi-threading
- Native dialogs and clipboard
- System-level capabilities

**Use Cases**:
- Desktop game applications
- Native tools and editors
- Offline-capable applications

### MobilePlatform

**Target**: iOS, Android (React Native, Capacitor)  
**Lines of Code**: 551

**Features**:
- Mobile screen management
- Touch-optimized input
- React Native/Capacitor file system integration
- Mobile-specific capability detection
- Device-aware rendering (devicePixelRatio)

**Use Cases**:
- Mobile games
- Touch-based applications
- Platform-specific optimizations

## Usage

### Auto-Detection

The Platform Manager automatically detects the current platform:

```typescript
import { initializePlatform } from '@nova-engine/platform';

// Automatically detects and initializes the correct platform
const platform = await initializePlatform();

console.log(`Running on: ${platform.getName()}`);
console.log(`Capabilities:`, platform.getCapabilities());
```

### Manual Selection

```typescript
import { WebPlatform, DesktopPlatform, MobilePlatform } from '@nova-engine/platform';

// Explicitly create a platform instance
const platform = new WebPlatform();
await platform.initialize();
```

### Creating Windows

```typescript
const window = platform.createWindow({
  title: 'Nova Engine',
  width: 1920,
  height: 1080,
  fullscreen: false,
  resizable: true,
});

console.log(`Window: ${window.getWidth()}x${window.getHeight()}`);
```

### File System Operations

```typescript
const fs = platform.getFileSystem();

// Write file
await fs.writeTextFile('/data/config.json', JSON.stringify(config));

// Read file
const data = await fs.readTextFile('/data/config.json');

// Check existence
if (await fs.exists('/data/save.dat')) {
  const saveData = await fs.readFile('/data/save.dat');
}
```

### Threading

```typescript
const worker = platform.createWorker('/workers/physics.js');

worker.onMessage((data) => {
  console.log('Worker result:', data);
});

worker.postMessage({ type: 'simulate', deltaTime: 0.016 });
```

### Networking

```typescript
// HTTP Request
const response = await platform.fetch('https://api.example.com/data', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({ query: 'test' }),
});

const data = await response.json();

// WebSocket
const ws = platform.createWebSocket('wss://game.example.com/socket');

ws.onOpen(() => console.log('Connected'));
ws.onMessage((data) => console.log('Received:', data));
ws.send(JSON.stringify({ type: 'join', room: 'lobby' }));
```

## Platform Capabilities

Query platform capabilities to enable/disable features:

```typescript
const caps = platform.getCapabilities();

if (caps.supportsWebGPU) {
  // Initialize WebGPU renderer
} else if (caps.supportsWebGL2) {
  // Fallback to WebGL 2
} else if (caps.supportsWebGL) {
  // Fallback to WebGL 1
}

if (caps.supportsTouch) {
  // Enable touch controls
}

if (caps.supportsGamepad) {
  // Enable gamepad support
}

console.log(`Max texture size: ${caps.maxTextureSize}`);
console.log(`Device pixel ratio: ${caps.devicePixelRatio}`);
```

## Platform Information

```typescript
const info = platform.getInfo();

console.log(`Platform: ${info.name}`);
console.log(`OS: ${info.os}`);
console.log(`Architecture: ${info.arch}`);
console.log(`Device: ${info.deviceType}`);
console.log(`Development mode: ${info.isDevelopment}`);
```

## Testing

The platform layer includes 70+ comprehensive tests covering:

- Platform initialization and shutdown
- Window creation and management
- File system operations
- Threading and workers
- Timing functions
- Input device enumeration
- Network operations
- Clipboard and dialogs
- Event handling

Run tests:
```bash
npm test -- --testPathPattern=platform
```

## Future Enhancements

- **Console Platform Support**: PlayStation, Xbox, Nintendo Switch
- **VR/AR Platform Support**: Quest, PSVR2, Vision Pro
- **Cloud Gaming Platform**: Streaming-optimized platform
- **Headless Platform**: Server-side rendering and simulations

## Design Principles

1. **Unified Interface**: Single API across all platforms
2. **Zero Dependencies**: No platform-specific code in engine core
3. **Capability Detection**: Runtime feature detection, not compile-time
4. **Graceful Degradation**: Fallbacks for missing features
5. **Performance**: Minimal overhead, direct API calls where possible
6. **Type Safety**: Full TypeScript support with strict types

## Performance Considerations

- **File System**: Web uses IndexedDB (async), Desktop uses Node.js fs (async)
- **Threading**: Worker creation has startup cost, pool workers for best performance
- **Window Management**: Creating windows is expensive, reuse when possible
- **Network**: Use WebSocket for persistent connections, fetch for one-off requests

## Contributing

When adding new platform implementations:

1. Implement the full `IPlatform` interface
2. Add comprehensive tests (50+ per platform)
3. Document platform-specific behaviors
4. Update `PlatformManager` detection logic
5. Add usage examples to this README

## License

See LICENSE file in repository root.

---

**Part of Phase 1: Core Engine Architecture**  
**Target**: 10k LOC, 50+ tests  
**Status**: 95% Complete (5.5k LOC, 70+ tests)
