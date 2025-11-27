/**
 * @file test_platform.cpp
 * @brief Unit tests for NovaCore Platform System Types
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/platform/platform_types.hpp>

using namespace nova;
using namespace nova::platform;
using Catch::Approx;

// ============================================================================
// Platform Detection Tests
// ============================================================================

TEST_CASE("Platform Types - Platform Enum", "[platform][types]") {
    SECTION("All platforms are defined") {
        REQUIRE(static_cast<u8>(Platform::Unknown) == 0);
        REQUIRE(static_cast<u8>(Platform::Windows) == 1);
        REQUIRE(static_cast<u8>(Platform::Linux) == 2);
        REQUIRE(static_cast<u8>(Platform::macOS) == 3);
        REQUIRE(static_cast<u8>(Platform::iOS) == 4);
        REQUIRE(static_cast<u8>(Platform::Android) == 5);
        REQUIRE(static_cast<u8>(Platform::Web) == 6);
        REQUIRE(static_cast<u8>(Platform::PlayStation) == 7);
        REQUIRE(static_cast<u8>(Platform::Xbox) == 8);
        REQUIRE(static_cast<u8>(Platform::Nintendo) == 9);
    }
}

TEST_CASE("Platform Types - Architecture Enum", "[platform][types]") {
    SECTION("All architectures are defined") {
        REQUIRE(static_cast<u8>(Architecture::Unknown) == 0);
        REQUIRE(static_cast<u8>(Architecture::x86) == 1);
        REQUIRE(static_cast<u8>(Architecture::x64) == 2);
        REQUIRE(static_cast<u8>(Architecture::ARM32) == 3);
        REQUIRE(static_cast<u8>(Architecture::ARM64) == 4);
        REQUIRE(static_cast<u8>(Architecture::WASM) == 5);
    }
}

TEST_CASE("Platform Types - GraphicsAPI Enum", "[platform][types]") {
    SECTION("All graphics APIs are defined") {
        REQUIRE(static_cast<u8>(GraphicsAPI::None) == 0);
        REQUIRE(static_cast<u8>(GraphicsAPI::Vulkan) == 1);
        REQUIRE(static_cast<u8>(GraphicsAPI::DirectX11) == 2);
        REQUIRE(static_cast<u8>(GraphicsAPI::DirectX12) == 3);
        REQUIRE(static_cast<u8>(GraphicsAPI::Metal) == 4);
        REQUIRE(static_cast<u8>(GraphicsAPI::OpenGL) == 5);
        REQUIRE(static_cast<u8>(GraphicsAPI::OpenGLES) == 6);
        REQUIRE(static_cast<u8>(GraphicsAPI::WebGPU) == 7);
    }
}

TEST_CASE("Platform Types - Platform Detection Functions", "[platform][types]") {
    SECTION("getCurrentPlatform returns valid platform") {
        Platform p = getCurrentPlatform();
        REQUIRE(p != Platform::Unknown);
        // On Linux CI runner
        #if defined(__linux__) && !defined(__ANDROID__)
        REQUIRE(p == Platform::Linux);
        #endif
    }
    
    SECTION("getCurrentArchitecture returns valid architecture") {
        Architecture a = getCurrentArchitecture();
        REQUIRE(a != Architecture::Unknown);
        // On x64 CI runner
        #if defined(__x86_64__) || defined(_M_X64)
        REQUIRE(a == Architecture::x64);
        #endif
    }
    
    SECTION("isMobilePlatform returns correct value") {
        // On desktop CI runner, this should be false
        #if !defined(__ANDROID__) && !defined(__APPLE__)
        REQUIRE(isMobilePlatform() == false);
        #endif
    }
    
    SECTION("isDesktopPlatform returns correct value") {
        // On Linux CI runner
        #if defined(__linux__) && !defined(__ANDROID__)
        REQUIRE(isDesktopPlatform() == true);
        #endif
    }
}

// ============================================================================
// Window Types Tests
// ============================================================================

TEST_CASE("Platform Types - WindowStyle Enum", "[platform][types]") {
    SECTION("Window style flags") {
        REQUIRE(static_cast<u32>(WindowStyle::None) == 0);
        REQUIRE(static_cast<u32>(WindowStyle::Titled) == 1);
        REQUIRE(static_cast<u32>(WindowStyle::Closable) == 2);
        REQUIRE(static_cast<u32>(WindowStyle::Minimizable) == 4);
        REQUIRE(static_cast<u32>(WindowStyle::Maximizable) == 8);
        REQUIRE(static_cast<u32>(WindowStyle::Resizable) == 16);
        REQUIRE(static_cast<u32>(WindowStyle::Borderless) == 32);
        REQUIRE(static_cast<u32>(WindowStyle::Fullscreen) == 64);
        REQUIRE(static_cast<u32>(WindowStyle::AlwaysOnTop) == 128);
        REQUIRE(static_cast<u32>(WindowStyle::Transparent) == 256);
    }
    
    SECTION("Window style operators") {
        WindowStyle combined = WindowStyle::Titled | WindowStyle::Closable;
        REQUIRE(static_cast<u32>(combined) == 3);
        
        WindowStyle masked = combined & WindowStyle::Titled;
        REQUIRE(static_cast<u32>(masked) == 1);
    }
    
    SECTION("hasStyle helper function") {
        WindowStyle styles = WindowStyle::Titled | WindowStyle::Closable | WindowStyle::Resizable;
        REQUIRE(hasStyle(styles, WindowStyle::Titled) == true);
        REQUIRE(hasStyle(styles, WindowStyle::Closable) == true);
        REQUIRE(hasStyle(styles, WindowStyle::Resizable) == true);
        REQUIRE(hasStyle(styles, WindowStyle::Borderless) == false);
        REQUIRE(hasStyle(styles, WindowStyle::Fullscreen) == false);
    }
    
    SECTION("Default window style") {
        WindowStyle def = WindowStyle::Default;
        REQUIRE(hasStyle(def, WindowStyle::Titled) == true);
        REQUIRE(hasStyle(def, WindowStyle::Closable) == true);
        REQUIRE(hasStyle(def, WindowStyle::Minimizable) == true);
        REQUIRE(hasStyle(def, WindowStyle::Maximizable) == true);
        REQUIRE(hasStyle(def, WindowStyle::Resizable) == true);
    }
}

TEST_CASE("Platform Types - WindowState Enum", "[platform][types]") {
    SECTION("Window states") {
        REQUIRE(static_cast<u8>(WindowState::Normal) == 0);
        REQUIRE(static_cast<u8>(WindowState::Minimized) == 1);
        REQUIRE(static_cast<u8>(WindowState::Maximized) == 2);
        REQUIRE(static_cast<u8>(WindowState::Fullscreen) == 3);
        REQUIRE(static_cast<u8>(WindowState::Hidden) == 4);
    }
}

TEST_CASE("Platform Types - FullscreenMode Enum", "[platform][types]") {
    SECTION("Fullscreen modes") {
        REQUIRE(static_cast<u8>(FullscreenMode::Windowed) == 0);
        REQUIRE(static_cast<u8>(FullscreenMode::Borderless) == 1);
        REQUIRE(static_cast<u8>(FullscreenMode::Exclusive) == 2);
    }
}

TEST_CASE("Platform Types - DisplayMode Structure", "[platform][types]") {
    SECTION("Default display mode") {
        DisplayMode dm;
        REQUIRE(dm.width == 0);
        REQUIRE(dm.height == 0);
        REQUIRE(dm.refreshRate == 60);
        REQUIRE(dm.bitsPerPixel == 32);
    }
    
    SECTION("DisplayMode comparison") {
        DisplayMode dm1{1920, 1080, 60, 32};
        DisplayMode dm2{1920, 1080, 60, 32};
        DisplayMode dm3{1280, 720, 60, 32};
        
        REQUIRE(dm1 == dm2);
        REQUIRE_FALSE(dm1 == dm3);
    }
}

TEST_CASE("Platform Types - MonitorInfo Structure", "[platform][types]") {
    SECTION("Default monitor info") {
        MonitorInfo mi;
        REQUIRE(mi.name.empty());
        REQUIRE(mi.index == 0);
        REQUIRE(mi.dpi == Approx(96.0f));
        REQUIRE(mi.scale == Approx(1.0f));
        REQUIRE(mi.isPrimary == false);
    }
}

TEST_CASE("Platform Types - WindowDesc Structure", "[platform][types]") {
    SECTION("Default window description") {
        WindowDesc wd;
        REQUIRE(wd.title == "NovaCore");
        REQUIRE(wd.width == 1280);
        REQUIRE(wd.height == 720);
        REQUIRE(wd.x == -1);
        REQUIRE(wd.y == -1);
        REQUIRE(wd.style == WindowStyle::Default);
        REQUIRE(wd.fullscreenMode == FullscreenMode::Windowed);
        REQUIRE(wd.targetMonitor == 0);
        REQUIRE(wd.vsync == true);
        REQUIRE(wd.samples == 1);
        REQUIRE(wd.graphicsAPI == GraphicsAPI::Vulkan);
        REQUIRE(wd.highDPI == true);
        REQUIRE(wd.parentWindow == nullptr);
    }
    
    SECTION("Custom window description") {
        WindowDesc wd;
        wd.title = "My Game";
        wd.width = 1920;
        wd.height = 1080;
        wd.style = WindowStyle::Borderless | WindowStyle::Fullscreen;
        wd.fullscreenMode = FullscreenMode::Exclusive;
        wd.vsync = false;
        wd.samples = 4;
        
        REQUIRE(wd.title == "My Game");
        REQUIRE(wd.width == 1920);
        REQUIRE(wd.height == 1080);
        REQUIRE(hasStyle(wd.style, WindowStyle::Borderless) == true);
        REQUIRE(hasStyle(wd.style, WindowStyle::Fullscreen) == true);
        REQUIRE(wd.fullscreenMode == FullscreenMode::Exclusive);
        REQUIRE(wd.vsync == false);
        REQUIRE(wd.samples == 4);
    }
}

// ============================================================================
// System Information Tests
// ============================================================================

TEST_CASE("Platform Types - CPUInfo Structure", "[platform][types]") {
    SECTION("Default CPU info") {
        CPUInfo cpu;
        REQUIRE(cpu.name.empty());
        REQUIRE(cpu.vendor.empty());
        REQUIRE(cpu.coreCount == 0);
        REQUIRE(cpu.threadCount == 0);
        REQUIRE(cpu.frequencyMHz == 0);
        REQUIRE(cpu.hasSSE == false);
        REQUIRE(cpu.hasAVX == false);
        REQUIRE(cpu.hasNEON == false);
    }
    
    SECTION("CPU info with values") {
        CPUInfo cpu;
        cpu.name = "Intel Core i9-12900K";
        cpu.vendor = "GenuineIntel";
        cpu.coreCount = 16;
        cpu.threadCount = 24;
        cpu.frequencyMHz = 3200;
        cpu.hasSSE = true;
        cpu.hasSSE2 = true;
        cpu.hasSSE3 = true;
        cpu.hasSSE4_1 = true;
        cpu.hasSSE4_2 = true;
        cpu.hasAVX = true;
        cpu.hasAVX2 = true;
        
        REQUIRE(cpu.name == "Intel Core i9-12900K");
        REQUIRE(cpu.vendor == "GenuineIntel");
        REQUIRE(cpu.coreCount == 16);
        REQUIRE(cpu.threadCount == 24);
        REQUIRE(cpu.hasAVX2 == true);
    }
}

TEST_CASE("Platform Types - GPUInfo Structure", "[platform][types]") {
    SECTION("Default GPU info") {
        GPUInfo gpu;
        REQUIRE(gpu.name.empty());
        REQUIRE(gpu.vendor.empty());
        REQUIRE(gpu.driver.empty());
        REQUIRE(gpu.dedicatedMemory == 0);
        REQUIRE(gpu.sharedMemory == 0);
        REQUIRE(gpu.isIntegrated == false);
        REQUIRE(gpu.isDiscrete == false);
    }
    
    SECTION("GPU info with values") {
        GPUInfo gpu;
        gpu.name = "NVIDIA GeForce RTX 4090";
        gpu.vendor = "NVIDIA Corporation";
        gpu.dedicatedMemory = 24ULL * 1024 * 1024 * 1024;  // 24GB
        gpu.isDiscrete = true;
        
        REQUIRE(gpu.name == "NVIDIA GeForce RTX 4090");
        REQUIRE(gpu.vendor == "NVIDIA Corporation");
        REQUIRE(gpu.dedicatedMemory == 24ULL * 1024 * 1024 * 1024);
        REQUIRE(gpu.isDiscrete == true);
    }
}

TEST_CASE("Platform Types - MemoryInfo Structure", "[platform][types]") {
    SECTION("Default memory info") {
        MemoryInfo mem;
        REQUIRE(mem.totalPhysical == 0);
        REQUIRE(mem.availablePhysical == 0);
        REQUIRE(mem.totalVirtual == 0);
        REQUIRE(mem.availableVirtual == 0);
        REQUIRE(mem.pageSize == 4096);
    }
    
    SECTION("Memory info with values") {
        MemoryInfo mem;
        mem.totalPhysical = 32ULL * 1024 * 1024 * 1024;  // 32GB
        mem.availablePhysical = 16ULL * 1024 * 1024 * 1024;  // 16GB available
        mem.totalVirtual = 128ULL * 1024 * 1024 * 1024;  // 128GB virtual
        mem.pageSize = 4096;
        
        REQUIRE(mem.totalPhysical == 32ULL * 1024 * 1024 * 1024);
        REQUIRE(mem.availablePhysical == 16ULL * 1024 * 1024 * 1024);
        REQUIRE(mem.pageSize == 4096);
    }
}
