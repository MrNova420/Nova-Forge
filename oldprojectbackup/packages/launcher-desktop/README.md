# Nova Launcher Desktop

Native desktop application for the Nova Engine game platform. Built with Tauri and React for cross-platform support (Windows, macOS, Linux).

## Features

### ✅ Implemented (Phase 9.1)

- **Local Game Storage** - Download and manage games offline
- **Save Data Management** - Local save data with cloud sync support
- **Offline Sync** - Automatic synchronization when online
- **Update Manager** - Game and launcher auto-updates
- **Native UI** - Desktop-optimized user interface
- **Cross-Platform** - Windows, macOS, and Linux support

### Core Services

1. **LocalGameStorage** - Manages downloaded games on local filesystem
2. **LocalSaveData** - Manages game save data
3. **OfflineSync** - Synchronizes data with Nova Hub
4. **UpdateManager** - Handles updates

## Technology Stack

- **Frontend**: React 19 + TypeScript + Vite
- **Desktop Framework**: Tauri 2.0
- **Routing**: React Router v6
- **State**: Zustand
- **Backend**: Rust (Tauri core)

## Development

```bash
# Install dependencies
npm install

# Run in development mode
npm run dev

# Build for production
npm run build
```

## Status

✅ Phase 9.1 Complete - Desktop Application Foundation
