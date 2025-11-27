// =============================================================================
// NovaForge Platform - API Types
// =============================================================================
// Platform: NovaForge | Engine: NovaCore | Company: WeNova Interactive
// (operating as Kayden Shawn Massengill)
//
// Core types used throughout the NovaForge API system.
// =============================================================================

#pragma once

#include <nova/core/types/types.hpp>
#include <nova/core/types/result.hpp>

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <functional>
#include <optional>
#include <chrono>

namespace nova::api {

// =============================================================================
// Forward Declarations
// =============================================================================

class NovaForgeApi;
class EngineApi;
class PlatformApi;
class ServicesApi;

// =============================================================================
// Basic Types
// =============================================================================

/// API result type for operations that can fail
using ApiResult = Result<void, Error>;

/// API result type with a return value
template<typename T>
using ApiResultOf = Result<T, Error>;

/// Callback handle for unregistering callbacks
using ApiCallbackHandle = u64;

/// Timestamp type
using ApiTimestamp = std::chrono::system_clock::time_point;

// =============================================================================
// Version Information
// =============================================================================

/**
 * @brief API version information
 */
struct ApiVersion {
    u32 major;      ///< Major version (breaking changes)
    u32 minor;      ///< Minor version (new features)
    u32 patch;      ///< Patch version (bug fixes)
    
    /// Get version as string (e.g., "1.0.0")
    [[nodiscard]] std::string toString() const {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }
    
    /// Get version as packed integer
    [[nodiscard]] u32 packed() const noexcept {
        return (major << 16) | (minor << 8) | patch;
    }
};

// =============================================================================
// Platform Information
// =============================================================================

/**
 * @brief API platform information - Full enterprise-grade implementation
 */
struct ApiPlatformInfo {
    // Operating System
    std::string osName;             ///< Operating system name (Windows, Linux, macOS, Android, iOS, Web)
    std::string osVersion;          ///< Operating system version
    std::string osCodename;         ///< OS codename (e.g., "Sonoma", "Jammy")
    u32 osBuildNumber = 0;          ///< OS build number
    
    // Architecture
    std::string architecture;       ///< CPU architecture (x86_64, ARM64, ARM32, WASM)
    std::string simdSupport;        ///< SIMD support (AVX2, AVX, SSE4.2, NEON, None)
    bool is64Bit = true;            ///< Whether running in 64-bit mode
    
    // Device Information
    std::string deviceModel;        ///< Device model name
    std::string deviceManufacturer; ///< Device manufacturer
    std::string deviceId;           ///< Unique device identifier (hashed for privacy)
    std::string deviceType;         ///< Device type (Desktop, Laptop, Tablet, Phone, Console, XR)
    
    // CPU Information
    std::string cpuName;            ///< CPU model name
    std::string cpuVendor;          ///< CPU vendor (Intel, AMD, Apple, Qualcomm, etc.)
    u32 cpuCores = 1;               ///< Number of physical CPU cores
    u32 cpuThreads = 1;             ///< Number of logical CPU threads
    u32 cpuFrequencyMHz = 0;        ///< CPU base frequency in MHz
    u32 cpuCacheL1KB = 0;           ///< L1 cache size in KB
    u32 cpuCacheL2KB = 0;           ///< L2 cache size in KB
    u32 cpuCacheL3KB = 0;           ///< L3 cache size in KB
    bool cpuHasSSE = false;         ///< SSE support
    bool cpuHasSSE2 = false;        ///< SSE2 support
    bool cpuHasSSE3 = false;        ///< SSE3 support
    bool cpuHasSSE41 = false;       ///< SSE4.1 support
    bool cpuHasSSE42 = false;       ///< SSE4.2 support
    bool cpuHasAVX = false;         ///< AVX support
    bool cpuHasAVX2 = false;        ///< AVX2 support
    bool cpuHasAVX512 = false;      ///< AVX-512 support
    bool cpuHasNEON = false;        ///< ARM NEON support
    bool cpuHasSVE = false;         ///< ARM SVE support
    
    // Memory Information
    u64 totalMemoryMB = 0;          ///< Total system memory in MB
    u64 availableMemoryMB = 0;      ///< Available system memory in MB
    u64 usedMemoryMB = 0;           ///< Used system memory in MB
    u64 pageSize = 4096;            ///< System page size in bytes
    
    // GPU Information
    bool hasGpu = false;            ///< Whether a GPU is available
    std::string gpuName;            ///< GPU model name
    std::string gpuVendor;          ///< GPU vendor (NVIDIA, AMD, Intel, Apple, Qualcomm, etc.)
    std::string gpuDriver;          ///< GPU driver version
    u64 gpuMemoryMB = 0;            ///< Dedicated GPU memory in MB
    u64 gpuSharedMemoryMB = 0;      ///< Shared GPU memory in MB
    bool gpuSupportsVulkan = false; ///< Vulkan support
    bool gpuSupportsMetal = false;  ///< Metal support (Apple)
    bool gpuSupportsD3D12 = false;  ///< Direct3D 12 support
    bool gpuSupportsWebGPU = false; ///< WebGPU support
    bool gpuSupportsRayTracing = false; ///< Hardware ray tracing support
    bool gpuSupportsMeshShaders = false; ///< Mesh shader support
    u32 gpuVulkanVersion = 0;       ///< Vulkan version (packed)
    u32 gpuMetalVersion = 0;        ///< Metal version
    
    // Display Information
    u32 displayCount = 1;           ///< Number of displays
    u32 primaryDisplayWidth = 0;    ///< Primary display width in pixels
    u32 primaryDisplayHeight = 0;   ///< Primary display height in pixels
    u32 primaryDisplayRefreshHz = 60; ///< Primary display refresh rate
    f32 primaryDisplayDPI = 96.0f;  ///< Primary display DPI
    f32 primaryDisplayScale = 1.0f; ///< Primary display scale factor
    bool hdrSupported = false;      ///< HDR display support
    
    // Battery Information (mobile/laptop)
    bool hasBattery = false;        ///< Whether device has battery
    f32 batteryLevel = 1.0f;        ///< Battery level (0.0 - 1.0)
    bool batteryCharging = false;   ///< Whether battery is charging
    u32 batteryEstimatedMinutes = 0; ///< Estimated battery life in minutes
    
    // Network Information
    bool hasNetwork = false;        ///< Whether network is available
    std::string networkType;        ///< Network type (WiFi, Cellular, Ethernet, None)
    bool isMeteredConnection = false; ///< Whether connection is metered
    
    // Build Information
    std::string buildType;          ///< Build type (Debug, Release, RelWithDebInfo)
    std::string compiler;           ///< Compiler used
    std::string compilerVersion;    ///< Compiler version
    std::string cppStandard;        ///< C++ standard (C++23, C++20, etc.)
    
    // Capabilities
    bool supportsMultithreading = true;  ///< Multithreading support
    bool supportsAsyncIO = true;         ///< Async I/O support
    bool supportsSIMD = false;           ///< Any SIMD support
    bool supportsHardwareAccel = false;  ///< Hardware acceleration support
    bool supportsVibration = false;      ///< Haptic/vibration support
    bool supportsCamera = false;         ///< Camera access support
    bool supportsMicrophone = false;     ///< Microphone access support
    bool supportsGPS = false;            ///< GPS/location support
    bool supportsAccelerometer = false;  ///< Accelerometer support
    bool supportsGyroscope = false;      ///< Gyroscope support
    bool supportsBluetooth = false;      ///< Bluetooth support
    bool supportsNFC = false;            ///< NFC support
};

// =============================================================================
// Configuration
// =============================================================================

/**
 * @brief Log level for API logging
 */
enum class ApiLogLevel : u8 {
    Trace = 0,      ///< Detailed trace information
    Debug = 1,      ///< Debug information
    Info = 2,       ///< Informational messages
    Warning = 3,    ///< Warnings
    Error = 4,      ///< Errors
    Fatal = 5,      ///< Fatal errors
    Off = 6         ///< Logging disabled
};

/**
 * @brief API configuration settings
 */
struct ApiConfig {
    /// Application name (shown in logs, etc.)
    std::string appName = "NovaForge Application";
    
    /// Application version
    std::string appVersion = "1.0.0";
    
    /// Enable debug mode (more logging, validation)
    bool debugMode = false;
    
    /// Log level
    ApiLogLevel logLevel = ApiLogLevel::Info;
    
    /// Enable analytics collection
    bool enableAnalytics = true;
    
    /// Enable crash reporting
    bool enableCrashReporting = true;
    
    /// Custom data directory (empty = use default)
    std::string dataDirectory;
    
    /// Custom cache directory (empty = use default)
    std::string cacheDirectory;
    
    /// Maximum memory usage in MB (0 = unlimited)
    u64 maxMemoryMB = 0;
    
    /// Enable threading (set to false for single-threaded mode)
    bool enableThreading = true;
    
    /// Number of worker threads (0 = auto-detect)
    u32 workerThreads = 0;
    
    /// Create default configuration
    [[nodiscard]] static ApiConfig defaults() {
        return ApiConfig{};
    }
    
    /// Create debug configuration
    [[nodiscard]] static ApiConfig debug() {
        ApiConfig config;
        config.debugMode = true;
        config.logLevel = ApiLogLevel::Debug;
        return config;
    }
    
    /// Create release configuration
    [[nodiscard]] static ApiConfig release() {
        ApiConfig config;
        config.debugMode = false;
        config.logLevel = ApiLogLevel::Warning;
        return config;
    }
};

// =============================================================================
// Events and Callbacks
// =============================================================================

/**
 * @brief API event types
 */
enum class ApiEventType : u32 {
    // Lifecycle events
    Initialized = 0,
    ShuttingDown = 1,
    
    // Engine events
    EngineStarted = 100,
    EngineStopped = 101,
    FrameBegin = 102,
    FrameEnd = 103,
    
    // Platform events
    UserLoggedIn = 200,
    UserLoggedOut = 201,
    ConnectionChanged = 202,
    
    // Service events
    CloudSyncStarted = 300,
    CloudSyncCompleted = 301,
    CloudSyncFailed = 302,
    
    // Error events
    ErrorOccurred = 900,
    WarningOccurred = 901,
    
    // Custom events start at 10000
    Custom = 10000
};

/**
 * @brief API event data
 */
struct ApiEvent {
    ApiEventType type;                  ///< Event type
    ApiTimestamp timestamp;             ///< When the event occurred
    std::string message;                ///< Optional message
    std::vector<u8> data;               ///< Optional binary data
    
    /// Create an event
    [[nodiscard]] static ApiEvent create(ApiEventType type, std::string_view message = "") {
        ApiEvent event;
        event.type = type;
        event.timestamp = std::chrono::system_clock::now();
        event.message = std::string(message);
        return event;
    }
};

/// Callback function type for API events
using ApiEventCallback = std::function<void(const ApiEvent&)>;

// =============================================================================
// Resource Types
// =============================================================================

/**
 * @brief Resource identifier
 */
struct ResourceId {
    u64 id;
    std::string path;
    
    [[nodiscard]] bool isValid() const noexcept { return id != 0; }
    [[nodiscard]] bool operator==(const ResourceId& other) const noexcept { return id == other.id; }
    [[nodiscard]] bool operator!=(const ResourceId& other) const noexcept { return id != other.id; }
    
    [[nodiscard]] static ResourceId invalid() { return ResourceId{0, ""}; }
};

/**
 * @brief Resource state
 */
enum class ResourceState : u8 {
    Unloaded = 0,
    Loading = 1,
    Loaded = 2,
    Failed = 3
};

// =============================================================================
// User Types
// =============================================================================

/**
 * @brief User identifier
 */
struct UserId {
    std::string id;
    
    [[nodiscard]] bool isValid() const noexcept { return !id.empty(); }
    [[nodiscard]] bool operator==(const UserId& other) const noexcept { return id == other.id; }
    [[nodiscard]] bool operator!=(const UserId& other) const noexcept { return id != other.id; }
    [[nodiscard]] bool operator<(const UserId& other) const noexcept { return id < other.id; }
    
    [[nodiscard]] static UserId invalid() { return UserId{""}; }
};

/**
 * @brief User profile information
 */
struct UserProfile {
    UserId userId;
    std::string username;
    std::string displayName;
    std::string email;
    std::string avatarUrl;
    std::string bio;
    ApiTimestamp createdAt;
    ApiTimestamp lastLoginAt;
    bool isVerified = false;
    bool isPremium = false;
};

// =============================================================================
// Progress and Status
// =============================================================================

/**
 * @brief Progress information for async operations
 */
struct ApiProgress {
    f32 percentage;         ///< Progress percentage (0-100)
    u64 bytesCompleted;     ///< Bytes completed
    u64 bytesTotal;         ///< Total bytes
    std::string status;     ///< Status message
    
    [[nodiscard]] bool isComplete() const noexcept { return percentage >= 100.0f; }
};

/// Progress callback type
using ApiProgressCallback = std::function<void(const ApiProgress&)>;

// =============================================================================
// Error Codes
// =============================================================================

/**
 * @brief API-specific error codes
 */
enum class ApiErrorCode : u32 {
    Success = 0,
    
    // Initialization errors (1-99)
    NotInitialized = 1,
    AlreadyInitialized = 2,
    InitializationFailed = 3,
    
    // Authentication errors (100-199)
    AuthenticationRequired = 100,
    AuthenticationFailed = 101,
    SessionExpired = 102,
    InvalidCredentials = 103,
    AccountLocked = 104,
    AccountSuspended = 105,
    TokenExpired = 106,
    
    // Network errors (200-299)
    NetworkUnavailable = 200,
    ConnectionFailed = 201,
    ConnectionTimeout = 202,
    ServerUnavailable = 203,
    
    // Resource errors (300-399)
    ResourceNotFound = 300,
    ResourceLoadFailed = 301,
    ResourceInvalid = 302,
    ResourceAccessDenied = 303,
    NotFound = 304,
    AlreadyExists = 305,
    
    // Operation errors (400-499)
    OperationFailed = 400,
    OperationCancelled = 401,
    OperationTimeout = 402,
    InvalidParameter = 403,
    InvalidState = 404,
    InvalidInput = 405,
    Forbidden = 406,
    
    // Platform errors (500-599)
    PlatformError = 500,
    FeatureNotSupported = 501,
    PermissionDenied = 502,
    StorageQuotaExceeded = 503,
    
    // Internal errors (900-999)
    InternalError = 900,
    OutOfMemory = 901,
    UnknownError = 999
};

/// Create an API error
[[nodiscard]] inline Error makeApiError(ApiErrorCode code, std::string_view message) {
    return Error(ErrorCategory::System, static_cast<i32>(code), std::string(message));
}

} // namespace nova::api
