// =============================================================================
// NovaForge Platform - Decal System Tests
// =============================================================================
// Platform: NovaForge | Engine: NovaCore | Company: WeNova Interactive
// (operating as Kayden Shawn Massengill)
//
// Comprehensive tests for the decal system including:
// - Decal projections (box, sphere, cylinder)
// - Decal materials and channels
// - Decal transforms and fading
// - Decal manager operations
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <nova/core/render/decal_system.hpp>

using namespace nova;
using namespace nova::render;
using Catch::Approx;

// =============================================================================
// Decal Configuration Tests
// =============================================================================

TEST_CASE("DecalConfig - Constants are valid", "[render][decal]") {
    SECTION("Batch and limits") {
        REQUIRE(DecalConfig::MAX_DECALS_PER_BATCH > 0);
        REQUIRE(DecalConfig::MAX_TOTAL_DECALS > 0);
        REQUIRE(DecalConfig::DEFAULT_ATLAS_SIZE > 0);
    }
    
    SECTION("Size limits") {
        REQUIRE(DecalConfig::MIN_DECAL_SIZE > 0.0f);
        REQUIRE(DecalConfig::MAX_DECAL_SIZE > DecalConfig::MIN_DECAL_SIZE);
    }
    
    SECTION("Other constants") {
        REQUIRE(DecalConfig::DEFAULT_DEPTH_BIAS > 0.0f);
        REQUIRE(DecalConfig::MAX_DECALS_PER_CLUSTER > 0);
    }
}

// =============================================================================
// Decal Projection Tests
// =============================================================================

TEST_CASE("DecalProjection - Enum values and names", "[render][decal]") {
    SECTION("All projections are defined") {
        REQUIRE(static_cast<int>(DecalProjection::Box) == 0);
        REQUIRE(static_cast<int>(DecalProjection::Sphere) == 1);
        REQUIRE(static_cast<int>(DecalProjection::Cylinder) == 2);
        REQUIRE(static_cast<int>(DecalProjection::Planar) == 3);
        REQUIRE(static_cast<int>(DecalProjection::Triplanar) == 4);
    }
    
    SECTION("Projection names are correct") {
        REQUIRE(std::string(getProjectionName(DecalProjection::Box)) == "Box");
        REQUIRE(std::string(getProjectionName(DecalProjection::Sphere)) == "Sphere");
        REQUIRE(std::string(getProjectionName(DecalProjection::Cylinder)) == "Cylinder");
        REQUIRE(std::string(getProjectionName(DecalProjection::Planar)) == "Planar");
        REQUIRE(std::string(getProjectionName(DecalProjection::Triplanar)) == "Triplanar");
    }
}

// =============================================================================
// Decal Blend Mode Tests
// =============================================================================

TEST_CASE("DecalBlendMode - Enum values and names", "[render][decal]") {
    SECTION("All blend modes are defined") {
        REQUIRE(static_cast<int>(DecalBlendMode::Normal) == 0);
        REQUIRE(static_cast<int>(DecalBlendMode::Additive) == 1);
        REQUIRE(static_cast<int>(DecalBlendMode::Multiply) == 2);
        REQUIRE(static_cast<int>(DecalBlendMode::DBuffer) == 3);
        REQUIRE(static_cast<int>(DecalBlendMode::Stain) == 4);
    }
    
    SECTION("Blend mode names are correct") {
        REQUIRE(std::string(getBlendModeName(DecalBlendMode::Normal)) == "Normal");
        REQUIRE(std::string(getBlendModeName(DecalBlendMode::Additive)) == "Additive");
        REQUIRE(std::string(getBlendModeName(DecalBlendMode::Multiply)) == "Multiply");
        REQUIRE(std::string(getBlendModeName(DecalBlendMode::DBuffer)) == "DBuffer");
        REQUIRE(std::string(getBlendModeName(DecalBlendMode::Stain)) == "Stain");
    }
}

// =============================================================================
// Decal Queue Tests
// =============================================================================

TEST_CASE("DecalQueue - Enum values", "[render][decal]") {
    REQUIRE(static_cast<int>(DecalQueue::BeforeLighting) == 0);
    REQUIRE(static_cast<int>(DecalQueue::AfterLighting) == 1);
    REQUIRE(static_cast<int>(DecalQueue::AfterOpaques) == 2);
    REQUIRE(static_cast<int>(DecalQueue::BeforeTransparents) == 3);
    REQUIRE(static_cast<int>(DecalQueue::AfterAll) == 4);
}

// =============================================================================
// Decal Channels Tests
// =============================================================================

TEST_CASE("DecalChannels - Flags and operators", "[render][decal]") {
    SECTION("Flag values") {
        REQUIRE(static_cast<u8>(DecalChannels::None) == 0);
        REQUIRE(static_cast<u8>(DecalChannels::Albedo) == 1);
        REQUIRE(static_cast<u8>(DecalChannels::Normal) == 2);
        REQUIRE(static_cast<u8>(DecalChannels::Roughness) == 4);
        REQUIRE(static_cast<u8>(DecalChannels::Metallic) == 8);
        REQUIRE(static_cast<u8>(DecalChannels::Emissive) == 16);
        REQUIRE(static_cast<u8>(DecalChannels::AO) == 32);
        REQUIRE(static_cast<u8>(DecalChannels::All) == 0x3F);
    }
    
    SECTION("Channel operators") {
        DecalChannels combined = DecalChannels::Albedo | DecalChannels::Normal;
        REQUIRE(hasChannel(combined, DecalChannels::Albedo) == true);
        REQUIRE(hasChannel(combined, DecalChannels::Normal) == true);
        REQUIRE(hasChannel(combined, DecalChannels::Roughness) == false);
        
        DecalChannels masked = combined & DecalChannels::Albedo;
        REQUIRE(hasChannel(masked, DecalChannels::Albedo) == true);
        REQUIRE(hasChannel(masked, DecalChannels::Normal) == false);
    }
}

// =============================================================================
// Decal Material Tests
// =============================================================================

TEST_CASE("DecalMaterial - Properties", "[render][decal]") {
    SECTION("Default values") {
        DecalMaterial mat;
        REQUIRE(mat.albedoTexture == 0);
        REQUIRE(mat.opacity == Approx(1.0f));
        REQUIRE(mat.blendMode == DecalBlendMode::Normal);
        REQUIRE(mat.channels == DecalChannels::Albedo);
    }
    
    SECTION("Channel affect checks") {
        DecalMaterial mat;
        mat.channels = DecalChannels::Albedo | DecalChannels::Normal | DecalChannels::Roughness;
        
        REQUIRE(mat.affectsAlbedo() == true);
        REQUIRE(mat.affectsNormal() == true);
        REQUIRE(mat.affectsRoughness() == true);
        REQUIRE(mat.affectsMetallic() == false);
        REQUIRE(mat.affectsEmissive() == false);
    }
}

TEST_CASE("DecalMaterial - Presets", "[render][decal]") {
    SECTION("Basic material") {
        auto mat = DecalMaterial::basic(Color{1.0f, 0.0f, 0.0f, 1.0f});
        REQUIRE(mat.albedoColor.x == Approx(1.0f));
        REQUIRE(mat.albedoColor.y == Approx(0.0f));
        REQUIRE(mat.channels == DecalChannels::Albedo);
    }
    
    SECTION("Blood preset") {
        auto mat = DecalMaterial::blood();
        REQUIRE(mat.name == "Blood");
        REQUIRE(mat.affectsAlbedo() == true);
        REQUIRE(mat.roughness < 0.5f);
    }
    
    SECTION("Bullet hole preset") {
        auto mat = DecalMaterial::bulletHole();
        REQUIRE(mat.name == "Bullet Hole");
        REQUIRE(mat.affectsAlbedo() == true);
        REQUIRE(mat.affectsNormal() == true);
    }
    
    SECTION("Dirt preset") {
        auto mat = DecalMaterial::dirt();
        REQUIRE(mat.name == "Dirt");
        REQUIRE(mat.blendMode == DecalBlendMode::Stain);
    }
    
    SECTION("Emissive preset") {
        auto mat = DecalMaterial::emissive(Color{1.0f, 0.5f, 0.0f, 1.0f}, 10.0f);
        REQUIRE(mat.name == "Emissive");
        REQUIRE(mat.emissiveIntensity == Approx(10.0f));
        REQUIRE(mat.blendMode == DecalBlendMode::Additive);
    }
}

// =============================================================================
// Decal Transform Tests
// =============================================================================

TEST_CASE("DecalTransform - Properties", "[render][decal]") {
    SECTION("Default values") {
        DecalTransform t;
        REQUIRE(t.position.x == Approx(0.0f));
        REQUIRE(t.position.y == Approx(0.0f));
        REQUIRE(t.position.z == Approx(0.0f));
        REQUIRE(t.size.x == Approx(1.0f));
    }
    
    SECTION("Direction vectors") {
        DecalTransform t;
        Vec3 forward = t.getForward();
        Vec3 up = t.getUp();
        Vec3 right = t.getRight();
        
        REQUIRE(forward.z == Approx(1.0f));
        REQUIRE(up.y == Approx(1.0f));
        REQUIRE(right.x == Approx(1.0f));
    }
    
    SECTION("fromNormal creates correct transform") {
        Vec3 pos{5.0f, 3.0f, 2.0f};
        Vec3 normal{0.0f, 1.0f, 0.0f};
        
        auto t = DecalTransform::fromNormal(pos, normal, 2.0f);
        
        REQUIRE(t.position.x == Approx(5.0f));
        REQUIRE(t.position.y == Approx(3.0f));
        REQUIRE(t.position.z == Approx(2.0f));
        REQUIRE(t.size.x == Approx(2.0f));
        REQUIRE(t.size.y == Approx(2.0f));
    }
}

// =============================================================================
// Decal Fade Tests
// =============================================================================

TEST_CASE("DecalFade - Settings", "[render][decal]") {
    SECTION("Default values") {
        DecalFade fade;
        REQUIRE(fade.mode == DecalFadeMode::None);
        REQUIRE(fade.lifetime == Approx(0.0f));
    }
    
    SECTION("isExpired for permanent decal") {
        DecalFade fade;
        fade.lifetime = 0.0f;
        REQUIRE(fade.isExpired(1000.0f) == false);
    }
    
    SECTION("isExpired for temporary decal") {
        DecalFade fade;
        fade.lifetime = 5.0f;
        REQUIRE(fade.isExpired(4.0f) == false);
        REQUIRE(fade.isExpired(5.0f) == true);
        REQUIRE(fade.isExpired(6.0f) == true);
    }
    
    SECTION("calculateFade for no fade mode") {
        DecalFade fade;
        fade.mode = DecalFadeMode::None;
        REQUIRE(fade.calculateFade(5.0f, 100.0f, 45.0f) == Approx(1.0f));
    }
    
    SECTION("calculateFade for distance fade") {
        DecalFade fade;
        fade.mode = DecalFadeMode::Distance;
        fade.fadeStartDistance = 50.0f;
        fade.fadeEndDistance = 100.0f;
        
        REQUIRE(fade.calculateFade(0.0f, 25.0f, 0.0f) == Approx(1.0f));
        REQUIRE(fade.calculateFade(0.0f, 75.0f, 0.0f) == Approx(0.5f));
        REQUIRE(fade.calculateFade(0.0f, 100.0f, 0.0f) == Approx(0.0f));
    }
}

TEST_CASE("DecalFade - Factories", "[render][decal]") {
    SECTION("permanent factory") {
        auto fade = DecalFade::permanent();
        REQUIRE(fade.mode == DecalFadeMode::None);
    }
    
    SECTION("temporary factory") {
        auto fade = DecalFade::temporary(5.0f, 1.0f);
        REQUIRE(fade.mode == DecalFadeMode::Time);
        REQUIRE(fade.lifetime == Approx(5.0f));
        REQUIRE(fade.fadeOutTime == Approx(1.0f));
    }
    
    SECTION("distanceFade factory") {
        auto fade = DecalFade::distanceFade(20.0f, 50.0f);
        REQUIRE(fade.mode == DecalFadeMode::Distance);
        REQUIRE(fade.fadeStartDistance == Approx(20.0f));
        REQUIRE(fade.fadeEndDistance == Approx(50.0f));
    }
}

// =============================================================================
// Decal Tests
// =============================================================================

TEST_CASE("Decal - Properties", "[render][decal]") {
    SECTION("Default values") {
        Decal decal;
        REQUIRE(decal.enabled == true);
        REQUIRE(decal.visible == true);
        REQUIRE(decal.age == Approx(0.0f));
        REQUIRE(decal.opacity == Approx(1.0f));
        REQUIRE(decal.priority == 0);
    }
    
    SECTION("isExpired delegates to fade") {
        Decal decal;
        decal.fade.lifetime = 5.0f;
        decal.age = 3.0f;
        REQUIRE(decal.isExpired() == false);
        
        decal.age = 6.0f;
        REQUIRE(decal.isExpired() == true);
    }
    
    SECTION("getEffectiveOpacity combines factors") {
        Decal decal;
        decal.opacity = 0.8f;
        decal.colorTint = Color{1.0f, 1.0f, 1.0f, 0.5f};
        decal.currentFade = 0.5f;
        
        f32 effective = decal.getEffectiveOpacity();
        REQUIRE(effective == Approx(0.8f * 0.5f * 0.5f));
    }
    
    SECTION("getBoundingRadius calculation") {
        Decal decal;
        decal.transform.size = Vec3{1.0f, 2.0f, 3.0f};
        f32 radius = decal.getBoundingRadius();
        REQUIRE(radius > 0.0f);
    }
    
    SECTION("update advances age") {
        Decal decal;
        decal.age = 0.0f;
        decal.update(1.0f, Vec3{0.0f, 0.0f, 0.0f});
        REQUIRE(decal.age == Approx(1.0f));
    }
}

// =============================================================================
// GPU Decal Data Tests
// =============================================================================

TEST_CASE("GPU Decal Data - Alignment and operations", "[render][decal]") {
    SECTION("GPUDecalData alignment") {
        // GPUDecalData may be 16 or 64 depending on Mat4 alignment
        REQUIRE(alignof(GPUDecalData) >= 16);
    }
    
    SECTION("GPUDecalBatch operations") {
        GPUDecalBatch batch;
        REQUIRE(batch.count() == 0);
        REQUIRE(batch.canAdd() == true);
        
        for (u32 i = 0; i < DecalConfig::MAX_DECALS_PER_BATCH; ++i) {
            batch.decals.push_back(GPUDecalData{});
        }
        
        REQUIRE(batch.count() == DecalConfig::MAX_DECALS_PER_BATCH);
        REQUIRE(batch.canAdd() == false);
    }
}

// =============================================================================
// Decal Spawn Settings Tests
// =============================================================================

TEST_CASE("DecalSpawnSettings - Spawning", "[render][decal]") {
    SECTION("Default values") {
        DecalSpawnSettings settings;
        REQUIRE(settings.randomRotation == true);
        REQUIRE(settings.sizeRange.x > 0.0f);
        REQUIRE(settings.sizeRange.y >= settings.sizeRange.x);
    }
    
    SECTION("spawn creates decal at position") {
        DecalSpawnSettings settings;
        settings.sizeRange = Vec2{1.0f, 1.0f};
        settings.randomRotation = false;
        
        Vec3 pos{5.0f, 3.0f, 2.0f};
        Vec3 normal{0.0f, 1.0f, 0.0f};
        
        Decal decal = settings.spawn(pos, normal, 0);
        
        REQUIRE(decal.transform.position.x == Approx(5.0f));
        REQUIRE(decal.transform.position.y == Approx(3.0f));
        REQUIRE(decal.transform.position.z == Approx(2.0f));
    }
}

// =============================================================================
// Decal Manager Tests
// =============================================================================

TEST_CASE("DecalManager - Singleton", "[render][decal]") {
    SECTION("Singleton access returns same instance") {
        auto& manager1 = DecalManager::getInstance();
        auto& manager2 = DecalManager::getInstance();
        REQUIRE(&manager1 == &manager2);
    }
    
    SECTION("Initialize and shutdown") {
        auto& manager = DecalManager::getInstance();
        manager.initialize();
        REQUIRE(manager.isInitialized() == true);
        
        manager.shutdown();
        REQUIRE(manager.isInitialized() == false);
    }
}

TEST_CASE("DecalManager - Operations", "[render][decal]") {
    auto& manager = DecalManager::getInstance();
    manager.initialize();
    manager.clearAll();
    
    SECTION("Add and remove decal") {
        Decal decal;
        decal.transform.position = Vec3{1.0f, 2.0f, 3.0f};
        
        DecalHandle handle = manager.addDecal(decal);
        REQUIRE(handle.isValid());
        REQUIRE(manager.getDecals().size() == 1);
        
        bool removed = manager.removeDecal(handle);
        REQUIRE(removed == true);
        REQUIRE(manager.getDecals().empty());
    }
    
    SECTION("Register and get material") {
        DecalMaterial mat = DecalMaterial::bulletHole();
        DecalMaterialHandle handle = manager.registerMaterial(mat);
        
        const DecalMaterial* retrieved = manager.getMaterial(handle);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->name == "Bullet Hole");
    }
    
    SECTION("clearAll removes all decals") {
        manager.addDecal(Decal{});
        manager.addDecal(Decal{});
        manager.addDecal(Decal{});
        
        manager.clearAll();
        REQUIRE(manager.getDecals().empty());
    }
    
    manager.shutdown();
}

// =============================================================================
// Utility Function Tests
// =============================================================================

TEST_CASE("Decal Utilities - Projection helpers", "[render][decal]") {
    SECTION("isInsideDecal for box projection") {
        DecalTransform t;
        t.position = Vec3{0.0f, 0.0f, 0.0f};
        t.size = Vec3{1.0f, 1.0f, 1.0f};
        
        REQUIRE(isInsideDecal(Vec3{0.0f, 0.0f, 0.0f}, t, DecalProjection::Box) == true);
        REQUIRE(isInsideDecal(Vec3{0.5f, 0.5f, 0.5f}, t, DecalProjection::Box) == true);
        REQUIRE(isInsideDecal(Vec3{2.0f, 0.0f, 0.0f}, t, DecalProjection::Box) == false);
    }
    
    SECTION("isInsideDecal for sphere projection") {
        DecalTransform t;
        t.position = Vec3{0.0f, 0.0f, 0.0f};
        t.size = Vec3{1.0f, 1.0f, 1.0f};
        
        REQUIRE(isInsideDecal(Vec3{0.0f, 0.0f, 0.0f}, t, DecalProjection::Sphere) == true);
        REQUIRE(isInsideDecal(Vec3{0.5f, 0.0f, 0.0f}, t, DecalProjection::Sphere) == true);
        REQUIRE(isInsideDecal(Vec3{2.0f, 0.0f, 0.0f}, t, DecalProjection::Sphere) == false);
    }
}
