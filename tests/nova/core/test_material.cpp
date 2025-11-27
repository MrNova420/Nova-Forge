/**
 * @file test_material.cpp
 * @brief Comprehensive tests for the NovaCore PBR material system
 * 
 * Tests all material system functionality including:
 * - Material enums and configuration
 * - PBR parameters and presets
 * - Texture slot management
 * - Material feature flags
 * - Material factory methods
 * - Material instances
 * - Material library
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "nova/core/render/material.hpp"

using namespace nova;
using namespace nova::render;
using namespace nova::math;
using Catch::Approx;
using Catch::Matchers::WithinRel;

// ============================================================================
// Material Configuration Tests
// ============================================================================

// TEST_SUITE("MaterialConfig") {
    TEST_CASE("Configuration constants are valid") {
        REQUIRE(MaterialConfig::MAX_TEXTURES_PER_MATERIAL == 16);
        REQUIRE(MaterialConfig::MAX_MATERIAL_INSTANCES == 65536);
        REQUIRE(MaterialConfig::MAX_SHADER_PERMUTATIONS == 4096);
        REQUIRE(MaterialConfig::MAX_PARAMETER_BUFFER_SIZE == 256);
        REQUIRE(MaterialConfig::CACHE_VERSION == 1);
    }
    
    TEST_CASE("Default values are reasonable") {
        REQUIRE(MaterialConfig::DEFAULT_METALLIC == 0.0f);
        REQUIRE(MaterialConfig::DEFAULT_ROUGHNESS == 0.5f);
        REQUIRE(MaterialConfig::DEFAULT_AO == 1.0f);
        REQUIRE(MaterialConfig::DEFAULT_EMISSIVE_INTENSITY == 1.0f);
        REQUIRE(MaterialConfig::DEFAULT_NORMAL_STRENGTH == 1.0f);
        REQUIRE(MaterialConfig::DEFAULT_IOR == 1.5f);
    }
    
    TEST_CASE("Default albedo is white") {
        Vec4 albedo = MaterialConfig::DEFAULT_ALBEDO;
        REQUIRE(albedo.x == 1.0f);
        REQUIRE(albedo.y == 1.0f);
        REQUIRE(albedo.z == 1.0f);
        REQUIRE(albedo.w == 1.0f);
    }
// }

// ============================================================================
// Blend Mode Tests
// ============================================================================

// TEST_SUITE("BlendMode") {
    TEST_CASE("BlendMode enum values") {
        REQUIRE(static_cast<u8>(BlendMode::Opaque) == 0);
        REQUIRE(static_cast<u8>(BlendMode::Masked) == 1);
        REQUIRE(static_cast<u8>(BlendMode::Translucent) == 2);
        REQUIRE(static_cast<u8>(BlendMode::Additive) == 3);
        REQUIRE(static_cast<u8>(BlendMode::Modulate) == 4);
        REQUIRE(static_cast<u8>(BlendMode::PreMultiplied) == 5);
    }
    
    TEST_CASE("getBlendModeName returns correct names") {
        REQUIRE(std::string(getBlendModeName(BlendMode::Opaque)) == "Opaque");
        REQUIRE(std::string(getBlendModeName(BlendMode::Masked)) == "Masked");
        REQUIRE(std::string(getBlendModeName(BlendMode::Translucent)) == "Translucent");
        REQUIRE(std::string(getBlendModeName(BlendMode::Additive)) == "Additive");
        REQUIRE(std::string(getBlendModeName(BlendMode::Modulate)) == "Modulate");
        REQUIRE(std::string(getBlendModeName(BlendMode::PreMultiplied)) == "PreMultiplied");
    }
// }

// ============================================================================
// Shading Model Tests
// ============================================================================

// TEST_SUITE("ShadingModel") {
    TEST_CASE("ShadingModel enum values") {
        REQUIRE(static_cast<u8>(ShadingModel::Unlit) == 0);
        REQUIRE(static_cast<u8>(ShadingModel::DefaultLit) == 1);
        REQUIRE(static_cast<u8>(ShadingModel::Subsurface) == 2);
        REQUIRE(static_cast<u8>(ShadingModel::ClearCoat) == 3);
        REQUIRE(static_cast<u8>(ShadingModel::Hair) == 4);
        REQUIRE(static_cast<u8>(ShadingModel::Cloth) == 5);
        REQUIRE(static_cast<u8>(ShadingModel::Eye) == 6);
        REQUIRE(static_cast<u8>(ShadingModel::Foliage) == 7);
        REQUIRE(static_cast<u8>(ShadingModel::ThinTranslucent) == 8);
        REQUIRE(static_cast<u8>(ShadingModel::TwoSidedFoliage) == 9);
    }
    
    TEST_CASE("getShadingModelName returns correct names") {
        REQUIRE(std::string(getShadingModelName(ShadingModel::Unlit)) == "Unlit");
        REQUIRE(std::string(getShadingModelName(ShadingModel::DefaultLit)) == "DefaultLit");
        REQUIRE(std::string(getShadingModelName(ShadingModel::Subsurface)) == "Subsurface");
        REQUIRE(std::string(getShadingModelName(ShadingModel::ClearCoat)) == "ClearCoat");
        REQUIRE(std::string(getShadingModelName(ShadingModel::Hair)) == "Hair");
        REQUIRE(std::string(getShadingModelName(ShadingModel::Cloth)) == "Cloth");
        REQUIRE(std::string(getShadingModelName(ShadingModel::Eye)) == "Eye");
        REQUIRE(std::string(getShadingModelName(ShadingModel::Foliage)) == "Foliage");
        REQUIRE(std::string(getShadingModelName(ShadingModel::ThinTranslucent)) == "ThinTranslucent");
        REQUIRE(std::string(getShadingModelName(ShadingModel::TwoSidedFoliage)) == "TwoSidedFoliage");
    }
// }

// ============================================================================
// Texture Slot Tests
// ============================================================================

// TEST_SUITE("TextureSlot") {
    TEST_CASE("TextureSlot enum values") {
        REQUIRE(static_cast<u8>(TextureSlot::Albedo) == 0);
        REQUIRE(static_cast<u8>(TextureSlot::Normal) == 1);
        REQUIRE(static_cast<u8>(TextureSlot::MetallicRoughness) == 2);
        REQUIRE(static_cast<u8>(TextureSlot::AmbientOcclusion) == 3);
        REQUIRE(static_cast<u8>(TextureSlot::Emissive) == 4);
        REQUIRE(static_cast<u8>(TextureSlot::Height) == 5);
        REQUIRE(static_cast<u8>(TextureSlot::DetailNormal) == 6);
        REQUIRE(static_cast<u8>(TextureSlot::DetailAlbedo) == 7);
        REQUIRE(static_cast<u8>(TextureSlot::SubsurfaceColor) == 8);
        REQUIRE(static_cast<u8>(TextureSlot::ClearCoatNormal) == 9);
        REQUIRE(static_cast<u8>(TextureSlot::Anisotropy) == 10);
        REQUIRE(static_cast<u8>(TextureSlot::Sheen) == 11);
        REQUIRE(static_cast<u8>(TextureSlot::Transmission) == 12);
        REQUIRE(static_cast<u8>(TextureSlot::Thickness) == 13);
        REQUIRE(static_cast<u8>(TextureSlot::SpecularColor) == 14);
        REQUIRE(static_cast<u8>(TextureSlot::Custom0) == 15);
    }
    
    TEST_CASE("getTextureSlotName returns correct names") {
        REQUIRE(std::string(getTextureSlotName(TextureSlot::Albedo)) == "Albedo");
        REQUIRE(std::string(getTextureSlotName(TextureSlot::Normal)) == "Normal");
        REQUIRE(std::string(getTextureSlotName(TextureSlot::MetallicRoughness)) == "MetallicRoughness");
        REQUIRE(std::string(getTextureSlotName(TextureSlot::AmbientOcclusion)) == "AmbientOcclusion");
        REQUIRE(std::string(getTextureSlotName(TextureSlot::Emissive)) == "Emissive");
        REQUIRE(std::string(getTextureSlotName(TextureSlot::SubsurfaceColor)) == "SubsurfaceColor");
        REQUIRE(std::string(getTextureSlotName(TextureSlot::ClearCoatNormal)) == "ClearCoatNormal");
        REQUIRE(std::string(getTextureSlotName(TextureSlot::Custom0)) == "Custom0");
    }
// }

// ============================================================================
// Material Feature Tests
// ============================================================================

// TEST_SUITE("MaterialFeature") {
    TEST_CASE("MaterialFeature bit values") {
        REQUIRE(static_cast<u32>(MaterialFeature::None) == 0);
        REQUIRE(static_cast<u32>(MaterialFeature::HasAlbedoMap) == 1);
        REQUIRE(static_cast<u32>(MaterialFeature::HasNormalMap) == 2);
        REQUIRE(static_cast<u32>(MaterialFeature::HasMetallicRoughness) == 4);
        REQUIRE(static_cast<u32>(MaterialFeature::HasAOMap) == 8);
        REQUIRE(static_cast<u32>(MaterialFeature::HasEmissiveMap) == 16);
        REQUIRE(static_cast<u32>(MaterialFeature::DoubleSided) == 256);
        REQUIRE(static_cast<u32>(MaterialFeature::ReceiveShadows) == 512);
        REQUIRE(static_cast<u32>(MaterialFeature::CastShadows) == 1024);
    }
    
    TEST_CASE("Bitwise operators work correctly") {
        MaterialFeature combined = MaterialFeature::HasAlbedoMap | MaterialFeature::HasNormalMap;
        REQUIRE(static_cast<u32>(combined) == 3);
        
        MaterialFeature intersection = combined & MaterialFeature::HasAlbedoMap;
        REQUIRE(static_cast<u32>(intersection) == 1);
    }
    
    TEST_CASE("hasFeature helper works") {
        MaterialFeature features = MaterialFeature::HasAlbedoMap | MaterialFeature::CastShadows;
        
        REQUIRE(hasFeature(features, MaterialFeature::HasAlbedoMap));
        REQUIRE(hasFeature(features, MaterialFeature::CastShadows));
        REQUIRE_FALSE(hasFeature(features, MaterialFeature::HasNormalMap));
        REQUIRE_FALSE(hasFeature(features, MaterialFeature::DoubleSided));
    }
// }

// ============================================================================
// Material Workflow Tests
// ============================================================================

// TEST_SUITE("MaterialWorkflow") {
    TEST_CASE("MaterialWorkflow enum values") {
        REQUIRE(static_cast<u8>(MaterialWorkflow::MetallicRoughness) == 0);
        REQUIRE(static_cast<u8>(MaterialWorkflow::SpecularGlossiness) == 1);
    }
// }

// ============================================================================
// Material Texture Tests
// ============================================================================

// TEST_SUITE("MaterialTexture") {
    TEST_CASE("MaterialTexture - Default construction") {
        MaterialTexture tex;
        REQUIRE_FALSE(tex.isValid());
        REQUIRE(tex.bindlessIndex == 0);
        REQUIRE(tex.uvOffset.x == 0.0f);
        REQUIRE(tex.uvOffset.y == 0.0f);
        REQUIRE(tex.uvScale.x == 1.0f);
        REQUIRE(tex.uvScale.y == 1.0f);
        REQUIRE(tex.uvRotation == 0.0f);
        REQUIRE(tex.uvChannel == 0);
    }
    
    TEST_CASE("Texture with handle is valid") {
        MaterialTexture tex;
        tex.handle = TextureHandle(1);
        REQUIRE(tex.isValid());
    }
    
    TEST_CASE("UV matrix computation - identity") {
        MaterialTexture tex;
        auto matrix = tex.computeUVMatrix();
        
        // Identity matrix for 2D transform
        REQUIRE(matrix[0] == Approx(1.0f));
        REQUIRE(matrix[1] == Approx(0.0f));
        REQUIRE(matrix[2] == Approx(0.0f));
        REQUIRE(matrix[3] == Approx(0.0f));
        REQUIRE(matrix[4] == Approx(1.0f));
        REQUIRE(matrix[5] == Approx(0.0f));
    }
    
    TEST_CASE("UV matrix computation - with scale") {
        MaterialTexture tex;
        tex.uvScale = Vec2(2.0f, 3.0f);
        auto matrix = tex.computeUVMatrix();
        
        REQUIRE(matrix[0] == Approx(2.0f));
        REQUIRE(matrix[4] == Approx(3.0f));
    }
    
    TEST_CASE("UV matrix computation - with offset") {
        MaterialTexture tex;
        tex.uvOffset = Vec2(0.5f, 0.25f);
        auto matrix = tex.computeUVMatrix();
        
        REQUIRE(matrix[2] == Approx(0.5f));
        REQUIRE(matrix[5] == Approx(0.25f));
    }
// }

// ============================================================================
// PBR Parameters Tests
// ============================================================================

// TEST_SUITE("PBRParameters") {
    TEST_CASE("PBRParameters - Size is 128 bytes for GPU alignment") {
        REQUIRE(sizeof(PBRParameters) == 128);
    }
    
    TEST_CASE("PBRParameters - Default construction") {
        PBRParameters params;
        
        // Albedo
        REQUIRE(params.albedo.x == 1.0f);
        REQUIRE(params.albedo.y == 1.0f);
        REQUIRE(params.albedo.z == 1.0f);
        REQUIRE(params.albedo.w == 1.0f);
        
        // Base properties
        REQUIRE(params.metallic == 0.0f);
        REQUIRE(params.roughness == 0.5f);
        REQUIRE(params.ambientOcclusion == 1.0f);
        REQUIRE(params.normalStrength == 1.0f);
        
        // Advanced
        REQUIRE(params.ior == 1.5f);
        REQUIRE(params.transmission == 0.0f);
        REQUIRE(params.clearCoat == 0.0f);
        REQUIRE(params.anisotropy == 0.0f);
    }
    
    TEST_CASE("defaults() factory method") {
        auto params = PBRParameters::defaults();
        REQUIRE(params.metallic == 0.0f);
        REQUIRE(params.roughness == 0.5f);
    }
    
    TEST_CASE("metal() factory method") {
        auto params = PBRParameters::metal(Vec3(0.9f, 0.9f, 0.9f), 0.2f);
        REQUIRE(params.metallic == 1.0f);
        REQUIRE(params.roughness == 0.2f);
        REQUIRE(params.albedo.x == Approx(0.9f));
    }
    
    TEST_CASE("dielectric() factory method") {
        auto params = PBRParameters::dielectric(Vec3(0.8f, 0.2f, 0.2f), 0.6f);
        REQUIRE(params.metallic == 0.0f);
        REQUIRE(params.roughness == 0.6f);
        REQUIRE(params.albedo.x == Approx(0.8f));
    }
    
    TEST_CASE("glass() factory method") {
        auto params = PBRParameters::glass(1.45f);
        REQUIRE(params.metallic == 0.0f);
        REQUIRE(params.roughness == 0.0f);
        REQUIRE(params.ior == 1.45f);
        REQUIRE(params.transmission == 1.0f);
        REQUIRE(params.albedo.w == 0.0f); // Transparent
    }
    
    TEST_CASE("carPaint() factory method") {
        auto params = PBRParameters::carPaint(Vec3(0.8f, 0.0f, 0.0f), Vec3(1.0f));
        REQUIRE(params.metallic == 0.9f);
        REQUIRE(params.clearCoat == 1.0f);
        REQUIRE(params.clearCoatRoughness == Approx(0.03f));
    }
    
    TEST_CASE("skin() factory method") {
        auto params = PBRParameters::skin();
        REQUIRE(params.metallic == 0.0f);
        REQUIRE(params.subsurfaceIntensity == 0.5f);
        REQUIRE(params.subsurfaceRadius == 2.0f);
        REQUIRE(params.subsurfaceColor.x == 1.0f); // Red tint
    }
    
    TEST_CASE("cloth() factory method") {
        auto params = PBRParameters::cloth(Vec3(0.2f, 0.3f, 0.8f));
        REQUIRE(params.metallic == 0.0f);
        REQUIRE(params.roughness == 0.8f);
        REQUIRE(params.sheenIntensity == 0.5f);
        REQUIRE(params.sheenRoughness == 0.5f);
    }
// }

// ============================================================================
// Material Class Tests
// ============================================================================

// TEST_SUITE("Material") {
    TEST_CASE("Material - Default construction") {
        Material mat;
        REQUIRE(mat.getName().empty());
        REQUIRE(mat.getShadingModel() == ShadingModel::DefaultLit);
        REQUIRE(mat.getBlendMode() == BlendMode::Opaque);
        REQUIRE(mat.getWorkflow() == MaterialWorkflow::MetallicRoughness);
        REQUIRE(mat.isDirty());
    }
    
    TEST_CASE("Construction with name") {
        Material mat("TestMaterial");
        REQUIRE(mat.getName() == "TestMaterial");
    }
    
    TEST_CASE("Set and get name") {
        Material mat;
        mat.setName("NewName");
        REQUIRE(mat.getName() == "NewName");
    }
    
    TEST_CASE("Set and get shading model") {
        Material mat;
        mat.setShadingModel(ShadingModel::Subsurface);
        REQUIRE(mat.getShadingModel() == ShadingModel::Subsurface);
        REQUIRE(mat.isDirty());
    }
    
    TEST_CASE("Set and get blend mode") {
        Material mat;
        mat.setBlendMode(BlendMode::Translucent);
        REQUIRE(mat.getBlendMode() == BlendMode::Translucent);
    }
    
    TEST_CASE("Set and get workflow") {
        Material mat;
        mat.setWorkflow(MaterialWorkflow::SpecularGlossiness);
        REQUIRE(mat.getWorkflow() == MaterialWorkflow::SpecularGlossiness);
    }
    
    TEST_CASE("PBR parameters access") {
        Material mat;
        mat.setAlbedo(Vec4(1.0f, 0.0f, 0.0f, 1.0f));
        REQUIRE(mat.getParameters().albedo.x == 1.0f);
        REQUIRE(mat.getParameters().albedo.y == 0.0f);
        
        mat.setMetallic(0.8f);
        REQUIRE(mat.getParameters().metallic == 0.8f);
        
        mat.setRoughness(0.3f);
        REQUIRE(mat.getParameters().roughness == 0.3f);
        
        mat.setEmissive(Vec3(1.0f, 0.5f, 0.0f), 2.0f);
        REQUIRE(mat.getParameters().emissive.x == 1.0f);
        REQUIRE(mat.getParameters().emissive.w == 2.0f);
    }
    
    TEST_CASE("Texture management") {
        Material mat;
        
        // Initially no textures
        REQUIRE_FALSE(mat.hasTexture(TextureSlot::Albedo));
        
        // Set texture
        MaterialTexture tex;
        tex.handle = TextureHandle(123);
        mat.setTexture(TextureSlot::Albedo, tex);
        
        REQUIRE(mat.hasTexture(TextureSlot::Albedo));
        REQUIRE(mat.getTexture(TextureSlot::Albedo).handle.id() == 123);
        
        // Clear texture
        mat.clearTexture(TextureSlot::Albedo);
        REQUIRE_FALSE(mat.hasTexture(TextureSlot::Albedo));
    }
    
    TEST_CASE("Feature flags") {
        Material mat;
        
        REQUIRE_FALSE(mat.hasFeature(MaterialFeature::DoubleSided));
        
        mat.setFeature(MaterialFeature::DoubleSided, true);
        REQUIRE(mat.hasFeature(MaterialFeature::DoubleSided));
        REQUIRE(mat.isDoubleSided());
        
        mat.setFeature(MaterialFeature::DoubleSided, false);
        REQUIRE_FALSE(mat.isDoubleSided());
    }
    
    TEST_CASE("Shadow settings") {
        Material mat;
        
        mat.setCastsShadows(true);
        REQUIRE(mat.castsShadows());
        
        mat.setReceivesShadows(true);
        REQUIRE(mat.receivesShadows());
        
        mat.setCastsShadows(false);
        REQUIRE_FALSE(mat.castsShadows());
    }
    
    TEST_CASE("Alpha cutoff") {
        Material mat;
        REQUIRE(mat.getAlphaCutoff() == 0.5f); // Default
        
        mat.setAlphaCutoff(0.75f);
        REQUIRE(mat.getAlphaCutoff() == 0.75f);
    }
    
    TEST_CASE("Render queue") {
        Material mat;
        REQUIRE(mat.getRenderQueue() == 0); // Default
        
        mat.setRenderQueue(100);
        REQUIRE(mat.getRenderQueue() == 100);
    }
    
    TEST_CASE("Dirty flag management") {
        Material mat;
        REQUIRE(mat.isDirty());
        
        mat.markClean();
        REQUIRE_FALSE(mat.isDirty());
        
        mat.setAlbedo(Vec4(1.0f, 0.0f, 0.0f, 1.0f));
        REQUIRE(mat.isDirty());
    }
    
    TEST_CASE("Shader hash computation") {
        Material mat1;
        Material mat2;
        
        // Same materials should have same hash
        REQUIRE(mat1.computeShaderHash() == mat2.computeShaderHash());
        
        // Different shading model should have different hash
        mat2.setShadingModel(ShadingModel::Subsurface);
        REQUIRE(mat1.computeShaderHash() != mat2.computeShaderHash());
    }
    
    TEST_CASE("GPU data access") {
        Material mat;
        REQUIRE(mat.getGPUData() != nullptr);
        REQUIRE(mat.getGPUDataSize() == sizeof(PBRParameters));
        REQUIRE(mat.getGPUDataSize() == 128);
    }
    
    TEST_CASE("Bindless indices") {
        Material mat;
        MaterialTexture tex;
        tex.handle = TextureHandle(1);
        tex.bindlessIndex = 42;
        mat.setTexture(TextureSlot::Albedo, tex);
        
        auto indices = mat.getBindlessIndices();
        REQUIRE(indices[0] == 42); // Albedo slot
        REQUIRE(indices[1] == 0);  // Normal slot (empty)
    }
// }

// ============================================================================
// Material Factory Method Tests
// ============================================================================

// TEST_SUITE("Material Factory Methods") {
    TEST_CASE("createUnlit()") {
        auto mat = Material::createUnlit("UnlitTest", Vec4(1.0f, 0.0f, 0.0f, 1.0f));
        REQUIRE(mat.getName() == "UnlitTest");
        REQUIRE(mat.getShadingModel() == ShadingModel::Unlit);
        REQUIRE(mat.getParameters().albedo.x == 1.0f);
    }
    
    TEST_CASE("createPBR()") {
        auto mat = Material::createPBR("PBRTest");
        REQUIRE(mat.getName() == "PBRTest");
        REQUIRE(mat.getShadingModel() == ShadingModel::DefaultLit);
        REQUIRE(mat.getBlendMode() == BlendMode::Opaque);
        REQUIRE(mat.castsShadows());
        REQUIRE(mat.receivesShadows());
    }
    
    TEST_CASE("createMetal()") {
        auto mat = Material::createMetal("MetalTest", Vec3(0.9f, 0.9f, 0.9f), 0.2f);
        REQUIRE(mat.getName() == "MetalTest");
        REQUIRE(mat.getParameters().metallic == 1.0f);
        REQUIRE(mat.getParameters().roughness == 0.2f);
    }
    
    TEST_CASE("createGlass()") {
        auto mat = Material::createGlass("GlassTest", 1.45f);
        REQUIRE(mat.getName() == "GlassTest");
        REQUIRE(mat.getBlendMode() == BlendMode::Translucent);
        REQUIRE(mat.getParameters().transmission == 1.0f);
        REQUIRE(mat.hasFeature(MaterialFeature::UseRefraction));
        REQUIRE(mat.hasFeature(MaterialFeature::UseTransmission));
    }
    
    TEST_CASE("createSkin()") {
        auto mat = Material::createSkin("SkinTest");
        REQUIRE(mat.getName() == "SkinTest");
        REQUIRE(mat.getShadingModel() == ShadingModel::Subsurface);
        REQUIRE(mat.hasFeature(MaterialFeature::UseSubsurface));
    }
    
    TEST_CASE("createCloth()") {
        auto mat = Material::createCloth("ClothTest", Vec3(0.2f, 0.3f, 0.8f));
        REQUIRE(mat.getName() == "ClothTest");
        REQUIRE(mat.getShadingModel() == ShadingModel::Cloth);
        REQUIRE(mat.hasFeature(MaterialFeature::UseSheen));
        REQUIRE(mat.isDoubleSided());
    }
    
    TEST_CASE("createCarPaint()") {
        auto mat = Material::createCarPaint("CarPaintTest", Vec3(0.8f, 0.0f, 0.0f));
        REQUIRE(mat.getName() == "CarPaintTest");
        REQUIRE(mat.getShadingModel() == ShadingModel::ClearCoat);
        REQUIRE(mat.hasFeature(MaterialFeature::UseClearCoat));
    }
// }

// ============================================================================
// Material Instance Tests
// ============================================================================

// TEST_SUITE("MaterialInstance") {
    TEST_CASE("Construction with parent") {
        auto parent = std::make_shared<Material>("Parent");
        parent->setAlbedo(Vec4(1.0f, 0.0f, 0.0f, 1.0f));
        
        MaterialInstance instance(parent);
        
        REQUIRE(instance.getParent() == parent.get());
        REQUIRE(instance.getParameters().albedo.x == 1.0f);
    }
    
    TEST_CASE("Parameter override") {
        auto parent = std::make_shared<Material>("Parent");
        parent->setAlbedo(Vec4(1.0f, 0.0f, 0.0f, 1.0f));
        
        MaterialInstance instance(parent);
        instance.setAlbedo(Vec4(0.0f, 1.0f, 0.0f, 1.0f));
        
        // Instance has different color
        REQUIRE(instance.getParameters().albedo.y == 1.0f);
        
        // Parent unchanged
        REQUIRE(parent->getParameters().albedo.x == 1.0f);
    }
    
    TEST_CASE("Dirty flag") {
        auto parent = std::make_shared<Material>("Parent");
        MaterialInstance instance(parent);
        
        REQUIRE(instance.isDirty());
        
        instance.markClean();
        REQUIRE_FALSE(instance.isDirty());
        
        instance.setAlbedo(Vec4(1.0f, 0.0f, 0.0f, 1.0f));
        REQUIRE(instance.isDirty());
    }
    
    TEST_CASE("Reset to parent") {
        auto parent = std::make_shared<Material>("Parent");
        parent->setAlbedo(Vec4(1.0f, 0.0f, 0.0f, 1.0f));
        
        MaterialInstance instance(parent);
        instance.setAlbedo(Vec4(0.0f, 1.0f, 0.0f, 1.0f));
        
        REQUIRE(instance.getParameters().albedo.y == 1.0f);
        
        instance.resetToParent();
        REQUIRE(instance.getParameters().albedo.x == 1.0f);
        REQUIRE(instance.getParameters().albedo.y == 0.0f);
    }
// }

// ============================================================================
// Material Library Tests
// ============================================================================

// TEST_SUITE("MaterialLibrary") {
    TEST_CASE("Singleton access") {
        auto& lib1 = MaterialLibrary::instance();
        auto& lib2 = MaterialLibrary::instance();
        REQUIRE(&lib1 == &lib2);
    }
    
    TEST_CASE("Create and get material") {
        auto& lib = MaterialLibrary::instance();
        lib.clear();
        
        auto mat = lib.createMaterial("TestMat");
        REQUIRE(mat != nullptr);
        REQUIRE(mat->getName() == "TestMat");
        REQUIRE(mat->getId().value != 0);
        
        auto retrieved = lib.getMaterial("TestMat");
        REQUIRE(retrieved == mat);
    }
    
    TEST_CASE("Material existence check") {
        auto& lib = MaterialLibrary::instance();
        lib.clear();
        
        REQUIRE_FALSE(lib.hasMaterial("NonExistent"));
        
        lib.createMaterial("ExistingMat");
        REQUIRE(lib.hasMaterial("ExistingMat"));
    }
    
    TEST_CASE("Remove material") {
        auto& lib = MaterialLibrary::instance();
        lib.clear();
        
        lib.createMaterial("ToRemove");
        REQUIRE(lib.hasMaterial("ToRemove"));
        
        lib.removeMaterial("ToRemove");
        REQUIRE_FALSE(lib.hasMaterial("ToRemove"));
    }
    
    TEST_CASE("Get material names") {
        auto& lib = MaterialLibrary::instance();
        lib.clear();
        
        lib.createMaterial("Mat1");
        lib.createMaterial("Mat2");
        lib.createMaterial("Mat3");
        
        auto names = lib.getMaterialNames();
        REQUIRE(names.size() == 3);
    }
    
    TEST_CASE("Material count") {
        auto& lib = MaterialLibrary::instance();
        lib.clear();
        
        REQUIRE(lib.getMaterialCount() == 0);
        
        lib.createMaterial("Mat1");
        REQUIRE(lib.getMaterialCount() == 1);
        
        lib.createMaterial("Mat2");
        REQUIRE(lib.getMaterialCount() == 2);
    }
    
    TEST_CASE("Clear library") {
        auto& lib = MaterialLibrary::instance();
        lib.createMaterial("ToClear1");
        lib.createMaterial("ToClear2");
        
        lib.clear();
        REQUIRE(lib.getMaterialCount() == 0);
    }
    
    TEST_CASE("Create default materials") {
        auto& lib = MaterialLibrary::instance();
        lib.clear();
        
        lib.createDefaultMaterials();
        
        // Check standard materials exist
        REQUIRE(lib.hasMaterial("Default"));
        REQUIRE(lib.hasMaterial("Unlit"));
        REQUIRE(lib.hasMaterial("Wireframe"));
        REQUIRE(lib.hasMaterial("Error"));
        REQUIRE(lib.hasMaterial("Chrome"));
        REQUIRE(lib.hasMaterial("Gold"));
        REQUIRE(lib.hasMaterial("Copper"));
        REQUIRE(lib.hasMaterial("Rubber"));
        REQUIRE(lib.hasMaterial("Plastic"));
        REQUIRE(lib.hasMaterial("Glass"));
        
        // Verify material properties
        auto chrome = lib.getMaterial("Chrome");
        REQUIRE(chrome != nullptr);
        REQUIRE(chrome->getParameters().metallic == 1.0f);
        
        auto glass = lib.getMaterial("Glass");
        REQUIRE(glass != nullptr);
        REQUIRE(glass->getBlendMode() == BlendMode::Translucent);
    }
// }
