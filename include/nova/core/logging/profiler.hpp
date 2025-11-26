// =============================================================================
// NovaCore Engine - Profiling System
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// Performance profiling system with:
// - Scoped timing markers
// - Memory tracking
// - GPU timing (when available)
// - Tracy profiler integration (optional)
// - Minimal overhead when disabled
// =============================================================================

#pragma once

#include <chrono>
#include <string_view>

#include "nova/core/types/types.hpp"

namespace nova::profiling {

// =============================================================================
// Profiling Configuration
// =============================================================================

// Enable profiling in debug/profile builds
#if defined(NOVA_DEBUG) || defined(NOVA_PROFILE)
    #define NOVA_PROFILING_ENABLED 1
#else
    #define NOVA_PROFILING_ENABLED 0
#endif

// Tracy integration (optional)
#if defined(NOVA_TRACY_ENABLED) && NOVA_PROFILING_ENABLED
    #define NOVA_TRACY_INTEGRATION 1
    // Note: Tracy headers would be included here when available
    // #include <tracy/Tracy.hpp>
#else
    #define NOVA_TRACY_INTEGRATION 0
#endif

// =============================================================================
// High-Resolution Timer
// =============================================================================

/// @brief High-resolution timestamp
using Timestamp = std::chrono::high_resolution_clock::time_point;

/// @brief Get current timestamp
[[nodiscard]] inline Timestamp now() noexcept {
    return std::chrono::high_resolution_clock::now();
}

/// @brief Calculate duration in nanoseconds
[[nodiscard]] inline i64 durationNs(Timestamp start, Timestamp end) noexcept {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

/// @brief Calculate duration in microseconds
[[nodiscard]] inline f64 durationUs(Timestamp start, Timestamp end) noexcept {
    return static_cast<f64>(durationNs(start, end)) / 1000.0;
}

/// @brief Calculate duration in milliseconds
[[nodiscard]] inline f64 durationMs(Timestamp start, Timestamp end) noexcept {
    return static_cast<f64>(durationNs(start, end)) / 1000000.0;
}

// =============================================================================
// Profiling Zone
// =============================================================================

/// @brief Color type for profiling zones
struct ZoneColor {
    u8 r, g, b;
    
    static constexpr ZoneColor red()     { return {255, 100, 100}; }
    static constexpr ZoneColor green()   { return {100, 255, 100}; }
    static constexpr ZoneColor blue()    { return {100, 100, 255}; }
    static constexpr ZoneColor yellow()  { return {255, 255, 100}; }
    static constexpr ZoneColor cyan()    { return {100, 255, 255}; }
    static constexpr ZoneColor magenta() { return {255, 100, 255}; }
    static constexpr ZoneColor orange()  { return {255, 200, 100}; }
    static constexpr ZoneColor purple()  { return {200, 100, 255}; }
    static constexpr ZoneColor gray()    { return {180, 180, 180}; }
    static constexpr ZoneColor white()   { return {255, 255, 255}; }
    
    // Category colors
    static constexpr ZoneColor core()    { return orange(); }
    static constexpr ZoneColor render()  { return green(); }
    static constexpr ZoneColor physics() { return blue(); }
    static constexpr ZoneColor audio()   { return cyan(); }
    static constexpr ZoneColor ai()      { return magenta(); }
    static constexpr ZoneColor network() { return yellow(); }
    static constexpr ZoneColor asset()   { return purple(); }
};

#if NOVA_PROFILING_ENABLED

/// @brief Scoped profiling zone
class ScopedZone {
public:
    /// @brief Start a profiling zone
    explicit ScopedZone(std::string_view name, ZoneColor color = ZoneColor::gray()) noexcept
        : m_name(name)
        , m_color(color)
        , m_start(now())
    {
#if NOVA_TRACY_INTEGRATION
        // Tracy zone start would go here
#endif
    }
    
    /// @brief End profiling zone
    ~ScopedZone() {
        m_end = now();
        
#if NOVA_TRACY_INTEGRATION
        // Tracy zone end would go here
#endif
        
        // Could log timing data here
        (void)m_name;
        (void)m_color;
    }
    
    /// @brief Get elapsed time so far
    [[nodiscard]] f64 elapsedMs() const noexcept {
        return durationMs(m_start, now());
    }
    
    // Non-copyable
    ScopedZone(const ScopedZone&) = delete;
    ScopedZone& operator=(const ScopedZone&) = delete;
    
private:
    std::string_view m_name;
    ZoneColor m_color;
    Timestamp m_start;
    Timestamp m_end;
};

/// @brief Manual timing for non-scoped measurements
class Timer {
public:
    Timer() noexcept : m_start(now()), m_running(true) {}
    
    void start() noexcept {
        m_start = now();
        m_running = true;
    }
    
    void stop() noexcept {
        if (m_running) {
            m_end = now();
            m_running = false;
        }
    }
    
    void reset() noexcept {
        m_start = now();
        m_running = true;
    }
    
    [[nodiscard]] f64 elapsedNs() const noexcept {
        Timestamp end = m_running ? now() : m_end;
        return static_cast<f64>(durationNs(m_start, end));
    }
    
    [[nodiscard]] f64 elapsedUs() const noexcept {
        return elapsedNs() / 1000.0;
    }
    
    [[nodiscard]] f64 elapsedMs() const noexcept {
        return elapsedNs() / 1000000.0;
    }
    
    [[nodiscard]] f64 elapsedSec() const noexcept {
        return elapsedNs() / 1000000000.0;
    }
    
    [[nodiscard]] bool isRunning() const noexcept {
        return m_running;
    }
    
private:
    Timestamp m_start;
    Timestamp m_end;
    bool m_running;
};

#else // NOVA_PROFILING_ENABLED == 0

// Stub implementations when profiling is disabled
class ScopedZone {
public:
    explicit ScopedZone([[maybe_unused]] std::string_view name, 
                       [[maybe_unused]] ZoneColor color = ZoneColor::gray()) noexcept {}
    [[nodiscard]] f64 elapsedMs() const noexcept { return 0.0; }
};

class Timer {
public:
    Timer() noexcept = default;
    void start() noexcept {}
    void stop() noexcept {}
    void reset() noexcept {}
    [[nodiscard]] f64 elapsedNs() const noexcept { return 0.0; }
    [[nodiscard]] f64 elapsedUs() const noexcept { return 0.0; }
    [[nodiscard]] f64 elapsedMs() const noexcept { return 0.0; }
    [[nodiscard]] f64 elapsedSec() const noexcept { return 0.0; }
    [[nodiscard]] bool isRunning() const noexcept { return false; }
};

#endif // NOVA_PROFILING_ENABLED

// =============================================================================
// Frame Timing
// =============================================================================

/// @brief Frame timing statistics
struct FrameStats {
    f64 frameTimeMs;        ///< Total frame time
    f64 updateTimeMs;       ///< Update/logic time
    f64 renderTimeMs;       ///< Render time
    f64 presentTimeMs;      ///< Present/swap time
    u64 frameCount;         ///< Total frame count
    
    f64 avgFrameTimeMs;     ///< Average frame time (rolling)
    f64 minFrameTimeMs;     ///< Minimum frame time (rolling)
    f64 maxFrameTimeMs;     ///< Maximum frame time (rolling)
    f64 fps;                ///< Frames per second
    
    void reset() noexcept {
        frameTimeMs = 0.0;
        updateTimeMs = 0.0;
        renderTimeMs = 0.0;
        presentTimeMs = 0.0;
        frameCount = 0;
        avgFrameTimeMs = 0.0;
        minFrameTimeMs = 999999.0;
        maxFrameTimeMs = 0.0;
        fps = 0.0;
    }
};

/// @brief Frame timer for tracking frame statistics
class FrameTimer {
public:
    FrameTimer() noexcept {
        m_stats.reset();
        m_frameStart = now();
    }
    
    /// @brief Call at start of frame
    void beginFrame() noexcept {
        m_frameStart = now();
    }
    
    /// @brief Call at end of frame
    void endFrame() noexcept {
        Timestamp frameEnd = now();
        m_stats.frameTimeMs = durationMs(m_frameStart, frameEnd);
        m_stats.frameCount++;
        
        // Update rolling stats
        updateRollingStats();
    }
    
    /// @brief Get current frame stats
    [[nodiscard]] const FrameStats& getStats() const noexcept {
        return m_stats;
    }
    
    /// @brief Get current FPS
    [[nodiscard]] f64 getFps() const noexcept {
        return m_stats.fps;
    }
    
    /// @brief Get last frame time in milliseconds
    [[nodiscard]] f64 getFrameTimeMs() const noexcept {
        return m_stats.frameTimeMs;
    }
    
private:
    void updateRollingStats() noexcept {
        // Simple exponential moving average
        constexpr f64 alpha = 0.1;
        
        if (m_stats.frameCount == 1) {
            m_stats.avgFrameTimeMs = m_stats.frameTimeMs;
        } else {
            m_stats.avgFrameTimeMs = alpha * m_stats.frameTimeMs + (1.0 - alpha) * m_stats.avgFrameTimeMs;
        }
        
        if (m_stats.frameTimeMs < m_stats.minFrameTimeMs) {
            m_stats.minFrameTimeMs = m_stats.frameTimeMs;
        }
        if (m_stats.frameTimeMs > m_stats.maxFrameTimeMs) {
            m_stats.maxFrameTimeMs = m_stats.frameTimeMs;
        }
        
        // Calculate FPS from average frame time
        if (m_stats.avgFrameTimeMs > 0.0) {
            m_stats.fps = 1000.0 / m_stats.avgFrameTimeMs;
        }
    }
    
    FrameStats m_stats;
    Timestamp m_frameStart;
};

// =============================================================================
// Profiling Macros
// =============================================================================

#if NOVA_PROFILING_ENABLED
    /// @brief Create a scoped profiling zone
    #define NOVA_PROFILE_ZONE(name) \
        ::nova::profiling::ScopedZone _nova_zone_##__LINE__(name)
    
    /// @brief Create a scoped profiling zone with color
    #define NOVA_PROFILE_ZONE_COLOR(name, color) \
        ::nova::profiling::ScopedZone _nova_zone_##__LINE__(name, color)
    
    /// @brief Profile the current function
    #define NOVA_PROFILE_FUNCTION() \
        ::nova::profiling::ScopedZone _nova_zone_func(__func__)
    
    /// @brief Category-specific profiling zones
    #define NOVA_PROFILE_CORE(name)    NOVA_PROFILE_ZONE_COLOR(name, ::nova::profiling::ZoneColor::core())
    #define NOVA_PROFILE_RENDER(name)  NOVA_PROFILE_ZONE_COLOR(name, ::nova::profiling::ZoneColor::render())
    #define NOVA_PROFILE_PHYSICS(name) NOVA_PROFILE_ZONE_COLOR(name, ::nova::profiling::ZoneColor::physics())
    #define NOVA_PROFILE_AUDIO(name)   NOVA_PROFILE_ZONE_COLOR(name, ::nova::profiling::ZoneColor::audio())
    #define NOVA_PROFILE_AI(name)      NOVA_PROFILE_ZONE_COLOR(name, ::nova::profiling::ZoneColor::ai())
    #define NOVA_PROFILE_NETWORK(name) NOVA_PROFILE_ZONE_COLOR(name, ::nova::profiling::ZoneColor::network())
    #define NOVA_PROFILE_ASSET(name)   NOVA_PROFILE_ZONE_COLOR(name, ::nova::profiling::ZoneColor::asset())
    
#else
    #define NOVA_PROFILE_ZONE(name) ((void)0)
    #define NOVA_PROFILE_ZONE_COLOR(name, color) ((void)0)
    #define NOVA_PROFILE_FUNCTION() ((void)0)
    #define NOVA_PROFILE_CORE(name) ((void)0)
    #define NOVA_PROFILE_RENDER(name) ((void)0)
    #define NOVA_PROFILE_PHYSICS(name) ((void)0)
    #define NOVA_PROFILE_AUDIO(name) ((void)0)
    #define NOVA_PROFILE_AI(name) ((void)0)
    #define NOVA_PROFILE_NETWORK(name) ((void)0)
    #define NOVA_PROFILE_ASSET(name) ((void)0)
#endif

} // namespace nova::profiling

// Bring common items into nova namespace
namespace nova {
    using profiling::Timer;
    using profiling::FrameTimer;
    using profiling::FrameStats;
    using profiling::ScopedZone;
}
