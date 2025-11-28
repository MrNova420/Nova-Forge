/**
 * @file ray_tracing.hpp
 * @brief Hardware ray tracing system for NovaCore engine
 * @author NovaForge Team
 * @copyright Copyright (c) 2024-2025 NovaForge - All Rights Reserved
 * 
 * Provides comprehensive ray tracing support including:
 * - Acceleration structure management (BLAS/TLAS)
 * - Ray tracing pipeline support
 * - Hybrid rendering integration
 * - Software fallback for unsupported hardware
 */

#pragma once

#include <nova/core/types/types.hpp>
#include <nova/core/math/math.hpp>
#include <array>
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <cstring>
#include <algorithm>

namespace nova {

// =============================================================================
// RAY TRACING CONFIGURATION
// =============================================================================

/**
 * @brief Ray tracing system configuration constants
 */
struct RTConfig {
    /// Maximum ray recursion depth
    static constexpr u32 MAX_RECURSION_DEPTH = 31;
    
    /// Maximum miss shaders
    static constexpr u32 MAX_MISS_SHADERS = 8;
    
    /// Maximum hit shaders
    static constexpr u32 MAX_HIT_SHADERS = 256;
    
    /// Maximum callable shaders
    static constexpr u32 MAX_CALLABLE_SHADERS = 64;
    
    /// Maximum geometry per BLAS
    static constexpr u32 MAX_GEOMETRY_PER_BLAS = 1024;
    
    /// Maximum instances per TLAS
    static constexpr u32 MAX_INSTANCES_PER_TLAS = 1000000;
    
    /// Default ray tracing resolution scale
    static constexpr f32 DEFAULT_RT_SCALE = 0.5f;
    
    /// Default samples per pixel for path tracing
    static constexpr u32 DEFAULT_SPP = 1;
    
    /// Maximum bounces for global illumination
    static constexpr u32 MAX_GI_BOUNCES = 4;
    
    /// Shader binding table alignment
    static constexpr u32 SBT_ALIGNMENT = 64;
};

// =============================================================================
// RAY TRACING ENUMERATIONS
// =============================================================================

/**
 * @brief Ray tracing technique selection
 */
enum class RTTechnique : u8 {
    None,           ///< Disabled (use rasterization only)
    Shadows,        ///< Ray traced shadows only
    AO,             ///< Ray traced ambient occlusion
    Reflections,    ///< Ray traced reflections
    GI,             ///< Ray traced global illumination
    PathTracing,    ///< Full path tracing
    Hybrid          ///< Mixed rasterization + ray tracing
};

/**
 * @brief Get string name for RT technique
 */
[[nodiscard]] constexpr const char* getRTTechniqueName(RTTechnique technique) noexcept {
    switch (technique) {
        case RTTechnique::None: return "None";
        case RTTechnique::Shadows: return "Shadows";
        case RTTechnique::AO: return "Ambient Occlusion";
        case RTTechnique::Reflections: return "Reflections";
        case RTTechnique::GI: return "Global Illumination";
        case RTTechnique::PathTracing: return "Path Tracing";
        case RTTechnique::Hybrid: return "Hybrid";
        default: return "Unknown";
    }
}

/**
 * @brief Acceleration structure type
 */
enum class AccelerationStructureType : u8 {
    BottomLevel,    ///< BLAS - geometry level
    TopLevel        ///< TLAS - instance level
};

/**
 * @brief Geometry type for BLAS
 */
enum class RTGeometryType : u8 {
    Triangles,      ///< Triangle meshes
    AABBs,          ///< Procedural geometry with AABBs
    Instances       ///< Instances (for TLAS)
};

/**
 * @brief Build flags for acceleration structures
 */
enum class ASBuildFlags : u32 {
    None = 0,
    AllowUpdate = 1 << 0,       ///< Allow incremental updates
    AllowCompaction = 1 << 1,   ///< Allow memory compaction
    PreferFastTrace = 1 << 2,   ///< Optimize for trace performance
    PreferFastBuild = 1 << 3,   ///< Optimize for build time
    LowMemory = 1 << 4          ///< Minimize memory usage
};

inline ASBuildFlags operator|(ASBuildFlags a, ASBuildFlags b) noexcept {
    return static_cast<ASBuildFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

inline ASBuildFlags operator&(ASBuildFlags a, ASBuildFlags b) noexcept {
    return static_cast<ASBuildFlags>(static_cast<u32>(a) & static_cast<u32>(b));
}

inline bool hasFlag(ASBuildFlags flags, ASBuildFlags flag) noexcept {
    return (static_cast<u32>(flags) & static_cast<u32>(flag)) != 0;
}

/**
 * @brief Ray flags for trace operations
 */
enum class RayFlags : u32 {
    None = 0,
    Opaque = 1 << 0,                    ///< All hit shaders are opaque
    NoOpaque = 1 << 1,                  ///< No hit shaders are opaque
    TerminateOnFirstHit = 1 << 2,       ///< Stop at first intersection
    SkipClosestHitShader = 1 << 3,      ///< Skip closest hit shader
    CullBackFacingTriangles = 1 << 4,   ///< Cull back-facing triangles
    CullFrontFacingTriangles = 1 << 5,  ///< Cull front-facing triangles
    CullOpaque = 1 << 6,                ///< Cull opaque geometry
    CullNoOpaque = 1 << 7,              ///< Cull non-opaque geometry
    SkipTriangles = 1 << 8,             ///< Skip triangle geometry
    SkipAABBs = 1 << 9                  ///< Skip AABB geometry
};

inline RayFlags operator|(RayFlags a, RayFlags b) noexcept {
    return static_cast<RayFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

inline RayFlags operator&(RayFlags a, RayFlags b) noexcept {
    return static_cast<RayFlags>(static_cast<u32>(a) & static_cast<u32>(b));
}

/**
 * @brief Denoiser type for ray traced output
 */
enum class RTDenoiser : u8 {
    None,           ///< No denoising
    Temporal,       ///< Temporal accumulation
    Spatial,        ///< Spatial filter (bilateral/NLM)
    SVGF,           ///< Spatiotemporal Variance-Guided Filter
    NRD,            ///< NVIDIA Real-time Denoiser
    OIDN,           ///< Intel Open Image Denoise
    Custom          ///< Custom denoiser
};

/**
 * @brief Ray type for shader binding table indexing
 */
enum class RayType : u8 {
    Primary,        ///< Primary camera rays
    Shadow,         ///< Shadow rays
    Reflection,     ///< Reflection rays
    Refraction,     ///< Refraction rays
    AO,             ///< Ambient occlusion rays
    GI,             ///< Global illumination rays
    Count
};

// =============================================================================
// RAY TRACING STRUCTURES
// =============================================================================

/**
 * @brief Ray description for tracing
 */
struct alignas(16) Ray {
    Vec3 origin{0.0f, 0.0f, 0.0f};
    f32 tMin{0.001f};
    Vec3 direction{0.0f, 0.0f, -1.0f};
    f32 tMax{10000.0f};
    
    /// Create ray from origin and direction
    static Ray create(const Vec3& o, const Vec3& d, f32 minT = 0.001f, f32 maxT = 10000.0f) noexcept {
        Ray ray;
        ray.origin = o;
        ray.direction = d.normalized();
        ray.tMin = minT;
        ray.tMax = maxT;
        return ray;
    }
    
    /// Get point along ray at parameter t
    [[nodiscard]] Vec3 at(f32 t) const noexcept {
        return origin + direction * t;
    }
};

/**
 * @brief Ray hit information
 */
struct RayHit {
    Vec3 position{0.0f, 0.0f, 0.0f};     ///< Hit position in world space
    Vec3 normal{0.0f, 1.0f, 0.0f};       ///< Surface normal at hit
    Vec3 geometryNormal{0.0f, 1.0f, 0.0f}; ///< Geometric (non-interpolated) normal
    Vec2 barycentrics{0.0f, 0.0f};       ///< Barycentric coordinates
    f32 t{0.0f};                          ///< Ray parameter at hit
    u32 instanceID{0};                    ///< Instance identifier
    u32 primitiveID{0};                   ///< Primitive (triangle) identifier
    u32 geometryID{0};                    ///< Geometry identifier within BLAS
    u32 materialID{0};                    ///< Material identifier
    bool frontFace{true};                 ///< True if hit front face
    
    [[nodiscard]] bool isValid() const noexcept {
        return t > 0.0f;
    }
};

/**
 * @brief Triangle geometry description for BLAS
 */
struct RTTriangleGeometry {
    u64 vertexBufferAddress{0};          ///< Device address of vertex buffer
    u64 indexBufferAddress{0};           ///< Device address of index buffer
    u32 vertexStride{12};                ///< Stride between vertices (bytes)
    u32 vertexCount{0};                  ///< Number of vertices
    u32 indexCount{0};                   ///< Number of indices
    u32 triangleCount{0};                ///< Number of triangles
    bool isOpaque{true};                 ///< True for opaque geometry
    u32 materialID{0};                   ///< Associated material
    
    /// Create from buffer addresses
    static RTTriangleGeometry create(u64 vertexAddr, u64 indexAddr,
                                     u32 vertexStride, u32 vertexCount,
                                     u32 indexCount) noexcept {
        RTTriangleGeometry geom;
        geom.vertexBufferAddress = vertexAddr;
        geom.indexBufferAddress = indexAddr;
        geom.vertexStride = vertexStride;
        geom.vertexCount = vertexCount;
        geom.indexCount = indexCount;
        geom.triangleCount = indexCount / 3;
        return geom;
    }
};

/**
 * @brief AABB geometry description for procedural geometry
 */
struct RTAABBGeometry {
    u64 aabbBufferAddress{0};            ///< Device address of AABB buffer
    u32 aabbCount{0};                    ///< Number of AABBs
    u32 aabbStride{24};                  ///< Stride between AABBs (bytes)
    bool isOpaque{true};                 ///< True for opaque geometry
    u32 intersectionShaderIndex{0};      ///< Custom intersection shader index
};

/**
 * @brief Bottom-level acceleration structure description
 */
struct BLASDesc {
    std::string name;
    std::vector<RTTriangleGeometry> triangleGeometries;
    std::vector<RTAABBGeometry> aabbGeometries;
    ASBuildFlags buildFlags{ASBuildFlags::PreferFastTrace};
    
    /// Add triangle geometry
    void addTriangles(const RTTriangleGeometry& geom) {
        triangleGeometries.push_back(geom);
    }
    
    /// Add AABB geometry
    void addAABBs(const RTAABBGeometry& geom) {
        aabbGeometries.push_back(geom);
    }
    
    /// Get total geometry count
    [[nodiscard]] u32 getGeometryCount() const noexcept {
        return static_cast<u32>(triangleGeometries.size() + aabbGeometries.size());
    }
    
    /// Check if valid
    [[nodiscard]] bool isValid() const noexcept {
        return !triangleGeometries.empty() || !aabbGeometries.empty();
    }
};

/**
 * @brief Instance description for TLAS
 */
struct alignas(64) RTInstance {
    Mat4 transform{Mat4::identity()};    ///< 3x4 row-major transform
    u32 instanceID{0};                   ///< Custom instance ID (24 bits)
    u8 mask{0xFF};                       ///< Instance visibility mask
    u32 sbtOffset{0};                    ///< Shader binding table offset
    u32 flags{0};                        ///< Instance flags
    u64 blasAddress{0};                  ///< Device address of BLAS
    
    /// Create instance with transform
    static RTInstance create(const Mat4& xform, u32 id, u64 blas) noexcept {
        RTInstance inst;
        inst.transform = xform;
        inst.instanceID = id & 0x00FFFFFF;
        inst.blasAddress = blas;
        return inst;
    }
    
    /// Set visibility mask
    void setMask(u8 m) noexcept { mask = m; }
    
    /// Set custom ID
    void setCustomIndex(u32 idx) noexcept { instanceID = idx & 0x00FFFFFF; }
};

/**
 * @brief Top-level acceleration structure description
 */
struct TLASDesc {
    std::string name;
    std::vector<RTInstance> instances;
    ASBuildFlags buildFlags{ASBuildFlags::PreferFastTrace | ASBuildFlags::AllowUpdate};
    
    /// Add instance
    void addInstance(const RTInstance& inst) {
        instances.push_back(inst);
    }
    
    /// Get instance count
    [[nodiscard]] u32 getInstanceCount() const noexcept {
        return static_cast<u32>(instances.size());
    }
    
    /// Update instance transform
    void updateInstanceTransform(u32 index, const Mat4& transform) noexcept {
        if (index < instances.size()) {
            instances[index].transform = transform;
        }
    }
    
    /// Check if valid
    [[nodiscard]] bool isValid() const noexcept {
        return !instances.empty();
    }
};

/**
 * @brief Acceleration structure handle
 */
struct AccelerationStructure {
    u64 handle{0};                       ///< GPU handle
    u64 deviceAddress{0};                ///< Device address for binding
    u64 bufferSize{0};                   ///< Memory size in bytes
    u64 scratchSize{0};                  ///< Scratch buffer size for build
    AccelerationStructureType type{AccelerationStructureType::BottomLevel};
    bool isBuilt{false};
    
    [[nodiscard]] bool isValid() const noexcept {
        return handle != 0 && deviceAddress != 0;
    }
};

/**
 * @brief Shader binding table entry
 */
struct SBTEntry {
    u64 shaderHandle{0};                 ///< Shader group handle
    std::vector<u8> localData;           ///< Local root arguments
    
    /// Set local data
    template<typename T>
    void setLocalData(const T& data) {
        localData.resize(sizeof(T));
        std::memcpy(localData.data(), &data, sizeof(T));
    }
    
    /// Get total size with alignment
    [[nodiscard]] u32 getAlignedSize(u32 handleSize, u32 alignment) const noexcept {
        u32 size = handleSize + static_cast<u32>(localData.size());
        return (size + alignment - 1) & ~(alignment - 1);
    }
};

/**
 * @brief Shader binding table
 */
struct ShaderBindingTable {
    /// Ray generation entries
    std::vector<SBTEntry> rayGenEntries;
    
    /// Miss shader entries
    std::vector<SBTEntry> missEntries;
    
    /// Hit group entries
    std::vector<SBTEntry> hitGroupEntries;
    
    /// Callable shader entries
    std::vector<SBTEntry> callableEntries;
    
    /// Buffer handle
    u64 bufferHandle{0};
    u64 bufferAddress{0};
    
    /// Region offsets and strides
    u64 rayGenOffset{0};
    u64 rayGenStride{0};
    u64 missOffset{0};
    u64 missStride{0};
    u64 hitGroupOffset{0};
    u64 hitGroupStride{0};
    u64 callableOffset{0};
    u64 callableStride{0};
    
    /// Add ray generation entry
    void addRayGen(const SBTEntry& entry) {
        rayGenEntries.push_back(entry);
    }
    
    /// Add miss entry
    void addMiss(const SBTEntry& entry) {
        missEntries.push_back(entry);
    }
    
    /// Add hit group entry
    void addHitGroup(const SBTEntry& entry) {
        hitGroupEntries.push_back(entry);
    }
    
    /// Add callable entry
    void addCallable(const SBTEntry& entry) {
        callableEntries.push_back(entry);
    }
    
    /// Calculate total buffer size
    [[nodiscard]] u64 calculateBufferSize(u32 handleSize, u32 alignment) const noexcept {
        u64 size = 0;
        
        // Ray gen (single entry, aligned)
        if (!rayGenEntries.empty()) {
            size += rayGenEntries[0].getAlignedSize(handleSize, alignment);
        }
        size = (size + RTConfig::SBT_ALIGNMENT - 1) & ~(RTConfig::SBT_ALIGNMENT - 1);
        
        // Miss entries
        for (const auto& entry : missEntries) {
            size += entry.getAlignedSize(handleSize, alignment);
        }
        size = (size + RTConfig::SBT_ALIGNMENT - 1) & ~(RTConfig::SBT_ALIGNMENT - 1);
        
        // Hit group entries
        for (const auto& entry : hitGroupEntries) {
            size += entry.getAlignedSize(handleSize, alignment);
        }
        size = (size + RTConfig::SBT_ALIGNMENT - 1) & ~(RTConfig::SBT_ALIGNMENT - 1);
        
        // Callable entries
        for (const auto& entry : callableEntries) {
            size += entry.getAlignedSize(handleSize, alignment);
        }
        
        return size;
    }
};

// =============================================================================
// RAY TRACING SETTINGS
// =============================================================================

/**
 * @brief Ray traced shadows settings
 */
struct RTShadowSettings {
    bool enabled{true};
    u32 samplesPerPixel{1};              ///< Samples for soft shadows
    f32 softShadowRadius{0.05f};         ///< Light source radius for soft shadows
    f32 maxDistance{1000.0f};            ///< Maximum shadow ray distance
    bool denoise{true};                  ///< Enable denoising
    
    /// Factory for hard shadows
    static RTShadowSettings hardShadows() noexcept {
        RTShadowSettings settings;
        settings.samplesPerPixel = 1;
        settings.softShadowRadius = 0.0f;
        return settings;
    }
    
    /// Factory for soft shadows
    static RTShadowSettings softShadows(u32 samples = 4, f32 radius = 0.1f) noexcept {
        RTShadowSettings settings;
        settings.samplesPerPixel = samples;
        settings.softShadowRadius = radius;
        return settings;
    }
};

/**
 * @brief Ray traced ambient occlusion settings
 */
struct RTAOSettings {
    bool enabled{true};
    u32 samplesPerPixel{4};              ///< AO ray samples
    f32 radius{2.0f};                    ///< AO effect radius
    f32 intensity{1.0f};                 ///< AO intensity multiplier
    f32 bias{0.01f};                     ///< Ray origin bias
    bool denoise{true};                  ///< Enable denoising
    
    /// Factory for low quality
    static RTAOSettings low() noexcept {
        RTAOSettings settings;
        settings.samplesPerPixel = 1;
        settings.radius = 1.0f;
        return settings;
    }
    
    /// Factory for medium quality
    static RTAOSettings medium() noexcept {
        RTAOSettings settings;
        settings.samplesPerPixel = 4;
        settings.radius = 2.0f;
        return settings;
    }
    
    /// Factory for high quality
    static RTAOSettings high() noexcept {
        RTAOSettings settings;
        settings.samplesPerPixel = 8;
        settings.radius = 3.0f;
        return settings;
    }
};

/**
 * @brief Ray traced reflections settings
 */
struct RTReflectionSettings {
    bool enabled{true};
    u32 maxBounces{2};                   ///< Maximum reflection bounces
    f32 maxRoughness{0.5f};              ///< Max roughness for RT reflections
    f32 resolutionScale{0.5f};           ///< Render resolution scale
    bool denoise{true};                  ///< Enable denoising
    bool enableTransparency{false};      ///< Enable transparent reflections
    
    /// Factory for performance mode
    static RTReflectionSettings performance() noexcept {
        RTReflectionSettings settings;
        settings.maxBounces = 1;
        settings.maxRoughness = 0.3f;
        settings.resolutionScale = 0.5f;
        return settings;
    }
    
    /// Factory for quality mode
    static RTReflectionSettings quality() noexcept {
        RTReflectionSettings settings;
        settings.maxBounces = 3;
        settings.maxRoughness = 0.7f;
        settings.resolutionScale = 0.75f;
        return settings;
    }
};

/**
 * @brief Ray traced global illumination settings
 */
struct RTGISettings {
    bool enabled{true};
    u32 samplesPerPixel{1};              ///< GI samples per pixel
    u32 maxBounces{2};                   ///< Maximum light bounces
    f32 intensity{1.0f};                 ///< GI intensity multiplier
    f32 maxDistance{100.0f};             ///< Maximum GI ray distance
    f32 resolutionScale{0.5f};           ///< Render resolution scale
    bool denoise{true};                  ///< Enable denoising
    bool useReservoirs{true};            ///< Use ReSTIR algorithm
    
    /// Factory for low quality
    static RTGISettings low() noexcept {
        RTGISettings settings;
        settings.samplesPerPixel = 1;
        settings.maxBounces = 1;
        settings.resolutionScale = 0.25f;
        return settings;
    }
    
    /// Factory for medium quality
    static RTGISettings medium() noexcept {
        RTGISettings settings;
        settings.samplesPerPixel = 1;
        settings.maxBounces = 2;
        settings.resolutionScale = 0.5f;
        return settings;
    }
    
    /// Factory for high quality
    static RTGISettings high() noexcept {
        RTGISettings settings;
        settings.samplesPerPixel = 2;
        settings.maxBounces = 3;
        settings.resolutionScale = 0.75f;
        return settings;
    }
};

/**
 * @brief Path tracing settings
 */
struct PathTracingSettings {
    bool enabled{false};
    u32 samplesPerPixel{1};              ///< SPP per frame
    u32 maxAccumulatedSamples{1024};     ///< Max samples to accumulate
    u32 maxBounces{8};                   ///< Maximum path depth
    f32 russianRouletteDepth{3};         ///< Depth to start RR termination
    bool denoise{true};                  ///< Enable denoising
    bool enableNEE{true};                ///< Enable next event estimation
    bool enableMIS{true};                ///< Enable multiple importance sampling
    
    /// Factory for real-time
    static PathTracingSettings realtime() noexcept {
        PathTracingSettings settings;
        settings.samplesPerPixel = 1;
        settings.maxBounces = 4;
        settings.denoise = true;
        return settings;
    }
    
    /// Factory for progressive
    static PathTracingSettings progressive() noexcept {
        PathTracingSettings settings;
        settings.samplesPerPixel = 1;
        settings.maxAccumulatedSamples = 4096;
        settings.maxBounces = 8;
        return settings;
    }
};

/**
 * @brief Denoiser settings
 */
struct DenoiserSettings {
    RTDenoiser type{RTDenoiser::SVGF};
    f32 strength{1.0f};                  ///< Denoising strength
    u32 historyLength{8};                ///< Temporal history frames
    f32 normalThreshold{0.95f};          ///< Normal similarity threshold
    f32 depthThreshold{0.1f};            ///< Depth similarity threshold
    bool useAlbedo{true};                ///< Use albedo guide
    bool useNormal{true};                ///< Use normal guide
    
    /// Factory for SVGF
    static DenoiserSettings svgf() noexcept {
        DenoiserSettings settings;
        settings.type = RTDenoiser::SVGF;
        return settings;
    }
    
    /// Factory for temporal only
    static DenoiserSettings temporal() noexcept {
        DenoiserSettings settings;
        settings.type = RTDenoiser::Temporal;
        settings.historyLength = 16;
        return settings;
    }
};

// =============================================================================
// RAY TRACING MANAGER
// =============================================================================

/**
 * @brief Ray tracing performance statistics
 */
struct RTStats {
    u64 rayCount{0};                     ///< Total rays traced this frame
    u64 blasBuilds{0};                   ///< BLAS builds this frame
    u64 tlasBuilds{0};                   ///< TLAS builds this frame
    u64 blasUpdates{0};                  ///< BLAS updates this frame
    u64 tlasUpdates{0};                  ///< TLAS updates this frame
    f32 buildTimeMs{0.0f};               ///< AS build time (ms)
    f32 traceTimeMs{0.0f};               ///< Ray trace time (ms)
    f32 denoiseTimeMs{0.0f};             ///< Denoising time (ms)
    u64 asMemoryUsage{0};                ///< Acceleration structure memory (bytes)
    
    void reset() noexcept {
        rayCount = 0;
        blasBuilds = 0;
        tlasBuilds = 0;
        blasUpdates = 0;
        tlasUpdates = 0;
        buildTimeMs = 0.0f;
        traceTimeMs = 0.0f;
        denoiseTimeMs = 0.0f;
    }
};

/**
 * @brief Hardware ray tracing capabilities
 */
struct RTCapabilities {
    bool supported{false};               ///< RT hardware available
    bool rayQuery{false};                ///< Ray query (inline RT) supported
    bool rayTracingPipeline{false};      ///< RT pipeline supported
    u32 maxRecursionDepth{0};            ///< Maximum recursion depth
    u32 maxGeometryCount{0};             ///< Max geometry per BLAS
    u32 maxInstanceCount{0};             ///< Max instances per TLAS
    u32 shaderGroupHandleSize{0};        ///< Size of shader group handles
    u32 shaderGroupBaseAlignment{0};     ///< Required SBT alignment
    f32 maxRayHitAttributeSize{0};       ///< Max hit attribute size
    std::string vendorName;
    std::string driverVersion;
    
    [[nodiscard]] bool canUseRayTracing() const noexcept {
        return supported && (rayQuery || rayTracingPipeline);
    }
};

/**
 * @brief Ray tracing manager
 * 
 * Manages acceleration structures, ray tracing pipelines, and hybrid rendering
 */
class RayTracingManager {
public:
    /// Singleton access
    static RayTracingManager& instance() noexcept {
        static RayTracingManager mgr;
        return mgr;
    }
    
    // -------------------------------------------------------------------------
    // INITIALIZATION
    // -------------------------------------------------------------------------
    
    /// Initialize ray tracing system
    bool initialize() noexcept {
        if (m_initialized) return true;
        
        // Query hardware capabilities
        queryCapabilities();
        
        m_initialized = true;
        return true;
    }
    
    /// Shutdown ray tracing system
    void shutdown() noexcept {
        m_blasCache.clear();
        m_activeTLAS = {};
        m_initialized = false;
    }
    
    /// Check if initialized
    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }
    
    // -------------------------------------------------------------------------
    // CAPABILITIES
    // -------------------------------------------------------------------------
    
    /// Get hardware capabilities
    [[nodiscard]] const RTCapabilities& getCapabilities() const noexcept {
        return m_capabilities;
    }
    
    /// Check if ray tracing is available
    [[nodiscard]] bool isAvailable() const noexcept {
        return m_capabilities.canUseRayTracing();
    }
    
    /// Check if specific technique is supported
    [[nodiscard]] bool isTechniqueSupported(RTTechnique technique) const noexcept {
        if (!isAvailable()) return false;
        
        switch (technique) {
            case RTTechnique::None: return true;
            case RTTechnique::Shadows: return true;
            case RTTechnique::AO: return true;
            case RTTechnique::Reflections: return true;
            case RTTechnique::GI: return m_capabilities.maxRecursionDepth >= 2;
            case RTTechnique::PathTracing: return m_capabilities.maxRecursionDepth >= 4;
            case RTTechnique::Hybrid: return true;
            default: return false;
        }
    }
    
    // -------------------------------------------------------------------------
    // ACCELERATION STRUCTURES
    // -------------------------------------------------------------------------
    
    /// Create bottom-level acceleration structure
    [[nodiscard]] AccelerationStructure createBLAS(const BLASDesc& desc) noexcept {
        AccelerationStructure blas;
        blas.type = AccelerationStructureType::BottomLevel;
        
        if (!desc.isValid()) return blas;
        
        // Calculate size requirements (simulated)
        u64 geometrySize = 0;
        for (const auto& geom : desc.triangleGeometries) {
            geometrySize += geom.triangleCount * 64; // ~64 bytes per triangle
        }
        for (const auto& geom : desc.aabbGeometries) {
            geometrySize += geom.aabbCount * 32; // ~32 bytes per AABB
        }
        
        blas.bufferSize = geometrySize;
        blas.scratchSize = geometrySize / 2;
        blas.handle = generateHandle();
        blas.deviceAddress = blas.handle;
        
        // Cache BLAS
        m_blasCache[desc.name] = blas;
        m_stats.blasBuilds++;
        
        return blas;
    }
    
    /// Create top-level acceleration structure
    [[nodiscard]] AccelerationStructure createTLAS(const TLASDesc& desc) noexcept {
        AccelerationStructure tlas;
        tlas.type = AccelerationStructureType::TopLevel;
        
        if (!desc.isValid()) return tlas;
        
        // Calculate size requirements
        tlas.bufferSize = desc.instances.size() * 128; // ~128 bytes per instance
        tlas.scratchSize = tlas.bufferSize / 4;
        tlas.handle = generateHandle();
        tlas.deviceAddress = tlas.handle;
        
        m_activeTLAS = tlas;
        m_stats.tlasBuilds++;
        
        return tlas;
    }
    
    /// Update BLAS (for animated geometry)
    bool updateBLAS(const std::string& name) noexcept {
        auto it = m_blasCache.find(name);
        if (it == m_blasCache.end()) return false;
        
        m_stats.blasUpdates++;
        return true;
    }
    
    /// Update TLAS (for moving instances)
    bool updateTLAS() noexcept {
        if (!m_activeTLAS.isValid()) return false;
        
        m_stats.tlasUpdates++;
        return true;
    }
    
    /// Get cached BLAS by name
    [[nodiscard]] const AccelerationStructure* getBLAS(const std::string& name) const noexcept {
        auto it = m_blasCache.find(name);
        return it != m_blasCache.end() ? &it->second : nullptr;
    }
    
    /// Get active TLAS
    [[nodiscard]] const AccelerationStructure& getTLAS() const noexcept {
        return m_activeTLAS;
    }
    
    // -------------------------------------------------------------------------
    // SHADER BINDING TABLE
    // -------------------------------------------------------------------------
    
    /// Build shader binding table
    bool buildSBT(ShaderBindingTable& sbt) noexcept {
        if (!isAvailable()) return false;
        
        u32 handleSize = m_capabilities.shaderGroupHandleSize;
        u32 alignment = m_capabilities.shaderGroupBaseAlignment;
        
        // Calculate buffer size and offsets
        u64 totalSize = sbt.calculateBufferSize(handleSize, alignment);
        
        // Assign buffer (simulated)
        sbt.bufferHandle = generateHandle();
        sbt.bufferAddress = sbt.bufferHandle;
        
        // Calculate region offsets
        u64 offset = 0;
        
        // Ray gen
        sbt.rayGenOffset = offset;
        sbt.rayGenStride = handleSize;
        if (!sbt.rayGenEntries.empty()) {
            offset += sbt.rayGenEntries[0].getAlignedSize(handleSize, alignment);
        }
        offset = (offset + RTConfig::SBT_ALIGNMENT - 1) & ~(RTConfig::SBT_ALIGNMENT - 1);
        
        // Miss
        sbt.missOffset = offset;
        sbt.missStride = handleSize;
        for (const auto& entry : sbt.missEntries) {
            sbt.missStride = std::max(sbt.missStride, static_cast<u64>(entry.getAlignedSize(handleSize, alignment)));
            offset += entry.getAlignedSize(handleSize, alignment);
        }
        offset = (offset + RTConfig::SBT_ALIGNMENT - 1) & ~(RTConfig::SBT_ALIGNMENT - 1);
        
        // Hit groups
        sbt.hitGroupOffset = offset;
        sbt.hitGroupStride = handleSize;
        for (const auto& entry : sbt.hitGroupEntries) {
            sbt.hitGroupStride = std::max(sbt.hitGroupStride, static_cast<u64>(entry.getAlignedSize(handleSize, alignment)));
        }
        
        return true;
    }
    
    // -------------------------------------------------------------------------
    // SETTINGS
    // -------------------------------------------------------------------------
    
    /// Set active technique
    void setTechnique(RTTechnique technique) noexcept {
        m_activeTechnique = technique;
    }
    
    /// Get active technique
    [[nodiscard]] RTTechnique getTechnique() const noexcept {
        return m_activeTechnique;
    }
    
    /// Set shadow settings
    void setShadowSettings(const RTShadowSettings& settings) noexcept {
        m_shadowSettings = settings;
    }
    
    [[nodiscard]] const RTShadowSettings& getShadowSettings() const noexcept {
        return m_shadowSettings;
    }
    
    /// Set AO settings
    void setAOSettings(const RTAOSettings& settings) noexcept {
        m_aoSettings = settings;
    }
    
    [[nodiscard]] const RTAOSettings& getAOSettings() const noexcept {
        return m_aoSettings;
    }
    
    /// Set reflection settings
    void setReflectionSettings(const RTReflectionSettings& settings) noexcept {
        m_reflectionSettings = settings;
    }
    
    [[nodiscard]] const RTReflectionSettings& getReflectionSettings() const noexcept {
        return m_reflectionSettings;
    }
    
    /// Set GI settings
    void setGISettings(const RTGISettings& settings) noexcept {
        m_giSettings = settings;
    }
    
    [[nodiscard]] const RTGISettings& getGISettings() const noexcept {
        return m_giSettings;
    }
    
    /// Set path tracing settings
    void setPathTracingSettings(const PathTracingSettings& settings) noexcept {
        m_pathTracingSettings = settings;
    }
    
    [[nodiscard]] const PathTracingSettings& getPathTracingSettings() const noexcept {
        return m_pathTracingSettings;
    }
    
    /// Set denoiser settings
    void setDenoiserSettings(const DenoiserSettings& settings) noexcept {
        m_denoiserSettings = settings;
    }
    
    [[nodiscard]] const DenoiserSettings& getDenoiserSettings() const noexcept {
        return m_denoiserSettings;
    }
    
    // -------------------------------------------------------------------------
    // STATISTICS
    // -------------------------------------------------------------------------
    
    /// Get statistics
    [[nodiscard]] const RTStats& getStats() const noexcept {
        return m_stats;
    }
    
    /// Reset statistics
    void resetStats() noexcept {
        m_stats.reset();
    }
    
    /// Begin frame
    void beginFrame() noexcept {
        m_stats.reset();
    }
    
    /// End frame
    void endFrame() noexcept {
        // Calculate memory usage
        m_stats.asMemoryUsage = 0;
        for (const auto& [name, blas] : m_blasCache) {
            m_stats.asMemoryUsage += blas.bufferSize;
        }
        m_stats.asMemoryUsage += m_activeTLAS.bufferSize;
    }

private:
    RayTracingManager() = default;
    
    /// Query hardware capabilities
    void queryCapabilities() noexcept {
        // Simulated capabilities for development
        m_capabilities.supported = true;
        m_capabilities.rayQuery = true;
        m_capabilities.rayTracingPipeline = true;
        m_capabilities.maxRecursionDepth = RTConfig::MAX_RECURSION_DEPTH;
        m_capabilities.maxGeometryCount = RTConfig::MAX_GEOMETRY_PER_BLAS;
        m_capabilities.maxInstanceCount = RTConfig::MAX_INSTANCES_PER_TLAS;
        m_capabilities.shaderGroupHandleSize = 32;
        m_capabilities.shaderGroupBaseAlignment = 64;
        m_capabilities.vendorName = "NovaCore Simulated";
        m_capabilities.driverVersion = "1.0.0";
    }
    
    /// Generate unique handle
    [[nodiscard]] u64 generateHandle() noexcept {
        return ++m_handleCounter;
    }
    
    bool m_initialized{false};
    RTCapabilities m_capabilities{};
    RTStats m_stats{};
    
    RTTechnique m_activeTechnique{RTTechnique::Hybrid};
    RTShadowSettings m_shadowSettings{};
    RTAOSettings m_aoSettings{};
    RTReflectionSettings m_reflectionSettings{};
    RTGISettings m_giSettings{};
    PathTracingSettings m_pathTracingSettings{};
    DenoiserSettings m_denoiserSettings{};
    
    std::unordered_map<std::string, AccelerationStructure> m_blasCache;
    AccelerationStructure m_activeTLAS{};
    
    u64 m_handleCounter{0};
};

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * @brief Generate random direction on hemisphere (cosine weighted)
 */
[[nodiscard]] inline Vec3 cosineWeightedHemisphere(const Vec2& u, const Vec3& normal) noexcept {
    // Create orthonormal basis
    Vec3 tangent, bitangent;
    if (std::abs(normal.x) > 0.9f) {
        tangent = Vec3{0.0f, 1.0f, 0.0f}.cross(normal).normalized();
    } else {
        tangent = Vec3{1.0f, 0.0f, 0.0f}.cross(normal).normalized();
    }
    bitangent = normal.cross(tangent);
    
    // Cosine-weighted sampling
    f32 phi = 2.0f * 3.14159265f * u.x;
    f32 cosTheta = std::sqrt(1.0f - u.y);
    f32 sinTheta = std::sqrt(u.y);
    
    return (tangent * std::cos(phi) * sinTheta +
            bitangent * std::sin(phi) * sinTheta +
            normal * cosTheta).normalized();
}

/**
 * @brief Generate random direction on sphere (uniform)
 */
[[nodiscard]] inline Vec3 uniformSphere(const Vec2& u) noexcept {
    f32 z = 1.0f - 2.0f * u.x;
    f32 r = std::sqrt(std::max(0.0f, 1.0f - z * z));
    f32 phi = 2.0f * 3.14159265f * u.y;
    return Vec3{r * std::cos(phi), r * std::sin(phi), z};
}

/**
 * @brief Calculate Fresnel reflectance (Schlick approximation)
 */
[[nodiscard]] inline f32 fresnelSchlick(f32 cosTheta, f32 f0) noexcept {
    return f0 + (1.0f - f0) * std::pow(1.0f - cosTheta, 5.0f);
}

/**
 * @brief Calculate Fresnel reflectance for RGB (Schlick approximation)
 */
[[nodiscard]] inline Vec3 fresnelSchlickRGB(f32 cosTheta, const Vec3& f0) noexcept {
    f32 t = std::pow(1.0f - cosTheta, 5.0f);
    return f0 + (Vec3{1.0f, 1.0f, 1.0f} - f0) * t;
}

/**
 * @brief GGX/Trowbridge-Reitz normal distribution function
 */
[[nodiscard]] inline f32 distributionGGX(const Vec3& n, const Vec3& h, f32 roughness) noexcept {
    f32 a = roughness * roughness;
    f32 a2 = a * a;
    f32 NdotH = std::max(n.dot(h), 0.0f);
    f32 NdotH2 = NdotH * NdotH;
    
    f32 nom = a2;
    f32 denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = 3.14159265f * denom * denom;
    
    return nom / std::max(denom, 0.0001f);
}

/**
 * @brief Smith's geometry function (combined G1 terms)
 */
[[nodiscard]] inline f32 geometrySmith(const Vec3& n, const Vec3& v, const Vec3& l, f32 roughness) noexcept {
    f32 NdotV = std::max(n.dot(v), 0.0f);
    f32 NdotL = std::max(n.dot(l), 0.0f);
    f32 r = roughness + 1.0f;
    f32 k = (r * r) / 8.0f;
    
    auto G1 = [k](f32 NdotX) {
        return NdotX / (NdotX * (1.0f - k) + k);
    };
    
    return G1(NdotV) * G1(NdotL);
}

} // namespace nova
