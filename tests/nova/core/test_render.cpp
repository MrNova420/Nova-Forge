/**
 * @file test_render.cpp
 * @brief Unit tests for NovaCore Render System Types
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/render/render_types.hpp>

using namespace nova;
using namespace nova::render;
using Catch::Approx;

// ============================================================================
// Graphics Backend Tests
// ============================================================================

TEST_CASE("Render Types - GraphicsBackend Enum", "[render][types]") {
    SECTION("All graphics backends are defined") {
        REQUIRE(static_cast<u8>(GraphicsBackend::None) == 0);
        REQUIRE(static_cast<u8>(GraphicsBackend::Vulkan) == 1);
        REQUIRE(static_cast<u8>(GraphicsBackend::Metal) == 2);
        REQUIRE(static_cast<u8>(GraphicsBackend::WebGPU) == 3);
        REQUIRE(static_cast<u8>(GraphicsBackend::D3D12) == 4);
        REQUIRE(static_cast<u8>(GraphicsBackend::OpenGLES) == 5);
        REQUIRE(static_cast<u8>(GraphicsBackend::OpenGL) == 6);
        REQUIRE(static_cast<u8>(GraphicsBackend::Software) == 7);
    }
    
    SECTION("Backend names are correct") {
        REQUIRE(std::string(getBackendName(GraphicsBackend::None)) == "None");
        REQUIRE(std::string(getBackendName(GraphicsBackend::Vulkan)) == "Vulkan");
        REQUIRE(std::string(getBackendName(GraphicsBackend::Metal)) == "Metal");
        REQUIRE(std::string(getBackendName(GraphicsBackend::WebGPU)) == "WebGPU");
        REQUIRE(std::string(getBackendName(GraphicsBackend::D3D12)) == "D3D12");
        REQUIRE(std::string(getBackendName(GraphicsBackend::OpenGLES)) == "OpenGL ES");
        REQUIRE(std::string(getBackendName(GraphicsBackend::OpenGL)) == "OpenGL");
        REQUIRE(std::string(getBackendName(GraphicsBackend::Software)) == "Software");
    }
}

TEST_CASE("Render Types - QualityTier Enum", "[render][types]") {
    SECTION("All quality tiers are defined") {
        REQUIRE(static_cast<u8>(QualityTier::Minimal) == 0);
        REQUIRE(static_cast<u8>(QualityTier::Basic) == 1);
        REQUIRE(static_cast<u8>(QualityTier::Standard) == 2);
        REQUIRE(static_cast<u8>(QualityTier::High) == 3);
        REQUIRE(static_cast<u8>(QualityTier::Ultra) == 4);
    }
    
    SECTION("Quality tier names are correct") {
        REQUIRE(std::string(getQualityTierName(QualityTier::Minimal)) == "Minimal");
        REQUIRE(std::string(getQualityTierName(QualityTier::Basic)) == "Basic");
        REQUIRE(std::string(getQualityTierName(QualityTier::Standard)) == "Standard");
        REQUIRE(std::string(getQualityTierName(QualityTier::High)) == "High");
        REQUIRE(std::string(getQualityTierName(QualityTier::Ultra)) == "Ultra");
    }
}

TEST_CASE("Render Types - TextureFormat Enum", "[render][types]") {
    SECTION("Basic texture formats") {
        REQUIRE(static_cast<u16>(TextureFormat::Unknown) == 0);
        REQUIRE(static_cast<u16>(TextureFormat::R8_UNORM) == 1);
        // Enum values depend on actual definition order
        REQUIRE(static_cast<u16>(TextureFormat::RGBA8_UNORM) > 0);
    }
    
    SECTION("Depth formats exist") {
        REQUIRE(static_cast<u16>(TextureFormat::D16_UNORM) > 0);
        REQUIRE(static_cast<u16>(TextureFormat::D24_UNORM_S8_UINT) > 0);
        REQUIRE(static_cast<u16>(TextureFormat::D32_FLOAT) > 0);
    }
}

TEST_CASE("Render Types - PrimitiveTopology Enum", "[render][types]") {
    SECTION("Primitive topologies") {
        REQUIRE(static_cast<u8>(PrimitiveTopology::PointList) == 0);
        REQUIRE(static_cast<u8>(PrimitiveTopology::LineList) == 1);
        REQUIRE(static_cast<u8>(PrimitiveTopology::LineStrip) == 2);
        REQUIRE(static_cast<u8>(PrimitiveTopology::TriangleList) == 3);
        REQUIRE(static_cast<u8>(PrimitiveTopology::TriangleStrip) == 4);
        REQUIRE(static_cast<u8>(PrimitiveTopology::TriangleFan) == 5);
    }
}

TEST_CASE("Render Types - CullMode Enum", "[render][types]") {
    SECTION("Cull modes") {
        REQUIRE(static_cast<u8>(CullMode::None) == 0);
        REQUIRE(static_cast<u8>(CullMode::Front) == 1);
        REQUIRE(static_cast<u8>(CullMode::Back) == 2);
    }
}

TEST_CASE("Render Types - BlendFactor Enum", "[render][types]") {
    SECTION("Blend factors") {
        REQUIRE(static_cast<u8>(BlendFactor::Zero) == 0);
        REQUIRE(static_cast<u8>(BlendFactor::One) == 1);
        REQUIRE(static_cast<u8>(BlendFactor::SrcColor) == 2);
        REQUIRE(static_cast<u8>(BlendFactor::OneMinusSrcColor) == 3);
        REQUIRE(static_cast<u8>(BlendFactor::DstColor) == 4);
        REQUIRE(static_cast<u8>(BlendFactor::OneMinusDstColor) == 5);
        REQUIRE(static_cast<u8>(BlendFactor::SrcAlpha) == 6);
        REQUIRE(static_cast<u8>(BlendFactor::OneMinusSrcAlpha) == 7);
        REQUIRE(static_cast<u8>(BlendFactor::DstAlpha) == 8);
        REQUIRE(static_cast<u8>(BlendFactor::OneMinusDstAlpha) == 9);
    }
}

TEST_CASE("Render Types - CompareOp Enum", "[render][types]") {
    SECTION("Comparison operations") {
        REQUIRE(static_cast<u8>(CompareOp::Never) == 0);
        REQUIRE(static_cast<u8>(CompareOp::Less) == 1);
        REQUIRE(static_cast<u8>(CompareOp::Equal) == 2);
        REQUIRE(static_cast<u8>(CompareOp::LessOrEqual) == 3);
        REQUIRE(static_cast<u8>(CompareOp::Greater) == 4);
        REQUIRE(static_cast<u8>(CompareOp::NotEqual) == 5);
        REQUIRE(static_cast<u8>(CompareOp::GreaterOrEqual) == 6);
        REQUIRE(static_cast<u8>(CompareOp::Always) == 7);
    }
}

TEST_CASE("Render Types - FilterMode Enum", "[render][types]") {
    SECTION("Filter modes") {
        REQUIRE(static_cast<u8>(FilterMode::Nearest) == 0);
        REQUIRE(static_cast<u8>(FilterMode::Linear) == 1);
    }
}

TEST_CASE("Render Types - AddressMode Enum", "[render][types]") {
    SECTION("Address modes") {
        REQUIRE(static_cast<u8>(AddressMode::Repeat) == 0);
        REQUIRE(static_cast<u8>(AddressMode::MirroredRepeat) == 1);
        REQUIRE(static_cast<u8>(AddressMode::ClampToEdge) == 2);
        REQUIRE(static_cast<u8>(AddressMode::ClampToBorder) == 3);
    }
}

TEST_CASE("Render Types - PolygonMode Enum", "[render][types]") {
    SECTION("Polygon modes") {
        REQUIRE(static_cast<u8>(PolygonMode::Fill) == 0);
        REQUIRE(static_cast<u8>(PolygonMode::Line) == 1);
        REQUIRE(static_cast<u8>(PolygonMode::Point) == 2);
    }
}

TEST_CASE("Render Types - FrontFace Enum", "[render][types]") {
    SECTION("Front face winding") {
        REQUIRE(static_cast<u8>(FrontFace::CounterClockwise) == 0);
        REQUIRE(static_cast<u8>(FrontFace::Clockwise) == 1);
    }
}

TEST_CASE("Render Types - getFormatBytesPerPixel", "[render][types]") {
    SECTION("Format byte sizes") {
        REQUIRE(getFormatBytesPerPixel(TextureFormat::R8_UNORM) == 1);
        REQUIRE(getFormatBytesPerPixel(TextureFormat::RG8_UNORM) == 2);
        REQUIRE(getFormatBytesPerPixel(TextureFormat::RGBA8_UNORM) == 4);
        REQUIRE(getFormatBytesPerPixel(TextureFormat::RGBA16_FLOAT) == 8);
        REQUIRE(getFormatBytesPerPixel(TextureFormat::RGBA32_FLOAT) == 16);
    }
}
