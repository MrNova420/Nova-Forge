# 🎮 Nova Engine - Quick Start Guide

Welcome to Nova Engine! This guide will help you get started with the unified platform.

## 🚀 Quick Start

### Option 1: Simple Start (Recommended)

**On Linux/Mac:**

```bash
npm start
```

**On Windows:**

```bash
npm run start:windows
```

This will:

- ✅ Auto-install dependencies if needed
- ✅ Start the backend server (Hub API)
- ✅ Start the frontend (Unified Platform)
- ✅ Auto-restart services if they crash
- ✅ Keep running even if you close the terminal

### Option 2: Development Mode

If you want to see live console output:

```bash
npm run start:dev
```

## 🌐 Access the Platform

Once started, open your browser:

- **Frontend (Main App):** http://localhost:3000
- **Backend API:** http://localhost:3001
- **API Docs:** http://localhost:3001/api

## 📱 PWA Installation

When you first open the platform on mobile:

1. You'll see a prompt to "Install App"
2. Click "Install" to add Nova Engine to your home screen
3. The app will work like a native app!

## 🎛️ Platform Controls

### Navigation Sidebar

- **Toggle Button:** Click the circular button (☰/✕) to show/hide the sidebar
- Works on all devices - desktop, tablet, and mobile
- Automatically collapses on mobile for more screen space

### Homepage

- Click on any planet to navigate to that feature
- Click the **Nova Engine logo** or **Home button** in the top bar to return to homepage

### Features

Each feature has its own dedicated page:

- 🏠 **Hub** - Browse and discover games
- ✏️ **Editor** - Create games with the visual editor
- 🎮 **Launcher** - Play your games
- 🌐 **Multiplayer** - Play online with friends
- 👥 **Social** - Friends, achievements, leaderboards
- ⚙️ **Settings** - Configure your preferences

## 🔧 Management Commands

### Check Status

```bash
npm run status
```

### Stop All Services

```bash
npm run stop
```

### Restart Services

```bash
npm run restart
```

### View Logs

```bash
npm run logs
```

## 📝 Important Notes

### First Time Setup

The first time you run the platform, it will:

1. Install all dependencies (this takes a few minutes)
2. Initialize the database (if configured)
3. Start all services

### Data Storage

- **Demo Mode:** If backend is unavailable, the platform uses demo data
- **Real Mode:** When backend is connected, all data is stored in the database

### Mobile Usage

- The platform is fully responsive
- Sidebar auto-collapses on mobile
- Touch targets are optimized for mobile (minimum 44px)
- PWA support allows offline usage

## 🐛 Troubleshooting

### Services Won't Start

```bash
# Clean everything and reinstall
npm run clean:all
npm install
npm start
```

### Port Already in Use

If ports 3000 or 3001 are already in use:

1. Stop the conflicting services
2. Or modify the port in `.env` files

### Backend Not Connecting

Check if the backend is running:

```bash
npm run status
```

If not running:

```bash
npm run restart
```

### View Detailed Logs

```bash
# View all logs
npm run logs

# Or view specific log files
tail -f logs/frontend.log
tail -f logs/hub-server.log
```

## 🎯 Next Steps

1. **Create an Account:** Register from the homepage
2. **Explore Games:** Visit the Hub to browse games
3. **Create a Game:** Use the Editor to build your first game
4. **Play Online:** Join multiplayer lobbies
5. **Connect with Friends:** Add friends in the Social tab

## 💡 Tips

- **Sidebar Toggle:** Use the floating button to hide/show navigation
- **Keyboard Shortcuts:** Will be available in editor (coming soon)
- **Mobile Performance:** App works offline after first load
- **Auto-Save:** Projects auto-save as you work
- **Real-time Updates:** Friends and multiplayer updates in real-time

## 📚 Documentation

For more detailed documentation:

- Full docs: `docs/` directory
- API Reference: http://localhost:3001/api
- Engine docs: `packages/engine/README.md`
- Editor docs: `packages/editor/README.md`

## ⚖️ Legal & Licensing

⚠️ **IMPORTANT:** Before using Nova Engine, you must review and agree to:

- **[LEGAL.md](LEGAL.md)** - Complete legal documentation overview
- **[LICENSE](LICENSE)** - Proprietary license agreement
- **[Developer License](legal/NovaEngine_DeveloperLicense.pdf)** - Terms for game developers

**Nova Engine is proprietary software** with comprehensive licensing terms that govern all use.

## 🆘 Support

Having issues?

1. Check the logs: `npm run logs`
2. Check service status: `npm run status`
3. Restart services: `npm run restart`
4. Review documentation in `docs/` folder

---

**Happy Game Development! 🎮✨**

_Nova Engine - Create • Play • Share_
