/**
 * @file terrain_system.hpp
 * @brief Advanced terrain rendering system for NovaCore engine
 * @author NovaForge Team
 * @copyright Copyright (c) 2024-2025 NovaForge - All Rights Reserved
 * 
 * Provides comprehensive terrain rendering including:
 * - Clipmap-based terrain LOD
 * - Virtual texturing for terrain materials
 * - GPU-driven terrain tessellation
 * - Procedural terrain generation
 */

#pragma once

#include <nova/core/types/types.hpp>
#include <nova/core/math/math.hpp>
#include <array>
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <cmath>
#include <algorithm>

namespace nova {

// =============================================================================
// TERRAIN CONFIGURATION
// =============================================================================

/**
 * @brief Terrain system configuration constants
 */
struct TerrainConfig {
    /// Maximum terrain size (meters)
    static constexpr f32 MAX_TERRAIN_SIZE = 1000000.0f;
    
    /// Maximum heightmap resolution
    static constexpr u32 MAX_HEIGHTMAP_RESOLUTION = 8192;
    
    /// Maximum number of clipmap levels
    static constexpr u32 MAX_CLIPMAP_LEVELS = 12;
    
    /// Default clipmap level size
    static constexpr u32 DEFAULT_CLIPMAP_SIZE = 255;
    
    /// Maximum texture layers
    static constexpr u32 MAX_TEXTURE_LAYERS = 16;
    
    /// Maximum number of terrain chunks
    static constexpr u32 MAX_CHUNKS = 65536;
    
    /// Default tessellation factor
    static constexpr f32 DEFAULT_TESSELLATION = 64.0f;
    
    /// Virtual texture tile size
    static constexpr u32 VT_TILE_SIZE = 256;
    
    /// Maximum detail meshes per chunk
    static constexpr u32 MAX_DETAIL_MESHES = 256;
    
    /// Default terrain scale
    static constexpr f32 DEFAULT_SCALE = 1.0f;
};

// =============================================================================
// TERRAIN ENUMERATIONS
// =============================================================================

/**
 * @brief Terrain LOD technique
 */
enum class TerrainLODTechnique : u8 {
    Clipmap,        ///< Geometry clipmaps (GPU-friendly)
    Quadtree,       ///< Traditional quadtree chunking
    CDLOD,          ///< Continuous Distance-based LOD
    Tessellation    ///< GPU tessellation with displacement
};

/**
 * @brief Get string name for LOD technique
 */
[[nodiscard]] constexpr const char* getTerrainLODTechniqueName(TerrainLODTechnique tech) noexcept {
    switch (tech) {
        case TerrainLODTechnique::Clipmap: return "Clipmap";
        case TerrainLODTechnique::Quadtree: return "Quadtree";
        case TerrainLODTechnique::CDLOD: return "CDLOD";
        case TerrainLODTechnique::Tessellation: return "Tessellation";
        default: return "Unknown";
    }
}

/**
 * @brief Terrain texture blending mode
 */
enum class TerrainBlendMode : u8 {
    Height,         ///< Height-based blending
    Slope,          ///< Slope-based blending
    Noise,          ///< Noise-based variation
    Combined        ///< Combined approach
};

/**
 * @brief Heightmap source type
 */
enum class HeightmapSource : u8 {
    Texture,        ///< Loaded from texture
    Procedural,     ///< Generated procedurally
    RAW,            ///< Raw heightmap file
    Hybrid          ///< Combined sources
};

/**
 * @brief Terrain brush type for editing
 */
enum class TerrainBrushType : u8 {
    Raise,          ///< Raise terrain
    Lower,          ///< Lower terrain
    Flatten,        ///< Flatten to target height
    Smooth,         ///< Smooth terrain
    Noise,          ///< Add noise
    Paint,          ///< Paint texture layers
    Erode           ///< Erosion simulation
};

/**
 * @brief Detail mesh distribution
 */
enum class DetailDistribution : u8 {
    Uniform,        ///< Uniform distribution
    Clustered,      ///< Clustered distribution
    Slope,          ///< Slope-based
    HeightRange,    ///< Height-based
    TextureBased    ///< Based on texture layer
};

/**
 * @brief Terrain chunk state
 */
enum class ChunkState : u8 {
    Unloaded,       ///< Not in memory
    Loading,        ///< Being loaded
    Loaded,         ///< Ready for rendering
    Streaming,      ///< Streaming higher detail
    Error           ///< Load error
};

// =============================================================================
// TERRAIN STRUCTURES
// =============================================================================

/**
 * @brief Heightmap data structure
 */
struct Heightmap {
    u32 width{0};
    u32 height{0};
    f32 minHeight{0.0f};
    f32 maxHeight{100.0f};
    std::vector<f32> data;
    
    /// Create with dimensions
    static Heightmap create(u32 w, u32 h, f32 minH = 0.0f, f32 maxH = 100.0f) noexcept {
        Heightmap hm;
        hm.width = w;
        hm.height = h;
        hm.minHeight = minH;
        hm.maxHeight = maxH;
        hm.data.resize(w * h, 0.0f);
        return hm;
    }
    
    /// Sample height at normalized coordinates
    [[nodiscard]] f32 sample(f32 u, f32 v) const noexcept {
        if (data.empty()) return 0.0f;
        
        u = std::clamp(u, 0.0f, 1.0f);
        v = std::clamp(v, 0.0f, 1.0f);
        
        f32 fx = u * (width - 1);
        f32 fy = v * (height - 1);
        
        u32 x0 = static_cast<u32>(fx);
        u32 y0 = static_cast<u32>(fy);
        u32 x1 = std::min(x0 + 1, width - 1);
        u32 y1 = std::min(y0 + 1, height - 1);
        
        f32 tx = fx - x0;
        f32 ty = fy - y0;
        
        f32 h00 = data[y0 * width + x0];
        f32 h10 = data[y0 * width + x1];
        f32 h01 = data[y1 * width + x0];
        f32 h11 = data[y1 * width + x1];
        
        f32 h0 = h00 * (1.0f - tx) + h10 * tx;
        f32 h1 = h01 * (1.0f - tx) + h11 * tx;
        
        return h0 * (1.0f - ty) + h1 * ty;
    }
    
    /// Sample normal at normalized coordinates
    [[nodiscard]] Vec3 sampleNormal(f32 u, f32 v, f32 scale = 1.0f) const noexcept {
        f32 delta = 1.0f / std::max(width, height);
        
        f32 hl = sample(u - delta, v);
        f32 hr = sample(u + delta, v);
        f32 hd = sample(u, v - delta);
        f32 hu = sample(u, v + delta);
        
        Vec3 normal;
        normal.x = (hl - hr) * scale;
        normal.z = (hd - hu) * scale;
        normal.y = 2.0f * delta;
        
        return normal.normalized();
    }
    
    /// Check if valid
    [[nodiscard]] bool isValid() const noexcept {
        return width > 0 && height > 0 && !data.empty();
    }
    
    /// Get memory size in bytes
    [[nodiscard]] u64 getMemorySize() const noexcept {
        return data.size() * sizeof(f32);
    }
};

/**
 * @brief Terrain texture layer
 */
struct TerrainTextureLayer {
    std::string albedoTexture;
    std::string normalTexture;
    std::string roughnessTexture;
    std::string aoTexture;
    std::string displacementTexture;
    
    f32 uvScale{1.0f};                   ///< Texture UV scale
    f32 uvRotation{0.0f};                ///< Texture UV rotation (radians)
    f32 blendSharpness{1.0f};            ///< Blend transition sharpness
    f32 heightOffset{0.0f};              ///< Height-based blending offset
    f32 slopeMin{0.0f};                  ///< Minimum slope for application
    f32 slopeMax{90.0f};                 ///< Maximum slope for application
    f32 heightMin{-10000.0f};            ///< Minimum height for application
    f32 heightMax{10000.0f};             ///< Maximum height for application
    Vec3 tintColor{1.0f, 1.0f, 1.0f};    ///< Color tint
    
    /// Check if in slope range
    [[nodiscard]] bool isInSlopeRange(f32 slopeDegrees) const noexcept {
        return slopeDegrees >= slopeMin && slopeDegrees <= slopeMax;
    }
    
    /// Check if in height range
    [[nodiscard]] bool isInHeightRange(f32 h) const noexcept {
        return h >= heightMin && h <= heightMax;
    }
    
    /// Calculate blend weight
    [[nodiscard]] f32 calculateBlendWeight(f32 h, f32 slopeDegrees, f32 noiseValue = 0.0f) const noexcept {
        f32 heightWeight = 1.0f;
        if (heightMax > heightMin) {
            f32 heightRange = heightMax - heightMin;
            f32 heightCenter = (heightMin + heightMax) * 0.5f;
            heightWeight = 1.0f - std::abs(h - heightCenter) / (heightRange * 0.5f);
            heightWeight = std::clamp(heightWeight + heightOffset, 0.0f, 1.0f);
        }
        
        f32 slopeWeight = 1.0f;
        if (slopeMax > slopeMin) {
            f32 slopeRange = slopeMax - slopeMin;
            f32 slopeCenter = (slopeMin + slopeMax) * 0.5f;
            slopeWeight = 1.0f - std::abs(slopeDegrees - slopeCenter) / (slopeRange * 0.5f);
            slopeWeight = std::clamp(slopeWeight, 0.0f, 1.0f);
        }
        
        f32 weight = heightWeight * slopeWeight + noiseValue * 0.1f;
        return std::pow(std::clamp(weight, 0.0f, 1.0f), blendSharpness);
    }
};

/**
 * @brief Clipmap level description
 */
struct ClipmapLevel {
    u32 level{0};                        ///< Level index (0 = finest)
    u32 resolution{255};                 ///< Grid resolution
    f32 cellSize{1.0f};                  ///< World space cell size
    f32 scale{1.0f};                     ///< Scale relative to level 0
    Vec2 centerOffset{0.0f, 0.0f};       ///< Center offset in world space
    bool needsUpdate{true};              ///< Needs geometry update
    
    /// Calculate world extent
    [[nodiscard]] f32 getWorldExtent() const noexcept {
        return resolution * cellSize;
    }
    
    /// Calculate vertex count
    [[nodiscard]] u32 getVertexCount() const noexcept {
        return resolution * resolution;
    }
    
    /// Calculate index count (for triangle strip or indexed rendering)
    [[nodiscard]] u32 getIndexCount() const noexcept {
        return (resolution - 1) * (resolution - 1) * 6;
    }
};

/**
 * @brief Terrain chunk for streaming
 */
struct TerrainChunk {
    u32 id{0};
    i32 gridX{0};                        ///< Grid X position
    i32 gridZ{0};                        ///< Grid Z position
    Vec3 worldPosition{0.0f, 0.0f, 0.0f}; ///< World space position
    Vec3 boundsMin{0.0f, 0.0f, 0.0f};    ///< AABB minimum
    Vec3 boundsMax{0.0f, 0.0f, 0.0f};    ///< AABB maximum
    f32 minHeight{0.0f};                 ///< Minimum height in chunk
    f32 maxHeight{100.0f};               ///< Maximum height in chunk
    ChunkState state{ChunkState::Unloaded};
    u32 lodLevel{0};                     ///< Current LOD level
    f32 lastAccessTime{0.0f};            ///< For LRU streaming
    
    /// Calculate center
    [[nodiscard]] Vec3 getCenter() const noexcept {
        return (boundsMin + boundsMax) * 0.5f;
    }
    
    /// Calculate size
    [[nodiscard]] Vec3 getSize() const noexcept {
        return boundsMax - boundsMin;
    }
    
    /// Check if point is inside
    [[nodiscard]] bool contains(const Vec3& point) const noexcept {
        return point.x >= boundsMin.x && point.x <= boundsMax.x &&
               point.y >= boundsMin.y && point.y <= boundsMax.y &&
               point.z >= boundsMin.z && point.z <= boundsMax.z;
    }
    
    /// Update bounds
    void updateBounds(const Vec3& center, const Vec3& halfSize) noexcept {
        boundsMin = center - halfSize;
        boundsMax = center + halfSize;
    }
};

/**
 * @brief Terrain brush parameters
 */
struct TerrainBrush {
    TerrainBrushType type{TerrainBrushType::Raise};
    f32 radius{10.0f};                   ///< Brush radius (world units)
    f32 strength{1.0f};                  ///< Effect strength
    f32 falloff{0.5f};                   ///< Edge falloff (0 = hard, 1 = soft)
    f32 targetHeight{0.0f};              ///< Target height for flatten
    u32 textureLayer{0};                 ///< Target texture layer for painting
    bool additive{true};                 ///< Additive or replace
    
    /// Calculate falloff at distance
    [[nodiscard]] f32 getFalloff(f32 distance) const noexcept {
        if (distance >= radius) return 0.0f;
        if (distance <= 0.0f) return 1.0f;
        
        f32 t = distance / radius;
        f32 edge = 1.0f - falloff;
        
        if (t <= edge) return 1.0f;
        
        f32 fade = (t - edge) / falloff;
        return 1.0f - fade * fade;
    }
};

/**
 * @brief Detail mesh for terrain vegetation/rocks
 */
struct DetailMesh {
    std::string meshPath;
    std::string materialPath;
    
    f32 density{1.0f};                   ///< Instances per square meter
    f32 minScale{0.8f};                  ///< Minimum random scale
    f32 maxScale{1.2f};                  ///< Maximum random scale
    f32 minRotation{0.0f};               ///< Minimum Y rotation (radians)
    f32 maxRotation{6.28318f};           ///< Maximum Y rotation (radians)
    f32 alignToNormal{0.0f};             ///< How much to align to surface normal (0-1)
    f32 minSlope{0.0f};                  ///< Minimum slope for placement
    f32 maxSlope{45.0f};                 ///< Maximum slope for placement
    f32 minHeight{-10000.0f};            ///< Minimum height for placement
    f32 maxHeight{10000.0f};             ///< Maximum height for placement
    u32 textureLayerMask{0xFFFFFFFF};    ///< Which texture layers to place on
    DetailDistribution distribution{DetailDistribution::Uniform};
    f32 viewDistance{500.0f};            ///< Maximum view distance
    f32 fadeStart{400.0f};               ///< Distance to start fading
    
    /// Check if placement is valid at position
    [[nodiscard]] bool isValidPlacement(f32 h, f32 slopeDegrees, u32 textureLayer) const noexcept {
        if (h < minHeight || h > maxHeight) return false;
        if (slopeDegrees < minSlope || slopeDegrees > maxSlope) return false;
        if ((textureLayerMask & (1u << textureLayer)) == 0) return false;
        return true;
    }
    
    /// Calculate fade alpha based on distance
    [[nodiscard]] f32 getFadeAlpha(f32 distance) const noexcept {
        if (distance >= viewDistance) return 0.0f;
        if (distance <= fadeStart) return 1.0f;
        return 1.0f - (distance - fadeStart) / (viewDistance - fadeStart);
    }
};

/**
 * @brief GPU terrain data for shader consumption
 */
struct alignas(16) GPUTerrainData {
    Mat4 worldTransform;                 ///< World transform matrix
    Vec4 terrainParams;                  ///< x=size, y=heightScale, z=uvScale, w=tessellation
    Vec4 heightmapParams;                ///< x=width, y=height, z=minHeight, w=maxHeight
    Vec4 lodParams;                      ///< x=lodBias, y=minTess, z=maxTess, w=morphStart
    Vec4 clipmapParams[TerrainConfig::MAX_CLIPMAP_LEVELS]; ///< Per-level params
    
    static GPUTerrainData create(f32 size, f32 heightScale, u32 hmWidth, u32 hmHeight) noexcept {
        GPUTerrainData data;
        data.worldTransform = Mat4::identity();
        data.terrainParams = Vec4{size, heightScale, 1.0f / size, TerrainConfig::DEFAULT_TESSELLATION};
        data.heightmapParams = Vec4{static_cast<f32>(hmWidth), static_cast<f32>(hmHeight), 0.0f, 100.0f};
        data.lodParams = Vec4{0.0f, 1.0f, 64.0f, 0.8f};
        return data;
    }
};

/**
 * @brief Terrain settings
 */
struct TerrainSettings {
    TerrainLODTechnique lodTechnique{TerrainLODTechnique::Clipmap};
    u32 clipmapLevels{8};                ///< Number of clipmap levels
    u32 clipmapResolution{255};          ///< Resolution per clipmap level
    f32 tessellationFactor{64.0f};       ///< Base tessellation factor
    f32 tessellationMinDistance{10.0f};  ///< Distance for max tessellation
    f32 tessellationMaxDistance{500.0f}; ///< Distance for min tessellation
    f32 lodBias{0.0f};                   ///< LOD selection bias
    f32 morphingBlend{0.3f};             ///< LOD morphing blend zone
    bool enableTessellation{true};       ///< Enable GPU tessellation
    bool enableDisplacement{true};       ///< Enable displacement mapping
    bool enableTriplanar{false};         ///< Enable triplanar mapping
    bool castShadows{true};              ///< Cast shadows
    bool receiveShadows{true};           ///< Receive shadows
    
    /// Factory for low quality
    static TerrainSettings low() noexcept {
        TerrainSettings settings;
        settings.clipmapLevels = 4;
        settings.clipmapResolution = 127;
        settings.tessellationFactor = 16.0f;
        settings.enableTessellation = false;
        return settings;
    }
    
    /// Factory for medium quality
    static TerrainSettings medium() noexcept {
        TerrainSettings settings;
        settings.clipmapLevels = 6;
        settings.clipmapResolution = 255;
        settings.tessellationFactor = 32.0f;
        return settings;
    }
    
    /// Factory for high quality
    static TerrainSettings high() noexcept {
        TerrainSettings settings;
        settings.clipmapLevels = 8;
        settings.clipmapResolution = 511;
        settings.tessellationFactor = 64.0f;
        settings.enableTriplanar = true;
        return settings;
    }
    
    /// Factory for ultra quality
    static TerrainSettings ultra() noexcept {
        TerrainSettings settings;
        settings.clipmapLevels = 10;
        settings.clipmapResolution = 1023;
        settings.tessellationFactor = 128.0f;
        settings.enableTriplanar = true;
        return settings;
    }
};

// =============================================================================
// TERRAIN MANAGER
// =============================================================================

/**
 * @brief Terrain rendering statistics
 */
struct TerrainStats {
    u32 visibleChunks{0};
    u32 totalChunks{0};
    u32 streamingChunks{0};
    u32 triangleCount{0};
    u32 vertexCount{0};
    u32 textureLayerCount{0};
    u32 detailMeshInstances{0};
    f32 heightmapMemoryMB{0.0f};
    f32 streamingBudgetUsed{0.0f};
    
    void reset() noexcept {
        visibleChunks = 0;
        triangleCount = 0;
        vertexCount = 0;
        detailMeshInstances = 0;
    }
};

/**
 * @brief Terrain system manager
 * 
 * Manages terrain rendering, streaming, and editing
 */
class TerrainManager {
public:
    /// Singleton access
    static TerrainManager& instance() noexcept {
        static TerrainManager mgr;
        return mgr;
    }
    
    // -------------------------------------------------------------------------
    // INITIALIZATION
    // -------------------------------------------------------------------------
    
    /// Initialize terrain system
    bool initialize() noexcept {
        if (m_initialized) return true;
        
        // Initialize clipmap levels
        initializeClipmaps();
        
        m_initialized = true;
        return true;
    }
    
    /// Shutdown terrain system
    void shutdown() noexcept {
        m_heightmap = Heightmap{};
        m_textureLayers.clear();
        m_detailMeshes.clear();
        m_chunks.clear();
        m_clipmapLevels.clear();
        m_initialized = false;
    }
    
    /// Check if initialized
    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }
    
    // -------------------------------------------------------------------------
    // TERRAIN CREATION
    // -------------------------------------------------------------------------
    
    /// Create terrain from heightmap
    bool createTerrain(const Heightmap& heightmap, f32 worldSize, f32 heightScale) noexcept {
        if (!heightmap.isValid()) return false;
        
        m_heightmap = heightmap;
        m_worldSize = worldSize;
        m_heightScale = heightScale;
        
        // Calculate chunk grid
        u32 chunkSize = 64;
        m_chunkGridSize = static_cast<u32>(std::ceil(worldSize / chunkSize));
        
        // Create chunks
        m_chunks.clear();
        m_chunks.reserve(m_chunkGridSize * m_chunkGridSize);
        
        for (u32 z = 0; z < m_chunkGridSize; ++z) {
            for (u32 x = 0; x < m_chunkGridSize; ++x) {
                TerrainChunk chunk;
                chunk.id = z * m_chunkGridSize + x;
                chunk.gridX = static_cast<i32>(x);
                chunk.gridZ = static_cast<i32>(z);
                
                f32 worldX = (x + 0.5f) * chunkSize - worldSize * 0.5f;
                f32 worldZ = (z + 0.5f) * chunkSize - worldSize * 0.5f;
                chunk.worldPosition = Vec3{worldX, 0.0f, worldZ};
                
                chunk.boundsMin = Vec3{worldX - chunkSize * 0.5f, m_heightmap.minHeight * m_heightScale,
                                       worldZ - chunkSize * 0.5f};
                chunk.boundsMax = Vec3{worldX + chunkSize * 0.5f, m_heightmap.maxHeight * m_heightScale,
                                       worldZ + chunkSize * 0.5f};
                
                chunk.state = ChunkState::Loaded;
                m_chunks.push_back(chunk);
            }
        }
        
        m_stats.totalChunks = static_cast<u32>(m_chunks.size());
        m_stats.heightmapMemoryMB = static_cast<f32>(m_heightmap.getMemorySize()) / (1024.0f * 1024.0f);
        
        // Update clipmap scales
        updateClipmapScales();
        
        return true;
    }
    
    /// Create flat terrain
    bool createFlatTerrain(f32 worldSize, f32 height = 0.0f) noexcept {
        Heightmap flat = Heightmap::create(256, 256, 0.0f, 1.0f);
        std::fill(flat.data.begin(), flat.data.end(), height);
        return createTerrain(flat, worldSize, 1.0f);
    }
    
    /// Generate procedural terrain
    bool generateProceduralTerrain(u32 resolution, f32 worldSize, u32 octaves = 6, f32 persistence = 0.5f) noexcept {
        Heightmap hm = Heightmap::create(resolution, resolution, 0.0f, 1.0f);
        
        // Multi-octave noise generation
        for (u32 y = 0; y < resolution; ++y) {
            for (u32 x = 0; x < resolution; ++x) {
                f32 h = 0.0f;
                f32 amplitude = 1.0f;
                f32 frequency = 1.0f;
                f32 maxAmplitude = 0.0f;
                
                for (u32 o = 0; o < octaves; ++o) {
                    f32 nx = static_cast<f32>(x) / resolution * frequency * 4.0f;
                    f32 ny = static_cast<f32>(y) / resolution * frequency * 4.0f;
                    
                    // Simple hash-based noise
                    f32 noise = hashNoise(nx, ny);
                    h += noise * amplitude;
                    
                    maxAmplitude += amplitude;
                    amplitude *= persistence;
                    frequency *= 2.0f;
                }
                
                hm.data[y * resolution + x] = (h / maxAmplitude + 1.0f) * 0.5f;
            }
        }
        
        return createTerrain(hm, worldSize, 100.0f);
    }
    
    // -------------------------------------------------------------------------
    // HEIGHTMAP ACCESS
    // -------------------------------------------------------------------------
    
    /// Get height at world position
    [[nodiscard]] f32 getHeightAt(f32 worldX, f32 worldZ) const noexcept {
        if (!m_heightmap.isValid()) return 0.0f;
        
        f32 u = (worldX + m_worldSize * 0.5f) / m_worldSize;
        f32 v = (worldZ + m_worldSize * 0.5f) / m_worldSize;
        
        return m_heightmap.sample(u, v) * m_heightScale;
    }
    
    /// Get normal at world position
    [[nodiscard]] Vec3 getNormalAt(f32 worldX, f32 worldZ) const noexcept {
        if (!m_heightmap.isValid()) return Vec3{0.0f, 1.0f, 0.0f};
        
        f32 u = (worldX + m_worldSize * 0.5f) / m_worldSize;
        f32 v = (worldZ + m_worldSize * 0.5f) / m_worldSize;
        
        return m_heightmap.sampleNormal(u, v, m_heightScale / m_worldSize);
    }
    
    /// Get slope at world position (degrees)
    [[nodiscard]] f32 getSlopeAt(f32 worldX, f32 worldZ) const noexcept {
        Vec3 normal = getNormalAt(worldX, worldZ);
        f32 cosAngle = normal.dot(Vec3{0.0f, 1.0f, 0.0f});
        return std::acos(std::clamp(cosAngle, -1.0f, 1.0f)) * 57.2957795f;
    }
    
    /// Get heightmap
    [[nodiscard]] const Heightmap& getHeightmap() const noexcept {
        return m_heightmap;
    }
    
    // -------------------------------------------------------------------------
    // TEXTURE LAYERS
    // -------------------------------------------------------------------------
    
    /// Add texture layer
    u32 addTextureLayer(const TerrainTextureLayer& layer) noexcept {
        if (m_textureLayers.size() >= TerrainConfig::MAX_TEXTURE_LAYERS) {
            return UINT32_MAX;
        }
        
        u32 index = static_cast<u32>(m_textureLayers.size());
        m_textureLayers.push_back(layer);
        m_stats.textureLayerCount = static_cast<u32>(m_textureLayers.size());
        return index;
    }
    
    /// Get texture layer
    [[nodiscard]] const TerrainTextureLayer* getTextureLayer(u32 index) const noexcept {
        return index < m_textureLayers.size() ? &m_textureLayers[index] : nullptr;
    }
    
    /// Get texture layer count
    [[nodiscard]] u32 getTextureLayerCount() const noexcept {
        return static_cast<u32>(m_textureLayers.size());
    }
    
    /// Calculate texture blend weights at position
    [[nodiscard]] std::array<f32, TerrainConfig::MAX_TEXTURE_LAYERS> getBlendWeightsAt(f32 worldX, f32 worldZ) const noexcept {
        std::array<f32, TerrainConfig::MAX_TEXTURE_LAYERS> weights{};
        
        f32 h = getHeightAt(worldX, worldZ);
        f32 slope = getSlopeAt(worldX, worldZ);
        
        f32 totalWeight = 0.0f;
        for (usize i = 0; i < m_textureLayers.size() && i < TerrainConfig::MAX_TEXTURE_LAYERS; ++i) {
            weights[i] = m_textureLayers[i].calculateBlendWeight(h, slope);
            totalWeight += weights[i];
        }
        
        // Normalize
        if (totalWeight > 0.0f) {
            for (auto& w : weights) {
                w /= totalWeight;
            }
        }
        
        return weights;
    }
    
    // -------------------------------------------------------------------------
    // DETAIL MESHES
    // -------------------------------------------------------------------------
    
    /// Add detail mesh
    u32 addDetailMesh(const DetailMesh& mesh) noexcept {
        if (m_detailMeshes.size() >= TerrainConfig::MAX_DETAIL_MESHES) {
            return UINT32_MAX;
        }
        
        u32 index = static_cast<u32>(m_detailMeshes.size());
        m_detailMeshes.push_back(mesh);
        return index;
    }
    
    /// Get detail mesh
    [[nodiscard]] const DetailMesh* getDetailMesh(u32 index) const noexcept {
        return index < m_detailMeshes.size() ? &m_detailMeshes[index] : nullptr;
    }
    
    /// Get detail mesh count
    [[nodiscard]] u32 getDetailMeshCount() const noexcept {
        return static_cast<u32>(m_detailMeshes.size());
    }
    
    // -------------------------------------------------------------------------
    // SETTINGS
    // -------------------------------------------------------------------------
    
    /// Set terrain settings
    void setSettings(const TerrainSettings& settings) noexcept {
        m_settings = settings;
        initializeClipmaps();
        updateClipmapScales();
    }
    
    /// Get terrain settings
    [[nodiscard]] const TerrainSettings& getSettings() const noexcept {
        return m_settings;
    }
    
    /// Get world size
    [[nodiscard]] f32 getWorldSize() const noexcept {
        return m_worldSize;
    }
    
    /// Get height scale
    [[nodiscard]] f32 getHeightScale() const noexcept {
        return m_heightScale;
    }
    
    // -------------------------------------------------------------------------
    // CLIPMAPS
    // -------------------------------------------------------------------------
    
    /// Get clipmap level
    [[nodiscard]] const ClipmapLevel* getClipmapLevel(u32 level) const noexcept {
        return level < m_clipmapLevels.size() ? &m_clipmapLevels[level] : nullptr;
    }
    
    /// Get clipmap level count
    [[nodiscard]] u32 getClipmapLevelCount() const noexcept {
        return static_cast<u32>(m_clipmapLevels.size());
    }
    
    /// Update clipmaps for camera position
    void updateClipmaps(const Vec3& cameraPosition) noexcept {
        for (auto& level : m_clipmapLevels) {
            // Snap to grid
            f32 gridSize = level.cellSize * 2.0f;
            level.centerOffset.x = std::floor(cameraPosition.x / gridSize) * gridSize;
            level.centerOffset.y = std::floor(cameraPosition.z / gridSize) * gridSize;
            level.needsUpdate = true;
        }
    }
    
    // -------------------------------------------------------------------------
    // CHUNKS
    // -------------------------------------------------------------------------
    
    /// Get visible chunks for camera
    [[nodiscard]] std::vector<const TerrainChunk*> getVisibleChunks(const Vec3& cameraPosition, f32 viewDistance) const noexcept {
        std::vector<const TerrainChunk*> visible;
        visible.reserve(m_chunks.size());
        
        f32 viewDistSq = viewDistance * viewDistance;
        
        for (const auto& chunk : m_chunks) {
            Vec3 toChunk = chunk.getCenter() - cameraPosition;
            f32 distSq = toChunk.x * toChunk.x + toChunk.z * toChunk.z;
            
            if (distSq <= viewDistSq) {
                visible.push_back(&chunk);
            }
        }
        
        return visible;
    }
    
    /// Get chunk at world position
    [[nodiscard]] const TerrainChunk* getChunkAt(f32 worldX, f32 worldZ) const noexcept {
        for (const auto& chunk : m_chunks) {
            if (worldX >= chunk.boundsMin.x && worldX <= chunk.boundsMax.x &&
                worldZ >= chunk.boundsMin.z && worldZ <= chunk.boundsMax.z) {
                return &chunk;
            }
        }
        return nullptr;
    }
    
    // -------------------------------------------------------------------------
    // EDITING
    // -------------------------------------------------------------------------
    
    /// Apply brush to terrain
    void applyBrush(const TerrainBrush& brush, f32 worldX, f32 worldZ, f32 deltaTime) noexcept {
        if (!m_heightmap.isValid()) return;
        
        // Convert world to heightmap coordinates
        f32 u = (worldX + m_worldSize * 0.5f) / m_worldSize;
        f32 v = (worldZ + m_worldSize * 0.5f) / m_worldSize;
        
        f32 hmX = u * m_heightmap.width;
        f32 hmZ = v * m_heightmap.height;
        
        f32 brushRadiusHM = brush.radius / m_worldSize * m_heightmap.width;
        
        i32 minX = std::max(0, static_cast<i32>(hmX - brushRadiusHM));
        i32 maxX = std::min(static_cast<i32>(m_heightmap.width - 1), static_cast<i32>(hmX + brushRadiusHM));
        i32 minZ = std::max(0, static_cast<i32>(hmZ - brushRadiusHM));
        i32 maxZ = std::min(static_cast<i32>(m_heightmap.height - 1), static_cast<i32>(hmZ + brushRadiusHM));
        
        for (i32 z = minZ; z <= maxZ; ++z) {
            for (i32 x = minX; x <= maxX; ++x) {
                f32 dx = x - hmX;
                f32 dz = z - hmZ;
                f32 dist = std::sqrt(dx * dx + dz * dz) / brushRadiusHM * brush.radius;
                
                f32 falloff = brush.getFalloff(dist);
                if (falloff <= 0.0f) continue;
                
                u32 idx = z * m_heightmap.width + x;
                f32& h = m_heightmap.data[idx];
                
                switch (brush.type) {
                    case TerrainBrushType::Raise:
                        h += brush.strength * falloff * deltaTime;
                        break;
                    case TerrainBrushType::Lower:
                        h -= brush.strength * falloff * deltaTime;
                        break;
                    case TerrainBrushType::Flatten:
                        h = h + (brush.targetHeight / m_heightScale - h) * falloff * brush.strength * deltaTime;
                        break;
                    case TerrainBrushType::Smooth: {
                        f32 avg = 0.0f;
                        i32 count = 0;
                        for (i32 dz = -1; dz <= 1; ++dz) {
                            for (i32 dx = -1; dx <= 1; ++dx) {
                                i32 nx = x + dx;
                                i32 nz = z + dz;
                                if (nx >= 0 && nx < static_cast<i32>(m_heightmap.width) &&
                                    nz >= 0 && nz < static_cast<i32>(m_heightmap.height)) {
                                    avg += m_heightmap.data[nz * m_heightmap.width + nx];
                                    count++;
                                }
                            }
                        }
                        if (count > 0) {
                            h = h + (avg / count - h) * falloff * brush.strength * deltaTime;
                        }
                        break;
                    }
                    default:
                        break;
                }
                
                h = std::clamp(h, 0.0f, 1.0f);
            }
        }
    }
    
    // -------------------------------------------------------------------------
    // GPU DATA
    // -------------------------------------------------------------------------
    
    /// Get GPU terrain data
    [[nodiscard]] GPUTerrainData getGPUData() const noexcept {
        return GPUTerrainData::create(m_worldSize, m_heightScale,
                                      m_heightmap.width, m_heightmap.height);
    }
    
    // -------------------------------------------------------------------------
    // STATISTICS
    // -------------------------------------------------------------------------
    
    /// Get statistics
    [[nodiscard]] const TerrainStats& getStats() const noexcept {
        return m_stats;
    }
    
    /// Begin frame
    void beginFrame() noexcept {
        m_stats.reset();
    }
    
    /// End frame
    void endFrame() noexcept {
        // Calculate frame stats
        for (const auto& level : m_clipmapLevels) {
            m_stats.vertexCount += level.getVertexCount();
            m_stats.triangleCount += level.getIndexCount() / 3;
        }
    }

private:
    TerrainManager() = default;
    
    /// Initialize clipmap levels
    void initializeClipmaps() noexcept {
        m_clipmapLevels.clear();
        m_clipmapLevels.resize(m_settings.clipmapLevels);
        
        for (u32 i = 0; i < m_settings.clipmapLevels; ++i) {
            auto& level = m_clipmapLevels[i];
            level.level = i;
            level.resolution = m_settings.clipmapResolution;
            level.scale = std::pow(2.0f, static_cast<f32>(i));
        }
    }
    
    /// Update clipmap scales based on terrain size
    void updateClipmapScales() noexcept {
        if (m_worldSize <= 0.0f) return;
        
        f32 baseCellSize = m_worldSize / (m_settings.clipmapResolution * std::pow(2.0f, static_cast<f32>(m_settings.clipmapLevels - 1)));
        
        for (auto& level : m_clipmapLevels) {
            level.cellSize = baseCellSize * level.scale;
        }
    }
    
    /// Simple hash-based noise function
    [[nodiscard]] static f32 hashNoise(f32 x, f32 y) noexcept {
        i32 ix = static_cast<i32>(std::floor(x));
        i32 iy = static_cast<i32>(std::floor(y));
        f32 fx = x - ix;
        f32 fy = y - iy;
        
        auto hash = [](i32 x, i32 y) {
            i32 h = x * 374761393 + y * 668265263;
            h = (h ^ (h >> 13)) * 1274126177;
            return static_cast<f32>(h & 0x7FFFFFFF) / 0x7FFFFFFF;
        };
        
        f32 n00 = hash(ix, iy);
        f32 n10 = hash(ix + 1, iy);
        f32 n01 = hash(ix, iy + 1);
        f32 n11 = hash(ix + 1, iy + 1);
        
        // Smoothstep interpolation
        fx = fx * fx * (3.0f - 2.0f * fx);
        fy = fy * fy * (3.0f - 2.0f * fy);
        
        f32 n0 = n00 * (1.0f - fx) + n10 * fx;
        f32 n1 = n01 * (1.0f - fx) + n11 * fx;
        
        return (n0 * (1.0f - fy) + n1 * fy) * 2.0f - 1.0f;
    }
    
    bool m_initialized{false};
    
    Heightmap m_heightmap{};
    f32 m_worldSize{1000.0f};
    f32 m_heightScale{100.0f};
    u32 m_chunkGridSize{0};
    
    TerrainSettings m_settings{};
    std::vector<TerrainTextureLayer> m_textureLayers;
    std::vector<DetailMesh> m_detailMeshes;
    std::vector<TerrainChunk> m_chunks;
    std::vector<ClipmapLevel> m_clipmapLevels;
    
    TerrainStats m_stats{};
};

} // namespace nova
