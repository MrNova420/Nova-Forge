/**
 * @file mesh_lod.hpp
 * @brief Advanced Mesh LOD (Level of Detail) System
 * 
 * Provides intelligent mesh level-of-detail management with:
 * - Automatic LOD generation from high-poly source meshes
 * - Screen-space error metrics for optimal LOD selection
 * - Seamless LOD transitions with morphing/blending
 * - GPU-driven LOD selection for massive scene scalability
 * - Nanite-inspired cluster-based rendering
 * - Mesh streaming for virtually unlimited geometry
 * 
 * Following the 12-year development roadmap from:
 * - NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md (Phase 1: Core Rendering)
 * - AUTONOMOUS_DEVELOPMENT_GUIDE.md
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "nova/core/types/types.hpp"
#include "nova/core/math/math.hpp"
#include "render_types.hpp"
#include <vector>
#include <array>
#include <memory>
#include <functional>
#include <string>

namespace nova::render {

// ============================================================================
// LOD Configuration
// ============================================================================

/**
 * @brief Configuration constants for the LOD system
 */
struct LODConfig {
    static constexpr usize MAX_LOD_LEVELS = 8;           ///< Maximum LOD levels per mesh
    static constexpr usize MAX_CLUSTERS_PER_MESH = 1024; ///< Maximum clusters (Nanite-style)
    static constexpr usize CLUSTER_TRIANGLE_COUNT = 128; ///< Triangles per cluster
    static constexpr usize CLUSTER_VERTEX_COUNT = 64;    ///< Max vertices per cluster
    static constexpr f32 DEFAULT_LOD_BIAS = 1.0f;        ///< Default LOD selection bias
    static constexpr f32 DEFAULT_TRANSITION_RANGE = 0.1f; ///< Default blend transition range
    static constexpr f32 MIN_SCREEN_SIZE = 0.001f;       ///< Minimum screen size before culling
    static constexpr u32 IMPOSTOR_ATLAS_SIZE = 2048;     ///< Impostor atlas resolution
    static constexpr u32 IMPOSTOR_FRAME_COUNT = 24;      ///< Impostor viewpoint count
};

// ============================================================================
// LOD Enumerations
// ============================================================================

/**
 * @brief LOD selection strategy
 */
enum class LODSelectionMode : u8 {
    ScreenSize,     ///< Based on screen-space projected size
    Distance,       ///< Based on camera distance
    ScreenError,    ///< Based on screen-space geometric error
    GPUDriven,      ///< GPU compute shader selects optimal LOD
    Manual          ///< Manually specified LOD level
};

/**
 * @brief LOD transition type
 */
enum class LODTransitionMode : u8 {
    Instant,        ///< Hard cut between LOD levels
    Dither,         ///< Screen-door dithering transition
    CrossFade,      ///< Alpha-blend cross-fade
    Morph           ///< Vertex morphing (requires morph targets)
};

/**
 * @brief LOD generation algorithm
 */
enum class LODGenerationAlgorithm : u8 {
    QuadricErrorMetric,     ///< Garland-Heckbert QEM (fast, good quality)
    EdgeCollapse,           ///< Progressive edge collapse
    VertexClustering,       ///< Voxel-based vertex clustering (fastest)
    NeuralSimplification,   ///< ML-based mesh simplification (best quality)
    AdaptiveSubdivision     ///< Reverse subdivision unwinding
};

/**
 * @brief Cluster group type for Nanite-style rendering
 */
enum class ClusterGroupType : u8 {
    Standard,       ///< Regular triangle clusters
    Boundary,       ///< Clusters at LOD boundaries
    Transition,     ///< Clusters with LOD transition data
    Impostor        ///< Billboard impostor clusters
};

// ============================================================================
// LOD Level Description
// ============================================================================

/**
 * @brief Description of a single LOD level
 */
struct LODLevelDesc {
    u32 triangleCount = 0;          ///< Number of triangles at this LOD
    u32 vertexCount = 0;            ///< Number of vertices at this LOD
    f32 screenSizeThreshold = 0.0f; ///< Minimum screen size to use this LOD
    f32 geometricError = 0.0f;      ///< Max geometric error (world units)
    f32 simplificationRatio = 1.0f; ///< Ratio compared to LOD 0 (1.0 = full)
    u32 indexBufferOffset = 0;      ///< Offset into shared index buffer
    u32 vertexBufferOffset = 0;     ///< Offset into shared vertex buffer
    bool hasMorphTargets = false;   ///< Whether this LOD has morph data
    
    /**
     * @brief Calculate memory usage for this LOD level
     */
    [[nodiscard]] constexpr usize calculateMemoryUsage(usize vertexStride) const {
        return (static_cast<usize>(vertexCount) * vertexStride) + 
               (static_cast<usize>(triangleCount) * 3 * sizeof(u32));
    }
    
    /**
     * @brief Check if this LOD should be used for given screen size
     */
    [[nodiscard]] constexpr bool shouldUse(f32 screenSize, f32 lodBias = 1.0f) const {
        return screenSize * lodBias >= screenSizeThreshold;
    }
};

/**
 * @brief Settings for LOD generation
 */
struct LODGenerationSettings {
    LODGenerationAlgorithm algorithm = LODGenerationAlgorithm::QuadricErrorMetric;
    u32 targetLODCount = 4;                     ///< Number of LOD levels to generate
    f32 reductionFactor = 0.5f;                 ///< Triangle reduction per LOD level
    f32 maxGeometricError = 0.001f;             ///< Maximum allowed error (world units)
    bool preserveBoundary = true;               ///< Preserve mesh boundaries
    bool preserveUVSeams = true;                ///< Preserve UV discontinuities
    bool preserveNormals = true;                ///< Maintain smooth normals
    bool generateMorphTargets = false;          ///< Generate vertex morph data
    f32 normalWeightCoefficient = 100.0f;       ///< Weight for normal preservation
    f32 uvWeightCoefficient = 100.0f;           ///< Weight for UV preservation
    std::array<f32, LODConfig::MAX_LOD_LEVELS> screenSizeThresholds = {
        1.0f, 0.5f, 0.25f, 0.125f, 0.0625f, 0.03125f, 0.015625f, 0.0078125f
    };
};

// ============================================================================
// Mesh Cluster (Nanite-style)
// ============================================================================

/**
 * @brief A cluster of triangles for GPU-driven rendering
 * 
 * Nanite-inspired hierarchical cluster system for massive geometry.
 * Each cluster contains a small number of triangles that can be:
 * - Culled independently on GPU
 * - LOD-selected per-cluster
 * - Streamed dynamically
 */
struct MeshCluster {
    u32 triangleOffset = 0;         ///< First triangle index
    u32 triangleCount = 0;          ///< Number of triangles in cluster
    u32 vertexOffset = 0;           ///< First vertex index
    u32 vertexCount = 0;            ///< Number of vertices in cluster
    math::Vec3 boundsCenter{0.0f, 0.0f, 0.0f}; ///< Bounding sphere center
    f32 boundsRadius = 0.0f;        ///< Bounding sphere radius
    f32 lodError = 0.0f;            ///< Geometric error for LOD selection
    u8 lodLevel = 0;                ///< LOD level this cluster belongs to
    ClusterGroupType groupType = ClusterGroupType::Standard;
    u32 parentCluster = UINT32_MAX; ///< Parent cluster for hierarchy
    u32 childClusterStart = UINT32_MAX; ///< First child cluster
    u32 childClusterCount = 0;      ///< Number of child clusters
    
    /**
     * @brief Calculate screen-space error for this cluster
     */
    [[nodiscard]] f32 calculateScreenError(
        const math::Vec3& cameraPos,
        f32 screenHeight,
        f32 fovY
    ) const {
        f32 distance = (cameraPos - boundsCenter).length();
        if (distance < 0.001f) distance = 0.001f;
        
        // Convert world-space error to screen-space pixels
        f32 projectedError = (lodError / distance) * screenHeight / (2.0f * std::tan(fovY * 0.5f));
        return projectedError;
    }
    
    /**
     * @brief Check if cluster should be rendered at given error threshold
     */
    [[nodiscard]] bool shouldRender(f32 maxScreenError, f32 screenError) const {
        return screenError <= maxScreenError;
    }
};

/**
 * @brief GPU-friendly cluster data for compute shader
 */
struct alignas(16) GPUClusterData {
    math::Vec4 boundingSphere;      ///< xyz = center, w = radius
    u32 triangleOffset;             ///< First triangle index
    u32 triangleCount;              ///< Number of triangles
    u32 vertexOffset;               ///< First vertex index
    f32 lodError;                   ///< Geometric error
    u32 lodLevel;                   ///< LOD level
    u32 flags;                      ///< Cluster flags
    u32 parentCluster;              ///< Parent cluster index
    u32 padding;                    ///< Alignment padding
};
static_assert(sizeof(GPUClusterData) == 48, "GPUClusterData must be 48 bytes");

// ============================================================================
// LOD Mesh
// ============================================================================

/**
 * @brief Complete LOD mesh with all detail levels
 */
class LODMesh {
public:
    /**
     * @brief Create an empty LOD mesh
     */
    LODMesh() = default;
    
    /**
     * @brief Create LOD mesh with name
     */
    explicit LODMesh(std::string name) : m_name(std::move(name)) {}
    
    /**
     * @brief Get mesh name
     */
    [[nodiscard]] const std::string& getName() const { return m_name; }
    
    /**
     * @brief Set mesh name
     */
    void setName(std::string name) { m_name = std::move(name); }
    
    /**
     * @brief Get number of LOD levels
     */
    [[nodiscard]] usize getLODCount() const { return m_lodLevels.size(); }
    
    /**
     * @brief Get LOD level description
     */
    [[nodiscard]] const LODLevelDesc& getLODLevel(usize index) const {
        return m_lodLevels[index];
    }
    
    /**
     * @brief Add a LOD level
     */
    void addLODLevel(const LODLevelDesc& level) {
        if (m_lodLevels.size() < LODConfig::MAX_LOD_LEVELS) {
            m_lodLevels.push_back(level);
        }
    }
    
    /**
     * @brief Get bounding sphere
     */
    [[nodiscard]] const math::Vec4& getBoundingSphere() const { return m_boundingSphere; }
    
    /**
     * @brief Set bounding sphere (xyz = center, w = radius)
     */
    void setBoundingSphere(const math::Vec4& sphere) { m_boundingSphere = sphere; }
    
    /**
     * @brief Get bounding box minimum
     */
    [[nodiscard]] const math::Vec3& getBoundsMin() const { return m_boundsMin; }
    
    /**
     * @brief Get bounding box maximum
     */
    [[nodiscard]] const math::Vec3& getBoundsMax() const { return m_boundsMax; }
    
    /**
     * @brief Set bounding box
     */
    void setBounds(const math::Vec3& min, const math::Vec3& max) {
        m_boundsMin = min;
        m_boundsMax = max;
    }
    
    /**
     * @brief Select optimal LOD for given parameters
     */
    [[nodiscard]] u32 selectLOD(
        f32 screenSize,
        f32 lodBias = 1.0f,
        LODSelectionMode mode = LODSelectionMode::ScreenSize
    ) const {
        if (m_lodLevels.empty()) return 0;
        
        f32 adjustedSize = screenSize * lodBias;
        
        // Find highest quality LOD that meets screen size threshold
        for (usize i = 0; i < m_lodLevels.size(); ++i) {
            if (m_lodLevels[i].shouldUse(adjustedSize)) {
                return static_cast<u32>(i);
            }
        }
        
        // Return lowest LOD if nothing matches
        return static_cast<u32>(m_lodLevels.size() - 1);
    }
    
    /**
     * @brief Calculate screen size for distance and FOV
     */
    [[nodiscard]] static f32 calculateScreenSize(
        f32 objectRadius,
        f32 distance,
        f32 screenHeight,
        f32 fovY
    ) {
        if (distance < 0.001f) distance = 0.001f;
        f32 projectedSize = (objectRadius / distance) * screenHeight / (2.0f * std::tan(fovY * 0.5f));
        return projectedSize / screenHeight; // Normalized 0-1
    }
    
    /**
     * @brief Get clusters for this mesh
     */
    [[nodiscard]] const std::vector<MeshCluster>& getClusters() const { return m_clusters; }
    
    /**
     * @brief Add a cluster
     */
    void addCluster(const MeshCluster& cluster) {
        if (m_clusters.size() < LODConfig::MAX_CLUSTERS_PER_MESH) {
            m_clusters.push_back(cluster);
        }
    }
    
    /**
     * @brief Clear all clusters
     */
    void clearClusters() { m_clusters.clear(); }
    
    /**
     * @brief Get total triangle count across all LODs
     */
    [[nodiscard]] u32 getTotalTriangleCount() const {
        u32 total = 0;
        for (const auto& lod : m_lodLevels) {
            total += lod.triangleCount;
        }
        return total;
    }
    
    /**
     * @brief Get total memory usage
     */
    [[nodiscard]] usize getTotalMemoryUsage(usize vertexStride) const {
        usize total = 0;
        for (const auto& lod : m_lodLevels) {
            total += lod.calculateMemoryUsage(vertexStride);
        }
        return total;
    }
    
    /**
     * @brief Check if mesh has cluster data
     */
    [[nodiscard]] bool hasClusterData() const { return !m_clusters.empty(); }
    
    /**
     * @brief Get GPU buffer handles
     */
    [[nodiscard]] BufferHandle getVertexBuffer() const { return m_vertexBuffer; }
    [[nodiscard]] BufferHandle getIndexBuffer() const { return m_indexBuffer; }
    [[nodiscard]] BufferHandle getClusterBuffer() const { return m_clusterBuffer; }
    
    /**
     * @brief Set GPU buffer handles
     */
    void setVertexBuffer(BufferHandle buffer) { m_vertexBuffer = buffer; }
    void setIndexBuffer(BufferHandle buffer) { m_indexBuffer = buffer; }
    void setClusterBuffer(BufferHandle buffer) { m_clusterBuffer = buffer; }
    
private:
    std::string m_name;
    std::vector<LODLevelDesc> m_lodLevels;
    std::vector<MeshCluster> m_clusters;
    math::Vec4 m_boundingSphere{0.0f, 0.0f, 0.0f, 1.0f};
    math::Vec3 m_boundsMin{-1.0f, -1.0f, -1.0f};
    math::Vec3 m_boundsMax{1.0f, 1.0f, 1.0f};
    BufferHandle m_vertexBuffer;
    BufferHandle m_indexBuffer;
    BufferHandle m_clusterBuffer;
};

// ============================================================================
// LOD Selection State
// ============================================================================

/**
 * @brief Per-instance LOD selection state
 */
struct LODSelectionState {
    u32 currentLOD = 0;             ///< Currently selected LOD
    u32 targetLOD = 0;              ///< Target LOD for transition
    f32 transitionProgress = 1.0f;  ///< Transition blend factor (0-1)
    f32 lastScreenSize = 1.0f;      ///< Last computed screen size
    f32 lodHysteresis = 0.0f;       ///< Hysteresis to prevent LOD flicker
    bool isTransitioning = false;   ///< Whether currently transitioning
    
    /**
     * @brief Update LOD selection with hysteresis
     */
    void update(u32 newLOD, f32 screenSize, f32 hysteresisThreshold = 0.1f) {
        lastScreenSize = screenSize;
        
        // Apply hysteresis to prevent rapid LOD switching
        if (newLOD != targetLOD) {
            f32 sizeDiff = std::abs(screenSize - lodHysteresis);
            if (sizeDiff > hysteresisThreshold || !isTransitioning) {
                targetLOD = newLOD;
                lodHysteresis = screenSize;
                if (targetLOD != currentLOD) {
                    isTransitioning = true;
                    transitionProgress = 0.0f;
                }
            }
        }
    }
    
    /**
     * @brief Step the transition animation
     */
    void stepTransition(f32 deltaTime, f32 transitionSpeed = 4.0f) {
        if (isTransitioning) {
            transitionProgress += deltaTime * transitionSpeed;
            if (transitionProgress >= 1.0f) {
                transitionProgress = 1.0f;
                currentLOD = targetLOD;
                isTransitioning = false;
            }
        }
    }
    
    /**
     * @brief Get blend factor for cross-fade
     */
    [[nodiscard]] f32 getBlendFactor() const {
        return isTransitioning ? transitionProgress : 1.0f;
    }
};

// ============================================================================
// Impostor System
// ============================================================================

/**
 * @brief Impostor data for distant objects
 */
struct ImpostorData {
    TextureHandle atlasTexture;     ///< Impostor texture atlas
    u32 frameCount = LODConfig::IMPOSTOR_FRAME_COUNT;
    u32 atlasWidth = LODConfig::IMPOSTOR_ATLAS_SIZE;
    u32 atlasHeight = LODConfig::IMPOSTOR_ATLAS_SIZE;
    f32 switchDistance = 100.0f;    ///< Distance to switch to impostor
    bool useOctahedral = true;      ///< Use octahedral mapping
    
    /**
     * @brief Calculate UV for view direction
     */
    [[nodiscard]] math::Vec2 calculateUV(const math::Vec3& viewDir) const {
        if (useOctahedral) {
            // Octahedral mapping for full sphere coverage
            f32 l1 = std::abs(viewDir.x) + std::abs(viewDir.y) + std::abs(viewDir.z);
            math::Vec2 oct{viewDir.x / l1, viewDir.y / l1};
            
            if (viewDir.z < 0.0f) {
                f32 signX = oct.x >= 0.0f ? 1.0f : -1.0f;
                f32 signY = oct.y >= 0.0f ? 1.0f : -1.0f;
                oct.x = (1.0f - std::abs(oct.y)) * signX;
                oct.y = (1.0f - std::abs(oct.x)) * signY;
            }
            
            return {oct.x * 0.5f + 0.5f, oct.y * 0.5f + 0.5f};
        } else {
            // Simple spherical mapping
            f32 u = std::atan2(viewDir.x, viewDir.z) / (2.0f * 3.14159265f) + 0.5f;
            f32 v = std::asin(viewDir.y) / 3.14159265f + 0.5f;
            return {u, v};
        }
    }
};

// ============================================================================
// LOD Manager
// ============================================================================

/**
 * @brief Manages LOD meshes and selection for the entire scene
 */
class LODManager {
public:
    /**
     * @brief LOD statistics for profiling
     */
    struct Statistics {
        u32 totalMeshes = 0;
        u32 totalLODLevels = 0;
        u32 totalClusters = 0;
        u32 visibleClusters = 0;
        u32 culledClusters = 0;
        u64 trianglesRendered = 0;
        u64 trianglesPotential = 0;  // If all LOD 0
        f32 averageLODLevel = 0.0f;
        f32 lodReductionRatio = 0.0f;
        
        void reset() {
            totalMeshes = 0;
            totalLODLevels = 0;
            totalClusters = 0;
            visibleClusters = 0;
            culledClusters = 0;
            trianglesRendered = 0;
            trianglesPotential = 0;
            averageLODLevel = 0.0f;
            lodReductionRatio = 0.0f;
        }
        
        void calculateDerived() {
            if (trianglesPotential > 0) {
                lodReductionRatio = 1.0f - static_cast<f32>(trianglesRendered) / 
                                          static_cast<f32>(trianglesPotential);
            }
        }
    };
    
    /**
     * @brief Get singleton instance
     */
    static LODManager& getInstance() {
        static LODManager instance;
        return instance;
    }
    
    /**
     * @brief Set global LOD bias
     */
    void setLODBias(f32 bias) { m_lodBias = bias; }
    
    /**
     * @brief Get global LOD bias
     */
    [[nodiscard]] f32 getLODBias() const { return m_lodBias; }
    
    /**
     * @brief Set LOD selection mode
     */
    void setSelectionMode(LODSelectionMode mode) { m_selectionMode = mode; }
    
    /**
     * @brief Get LOD selection mode
     */
    [[nodiscard]] LODSelectionMode getSelectionMode() const { return m_selectionMode; }
    
    /**
     * @brief Set transition mode
     */
    void setTransitionMode(LODTransitionMode mode) { m_transitionMode = mode; }
    
    /**
     * @brief Get transition mode
     */
    [[nodiscard]] LODTransitionMode getTransitionMode() const { return m_transitionMode; }
    
    /**
     * @brief Set maximum screen error for cluster selection
     */
    void setMaxScreenError(f32 error) { m_maxScreenError = error; }
    
    /**
     * @brief Get maximum screen error
     */
    [[nodiscard]] f32 getMaxScreenError() const { return m_maxScreenError; }
    
    /**
     * @brief Get statistics
     */
    [[nodiscard]] const Statistics& getStatistics() const { return m_statistics; }
    
    /**
     * @brief Reset statistics for new frame
     */
    void resetStatistics() { m_statistics.reset(); }
    
    /**
     * @brief Update statistics
     */
    void updateStatistics() { m_statistics.calculateDerived(); }
    
    /**
     * @brief Increment triangle counters
     */
    void recordTriangles(u64 rendered, u64 potential) {
        m_statistics.trianglesRendered += rendered;
        m_statistics.trianglesPotential += potential;
    }
    
    /**
     * @brief Record mesh LOD selection
     */
    void recordLODSelection(u32 lodLevel) {
        m_lodLevelSum += lodLevel;
        m_lodSelectionCount++;
        m_statistics.averageLODLevel = static_cast<f32>(m_lodLevelSum) / 
                                       static_cast<f32>(m_lodSelectionCount);
    }
    
private:
    LODManager() = default;
    
    f32 m_lodBias = LODConfig::DEFAULT_LOD_BIAS;
    f32 m_maxScreenError = 1.0f;
    LODSelectionMode m_selectionMode = LODSelectionMode::ScreenSize;
    LODTransitionMode m_transitionMode = LODTransitionMode::Dither;
    Statistics m_statistics;
    u64 m_lodLevelSum = 0;
    u64 m_lodSelectionCount = 0;
};

// ============================================================================
// LOD Generation Utilities
// ============================================================================

/**
 * @brief Calculate geometric error between simplified and original mesh
 */
[[nodiscard]] inline f32 calculateGeometricError(
    const math::Vec3* originalVertices,
    usize originalCount,
    const math::Vec3* simplifiedVertices,
    usize simplifiedCount
) {
    f32 maxError = 0.0f;
    
    // Simple Hausdorff distance approximation
    for (usize i = 0; i < originalCount; ++i) {
        f32 minDist = std::numeric_limits<f32>::max();
        for (usize j = 0; j < simplifiedCount; ++j) {
            f32 dist = (originalVertices[i] - simplifiedVertices[j]).length();
            minDist = std::min(minDist, dist);
        }
        maxError = std::max(maxError, minDist);
    }
    
    return maxError;
}

/**
 * @brief Calculate screen size threshold for LOD level
 */
[[nodiscard]] inline f32 calculateScreenSizeThreshold(
    u32 lodLevel,
    u32 totalLODs,
    f32 baseFactor = 0.5f
) {
    // Exponential falloff for screen size thresholds
    return std::pow(baseFactor, static_cast<f32>(lodLevel));
}

/**
 * @brief Calculate triangle budget for LOD level
 */
[[nodiscard]] inline u32 calculateTriangleBudget(
    u32 baseTris,
    u32 lodLevel,
    f32 reductionFactor = 0.5f
) {
    f32 multiplier = std::pow(reductionFactor, static_cast<f32>(lodLevel));
    return static_cast<u32>(static_cast<f32>(baseTris) * multiplier);
}

} // namespace nova::render
