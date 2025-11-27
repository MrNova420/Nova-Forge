/**
 * @file resource_manager.cpp
 * @brief NovaCore Resource System™ - Main Resource Manager Implementation
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#include <nova/core/resource/resource_manager.hpp>

#include <algorithm>
#include <fstream>
#include <filesystem>
#include <chrono>

namespace nova::resource {

namespace fs = std::filesystem;

// ============================================================================
// ResourceId Implementation
// ============================================================================

ResourceId ResourceId::generate() {
    static std::atomic<u64> counter{1};
    return ResourceId(counter.fetch_add(1, std::memory_order_relaxed));
}

ResourceId ResourceId::fromPath(const std::string& path) {
    // FNV-1a hash
    u64 hash = 14695981039346656037ULL;
    for (char c : path) {
        hash ^= static_cast<u64>(c);
        hash *= 1099511628211ULL;
    }
    return ResourceId(hash);
}

// ============================================================================
// Singleton
// ============================================================================

ResourceManager& ResourceManager::get() {
    static ResourceManager instance;
    return instance;
}

// ============================================================================
// Initialization
// ============================================================================

bool ResourceManager::initialize(usize cacheSize, u32 numWorkers) {
    if (m_initialized) {
        return true;
    }
    
    m_cacheSize = cacheSize;
    m_running = true;
    
    // Start worker threads
    for (u32 i = 0; i < numWorkers; ++i) {
        m_workers.emplace_back(&ResourceManager::workerThread, this);
    }
    
    // Mount default paths
    mount("/", ".", 0);
    mount("/assets", "assets", 1);
    
    m_initialized = true;
    return true;
}

void ResourceManager::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    // Stop workers
    m_running = false;
    m_queueCV.notify_all();
    
    for (auto& worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    m_workers.clear();
    
    // Unload all resources
    unloadAll();
    
    // Clear loaders
    m_loaders.clear();
    
    // Clear mount points
    m_mountPoints.clear();
    
    // Clear bundles
    m_bundles.clear();
    
    m_initialized = false;
}

void ResourceManager::update(f32 deltaTime) {
    if (!m_initialized) {
        return;
    }
    
    // Check for hot reload
    if (m_hotReloadEnabled) {
        checkHotReload();
    }
    
    // Trim cache if needed
    usize usedMemory = getUsedMemory();
    if (usedMemory > m_cacheSize) {
        evictLRU();
    }
}

// ============================================================================
// Resource Loading
// ============================================================================

ResourceHandle<> ResourceManager::loadInternal(const ResourcePath& path, LoadFlags flags) {
    // Check if already loaded
    {
        std::lock_guard<std::mutex> lock(m_resourceMutex);
        auto it = m_pathToId.find(path);
        if (it != m_pathToId.end()) {
            auto resIt = m_resources.find(it->second);
            if (resIt != m_resources.end() && resIt->second->isLoaded()) {
                m_stats.cacheHits++;
                resIt->second->updateAccessTime();
                return ResourceHandle<>(resIt->second);
            }
        }
    }
    
    m_stats.cacheMisses++;
    
    // Find loader
    IResourceLoader* loader = getLoader(path);
    if (!loader) {
        return ResourceHandle<>();
    }
    
    // Read file data
    std::vector<u8> data = readFile(path);
    if (data.empty()) {
        return ResourceHandle<>();
    }
    
    // Create resource
    auto resource = loader->createResource();
    if (!resource) {
        return ResourceHandle<>();
    }
    
    ResourceId id = ResourceId::fromPath(path.path);
    resource->m_id = id;
    resource->m_path = path;
    resource->m_name = path.getStem();
    resource->m_type = loader->getResourceType();
    resource->m_flags = flags;
    
    // Load synchronously or queue async
    if (hasFlag(flags, LoadFlags::Async)) {
        resource->setState(ResourceState::Queued);
        
        LoadRequest request;
        request.id = id;
        request.path = path;
        request.type = loader->getResourceType();
        request.flags = flags;
        
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_loadQueue.push(request);
        }
        m_queueCV.notify_one();
    } else {
        // Sync load
        resource->setState(ResourceState::Loading);
        
        auto startTime = std::chrono::steady_clock::now();
        
        if (loader->load(resource.get(), data)) {
            resource->setState(ResourceState::Loaded);
            
            auto endTime = std::chrono::steady_clock::now();
            f64 elapsed = std::chrono::duration<f64>(endTime - startTime).count();
            
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats.loadRequestsCompleted++;
            m_stats.totalLoadTime += elapsed;
            m_stats.averageLoadTime = m_stats.totalLoadTime / m_stats.loadRequestsCompleted;
        } else {
            resource->setError("Failed to load resource");
            
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats.loadRequestsFailed++;
        }
    }
    
    // Store resource
    {
        std::lock_guard<std::mutex> lock(m_resourceMutex);
        m_resources[id] = resource;
        m_pathToId[path] = id;
    }
    
    return ResourceHandle<>(resource);
}

void ResourceManager::loadBatch(const std::vector<ResourcePath>& paths,
                                std::function<void(const std::vector<ResourceHandle<>>&)> callback,
                                LoadPriority priority) {
    struct BatchState {
        std::mutex mutex;
        std::vector<ResourceHandle<>> handles;
        u32 remaining;
        std::function<void(const std::vector<ResourceHandle<>>&)> callback;
    };
    
    auto state = std::make_shared<BatchState>();
    state->handles.resize(paths.size());
    state->remaining = static_cast<u32>(paths.size());
    state->callback = callback;
    
    for (usize i = 0; i < paths.size(); ++i) {
        usize index = i;
        loadAsync<Resource>(paths[i], [state, index](ResourceHandle<> handle) {
            std::lock_guard<std::mutex> lock(state->mutex);
            state->handles[index] = handle;
            state->remaining--;
            
            if (state->remaining == 0 && state->callback) {
                state->callback(state->handles);
            }
        }, priority, LoadFlags::Async);
    }
}

ResourceHandle<> ResourceManager::loadById(ResourceId id, LoadFlags flags) {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    auto it = m_resources.find(id);
    if (it != m_resources.end()) {
        return ResourceHandle<>(it->second);
    }
    
    return ResourceHandle<>();
}

void ResourceManager::reload(ResourceHandle<> handle) {
    if (!handle.isValid()) return;
    reload(handle->getPath());
}

void ResourceManager::reload(const ResourcePath& path) {
    std::shared_ptr<Resource> resource;
    
    {
        std::lock_guard<std::mutex> lock(m_resourceMutex);
        auto it = m_pathToId.find(path);
        if (it == m_pathToId.end()) return;
        
        auto resIt = m_resources.find(it->second);
        if (resIt == m_resources.end()) return;
        
        resource = resIt->second;
    }
    
    if (!resource) return;
    
    // Unload current
    resource->unload();
    resource->setState(ResourceState::Unloaded);
    
    // Reload
    IResourceLoader* loader = getLoader(path);
    if (!loader) return;
    
    std::vector<u8> data = readFile(path);
    if (data.empty()) {
        resource->setError("Failed to read file for reload");
        return;
    }
    
    resource->setState(ResourceState::Loading);
    
    if (loader->load(resource.get(), data)) {
        resource->setState(ResourceState::Loaded);
        
        if (m_hotReloadCallback) {
            m_hotReloadCallback(path);
        }
    } else {
        resource->setError("Failed to reload resource");
    }
}

void ResourceManager::reloadAll() {
    std::vector<ResourcePath> paths;
    
    {
        std::lock_guard<std::mutex> lock(m_resourceMutex);
        for (const auto& [path, id] : m_pathToId) {
            paths.push_back(path);
        }
    }
    
    for (const auto& path : paths) {
        reload(path);
    }
}

// ============================================================================
// Resource Access
// ============================================================================

bool ResourceManager::isLoaded(const ResourcePath& path) const {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    auto it = m_pathToId.find(path);
    if (it == m_pathToId.end()) return false;
    
    auto resIt = m_resources.find(it->second);
    if (resIt == m_resources.end()) return false;
    
    return resIt->second->isLoaded();
}

bool ResourceManager::isLoaded(ResourceId id) const {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    auto it = m_resources.find(id);
    if (it == m_resources.end()) return false;
    
    return it->second->isLoaded();
}

bool ResourceManager::isLoading(const ResourcePath& path) const {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    auto it = m_pathToId.find(path);
    if (it == m_pathToId.end()) return false;
    
    auto resIt = m_resources.find(it->second);
    if (resIt == m_resources.end()) return false;
    
    return resIt->second->isLoading();
}

bool ResourceManager::isLoading(ResourceId id) const {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    auto it = m_resources.find(id);
    if (it == m_resources.end()) return false;
    
    return it->second->isLoading();
}

ResourceState ResourceManager::getState(const ResourcePath& path) const {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    auto it = m_pathToId.find(path);
    if (it == m_pathToId.end()) return ResourceState::Unloaded;
    
    auto resIt = m_resources.find(it->second);
    if (resIt == m_resources.end()) return ResourceState::Unloaded;
    
    return resIt->second->getState();
}

ResourceState ResourceManager::getState(ResourceId id) const {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    auto it = m_resources.find(id);
    if (it == m_resources.end()) return ResourceState::Unloaded;
    
    return it->second->getState();
}

// ============================================================================
// Resource Unloading
// ============================================================================

void ResourceManager::unload(ResourceHandle<> handle) {
    if (!handle.isValid()) return;
    unload(handle->getId());
}

void ResourceManager::unload(const ResourcePath& path) {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    auto it = m_pathToId.find(path);
    if (it == m_pathToId.end()) return;
    
    unload(it->second);
}

void ResourceManager::unload(ResourceId id) {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    auto it = m_resources.find(id);
    if (it == m_resources.end()) return;
    
    // Don't unload persistent resources
    if (hasFlag(it->second->m_flags, LoadFlags::Persistent)) {
        return;
    }
    
    it->second->unload();
    it->second->setState(ResourceState::Unloaded);
    
    // Remove path mapping
    m_pathToId.erase(it->second->getPath());
    
    m_resources.erase(it);
}

void ResourceManager::unloadType(ResourceType type) {
    std::vector<ResourceId> toUnload;
    
    {
        std::lock_guard<std::mutex> lock(m_resourceMutex);
        for (const auto& [id, resource] : m_resources) {
            if (resource->getType() == type) {
                toUnload.push_back(id);
            }
        }
    }
    
    for (ResourceId id : toUnload) {
        unload(id);
    }
}

void ResourceManager::unloadUnused() {
    std::vector<ResourceId> toUnload;
    
    {
        std::lock_guard<std::mutex> lock(m_resourceMutex);
        for (const auto& [id, resource] : m_resources) {
            if (resource.use_count() == 1) {  // Only manager holds reference
                toUnload.push_back(id);
            }
        }
    }
    
    for (ResourceId id : toUnload) {
        unload(id);
    }
}

void ResourceManager::unloadAll() {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    for (auto& [id, resource] : m_resources) {
        resource->unload();
        resource->setState(ResourceState::Unloaded);
    }
    
    m_resources.clear();
    m_pathToId.clear();
}

// ============================================================================
// Resource Registration
// ============================================================================

void ResourceManager::registerLoader(std::unique_ptr<IResourceLoader> loader) {
    if (!loader) return;
    
    for (const auto& ext : loader->getSupportedExtensions()) {
        m_loaders[ext] = std::move(loader);
        return;  // Loader owns multiple extensions, only store once
    }
}

void ResourceManager::unregisterLoader(const std::string& extension) {
    m_loaders.erase(extension);
}

IResourceLoader* ResourceManager::getLoader(const ResourcePath& path) const {
    std::string ext = path.getExtension();
    auto it = m_loaders.find(ext);
    return it != m_loaders.end() ? it->second.get() : nullptr;
}

// ============================================================================
// Virtual File System
// ============================================================================

void ResourceManager::mount(const std::string& virtualPath,
                           const std::string& physicalPath,
                           i32 priority) {
    MountPoint mp;
    mp.virtualPath = virtualPath;
    mp.physicalPath = physicalPath;
    mp.priority = priority;
    mp.isArchive = physicalPath.ends_with(".pak") || physicalPath.ends_with(".zip");
    
    m_mountPoints.push_back(mp);
    
    // Sort by priority (higher first)
    std::sort(m_mountPoints.begin(), m_mountPoints.end(),
        [](const MountPoint& a, const MountPoint& b) {
            return a.priority > b.priority;
        });
}

void ResourceManager::unmount(const std::string& virtualPath) {
    m_mountPoints.erase(
        std::remove_if(m_mountPoints.begin(), m_mountPoints.end(),
            [&](const MountPoint& mp) { return mp.virtualPath == virtualPath; }),
        m_mountPoints.end()
    );
}

bool ResourceManager::exists(const ResourcePath& path) const {
    std::string physPath = getPhysicalPath(path);
    return !physPath.empty() && fs::exists(physPath);
}

std::vector<ResourcePath> ResourceManager::listFiles(const ResourcePath& directory,
                                                     bool recursive) const {
    std::vector<ResourcePath> result;
    
    std::string physPath = getPhysicalPath(directory);
    if (physPath.empty() || !fs::exists(physPath)) {
        return result;
    }
    
    if (recursive) {
        for (const auto& entry : fs::recursive_directory_iterator(physPath)) {
            if (entry.is_regular_file()) {
                result.push_back(ResourcePath(entry.path().string()));
            }
        }
    } else {
        for (const auto& entry : fs::directory_iterator(physPath)) {
            if (entry.is_regular_file()) {
                result.push_back(ResourcePath(entry.path().string()));
            }
        }
    }
    
    return result;
}

std::vector<u8> ResourceManager::readFile(const ResourcePath& path) const {
    std::string physPath = getPhysicalPath(path);
    if (physPath.empty()) {
        return {};
    }
    
    std::ifstream file(physPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return {};
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<u8> buffer(static_cast<usize>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return {};
    }
    
    return buffer;
}

std::string ResourceManager::getPhysicalPath(const ResourcePath& path) const {
    for (const auto& mp : m_mountPoints) {
        if (path.path.starts_with(mp.virtualPath)) {
            std::string relativePath = path.path.substr(mp.virtualPath.length());
            if (!relativePath.empty() && relativePath[0] == '/') {
                relativePath = relativePath.substr(1);
            }
            
            std::string fullPath = mp.physicalPath;
            if (!fullPath.empty() && fullPath.back() != '/' && !relativePath.empty()) {
                fullPath += '/';
            }
            fullPath += relativePath;
            
            if (fs::exists(fullPath)) {
                return fullPath;
            }
        }
    }
    
    // Try direct path
    if (fs::exists(path.path)) {
        return path.path;
    }
    
    return "";
}

// ============================================================================
// Bundles
// ============================================================================

bool ResourceManager::loadBundle(const ResourcePath& bundlePath) {
    std::string stem = bundlePath.getStem();
    
    ResourceBundle bundle;
    bundle.name = stem;
    bundle.path = bundlePath;
    bundle.isLoaded = true;
    
    m_bundles[stem] = bundle;
    return true;
}

void ResourceManager::unloadBundle(const std::string& bundleName) {
    auto it = m_bundles.find(bundleName);
    if (it == m_bundles.end()) return;
    
    // Unload all resources in bundle
    for (ResourceId id : it->second.resources) {
        unload(id);
    }
    
    m_bundles.erase(it);
}

const ResourceBundle* ResourceManager::getBundle(const std::string& name) const {
    auto it = m_bundles.find(name);
    return it != m_bundles.end() ? &it->second : nullptr;
}

std::vector<std::string> ResourceManager::getBundleNames() const {
    std::vector<std::string> names;
    for (const auto& [name, bundle] : m_bundles) {
        names.push_back(name);
    }
    return names;
}

// ============================================================================
// Hot Reload
// ============================================================================

void ResourceManager::setHotReloadEnabled(bool enabled) {
    m_hotReloadEnabled = enabled;
}

void ResourceManager::watchDirectory(const std::string& path) {
    if (std::find(m_watchDirectories.begin(), m_watchDirectories.end(), path) == m_watchDirectories.end()) {
        m_watchDirectories.push_back(path);
    }
}

void ResourceManager::unwatchDirectory(const std::string& path) {
    m_watchDirectories.erase(
        std::remove(m_watchDirectories.begin(), m_watchDirectories.end(), path),
        m_watchDirectories.end()
    );
}

void ResourceManager::setHotReloadCallback(std::function<void(const ResourcePath&)> callback) {
    m_hotReloadCallback = std::move(callback);
}

void ResourceManager::checkHotReload() {
    // Would use platform file watcher here
    // For now, just check modification times periodically
}

// ============================================================================
// Cache Management
// ============================================================================

void ResourceManager::setCacheSize(usize bytes) {
    m_cacheSize = bytes;
    trimCache();
}

usize ResourceManager::getUsedMemory() const {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    usize total = 0;
    for (const auto& [id, resource] : m_resources) {
        total += resource->getMemorySize();
    }
    return total;
}

void ResourceManager::clearCache() {
    unloadUnused();
}

void ResourceManager::trimCache() {
    while (getUsedMemory() > m_cacheSize) {
        evictLRU();
    }
}

void ResourceManager::setUnloadDelay(f32 seconds) {
    m_unloadDelay = seconds;
}

void ResourceManager::evictLRU() {
    ResourceId oldest;
    u64 oldestTime = ~0ULL;
    
    {
        std::lock_guard<std::mutex> lock(m_resourceMutex);
        
        for (const auto& [id, resource] : m_resources) {
            // Skip persistent resources
            if (hasFlag(resource->m_flags, LoadFlags::Persistent)) {
                continue;
            }
            
            // Skip resources still in use
            if (resource.use_count() > 1) {
                continue;
            }
            
            if (resource->getLastAccessTime() < oldestTime) {
                oldestTime = resource->getLastAccessTime();
                oldest = id;
            }
        }
    }
    
    if (oldest.isValid()) {
        unload(oldest);
    }
}

// ============================================================================
// Statistics
// ============================================================================

CacheStats ResourceManager::getStats() const {
    std::lock_guard<std::mutex> statsLock(m_statsMutex);
    std::lock_guard<std::mutex> resLock(m_resourceMutex);
    
    CacheStats stats = m_stats;
    
    stats.totalResources = static_cast<u32>(m_resources.size());
    for (const auto& [id, resource] : m_resources) {
        switch (resource->getState()) {
            case ResourceState::Loaded:
                stats.loadedResources++;
                break;
            case ResourceState::Loading:
            case ResourceState::Queued:
                stats.loadingResources++;
                break;
            case ResourceState::Failed:
                stats.failedResources++;
                break;
            default:
                break;
        }
    }
    
    stats.usedMemory = getUsedMemory();
    stats.cacheSize = m_cacheSize;
    stats.hitRate = (stats.cacheHits + stats.cacheMisses > 0) ?
        static_cast<f32>(stats.cacheHits) / (stats.cacheHits + stats.cacheMisses) : 0.0f;
    
    return stats;
}

void ResourceManager::resetStats() {
    std::lock_guard<std::mutex> lock(m_statsMutex);
    m_stats = CacheStats();
}

ResourceMetadata ResourceManager::getMetadata(const ResourcePath& path) const {
    ResourceMetadata metadata;
    metadata.path = path;
    metadata.name = path.getStem();
    
    std::string physPath = getPhysicalPath(path);
    if (!physPath.empty() && fs::exists(physPath)) {
        metadata.fileSize = static_cast<u64>(fs::file_size(physPath));
        auto modTime = fs::last_write_time(physPath);
        metadata.lastModified = static_cast<u64>(modTime.time_since_epoch().count());
    }
    
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    auto it = m_pathToId.find(path);
    if (it != m_pathToId.end()) {
        auto resIt = m_resources.find(it->second);
        if (resIt != m_resources.end()) {
            metadata.id = resIt->second->getId();
            metadata.type = resIt->second->getType();
        }
    }
    
    return metadata;
}

std::vector<ResourcePath> ResourceManager::getLoadedPaths() const {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    std::vector<ResourcePath> paths;
    for (const auto& [path, id] : m_pathToId) {
        auto it = m_resources.find(id);
        if (it != m_resources.end() && it->second->isLoaded()) {
            paths.push_back(path);
        }
    }
    return paths;
}

std::vector<ResourceHandle<>> ResourceManager::getResourcesByType(ResourceType type) const {
    std::lock_guard<std::mutex> lock(m_resourceMutex);
    
    std::vector<ResourceHandle<>> result;
    for (const auto& [id, resource] : m_resources) {
        if (resource->getType() == type) {
            result.push_back(ResourceHandle<>(resource));
        }
    }
    return result;
}

// ============================================================================
// Dependencies
// ============================================================================

std::vector<ResourceId> ResourceManager::getDependencies(ResourceId id) const {
    auto it = m_dependencies.find(id);
    return it != m_dependencies.end() ? it->second : std::vector<ResourceId>();
}

std::vector<ResourceId> ResourceManager::getDependents(ResourceId id) const {
    auto it = m_dependents.find(id);
    return it != m_dependents.end() ? it->second : std::vector<ResourceId>();
}

void ResourceManager::addDependency(ResourceId dependent, ResourceId dependency) {
    m_dependencies[dependent].push_back(dependency);
    m_dependents[dependency].push_back(dependent);
}

// ============================================================================
// Worker Thread
// ============================================================================

void ResourceManager::workerThread() {
    while (m_running) {
        LoadRequest request;
        
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_queueCV.wait(lock, [this] { return !m_running || !m_loadQueue.empty(); });
            
            if (!m_running) break;
            if (m_loadQueue.empty()) continue;
            
            request = m_loadQueue.top();
            m_loadQueue.pop();
        }
        
        // Process load request
        auto handle = loadInternal(request.path, request.flags & ~LoadFlags::Async);
        
        if (request.onComplete && handle.isValid()) {
            request.onComplete(handle);
        } else if (request.onError && !handle.isValid()) {
            request.onError("Failed to load resource: " + request.path.path);
        }
    }
}

} // namespace nova::resource
