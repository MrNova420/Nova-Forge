/**
 * @file test_resource.cpp
 * @brief Unit tests for NovaCore Resource System
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/resource/resource.hpp>

using namespace nova;
using namespace nova::resource;
using Catch::Approx;

// ============================================================================
// Resource Types Tests
// ============================================================================

TEST_CASE("Resource Types - ResourceType Enum", "[resource][types]") {
    SECTION("Basic resource types") {
        REQUIRE(static_cast<u16>(ResourceType::Unknown) == 0);
        REQUIRE(static_cast<u16>(ResourceType::Texture2D) == 100);
        REQUIRE(static_cast<u16>(ResourceType::Mesh) == 200);
        REQUIRE(static_cast<u16>(ResourceType::Material) == 300);
        REQUIRE(static_cast<u16>(ResourceType::AudioClip) == 400);
        REQUIRE(static_cast<u16>(ResourceType::Animation) == 500);
        REQUIRE(static_cast<u16>(ResourceType::Prefab) == 600);
    }
}

TEST_CASE("Resource Types - ResourceState Enum", "[resource][types]") {
    SECTION("All resource states are defined") {
        REQUIRE(static_cast<u8>(ResourceState::Unloaded) == 0);
        REQUIRE(static_cast<u8>(ResourceState::Queued) == 1);
        REQUIRE(static_cast<u8>(ResourceState::Loading) == 2);
        REQUIRE(static_cast<u8>(ResourceState::Processing) == 3);
        REQUIRE(static_cast<u8>(ResourceState::Loaded) == 4);
        REQUIRE(static_cast<u8>(ResourceState::Failed) == 5);
        REQUIRE(static_cast<u8>(ResourceState::Unloading) == 6);
    }
}

TEST_CASE("Resource Types - LoadPriority Enum", "[resource][types]") {
    SECTION("Priority levels") {
        REQUIRE(static_cast<u8>(LoadPriority::Background) == 0);
        REQUIRE(static_cast<u8>(LoadPriority::Low) == 64);
        REQUIRE(static_cast<u8>(LoadPriority::Normal) == 128);
        REQUIRE(static_cast<u8>(LoadPriority::High) == 192);
        REQUIRE(static_cast<u8>(LoadPriority::Immediate) == 255);
    }
}

TEST_CASE("Resource Types - ResourceConfig", "[resource][types]") {
    SECTION("Default configuration values") {
        REQUIRE(ResourceConfig::DEFAULT_CACHE_SIZE > 0);
        REQUIRE(ResourceConfig::MAX_CONCURRENT_LOADS == 8);
        REQUIRE(ResourceConfig::MAX_RESOURCE_NAME_LENGTH == 256);
        REQUIRE(ResourceConfig::DEFAULT_UNLOAD_DELAY == Approx(30.0f));
        REQUIRE(ResourceConfig::RESOURCE_POOL_INITIAL_SIZE == 1024);
    }
}

TEST_CASE("Resource Types - LoadFlags", "[resource][types]") {
    SECTION("Flag values") {
        REQUIRE(static_cast<u32>(LoadFlags::None) == 0);
        REQUIRE(static_cast<u32>(LoadFlags::Async) == 1);
        REQUIRE(static_cast<u32>(LoadFlags::Stream) == 2);
        REQUIRE(static_cast<u32>(LoadFlags::KeepSourceData) == 4);
        REQUIRE(static_cast<u32>(LoadFlags::NoCache) == 8);
        REQUIRE(static_cast<u32>(LoadFlags::Compressed) == 16);
    }
}

TEST_CASE("Resource Types - ResourceId", "[resource][types]") {
    SECTION("Default ResourceId") {
        ResourceId id;
        REQUIRE(id.value == 0);
    }
    
    SECTION("ResourceId with value") {
        ResourceId id{12345};
        REQUIRE(id.value == 12345);
    }
    
    SECTION("ResourceId comparison") {
        ResourceId id1{100};
        ResourceId id2{100};
        ResourceId id3{200};
        
        REQUIRE(id1 == id2);
        REQUIRE(id1 != id3);
    }
}

TEST_CASE("Resource Types - ResourcePath", "[resource][types]") {
    SECTION("Default ResourcePath") {
        ResourcePath path;
        REQUIRE(path.path.empty());
    }
    
    SECTION("ResourcePath from string") {
        ResourcePath path{"textures/player.png"};
        REQUIRE(path.path == "textures/player.png");
    }
}
