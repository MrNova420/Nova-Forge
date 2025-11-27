/**
 * @file resource.hpp
 * @brief NovaCore Resource System™ - Main Include Header
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#pragma once

#include "resource_types.hpp"
#include "resource_manager.hpp"

namespace nova::resource {

/**
 * @brief Resource system version
 */
namespace version {
    constexpr u32 MAJOR = 1;
    constexpr u32 MINOR = 0;
    constexpr u32 PATCH = 0;
    constexpr const char* STRING = "1.0.0";
}

/**
 * @brief Initialize the resource system
 */
inline bool initializeResources(usize cacheSize = ResourceConfig::DEFAULT_CACHE_SIZE) {
    return ResourceManager::get().initialize(cacheSize);
}

/**
 * @brief Shutdown the resource system
 */
inline void shutdownResources() {
    ResourceManager::get().shutdown();
}

/**
 * @brief Update resource system (call each frame)
 */
inline void updateResources(f32 deltaTime) {
    ResourceManager::get().update(deltaTime);
}

/**
 * @brief Get the resource manager singleton
 */
inline ResourceManager& getResources() {
    return ResourceManager::get();
}

// Convenience functions
template<typename T = Resource>
inline ResourceHandle<T> loadResource(const ResourcePath& path) {
    return ResourceManager::get().load<T>(path);
}

template<typename T = Resource>
inline void loadResourceAsync(const ResourcePath& path,
                              std::function<void(ResourceHandle<T>)> callback) {
    ResourceManager::get().loadAsync<T>(path, callback);
}

template<typename T = Resource>
inline ResourceHandle<T> getResource(const ResourcePath& path) {
    return ResourceManager::get().get<T>(path);
}

inline void unloadResource(const ResourcePath& path) {
    ResourceManager::get().unload(path);
}

} // namespace nova::resource
