/**
 * @file platform_types.hpp
 * @brief NovaCore Platform™ - Core Type Definitions
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 * 
 * Platform abstraction layer for cross-platform development:
 * - Window management
 * - Platform detection
 * - System information
 * - Application lifecycle
 */

#pragma once

#include <nova/core/types/types.hpp>
#include <nova/core/math/math.hpp>

#include <string>
#include <vector>
#include <functional>

namespace nova::platform {

using namespace nova::core;
using namespace nova::core::math;

// ============================================================================
// Platform Detection
// ============================================================================

/**
 * @brief Target platform
 */
enum class Platform : u8 {
    Unknown,
    Windows,
    Linux,
    macOS,
    iOS,
    Android,
    Web,           // WebAssembly
    PlayStation,
    Xbox,
    Nintendo
};

/**
 * @brief CPU architecture
 */
enum class Architecture : u8 {
    Unknown,
    x86,
    x64,
    ARM32,
    ARM64,
    WASM
};

/**
 * @brief Graphics API
 */
enum class GraphicsAPI : u8 {
    None,
    Vulkan,
    DirectX11,
    DirectX12,
    Metal,
    OpenGL,
    OpenGLES,
    WebGPU
};

/**
 * @brief Get current platform
 */
constexpr Platform getCurrentPlatform() {
    #if defined(_WIN32) || defined(_WIN64)
    return Platform::Windows;
    #elif defined(__APPLE__)
        #include <TargetConditionals.h>
        #if TARGET_OS_IPHONE
        return Platform::iOS;
        #else
        return Platform::macOS;
        #endif
    #elif defined(__ANDROID__)
    return Platform::Android;
    #elif defined(__linux__)
    return Platform::Linux;
    #elif defined(__EMSCRIPTEN__)
    return Platform::Web;
    #else
    return Platform::Unknown;
    #endif
}

/**
 * @brief Get current architecture
 */
constexpr Architecture getCurrentArchitecture() {
    #if defined(__x86_64__) || defined(_M_X64)
    return Architecture::x64;
    #elif defined(__i386__) || defined(_M_IX86)
    return Architecture::x86;
    #elif defined(__aarch64__) || defined(_M_ARM64)
    return Architecture::ARM64;
    #elif defined(__arm__) || defined(_M_ARM)
    return Architecture::ARM32;
    #elif defined(__EMSCRIPTEN__)
    return Architecture::WASM;
    #else
    return Architecture::Unknown;
    #endif
}

/**
 * @brief Check if running on mobile
 */
constexpr bool isMobilePlatform() {
    Platform p = getCurrentPlatform();
    return p == Platform::iOS || p == Platform::Android;
}

/**
 * @brief Check if running on desktop
 */
constexpr bool isDesktopPlatform() {
    Platform p = getCurrentPlatform();
    return p == Platform::Windows || p == Platform::Linux || p == Platform::macOS;
}

// ============================================================================
// Window Types
// ============================================================================

/**
 * @brief Window style flags
 */
enum class WindowStyle : u32 {
    None = 0,
    Titled = 1 << 0,          // Has title bar
    Closable = 1 << 1,        // Has close button
    Minimizable = 1 << 2,     // Has minimize button
    Maximizable = 1 << 3,     // Has maximize button
    Resizable = 1 << 4,       // Can be resized
    Borderless = 1 << 5,      // No border
    Fullscreen = 1 << 6,      // Fullscreen mode
    AlwaysOnTop = 1 << 7,     // Always on top
    Transparent = 1 << 8,     // Transparent background
    
    Default = Titled | Closable | Minimizable | Maximizable | Resizable
};

inline WindowStyle operator|(WindowStyle a, WindowStyle b) {
    return static_cast<WindowStyle>(static_cast<u32>(a) | static_cast<u32>(b));
}

inline WindowStyle operator&(WindowStyle a, WindowStyle b) {
    return static_cast<WindowStyle>(static_cast<u32>(a) & static_cast<u32>(b));
}

inline bool hasStyle(WindowStyle styles, WindowStyle flag) {
    return (static_cast<u32>(styles) & static_cast<u32>(flag)) != 0;
}

/**
 * @brief Window state
 */
enum class WindowState : u8 {
    Normal,
    Minimized,
    Maximized,
    Fullscreen,
    Hidden
};

/**
 * @brief Fullscreen mode
 */
enum class FullscreenMode : u8 {
    Windowed,
    Borderless,       // Borderless fullscreen (fake fullscreen)
    Exclusive         // True exclusive fullscreen
};

/**
 * @brief Display mode
 */
struct DisplayMode {
    u32 width = 0;
    u32 height = 0;
    u32 refreshRate = 60;
    u32 bitsPerPixel = 32;
    
    bool operator==(const DisplayMode& other) const {
        return width == other.width && height == other.height &&
               refreshRate == other.refreshRate && bitsPerPixel == other.bitsPerPixel;
    }
};

/**
 * @brief Monitor/display info
 */
struct MonitorInfo {
    std::string name;
    u32 index = 0;
    Vec2i position;           // Position in virtual desktop
    Vec2i size;               // Physical size in pixels
    Vec2i workArea;           // Work area (excluding taskbar)
    f32 dpi = 96.0f;
    f32 scale = 1.0f;         // UI scale factor
    bool isPrimary = false;
    std::vector<DisplayMode> displayModes;
    DisplayMode currentMode;
};

/**
 * @brief Window creation parameters
 */
struct WindowDesc {
    std::string title = "NovaCore";
    u32 width = 1280;
    u32 height = 720;
    i32 x = -1;               // -1 = centered
    i32 y = -1;
    WindowStyle style = WindowStyle::Default;
    FullscreenMode fullscreenMode = FullscreenMode::Windowed;
    u32 targetMonitor = 0;
    bool vsync = true;
    u32 samples = 1;          // MSAA samples
    GraphicsAPI graphicsAPI = GraphicsAPI::Vulkan;
    bool highDPI = true;
    void* parentWindow = nullptr;  // For child windows
};

/**
 * @brief Window handle
 */
using WindowHandle = void*;

// ============================================================================
// System Information
// ============================================================================

/**
 * @brief CPU information
 */
struct CPUInfo {
    std::string name;
    std::string vendor;
    u32 coreCount = 0;
    u32 threadCount = 0;
    u32 frequencyMHz = 0;
    u32 cacheL1KB = 0;
    u32 cacheL2KB = 0;
    u32 cacheL3KB = 0;
    bool hasSSE = false;
    bool hasSSE2 = false;
    bool hasSSE3 = false;
    bool hasSSE4_1 = false;
    bool hasSSE4_2 = false;
    bool hasAVX = false;
    bool hasAVX2 = false;
    bool hasAVX512 = false;
    bool hasNEON = false;
};

/**
 * @brief GPU information
 */
struct GPUInfo {
    std::string name;
    std::string vendor;
    std::string driver;
    u64 dedicatedMemory = 0;
    u64 sharedMemory = 0;
    u32 vendorId = 0;
    u32 deviceId = 0;
    bool isIntegrated = false;
    bool isDiscrete = false;
    GraphicsAPI supportedAPIs = GraphicsAPI::None;
};

/**
 * @brief Memory information
 */
struct MemoryInfo {
    u64 totalPhysical = 0;
    u64 availablePhysical = 0;
    u64 totalVirtual = 0;
    u64 availableVirtual = 0;
    u64 pageSize = 4096;
};

/**
 * @brief Storage information
 */
struct StorageInfo {
    std::string path;
    u64 totalBytes = 0;
    u64 freeBytes = 0;
    bool isRemovable = false;
    bool isNetwork = false;
};

/**
 * @brief Battery information
 */
struct BatteryInfo {
    f32 percentage = 100.0f;  // 0-100
    bool isCharging = false;
    bool isPluggedIn = true;
    i32 minutesRemaining = -1;  // -1 = unknown
    bool hasBattery = false;
};

/**
 * @brief System information
 */
struct SystemInfo {
    Platform platform = Platform::Unknown;
    Architecture architecture = Architecture::Unknown;
    std::string osName;
    std::string osVersion;
    std::string deviceName;
    std::string deviceModel;
    std::string locale;
    std::string timezone;
    CPUInfo cpu;
    std::vector<GPUInfo> gpus;
    MemoryInfo memory;
    std::vector<StorageInfo> storage;
    BatteryInfo battery;
    std::vector<MonitorInfo> monitors;
};

// ============================================================================
// Application Types
// ============================================================================

/**
 * @brief Application state
 */
enum class AppState : u8 {
    Starting,
    Running,
    Paused,       // Background on mobile
    Resuming,
    Stopping,
    Stopped
};

/**
 * @brief Application creation parameters
 */
struct AppDesc {
    std::string name = "NovaCore Application";
    std::string version = "1.0.0";
    std::string organization = "WeNova Interactive";
    WindowDesc window;
    bool handleExceptions = true;
    bool enableConsole = true;    // Debug console on desktop
    u32 targetFPS = 60;
    bool unlimitedFPS = false;
};

/**
 * @brief Frame timing information
 */
struct FrameInfo {
    u64 frameNumber = 0;
    f64 totalTime = 0.0;      // Seconds since start
    f64 deltaTime = 0.0;      // Seconds since last frame
    f64 unscaledDeltaTime = 0.0;
    f32 timeScale = 1.0f;
    f32 fps = 0.0f;
    f32 averageFps = 0.0f;
    f64 frameTime = 0.0;      // Time to complete frame (ms)
    f64 updateTime = 0.0;     // Time in update (ms)
    f64 renderTime = 0.0;     // Time in render (ms)
};

// ============================================================================
// Path Types
// ============================================================================

/**
 * @brief Special folder types
 */
enum class SpecialFolder : u8 {
    Executable,       // Directory containing executable
    UserData,         // User application data
    UserDocuments,    // User documents
    UserDesktop,      // User desktop
    UserDownloads,    // User downloads
    UserPictures,     // User pictures
    UserMusic,        // User music
    UserVideos,       // User videos
    AppData,          // Application data (roaming)
    LocalAppData,     // Local application data
    Cache,            // Cache directory
    Temp,             // Temp directory
    SaveGames,        // Save games directory
    Screenshots       // Screenshots directory
};

// ============================================================================
// Event Callbacks
// ============================================================================

using WindowResizedCallback = std::function<void(u32 width, u32 height)>;
using WindowMovedCallback = std::function<void(i32 x, i32 y)>;
using WindowFocusCallback = std::function<void(bool focused)>;
using WindowCloseCallback = std::function<bool()>;  // Return false to cancel
using WindowStateCallback = std::function<void(WindowState state)>;
using AppStateCallback = std::function<void(AppState state)>;
using LowMemoryCallback = std::function<void()>;
using OrientationCallback = std::function<void(bool isPortrait)>;

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * @brief Get platform name as string
 */
inline const char* getPlatformName(Platform platform) {
    switch (platform) {
        case Platform::Windows: return "Windows";
        case Platform::Linux: return "Linux";
        case Platform::macOS: return "macOS";
        case Platform::iOS: return "iOS";
        case Platform::Android: return "Android";
        case Platform::Web: return "Web";
        case Platform::PlayStation: return "PlayStation";
        case Platform::Xbox: return "Xbox";
        case Platform::Nintendo: return "Nintendo";
        default: return "Unknown";
    }
}

/**
 * @brief Get architecture name as string
 */
inline const char* getArchitectureName(Architecture arch) {
    switch (arch) {
        case Architecture::x86: return "x86";
        case Architecture::x64: return "x64";
        case Architecture::ARM32: return "ARM32";
        case Architecture::ARM64: return "ARM64";
        case Architecture::WASM: return "WebAssembly";
        default: return "Unknown";
    }
}

/**
 * @brief Get graphics API name as string
 */
inline const char* getGraphicsAPIName(GraphicsAPI api) {
    switch (api) {
        case GraphicsAPI::Vulkan: return "Vulkan";
        case GraphicsAPI::DirectX11: return "DirectX 11";
        case GraphicsAPI::DirectX12: return "DirectX 12";
        case GraphicsAPI::Metal: return "Metal";
        case GraphicsAPI::OpenGL: return "OpenGL";
        case GraphicsAPI::OpenGLES: return "OpenGL ES";
        case GraphicsAPI::WebGPU: return "WebGPU";
        default: return "None";
    }
}

} // namespace nova::platform
