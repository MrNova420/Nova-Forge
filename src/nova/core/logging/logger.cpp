// =============================================================================
// NovaCore Engine - Logger Implementation
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
// =============================================================================

#include "nova/core/logging/logger.hpp"

#include <iostream>
#include <iomanip>
#include <ctime>
#include <thread>

namespace nova::logging {

// =============================================================================
// ANSI Color Codes
// =============================================================================

namespace colors {
    constexpr const char* RESET     = "\033[0m";
    constexpr const char* BLACK     = "\033[30m";
    constexpr const char* RED       = "\033[31m";
    constexpr const char* GREEN     = "\033[32m";
    constexpr const char* YELLOW    = "\033[33m";
    constexpr const char* BLUE      = "\033[34m";
    constexpr const char* MAGENTA   = "\033[35m";
    constexpr const char* CYAN      = "\033[36m";
    constexpr const char* WHITE     = "\033[37m";
    constexpr const char* BOLD      = "\033[1m";
    constexpr const char* DIM       = "\033[2m";
    
    constexpr const char* BG_RED    = "\033[41m";
    constexpr const char* BG_GREEN  = "\033[42m";
    constexpr const char* BG_YELLOW = "\033[43m";
    constexpr const char* BG_BLUE   = "\033[44m";
}

/// Get color code for log level
[[nodiscard]] const char* getLogLevelColor(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Trace:   return colors::DIM;
        case LogLevel::Debug:   return colors::CYAN;
        case LogLevel::Info:    return colors::GREEN;
        case LogLevel::Warning: return colors::YELLOW;
        case LogLevel::Error:   return colors::RED;
        case LogLevel::Fatal:   return colors::BG_RED;
        default:                return colors::WHITE;
    }
}

// =============================================================================
// Console Sink Implementation
// =============================================================================

void ConsoleSink::write(const LogMessage& msg) {
    std::lock_guard lock(m_mutex);
    
    // Choose output stream
    std::ostream& out = (msg.level >= LogLevel::Error) ? std::cerr : std::cout;
    
    // Format timestamp
    auto timeT = std::chrono::system_clock::to_time_t(msg.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        msg.timestamp.time_since_epoch()) % 1000;
    
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &timeT);
#else
    localtime_r(&timeT, &tm);
#endif
    
    if (m_colorEnabled) {
        // Colored output
        out << colors::DIM
            << std::setfill('0') << std::setw(2) << tm.tm_hour << ":"
            << std::setw(2) << tm.tm_min << ":"
            << std::setw(2) << tm.tm_sec << "."
            << std::setw(3) << ms.count()
            << colors::RESET << " ";
        
        out << getLogLevelColor(msg.level)
            << "[" << logLevelChar(msg.level) << "]"
            << colors::RESET << " ";
        
        out << colors::BLUE
            << "[" << logCategoryToString(msg.category) << "]"
            << colors::RESET << " ";
        
        out << msg.message;
        
        // Add file/line for warnings and above
        if (msg.level >= LogLevel::Warning && !msg.file.empty()) {
            out << colors::DIM
                << " (" << msg.file << ":" << msg.line << ")"
                << colors::RESET;
        }
        
        out << "\n";
    } else {
        // Plain output
        out << std::setfill('0') << std::setw(2) << tm.tm_hour << ":"
            << std::setw(2) << tm.tm_min << ":"
            << std::setw(2) << tm.tm_sec << "."
            << std::setw(3) << ms.count() << " ";
        
        out << "[" << logLevelChar(msg.level) << "] ";
        out << "[" << logCategoryToString(msg.category) << "] ";
        out << msg.message;
        
        if (msg.level >= LogLevel::Warning && !msg.file.empty()) {
            out << " (" << msg.file << ":" << msg.line << ")";
        }
        
        out << "\n";
    }
}

void ConsoleSink::flush() {
    std::lock_guard lock(m_mutex);
    std::cout.flush();
    std::cerr.flush();
}

// =============================================================================
// Logger Implementation
// =============================================================================

Logger& Logger::instance() noexcept {
    static Logger s_instance;
    return s_instance;
}

void Logger::initialize() {
    if (m_initialized) return;
    
    std::lock_guard lock(m_sinkMutex);
    
    // Add default console sink
    m_sinks.push_back(&m_consoleSink);
    
    m_initialized = true;
    
    log(LogLevel::Info, LogCategory::Core, "NovaCore Logger initialized");
}

void Logger::shutdown() {
    if (!m_initialized) return;
    
    log(LogLevel::Info, LogCategory::Core, "NovaCore Logger shutting down");
    flush();
    
    std::lock_guard lock(m_sinkMutex);
    m_sinks.clear();
    m_initialized = false;
}

void Logger::addSink(LogSink* sink) {
    if (!sink) return;
    
    std::lock_guard lock(m_sinkMutex);
    m_sinks.push_back(sink);
}

void Logger::removeSink(LogSink* sink) {
    if (!sink) return;
    
    std::lock_guard lock(m_sinkMutex);
    m_sinks.erase(
        std::remove(m_sinks.begin(), m_sinks.end(), sink),
        m_sinks.end()
    );
}

void Logger::log(LogLevel level, LogCategory category, 
                 std::string_view message,
                 const std::source_location& loc) {
    if (!shouldLog(level, category)) return;
    
    // Build log message
    LogMessage msg{};
    msg.level = level;
    msg.category = category;
    msg.timestamp = std::chrono::system_clock::now();
    msg.message = std::string(message);
    msg.file = loc.file_name();
    msg.line = loc.line();
    msg.function = loc.function_name();
    
    // Get thread ID (simplified)
    std::hash<std::thread::id> hasher;
    msg.threadId = static_cast<u32>(hasher(std::this_thread::get_id()) & 0xFFFF);
    
    // Write to all sinks
    {
        std::lock_guard lock(m_sinkMutex);
        for (auto* sink : m_sinks) {
            sink->write(msg);
        }
    }
    
    // Auto-flush for errors and fatal
    if (level >= LogLevel::Error) {
        flush();
    }
}

void Logger::flush() {
    std::lock_guard lock(m_sinkMutex);
    for (auto* sink : m_sinks) {
        sink->flush();
    }
}

} // namespace nova::logging
