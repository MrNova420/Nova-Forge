// =============================================================================
// NovaForge Platform - API Implementation
// =============================================================================
// Platform: NovaForge | Engine: NovaCore | Company: WeNova Interactive
// (operating as Kayden Shawn Massengill)
// =============================================================================

#include <nova/api/api.hpp>
#include <thread>

namespace nova::api {

// =============================================================================
// NovaForgeApi Implementation
// =============================================================================

struct NovaForgeApi::Impl {
    bool initialized = false;
    ApiConfig config;
    
    std::unique_ptr<EngineApi> engine;
    std::unique_ptr<PlatformApi> platform;
    std::unique_ptr<ServicesApi> services;
    
    std::vector<std::pair<ApiCallbackHandle, ApiEventCallback>> callbacks;
    ApiCallbackHandle nextCallbackHandle = 1;
    
    void fireEvent(const ApiEvent& event) {
        for (const auto& [handle, callback] : callbacks) {
            if (callback) {
                callback(event);
            }
        }
    }
};

NovaForgeApi& NovaForgeApi::get() noexcept {
    static NovaForgeApi instance;
    return instance;
}

NovaForgeApi::NovaForgeApi() : m_impl(std::make_unique<Impl>()) {}

NovaForgeApi::~NovaForgeApi() {
    if (m_impl && m_impl->initialized) {
        shutdown();
    }
}

ApiResult NovaForgeApi::initialize(const ApiConfig& config) {
    if (m_impl->initialized) {
        return std::unexpected(makeApiError(
            ApiErrorCode::AlreadyInitialized,
            "NovaForge API is already initialized"));
    }
    
    m_impl->config = config;
    
    // Initialize subsystems
    m_impl->engine = std::make_unique<EngineApi>();
    m_impl->platform = std::make_unique<PlatformApi>();
    m_impl->services = std::make_unique<ServicesApi>();
    
    // Initialize security
    security::SecurityManager::Config secConfig;
    if (config.debugMode) {
        secConfig.level = security::SecurityLevel::Low;
    }
    auto secResult = security::SecurityManager::get().initialize(secConfig);
    if (!secResult) {
        return std::unexpected(makeApiError(
            ApiErrorCode::InitializationFailed,
            "Failed to initialize security"));
    }
    
    m_impl->initialized = true;
    
    // Fire initialized event
    m_impl->fireEvent(ApiEvent::create(ApiEventType::Initialized, "NovaForge API initialized"));
    
    return {};
}

void NovaForgeApi::shutdown() {
    if (!m_impl->initialized) return;
    
    // Fire shutting down event
    m_impl->fireEvent(ApiEvent::create(ApiEventType::ShuttingDown, "NovaForge API shutting down"));
    
    // Shutdown security
    security::SecurityManager::get().shutdown();
    
    // Shutdown subsystems
    m_impl->services.reset();
    m_impl->platform.reset();
    m_impl->engine.reset();
    
    m_impl->callbacks.clear();
    m_impl->initialized = false;
}

bool NovaForgeApi::isInitialized() const noexcept {
    return m_impl->initialized;
}

EngineApi& NovaForgeApi::getEngine() noexcept {
    return *m_impl->engine;
}

const EngineApi& NovaForgeApi::getEngine() const noexcept {
    return *m_impl->engine;
}

PlatformApi& NovaForgeApi::getPlatform() noexcept {
    return *m_impl->platform;
}

const PlatformApi& NovaForgeApi::getPlatform() const noexcept {
    return *m_impl->platform;
}

ServicesApi& NovaForgeApi::getServices() noexcept {
    return *m_impl->services;
}

const ServicesApi& NovaForgeApi::getServices() const noexcept {
    return *m_impl->services;
}

ApiVersion NovaForgeApi::getVersion() const noexcept {
    return ApiVersion{API_VERSION_MAJOR, API_VERSION_MINOR, API_VERSION_PATCH};
}

ApiPlatformInfo NovaForgeApi::getPlatformInfo() const noexcept {
    ApiPlatformInfo info;
    
    // =========================================================================
    // Operating System Detection - Full Implementation
    // =========================================================================
    #if defined(_WIN32) || defined(_WIN64)
    info.osName = "Windows";
    info.osVersion = "10+";
    #elif defined(__APPLE__)
        #include <TargetConditionals.h>
        #if TARGET_OS_IOS
        info.osName = "iOS";
        info.osVersion = "14+";
        #elif TARGET_OS_MAC
        info.osName = "macOS";
        info.osVersion = "12+";
        #else
        info.osName = "Apple";
        info.osVersion = "Unknown";
        #endif
    #elif defined(__ANDROID__)
    info.osName = "Android";
    info.osVersion = "6.0+";
    #elif defined(__linux__)
    info.osName = "Linux";
    info.osVersion = "5.0+";
    #elif defined(__EMSCRIPTEN__)
    info.osName = "Web";
    info.osVersion = "WebAssembly";
    #else
    info.osName = "Unknown";
    info.osVersion = "Unknown";
    #endif
    
    // =========================================================================
    // Architecture Detection
    // =========================================================================
    #if defined(__x86_64__) || defined(_M_X64)
    info.architecture = "x86_64";
    info.is64Bit = true;
    #elif defined(__i386__) || defined(_M_IX86)
    info.architecture = "x86";
    info.is64Bit = false;
    #elif defined(__aarch64__) || defined(_M_ARM64)
    info.architecture = "ARM64";
    info.is64Bit = true;
    #elif defined(__arm__) || defined(_M_ARM)
    info.architecture = "ARM32";
    info.is64Bit = false;
    #elif defined(__wasm64__)
    info.architecture = "WASM64";
    info.is64Bit = true;
    #elif defined(__wasm__)
    info.architecture = "WASM32";
    info.is64Bit = false;
    #else
    info.architecture = "Unknown";
    info.is64Bit = sizeof(void*) == 8;
    #endif
    
    // =========================================================================
    // SIMD Capabilities Detection
    // =========================================================================
    #if defined(__AVX512F__)
    info.simdSupport = "AVX-512";
    info.cpuHasAVX512 = true;
    info.cpuHasAVX2 = true;
    info.cpuHasAVX = true;
    info.cpuHasSSE42 = true;
    info.cpuHasSSE41 = true;
    info.cpuHasSSE3 = true;
    info.cpuHasSSE2 = true;
    info.cpuHasSSE = true;
    info.supportsSIMD = true;
    #elif defined(__AVX2__)
    info.simdSupport = "AVX2";
    info.cpuHasAVX2 = true;
    info.cpuHasAVX = true;
    info.cpuHasSSE42 = true;
    info.cpuHasSSE41 = true;
    info.cpuHasSSE3 = true;
    info.cpuHasSSE2 = true;
    info.cpuHasSSE = true;
    info.supportsSIMD = true;
    #elif defined(__AVX__)
    info.simdSupport = "AVX";
    info.cpuHasAVX = true;
    info.cpuHasSSE42 = true;
    info.cpuHasSSE41 = true;
    info.cpuHasSSE3 = true;
    info.cpuHasSSE2 = true;
    info.cpuHasSSE = true;
    info.supportsSIMD = true;
    #elif defined(__SSE4_2__)
    info.simdSupport = "SSE4.2";
    info.cpuHasSSE42 = true;
    info.cpuHasSSE41 = true;
    info.cpuHasSSE3 = true;
    info.cpuHasSSE2 = true;
    info.cpuHasSSE = true;
    info.supportsSIMD = true;
    #elif defined(__SSE4_1__)
    info.simdSupport = "SSE4.1";
    info.cpuHasSSE41 = true;
    info.cpuHasSSE3 = true;
    info.cpuHasSSE2 = true;
    info.cpuHasSSE = true;
    info.supportsSIMD = true;
    #elif defined(__SSE3__)
    info.simdSupport = "SSE3";
    info.cpuHasSSE3 = true;
    info.cpuHasSSE2 = true;
    info.cpuHasSSE = true;
    info.supportsSIMD = true;
    #elif defined(__SSE2__)
    info.simdSupport = "SSE2";
    info.cpuHasSSE2 = true;
    info.cpuHasSSE = true;
    info.supportsSIMD = true;
    #elif defined(__ARM_NEON) || defined(__ARM_NEON__)
    info.simdSupport = "NEON";
    info.cpuHasNEON = true;
    info.supportsSIMD = true;
    #elif defined(__ARM_FEATURE_SVE)
    info.simdSupport = "SVE";
    info.cpuHasSVE = true;
    info.cpuHasNEON = true;
    info.supportsSIMD = true;
    #else
    info.simdSupport = "None";
    info.supportsSIMD = false;
    #endif
    
    // =========================================================================
    // Build Configuration
    // =========================================================================
    #if defined(NDEBUG)
    info.buildType = "Release";
    #elif defined(RELWITHDEBINFO)
    info.buildType = "RelWithDebInfo";
    #else
    info.buildType = "Debug";
    #endif
    
    // C++ Standard
    #if __cplusplus >= 202302L
    info.cppStandard = "C++23";
    #elif __cplusplus >= 202002L
    info.cppStandard = "C++20";
    #elif __cplusplus >= 201703L
    info.cppStandard = "C++17";
    #elif __cplusplus >= 201402L
    info.cppStandard = "C++14";
    #else
    info.cppStandard = "C++11";
    #endif
    
    // Compiler Detection
    #if defined(__clang__)
    info.compiler = "Clang";
    info.compilerVersion = std::to_string(__clang_major__) + "." + 
                           std::to_string(__clang_minor__) + "." +
                           std::to_string(__clang_patchlevel__);
    #elif defined(__GNUC__)
    info.compiler = "GCC";
    info.compilerVersion = std::to_string(__GNUC__) + "." + 
                           std::to_string(__GNUC_MINOR__) + "." +
                           std::to_string(__GNUC_PATCHLEVEL__);
    #elif defined(_MSC_VER)
    info.compiler = "MSVC";
    info.compilerVersion = std::to_string(_MSC_VER);
    #elif defined(__EMSCRIPTEN__)
    info.compiler = "Emscripten";
    info.compilerVersion = std::to_string(__EMSCRIPTEN_major__) + "." +
                           std::to_string(__EMSCRIPTEN_minor__);
    #else
    info.compiler = "Unknown";
    info.compilerVersion = "Unknown";
    #endif
    
    // =========================================================================
    // Hardware Thread Count (portable)
    // =========================================================================
    info.cpuThreads = std::thread::hardware_concurrency();
    if (info.cpuThreads == 0) {
        info.cpuThreads = 1;  // Fallback
    }
    // Estimate physical cores (heuristic: assume SMT/HT gives 2 threads per core)
    info.cpuCores = (info.cpuThreads + 1) / 2;
    
    // =========================================================================
    // Default Capabilities
    // =========================================================================
    info.supportsMultithreading = (info.cpuThreads > 1);
    info.supportsAsyncIO = true;
    info.supportsHardwareAccel = info.supportsSIMD;
    
    // Platform-specific capabilities
    #if defined(__ANDROID__)
    info.deviceType = "Mobile";
    info.hasBattery = true;
    info.supportsVibration = true;
    info.supportsCamera = true;
    info.supportsMicrophone = true;
    info.supportsGPS = true;
    info.supportsAccelerometer = true;
    info.supportsGyroscope = true;
    info.supportsBluetooth = true;
    #elif defined(__APPLE__) && TARGET_OS_IOS
    info.deviceType = "Mobile";
    info.hasBattery = true;
    info.supportsVibration = true;
    info.supportsCamera = true;
    info.supportsMicrophone = true;
    info.supportsGPS = true;
    info.supportsAccelerometer = true;
    info.supportsGyroscope = true;
    info.supportsBluetooth = true;
    info.supportsNFC = true;
    #elif defined(__APPLE__)
    info.deviceType = "Desktop";
    info.hasBattery = true;  // MacBooks
    info.gpuSupportsMetal = true;
    #elif defined(_WIN32)
    info.deviceType = "Desktop";
    info.gpuSupportsD3D12 = true;
    info.gpuSupportsVulkan = true;
    #elif defined(__linux__)
    info.deviceType = "Desktop";
    info.gpuSupportsVulkan = true;
    #elif defined(__EMSCRIPTEN__)
    info.deviceType = "Web";
    info.gpuSupportsWebGPU = true;
    #else
    info.deviceType = "Unknown";
    #endif
    
    // Default display info (would be populated by windowing system)
    info.displayCount = 1;
    info.primaryDisplayRefreshHz = 60;
    info.primaryDisplayDPI = 96.0f;
    info.primaryDisplayScale = 1.0f;
    
    // Default memory page size
    info.pageSize = 4096;  // Common default
    
    return info;
}

void NovaForgeApi::setLogLevel(ApiLogLevel level) noexcept {
    m_impl->config.logLevel = level;
}

ApiCallbackHandle NovaForgeApi::registerCallback(ApiEventCallback callback) {
    ApiCallbackHandle handle = m_impl->nextCallbackHandle++;
    m_impl->callbacks.emplace_back(handle, std::move(callback));
    return handle;
}

void NovaForgeApi::unregisterCallback(ApiCallbackHandle handle) {
    auto it = std::remove_if(m_impl->callbacks.begin(), m_impl->callbacks.end(),
        [handle](const auto& pair) { return pair.first == handle; });
    m_impl->callbacks.erase(it, m_impl->callbacks.end());
}

} // namespace nova::api
