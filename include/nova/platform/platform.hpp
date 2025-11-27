/**
 * @file platform.hpp
 * @brief NovaCore Platform™ - Main Include Header
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#pragma once

#include "platform_types.hpp"
#include "window.hpp"
#include "application.hpp"

namespace nova::platform {

/**
 * @brief Platform version
 */
namespace version {
    constexpr u32 MAJOR = 1;
    constexpr u32 MINOR = 0;
    constexpr u32 PATCH = 0;
    constexpr const char* STRING = "1.0.0";
}

/**
 * @brief Get platform information
 */
inline const char* getPlatformInfo() {
    return getPlatformName(getCurrentPlatform());
}

/**
 * @brief Get architecture information
 */
inline const char* getArchitectureInfo() {
    return getArchitectureName(getCurrentArchitecture());
}

} // namespace nova::platform
