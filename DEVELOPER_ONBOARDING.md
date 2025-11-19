# Nova Engine - Developer Onboarding Guide

Welcome to the Nova Engine development team! This guide will help you get up to speed quickly.

**Version**: 0.1.0  
**Last Updated**: 2025-11-19

---

## Table of Contents

1. [Quick Start (5 minutes)](#quick-start)
2. [Understanding the Codebase](#understanding-the-codebase)
3. [Development Environment](#development-environment)
4. [Key Concepts](#key-concepts)
5. [Common Tasks](#common-tasks)
6. [Debugging](#debugging)
7. [Resources](#resources)

---

## Quick Start

### 1. Clone and Install (5 minutes)

```bash
# Clone repository
git clone https://github.com/MrNova420/Nova-Engine.git
cd Nova-Engine

# Install dependencies
npm install

# Build all packages
npm run build

# Run tests to verify
npm test
```

**Expected Output**:

- ✅ All 11 packages building cleanly
- ✅ 215/215 tests passing
- ✅ Zero TypeScript errors

### 2. Start Development Server

```bash
# Start unified platform (all-in-one)
npm run dev -w @nova-engine/unified-platform

# Or start individual packages
npm run dev -w @nova-engine/editor      # Visual editor
npm run dev -w @nova-engine/server      # Backend server
npm run dev -w @nova-engine/launcher-web # Web launcher
```

### 3. Verify Your Setup

Visit:

- Editor: http://localhost:3000
- Server: http://localhost:3001
- Unified Platform: http://localhost:3002

---

## Understanding the Codebase

### Architecture Overview

Nova Engine uses a **monorepo structure** with multiple packages:

```
Nova-Engine/
├── packages/
│   ├── editor/              # Visual game editor (React + Redux)
│   ├── editor-web/          # Web-specific editor features
│   ├── engine/              # Core game engine (TypeScript + Three.js + Ammo.js)
│   ├── engine-web/          # Web platform abstractions
│   ├── hub/                 # Multiplayer hub & matchmaking
│   ├── launcher-desktop/    # Desktop launcher (Tauri)
│   ├── launcher-mobile/     # Mobile launcher (React Native)
│   ├── launcher-web/        # Web launcher
│   ├── server/              # HTTP server & auth
│   ├── shared/              # Shared utilities
│   └── unified-platform/    # All-in-one platform
├── scripts/                 # Build & automation scripts
└── docs/                    # Documentation
```

### Tech Stack

**Frontend**:

- React 18+ (UI)
- Redux Toolkit (State management)
- Radix UI (Component library)
- Tailwind CSS (Styling)
- Vite (Build tool)

**Backend**:

- Node.js + Express (HTTP server)
- PostgreSQL (Database)
- Redis (Caching, optional)
- WebSocket (Real-time communication)

**Game Engine**:

- Three.js (3D rendering)
- Ammo.js (Physics - Bullet port)
- Web Audio API (Sound)
- WebGL 2.0 (Graphics)

**Desktop**:

- Tauri (Rust + WebView)

**Mobile**:

- React Native

**Testing**:

- Jest (Unit tests)
- Testing Library (React tests)

**Code Quality**:

- TypeScript (Type safety)
- ESLint (Linting)
- Prettier (Formatting)
- Husky (Git hooks)

---

## Development Environment

### Recommended VS Code Extensions

```json
{
  "recommendations": [
    "dbaeumer.vscode-eslint",
    "esbenp.prettier-vscode",
    "eamodio.gitlens",
    "ms-vscode.vscode-typescript-next",
    "bradlc.vscode-tailwindcss",
    "usernamehw.errorlens",
    "streetsidesoftware.code-spell-checker"
  ]
}
```

### VS Code Settings

Create `.vscode/settings.json`:

```json
{
  "editor.formatOnSave": true,
  "editor.defaultFormatter": "esbenp.prettier-vscode",
  "editor.codeActionsOnSave": {
    "source.fixAll.eslint": true
  },
  "typescript.tsdk": "node_modules/typescript/lib",
  "typescript.enablePromptUseWorkspaceTsdk": true
}
```

### Environment Configuration

```bash
# Copy environment template
cp .env.example .env

# Edit with your values (for development, defaults are usually fine)
nano .env
```

**Minimal Development .env**:

```bash
NODE_ENV=development
PORT=3001
DATABASE_URL=postgresql://localhost:5432/nova_engine_dev
JWT_SECRET=dev-secret-change-in-production-min-32-chars
SESSION_SECRET=dev-session-secret-change-in-production
```

---

## Key Concepts

### 1. Monorepo Workspaces

We use npm workspaces. Each package is independent but can reference others:

```typescript
// In any package
import { Engine } from '@nova-engine/engine';
import { Entity } from '@nova-engine/shared';
```

**Running commands in specific packages**:

```bash
npm run build -w @nova-engine/editor    # Build just editor
npm test -w @nova-engine/engine        # Test just engine
```

### 2. Entity Component System (ECS)

The engine uses ECS architecture:

```typescript
// Create an entity
const player = world.createEntity();

// Add components
player.addComponent(new TransformComponent());
player.addComponent(new MeshRenderer());
player.addComponent(new PlayerController());

// Systems process entities with specific components
class MovementSystem extends System {
  update(entities: Entity[], deltaTime: number) {
    entities
      .filter((e) => e.has(TransformComponent) && e.has(Velocity))
      .forEach((entity) => {
        // Update position based on velocity
      });
  }
}
```

### 3. Asset Pipeline

Assets flow through this pipeline:

1. **Import** → Raw files (GLTF, PNG, MP3)
2. **Process** → Optimized for target platform
3. **Cache** → Stored in AssetCache
4. **Load** → Loaded on demand
5. **Render** → Used in scene

```typescript
const assetManager = new AssetManager();
const texture = await assetManager.load('texture.png', TextureAssetLoader);
```

### 4. State Management (Editor)

Editor uses Redux Toolkit with slices:

```typescript
// Reading state
const entities = useAppSelector((state) => state.scene.entities);

// Updating state
dispatch(
  addEntity({
    id: 'player-1',
    name: 'Player',
    components: {},
  })
);
```

### 5. Type Safety

**Always use proper types**:

```typescript
// ❌ Bad
const data: any = fetchData();

// ✅ Good
interface UserData {
  id: string;
  name: string;
}
const data: UserData = await fetchData();
```

---

## Common Tasks

### Adding a New Feature

1. **Create feature branch**:

   ```bash
   git checkout -b feature/my-feature
   ```

2. **Write code with types**:

   ```typescript
   // Add proper TypeScript types
   interface MyFeatureProps {
     title: string;
     onClose: () => void;
   }

   export const MyFeature: React.FC<MyFeatureProps> = ({ title, onClose }) => {
     // Implementation
   };
   ```

3. **Add tests**:

   ```typescript
   describe('MyFeature', () => {
     it('should render correctly', () => {
       const { getByText } = render(<MyFeature title="Test" onClose={() => {}} />);
       expect(getByText('Test')).toBeInTheDocument();
     });
   });
   ```

4. **Run checks**:

   ```bash
   npm run lint          # Check code quality
   npm run typecheck     # Check types
   npm test              # Run tests
   npm run build         # Ensure it builds
   ```

5. **Commit following convention**:
   ```bash
   git add .
   git commit -m "feat: add my amazing feature"
   ```

### Adding a New Component (Editor)

```typescript
// packages/editor/src/components/MyComponent.tsx
/**
 * NOVA ENGINE - Proprietary Software
 * ... (license header)
 */

import React from 'react';

interface MyComponentProps {
  title: string;
}

export const MyComponent: React.FC<MyComponentProps> = ({ title }) => {
  return (
    <div className="p-4 bg-editor-bg border border-editor-border rounded">
      <h2 className="text-editor-text font-bold">{title}</h2>
    </div>
  );
};
```

### Adding a New Engine System

```typescript
// packages/engine/src/systems/MySystem.ts
import { System } from '../ecs/System';
import { Entity } from '../ecs/Entity';

export class MySystem extends System {
  update(entities: Entity[], deltaTime: number): void {
    // Process entities that have required components
    const validEntities = entities.filter(
      (e) => e.hasComponent('Transform') && e.hasComponent('MyComponent')
    );

    validEntities.forEach((entity) => {
      // Update logic
    });
  }
}
```

### Adding API Endpoint

```typescript
// packages/server/src/routes/myRoute.ts
import { Router } from 'express';
import { validateBody, authMiddleware } from '../middleware';

const router = Router();

router.post(
  '/api/my-endpoint',
  authMiddleware, // Require authentication
  validateBody(mySchema), // Validate input
  async (req, res) => {
    try {
      const result = await myService.process(req.body);
      res.json({ success: true, data: result });
    } catch (error) {
      res.status(500).json({ success: false, error: error.message });
    }
  }
);

export default router;
```

---

## Debugging

### VS Code Launch Configuration

Create `.vscode/launch.json`:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Debug Editor",
      "type": "chrome",
      "request": "launch",
      "url": "http://localhost:3000",
      "webRoot": "${workspaceFolder}/packages/editor"
    },
    {
      "name": "Debug Server",
      "type": "node",
      "request": "launch",
      "program": "${workspaceFolder}/packages/server/src/index.ts",
      "preLaunchTask": "npm: build -w @nova-engine/server",
      "outFiles": ["${workspaceFolder}/packages/server/dist/**/*.js"]
    },
    {
      "name": "Debug Tests",
      "type": "node",
      "request": "launch",
      "program": "${workspaceFolder}/node_modules/.bin/jest",
      "args": ["--runInBand", "--no-cache"],
      "console": "integratedTerminal"
    }
  ]
}
```

### Common Debugging Commands

```bash
# Run single test file
npm test -- MyComponent.test.ts

# Run tests in watch mode
npm test -- --watch

# Debug specific package
npm run dev -w @nova-engine/editor

# Check TypeScript errors
npm run typecheck

# Verbose build
npm run build -- --verbose
```

### Logging

```typescript
// Use proper logging (not console.log in production code)
import { logger } from '@nova-engine/shared';

logger.info('User logged in', { userId: user.id });
logger.warn('Deprecated API used');
logger.error('Failed to load asset', { assetId, error });
```

---

## Resources

### Documentation

- **README.md** - Project overview
- **AUTONOMOUS_DEVELOPMENT_GUIDE2.md** - Long-term development roadmap
- **SECURITY_AUDIT.md** - Security guidelines
- **CODE_QUALITY_REPORT.md** - Code quality standards
- **DEPLOYMENT.md** - Deployment instructions
- **CONTRIBUTING.md** - Contribution guidelines

### Key Files to Understand

1. **package.json** (root) - Workspace configuration
2. **tsconfig.json** - TypeScript configuration
3. **jest.config.cjs** - Test configuration
4. **.eslintrc.json** - Linting rules
5. **.prettierrc.json** - Formatting rules

### Architecture Patterns

**Redux Slices** (Editor State):

- `packages/editor/src/store/slices/sceneSlice.ts`
- `packages/editor/src/store/slices/editorSlice.ts`

**ECS System** (Engine):

- `packages/engine/src/ecs/System.ts`
- `packages/engine/src/ecs/Entity.ts`
- `packages/engine/src/ecs/Component.ts`

**Asset Loaders**:

- `packages/engine/src/assets/loaders/GLTFLoader.ts`
- `packages/engine/src/assets/loaders/TextureAssetLoader.ts`

**Security Middleware**:

- `packages/server/src/middleware/rateLimiter.ts`
- `packages/server/src/middleware/validation.ts`
- `packages/server/src/middleware/securityHeaders.ts`

### Getting Help

1. **Check Documentation** - Most answers are in docs
2. **Read Code** - Code is well-commented with TSDoc
3. **Run Tests** - Tests show usage examples
4. **GitHub Issues** - Search existing issues
5. **Discord/Slack** - Ask the team (if available)

---

## Checklist for Your First Contribution

- [ ] Repository cloned and built successfully
- [ ] All tests passing locally
- [ ] VS Code configured with recommended extensions
- [ ] Read CONTRIBUTING.md and understood commit conventions
- [ ] Familiar with monorepo structure
- [ ] Know how to run individual packages
- [ ] Understand ECS architecture basics
- [ ] Can add a simple component/feature
- [ ] Can write and run tests
- [ ] Ready to make first PR!

---

## Quick Reference

### npm Scripts

```bash
npm install              # Install dependencies
npm run build            # Build all packages
npm test                 # Run all tests
npm run lint             # Lint all packages
npm run format           # Format code with Prettier
npm run typecheck        # Check TypeScript types
npm run dev              # Start dev server (varies by package)
npm run clean            # Clean build artifacts
```

### File Naming Conventions

- Components: `PascalCase.tsx`
- Utilities: `camelCase.ts`
- Tests: `*.test.ts` or `*.spec.ts`
- Types: `types.ts` or inline with implementation
- Constants: `SCREAMING_SNAKE_CASE`

### Import Order

1. External libraries
2. Internal packages (`@nova-engine/*`)
3. Relative imports
4. Types (last)

```typescript
import React from 'react';
import { Engine } from '@nova-engine/engine';
import { MyComponent } from './MyComponent';
import type { MyType } from './types';
```

---

**Welcome aboard! Let's build the world's best game engine together! 🚀**

**Last Updated**: 2025-11-19  
**Questions?** Open an issue or ask the team!
