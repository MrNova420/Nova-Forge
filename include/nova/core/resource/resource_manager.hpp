/**
 * @file resource_manager.hpp
 * @brief NovaCore Resource System™ - Main Resource Manager
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 * 
 * Central resource management system providing:
 * - Async and sync resource loading
 * - Resource caching and memory management
 * - Hot-reload support
 * - Dependency tracking
 * - Virtual file system
 */

#pragma once

#include "resource_types.hpp"

#include <unordered_map>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>

namespace nova::resource {

// ============================================================================
// Resource Manager
// ============================================================================

/**
 * @brief Central resource management system
 * 
 * The ResourceManager handles all resource loading, caching, and lifecycle:
 * 
 * Usage:
 * @code
 *     auto& resources = ResourceManager::get();
 *     
 *     // Sync load
 *     auto texture = resources.load<Texture2D>("textures/player.png");
 *     
 *     // Async load
 *     resources.loadAsync<Mesh>("meshes/character.glb", [](auto mesh) {
 *         // Use mesh
 *     });
 *     
 *     // Batch load
 *     resources.loadBatch({
 *         "textures/level1.png",
 *         "meshes/level1.glb",
 *         "sounds/ambient.ogg"
 *     }, [](auto& handles) {
 *         // All loaded
 *     });
 * @endcode
 */
class ResourceManager {
public:
    /**
     * @brief Get singleton instance
     */
    static ResourceManager& get();
    
    /**
     * @brief Initialize the resource manager
     * @param cacheSize Maximum cache size in bytes
     * @param numWorkers Number of loader threads
     * @return true if initialization succeeded
     */
    bool initialize(usize cacheSize = ResourceConfig::DEFAULT_CACHE_SIZE,
                   u32 numWorkers = 4);
    
    /**
     * @brief Shutdown the resource manager
     */
    void shutdown();
    
    /**
     * @brief Update resource manager (call each frame)
     * @param deltaTime Frame delta time
     */
    void update(f32 deltaTime);
    
    /**
     * @brief Check if initialized
     */
    bool isInitialized() const { return m_initialized; }
    
    // ========================================================================
    // Resource Loading
    // ========================================================================
    
    /**
     * @brief Load a resource synchronously
     * @tparam T Resource type
     * @param path Resource path
     * @param flags Load flags
     * @return Resource handle
     */
    template<typename T = Resource>
    ResourceHandle<T> load(const ResourcePath& path,
                          LoadFlags flags = LoadFlags::None);
    
    /**
     * @brief Load a resource asynchronously
     * @tparam T Resource type
     * @param path Resource path
     * @param callback Called when load completes
     * @param priority Load priority
     * @param flags Load flags
     */
    template<typename T = Resource>
    void loadAsync(const ResourcePath& path,
                  std::function<void(ResourceHandle<T>)> callback,
                  LoadPriority priority = LoadPriority::Normal,
                  LoadFlags flags = LoadFlags::Async);
    
    /**
     * @brief Load multiple resources asynchronously
     */
    void loadBatch(const std::vector<ResourcePath>& paths,
                  std::function<void(const std::vector<ResourceHandle<>>&)> callback,
                  LoadPriority priority = LoadPriority::Normal);
    
    /**
     * @brief Load a resource by ID
     */
    ResourceHandle<> loadById(ResourceId id, LoadFlags flags = LoadFlags::None);
    
    /**
     * @brief Reload a resource
     */
    void reload(ResourceHandle<> handle);
    
    /**
     * @brief Reload a resource by path
     */
    void reload(const ResourcePath& path);
    
    /**
     * @brief Reload all resources
     */
    void reloadAll();
    
    // ========================================================================
    // Resource Access
    // ========================================================================
    
    /**
     * @brief Get a loaded resource by path
     */
    template<typename T = Resource>
    ResourceHandle<T> get(const ResourcePath& path) const;
    
    /**
     * @brief Get a loaded resource by ID
     */
    template<typename T = Resource>
    ResourceHandle<T> get(ResourceId id) const;
    
    /**
     * @brief Check if resource is loaded
     */
    bool isLoaded(const ResourcePath& path) const;
    bool isLoaded(ResourceId id) const;
    
    /**
     * @brief Check if resource is loading
     */
    bool isLoading(const ResourcePath& path) const;
    bool isLoading(ResourceId id) const;
    
    /**
     * @brief Get resource state
     */
    ResourceState getState(const ResourcePath& path) const;
    ResourceState getState(ResourceId id) const;
    
    // ========================================================================
    // Resource Unloading
    // ========================================================================
    
    /**
     * @brief Unload a resource
     */
    void unload(ResourceHandle<> handle);
    void unload(const ResourcePath& path);
    void unload(ResourceId id);
    
    /**
     * @brief Unload all resources of a type
     */
    void unloadType(ResourceType type);
    
    /**
     * @brief Unload all unused resources (no handles)
     */
    void unloadUnused();
    
    /**
     * @brief Unload all resources
     */
    void unloadAll();
    
    // ========================================================================
    // Resource Registration
    // ========================================================================
    
    /**
     * @brief Register a resource loader
     */
    void registerLoader(std::unique_ptr<IResourceLoader> loader);
    
    /**
     * @brief Unregister a loader by extension
     */
    void unregisterLoader(const std::string& extension);
    
    /**
     * @brief Get loader for a path
     */
    IResourceLoader* getLoader(const ResourcePath& path) const;
    
    /**
     * @brief Register a resource directly
     */
    template<typename T>
    ResourceHandle<T> registerResource(std::shared_ptr<T> resource,
                                       const ResourcePath& path);
    
    // ========================================================================
    // Virtual File System
    // ========================================================================
    
    /**
     * @brief Mount a directory or archive
     * @param virtualPath Virtual mount point
     * @param physicalPath Physical path (directory or .pak file)
     * @param priority Mount priority (higher = checked first)
     */
    void mount(const std::string& virtualPath,
              const std::string& physicalPath,
              i32 priority = 0);
    
    /**
     * @brief Unmount a path
     */
    void unmount(const std::string& virtualPath);
    
    /**
     * @brief Check if a file exists
     */
    bool exists(const ResourcePath& path) const;
    
    /**
     * @brief List files in a directory
     */
    std::vector<ResourcePath> listFiles(const ResourcePath& directory,
                                        bool recursive = false) const;
    
    /**
     * @brief Read raw file data
     */
    std::vector<u8> readFile(const ResourcePath& path) const;
    
    /**
     * @brief Get physical path for a virtual path
     */
    std::string getPhysicalPath(const ResourcePath& path) const;
    
    // ========================================================================
    // Bundles
    // ========================================================================
    
    /**
     * @brief Load a resource bundle
     */
    bool loadBundle(const ResourcePath& bundlePath);
    
    /**
     * @brief Unload a resource bundle
     */
    void unloadBundle(const std::string& bundleName);
    
    /**
     * @brief Get bundle info
     */
    const ResourceBundle* getBundle(const std::string& name) const;
    
    /**
     * @brief Get all bundle names
     */
    std::vector<std::string> getBundleNames() const;
    
    // ========================================================================
    // Hot Reload
    // ========================================================================
    
    /**
     * @brief Enable/disable hot reload
     */
    void setHotReloadEnabled(bool enabled);
    
    /**
     * @brief Check if hot reload is enabled
     */
    bool isHotReloadEnabled() const { return m_hotReloadEnabled; }
    
    /**
     * @brief Add a directory to watch for changes
     */
    void watchDirectory(const std::string& path);
    
    /**
     * @brief Stop watching a directory
     */
    void unwatchDirectory(const std::string& path);
    
    /**
     * @brief Set hot reload callback
     */
    void setHotReloadCallback(std::function<void(const ResourcePath&)> callback);
    
    // ========================================================================
    // Cache Management
    // ========================================================================
    
    /**
     * @brief Set maximum cache size
     */
    void setCacheSize(usize bytes);
    
    /**
     * @brief Get current cache size
     */
    usize getCacheSize() const { return m_cacheSize; }
    
    /**
     * @brief Get used cache memory
     */
    usize getUsedMemory() const;
    
    /**
     * @brief Clear the cache
     */
    void clearCache();
    
    /**
     * @brief Trim cache to fit size
     */
    void trimCache();
    
    /**
     * @brief Set unload delay (seconds before unused resources are unloaded)
     */
    void setUnloadDelay(f32 seconds);
    
    // ========================================================================
    // Statistics
    // ========================================================================
    
    /**
     * @brief Get cache statistics
     */
    CacheStats getStats() const;
    
    /**
     * @brief Reset statistics
     */
    void resetStats();
    
    /**
     * @brief Get resource metadata
     */
    ResourceMetadata getMetadata(const ResourcePath& path) const;
    
    /**
     * @brief Get all loaded resource paths
     */
    std::vector<ResourcePath> getLoadedPaths() const;
    
    /**
     * @brief Get resources by type
     */
    std::vector<ResourceHandle<>> getResourcesByType(ResourceType type) const;
    
    // ========================================================================
    // Dependencies
    // ========================================================================
    
    /**
     * @brief Get resource dependencies
     */
    std::vector<ResourceId> getDependencies(ResourceId id) const;
    
    /**
     * @brief Get resources that depend on this one
     */
    std::vector<ResourceId> getDependents(ResourceId id) const;
    
    /**
     * @brief Add dependency relationship
     */
    void addDependency(ResourceId dependent, ResourceId dependency);
    
private:
    ResourceManager() = default;
    ~ResourceManager() = default;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    
    // Internal methods
    ResourceHandle<> loadInternal(const ResourcePath& path, LoadFlags flags);
    void processLoadQueue();
    void workerThread();
    void checkHotReload();
    void evictLRU();
    
    Resource* findResource(const ResourcePath& path) const;
    Resource* findResource(ResourceId id) const;
    
    // State
    bool m_initialized = false;
    std::atomic<bool> m_running{false};
    
    // Resources
    mutable std::mutex m_resourceMutex;
    std::unordered_map<ResourceId, std::shared_ptr<Resource>> m_resources;
    std::unordered_map<ResourcePath, ResourceId> m_pathToId;
    
    // Loaders
    std::unordered_map<std::string, std::unique_ptr<IResourceLoader>> m_loaders;
    
    // Load queue
    std::mutex m_queueMutex;
    std::condition_variable m_queueCV;
    std::priority_queue<LoadRequest, std::vector<LoadRequest>,
                       std::function<bool(const LoadRequest&, const LoadRequest&)>> m_loadQueue{
        [](const LoadRequest& a, const LoadRequest& b) {
            return a.priority < b.priority;
        }
    };
    
    // Worker threads
    std::vector<std::thread> m_workers;
    
    // Virtual file system
    struct MountPoint {
        std::string virtualPath;
        std::string physicalPath;
        i32 priority;
        bool isArchive;
    };
    std::vector<MountPoint> m_mountPoints;
    
    // Bundles
    std::unordered_map<std::string, ResourceBundle> m_bundles;
    
    // Hot reload
    bool m_hotReloadEnabled = true;
    std::vector<std::string> m_watchDirectories;
    std::function<void(const ResourcePath&)> m_hotReloadCallback;
    
    // Cache
    usize m_cacheSize = ResourceConfig::DEFAULT_CACHE_SIZE;
    f32 m_unloadDelay = ResourceConfig::DEFAULT_UNLOAD_DELAY;
    
    // Statistics
    mutable std::mutex m_statsMutex;
    CacheStats m_stats;
    
    // Dependencies
    std::unordered_map<ResourceId, std::vector<ResourceId>> m_dependencies;
    std::unordered_map<ResourceId, std::vector<ResourceId>> m_dependents;
    
    // ID generation
    std::atomic<u64> m_nextId{1};
};

// ============================================================================
// Template Implementations
// ============================================================================

template<typename T>
ResourceHandle<T> ResourceManager::load(const ResourcePath& path, LoadFlags flags) {
    auto handle = loadInternal(path, flags);
    return handle.cast<T>();
}

template<typename T>
void ResourceManager::loadAsync(const ResourcePath& path,
                                std::function<void(ResourceHandle<T>)> callback,
                                LoadPriority priority,
                                LoadFlags flags) {
    LoadRequest request;
    request.path = path;
    request.priority = priority;
    request.flags = flags | LoadFlags::Async;
    request.onComplete = [callback](ResourceHandle<> handle) {
        callback(handle.cast<T>());
    };
    
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_loadQueue.push(request);
    }
    m_queueCV.notify_one();
}

template<typename T>
ResourceHandle<T> ResourceManager::get(const ResourcePath& path) const {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    auto it = m_pathToId.find(path);
    if (it == m_pathToId.end()) {
        return ResourceHandle<T>();
    }
    
    auto resIt = m_resources.find(it->second);
    if (resIt == m_resources.end()) {
        return ResourceHandle<T>();
    }
    
    return ResourceHandle<T>(std::dynamic_pointer_cast<T>(resIt->second));
}

template<typename T>
ResourceHandle<T> ResourceManager::get(ResourceId id) const {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    auto it = m_resources.find(id);
    if (it == m_resources.end()) {
        return ResourceHandle<T>();
    }
    
    return ResourceHandle<T>(std::dynamic_pointer_cast<T>(it->second));
}

template<typename T>
ResourceHandle<T> ResourceManager::registerResource(std::shared_ptr<T> resource,
                                                    const ResourcePath& path) {
    if (!resource) return ResourceHandle<T>();
    
    ResourceId id = ResourceId::fromPath(path.path);
    resource->m_id = id;
    resource->m_path = path;
    resource->m_name = path.getStem();
    resource->setState(ResourceState::Loaded);
    
    {
        std::lock_guard<std::mutex> lock(m_resourceMutex);
        m_resources[id] = resource;
        m_pathToId[path] = id;
    }
    
    return ResourceHandle<T>(resource);
}

} // namespace nova::resource
