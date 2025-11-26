// =============================================================================
// NovaCore Engine - Logging System Tests
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <sstream>
#include <thread>

#include "nova/core/logging/logging.hpp"

using namespace nova;
using namespace nova::logging;
using namespace nova::profiling;

// =============================================================================
// Log Level Tests
// =============================================================================

TEST_CASE("Log level string conversion", "[logging][level]") {
    REQUIRE(logLevelToString(LogLevel::Trace) == "TRACE");
    REQUIRE(logLevelToString(LogLevel::Debug) == "DEBUG");
    REQUIRE(logLevelToString(LogLevel::Info) == "INFO");
    REQUIRE(logLevelToString(LogLevel::Warning) == "WARN");
    REQUIRE(logLevelToString(LogLevel::Error) == "ERROR");
    REQUIRE(logLevelToString(LogLevel::Fatal) == "FATAL");
    REQUIRE(logLevelToString(LogLevel::Off) == "OFF");
}

TEST_CASE("Log level character conversion", "[logging][level]") {
    REQUIRE(logLevelChar(LogLevel::Trace) == 'T');
    REQUIRE(logLevelChar(LogLevel::Debug) == 'D');
    REQUIRE(logLevelChar(LogLevel::Info) == 'I');
    REQUIRE(logLevelChar(LogLevel::Warning) == 'W');
    REQUIRE(logLevelChar(LogLevel::Error) == 'E');
    REQUIRE(logLevelChar(LogLevel::Fatal) == 'F');
    REQUIRE(logLevelChar(LogLevel::Off) == '-');
}

TEST_CASE("Log level ordering", "[logging][level]") {
    REQUIRE(LogLevel::Trace < LogLevel::Debug);
    REQUIRE(LogLevel::Debug < LogLevel::Info);
    REQUIRE(LogLevel::Info < LogLevel::Warning);
    REQUIRE(LogLevel::Warning < LogLevel::Error);
    REQUIRE(LogLevel::Error < LogLevel::Fatal);
    REQUIRE(LogLevel::Fatal < LogLevel::Off);
}

// =============================================================================
// Log Category Tests
// =============================================================================

TEST_CASE("Log category string conversion", "[logging][category]") {
    REQUIRE(logCategoryToString(LogCategory::Core) == "Core");
    REQUIRE(logCategoryToString(LogCategory::Memory) == "Memory");
    REQUIRE(logCategoryToString(LogCategory::Render) == "Render");
    REQUIRE(logCategoryToString(LogCategory::Physics) == "Physics");
    REQUIRE(logCategoryToString(LogCategory::Audio) == "Audio");
    REQUIRE(logCategoryToString(LogCategory::Input) == "Input");
    REQUIRE(logCategoryToString(LogCategory::Script) == "Script");
    REQUIRE(logCategoryToString(LogCategory::AI) == "AI");
    REQUIRE(logCategoryToString(LogCategory::Network) == "Network");
    REQUIRE(logCategoryToString(LogCategory::Asset) == "Asset");
    REQUIRE(logCategoryToString(LogCategory::Editor) == "Editor");
    REQUIRE(logCategoryToString(LogCategory::Platform) == "Platform");
    REQUIRE(logCategoryToString(LogCategory::Game) == "Game");
    REQUIRE(logCategoryToString(LogCategory::All) == "All");
}

TEST_CASE("Log category bitwise operations", "[logging][category]") {
    // Using underlying values to test bitmask operations
    u16 core = static_cast<u16>(LogCategory::Core);
    u16 render = static_cast<u16>(LogCategory::Render);
    u16 physics = static_cast<u16>(LogCategory::Physics);
    u16 combined = core | render;
    
    REQUIRE((combined & core) != 0);
    REQUIRE((combined & render) != 0);
    REQUIRE((combined & physics) == 0);
}

// =============================================================================
// Logger Tests
// =============================================================================

TEST_CASE("Logger singleton", "[logging][logger]") {
    auto& logger1 = Logger::instance();
    auto& logger2 = Logger::instance();
    REQUIRE(&logger1 == &logger2);
}

TEST_CASE("Logger level filtering", "[logging][logger]") {
    auto& logger = Logger::instance();
    
    logger.setLevel(LogLevel::Warning);
    REQUIRE(logger.getLevel() == LogLevel::Warning);
    
    REQUIRE_FALSE(logger.shouldLog(LogLevel::Trace, LogCategory::Core));
    REQUIRE_FALSE(logger.shouldLog(LogLevel::Debug, LogCategory::Core));
    REQUIRE_FALSE(logger.shouldLog(LogLevel::Info, LogCategory::Core));
    REQUIRE(logger.shouldLog(LogLevel::Warning, LogCategory::Core));
    REQUIRE(logger.shouldLog(LogLevel::Error, LogCategory::Core));
    REQUIRE(logger.shouldLog(LogLevel::Fatal, LogCategory::Core));
    
    // Reset to default
    logger.setLevel(LogLevel::Info);
}

TEST_CASE("Logger category filtering", "[logging][logger]") {
    auto& logger = Logger::instance();
    
    // Set specific category filter
    logger.setCategoryFilter(Flags<LogCategory>{LogCategory::Core} | Flags<LogCategory>{LogCategory::Render});
    logger.setLevel(LogLevel::Trace);
    
    REQUIRE(logger.shouldLog(LogLevel::Info, LogCategory::Core));
    REQUIRE(logger.shouldLog(LogLevel::Info, LogCategory::Render));
    // All category is always allowed in filter
    
    // Reset to all categories
    logger.setCategoryFilter(Flags<LogCategory>{LogCategory::All});
    logger.setLevel(LogLevel::Info);
}

// =============================================================================
// Timer Tests
// =============================================================================

TEST_CASE("Timer basic operations", "[profiling][timer]") {
    Timer timer;
    
    SECTION("Timer starts running") {
        REQUIRE(timer.isRunning());
    }
    
    SECTION("Timer measures time") {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        f64 elapsed = timer.elapsedMs();
        REQUIRE(elapsed >= 9.0); // Allow some tolerance
    }
    
    SECTION("Timer stop") {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        timer.stop();
        f64 stoppedTime = timer.elapsedMs();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        f64 laterTime = timer.elapsedMs();
        
        // Time should not change after stop
        REQUIRE(laterTime == stoppedTime);
    }
    
    SECTION("Timer reset") {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        timer.reset();
        f64 elapsed = timer.elapsedMs();
        REQUIRE(elapsed < 5.0); // Should be close to 0
    }
}

TEST_CASE("Timer unit conversions", "[profiling][timer]") {
    Timer timer;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    f64 ns = timer.elapsedNs();
    f64 us = timer.elapsedUs();
    f64 ms = timer.elapsedMs();
    f64 sec = timer.elapsedSec();
    
    // Check relationships
    REQUIRE(us > 90000.0);  // ~100ms in microseconds
    REQUIRE(ms > 90.0);     // ~100ms
    REQUIRE(sec > 0.090);   // ~100ms in seconds
    
    // Check conversions are consistent
    REQUIRE(ns / 1000.0 == Catch::Approx(us).epsilon(0.01));
    REQUIRE(us / 1000.0 == Catch::Approx(ms).epsilon(0.01));
    REQUIRE(ms / 1000.0 == Catch::Approx(sec).epsilon(0.01));
}

// =============================================================================
// Frame Timer Tests
// =============================================================================

TEST_CASE("Frame timer basic operations", "[profiling][frame]") {
    FrameTimer frameTimer;
    
    SECTION("Initial state") {
        auto& stats = frameTimer.getStats();
        REQUIRE(stats.frameCount == 0);
    }
    
    SECTION("Single frame") {
        frameTimer.beginFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        frameTimer.endFrame();
        
        auto& stats = frameTimer.getStats();
        REQUIRE(stats.frameCount == 1);
        REQUIRE(stats.frameTimeMs >= 15.0);
    }
    
    SECTION("Multiple frames with FPS calculation") {
        for (int i = 0; i < 5; ++i) {
            frameTimer.beginFrame();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
            frameTimer.endFrame();
        }
        
        auto& stats = frameTimer.getStats();
        REQUIRE(stats.frameCount == 5);
        REQUIRE(stats.fps > 0.0);
        REQUIRE(stats.avgFrameTimeMs > 0.0);
    }
}

// =============================================================================
// Profiling Zone Tests
// =============================================================================

TEST_CASE("Scoped zone timing", "[profiling][zone]") {
    f64 elapsedTime = 0.0;
    
    {
        ScopedZone zone("TestZone");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        elapsedTime = zone.elapsedMs();
    }
    
    REQUIRE(elapsedTime >= 9.0);
}

TEST_CASE("Zone color constants", "[profiling][zone]") {
    auto red = ZoneColor::red();
    REQUIRE(red.r == 255);
    REQUIRE(red.g == 100);
    REQUIRE(red.b == 100);
    
    auto render = ZoneColor::render();
    REQUIRE(render.r == 100);
    REQUIRE(render.g == 255);
    REQUIRE(render.b == 100);
}

// =============================================================================
// Timestamp Utilities Tests
// =============================================================================

TEST_CASE("Timestamp utilities", "[profiling][timestamp]") {
    auto start = profiling::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto end = profiling::now();
    
    REQUIRE(profiling::durationNs(start, end) > 9000000);  // >9ms in ns
    REQUIRE(profiling::durationUs(start, end) > 9000.0);   // >9ms in us
    REQUIRE(profiling::durationMs(start, end) > 9.0);      // >9ms
}
