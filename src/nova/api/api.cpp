// =============================================================================
// NovaForge Platform - API Implementation
// =============================================================================
// Platform: NovaForge | Engine: NovaCore | Company: WeNova Interactive
// (operating as Kayden Shawn Massengill)
// =============================================================================

#include <nova/api/api.hpp>

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
    
    // TODO: Populate with actual platform info
    #if defined(_WIN32)
    info.osName = "Windows";
    #elif defined(__APPLE__)
    info.osName = "macOS";
    #elif defined(__linux__)
    info.osName = "Linux";
    #elif defined(__ANDROID__)
    info.osName = "Android";
    #else
    info.osName = "Unknown";
    #endif
    
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
