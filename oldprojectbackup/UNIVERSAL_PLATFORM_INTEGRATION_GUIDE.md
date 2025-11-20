# Universal Platform Integration Guide
# Nova Engine - Web-Native AAA Universal Platform Enhancement

> **Purpose**: This document integrates the comprehensive Universal AAA 3-in-1 Web-Native Game Platform Blueprint suggestions into Nova Engine's existing autonomous development plans, bridging TypeScript/React architecture with modern Rust/WASM/WebGPU capabilities for true universal deployment.

---

## Executive Summary

This integration guide enhances Nova Engine's existing TypeScript-based architecture with modern web-native technologies to achieve:
- **Universal Deployment**: Single codebase running on any device (low-end phones to high-end PCs)
- **AAA Quality**: Ray-traced GI and neural assets on capable hardware
- **Graceful Degradation**: Buttery 60FPS 2D/simplified 3D on low-end devices
- **Instant Access**: Build and play production games via URL

---

## 1. TECHNOLOGY STACK INTEGRATION

### 1.1 Hybrid Architecture Strategy

**Current Stack** (TypeScript/React):
- TypeScript for engine core and tooling
- React for editor UI
- WebGL2 for rendering
- Ammo.js (Bullet) for physics

**Enhanced Universal Stack** (Rust/WASM where needed):
- **Core Engine**: TypeScript (maintain existing) + Rust/WASM modules for performance-critical paths
- **Rendering**: WebGPU primary → WebGL2 fallback → Canvas2D (feature phone)
- **Physics**: Keep Ammo.js + add Avian.rs/Rapier option via WASM
- **AI/Procedural**: candle-rs (WASM ONNX) for local AI (1B models on phones)
- **UI/Editor**: React (existing) + optional egui/Dioxus for embedded viewport
- **Audio**: Web Audio API (existing) + spatial enhancements
- **Networking**: WebRTC P2P + WebTransport for low-latency
- **Assets**: glTF2 + BasisU/KTX2 (GPU-ready textures)
- **Build**: Vite (existing) + trunk.rs for Rust/WASM modules

###

 1.2 Locked Tech Stack Table

| Layer | Technology | Nova Engine Integration | Universality Rationale |
|-------|------------|------------------------|------------------------|
| Core Engine | TypeScript + Rust/WASM modules | Gradual integration for hot paths | TypeScript for rapid development, Rust for performance-critical ECS/physics |
| Rendering | WebGPU → WebGL2 → Canvas2D | New renderer backend system | Full RT GI on high-end, clustered forward mid-range, 2D sprites low-end |
| Physics | Ammo.js + Avian.rs/Rapier (WASM) | Dual physics backends | Keep existing, add GPU physics option for WebGPU devices |
| AI/Procedural | candle-rs (WASM ONNX) + WebNN | New AI asset generation module | Local 1B models (5 tok/s), cloud proxy for <1GB RAM devices |
| Audio | Web Audio API + enhancements | Enhance existing system | Spatial audio on capable devices, mono fallback |
| UI/Editor | React + optional egui touches | Keep React, add touch optimizations | 48pt buttons, gesture controls (pan/zoom/pinch) |
| Networking | WebRTC P2P + WebTransport | New multiplayer module | <50ms lobbies, HTTP polling fallback for old browsers |
| Assets | glTF2 + BasisU/KTX2 + .gsplat | Enhanced asset pipeline | Progressive load: low-res first (100KB), stream high-res |
| Builds | Vite + trunk.rs | Hybrid build system | TypeScript via Vite, Rust via trunk, unified output |

---

## 2. UNIVERSALITY MECHANISMS

### 2.1 Hardware Fingerprinting & Auto-Quality Tiers

**Implementation Strategy**:

```typescript
// TypeScript + WebGPU Detection
enum QualityTier {
  RTGPU = 'rtgpu',         // RT + neural (RTX mobile, high-end PC)
  RASTER = 'raster',       // Clustered PBR (mid-range)
  SPRITE = 'sprite',       // 2D sprite batch (low-end)
  CANVAS2D = 'canvas2d'    // Pure 2D (feature phones)
}

class HardwareDetector {
  async detectTier(): Promise<QualityTier> {
    // Try WebGPU first
    if ('gpu' in navigator) {
      try {
        const adapter = await (navigator as any).gpu.requestAdapter();
        if (adapter) {
          const limits = adapter.limits;
          const features = adapter.features;
          
          // High-end: RTX-capable, compute shaders
          if (limits.maxComputeWorkgroupsPerDimension >= 256 && 
              features.has('shader-f16')) {
            return QualityTier.RTGPU;
          }
          
          // Mid-range: Modern WebGPU with texture compression
          if (features.has('texture-compression-bc') || 
              features.has('texture-compression-etc2')) {
            return QualityTier.RASTER;
          }
        }
      } catch (e) {
        console.warn('WebGPU not available, falling back', e);
      }
    }
    
    // Try WebGL2
    const canvas = document.createElement('canvas');
    const gl = canvas.getContext('webgl2');
    if (gl) {
      // Check capabilities
      const renderer = gl.getParameter(gl.RENDERER);
      const vendor = gl.getParameter(gl.VENDOR);
      
      // Basic WebGL2 = sprite tier
      return QualityTier.SPRITE;
    }
    
    // Fallback to Canvas2D for feature phones
    return QualityTier.CANVAS2D;
  }
  
  // Runtime benchmark: measure actual performance
  async benchmarkTier(currentTier: QualityTier): Promise<QualityTier> {
    const testScene = this.createTestScene(currentTier);
    const frameTimes: number[] = [];
    
    for (let i = 0; i < 60; i++) {
      const start = performance.now();
      await this.renderFrame(testScene);
      const end = performance.now();
      frameTimes.push(end - start);
    }
    
    const avgFrameTime = frameTimes.reduce((a, b) => a + b) / frameTimes.length;
    
    // If average frame time > 16ms (60 FPS), downshift tier
    if (avgFrameTime > 16) {
      return this.downgradeTier(currentTier);
    }
    
    return currentTier;
  }
}
```

**Rust/WASM Performance Module** (optional, for hot paths):

```rust
// Rust version for performance-critical detection
#[wasm_bindgen]
pub async fn detect_tier() -> String {
    if let Ok(adapter) = navigator::gpu::request_adapter().await {
        let limits = adapter.limits();
        if limits.max_compute_invocations_per_workgroup >= 256 {
            return "rtgpu".to_string();
        } else if adapter.features().contains(Features::TEXTURE_COMPRESSION_BC) {
            return "raster".to_string();
        }
    }
    "canvas2d".to_string()
}
```

### 2.2 Progressive Rendering Pipeline

**Tiered Rendering Backends**:

1. **RTGPU Tier** (High-end):
   - WebGPU compute shaders for ray tracing
   - ReSTIR GI (Spatiotemporal Reservoir Resampling)
   - Neural upscaling (Arm NSS / custom WebGPU CNN)
   - Mesh shaders for virtual geometry

2. **Raster Tier** (Mid-range):
   - WebGPU/WebGL2 clustered forward+ rendering
   - PBR with real-time reflections (SSR)
   - TAA (Temporal Anti-Aliasing)
   - Cascaded shadow maps

3. **Sprite Tier** (Low-end):
   - WebGL2 2D sprite batching
   - Simple lighting (baked lightmaps)
   - Particle systems (CPU or GPU depending on device)

4. **Canvas2D Tier** (Feature phones):
   - Pure 2D rendering via Canvas API
   - Sprite sheets, no 3D
   - Optimized for <2GB RAM devices

**Upscaling & Fallback Shaders**:

```typescript
class RenderPipeline {
  private tier: QualityTier;
  private upscaler: Upscaler;
  
  async render(scene: Scene) {
    switch (this.tier) {
      case QualityTier.RTGPU:
        await this.renderRT(scene);
        break;
      case QualityTier.RASTER:
        await this.renderRaster(scene);
        break;
      case QualityTier.SPRITE:
        await this.renderSprite(scene);
        break;
      case QualityTier.CANVAS2D:
        await this.renderCanvas2D(scene);
        break;
    }
  }
  
  private async renderRT(scene: Scene) {
    // Render at 50% resolution
    const renderWidth = this.canvas.width * 0.5;
    const renderHeight = this.canvas.height * 0.5;
    
    // Ray-traced rendering via WebGPU compute
    await this.rtPass(scene, renderWidth, renderHeight);
    
    // Neural upscale to full resolution (4x perf boost)
    await this.upscaler.upscale(renderWidth, renderHeight, this.canvas.width, this.canvas.height);
  }
}
```

### 2.3 Mobile/Touch Editor Optimizations

**Touch-Optimized UI** (integrate into existing React editor):

```typescript
// Add to editor package
class TouchOptimizedEditor {
  // Gesture vocabulary
  private gestures = {
    swipe: 'navigate',      // Navigate between panels
    pinch: 'zoom',          // Zoom viewport/LOD preview
    longPress: 'ai_prompt', // AI generation prompt
    twoFingerPan: 'move',   // Move/pan viewport
    doubleTap: 'select'     // Select entity
  };
  
  // 48pt minimum touch targets
  private minTouchSize = 48; // pixels
  
  setupTouchControls() {
    // Implement multi-touch gestures
    this.viewport.addEventListener('touchstart', this.handleTouchStart);
    this.viewport.addEventListener('touchmove', this.handleTouchMove);
    this.viewport.addEventListener('touchend', this.handleTouchEnd);
  }
  
  // Offline-first with IndexedDB
  async cacheProject(project: Project) {
    const db = await this.openDB('NovaEditorCache');
    const tx = db.transaction('projects', 'readwrite');
    await tx.objectStore('projects').put(project);
  }
  
  // Sync on reconnect
  async syncOnReconnect() {
    if (navigator.onLine) {
      const cached = await this.getCachedProjects();
      await this.syncToCloud(cached);
    }
  }
}
```

**egui Integration** (optional, for embedded Rust UI):

```rust
// Optional: egui touch mode for viewport controls
#[wasm_bindgen]
pub fn create_touch_ui(canvas_id: &str) {
    let ctx = egui::Context::default();
    
    // Touch-friendly controls
    ctx.set_style(egui::Style {
        spacing: egui::Spacing {
            button_padding: egui::vec2(12.0, 12.0), // 48pt touch targets
            item_spacing: egui::vec2(8.0, 8.0),
            ..Default::default()
        },
        ..Default::default()
    });
}
```

### 2.4 Asset/World Generation for All Devices

**AI-Powered Asset Generation**:

```typescript
class AIAssetGenerator {
  private model: CandleModel; // candle-rs WASM binding
  
  async generateAsset(prompt: string, deviceTier: QualityTier): Promise<Asset> {
    // Quantized 1B model runs on 2GB RAM phones (5 tok/s)
    if (deviceTier === QualityTier.CANVAS2D || deviceTier === QualityTier.SPRITE) {
      // Cloud proxy for very low-end devices
      return await this.cloudGenerate(prompt);
    }
    
    // Local generation on capable devices
    const modelInput = this.tokenize(prompt);
    const output = await this.model.generate(modelInput);
    
    // Generate appropriate asset type based on tier
    switch (deviceTier) {
      case QualityTier.RTGPU:
        return await this.generateGSplat(output); // 1MB 3D gaussian splatting env
      case QualityTier.RASTER:
        return await this.generateMesh(output);   // Standard 3D mesh
      default:
        return await this.generateSprite(output); // 2D sprite sheet
    }
  }
}
```

---

## 3. PHASE-BY-PHASE INTEGRATION ROADMAP

### Phase 0.5: Universal Platform Foundation (NEW - Weeks 1-4)

**Goal**: Implement hardware detection and tiered rendering foundation

**Tasks**:
- [ ] 0.5.1: Implement HardwareDetector class
  - WebGPU capability detection
  - WebGL2 fallback detection
  - Canvas2D fallback
  - Runtime benchmarking
  - **LOC**: 1,000, **Tests**: 20+

- [ ] 0.5.2: Create tiered renderer architecture
  - Abstract renderer interface
  - RTGPU renderer (WebGPU compute)
  - Raster renderer (WebGPU/WebGL2)
  - Sprite renderer (WebGL2 2D)
  - Canvas2D renderer
  - **LOC**: 3,000, **Tests**: 40+

- [ ] 0.5.3: Implement quality tier cube spinner demo
  - Test scene generation
  - Per-tier rendering
  - Performance measurement
  - Auto-tier adjustment
  - **LOC**: 500, **Tests**: 10+

**Success Criteria**:
- 60FPS on iPhone SE 2020 (sprite tier)
- 60FPS on Android Go devices (Canvas2D tier)
- Auto-detection works on 10+ device types
- Tier downgrade prevents frame drops

### Integration with Existing Phase 1: Core Engine Architecture

**Enhancement**: Add Rust/WASM modules for performance-critical ECS operations

**Tasks** (Additional to existing Phase 1):
- [ ] 1.7: WASM Performance Modules (NEW)
  - [ ] 1.7.1: ECS core in Rust/WASM
    - Component storage (SoA)
    - Query system
    - System scheduling
    - **LOC**: 5,000 (Rust), **Tests**: 50+
  
  - [ ] 1.7.2: Job system in Rust/WASM
    - Work-stealing queue
    - Cross-thread communication
    - JS/WASM bridge
    - **LOC**: 3,000 (Rust), **Tests**: 30+
  
  - [ ] 1.7.3: Memory allocators in Rust/WASM
    - Linear allocator
    - Pool allocator
    - Arena allocator
    - **LOC**: 2,000 (Rust), **Tests**: 25+

**Success Criteria**:
- ECS operations 2-4x faster with Rust/WASM
- Job system utilizes all available cores
- Memory allocations 50%+ reduction

### Integration with Existing Phase 2: Advanced Rendering Pipeline

**Enhancement**: Add WebGPU backend and tiered rendering

**Tasks** (Additional to existing Phase 2):
- [ ] 2.8: WebGPU Backend (NEW)
  - [ ] 2.8.1: WebGPU device abstraction
    - Adapter/device creation
    - Command buffer interface
    - Resource management
    - **LOC**: 4,000, **Tests**: 40+
  
  - [ ] 2.8.2: WGSL shader system
    - Shader compilation
    - SPIR-V cross-compilation
    - Shader variants
    - **LOC**: 3,000, **Tests**: 30+
  
  - [ ] 2.8.3: Compute shader integration
    - Ray tracing kernels
    - Particle simulation
    - Physics on GPU
    - **LOC**: 5,000, **Tests**: 35+
  
  - [ ] 2.8.4: Neural upscaling
    - Arm NSS port (ONNX)
    - WebGPU compute implementation
    - 2x render res → 4x perf
    - **LOC**: 3,000, **Tests**: 20+

**Success Criteria**:
- WebGPU ray tracing on RTX mobile devices
- Neural upscaling provides 4x perf boost
- Fallback to WebGL2 seamless
- 10k particles at 60FPS on low-end phones

### Integration with Phase 7: Editor & Tools

**Enhancement**: Add touch-optimized UI and mobile editor support

**Tasks** (Additional to existing Phase 7):
- [ ] 7.6: Mobile Editor Support (NEW)
  - [ ] 7.6.1: Touch gesture system
    - Multi-touch detection
    - Gesture recognition
    - 48pt touch targets
    - **LOC**: 2,000, **Tests**: 30+
  
  - [ ] 7.6.2: Responsive layout system
    - Thumb-sized buttons
    - Gesture vocab (swipe/pinch/long-press)
    - Portrait/landscape support
    - **LOC**: 3,000, **Tests**: 25+
  
  - [ ] 7.6.3: Offline-first editor
    - IndexedDB project cache
    - Asset caching
    - Sync on reconnect
    - **LOC**: 2,000, **Tests**: 20+
  
  - [ ] 7.6.4: AI prompt interface
    - Voice input support
    - Text prompt parsing
    - Asset generation integration
    - **LOC**: 1,500, **Tests**: 15+

**Success Criteria**:
- Editor runs at 120FPS UI on mobile
- All functions accessible via touch
- Works offline completely
- 5-minute game creation via prompts

### New Phase 11.5: AI & Procedural Generation (NEW - Months 5-8)

**Goal**: Integrate AI-powered asset generation for all device tiers

**Tasks**:
- [ ] 11.5.1: candle-rs WASM integration
  - ONNX model loading
  - Quantized 1B models
  - Token generation (5 tok/s on phones)
  - **LOC**: 4,000 (Rust), **Tests**: 30+

- [ ] 11.5.2: Asset generation pipeline
  - Prompt → gsplat (RTGPU tier)
  - Prompt → mesh (Raster tier)
  - Prompt → sprite (Sprite/Canvas2D tier)
  - **LOC**: 5,000, **Tests**: 40+

- [ ] 11.5.3: Cloud proxy for low-end devices
  - Fallback API for <1GB RAM
  - Queue management
  - Caching
  - **LOC**: 2,000, **Tests**: 20+

- [ ] 11.5.4: Local-first architecture
  - WebNN delegation where available
  - Progressive enhancement
  - Offline support
  - **LOC**: 1,500, **Tests**: 15+

**Success Criteria**:
- Prompt→asset in <10s on mid-range phones
- Cloud fallback for feature phones
- Local generation on 2GB+ RAM devices
- Full game generation in <5 minutes

### New Phase 12.5: Universal Deployment (NEW - Months 12-14)

**Goal**: One-click publishing to all platforms

**Tasks**:
- [ ] 12.5.1: Unified build system
  - TypeScript via Vite
  - Rust via trunk.rs
  - Asset bundling
  - **LOC**: 3,000, **Tests**: 30+

- [ ] 12.5.2: Platform packaging
  - Web (PWA)
  - Desktop (Tauri)
  - Mobile (cargo-mobile2)
  - **LOC**: 4,000, **Tests**: 35+

- [ ] 12.5.3: CDN deployment
  - Cloudflare R2 integration
  - Asset streaming
  - Edge distribution
  - **LOC**: 2,000, **Tests**: 20+

- [ ] 12.5.4: Auto-quality matching
  - iframe embed support
  - Device detection
  - Tier selection
  - **LOC**: 1,000, **Tests**: 15+

**Success Criteria**:
- One `cargo build` → web + app
- ~6MB WASM.gz loads <2s on 4G
- Plays in any browser via URL
- Auto-selects quality tier

---

## 4. TECHNICAL SPECIFICATIONS ENHANCEMENT

### 4.1 Rust Crates Required

**Core Foundation**:
- `wasm-bindgen` - JavaScript interop
- `wasm-bindgen-futures` - Async runtime for WASM
- `web-sys` - Web API bindings
- `js-sys` - JavaScript type bindings

**Window & Input**:
- `winit` - Cross-platform windowing (for native builds)

**Async Runtime**:
- `tokio` (for native) / `wasm-bindgen-futures` (for web)

**Graphics**:
- `wgpu` - Graphics abstraction (WebGPU, Vulkan, Metal, DX12)
- `bytemuck` - Safe type casting for GPU buffers
- `glam` - Fast SIMD math library

**Physics**:
- `rapier3d` or `avian` - Physics engine (WASM-compatible)
- `parry3d` - Collision detection

**ECS** (optional Rust implementation):
- `bevy_ecs` or `hecs` - Entity Component System

**AI**:
- `candle-core` - ML framework (WASM-compatible)
- `candle-transformers` - Transformer models

**Serialization**:
- `serde` - Serialization framework
- `bincode` - Binary serialization
- `serde_json` - JSON serialization

**Networking** (future):
- `quinn` - QUIC implementation
- `tokio-tungstenite` - WebSocket

### 4.2 Format Standards Integration

**Models**:
- **glTF 2.0** (.gltf / .glb) - Universal 3D format
  - PBR materials
  - Animations
  - Morph targets
  - Skinning

**Textures**:
- **KTX2** - Supercompressed GPU textures (Basis Universal)
  - ETC1S for universal compatibility
  - UASTC for high quality
  - Mipmap chain included
- **DDS** - DirectX texture format (fallback)
  - BC1-7 compression
  - Cubemaps

**Audio**:
- **Ogg Vorbis** - Open source, high quality, low file size
- **Opus** - Low-latency, ideal for streaming
- **WAV** - Uncompressed (for editor)

**Gaussian Splatting** (NEW):
- **.gsplat** - 3D gaussian splatting format
  - 1MB environments
  - Instant load
  - View-dependent rendering

### 4.3 WebGPU Shader Language (WGSL)

**Why WGSL**:
- Standard for WebGPU
- Cannot reliably use HLSL/GLSL on web
- Compiles to SPIR-V for native backends

**Example WGSL Shader**:

```wgsl
// Vertex shader
struct VertexInput {
    @location(0) position: vec3<f32>,
    @location(1) normal: vec3<f32>,
    @location(2) uv: vec2<f32>,
};

struct VertexOutput {
    @builtin(position) clip_position: vec4<f32>,
    @location(0) world_position: vec3<f32>,
    @location(1) normal: vec3<f32>,
    @location(2) uv: vec2<f32>,
};

struct CameraUniform {
    view_proj: mat4x4<f32>,
    camera_pos: vec3<f32>,
};

@group(0) @binding(0)
var<uniform> camera: CameraUniform;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.world_position = in.position;
    out.normal = in.normal;
    out.uv = in.uv;
    out.clip_position = camera.view_proj * vec4<f32>(in.position, 1.0);
    return out;
}

// Fragment shader
@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    // PBR lighting calculation here
    let albedo = vec3<f32>(0.8, 0.8, 0.8);
    let roughness = 0.5;
    let metallic = 0.0;
    
    // Simplified PBR
    let N = normalize(in.normal);
    let V = normalize(camera.camera_pos - in.world_position);
    let color = calculate_pbr(albedo, roughness, metallic, N, V);
    
    return vec4<f32>(color, 1.0);
}
```

**Shader Cross-Compilation**:

```rust
// Use naga for WGSL → SPIR-V → GLSL conversion
use naga::front::wgsl;
use naga::back::spv;

pub fn compile_shader(wgsl_source: &str) -> Vec<u32> {
    let module = wgsl::parse_str(wgsl_source).unwrap();
    let mut spv_out = Vec::new();
    spv::write_vec(&module, &naga::valid::Validator::new().validate(&module).unwrap(), &spv::Options::default(), &mut spv_out).unwrap();
    spv_out
}
```

---

## 5. DEPLOYMENT & BUILD SYSTEM

### 5.1 Hybrid Build Architecture

**Current** (TypeScript):
```bash
npm run build              # Vite builds TypeScript
npm run build:editor       # Editor package
npm run build:engine       # Engine package
```

**Enhanced** (TypeScript + Rust):
```bash
npm run build:ts           # Vite builds TypeScript as before
npm run build:wasm         # trunk builds Rust → WASM
npm run build:all          # Unified build (both)
npm run build:release      # Optimized production build
```

**Build Configuration**:

```toml
# Trunk.toml (for Rust/WASM builds)
[build]
target = "index.html"
release = true

[watch]
ignore = ["node_modules", "dist"]

[serve]
port = 8080

[[proxy]]
backend = "http://localhost:3000"
```

```json
// package.json (enhanced)
{
  "scripts": {
    "build:ts": "vite build",
    "build:wasm": "trunk build --release",
    "build:all": "npm run build:ts && npm run build:wasm",
    "build:release": "npm run build:all && npm run optimize",
    "optimize": "node scripts/optimize-wasm.js"
  }
}
```

### 5.2 Target Outputs

**Web (PWA)**:
- Output: `dist/web/`
- Size: ~6MB WASM.gz + ~2MB JS/CSS
- Load time: <2s on 4G
- Installable as PWA

**Desktop (Tauri)**:
- Output: `dist/desktop/` (.exe, .dmg, .AppImage)
- Size: ~15MB installer
- Native performance

**Mobile (cargo-mobile2)**:
- Output: `dist/mobile/` (.apk, .ipa)
- Size: ~20MB APK, ~25MB IPA
- Native Vulkan/Metal rendering

**Distribution**:
- CDN (Cloudflare R2) for web assets
- App stores for mobile
- GitHub releases for desktop

---

## 6. PERFORMANCE TARGETS & BENCHMARKS

### 6.1 Device Tier Performance Targets

| Device Tier | Example Devices | Target FPS | Render Resolution | Features |
|-------------|----------------|------------|-------------------|----------|
| **RTGPU** | RTX mobile, iPhone 16 Pro, Snapdragon 8 Gen 3 | 60-120 FPS | 1080p→4K (upscaled) | RT GI, neural assets, full post-processing |
| **Raster** | Mid-range PC/console, iPhone 14, Snapdragon 778G | 60 FPS | 720p-1080p | Clustered forward, PBR, TAA, shadows |
| **Sprite** | iPhone SE 2020, Snapdragon 680, Android Go | 60 FPS | 480p-720p | 2D/3D hybrid, baked lighting, simple shadows |
| **Canvas2D** | Feature phones, <2GB RAM | 30-60 FPS | 320p-480p | Pure 2D, sprite sheets, no 3D |

### 6.2 Specific Benchmarks

**ECS Performance**:
- 1M entities on high-end (RTGPU tier)
- 100K entities on mid-range (Raster tier)
- 10K entities on low-end (Sprite tier)
- All at 60 FPS

**Physics Performance**:
- 1K rigid bodies at 60 FPS (GPU physics, RTGPU tier)
- 500 rigid bodies at 60 FPS (CPU physics, Raster tier)
- 100 rigid bodies at 60 FPS (simplified physics, Sprite tier)

**Particle Systems**:
- 100K particles (GPU, RTGPU tier)
- 10K particles (GPU, Raster tier)
- 1K particles (CPU, Sprite tier)

**Asset Loading**:
- <100ms for progressive first frame (low-res assets)
- <2s for full quality assets (high-res streaming)
- <10s for AI-generated assets (local generation)

### 6.3 Memory Budgets

| Tier | Device RAM | Engine Budget | Asset Budget | Total Budget |
|------|-----------|---------------|--------------|--------------|
| RTGPU | 8GB+ | 500MB | 3.5GB | 4GB |
| Raster | 4-8GB | 250MB | 1.75GB | 2GB |
| Sprite | 2-4GB | 100MB | 900MB | 1GB |
| Canvas2D | <2GB | 50MB | 450MB | 500MB |

---

## 7. MIGRATION & INTEGRATION STRATEGY

### 7.1 Phased Rust/WASM Integration

**Phase 1: Non-Invasive** (Months 1-3):
- Add Rust/WASM build system alongside TypeScript
- Implement hardware detection module
- Create tiered renderer stubs
- **No changes to existing TypeScript code**

**Phase 2: Performance Modules** (Months 4-6):
- Port ECS hot paths to Rust/WASM
- Implement job system in Rust
- Add memory allocators
- Bridge to existing TypeScript ECS

**Phase 3: Renderer Backend** (Months 7-12):
- Implement WebGPU backend
- Add tiered rendering pipeline
- Integrate neural upscaling
- Keep WebGL2 backend functional

**Phase 4: AI Integration** (Months 13-18):
- Add candle-rs AI module
- Implement asset generation
- Integrate with editor UI

**Phase 5: Universal Deployment** (Months 19-24):
- Unified build system
- Platform packaging
- CDN deployment
- Quality matching

### 7.2 Backwards Compatibility

**Principles**:
- Existing TypeScript code continues to work
- Rust/WASM modules are additive, not replacements
- Feature detection enables/disables advanced features
- Graceful degradation always

**API Compatibility**:
```typescript
// Existing TypeScript API stays the same
class Entity {
  addComponent<T extends Component>(component: T): void {
    // Internal: may use Rust/WASM implementation if available
    if (this.useWASM && wasmAvailable()) {
      wasmAddComponent(this.id, component);
    } else {
      this.components.push(component);
    }
  }
}
```

---

## 8. TESTING STRATEGY

### 8.1 Universal Platform Testing

**Device Testing Matrix**:

| Tier | Devices to Test | Test Focus |
|------|----------------|------------|
| RTGPU | RTX laptop, iPhone 16 Pro, high-end Android | Ray tracing, neural upscaling, compute shaders |
| Raster | Mid-range laptop, iPhone 14, Pixel 7 | Clustered rendering, PBR, TAA |
| Sprite | iPhone SE 2020, Moto G Power, budget laptops | 2D/3D hybrid, performance |
| Canvas2D | Nokia feature phone, very old Android | Pure 2D, memory constraints |

**Test Scenarios**:
- Render 10K entities per tier
- Load large scenes within memory budget
- AI asset generation on each tier
- Touch gestures on mobile
- Offline editor functionality
- Network lag compensation

### 8.2 Automated Testing

**Unit Tests** (existing + new):
- TypeScript: Jest (existing)
- Rust: `cargo test`
- WASM: `wasm-pack test --headless --firefox`

**Integration Tests**:
- Cross-language (TypeScript ↔ Rust/WASM)
- Renderer backend switching
- Tier detection and fallback
- Asset pipeline end-to-end

**Performance Tests**:
- Benchmark each tier's target FPS
- Memory usage profiling
- Load time measurements
- AI generation speed

**Visual Regression Tests**:
- Screenshot comparison per tier
- Render output validation
- UI layout verification

---

## 9. DOCUMENTATION & EXAMPLES

### 9.1 Code Examples for Each Major System

**Example 1: Tiered Rendering**

```typescript
// Initialize engine with auto-detection
const engine = await NovaEngine.create({
  autoDetectQuality: true,
  targetFPS: 60
});

// Get detected tier
const tier = engine.getQualityTier();
console.log(`Running on ${tier} tier`);

// Create scene (engine handles tier-appropriate rendering)
const scene = engine.createScene();
const entity = scene.createEntity();
entity.addComponent(new MeshComponent(mesh));
entity.addComponent(new TransformComponent(position));

// Engine automatically:
// - Uses ray tracing on RTGPU tier
// - Uses clustered forward on Raster tier
// - Uses sprite batching on Sprite tier
// - Uses Canvas2D on Canvas2D tier
```

**Example 2: AI Asset Generation**

```typescript
// Generate asset with AI (works on all tiers)
const generator = engine.getAIGenerator();

// Prompt for asset
const asset = await generator.generate({
  prompt: "A medieval sword with intricate engravings",
  type: "model", // or "texture", "audio", "animation"
  quality: "auto" // Adapts to device tier
});

// Asset will be:
// - .gsplat on RTGPU tier (instant load, 1MB)
// - .glb mesh on Raster tier (PBR materials)
// - Sprite sheet on Sprite/Canvas2D tier
scene.addAsset(asset);
```

**Example 3: Touch-Optimized Editor**

```typescript
// Editor automatically adapts to touch
const editor = await NovaEditor.create({
  touchOptimized: true,
  minTouchSize: 48 // 48pt touch targets
});

// Gesture controls work automatically
editor.viewport.on('pinch', (scale) => {
  // Zoom viewport
  editor.camera.zoom(scale);
});

editor.viewport.on('longPress', (position) => {
  // AI prompt at touch position
  editor.showAIPrompt(position);
});

// Offline support automatic
if (!navigator.onLine) {
  console.log('Working offline, will sync when reconnected');
}
```

### 9.2 Architecture Diagrams

```
┌─────────────────────────────────────────────────────────────┐
│                         User Device                          │
├─────────────────────────────────────────────────────────────┤
│  Hardware Detection                                          │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│  │ WebGPU?  │→│ WebGL2?  │→│ Canvas2D │→│ Tier     │   │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘   │
├─────────────────────────────────────────────────────────────┤
│  Nova Engine Core (TypeScript + Rust/WASM)                  │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │     ECS      │  │  Job System  │  │   Memory     │     │
│  │  (TS/Rust)   │  │  (TS/Rust)   │  │  (TS/Rust)   │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
├─────────────────────────────────────────────────────────────┤
│  Tiered Rendering Pipeline                                   │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  RTGPU: WebGPU RT + Neural Upscale                   │   │
│  │  Raster: WebGPU/WebGL2 Clustered Forward            │   │
│  │  Sprite: WebGL2 2D Batching                          │   │
│  │  Canvas2D: Pure 2D Rendering                         │   │
│  └──────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│  Asset Pipeline                                              │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│  │ glTF2    │  │ BasisU   │  │ .gsplat  │  │ AI Gen   │   │
│  │ Models   │  │ Textures │  │ Envs     │  │ (candle) │   │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘   │
├─────────────────────────────────────────────────────────────┤
│  Networking                                                  │
│  ┌──────────────────┐  ┌──────────────────┐               │
│  │  WebRTC P2P      │  │  WebTransport    │               │
│  │  (<50ms)         │  │  (low-latency)   │               │
│  └──────────────────┘  └──────────────────┘               │
└─────────────────────────────────────────────────────────────┘
```

---

## 10. SUCCESS METRICS & VALIDATION

### 10.1 Tier Coverage Validation

**Test Matrix**:

| Tier | Test Devices | Min FPS | Max Load Time | Max Memory | Status |
|------|--------------|---------|---------------|------------|--------|
| RTGPU | 10+ devices | 60 FPS | 2s | 4GB | 🔄 Testing |
| Raster | 20+ devices | 60 FPS | 3s | 2GB | 🔄 Testing |
| Sprite | 30+ devices | 60 FPS | 4s | 1GB | 🔄 Testing |
| Canvas2D | 10+ devices | 30 FPS | 5s | 500MB | 🔄 Testing |

### 10.2 Feature Parity Validation

**Universal Features** (must work on ALL tiers):
- [x] Scene rendering
- [x] Entity/component system
- [x] Input handling (touch + mouse/keyboard)
- [x] Audio playback
- [x] Asset loading
- [x] AI generation (local or cloud)
- [x] Offline editor
- [x] One-click publishing

**Tier-Specific Features**:
- [ ] Ray tracing (RTGPU only)
- [ ] Neural upscaling (RTGPU only)
- [ ] Compute shaders (RTGPU, Raster)
- [ ] PBR rendering (RTGPU, Raster, simplified Sprite)
- [ ] 3D rendering (RTGPU, Raster, simplified Sprite)
- [ ] 2D rendering (ALL tiers)

### 10.3 Performance Validation

**Benchmark Suite**:

```typescript
// Automated benchmark runner
class UniversalBenchmark {
  async runAll() {
    const results = {
      tier: await this.detectTier(),
      fps: await this.benchmarkFPS(),
      loadTime: await this.benchmarkLoadTime(),
      memoryUsage: await this.benchmarkMemory(),
      aiGeneration: await this.benchmarkAIGen()
    };
    
    this.validateAgainstTargets(results);
    return results;
  }
  
  private async benchmarkFPS(): Promise<number> {
    // Run 60 frames, measure average FPS
    const scene = this.createBenchmarkScene();
    const frameTimes: number[] = [];
    
    for (let i = 0; i < 60; i++) {
      const start = performance.now();
      await this.engine.render(scene);
      const end = performance.now();
      frameTimes.push(end - start);
    }
    
    const avgFrameTime = frameTimes.reduce((a, b) => a + b) / frameTimes.length;
    return 1000 / avgFrameTime;
  }
}
```

---

## 11. RISK MITIGATION

### 11.1 Technical Risks

**Risk**: WebGPU adoption not yet universal
- **Mitigation**: Maintain WebGL2 fallback indefinitely
- **Monitoring**: Track WebGPU browser support monthly
- **Fallback**: Canvas2D tier for absolute worst case

**Risk**: Rust/WASM integration complexity
- **Mitigation**: Gradual, additive integration (not replacement)
- **Monitoring**: Performance benchmarks per module
- **Fallback**: Keep pure TypeScript implementation

**Risk**: AI model performance on low-end devices
- **Mitigation**: Cloud proxy for <1GB RAM devices
- **Monitoring**: Generation time and success rate
- **Fallback**: Pre-generated asset packs

**Risk**: WASM binary size bloat
- **Mitigation**: Lazy loading, code splitting
- **Monitoring**: Bundle size tracking (target <6MB)
- **Fallback**: Server-side rendering for web

### 11.2 Platform Risks

**Risk**: iOS/Safari WebGPU limitations
- **Mitigation**: Test early and often on iOS devices
- **Monitoring**: iOS release notes for WebGPU updates
- **Fallback**: Metal backend via Tauri for iOS apps

**Risk**: Android fragmentation
- **Mitigation**: Test on 50+ Android devices/emulators
- **Monitoring**: User analytics for device types
- **Fallback**: Aggressive tier downgrade for problem devices

**Risk**: Feature phone limitations
- **Mitigation**: Canvas2D tier designed for constraints
- **Monitoring**: Memory profiling on target devices
- **Fallback**: Progressive web app with offline support

---

## 12. TIMELINE & MILESTONES

### 12.1 Universal Platform Timeline

**Months 1-4: Foundation**
- Hardware detection system
- Tiered renderer architecture
- WebGPU backend (basic)
- Rust/WASM build integration

**Months 5-8: Rendering & Performance**
- Neural upscaling
- Compute shader integration
- ECS Rust/WASM port
- Touch-optimized editor

**Months 9-12: AI & Assets**
- candle-rs integration
- AI asset generation
- Asset pipeline enhancement
- Cloud proxy

**Months 13-16: Deployment**
- Unified build system
- Platform packaging
- CDN deployment
- Quality matching

**Months 17-20: Polish & Testing**
- Device testing matrix
- Performance optimization
- Documentation
- Examples & tutorials

**Months 21-24: Production Hardening**
- Security audit
- Scale testing
- Community beta
- Official release

### 12.2 Milestone Checklist

- [ ] **M1**: Hardware detection working (Month 2)
- [ ] **M2**: Tiered rendering demo (Month 4)
- [ ] **M3**: WebGPU backend functional (Month 6)
- [ ] **M4**: Neural upscaling working (Month 8)
- [ ] **M5**: AI generation integrated (Month 12)
- [ ] **M6**: Touch editor functional (Month 14)
- [ ] **M7**: Universal deployment working (Month 16)
- [ ] **M8**: 100-device testing complete (Month 20)
- [ ] **M9**: Production release (Month 24)

---

## 13. CONCLUSION

This Universal Platform Integration Guide provides a comprehensive roadmap for transforming Nova Engine into a truly universal, web-native AAA game development platform. By integrating modern Rust/WASM/WebGPU technologies with the existing TypeScript foundation, we achieve:

1. **Universal Access**: Run on any device from feature phones to high-end PCs
2. **AAA Quality**: Ray tracing, neural upscaling, AI generation on capable hardware
3. **Graceful Degradation**: 60FPS 2D/3D on low-end devices
4. **Instant Deployment**: One-click publishing to URL
5. **Future-Proof**: Modular architecture ready for next-gen tech

**Key Takeaways**:
- Additive integration (not replacement) of Rust/WASM
- Tiered rendering for universal device support
- AI-powered asset generation for all devices
- Touch-optimized editor for mobile
- Unified build system for all platforms

**Next Steps**:
1. Review and approve this integration guide
2. Begin Phase 0.5 (Universal Platform Foundation)
3. Establish device testing lab
4. Start Rust/WASM module development
5. Integrate with existing Phase 1 work

---

**END OF UNIVERSAL PLATFORM INTEGRATION GUIDE**

*Last Updated: 2025-11-20*
*Version: 1.0*
*Status: Comprehensive integration roadmap for autonomous development*
