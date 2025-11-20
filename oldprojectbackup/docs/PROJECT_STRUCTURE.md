# Nova Engine Editor - Project Structure

## Repository Organization

This document outlines the complete directory structure and organization for the Nova Engine Editor project.

---

## Root Directory Structure

```
Nova-Engine-editor/
├── .github/                    # GitHub configuration
│   ├── workflows/              # CI/CD workflows
│   ├── ISSUE_TEMPLATE/         # Issue templates
│   └── PULL_REQUEST_TEMPLATE.md
├── docs/                       # Documentation
│   ├── api/                    # API documentation
│   ├── guides/                 # User guides
│   ├── tutorials/              # Tutorial content
│   └── architecture/           # Architecture docs
├── packages/                   # Monorepo packages
│   ├── engine/                 # Core engine runtime
│   ├── editor/                 # Editor application
│   ├── server/                 # Local server
│   ├── shared/                 # Shared utilities
│   └── tools/                  # Build and dev tools
├── examples/                   # Example projects
│   ├── basic-scene/            # Basic 3D scene
│   ├── platformer/             # 2D platformer game
│   └── fps-demo/               # First-person shooter demo
├── scripts/                    # Build and utility scripts
├── tests/                      # Integration and E2E tests
├── .gitignore                  # Git ignore rules
├── .eslintrc.js                # ESLint configuration
├── .prettierrc                 # Prettier configuration
├── tsconfig.json               # TypeScript configuration
├── package.json                # Root package.json
├── lerna.json                  # Lerna configuration (if using)
├── README.md                   # Project readme
├── ROADMAP.md                  # Development roadmap
├── PROGRESS.md                 # Progress tracking
├── ARCHITECTURE.md             # Architecture documentation
├── PROJECT_STRUCTURE.md        # This file
├── CONTRIBUTING.md             # Contribution guidelines
├── LICENSE                     # License file
└── CHANGELOG.md                # Change log
```

---

## Package: Engine (Core Runtime)

**Location:** `packages/engine/`  
**Purpose:** Core game engine that runs in the browser

```
packages/engine/
├── src/
│   ├── core/                   # Core engine systems
│   │   ├── Engine.ts           # Main engine class
│   │   ├── Time.ts             # Time management
│   │   ├── Loop.ts             # Game loop
│   │   └── Application.ts      # Application lifecycle
│   ├── ecs/                    # Entity-Component-System
│   │   ├── Entity.ts           # Entity management
│   │   ├── Component.ts        # Component base
│   │   ├── System.ts           # System base
│   │   ├── World.ts            # ECS world
│   │   └── components/         # Built-in components
│   │       ├── Transform.ts
│   │       ├── MeshRenderer.ts
│   │       ├── Camera.ts
│   │       ├── Light.ts
│   │       └── RigidBody.ts
│   ├── graphics/               # Graphics rendering
│   │   ├── Renderer.ts         # Main renderer
│   │   ├── RenderPipeline.ts   # Render pipeline
│   │   ├── Shader.ts           # Shader management
│   │   ├── Material.ts         # Material system
│   │   ├── Texture.ts          # Texture management
│   │   ├── Mesh.ts             # Mesh data
│   │   ├── Camera.ts           # Camera system
│   │   ├── Light.ts            # Lighting system
│   │   └── shaders/            # Shader code
│   │       ├── standard.vert
│   │       ├── standard.frag
│   │       ├── pbr.vert
│   │       └── pbr.frag
│   ├── scene/                  # Scene management
│   │   ├── Scene.ts            # Scene class
│   │   ├── SceneGraph.ts       # Scene hierarchy
│   │   ├── SceneManager.ts     # Scene loading/switching
│   │   └── SceneSerializer.ts  # Scene serialization
│   ├── physics/                # Physics engine
│   │   ├── PhysicsWorld.ts     # Physics world
│   │   ├── RigidBody.ts        # Rigid body
│   │   ├── Collider.ts         # Colliders
│   │   ├── Constraint.ts       # Constraints
│   │   └── PhysicsSystem.ts    # Physics update system
│   ├── audio/                  # Audio system
│   │   ├── AudioSystem.ts      # Audio manager
│   │   ├── AudioSource.ts      # Audio source
│   │   ├── AudioListener.ts    # Audio listener
│   │   └── AudioEffects.ts     # Audio effects
│   ├── input/                  # Input handling
│   │   ├── InputManager.ts     # Input manager
│   │   ├── Keyboard.ts         # Keyboard input
│   │   ├── Mouse.ts            # Mouse input
│   │   ├── Touch.ts            # Touch input
│   │   ├── Gamepad.ts          # Gamepad input
│   │   └── InputMapping.ts     # Input mapping
│   ├── assets/                 # Asset management
│   │   ├── AssetManager.ts     # Asset loading
│   │   ├── AssetLoader.ts      # Asset loaders
│   │   ├── AssetCache.ts       # Asset caching
│   │   ├── loaders/            # Format-specific loaders
│   │   │   ├── GLTFLoader.ts
│   │   │   ├── OBJLoader.ts
│   │   │   ├── TextureLoader.ts
│   │   │   └── AudioLoader.ts
│   │   └── ResourceManager.ts  # Resource lifecycle
│   ├── math/                   # Math utilities
│   │   ├── Vector2.ts
│   │   ├── Vector3.ts
│   │   ├── Vector4.ts
│   │   ├── Quaternion.ts
│   │   ├── Matrix3.ts
│   │   ├── Matrix4.ts
│   │   ├── MathUtils.ts
│   │   └── Transform.ts
│   ├── animation/              # Animation system
│   │   ├── AnimationClip.ts    # Animation data
│   │   ├── AnimationController.ts # Animation control
│   │   ├── Animator.ts         # Animator component
│   │   ├── AnimationMixer.ts   # Animation blending
│   │   └── AnimationSystem.ts  # Animation update
│   ├── scripting/              # Scripting runtime
│   │   ├── ScriptComponent.ts  # Script component
│   │   ├── ScriptSystem.ts     # Script execution
│   │   └── API.ts              # Scripting API
│   └── utils/                  # Utility functions
│       ├── Logger.ts
│       ├── Events.ts
│       ├── Pool.ts
│       └── Debug.ts
├── tests/                      # Unit tests
├── tsconfig.json               # TypeScript config
├── package.json                # Package manifest
└── README.md                   # Package readme
```

---

## Package: Editor (Editor Application)

**Location:** `packages/editor/`  
**Purpose:** Web-based editor interface

```
packages/editor/
├── public/                     # Static assets
│   ├── index.html
│   ├── favicon.ico
│   └── manifest.json
├── src/
│   ├── App.tsx                 # Root component
│   ├── index.tsx               # Entry point
│   ├── components/             # React components
│   │   ├── layout/             # Layout components
│   │   │   ├── EditorShell.tsx
│   │   │   ├── MenuBar.tsx
│   │   │   ├── Toolbar.tsx
│   │   │   ├── StatusBar.tsx
│   │   │   └── PanelContainer.tsx
│   │   ├── panels/             # Editor panels
│   │   │   ├── Viewport.tsx
│   │   │   ├── Hierarchy.tsx
│   │   │   ├── Inspector.tsx
│   │   │   ├── AssetBrowser.tsx
│   │   │   ├── Console.tsx
│   │   │   ├── Timeline.tsx
│   │   │   └── MaterialEditor.tsx
│   │   ├── viewport/           # Viewport components
│   │   │   ├── ViewportCanvas.tsx
│   │   │   ├── Gizmos.tsx
│   │   │   ├── Grid.tsx
│   │   │   └── ViewportControls.tsx
│   │   ├── inspector/          # Inspector components
│   │   │   ├── ComponentEditor.tsx
│   │   │   ├── PropertyField.tsx
│   │   │   └── ComponentList.tsx
│   │   ├── assets/             # Asset browser components
│   │   │   ├── AssetGrid.tsx
│   │   │   ├── AssetTree.tsx
│   │   │   ├── AssetPreview.tsx
│   │   │   └── AssetImporter.tsx
│   │   └── common/             # Common UI components
│   │       ├── Button.tsx
│   │       ├── Input.tsx
│   │       ├── Dropdown.tsx
│   │       ├── Modal.tsx
│   │       └── ContextMenu.tsx
│   ├── store/                  # State management
│   │   ├── index.ts            # Store configuration
│   │   ├── slices/             # Redux slices
│   │   │   ├── editorSlice.ts
│   │   │   ├── sceneSlice.ts
│   │   │   ├── selectionSlice.ts
│   │   │   ├── assetSlice.ts
│   │   │   └── preferencesSlice.ts
│   │   └── middleware/         # Custom middleware
│   ├── services/               # Service layer
│   │   ├── EngineService.ts    # Engine integration
│   │   ├── ProjectService.ts   # Project operations
│   │   ├── AssetService.ts     # Asset operations
│   │   ├── APIService.ts       # Server communication
│   │   └── WebSocketService.ts # Real-time updates
│   ├── tools/                  # Editor tools
│   │   ├── ToolManager.ts      # Tool management
│   │   ├── SelectTool.ts       # Selection tool
│   │   ├── TranslateTool.ts    # Move tool
│   │   ├── RotateTool.ts       # Rotate tool
│   │   ├── ScaleTool.ts        # Scale tool
│   │   └── CustomTool.ts       # Custom tool base
│   ├── commands/               # Command pattern (undo/redo)
│   │   ├── CommandManager.ts   # Command stack
│   │   ├── Command.ts          # Command interface
│   │   └── commands/           # Specific commands
│   │       ├── TransformCommand.ts
│   │       ├── CreateEntityCommand.ts
│   │       ├── DeleteEntityCommand.ts
│   │       └── ModifyComponentCommand.ts
│   ├── hooks/                  # Custom React hooks
│   │   ├── useEngine.ts
│   │   ├── useSelection.ts
│   │   ├── useAssets.ts
│   │   └── useUndo.ts
│   ├── utils/                  # Utility functions
│   │   ├── EditorUtils.ts
│   │   ├── SerializationUtils.ts
│   │   └── ValidationUtils.ts
│   ├── styles/                 # Global styles
│   │   ├── globals.css
│   │   ├── themes.css
│   │   └── variables.css
│   └── types/                  # TypeScript types
│       ├── editor.types.ts
│       ├── asset.types.ts
│       └── project.types.ts
├── tests/                      # Component tests
├── tsconfig.json
├── package.json
├── vite.config.ts              # Vite configuration
└── README.md
```

---

## Package: Server (Local Server)

**Location:** `packages/server/`  
**Purpose:** Backend services and API

```
packages/server/
├── src/
│   ├── index.ts                # Server entry point
│   ├── app.ts                  # Express/Fastify app
│   ├── config/                 # Configuration
│   │   ├── database.ts
│   │   ├── server.ts
│   │   └── storage.ts
│   ├── routes/                 # API routes
│   │   ├── index.ts
│   │   ├── projects.ts         # Project endpoints
│   │   ├── assets.ts           # Asset endpoints
│   │   ├── build.ts            # Build endpoints
│   │   └── auth.ts             # Auth endpoints
│   ├── controllers/            # Route controllers
│   │   ├── ProjectController.ts
│   │   ├── AssetController.ts
│   │   ├── BuildController.ts
│   │   └── AuthController.ts
│   ├── services/               # Business logic
│   │   ├── ProjectService.ts
│   │   ├── AssetService.ts
│   │   ├── AssetProcessor.ts   # Asset processing
│   │   ├── BuildService.ts
│   │   └── StorageService.ts
│   ├── models/                 # Database models
│   │   ├── Project.ts
│   │   ├── Asset.ts
│   │   ├── Scene.ts
│   │   └── User.ts
│   ├── middleware/             # Express middleware
│   │   ├── auth.ts
│   │   ├── validation.ts
│   │   ├── errorHandler.ts
│   │   └── rateLimit.ts
│   ├── processors/             # Asset processors
│   │   ├── ImageProcessor.ts
│   │   ├── ModelProcessor.ts
│   │   ├── AudioProcessor.ts
│   │   └── VideoProcessor.ts
│   ├── builders/               # Build system
│   │   ├── WebBuilder.ts
│   │   ├── DesktopBuilder.ts
│   │   └── BuildManager.ts
│   ├── websocket/              # WebSocket server
│   │   ├── WebSocketServer.ts
│   │   ├── handlers/
│   │   │   ├── ProjectHandler.ts
│   │   │   ├── SceneHandler.ts
│   │   │   └── CollabHandler.ts
│   │   └── events.ts
│   ├── database/               # Database layer
│   │   ├── connection.ts
│   │   ├── migrations/         # Database migrations
│   │   └── seeds/              # Database seeds
│   ├── storage/                # File storage
│   │   ├── FileStorage.ts
│   │   ├── CacheStorage.ts
│   │   └── BackupStorage.ts
│   └── utils/                  # Server utilities
│       ├── Logger.ts
│       ├── Validator.ts
│       └── Security.ts
├── tests/                      # Server tests
├── tsconfig.json
├── package.json
└── README.md
```

---

## Package: Shared (Shared Code)

**Location:** `packages/shared/`  
**Purpose:** Code shared between packages

```
packages/shared/
├── src/
│   ├── types/                  # Shared TypeScript types
│   │   ├── common.types.ts
│   │   ├── asset.types.ts
│   │   ├── scene.types.ts
│   │   └── project.types.ts
│   ├── constants/              # Shared constants
│   │   ├── fileTypes.ts
│   │   ├── componentTypes.ts
│   │   └── systemTypes.ts
│   ├── utils/                  # Shared utilities
│   │   ├── uuid.ts
│   │   ├── validation.ts
│   │   └── serialization.ts
│   ├── schemas/                # Validation schemas
│   │   ├── project.schema.ts
│   │   ├── asset.schema.ts
│   │   └── scene.schema.ts
│   └── interfaces/             # Shared interfaces
│       ├── IAsset.ts
│       ├── IProject.ts
│       └── IComponent.ts
├── tests/
├── tsconfig.json
├── package.json
└── README.md
```

---

## Documentation Structure

**Location:** `docs/`

```
docs/
├── index.md                    # Documentation home
├── getting-started/            # Getting started guides
│   ├── installation.md
│   ├── quick-start.md
│   └── first-project.md
├── user-guide/                 # User documentation
│   ├── editor-interface.md
│   ├── scene-editing.md
│   ├── asset-management.md
│   ├── scripting.md
│   ├── animation.md
│   ├── physics.md
│   └── building-games.md
├── tutorials/                  # Step-by-step tutorials
│   ├── basic-scene.md
│   ├── platformer-game.md
│   ├── fps-game.md
│   └── multiplayer-game.md
├── api/                        # API documentation
│   ├── engine/                 # Engine API
│   ├── editor/                 # Editor API
│   └── server/                 # Server API
├── architecture/               # Architecture docs
│   ├── overview.md
│   ├── rendering.md
│   ├── ecs.md
│   ├── physics.md
│   └── networking.md
├── contributing/               # Contribution guides
│   ├── code-style.md
│   ├── testing.md
│   ├── pull-requests.md
│   └── documentation.md
└── faq.md                      # FAQ
```

---

## Build Artifacts

Build outputs are not committed to version control.

```
dist/                           # Build output (gitignored)
├── engine/                     # Engine bundle
├── editor/                     # Editor bundle
├── server/                     # Server bundle
└── docs/                       # Documentation site

node_modules/                   # Dependencies (gitignored)
```

---

## Configuration Files

### Root Configuration

- **package.json** - Root package and workspace config
- **tsconfig.json** - Base TypeScript configuration
- **lerna.json** or **pnpm-workspace.yaml** - Monorepo config
- **.eslintrc.js** - ESLint rules
- **.prettierrc** - Code formatting rules
- **.gitignore** - Git ignore patterns
- **.editorconfig** - Editor configuration

### Package-Specific

Each package has its own:

- **package.json** - Package dependencies and scripts
- **tsconfig.json** - TypeScript configuration (extends root)
- **vite.config.ts** or **webpack.config.js** - Build config

---

## Scripts Location

**Location:** `scripts/`

```
scripts/
├── setup.sh                    # Initial setup script
├── build-all.sh                # Build all packages
├── test-all.sh                 # Run all tests
├── clean.sh                    # Clean build artifacts
├── generate-docs.sh            # Generate documentation
└── release.sh                  # Release script
```

---

## Git Workflow

### Branch Structure

```
main                            # Production-ready code
├── develop                     # Development branch
│   ├── feature/*               # Feature branches
│   ├── bugfix/*                # Bug fix branches
│   ├── hotfix/*                # Hotfix branches
│   └── release/*               # Release branches
```

### Commit Convention

Follow Conventional Commits:

- `feat:` New features
- `fix:` Bug fixes
- `docs:` Documentation
- `style:` Code style
- `refactor:` Code refactoring
- `test:` Tests
- `chore:` Maintenance

---

## Best Practices

### File Organization

1. Group related files in directories
2. Keep files focused and single-purpose
3. Use index files for public API exports
4. Separate concerns (logic, UI, data)

### Naming Conventions

- **Files:** PascalCase for components/classes, camelCase for utilities
- **Directories:** kebab-case
- **Components:** PascalCase
- **Variables/Functions:** camelCase
- **Constants:** UPPER_SNAKE_CASE
- **Types/Interfaces:** PascalCase with I prefix for interfaces

### Module Structure

```typescript
// Public API exports
export * from './publicModule';

// Internal (not exported)
import { privateModule } from './private';
```

---

## Future Additions

As the project grows, consider adding:

- `packages/plugins/` - Plugin system
- `packages/cli/` - Command-line tools
- `packages/mobile/` - Mobile editor support
- `benchmarks/` - Performance benchmarks
- `docs/rfcs/` - Request for Comments

---

_Document Version: 1.0_  
_Last Updated: 2025-11-16_  
_Status: Initial Structure_
