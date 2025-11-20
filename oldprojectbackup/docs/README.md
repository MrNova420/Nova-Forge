# Nova Engine Documentation

This directory contains documentation for the Nova Engine project.

## Documentation Types

### API Documentation (TypeDoc)

Auto-generated API documentation from TypeScript source code.

**Generate:** `npm run docs:build`  
**Watch Mode:** `npm run docs:watch`  
**Output:** `docs/api/`

The API documentation is automatically generated from JSDoc comments in the source code.

### User Documentation (Future: Docusaurus)

When user-facing documentation is needed, Docusaurus will be set up here:

```bash
# To set up Docusaurus (when ready):
npx create-docusaurus@latest user-docs classic
```

## Existing Documentation Files

Located in the root directory:

- `README.md` - Project overview
- `DEVELOPMENT_GUIDE.md` - Complete development execution guide
- `ROADMAP.md` - 30-month development roadmap
- `ARCHITECTURE.md` - Technical architecture
- `COMPONENT_BREAKDOWN.md` - Detailed component specifications
- `PROJECT_STRUCTURE.md` - Code organization
- `CONTRIBUTING.md` - Development standards and workflow
- `AUTOMATION.md` - Automated systems
- `ECOSYSTEM_VISION.md` - Extended platform vision
- `PROGRESS.md` - Development progress tracker
- `QUICK_START.md` - Getting started guide
- `CHANGELOG.md` - Version history

## Contributing to Documentation

1. Add JSDoc comments to all public APIs
2. Keep markdown files up to date
3. Generate API docs after significant changes
4. Follow documentation standards in CONTRIBUTING.md
