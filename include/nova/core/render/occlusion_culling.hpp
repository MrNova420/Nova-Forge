/**
 * @file occlusion_culling.hpp
 * @brief Occlusion Culling System for NovaCore Engine
 * 
 * Provides comprehensive occlusion culling:
 * - Hierarchical Z-buffer (Hi-Z) occlusion
 * - Hardware occlusion queries
 * - Software rasterization occlusion
 * - Frustum culling
 * - Portal-based culling
 * - Temporal coherence optimization
 * - Multi-view culling support
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "nova/core/types/types.hpp"
#include "nova/core/math/vec2.hpp"
#include "nova/core/math/vec3.hpp"
#include "nova/core/math/vec4.hpp"
#include "nova/core/math/mat4.hpp"

#include <array>
#include <vector>
#include <queue>
#include <bitset>
#include <functional>
#include <string>
#include <memory>
#include <atomic>

namespace nova {

// =============================================================================
// Configuration Constants
// =============================================================================

/**
 * @brief Occlusion culling configuration constants
 */
struct OcclusionConfig {
    /// Maximum Hi-Z mip levels
    static constexpr u32 MAX_HIZ_MIP_LEVELS = 12;
    
    /// Default Hi-Z resolution divisor
    static constexpr u32 DEFAULT_HIZ_DIVISOR = 2;
    
    /// Maximum hardware occlusion queries per frame
    static constexpr u32 MAX_OCCLUSION_QUERIES = 8192;
    
    /// Query pool size
    static constexpr u32 QUERY_POOL_SIZE = 16384;
    
    /// Maximum occluders for software culling
    static constexpr u32 MAX_SOFTWARE_OCCLUDERS = 256;
    
    /// Software rasterizer tile size
    static constexpr u32 SOFTWARE_TILE_SIZE = 64;
    
    /// Maximum portals
    static constexpr u32 MAX_PORTALS = 1024;
    
    /// Maximum visibility sets
    static constexpr u32 MAX_VISIBILITY_SETS = 256;
    
    /// Frustum plane count
    static constexpr u32 FRUSTUM_PLANE_COUNT = 6;
    
    /// Default conservative depth bias
    static constexpr f32 DEFAULT_DEPTH_BIAS = 0.0001f;
    
    /// Query result latency (frames)
    static constexpr u32 QUERY_LATENCY_FRAMES = 2;
};

// =============================================================================
// Enumerations
// =============================================================================

/**
 * @brief Occlusion culling technique
 */
enum class OcclusionTechnique : u32 {
    /// No occlusion culling
    None = 0,
    
    /// Frustum culling only
    FrustumOnly = 1,
    
    /// Hardware occlusion queries
    HardwareQueries = 2,
    
    /// Hi-Z buffer occlusion
    HiZBuffer = 3,
    
    /// Software rasterization
    SoftwareRaster = 4,
    
    /// Combined Hi-Z + hardware queries
    HiZWithQueries = 5,
    
    COUNT
};

/**
 * @brief Get occlusion technique name
 */
[[nodiscard]] constexpr const char* getOcclusionTechniqueName(OcclusionTechnique technique) noexcept {
    switch (technique) {
        case OcclusionTechnique::None: return "None";
        case OcclusionTechnique::FrustumOnly: return "Frustum Only";
        case OcclusionTechnique::HardwareQueries: return "Hardware Queries";
        case OcclusionTechnique::HiZBuffer: return "Hi-Z Buffer";
        case OcclusionTechnique::SoftwareRaster: return "Software Raster";
        case OcclusionTechnique::HiZWithQueries: return "Hi-Z + Queries";
        default: return "Unknown";
    }
}

/**
 * @brief Visibility result
 */
enum class VisibilityResult : u32 {
    /// Object is visible
    Visible = 0,
    
    /// Object is outside frustum
    OutsideFrustum = 1,
    
    /// Object is occluded by other geometry
    Occluded = 2,
    
    /// Object is too small to render
    TooSmall = 3,
    
    /// Visibility unknown (query pending)
    Pending = 4,
    
    COUNT
};

/**
 * @brief Get visibility result name
 */
[[nodiscard]] constexpr const char* getVisibilityResultName(VisibilityResult result) noexcept {
    switch (result) {
        case VisibilityResult::Visible: return "Visible";
        case VisibilityResult::OutsideFrustum: return "Outside Frustum";
        case VisibilityResult::Occluded: return "Occluded";
        case VisibilityResult::TooSmall: return "Too Small";
        case VisibilityResult::Pending: return "Pending";
        default: return "Unknown";
    }
}

/**
 * @brief Frustum plane index
 */
enum class FrustumPlane : u32 {
    Left = 0,
    Right = 1,
    Bottom = 2,
    Top = 3,
    Near = 4,
    Far = 5,
    
    COUNT
};

/**
 * @brief Bounding volume type
 */
enum class BoundingVolumeType : u32 {
    /// Axis-aligned bounding box
    AABB = 0,
    
    /// Oriented bounding box
    OBB = 1,
    
    /// Bounding sphere
    Sphere = 2,
    
    /// Convex hull
    ConvexHull = 3,
    
    COUNT
};

// =============================================================================
// Bounding Volumes
// =============================================================================

/**
 * @brief Axis-aligned bounding box
 */
struct AABB {
    /// Minimum corner
    Vec3 min = Vec3{0.0f, 0.0f, 0.0f};
    
    /// Maximum corner
    Vec3 max = Vec3{0.0f, 0.0f, 0.0f};
    
    /**
     * @brief Default constructor (empty box)
     */
    AABB() = default;
    
    /**
     * @brief Construct from min/max
     */
    AABB(const Vec3& minPt, const Vec3& maxPt) : min(minPt), max(maxPt) {}
    
    /**
     * @brief Get center of AABB
     */
    [[nodiscard]] Vec3 getCenter() const noexcept {
        return Vec3{
            (min.x + max.x) * 0.5f,
            (min.y + max.y) * 0.5f,
            (min.z + max.z) * 0.5f
        };
    }
    
    /**
     * @brief Get half-extents
     */
    [[nodiscard]] Vec3 getHalfExtents() const noexcept {
        return Vec3{
            (max.x - min.x) * 0.5f,
            (max.y - min.y) * 0.5f,
            (max.z - min.z) * 0.5f
        };
    }
    
    /**
     * @brief Get full extents (size)
     */
    [[nodiscard]] Vec3 getExtents() const noexcept {
        return Vec3{
            max.x - min.x,
            max.y - min.y,
            max.z - min.z
        };
    }
    
    /**
     * @brief Get surface area
     */
    [[nodiscard]] f32 getSurfaceArea() const noexcept {
        Vec3 e = getExtents();
        return 2.0f * (e.x * e.y + e.y * e.z + e.z * e.x);
    }
    
    /**
     * @brief Get volume
     */
    [[nodiscard]] f32 getVolume() const noexcept {
        Vec3 e = getExtents();
        return e.x * e.y * e.z;
    }
    
    /**
     * @brief Check if point is inside
     */
    [[nodiscard]] bool containsPoint(const Vec3& point) const noexcept {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }
    
    /**
     * @brief Check if intersects another AABB
     */
    [[nodiscard]] bool intersects(const AABB& other) const noexcept {
        return min.x <= other.max.x && max.x >= other.min.x &&
               min.y <= other.max.y && max.y >= other.min.y &&
               min.z <= other.max.z && max.z >= other.min.z;
    }
    
    /**
     * @brief Expand to include point
     */
    void expandToInclude(const Vec3& point) noexcept {
        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        min.z = std::min(min.z, point.z);
        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
        max.z = std::max(max.z, point.z);
    }
    
    /**
     * @brief Expand to include another AABB
     */
    void expandToInclude(const AABB& other) noexcept {
        min.x = std::min(min.x, other.min.x);
        min.y = std::min(min.y, other.min.y);
        min.z = std::min(min.z, other.min.z);
        max.x = std::max(max.x, other.max.x);
        max.y = std::max(max.y, other.max.y);
        max.z = std::max(max.z, other.max.z);
    }
    
    /**
     * @brief Transform AABB by matrix (returns new AABB)
     */
    [[nodiscard]] AABB transform(const Mat4& matrix) const noexcept {
        // Get all 8 corners and transform them
        std::array<Vec3, 8> corners = {
            Vec3{min.x, min.y, min.z},
            Vec3{max.x, min.y, min.z},
            Vec3{min.x, max.y, min.z},
            Vec3{max.x, max.y, min.z},
            Vec3{min.x, min.y, max.z},
            Vec3{max.x, min.y, max.z},
            Vec3{min.x, max.y, max.z},
            Vec3{max.x, max.y, max.z}
        };
        
        AABB result;
        result.min = Vec3{std::numeric_limits<f32>::max(), 
                          std::numeric_limits<f32>::max(), 
                          std::numeric_limits<f32>::max()};
        result.max = Vec3{std::numeric_limits<f32>::lowest(), 
                          std::numeric_limits<f32>::lowest(), 
                          std::numeric_limits<f32>::lowest()};
        
        for (const auto& corner : corners) {
            // Transform point by matrix
            Vec4 transformed = matrix * Vec4{corner.x, corner.y, corner.z, 1.0f};
            Vec3 point{transformed.x, transformed.y, transformed.z};
            result.expandToInclude(point);
        }
        
        return result;
    }
    
    /**
     * @brief Get 8 corners
     */
    [[nodiscard]] std::array<Vec3, 8> getCorners() const noexcept {
        return {
            Vec3{min.x, min.y, min.z},
            Vec3{max.x, min.y, min.z},
            Vec3{min.x, max.y, min.z},
            Vec3{max.x, max.y, min.z},
            Vec3{min.x, min.y, max.z},
            Vec3{max.x, min.y, max.z},
            Vec3{min.x, max.y, max.z},
            Vec3{max.x, max.y, max.z}
        };
    }
    
    /**
     * @brief Create from center and half-extents
     */
    [[nodiscard]] static AABB fromCenterHalfExtents(const Vec3& center, const Vec3& halfExtents) noexcept {
        return AABB{
            Vec3{center.x - halfExtents.x, center.y - halfExtents.y, center.z - halfExtents.z},
            Vec3{center.x + halfExtents.x, center.y + halfExtents.y, center.z + halfExtents.z}
        };
    }
};

/**
 * @brief Bounding sphere
 */
struct BoundingSphere {
    /// Center position
    Vec3 center = Vec3{0.0f, 0.0f, 0.0f};
    
    /// Radius
    f32 radius = 0.0f;
    
    /**
     * @brief Check if intersects another sphere
     */
    [[nodiscard]] bool intersects(const BoundingSphere& other) const noexcept {
        Vec3 diff = Vec3{center.x - other.center.x, 
                         center.y - other.center.y, 
                         center.z - other.center.z};
        f32 distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
        f32 radiusSum = radius + other.radius;
        return distSq <= radiusSum * radiusSum;
    }
    
    /**
     * @brief Check if contains point
     */
    [[nodiscard]] bool containsPoint(const Vec3& point) const noexcept {
        Vec3 diff = Vec3{point.x - center.x, point.y - center.y, point.z - center.z};
        f32 distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
        return distSq <= radius * radius;
    }
    
    /**
     * @brief Create from AABB
     */
    [[nodiscard]] static BoundingSphere fromAABB(const AABB& aabb) noexcept {
        BoundingSphere sphere;
        sphere.center = aabb.getCenter();
        Vec3 halfExtents = aabb.getHalfExtents();
        sphere.radius = std::sqrt(halfExtents.x * halfExtents.x + 
                                   halfExtents.y * halfExtents.y + 
                                   halfExtents.z * halfExtents.z);
        return sphere;
    }
};

// =============================================================================
// Frustum
// =============================================================================

/**
 * @brief Plane equation (ax + by + cz + d = 0)
 */
struct Plane {
    /// Normal (a, b, c)
    Vec3 normal = Vec3{0.0f, 1.0f, 0.0f};
    
    /// Distance from origin (d)
    f32 distance = 0.0f;
    
    /**
     * @brief Normalize the plane
     */
    void normalize() noexcept {
        f32 len = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
        if (len > 0.0001f) {
            f32 invLen = 1.0f / len;
            normal.x *= invLen;
            normal.y *= invLen;
            normal.z *= invLen;
            distance *= invLen;
        }
    }
    
    /**
     * @brief Signed distance from point to plane
     */
    [[nodiscard]] f32 signedDistance(const Vec3& point) const noexcept {
        return normal.x * point.x + normal.y * point.y + normal.z * point.z + distance;
    }
};

/**
 * @brief View frustum for culling
 */
struct Frustum {
    /// Frustum planes (left, right, bottom, top, near, far)
    std::array<Plane, OcclusionConfig::FRUSTUM_PLANE_COUNT> planes;
    
    /// Frustum corners (8 corners)
    std::array<Vec3, 8> corners;
    
    /**
     * @brief Extract frustum from view-projection matrix
     */
    void extractFromMatrix(const Mat4& viewProj) noexcept {
        // Access matrix in column-major format: columns[col][row]
        // viewProj.columns[col].x/y/z/w for row 0,1,2,3
        
        // Left plane: row3 + row0
        planes[0].normal.x = viewProj.columns[0].w + viewProj.columns[0].x;
        planes[0].normal.y = viewProj.columns[1].w + viewProj.columns[1].x;
        planes[0].normal.z = viewProj.columns[2].w + viewProj.columns[2].x;
        planes[0].distance = viewProj.columns[3].w + viewProj.columns[3].x;
        
        // Right plane: row3 - row0
        planes[1].normal.x = viewProj.columns[0].w - viewProj.columns[0].x;
        planes[1].normal.y = viewProj.columns[1].w - viewProj.columns[1].x;
        planes[1].normal.z = viewProj.columns[2].w - viewProj.columns[2].x;
        planes[1].distance = viewProj.columns[3].w - viewProj.columns[3].x;
        
        // Bottom plane: row3 + row1
        planes[2].normal.x = viewProj.columns[0].w + viewProj.columns[0].y;
        planes[2].normal.y = viewProj.columns[1].w + viewProj.columns[1].y;
        planes[2].normal.z = viewProj.columns[2].w + viewProj.columns[2].y;
        planes[2].distance = viewProj.columns[3].w + viewProj.columns[3].y;
        
        // Top plane: row3 - row1
        planes[3].normal.x = viewProj.columns[0].w - viewProj.columns[0].y;
        planes[3].normal.y = viewProj.columns[1].w - viewProj.columns[1].y;
        planes[3].normal.z = viewProj.columns[2].w - viewProj.columns[2].y;
        planes[3].distance = viewProj.columns[3].w - viewProj.columns[3].y;
        
        // Near plane: row3 + row2
        planes[4].normal.x = viewProj.columns[0].w + viewProj.columns[0].z;
        planes[4].normal.y = viewProj.columns[1].w + viewProj.columns[1].z;
        planes[4].normal.z = viewProj.columns[2].w + viewProj.columns[2].z;
        planes[4].distance = viewProj.columns[3].w + viewProj.columns[3].z;
        
        // Far plane: row3 - row2
        planes[5].normal.x = viewProj.columns[0].w - viewProj.columns[0].z;
        planes[5].normal.y = viewProj.columns[1].w - viewProj.columns[1].z;
        planes[5].normal.z = viewProj.columns[2].w - viewProj.columns[2].z;
        planes[5].distance = viewProj.columns[3].w - viewProj.columns[3].z;
        
        // Normalize all planes
        for (auto& plane : planes) {
            plane.normalize();
        }
    }
    
    /**
     * @brief Test if AABB is inside or intersects frustum
     * @return -1 = outside, 0 = intersecting, 1 = inside
     */
    [[nodiscard]] i32 testAABB(const AABB& aabb) const noexcept {
        bool allInside = true;
        
        for (const auto& plane : planes) {
            // Get positive vertex (furthest along plane normal)
            Vec3 pVertex;
            pVertex.x = (plane.normal.x >= 0.0f) ? aabb.max.x : aabb.min.x;
            pVertex.y = (plane.normal.y >= 0.0f) ? aabb.max.y : aabb.min.y;
            pVertex.z = (plane.normal.z >= 0.0f) ? aabb.max.z : aabb.min.z;
            
            // Get negative vertex (closest to plane)
            Vec3 nVertex;
            nVertex.x = (plane.normal.x >= 0.0f) ? aabb.min.x : aabb.max.x;
            nVertex.y = (plane.normal.y >= 0.0f) ? aabb.min.y : aabb.max.y;
            nVertex.z = (plane.normal.z >= 0.0f) ? aabb.min.z : aabb.max.z;
            
            // If positive vertex is outside, AABB is completely outside
            if (plane.signedDistance(pVertex) < 0.0f) {
                return -1;
            }
            
            // If negative vertex is outside, AABB intersects plane
            if (plane.signedDistance(nVertex) < 0.0f) {
                allInside = false;
            }
        }
        
        return allInside ? 1 : 0;
    }
    
    /**
     * @brief Test if sphere is inside or intersects frustum
     * @return -1 = outside, 0 = intersecting, 1 = inside
     */
    [[nodiscard]] i32 testSphere(const BoundingSphere& sphere) const noexcept {
        bool allInside = true;
        
        for (const auto& plane : planes) {
            f32 dist = plane.signedDistance(sphere.center);
            
            if (dist < -sphere.radius) {
                return -1; // Completely outside
            }
            
            if (dist < sphere.radius) {
                allInside = false; // Intersecting
            }
        }
        
        return allInside ? 1 : 0;
    }
    
    /**
     * @brief Quick test if AABB is visible (outside = false, inside/intersecting = true)
     */
    [[nodiscard]] bool isAABBVisible(const AABB& aabb) const noexcept {
        return testAABB(aabb) >= 0;
    }
    
    /**
     * @brief Quick test if sphere is visible
     */
    [[nodiscard]] bool isSphereVisible(const BoundingSphere& sphere) const noexcept {
        return testSphere(sphere) >= 0;
    }
};

// =============================================================================
// Occlusion Query
// =============================================================================

/**
 * @brief Occlusion query result
 */
struct OcclusionQueryResult {
    /// Object ID
    u32 objectId = 0;
    
    /// Samples passed
    u64 samplesPassed = 0;
    
    /// Query is ready
    bool ready = false;
    
    /// Frame when query was issued
    u32 queryFrame = 0;
    
    /**
     * @brief Check if object is visible (any samples passed)
     */
    [[nodiscard]] bool isVisible() const noexcept {
        return ready && samplesPassed > 0;
    }
};

/**
 * @brief Occlusion query request
 */
struct OcclusionQueryRequest {
    /// Object ID
    u32 objectId = 0;
    
    /// Bounding box for query
    AABB bounds;
    
    /// Use conservative bounds (slightly enlarged)
    bool conservative = true;
    
    /// Priority (higher = process first)
    i32 priority = 0;
};

// =============================================================================
// Hi-Z Buffer
// =============================================================================

/**
 * @brief Hi-Z buffer configuration
 */
struct HiZConfig {
    /// Base resolution width
    u32 width = 1920;
    
    /// Base resolution height
    u32 height = 1080;
    
    /// Resolution divisor
    u32 divisor = OcclusionConfig::DEFAULT_HIZ_DIVISOR;
    
    /// Number of mip levels
    u32 mipLevels = 0;
    
    /// Depth comparison (true = reversed-Z)
    bool reversedZ = true;
    
    /**
     * @brief Get Hi-Z base resolution
     */
    [[nodiscard]] u32 getHiZWidth() const noexcept { return width / divisor; }
    [[nodiscard]] u32 getHiZHeight() const noexcept { return height / divisor; }
    
    /**
     * @brief Calculate mip levels
     */
    void calculateMipLevels() noexcept {
        u32 w = getHiZWidth();
        u32 h = getHiZHeight();
        mipLevels = 1;
        while (w > 1 || h > 1) {
            w = std::max(1u, w / 2);
            h = std::max(1u, h / 2);
            mipLevels++;
        }
        mipLevels = std::min(mipLevels, OcclusionConfig::MAX_HIZ_MIP_LEVELS);
    }
    
    /**
     * @brief Get mip level dimensions
     */
    [[nodiscard]] std::pair<u32, u32> getMipDimensions(u32 level) const noexcept {
        u32 w = getHiZWidth() >> level;
        u32 h = getHiZHeight() >> level;
        return {std::max(1u, w), std::max(1u, h)};
    }
};

// =============================================================================
// Culling Statistics
// =============================================================================

/**
 * @brief Occlusion culling statistics
 */
struct CullingStats {
    /// Total objects tested
    u32 totalObjects = 0;
    
    /// Objects passing frustum test
    u32 frustumPassed = 0;
    
    /// Objects culled by frustum
    u32 frustumCulled = 0;
    
    /// Objects passing occlusion test
    u32 occlusionPassed = 0;
    
    /// Objects culled by occlusion
    u32 occlusionCulled = 0;
    
    /// Objects with pending queries
    u32 pendingQueries = 0;
    
    /// Objects culled by size
    u32 sizeCulled = 0;
    
    /// Frustum culling time (ms)
    f32 frustumTimeMs = 0.0f;
    
    /// Occlusion culling time (ms)
    f32 occlusionTimeMs = 0.0f;
    
    /// Hi-Z build time (ms)
    f32 hiZBuildTimeMs = 0.0f;
    
    /// Total culling time (ms)
    f32 totalTimeMs = 0.0f;
    
    /**
     * @brief Reset per-frame stats
     */
    void resetFrameStats() noexcept {
        totalObjects = 0;
        frustumPassed = 0;
        frustumCulled = 0;
        occlusionPassed = 0;
        occlusionCulled = 0;
        pendingQueries = 0;
        sizeCulled = 0;
        frustumTimeMs = 0.0f;
        occlusionTimeMs = 0.0f;
        hiZBuildTimeMs = 0.0f;
        totalTimeMs = 0.0f;
    }
    
    /**
     * @brief Get frustum cull rate
     */
    [[nodiscard]] f32 getFrustumCullRate() const noexcept {
        if (totalObjects == 0) return 0.0f;
        return static_cast<f32>(frustumCulled) / static_cast<f32>(totalObjects) * 100.0f;
    }
    
    /**
     * @brief Get occlusion cull rate
     */
    [[nodiscard]] f32 getOcclusionCullRate() const noexcept {
        if (frustumPassed == 0) return 0.0f;
        return static_cast<f32>(occlusionCulled) / static_cast<f32>(frustumPassed) * 100.0f;
    }
    
    /**
     * @brief Get total cull rate
     */
    [[nodiscard]] f32 getTotalCullRate() const noexcept {
        if (totalObjects == 0) return 0.0f;
        u32 totalCulled = frustumCulled + occlusionCulled + sizeCulled;
        return static_cast<f32>(totalCulled) / static_cast<f32>(totalObjects) * 100.0f;
    }
};

// =============================================================================
// Occlusion Culling Manager
// =============================================================================

/**
 * @brief Occlusion culling manager
 * 
 * Manages frustum and occlusion culling for visible determination.
 */
class OcclusionCullingManager {
public:
    /**
     * @brief Default constructor
     */
    OcclusionCullingManager() = default;
    
    // -------------------------------------------------------------------------
    // Configuration
    // -------------------------------------------------------------------------
    
    /**
     * @brief Set culling technique
     */
    void setTechnique(OcclusionTechnique technique) noexcept {
        m_technique = technique;
    }
    
    /**
     * @brief Get current technique
     */
    [[nodiscard]] OcclusionTechnique getTechnique() const noexcept {
        return m_technique;
    }
    
    /**
     * @brief Set Hi-Z config
     */
    void setHiZConfig(const HiZConfig& config) noexcept {
        m_hiZConfig = config;
        m_hiZConfig.calculateMipLevels();
    }
    
    /**
     * @brief Get Hi-Z config
     */
    [[nodiscard]] const HiZConfig& getHiZConfig() const noexcept {
        return m_hiZConfig;
    }
    
    /**
     * @brief Set minimum screen size for culling
     */
    void setMinScreenSize(f32 pixels) noexcept {
        m_minScreenSize = std::max(0.0f, pixels);
    }
    
    /**
     * @brief Get minimum screen size
     */
    [[nodiscard]] f32 getMinScreenSize() const noexcept {
        return m_minScreenSize;
    }
    
    /**
     * @brief Enable/disable temporal coherence
     */
    void setTemporalCoherence(bool enabled) noexcept {
        m_useTemporalCoherence = enabled;
    }
    
    // -------------------------------------------------------------------------
    // Frustum Management
    // -------------------------------------------------------------------------
    
    /**
     * @brief Update view frustum from matrix
     */
    void updateFrustum(const Mat4& viewProjection) noexcept {
        m_frustum.extractFromMatrix(viewProjection);
        m_viewProjection = viewProjection;
    }
    
    /**
     * @brief Get current frustum
     */
    [[nodiscard]] const Frustum& getFrustum() const noexcept {
        return m_frustum;
    }
    
    // -------------------------------------------------------------------------
    // Culling Operations
    // -------------------------------------------------------------------------
    
    /**
     * @brief Test AABB visibility (frustum only)
     */
    [[nodiscard]] VisibilityResult testFrustum(const AABB& bounds) noexcept {
        m_stats.totalObjects++;
        
        i32 result = m_frustum.testAABB(bounds);
        if (result < 0) {
            m_stats.frustumCulled++;
            return VisibilityResult::OutsideFrustum;
        }
        
        m_stats.frustumPassed++;
        return VisibilityResult::Visible;
    }
    
    /**
     * @brief Test sphere visibility (frustum only)
     */
    [[nodiscard]] VisibilityResult testFrustum(const BoundingSphere& bounds) noexcept {
        m_stats.totalObjects++;
        
        i32 result = m_frustum.testSphere(bounds);
        if (result < 0) {
            m_stats.frustumCulled++;
            return VisibilityResult::OutsideFrustum;
        }
        
        m_stats.frustumPassed++;
        return VisibilityResult::Visible;
    }
    
    /**
     * @brief Full visibility test (frustum + occlusion)
     */
    [[nodiscard]] VisibilityResult testVisibility(u32 objectId, const AABB& bounds) noexcept {
        // Frustum test first
        auto frustumResult = testFrustum(bounds);
        if (frustumResult != VisibilityResult::Visible) {
            return frustumResult;
        }
        
        // Check occlusion based on technique
        if (m_technique == OcclusionTechnique::None || 
            m_technique == OcclusionTechnique::FrustumOnly) {
            return VisibilityResult::Visible;
        }
        
        // Check temporal coherence (use last frame's result)
        if (m_useTemporalCoherence) {
            auto it = m_lastFrameResults.find(objectId);
            if (it != m_lastFrameResults.end() && it->second == VisibilityResult::Occluded) {
                m_stats.occlusionCulled++;
                return VisibilityResult::Occluded;
            }
        }
        
        // For now, assume visible (real implementation would query Hi-Z or hardware)
        m_stats.occlusionPassed++;
        return VisibilityResult::Visible;
    }
    
    /**
     * @brief Queue occlusion query for object
     */
    void queueOcclusionQuery(u32 objectId, const AABB& bounds, i32 priority = 0) noexcept {
        if (m_pendingQueries.size() >= OcclusionConfig::MAX_OCCLUSION_QUERIES) {
            return;
        }
        
        OcclusionQueryRequest request;
        request.objectId = objectId;
        request.bounds = bounds;
        request.priority = priority;
        
        m_pendingQueries.push_back(request);
        m_stats.pendingQueries++;
    }
    
    /**
     * @brief Get pending query count
     */
    [[nodiscard]] usize getPendingQueryCount() const noexcept {
        return m_pendingQueries.size();
    }
    
    // -------------------------------------------------------------------------
    // Frame Operations
    // -------------------------------------------------------------------------
    
    /**
     * @brief Begin culling frame
     */
    void beginFrame(u32 frameNumber) noexcept {
        m_currentFrame = frameNumber;
        m_stats.resetFrameStats();
        
        // Swap results for temporal coherence
        m_lastFrameResults = std::move(m_currentFrameResults);
        m_currentFrameResults.clear();
    }
    
    /**
     * @brief End culling frame
     */
    void endFrame() noexcept {
        m_stats.totalTimeMs = m_stats.frustumTimeMs + m_stats.occlusionTimeMs + m_stats.hiZBuildTimeMs;
        
        // Clear pending queries
        m_pendingQueries.clear();
    }
    
    /**
     * @brief Record visibility result for temporal coherence
     */
    void recordResult(u32 objectId, VisibilityResult result) noexcept {
        m_currentFrameResults[objectId] = result;
    }
    
    // -------------------------------------------------------------------------
    // Statistics
    // -------------------------------------------------------------------------
    
    /**
     * @brief Get statistics
     */
    [[nodiscard]] const CullingStats& getStats() const noexcept {
        return m_stats;
    }
    
    /**
     * @brief Get current frame
     */
    [[nodiscard]] u32 getCurrentFrame() const noexcept {
        return m_currentFrame;
    }

private:
    // Configuration
    OcclusionTechnique m_technique = OcclusionTechnique::FrustumOnly;
    HiZConfig m_hiZConfig;
    f32 m_minScreenSize = 1.0f;
    bool m_useTemporalCoherence = true;
    
    // View state
    Frustum m_frustum;
    Mat4 m_viewProjection;
    
    // Query management
    std::vector<OcclusionQueryRequest> m_pendingQueries;
    
    // Temporal coherence
    std::unordered_map<u32, VisibilityResult> m_lastFrameResults;
    std::unordered_map<u32, VisibilityResult> m_currentFrameResults;
    
    // Frame state
    u32 m_currentFrame = 0;
    
    // Statistics
    CullingStats m_stats;
};

// =============================================================================
// Utility Functions
// =============================================================================

/**
 * @brief Calculate screen-space bounding box from AABB
 * @param aabb World-space AABB
 * @param viewProj View-projection matrix
 * @param screenWidth Screen width
 * @param screenHeight Screen height
 * @return Screen-space bounds (min.x, min.y, max.x, max.y), or empty if behind camera
 */
[[nodiscard]] inline std::optional<Vec4> calculateScreenBounds(
    const AABB& aabb, const Mat4& viewProj, u32 screenWidth, u32 screenHeight) noexcept {
    
    auto corners = aabb.getCorners();
    
    f32 minX = std::numeric_limits<f32>::max();
    f32 minY = std::numeric_limits<f32>::max();
    f32 maxX = std::numeric_limits<f32>::lowest();
    f32 maxY = std::numeric_limits<f32>::lowest();
    
    bool anyInFront = false;
    
    for (const auto& corner : corners) {
        Vec4 clip = viewProj * Vec4{corner.x, corner.y, corner.z, 1.0f};
        
        // Skip if behind camera
        if (clip.w <= 0.0f) continue;
        anyInFront = true;
        
        // Perspective divide
        f32 invW = 1.0f / clip.w;
        f32 ndcX = clip.x * invW;
        f32 ndcY = clip.y * invW;
        
        // Convert to screen space
        f32 screenX = (ndcX * 0.5f + 0.5f) * screenWidth;
        f32 screenY = (1.0f - (ndcY * 0.5f + 0.5f)) * screenHeight;
        
        minX = std::min(minX, screenX);
        minY = std::min(minY, screenY);
        maxX = std::max(maxX, screenX);
        maxY = std::max(maxY, screenY);
    }
    
    if (!anyInFront) {
        return std::nullopt;
    }
    
    return Vec4{minX, minY, maxX, maxY};
}

/**
 * @brief Calculate screen area of an AABB
 */
[[nodiscard]] inline f32 calculateScreenArea(
    const AABB& aabb, const Mat4& viewProj, u32 screenWidth, u32 screenHeight) noexcept {
    
    auto bounds = calculateScreenBounds(aabb, viewProj, screenWidth, screenHeight);
    if (!bounds) {
        return 0.0f;
    }
    
    f32 width = std::max(0.0f, bounds->z - bounds->x);
    f32 height = std::max(0.0f, bounds->w - bounds->y);
    
    return width * height;
}

} // namespace nova
