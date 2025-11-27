/**
 * @file network.hpp
 * @brief Nova Network™ - Main networking include header
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "network_types.hpp"
#include "network_system.hpp"

namespace nova::network {

/// Network module version
constexpr u32 NETWORK_VERSION_MAJOR = 0;
constexpr u32 NETWORK_VERSION_MINOR = 1;
constexpr u32 NETWORK_VERSION_PATCH = 0;

/// Network module name
constexpr const char* NETWORK_MODULE_NAME = "Nova Network";

} // namespace nova::network
