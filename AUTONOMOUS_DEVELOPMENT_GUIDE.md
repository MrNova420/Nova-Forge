# 🚀 NovaForge A Single All-In-One App Autonomous Development Master Guide

> **Platform**: NovaForge | **Engine**: NovaCore | **Company**: WeNova Interactive (operating as Kayden Shawn Massengill)  
> **Document Version**: 1.0 | **Last Updated**: 2025-11-25  
> **Purpose**: **THE SINGLE REFERENCE DOCUMENT** for starting and continuing full autonomous development

---

## ⚠️ THIS IS YOUR STARTING POINT ⚠️

> **Before ANY development work, READ THIS ENTIRE DOCUMENT.**  
> This document is the single source of truth for autonomous development.  
> Reference this document at the START of every development session.

---

## 📚 DOCUMENT HIERARCHY - WHAT TO READ AND WHEN

### Master Documents (Read in Order)

| Priority | Document | Purpose | When to Read |
|----------|----------|---------|--------------|
| **1** | **THIS FILE** (`AUTONOMOUS_DEVELOPMENT_GUIDE.md`) | Master reference for autonomous development | FIRST - Every session |
| **2** | `docs/PROGRESS.md` | Current development state & what to do next | SECOND - Every session |
| **3** | `NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md` | Complete technical specifications | During implementation |
| **4** | `docs/NOVACORE_COMPLETE_DOCUMENTATION.md` | Detailed engine documentation | Reference during coding |
| **5** | `docs/EDITOR_AND_GAME_BUILDER_GUIDE.md` | Editor/tool specifications | When building editor |
| **6** | `docs/SELF_LEARNING_AND_TRAINING_GUIDE.md` | AI/ML training systems | When building learning systems |
| **7** | `docs/TECHNICAL_README.md` | Technical details | Reference as needed |

---

## 🔴 CRITICAL: ENGINE-FIRST DEVELOPMENT MANDATE 🔴

### The #1 Rule - NEVER Forget This

> **The NovaCore game engine MUST be fully developed and functional BEFORE building the complete NovaForge platform.**

### Development Priority Order (STRICT)

```
PRIORITY 1 (HIGHEST): NovaCore Engine Development
├── Core Systems (memory, math, logging)
├── Entity-Component System
├── Rendering Pipeline (Vulkan/Metal)
├── Physics Engine
├── Scripting System
├── Audio System
├── Input System
├── Mobile Editor
└── ALL engine features must be COMPLETE before platform

PRIORITY 2 (MEDIUM): Basic Platform (for engine testing/distribution)
├── Minimal game library
├── Basic user accounts
├── Simple project export

PRIORITY 3 (AFTER ENGINE COMPLETE): Full Platform Features
├── Complete game library
├── Publishing workflow
├── Asset store
├── Payment processing
├── Analytics dashboard
├── Cloud services
└── All platform features built AFTER engine is stable
```

### Timeline Summary

| Phase | Focus | Duration |
|-------|-------|----------|
| **Month 1-2** | **ENGINE ONLY** - Core engine development | 8 weeks |
| **Month 3** | Engine completion + Basic Platform | 4 weeks |
| **Post-Launch** | Full Platform Features | Ongoing |

---

## 📋 SESSION START CHECKLIST

### EVERY Session Must Begin With:

```
□ 1. Read this document (AUTONOMOUS_DEVELOPMENT_GUIDE.md)
□ 2. Read PROGRESS.md - understand current state
□ 3. Check current phase and what's completed
□ 4. Review what needs to be done next
□ 5. Run all existing tests (must pass)
□ 6. Begin work on next task
```

### EVERY Session Must End With:

```
□ 1. Update PROGRESS.md with what was done
□ 2. Include code snippets and file paths
□ 3. Include test results and performance metrics
□ 4. List known issues discovered
□ 5. Specify exact next steps
□ 6. Commit with descriptive message
```

---

## 🚨 CORE COMMANDMENTS - LAWS, NOT SUGGESTIONS

### Golden Rules (NEVER Violate)

1. **NO LAZINESS - FULL DEPTH ALWAYS**
   - Never skip implementation details
   - Never use placeholders like "TODO" or "implement later"
   - Every feature must be fully implemented, tested, and documented
   - If something takes 1000 lines, write 1000 lines - no shortcuts

2. **PROGRESS TRACKING IS MANDATORY**
   - Update `docs/PROGRESS.md` after EVERY significant change
   - Document what was done, what works, what's next
   - Include code samples, benchmarks, test results
   - Never commit without updating progress tracking

3. **CUSTOM GROUND-UP IMPLEMENTATION**
   - Build everything from scratch - no copying from Unreal/Unity/Godot
   - Write custom Vulkan/Metal renderers (not wgpu wrapper)
   - Write custom physics engine (inspired by Jolt, but custom implementation)
   - Write custom ECS architecture (Neural-Symbolic ECW is unique)
   - Only use libraries for low-level APIs (Vulkan, Metal, SDL) and integration (Wwise)

4. **QUALITY OVER SPEED**
   - AAA production-grade code only
   - Every system must be optimized for mobile (ARM NEON, cache-friendly)
   - Profile everything - Tracy profiler integration required
   - Zero tolerance for performance regressions

5. **DOCUMENTATION MUST BE COMPLETE**
   - Every function, class, system must have comprehensive documentation
   - Architecture decisions must be explained
   - Performance characteristics must be documented
   - Include code examples for all public APIs

6. **TESTING IS NON-NEGOTIABLE**
   - Unit tests for all core systems
   - Integration tests for rendering/physics/ECS
   - Performance benchmarks for every optimization
   - Device testing on low/mid/high-end hardware
   - Document test results in PROGRESS.md

7. **PHASE COMPLETION CRITERIA**
   - A phase is NOT complete until all deliverables are done
   - All tests must pass
   - Performance targets must be met
   - Documentation must be complete
   - PROGRESS.md must be updated with detailed summary

---

## ❌ FORBIDDEN ACTIONS (NEVER DO THESE)

- ❌ **NEVER** use placeholder implementations
- ❌ **NEVER** skip testing
- ❌ **NEVER** ignore performance targets
- ❌ **NEVER** copy code from other engines
- ❌ **NEVER** commit without updating PROGRESS.md
- ❌ **NEVER** proceed to next phase with incomplete previous phase
- ❌ **NEVER** sacrifice code quality for speed
- ❌ **NEVER** skip documentation
- ❌ **NEVER** ignore mobile optimization requirements
- ❌ **NEVER** break existing functionality
- ❌ **NEVER** build platform features before engine is complete

---

## ✅ REQUIRED ACTIONS (ALWAYS DO THESE)

- ✅ **ALWAYS** build from scratch (custom implementation)
- ✅ **ALWAYS** write comprehensive tests
- ✅ **ALWAYS** profile and optimize
- ✅ **ALWAYS** update PROGRESS.md
- ✅ **ALWAYS** follow mobile-first principles
- ✅ **ALWAYS** validate on target hardware
- ✅ **ALWAYS** document everything
- ✅ **ALWAYS** meet performance targets
- ✅ **ALWAYS** complete phases 100% before moving on
- ✅ **ALWAYS** maintain AAA production quality
- ✅ **ALWAYS** complete engine before platform

---

## 🎯 AUTONOMOUS DEVELOPMENT WORKFLOW

### Step 1: Read Current State (PROGRESS.md)

```markdown
WHAT TO CHECK IN PROGRESS.md:
- Current phase
- What's completed
- What's in progress
- Known issues
- Next tasks
- Performance metrics
```

### Step 2: Implement Current Phase Deliverables

```markdown
HOW TO IMPLEMENT:
- Follow the blueprint specifications EXACTLY
- Build from scratch (no copying existing engines)
- Write comprehensive tests as you go
- Profile and optimize continuously
- Document as you code
```

### Step 3: Validate Implementation

```markdown
VALIDATION CHECKLIST:
□ Run all unit tests
□ Run integration tests
□ Run performance benchmarks
□ Test on target devices (emulators minimum)
□ Verify all acceptance criteria are met
□ Check for memory leaks
□ Check for performance regressions
```

### Step 4: Update PROGRESS.md

```markdown
PROGRESS.md UPDATE TEMPLATE:

---
## Progress Update - [DATE TIME]

**Phase**: Phase X: [Name] - [XX%] Complete

**Session Goal**: [What you planned to accomplish]

**What Was Built**:
- [Detailed description]
- Files: `path/to/file.cpp`, `path/to/file.h`
- Lines Added: XXX

**Architecture Decisions**:
- Decision: [What you decided]
- Rationale: [Why this approach]

**Performance Metrics**:
- Low-end: XX FPS, XXX MB RAM
- Mid-range: XX FPS, XXX MB RAM
- High-end: XX FPS, XXX MB RAM

**Test Results**:
- Unit tests: XX/XX passing
- Integration tests: XX/XX passing

**Known Issues**:
- [Bug/issue description]

**Next Steps**:
- [ ] [Next task 1]
- [ ] [Next task 2]

---
```

### Step 5: Commit and Continue

```markdown
COMMIT PROCESS:
1. Ensure all changes are tested
2. Update PROGRESS.md
3. Write descriptive commit message
4. Move to next task only when current is 100% done
```

---

## 📅 DEVELOPMENT ROADMAP SUMMARY

### Month 1: ENGINE Foundation & Core

**Week 1: Project Setup**
- Git repository structure
- CMake build system
- Android NDK toolchain
- CI/CD pipeline

**Week 2: Core Systems**
- Memory allocators
- Math library
- Logging system
- Platform abstraction

**Week 3: Entity System**
- Entity ID system
- Entity manager
- Component storage
- Entity queries

**Week 4: Rendering Foundation**
- Vulkan initialization
- Swap chain management
- Render pass system
- "Hello Triangle" on Android

### Month 2: ENGINE Rendering & Physics

**Week 5: Graphics Pipeline**
- PBR material system
- Texture/mesh loading
- Forward rendering pipeline
- Basic lighting

**Week 6: Advanced Rendering**
- Shadow mapping
- Skybox rendering
- Post-processing framework
- LOD system

**Week 7: Physics Integration**
- Jolt Physics integration
- Rigid body component
- Collision shapes
- Character controller

**Week 8: Mobile Editor Foundation**
- Editor UI architecture
- ImGui integration
- Scene hierarchy panel
- Property inspector

### Month 3: ENGINE Completion + Basic Platform

**Weeks 9-10: ENGINE Features**
- Lua scripting integration
- Audio system
- Input handling

**Weeks 11-12: BASIC Platform**
- Basic game library UI
- Basic publishing workflow
- Basic user accounts
- Google Play submission

---

## 🎯 PERFORMANCE TARGETS

### FPS Targets by Device Tier

| Device Tier | FPS Target | RAM Budget |
|-------------|------------|------------|
| Ultra-low (2014-2017) | 30 FPS | 200 MB |
| Low-end (2017-2021) | 45 FPS | 300 MB |
| Mid-range (2019-2023) | 60 FPS | 400 MB |
| High-end (2023+) | 120+ FPS | 600 MB |

### Quality Standards

- **Code Quality**: Industry-leading, peer-reviewed standard
- **Performance**: Better than Unreal/Unity/Godot on equivalent hardware
- **Reliability**: Zero crashes, graceful degradation, bulletproof
- **Maintainability**: Clean architecture, clear patterns, well-documented

---

## 🔧 TECHNICAL STACK

### Languages
- **C++23** (95%) - Core engine
- **Mojo** (3%) - AI/ML components
- **Rust** (2%) - Networking, crypto

### Graphics APIs
- **Vulkan** - Android primary
- **Metal** - iOS
- **WebGPU** - Web

### Build System
- **CMake** - Cross-platform builds
- **Android NDK** - Android builds
- **Xcode** - iOS builds

### Libraries (Only These Allowed)
- **Low-level APIs**: Vulkan, Metal, SDL
- **Audio**: Wwise integration
- **Physics**: Jolt (inspiration, custom implementation)
- **Profiling**: Tracy

---

## 📂 REPOSITORY STRUCTURE

```
Nova-Forge/
├── AUTONOMOUS_DEVELOPMENT_GUIDE.md  ← THIS FILE (Start Here)
├── NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md  ← Technical specs
├── README.md  ← Project overview
├── LICENSE
├── docs/
│   ├── PROGRESS.md  ← Current state & what to do next
│   ├── NOVACORE_COMPLETE_DOCUMENTATION.md
│   ├── EDITOR_AND_GAME_BUILDER_GUIDE.md
│   ├── SELF_LEARNING_AND_TRAINING_GUIDE.md
│   └── TECHNICAL_README.md
├── src/  ← Source code (to be created)
├── include/  ← Headers (to be created)
├── tests/  ← Tests (to be created)
└── tools/  ← Build tools (to be created)
```

---

## 🔄 QUICK REFERENCE - WHAT TO DO WHEN...

### Starting a New Session
1. Read this document
2. Read PROGRESS.md
3. Continue from where you left off

### Finishing a Task
1. Run all tests
2. Update PROGRESS.md
3. Commit with descriptive message

### Starting a New Phase
1. Verify previous phase is 100% complete
2. All tests passing
3. Performance targets met
4. Documentation complete
5. Begin new phase tasks

### Hitting a Blocker
1. Document the issue in PROGRESS.md
2. Research solutions
3. Implement and test solution
4. Document the resolution

### Making Architecture Decisions
1. Document the options considered
2. Explain rationale for choice
3. Note trade-offs
4. Update relevant documentation

---

## 📞 PROJECT INFO

**Company**: WeNova Interactive (operating as Kayden Shawn Massengill)  
**Platform**: NovaForge  
**Engine**: NovaCore  
**Repository**: github.com/MrNova420/Nova-Forge

---

## 🎯 FINAL REMINDER

> **ENGINE FIRST. ALWAYS.**  
> The NovaCore game engine is the foundation.  
> Build the engine completely before building platform features.  
> Follow this guide, update PROGRESS.md, and maintain AAA quality.

---

**NovaForge** - *Play. Create. Earn.*
