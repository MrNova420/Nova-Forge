// =============================================================================
// NovaCore Engine - Logging System
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// High-performance logging system designed for:
// - Minimal runtime overhead (compile-time filtering)
// - Thread-safe async logging
// - Multiple output targets (console, file, network)
// - Integration with Tracy profiler
// - Mobile-friendly (battery/storage aware)
// =============================================================================

#pragma once

#include <string>
#include <string_view>
#include <format>
#include <source_location>
#include <chrono>
#include <functional>
#include <array>
#include <atomic>
#include <mutex>

#include "nova/core/types/types.hpp"

namespace nova::logging {

// =============================================================================
// Log Levels
// =============================================================================

/// @brief Log severity levels
enum class LogLevel : u8 {
    Trace = 0,    ///< Detailed tracing (disabled in release)
    Debug = 1,    ///< Debug information (disabled in release)
    Info = 2,     ///< General information
    Warning = 3,  ///< Warning conditions
    Error = 4,    ///< Error conditions
    Fatal = 5,    ///< Fatal errors (may terminate)
    Off = 6       ///< Logging disabled
};

/// @brief Convert log level to string
[[nodiscard]] constexpr std::string_view logLevelToString(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Trace:   return "TRACE";
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Fatal:   return "FATAL";
        case LogLevel::Off:     return "OFF";
    }
    return "UNKNOWN";
}

/// @brief Get short form of log level (1 char)
[[nodiscard]] constexpr char logLevelChar(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Trace:   return 'T';
        case LogLevel::Debug:   return 'D';
        case LogLevel::Info:    return 'I';
        case LogLevel::Warning: return 'W';
        case LogLevel::Error:   return 'E';
        case LogLevel::Fatal:   return 'F';
        case LogLevel::Off:     return '-';
    }
    return '?';
}

// =============================================================================
// Log Categories
// =============================================================================

/// @brief Log categories for filtering
enum class LogCategory : u16 {
    Core        = 1 << 0,   ///< Core engine
    Memory      = 1 << 1,   ///< Memory system
    Render      = 1 << 2,   ///< Rendering
    Physics     = 1 << 3,   ///< Physics
    Audio       = 1 << 4,   ///< Audio
    Input       = 1 << 5,   ///< Input handling
    Script      = 1 << 6,   ///< Scripting
    AI          = 1 << 7,   ///< AI systems
    Network     = 1 << 8,   ///< Networking
    Asset       = 1 << 9,   ///< Asset loading
    Editor      = 1 << 10,  ///< Editor
    Platform    = 1 << 11,  ///< Platform layer
    Game        = 1 << 12,  ///< Game logic
    All         = 0xFFFF    ///< All categories
};
NOVA_ENABLE_BITMASK_OPERATORS(LogCategory)

/// @brief Convert category to string
[[nodiscard]] constexpr std::string_view logCategoryToString(LogCategory cat) noexcept {
    switch (cat) {
        case LogCategory::Core:     return "Core";
        case LogCategory::Memory:   return "Memory";
        case LogCategory::Render:   return "Render";
        case LogCategory::Physics:  return "Physics";
        case LogCategory::Audio:    return "Audio";
        case LogCategory::Input:    return "Input";
        case LogCategory::Script:   return "Script";
        case LogCategory::AI:       return "AI";
        case LogCategory::Network:  return "Network";
        case LogCategory::Asset:    return "Asset";
        case LogCategory::Editor:   return "Editor";
        case LogCategory::Platform: return "Platform";
        case LogCategory::Game:     return "Game";
        case LogCategory::All:      return "All";
    }
    return "Unknown";
}

// =============================================================================
// Log Message
// =============================================================================

/// @brief Timestamp type
using Timestamp = std::chrono::system_clock::time_point;

/// @brief Log message structure
struct LogMessage {
    LogLevel level;
    LogCategory category;
    Timestamp timestamp;
    std::string message;
    std::string_view file;
    u32 line;
    std::string_view function;
    u32 threadId;
};

// =============================================================================
// Log Sink Interface
// =============================================================================

/// @brief Interface for log output destinations
class LogSink {
public:
    virtual ~LogSink() = default;
    
    /// @brief Write a log message
    virtual void write(const LogMessage& msg) = 0;
    
    /// @brief Flush any buffered output
    virtual void flush() = 0;
    
    /// @brief Get sink name
    [[nodiscard]] virtual const char* getName() const noexcept = 0;
};

// =============================================================================
// Console Log Sink
// =============================================================================

/// @brief Log sink that outputs to console (stdout/stderr)
class ConsoleSink final : public LogSink {
public:
    /// @brief Configure colored output
    void setColorEnabled(bool enabled) noexcept { m_colorEnabled = enabled; }
    [[nodiscard]] bool isColorEnabled() const noexcept { return m_colorEnabled; }
    
    void write(const LogMessage& msg) override;
    void flush() override;
    [[nodiscard]] const char* getName() const noexcept override { return "Console"; }
    
private:
    bool m_colorEnabled = true;
    std::mutex m_mutex;
};

// =============================================================================
// Logger Class
// =============================================================================

/// @brief Main logger class
/// @note Use the global logging macros instead of direct access
class Logger {
public:
    /// @brief Get singleton instance
    [[nodiscard]] static Logger& instance() noexcept;
    
    /// @brief Initialize logger
    void initialize();
    
    /// @brief Shutdown logger
    void shutdown();
    
    /// @brief Set minimum log level
    void setLevel(LogLevel level) noexcept { m_level.store(level, std::memory_order_relaxed); }
    
    /// @brief Get current log level
    [[nodiscard]] LogLevel getLevel() const noexcept { return m_level.load(std::memory_order_relaxed); }
    
    /// @brief Set category filter
    void setCategoryFilter(Flags<LogCategory> categories) noexcept {
        m_categoryFilter.store(categories.value, std::memory_order_relaxed);
    }
    
    /// @brief Get category filter
    [[nodiscard]] Flags<LogCategory> getCategoryFilter() const noexcept {
        return Flags<LogCategory>{m_categoryFilter.load(std::memory_order_relaxed)};
    }
    
    /// @brief Check if a level/category combo would be logged
    [[nodiscard]] bool shouldLog(LogLevel level, LogCategory category) const noexcept {
        if (level < m_level.load(std::memory_order_relaxed)) return false;
        auto filter = Flags<LogCategory>{m_categoryFilter.load(std::memory_order_relaxed)};
        return filter.has(category) || filter.has(LogCategory::All);
    }
    
    /// @brief Add a log sink
    void addSink(LogSink* sink);
    
    /// @brief Remove a log sink
    void removeSink(LogSink* sink);
    
    /// @brief Log a message
    void log(LogLevel level, LogCategory category, 
             std::string_view message,
             const std::source_location& loc = std::source_location::current());
    
    /// @brief Formatted log message
    template<typename... Args>
    void logFmt(LogLevel level, LogCategory category,
                std::format_string<Args...> fmt, Args&&... args) {
        if (!shouldLog(level, category)) return;
        log(level, category, std::format(fmt, std::forward<Args>(args)...));
    }
    
    /// @brief Flush all sinks
    void flush();
    
private:
    Logger() = default;
    ~Logger() = default;
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    std::atomic<LogLevel> m_level{LogLevel::Info};
    std::atomic<u16> m_categoryFilter{static_cast<u16>(LogCategory::All)};
    std::vector<LogSink*> m_sinks;
    std::mutex m_sinkMutex;
    bool m_initialized{false};
    
    // Default console sink
    ConsoleSink m_consoleSink;
};

// =============================================================================
// Logging Macros
// =============================================================================

// Compile-time log level filtering
#if defined(NOVA_DEBUG) || defined(NOVA_PROFILE)
    #define NOVA_LOG_LEVEL_TRACE_ENABLED 1
    #define NOVA_LOG_LEVEL_DEBUG_ENABLED 1
#else
    #define NOVA_LOG_LEVEL_TRACE_ENABLED 0
    #define NOVA_LOG_LEVEL_DEBUG_ENABLED 0
#endif

/// @brief Log trace message
#if NOVA_LOG_LEVEL_TRACE_ENABLED
    #define NOVA_LOG_TRACE(category, ...) \
        ::nova::logging::Logger::instance().logFmt(::nova::logging::LogLevel::Trace, category, __VA_ARGS__)
#else
    #define NOVA_LOG_TRACE(category, ...) ((void)0)
#endif

/// @brief Log debug message
#if NOVA_LOG_LEVEL_DEBUG_ENABLED
    #define NOVA_LOG_DEBUG(category, ...) \
        ::nova::logging::Logger::instance().logFmt(::nova::logging::LogLevel::Debug, category, __VA_ARGS__)
#else
    #define NOVA_LOG_DEBUG(category, ...) ((void)0)
#endif

/// @brief Log info message
#define NOVA_LOG_INFO(category, ...) \
    ::nova::logging::Logger::instance().logFmt(::nova::logging::LogLevel::Info, category, __VA_ARGS__)

/// @brief Log warning message
#define NOVA_LOG_WARN(category, ...) \
    ::nova::logging::Logger::instance().logFmt(::nova::logging::LogLevel::Warning, category, __VA_ARGS__)

/// @brief Log error message
#define NOVA_LOG_ERROR(category, ...) \
    ::nova::logging::Logger::instance().logFmt(::nova::logging::LogLevel::Error, category, __VA_ARGS__)

/// @brief Log fatal message
#define NOVA_LOG_FATAL(category, ...) \
    ::nova::logging::Logger::instance().logFmt(::nova::logging::LogLevel::Fatal, category, __VA_ARGS__)

// Shorthand macros for common categories
#define NOVA_LOG_CORE_TRACE(...) NOVA_LOG_TRACE(::nova::logging::LogCategory::Core, __VA_ARGS__)
#define NOVA_LOG_CORE_DEBUG(...) NOVA_LOG_DEBUG(::nova::logging::LogCategory::Core, __VA_ARGS__)
#define NOVA_LOG_CORE_INFO(...)  NOVA_LOG_INFO(::nova::logging::LogCategory::Core, __VA_ARGS__)
#define NOVA_LOG_CORE_WARN(...)  NOVA_LOG_WARN(::nova::logging::LogCategory::Core, __VA_ARGS__)
#define NOVA_LOG_CORE_ERROR(...) NOVA_LOG_ERROR(::nova::logging::LogCategory::Core, __VA_ARGS__)

#define NOVA_LOG_RENDER_TRACE(...) NOVA_LOG_TRACE(::nova::logging::LogCategory::Render, __VA_ARGS__)
#define NOVA_LOG_RENDER_DEBUG(...) NOVA_LOG_DEBUG(::nova::logging::LogCategory::Render, __VA_ARGS__)
#define NOVA_LOG_RENDER_INFO(...)  NOVA_LOG_INFO(::nova::logging::LogCategory::Render, __VA_ARGS__)
#define NOVA_LOG_RENDER_WARN(...)  NOVA_LOG_WARN(::nova::logging::LogCategory::Render, __VA_ARGS__)
#define NOVA_LOG_RENDER_ERROR(...) NOVA_LOG_ERROR(::nova::logging::LogCategory::Render, __VA_ARGS__)

} // namespace nova::logging

// Bring common items into nova namespace
namespace nova {
    using logging::LogLevel;
    using logging::LogCategory;
    using logging::Logger;
}
