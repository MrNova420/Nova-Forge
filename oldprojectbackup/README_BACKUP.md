# Old Project Backup

This folder contains the complete backup of the Nova Engine project as it existed before the project direction change on November 20, 2025.

## What's Included

This backup contains:
- All source code (~97K LOC production code)
- 11 packages (engine, editor, hub, launcher variants, etc.)
- All documentation (70+ markdown files)
- Configuration files
- Scripts and build tools
- Tests (215+ passing tests)
- Examples and benchmarks

## Original Project Structure

The original project was a **4-pillar game development ecosystem**:

1. **Nova Runtime** - Core game engine (39K LOC, TypeScript/WebGL2/Ammo.js)
2. **Nova Editor** - Visual creation tools (4K LOC)
3. **Nova Hub** - Platform server for builds, hosting, distribution (10K LOC)
4. **Nova Launcher** - Player platform (Web + Desktop + Mobile)

## Status at Time of Backup

- ✅ Phase 0 complete (build system, security, organization)
- 🔄 Phase 1 17% complete (platform abstraction, job system, memory mgmt)
- ✅ 215+ tests passing
- ✅ Modern TypeScript/React/Vite stack functional
- ✅ Complete ECS, renderer, physics, audio, asset pipeline working

## Key Documentation Files

- `AUTONOMOUS_DEVELOPMENT_GUIDE2.md` - Comprehensive 25-phase development plan
- `ECOSYSTEM_VISION.md` - Complete 4-pillar platform vision
- `QUALITY_OF_LIFE_FEATURES.md` - QoL features catalog
- `NOVA_ENGINE_AAA_UNIVERSAL_SPEC.md` - Technical specifications
- `ROADMAP.md` - Original development roadmap
- `PROGRESS.md` - Status tracker

## Reason for Backup

The project is being restarted with a new direction:
- Focus on building a custom game engine FIRST
- Building for PC/Console platforms (not universal mobile-first)
- Creating a complete game builder/editor platform
- Fresh start with proper planning

## Important Note

**This backup is preserved for reference only.** The new project will be developed separately with a different architecture and goals. This backup may be moved to a separate repository in the future.

---

*Backup created: November 20, 2025*
