// =============================================================================
// NovaForge Platform - API System
// =============================================================================
// Platform: NovaForge | Engine: NovaCore | Company: WeNova Interactive
// (operating as Kayden Shawn Massengill)
//
// The NovaForge API provides a unified interface for accessing all platform
// and engine functionality. This is the primary entry point for external
// applications, plugins, and services to interact with NovaForge.
//
// Security: All API access is authenticated and authorized through the
// security module. Rate limiting and request signing are enforced.
// =============================================================================

#pragma once

#include "nova/api/api_types.hpp"
#include "nova/api/api_engine.hpp"
#include "nova/api/api_platform.hpp"
#include "nova/api/api_services.hpp"
#include "nova/api/api_security.hpp"

namespace nova::api {

// =============================================================================
// API Version Information
// =============================================================================

/// NovaForge API major version
inline constexpr u32 API_VERSION_MAJOR = 1;

/// NovaForge API minor version
inline constexpr u32 API_VERSION_MINOR = 0;

/// NovaForge API patch version
inline constexpr u32 API_VERSION_PATCH = 0;

/// Full API version as string
inline constexpr const char* API_VERSION_STRING = "1.0.0";

/// Get API version as packed integer (major << 16 | minor << 8 | patch)
[[nodiscard]] constexpr u32 getApiVersion() noexcept {
    return (API_VERSION_MAJOR << 16) | (API_VERSION_MINOR << 8) | API_VERSION_PATCH;
}

/// Check if API version is compatible (same major version)
[[nodiscard]] constexpr bool isApiVersionCompatible(u32 version) noexcept {
    return (version >> 16) == API_VERSION_MAJOR;
}

// =============================================================================
// NovaForge API - Main Entry Point
// =============================================================================

/**
 * @brief The main NovaForge API class
 * 
 * This singleton provides access to all NovaForge functionality including:
 * - Engine systems (rendering, physics, audio, etc.)
 * - Platform services (user accounts, cloud storage, analytics)
 * - Editor tools
 * - Plugin management
 * 
 * @example
 * ```cpp
 * // Initialize the API
 * auto& api = nova::api::NovaForgeApi::get();
 * api.initialize();
 * 
 * // Access engine functionality
 * auto& engine = api.getEngine();
 * engine.createWindow("My Game", 1920, 1080);
 * 
 * // Access platform services
 * auto& platform = api.getPlatform();
 * platform.authenticate(credentials);
 * 
 * // Shutdown
 * api.shutdown();
 * ```
 */
class NovaForgeApi {
public:
    /// Get the singleton instance
    [[nodiscard]] static NovaForgeApi& get() noexcept;
    
    // Non-copyable, non-movable
    NovaForgeApi(const NovaForgeApi&) = delete;
    NovaForgeApi& operator=(const NovaForgeApi&) = delete;
    NovaForgeApi(NovaForgeApi&&) = delete;
    NovaForgeApi& operator=(NovaForgeApi&&) = delete;
    
    // =========================================================================
    // Lifecycle
    // =========================================================================
    
    /**
     * @brief Initialize the NovaForge API
     * @param config Optional configuration settings
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult initialize(const ApiConfig& config = ApiConfig{});
    
    /**
     * @brief Shutdown the NovaForge API
     * 
     * Releases all resources and shuts down all subsystems.
     * Call this before application exit.
     */
    void shutdown();
    
    /**
     * @brief Check if API is initialized
     * @return true if initialized, false otherwise
     */
    [[nodiscard]] bool isInitialized() const noexcept;
    
    // =========================================================================
    // Subsystem Access
    // =========================================================================
    
    /**
     * @brief Get the engine API
     * @return Reference to the engine API
     */
    [[nodiscard]] EngineApi& getEngine() noexcept;
    [[nodiscard]] const EngineApi& getEngine() const noexcept;
    
    /**
     * @brief Get the platform API
     * @return Reference to the platform API
     */
    [[nodiscard]] PlatformApi& getPlatform() noexcept;
    [[nodiscard]] const PlatformApi& getPlatform() const noexcept;
    
    /**
     * @brief Get the services API
     * @return Reference to the services API
     */
    [[nodiscard]] ServicesApi& getServices() noexcept;
    [[nodiscard]] const ServicesApi& getServices() const noexcept;
    
    // =========================================================================
    // Utility Functions
    // =========================================================================
    
    /**
     * @brief Get API version information
     * @return Version struct with major, minor, patch
     */
    [[nodiscard]] ApiVersion getVersion() const noexcept;
    
    /**
     * @brief Get platform information
     * @return Platform info struct
     */
    [[nodiscard]] ApiPlatformInfo getPlatformInfo() const noexcept;
    
    /**
     * @brief Set global log level
     * @param level The log level to set
     */
    void setLogLevel(ApiLogLevel level) noexcept;
    
    /**
     * @brief Register a callback for API events
     * @param callback The callback function
     * @return Handle to unregister the callback
     */
    [[nodiscard]] ApiCallbackHandle registerCallback(ApiEventCallback callback);
    
    /**
     * @brief Unregister a callback
     * @param handle The handle returned from registerCallback
     */
    void unregisterCallback(ApiCallbackHandle handle);
    
private:
    NovaForgeApi();
    ~NovaForgeApi();
    
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

// =============================================================================
// Convenience Functions
// =============================================================================

/// Initialize the NovaForge API with default configuration
[[nodiscard]] inline ApiResult initializeApi() {
    return NovaForgeApi::get().initialize();
}

/// Initialize the NovaForge API with custom configuration
[[nodiscard]] inline ApiResult initializeApi(const ApiConfig& config) {
    return NovaForgeApi::get().initialize(config);
}

/// Shutdown the NovaForge API
inline void shutdownApi() {
    NovaForgeApi::get().shutdown();
}

/// Get the engine API
[[nodiscard]] inline EngineApi& engine() {
    return NovaForgeApi::get().getEngine();
}

/// Get the platform API
[[nodiscard]] inline PlatformApi& platform() {
    return NovaForgeApi::get().getPlatform();
}

/// Get the services API
[[nodiscard]] inline ServicesApi& services() {
    return NovaForgeApi::get().getServices();
}

} // namespace nova::api
