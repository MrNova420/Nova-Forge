# Nova Engine - Local Setup Guide

## 🚀 Run Everything Locally - No Server Required!

Nova Engine is designed to work **completely offline** or with **peer-to-peer networking** when you have internet. Just download, start, and use!

## Quick Start (3 Steps)

### 1. Clone the Repository

```bash
git clone https://github.com/MrNova420/Nova-Engine.git
cd Nova-Engine
```

### 2. Install Dependencies

```bash
npm install
```

### 3. Start Nova Engine

```bash
npm run start:local
```

That's it! Nova Engine will:

- ✅ Start on `http://localhost:3000`
- ✅ Open automatically in your browser
- ✅ Work 100% offline
- ✅ Auto-connect to other local users if online
- ✅ Enable P2P game sharing

## What You Get Locally

### Fully Functional Features:

- ✅ **Game Editor** - Create games with visual tools
- ✅ **Game Launcher** - Play games locally
- ✅ **Local Game Library** - Store games on your device
- ✅ **Project Management** - Save and load projects
- ✅ **Asset Management** - Import textures, models, audio
- ✅ **Build System** - Export games for web/desktop/mobile
- ✅ **Local Multiplayer** - LAN play with friends

### When Connected to Internet:

- ✅ **P2P Game Discovery** - Find games from other users
- ✅ **P2P Publishing** - Share your games instantly
- ✅ **P2P Multiplayer** - Play with anyone online
- ✅ **Auto-Sync** - Games sync across all connected users
- ✅ **Friend Discovery** - Connect with nearby users
- ✅ **Cloud Backup** - Optional sync to cloud storage

## Local Data Storage

All data is stored locally in:

```
~/.nova-engine/
├── games/          # Downloaded and created games
├── projects/       # Your game projects
├── assets/         # Imported assets
├── saves/          # Game save data
└── config/         # Settings and preferences
```

## Peer-to-Peer Networking

When online, Nova Engine automatically:

1. **Discovers nearby users** on your LAN
2. **Connects to peers** using WebRTC
3. **Shares game catalog** with connected peers
4. **Enables direct downloads** from peers
5. **Syncs published games** across the network

### How P2P Works:

```
Your Computer                Other Users
     │                            │
     ├─────────────────────────────┤
     │    WebRTC P2P Connection    │
     │                              │
     │   Share: Games, Projects    │
     │   Discover: Friends, Lobbies│
     │   Play: Multiplayer Games   │
     └──────────────────────────────┘
```

### Network Modes:

1. **Offline Mode** - Everything works locally
2. **LAN Mode** - Connect with local network users
3. **Internet Mode** - Connect globally via relay servers
4. **Hybrid Mode** - Best connection automatically

## Running Different Components

### All-in-One (Recommended)

```bash
npm run start:local
```

Starts unified platform with everything integrated.

### Individual Components

```bash
# Just the editor
npm run start:editor

# Just the launcher
npm run start:launcher

# Just the hub (game browser)
npm run start:hub

# Just the server (for hosting)
npm run start:server
```

## Building for Distribution

### Web Build

```bash
npm run build
# Output: dist/web/
# Deploy to any static host
```

### Desktop Build (Tauri)

```bash
npm run build:desktop
# Output: dist/desktop/
# Standalone executable for Windows/Mac/Linux
```

### Mobile Build

```bash
npm run build:mobile
# Output: dist/mobile/
# APK for Android, IPA for iOS
```

## Publishing Games (Local Network)

1. Create your game in the editor
2. Click "Publish"
3. Game is automatically shared via P2P
4. Other users see it in their hub
5. They can download and play instantly

No central server needed!

## Playing Games

### Local Games

- Play any game in your library
- Works 100% offline
- Save data stored locally

### Shared Games (P2P)

- Browse games from connected peers
- Download directly from other users
- Cache locally for offline play

### Multiplayer

- Create/join lobbies on LAN
- Automatic peer discovery
- Direct P2P connections
- Voice chat via WebRTC

## Advanced Configuration

Edit `~/.nova-engine/config.json`:

```json
{
  "network": {
    "mode": "hybrid",
    "enableP2P": true,
    "maxPeers": 50,
    "relayServers": [
      "wss://relay1.novaengine.com",
      "wss://relay2.novaengine.com"
    ]
  },
  "storage": {
    "maxCacheSize": 10737418240,
    "autoCleanup": true
  },
  "editor": {
    "autoSave": true,
    "autoSaveInterval": 60000
  }
}
```

## Troubleshooting

### Port Already in Use

```bash
# Use different port
PORT=3001 npm run start:local
```

### Can't Find Other Users

- Check firewall settings
- Ensure same network
- Verify P2P is enabled

### Games Not Loading

- Check `~/.nova-engine/games/` directory
- Verify disk space
- Check browser console for errors

## System Requirements

### Minimum:

- **OS**: Windows 10, macOS 10.15, Linux (Ubuntu 20.04+)
- **CPU**: Dual-core 2.0 GHz
- **RAM**: 4 GB
- **Storage**: 2 GB free space
- **Browser**: Chrome 90+, Firefox 88+, Safari 14+

### Recommended:

- **CPU**: Quad-core 3.0 GHz+
- **RAM**: 8 GB+
- **GPU**: Dedicated graphics card with WebGL 2.0
- **Storage**: 10 GB+ free space
- **Network**: Broadband connection for P2P

## Development Mode

For contributors:

```bash
# Install dev dependencies
npm install

# Run in development mode
npm run dev

# Run tests
npm run test

# Build everything
npm run build:all

# Lint code
npm run lint

# Format code
npm run format
```

## Future: Deployed Version

Later, Nova Engine will also be available as a hosted service:

- **Website**: https://novaengine.com
- **Desktop App**: Download from website
- **Mobile Apps**: iOS App Store, Google Play Store

But you can **always** run it locally from GitHub!

## Support

- **Issues**: https://github.com/MrNova420/Nova-Engine/issues
- **Discussions**: https://github.com/MrNova420/Nova-Engine/discussions
- **Wiki**: https://github.com/MrNova420/Nova-Engine/wiki

---

**Nova Engine - Create. Play. Share. Anywhere.**
