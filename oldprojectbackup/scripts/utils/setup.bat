@echo off
REM Nova Engine - Easy Setup Script (Windows)
REM One command to install everything!

echo.
echo 🎮 Nova Engine - Easy Setup
echo ============================
echo.

REM Check if Node.js is installed
where node >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ❌ Node.js not found!
    echo Please install Node.js 18+ from https://nodejs.org/
    pause
    exit /b 1
)

echo ✅ Node.js detected: 
node -v
echo.

REM Install dependencies
echo 📦 Installing all dependencies...
call npm install
if %ERRORLEVEL% NEQ 0 (
    echo ❌ Installation failed!
    pause
    exit /b 1
)

echo.
echo 🏗️ Building all packages...
call npm run build
if %ERRORLEVEL% NEQ 0 (
    echo ❌ Build failed!
    pause
    exit /b 1
)

echo.
echo 🧪 Running tests...
call npm test
if %ERRORLEVEL% NEQ 0 (
    echo ⚠️ Some tests failed, but setup is complete
)

echo.
echo ✅ Setup Complete!
echo.
echo 🚀 Next steps:
echo    npm run dev:editor    # Start the editor
echo    npm run dev:engine    # Start engine dev server
echo.
echo 📚 Documentation:
echo    SETUP.md              # Detailed setup guide
echo    USER_GUIDE.md         # How to use Nova Engine
echo    examples\             # Example projects
echo.
echo Happy game development! 🎮✨
echo.
pause
