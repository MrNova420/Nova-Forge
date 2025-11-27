/**
 * @file resource_types.hpp
 * @brief NovaCore Resource System™ - Core Type Definitions
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 * 
 * Defines the fundamental types for the resource/asset management system:
 * - Resource handles and references
 * - Asset types and metadata
 * - Loading states and callbacks
 * - Streaming and caching
 */

#pragma once

#include <nova/core/types/types.hpp>

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>

namespace nova::resource {

using namespace nova::core;

// ============================================================================
// Configuration Constants
// ============================================================================

namespace ResourceConfig {
    constexpr usize DEFAULT_CACHE_SIZE = 256_MB;
    constexpr usize MAX_CONCURRENT_LOADS = 8;
    constexpr u32 MAX_RESOURCE_NAME_LENGTH = 256;
    constexpr f32 DEFAULT_UNLOAD_DELAY = 30.0f;  // Seconds
    constexpr u32 RESOURCE_POOL_INITIAL_SIZE = 1024;
}

// ============================================================================
// Resource Types
// ============================================================================

/**
 * @brief Built-in resource types
 */
enum class ResourceType : u16 {
    Unknown = 0,
    
    // Textures
    Texture2D = 100,
    Texture3D,
    TextureCube,
    TextureArray,
    RenderTarget,
    
    // Meshes
    Mesh = 200,
    SkeletalMesh,
    StaticMesh,
    
    // Materials
    Material = 300,
    Shader,
    ShaderProgram,
    
    // Audio
    AudioClip = 400,
    AudioBank,
    
    // Animation
    Animation = 500,
    AnimationGraph,
    Skeleton,
    
    // Prefabs/Scenes
    Prefab = 600,
    Scene,
    Level,
    
    // Scripts
    Script = 700,
    VisualScript,
    
    // Physics
    PhysicsMaterial = 800,
    CollisionMesh,
    
    // Data
    DataTable = 900,
    Config,
    Localization,
    
    // Fonts
    Font = 1000,
    FontAtlas,
    
    // UI
    UILayout = 1100,
    UIStyle,
    
    // Custom
    Custom = 10000
};

/**
 * @brief Resource loading state
 */
enum class ResourceState : u8 {
    Unloaded,       // Not in memory
    Queued,         // Waiting to load
    Loading,        // Currently loading
    Processing,     // Post-load processing
    Loaded,         // Ready to use
    Failed,         // Load failed
    Unloading       // Being unloaded
};

/**
 * @brief Resource load priority
 */
enum class LoadPriority : u8 {
    Background = 0,  // Load when idle
    Low = 64,
    Normal = 128,
    High = 192,
    Immediate = 255  // Block until loaded
};

/**
 * @brief Resource load flags
 */
enum class LoadFlags : u32 {
    None = 0,
    Async = 1 << 0,           // Load asynchronously
    Stream = 1 << 1,          // Stream from disk
    KeepSourceData = 1 << 2,  // Keep original data
    NoCache = 1 << 3,         // Don't cache
    Compressed = 1 << 4,      // Data is compressed
    Encrypted = 1 << 5,       // Data is encrypted
    HotReload = 1 << 6,       // Enable hot-reload
    Persistent = 1 << 7,      // Never unload
    Preload = 1 << 8          // Load before scene
};

inline LoadFlags operator|(LoadFlags a, LoadFlags b) {
    return static_cast<LoadFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

inline LoadFlags operator&(LoadFlags a, LoadFlags b) {
    return static_cast<LoadFlags>(static_cast<u32>(a) & static_cast<u32>(b));
}

inline bool hasFlag(LoadFlags flags, LoadFlags flag) {
    return (static_cast<u32>(flags) & static_cast<u32>(flag)) != 0;
}

// ============================================================================
// Resource Identification
// ============================================================================

/**
 * @brief Unique resource identifier
 */
struct ResourceId {
    u64 value = 0;
    
    ResourceId() = default;
    explicit ResourceId(u64 v) : value(v) {}
    
    bool isValid() const { return value != 0; }
    operator u64() const { return value; }
    
    bool operator==(const ResourceId& other) const { return value == other.value; }
    bool operator!=(const ResourceId& other) const { return value != other.value; }
    bool operator<(const ResourceId& other) const { return value < other.value; }
    
    static ResourceId invalid() { return ResourceId(0); }
    static ResourceId generate();
    static ResourceId fromPath(const std::string& path);
};

/**
 * @brief Resource path (virtual file path)
 */
struct ResourcePath {
    std::string path;
    
    ResourcePath() = default;
    ResourcePath(const char* p) : path(p) {}
    ResourcePath(const std::string& p) : path(p) {}
    
    bool isEmpty() const { return path.empty(); }
    bool isValid() const { return !path.empty(); }
    
    std::string getExtension() const;
    std::string getFilename() const;
    std::string getDirectory() const;
    std::string getStem() const;  // Filename without extension
    
    ResourcePath join(const std::string& other) const;
    ResourcePath parent() const;
    
    bool operator==(const ResourcePath& other) const { return path == other.path; }
    bool operator!=(const ResourcePath& other) const { return path != other.path; }
    
    operator const std::string&() const { return path; }
    const char* c_str() const { return path.c_str(); }
};

// ============================================================================
// Resource Handle
// ============================================================================

/**
 * @brief Forward declaration of Resource base
 */
class Resource;

/**
 * @brief Type-safe resource handle
 * 
 * ResourceHandle provides reference-counted access to resources.
 * When all handles are released, the resource may be unloaded.
 */
template<typename T = Resource>
class ResourceHandle {
public:
    ResourceHandle() = default;
    ResourceHandle(std::nullptr_t) : m_resource(nullptr) {}
    
    explicit ResourceHandle(std::shared_ptr<T> resource)
        : m_resource(std::move(resource)) {}
    
    // Access
    T* get() const { return m_resource.get(); }
    T* operator->() const { return m_resource.get(); }
    T& operator*() const { return *m_resource; }
    
    // State
    bool isValid() const { return m_resource != nullptr; }
    bool isLoaded() const;
    bool isLoading() const;
    
    explicit operator bool() const { return isValid(); }
    
    // Comparison
    bool operator==(const ResourceHandle& other) const { return m_resource == other.m_resource; }
    bool operator!=(const ResourceHandle& other) const { return m_resource != other.m_resource; }
    
    // Conversion
    template<typename U>
    ResourceHandle<U> cast() const {
        return ResourceHandle<U>(std::dynamic_pointer_cast<U>(m_resource));
    }
    
    // Release
    void reset() { m_resource.reset(); }
    
private:
    std::shared_ptr<T> m_resource;
};

// ============================================================================
// Resource Base Class
// ============================================================================

/**
 * @brief Base class for all loadable resources
 */
class Resource : public std::enable_shared_from_this<Resource> {
public:
    virtual ~Resource() = default;
    
    // Identification
    ResourceId getId() const { return m_id; }
    const ResourcePath& getPath() const { return m_path; }
    ResourceType getType() const { return m_type; }
    const std::string& getName() const { return m_name; }
    
    // State
    ResourceState getState() const { return m_state; }
    bool isLoaded() const { return m_state == ResourceState::Loaded; }
    bool isLoading() const { return m_state == ResourceState::Loading || m_state == ResourceState::Queued; }
    bool isFailed() const { return m_state == ResourceState::Failed; }
    
    // Metadata
    usize getMemorySize() const { return m_memorySize; }
    u64 getLastAccessTime() const { return m_lastAccessTime; }
    u32 getReferenceCount() const { return m_refCount; }
    
    // Error info
    const std::string& getErrorMessage() const { return m_errorMessage; }
    
    // Callbacks
    void onLoaded(std::function<void(Resource*)> callback);
    void onFailed(std::function<void(Resource*, const std::string&)> callback);
    
protected:
    friend class ResourceManager;
    
    Resource() = default;
    
    virtual bool load(const std::vector<u8>& data) = 0;
    virtual void unload() = 0;
    virtual usize calculateMemorySize() const { return 0; }
    
    void setState(ResourceState state);
    void setError(const std::string& message);
    void updateAccessTime();
    
    ResourceId m_id;
    ResourcePath m_path;
    ResourceType m_type = ResourceType::Unknown;
    std::string m_name;
    ResourceState m_state = ResourceState::Unloaded;
    LoadFlags m_flags = LoadFlags::None;
    LoadPriority m_priority = LoadPriority::Normal;
    
    usize m_memorySize = 0;
    u64 m_lastAccessTime = 0;
    u32 m_refCount = 0;
    
    std::string m_errorMessage;
    std::vector<std::function<void(Resource*)>> m_loadedCallbacks;
    std::vector<std::function<void(Resource*, const std::string&)>> m_failedCallbacks;
};

// ============================================================================
// Resource Metadata
// ============================================================================

/**
 * @brief Resource metadata (stored in asset database)
 */
struct ResourceMetadata {
    ResourceId id;
    ResourcePath path;
    ResourceType type = ResourceType::Unknown;
    std::string name;
    
    // File info
    u64 fileSize = 0;
    u64 compressedSize = 0;
    u64 lastModified = 0;
    std::string hash;  // Content hash for integrity
    
    // Dependencies
    std::vector<ResourceId> dependencies;
    std::vector<ResourceId> dependents;
    
    // Editor info
    std::string importerName;
    std::string sourceFile;
    std::vector<std::pair<std::string, std::string>> importSettings;
    
    // Tags for searching
    std::vector<std::string> tags;
    
    // Custom data
    std::string customData;
};

/**
 * @brief Resource bundle (package of resources)
 */
struct ResourceBundle {
    std::string name;
    ResourcePath path;
    std::vector<ResourceId> resources;
    
    u64 totalSize = 0;
    bool isLoaded = false;
    bool isCompressed = false;
    bool isEncrypted = false;
    
    // Streaming info
    u64 streamOffset = 0;
    u64 streamSize = 0;
};

// ============================================================================
// Loading Request
// ============================================================================

/**
 * @brief Resource load request
 */
struct LoadRequest {
    ResourceId id;
    ResourcePath path;
    ResourceType type = ResourceType::Unknown;
    LoadPriority priority = LoadPriority::Normal;
    LoadFlags flags = LoadFlags::None;
    
    std::function<void(ResourceHandle<>)> onComplete;
    std::function<void(const std::string&)> onError;
    std::function<void(f32)> onProgress;
};

/**
 * @brief Batch load request
 */
struct BatchLoadRequest {
    std::vector<ResourcePath> paths;
    LoadPriority priority = LoadPriority::Normal;
    LoadFlags flags = LoadFlags::None;
    
    std::function<void(const std::vector<ResourceHandle<>>&)> onComplete;
    std::function<void(const std::string&, const ResourcePath&)> onError;
    std::function<void(f32, u32 loaded, u32 total)> onProgress;
};

// ============================================================================
// Cache Statistics
// ============================================================================

/**
 * @brief Resource cache statistics
 */
struct CacheStats {
    u32 totalResources = 0;
    u32 loadedResources = 0;
    u32 loadingResources = 0;
    u32 failedResources = 0;
    
    usize totalMemory = 0;
    usize usedMemory = 0;
    usize cacheSize = 0;
    
    u32 cacheHits = 0;
    u32 cacheMisses = 0;
    f32 hitRate = 0.0f;
    
    u32 loadRequestsQueued = 0;
    u32 loadRequestsActive = 0;
    u32 loadRequestsCompleted = 0;
    u32 loadRequestsFailed = 0;
    
    f64 averageLoadTime = 0.0;
    f64 totalLoadTime = 0.0;
};

// ============================================================================
// Resource Loader Interface
// ============================================================================

/**
 * @brief Resource loader interface
 */
class IResourceLoader {
public:
    virtual ~IResourceLoader() = default;
    
    /**
     * @brief Get supported extensions
     */
    virtual std::vector<std::string> getSupportedExtensions() const = 0;
    
    /**
     * @brief Get resource type this loader produces
     */
    virtual ResourceType getResourceType() const = 0;
    
    /**
     * @brief Check if loader can handle this file
     */
    virtual bool canLoad(const ResourcePath& path) const = 0;
    
    /**
     * @brief Create resource instance
     */
    virtual std::shared_ptr<Resource> createResource() = 0;
    
    /**
     * @brief Load resource data
     */
    virtual bool load(Resource* resource, const std::vector<u8>& data) = 0;
    
    /**
     * @brief Get loader name
     */
    virtual const char* getName() const = 0;
};

// ============================================================================
// Callbacks
// ============================================================================

using ResourceLoadedCallback = std::function<void(ResourceHandle<>)>;
using ResourceErrorCallback = std::function<void(const std::string&)>;
using ResourceProgressCallback = std::function<void(f32)>;

// ============================================================================
// Inline Implementations
// ============================================================================

inline std::string ResourcePath::getExtension() const {
    auto pos = path.rfind('.');
    if (pos == std::string::npos) return "";
    return path.substr(pos + 1);
}

inline std::string ResourcePath::getFilename() const {
    auto pos = path.rfind('/');
    if (pos == std::string::npos) return path;
    return path.substr(pos + 1);
}

inline std::string ResourcePath::getDirectory() const {
    auto pos = path.rfind('/');
    if (pos == std::string::npos) return "";
    return path.substr(0, pos);
}

inline std::string ResourcePath::getStem() const {
    std::string filename = getFilename();
    auto pos = filename.rfind('.');
    if (pos == std::string::npos) return filename;
    return filename.substr(0, pos);
}

inline ResourcePath ResourcePath::join(const std::string& other) const {
    if (path.empty()) return ResourcePath(other);
    if (path.back() == '/') return ResourcePath(path + other);
    return ResourcePath(path + "/" + other);
}

inline ResourcePath ResourcePath::parent() const {
    auto pos = path.rfind('/');
    if (pos == std::string::npos) return ResourcePath("");
    return ResourcePath(path.substr(0, pos));
}

template<typename T>
bool ResourceHandle<T>::isLoaded() const {
    return m_resource && m_resource->isLoaded();
}

template<typename T>
bool ResourceHandle<T>::isLoading() const {
    return m_resource && m_resource->isLoading();
}

inline void Resource::onLoaded(std::function<void(Resource*)> callback) {
    if (m_state == ResourceState::Loaded) {
        callback(this);
    } else {
        m_loadedCallbacks.push_back(std::move(callback));
    }
}

inline void Resource::onFailed(std::function<void(Resource*, const std::string&)> callback) {
    if (m_state == ResourceState::Failed) {
        callback(this, m_errorMessage);
    } else {
        m_failedCallbacks.push_back(std::move(callback));
    }
}

inline void Resource::setState(ResourceState state) {
    m_state = state;
    
    if (state == ResourceState::Loaded) {
        m_memorySize = calculateMemorySize();
        for (auto& cb : m_loadedCallbacks) {
            cb(this);
        }
        m_loadedCallbacks.clear();
    } else if (state == ResourceState::Failed) {
        for (auto& cb : m_failedCallbacks) {
            cb(this, m_errorMessage);
        }
        m_failedCallbacks.clear();
    }
}

inline void Resource::setError(const std::string& message) {
    m_errorMessage = message;
    setState(ResourceState::Failed);
}

inline void Resource::updateAccessTime() {
    auto now = std::chrono::steady_clock::now();
    m_lastAccessTime = static_cast<u64>(now.time_since_epoch().count());
}

} // namespace nova::resource

// Hash for ResourceId
namespace std {
    template<>
    struct hash<nova::resource::ResourceId> {
        size_t operator()(const nova::resource::ResourceId& id) const {
            return std::hash<u64>()(id.value);
        }
    };
    
    template<>
    struct hash<nova::resource::ResourcePath> {
        size_t operator()(const nova::resource::ResourcePath& path) const {
            return std::hash<std::string>()(path.path);
        }
    };
}
