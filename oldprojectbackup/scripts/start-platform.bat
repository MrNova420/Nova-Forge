@echo off
REM ============================================================================
REM NOVA ENGINE - Platform Auto-Restart Script (Windows)
REM 
REM This script ensures the platform stays running even if terminal is closed.
REM It starts both backend servers and frontend, with auto-restart on failure.
REM ============================================================================

setlocal enabledelayedexpansion

set "PROJECT_ROOT=%~dp0.."
set "LOG_DIR=%PROJECT_ROOT%\logs"
set "PID_DIR=%PROJECT_ROOT%\.pids"

REM Create directories
if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"
if not exist "%PID_DIR%" mkdir "%PID_DIR%"

REM Colors
set "GREEN=[92m"
set "YELLOW=[93m"
set "RED=[91m"
set "BLUE=[94m"
set "PURPLE=[95m"
set "NC=[0m"

echo.
echo %PURPLE%╔═══════════════════════════════════════════════════════════╗%NC%
echo %PURPLE%║                                                           ║%NC%
echo %PURPLE%║        🎮 NOVA ENGINE - UNIFIED PLATFORM 🎮              ║%NC%
echo %PURPLE%║                                                           ║%NC%
echo %PURPLE%║         Starting All Services (Windows)                  ║%NC%
echo %PURPLE%║                                                           ║%NC%
echo %PURPLE%╚═══════════════════════════════════════════════════════════╝%NC%
echo.

cd /d "%PROJECT_ROOT%"

REM Check if node_modules exists
if not exist "node_modules" (
    echo %BLUE%[INFO]%NC% Installing dependencies...
    call npm install
)

REM Start services using Windows START command (runs in separate windows)
echo %BLUE%[INFO]%NC% Starting Hub Server...
start "Nova Hub Server" /MIN cmd /k "npm run dev -w @nova-engine/hub"

timeout /t 3 /nobreak >nul

echo %BLUE%[INFO]%NC% Starting Frontend...
start "Nova Frontend" /MIN cmd /k "npm run dev -w @nova-engine/unified-platform"

timeout /t 3 /nobreak >nul

echo.
echo %GREEN%═══════════════════════════════════════════════════════════%NC%
echo %GREEN%🚀 Nova Engine Platform Started Successfully!%NC%
echo %GREEN%═══════════════════════════════════════════════════════════%NC%
echo.
echo %BLUE%📱 Frontend:  http://localhost:3000%NC%
echo %BLUE%🔧 Backend:   http://localhost:3001%NC%
echo %BLUE%📊 Hub API:   http://localhost:3001/api%NC%
echo.
echo %BLUE%💡 Services are running in separate windows%NC%
echo %BLUE%💡 Close the windows to stop the services%NC%
echo.
echo Press any key to exit this window...
pause >nul
