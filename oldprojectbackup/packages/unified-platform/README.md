# 🎮 Nova Engine - Unified All-in-One Platform

## THE MAIN APP - Everything in One Place

**This is the primary application users should run!**

## ONE Web App - Everything Included

Nova Engine is a **single unified web application** that provides everything:

```
┌─────────────────────────────────────────────────────────────┐
│              NOVA ENGINE WEB APPLICATION                     │
│              (One App - All Features)                        │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  🎮 HUB          🛠️ EDITOR       🚀 LAUNCHER                │
│  Browse Games    Create Games    Play Games                  │
│                                                               │
│  👥 SOCIAL       🎯 MULTIPLAYER  ⚙️  SETTINGS               │
│  Friends         Online Play      Account                    │
│                                                               │
└─────────────────────────────────────────────────────────────┘
         Access everything from http://localhost:3000
              OR deployed at novaengine.com
```

## Quick Start - Just Run It! ⚡

**THE SIMPLEST WAY TO USE NOVA ENGINE:**

### 1. Clone and Install (One Time Setup)

```bash
git clone https://github.com/MrNova420/Nova-Engine.git
cd Nova-Engine
npm install
```

### 2. Start the Web App

```bash
npm start
```

### 3. Open Your Browser

The app automatically opens at `http://localhost:3000`

**That's it!** You now have:

- ✅ Full game editor
- ✅ Game launcher/player
- ✅ Game browser (hub)
- ✅ Multiplayer lobbies
- ✅ Social features
- ✅ Everything in ONE web app!

## How It Works

### Single Web Application

- ONE React app with all features integrated
- Switch between modes instantly (no page reloads)
- Single sign-on for all features
- All data synced in one place

### Works Locally

When you run `npm start` locally:

- Everything works offline
- Games stored in browser storage
- P2P networking when online
- No server required initially

### Works Deployed

When deployed to novaengine.com:

- Professional hosting
- Cloud storage
- Central game catalog
- But still the SAME web app!

## Features in ONE App

### 🎮 Hub (Game Discovery)

- Browse all published games
- Search and filter
- Featured games
- Top rated
- New releases
- All in the web app!

### 🛠️ Editor (Game Creation)

- Visual scene editor
- Component system
- Asset browser
- Material editor
- Animation tools
- Test games instantly
- Publish directly to hub
- All in the same window!

### 🚀 Launcher (Game Player)

- Play any game
- Save/load games
- Controller support
- Fullscreen mode
- Performance metrics
- Runs in the browser!

### 👥 Social

- Friends list
- Activity feed
- Achievements
- Chat
- Profile
- Integrated seamlessly!

### 🎯 Multiplayer

- Create lobbies
- Quick match
- Voice chat
- In-game overlay
- Part of the web app!

## Navigation

Users never leave the app. Just click to switch:

```
┌─────────────────────────────────────────────┐
│  [Nova Engine]  User  🔔  ⚙️                │  ← Top Bar
├──────┬──────────────────────────────────────┤
│ 🎮   │  Game Browser Content                │
│ 🛠️   │  (Hub, Editor, Launcher, etc.)       │
│ 🚀   │                                       │
│ 👥   │  Everything switches here            │
│ 🎯   │  No page reloads!                    │
│ ⚙️    │                                       │
└──────┴──────────────────────────────────────┘
  Side    Main Content Area
  Nav     (Single Page App)
```

## File Structure

```
packages/unified-platform/
├── src/
│   ├── core/
│   │   ├── UnifiedApp.tsx         ← Main React app
│   │   └── UnifiedPlatformCore.ts ← Core logic
│   ├── modules/
│   │   ├── HubModule.tsx          ← Hub integrated
│   │   ├── EditorModule.tsx       ← Editor integrated
│   │   ├── LauncherModule.tsx     ← Launcher integrated
│   │   ├── MultiplayerModule.tsx  ← Multiplayer integrated
│   │   ├── SocialModule.tsx       ← Social integrated
│   │   └── SettingsModule.tsx     ← Settings integrated
│   ├── ui/
│   │   ├── UnifiedNavigation.tsx  ← Side navigation
│   │   ├── UnifiedTopBar.tsx      ← Top bar
│   │   └── NotificationCenter.tsx ← Notifications
│   └── services/
│       ├── P2PNetworkService.ts   ← P2P for local use
│       ├── FriendsService.ts      ← Friends
│       ├── AchievementsService.ts ← Achievements
│       └── MultiplayerLobbyService.ts ← Lobbies
└── public/
    └── index.html                  ← Single HTML file
```

## Deployment Options

### Option 1: Local Development (NOW)

```bash
npm start
# Runs on http://localhost:3000
# Works fully offline
# P2P networking when online
```

### Option 2: Self-Hosted

```bash
npm run build
# Upload dist/ to any web host
# Works as static site
# Configure backend API URL
```

### Option 3: Full Deployment (FUTURE)

```bash
# Deploy to production
# novaengine.com
# Professional hosting
# Central backend
```

## Why ONE Web App?

### ✅ User Benefits

- No installations
- Access from anywhere
- Single account
- Seamless transitions
- Cross-device sync

### ✅ Developer Benefits

- One codebase
- Shared components
- Unified state
- Easy deployment
- Consistent UX

### ✅ Technical Benefits

- React SPA
- Code splitting
- Service workers
- Offline support
- Progressive Web App

## Technology

- **Framework**: React 18
- **Routing**: React Router (SPA)
- **State**: Zustand + Context
- **Build**: Vite
- **TypeScript**: Full type safety
- **PWA**: Installable web app

## Browser Requirements

- Chrome 90+
- Firefox 88+
- Safari 14+
- Edge 90+

All features work in modern browsers!

## Coming Soon

- PWA installation (add to home screen)
- Offline game playing
- Background sync
- Push notifications
- Desktop app wrapper (optional)

But always accessible as a web app!

---

**Nova Engine - ONE web application for everything.**

Visit: http://localhost:3000 (local) or https://novaengine.com (deployed)
