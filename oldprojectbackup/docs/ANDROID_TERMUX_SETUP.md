# Android/Termux Setup Guide

This guide provides instructions for running Nova Engine on Android devices using Termux.

## Prerequisites

1. **Install Termux** from F-Droid (recommended) or Google Play Store
2. **Update packages**:

   ```bash
   pkg update && pkg upgrade
   ```

3. **Install required packages**:
   ```bash
   pkg install nodejs git python
   ```

## Installation Steps

### 1. Clone the Repository

```bash
git clone https://github.com/MrNova420/Nova-Engine.git
cd Nova-Engine
```

### 2. Install Dependencies

The project is configured to work on Termux with `legacy-peer-deps` enabled by default in `.npmrc`:

```bash
npm install
```

**Note**: You may see warnings about deprecated packages - these are normal and can be safely ignored.

### 3. Start the Application

```bash
npm start
```

This will start:

- **Frontend**: http://localhost:3000 (Unified Platform)
- **Backend**: http://localhost:3001 (Nova Hub Server)

## Troubleshooting

### Issue: "concurrently: not found" or "vite: not found"

**Cause**: Node modules binaries are not in PATH after installation.

**Solutions**:

1. **Try reinstalling**:

   ```bash
   rm -rf node_modules package-lock.json
   npm install
   ```

2. **Use npx to run binaries**:

   ```bash
   npx concurrently "npm run dev -w @nova-engine/hub" "npm run dev -w @nova-engine/unified-platform"
   ```

3. **Run workspaces separately** (if concurrently doesn't work):

   ```bash
   # Terminal 1
   npm run dev -w @nova-engine/hub

   # Terminal 2 (swipe right in Termux to create new session)
   npm run dev -w @nova-engine/unified-platform
   ```

### Issue: bcrypt compilation errors

**Cause**: Native bcrypt requires Android NDK which is not available in standard Termux.

**Solution**: The project has been updated to use `bcryptjs` (pure JavaScript implementation) instead of native `bcrypt`. If you still see this error:

1. Make sure you have the latest version:

   ```bash
   git pull origin main
   ```

2. Clean and reinstall:
   ```bash
   npm run clean:all
   npm install
   ```

### Issue: Out of memory errors

**Cause**: Limited RAM on mobile devices.

**Solutions**:

1. **Increase Node.js memory limit**:

   ```bash
   export NODE_OPTIONS="--max-old-space-size=2048"
   npm start
   ```

2. **Close other apps** to free up memory

3. **Run only the frontend or backend** instead of both:

   ```bash
   # Frontend only
   npm run start:frontend

   # Backend only
   npm run start:backend
   ```

### Issue: Port already in use

**Solution**: Kill existing Node processes:

```bash
pkill node
npm start
```

## Performance Tips

1. **Use a device with at least 4GB RAM** for better experience
2. **Close background apps** before starting Nova Engine
3. **Use a modern Android version** (Android 9+)
4. **Consider using Termux:Boot** to automatically start services

## Accessing from Other Devices

Once running, you can access Nova Engine from other devices on your network:

1. Find your device's IP address:

   ```bash
   ifconfig | grep inet
   ```

2. Access from other devices:
   - Frontend: `http://YOUR_DEVICE_IP:3000`
   - Backend API: `http://YOUR_DEVICE_IP:3001`

## Building

Building the full project on Termux is not recommended due to:

- Desktop launcher requires system libraries not available on Android
- Large memory requirements for bundling

If you need to build specific packages:

```bash
# Build only the packages that work on Termux
npm run build -w @nova-engine/engine
npm run build -w @nova-engine/server
npm run build -w @nova-engine/shared
npm run build -w @nova-engine/unified-platform
```

## Limitations on Android/Termux

- **Desktop launcher** cannot be built (requires GTK/native libraries)
- **Some build tools** may be slower due to mobile hardware
- **3D performance** may be limited compared to desktop browsers
- **File system access** is restricted to Termux's home directory

## Support

If you encounter issues not covered here:

1. Check [main documentation](../README.md)
2. Search [existing issues](https://github.com/MrNova420/Nova-Engine/issues)
3. Create a new issue with the "android" or "termux" label

## Contributing

Contributions to improve Android/Termux support are welcome! See [CONTRIBUTING.md](../CONTRIBUTING.md).
