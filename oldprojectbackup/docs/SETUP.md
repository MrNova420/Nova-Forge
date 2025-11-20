# Nova Engine - Easy Setup Guide

> **Simple, step-by-step instructions to get Nova Engine running**

---

## 🚀 Quick Start (5 Minutes)

### Prerequisites

You only need:

- **Node.js** 18.0.0 or higher ([Download here](https://nodejs.org/))
- **Git** (optional, for cloning)

That's it! No complex setup required.

### Step 1: Get the Code

**Option A: Clone from GitHub**

```bash
git clone https://github.com/MrNova420/Nova-Engine-editor.git
cd Nova-Engine-editor
```

**Option B: Download ZIP**

1. Download ZIP from GitHub
2. Extract to a folder
3. Open terminal in that folder

### Step 2: Install Everything (One Command!)

```bash
npm install
```

This automatically installs ALL dependencies for ALL packages. Just wait 10-30 seconds.

### Step 3: Build Everything

```bash
npm run build
```

Builds all packages in the correct order. Takes about 15 seconds.

### Step 4: Run Tests (Optional)

```bash
npm test
```

Runs all 198 tests to verify everything works. Takes about 3 seconds.

### Step 5: Start Developing!

```bash
# Start the editor
npm run dev:editor

# Or start the engine dev server
npm run dev:engine
```

**That's it!** You're ready to build games with Nova Engine! 🎮

---

## 📦 What Gets Installed

All dependencies are automatically installed with `npm install`:

### Root Dependencies

- TypeScript
- Jest (testing)
- ESLint (code quality)
- Prettier (formatting)
- Vite (build tool)
- Husky (git hooks)

### Engine Dependencies

- ammojs-typed (physics)

### Editor Dependencies

- React
- Redux Toolkit
- Material-UI

### Server Dependencies

- Node.js built-ins (no external deps needed)

**Everything is pre-configured and ready to go!**

---

## 🏗️ Project Structure

```
Nova-Engine-editor/
├── packages/
│   ├── engine/      # Game engine (TypeScript + WebGL)
│   ├── editor/      # Visual editor (React)
│   ├── server/      # Build server (Node.js)
│   └── shared/      # Shared utilities
├── examples/        # Example projects
├── docs/            # Documentation
└── scripts/         # Utility scripts
```

---

## 🎯 Common Tasks

### Building

```bash
# Build everything
npm run build

# Build specific package
npm run build:engine
npm run build:editor
npm run build:server
```

### Testing

```bash
# Run all tests
npm test

# Run specific package tests
npm run test:engine
npm run test:server
```

### Development

```bash
# Start editor in dev mode
npm run dev:editor

# Start engine dev server
npm run dev:engine

# Watch mode (auto-rebuild)
npm run watch
```

### Linting & Formatting

```bash
# Lint code
npm run lint

# Fix lint issues
npm run lint:fix

# Format code
npm run format
```

---

## 🐛 Troubleshooting

### "npm install" fails

**Solution:** Make sure you have Node.js 18+ installed

```bash
node --version  # Should be 18.0.0 or higher
```

### "Cannot find module" errors

**Solution:** Install dependencies

```bash
npm install
```

### Build fails

**Solution:** Clean and rebuild

```bash
rm -rf node_modules packages/*/node_modules
npm install
npm run build
```

### Tests fail

**Solution:** Make sure you built first

```bash
npm run build
npm test
```

### Port already in use

**Solution:** Kill the process or use different port

```bash
# Kill process on port 3000
lsof -ti:3000 | xargs kill -9

# Or use different port
PORT=3001 npm run dev:editor
```

---

## 📚 Next Steps

After setup, check out:

1. **[USER_GUIDE.md](./USER_GUIDE.md)** - Learn how to use Nova Engine
2. **[Examples](./examples/)** - See working code samples
3. **[API Documentation](./docs/api/)** - Complete API reference
4. **[Tutorials](./docs/tutorials/)** - Step-by-step guides

---

## 🆘 Need Help?

### Documentation

- [User Guide](./USER_GUIDE.md) - Complete usage guide
- [API Docs](./docs/api/) - API reference
- [Development Guide](./DEVELOPMENT_GUIDE.md) - For contributors

### Support

- **GitHub Issues:** [Report bugs](https://github.com/MrNova420/Nova-Engine-editor/issues)
- **Discussions:** [Ask questions](https://github.com/MrNova420/Nova-Engine-editor/discussions)

---

## ✅ Verification

To verify everything is working:

```bash
# 1. Dependencies installed?
npm list --depth=0

# 2. Build successful?
npm run build

# 3. Tests passing?
npm test

# 4. Editor starts?
npm run dev:editor
```

If all four work, you're good to go! 🎉

---

## 🚀 Pro Tips

### Fast Development

```bash
# Use watch mode for auto-rebuild
npm run watch
```

### Clean Build

```bash
# Start fresh if issues occur
rm -rf dist packages/*/dist
npm run build
```

### Check Everything

```bash
# Run full verification
npm run build && npm test && npm run lint
```

---

**Nova Engine is designed to be easy!** If something seems complicated, it's probably a bug. Please report it! 🐛

---

_Last Updated: 2025-11-16_  
_Status: Production Ready_  
_Difficulty: Easy_ ✨
