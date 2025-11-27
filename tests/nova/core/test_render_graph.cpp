/**
 * @file test_render_graph.cpp
 * @brief Comprehensive tests for the NovaCore render graph system
 * 
 * Tests all render graph functionality including:
 * - Resource creation and management
 * - Pass creation and dependencies
 * - Graph compilation and execution
 * - Pass culling
 * - Resource lifetime tracking
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "nova/core/render/render_graph.hpp"

using namespace nova;
using namespace nova::render;
using Catch::Approx;

// ============================================================================
// Render Graph Configuration Tests
// ============================================================================

TEST_CASE("RenderGraphConfig constants are valid", "[render_graph][config]") {
    REQUIRE(RenderGraphConfig::MAX_PASSES == 256);
    REQUIRE(RenderGraphConfig::MAX_RESOURCES == 512);
    REQUIRE(RenderGraphConfig::MAX_DEPENDENCIES == 64);
    REQUIRE(RenderGraphConfig::MAX_RENDER_TARGETS == 8);
    REQUIRE(RenderGraphConfig::TRANSIENT_BUFFER_POOL_SIZE_MB == 256);
    REQUIRE(RenderGraphConfig::TRANSIENT_TEXTURE_POOL_SIZE_MB == 512);
    REQUIRE(RenderGraphConfig::AUTO_BARRIERS == true);
    REQUIRE(RenderGraphConfig::PASS_CULLING == true);
    REQUIRE(RenderGraphConfig::RESOURCE_ALIASING == true);
}

// ============================================================================
// Resource Type Tests
// ============================================================================

TEST_CASE("ResourceType enum values", "[render_graph][resource]") {
    REQUIRE(static_cast<u8>(ResourceType::Buffer) == 0);
    REQUIRE(static_cast<u8>(ResourceType::Texture) == 1);
    REQUIRE(static_cast<u8>(ResourceType::Imported) == 2);
}

TEST_CASE("ResourceAccess flags", "[render_graph][resource]") {
    REQUIRE(static_cast<u16>(ResourceAccess::None) == 0);
    REQUIRE(static_cast<u16>(ResourceAccess::Read) == 1);
    REQUIRE(static_cast<u16>(ResourceAccess::Write) == 2);
    REQUIRE(static_cast<u16>(ResourceAccess::ReadWrite) == 3);
    
    // Test combining flags
    ResourceAccess combined = ResourceAccess::Read | ResourceAccess::VertexShader;
    REQUIRE(hasAccess(combined, ResourceAccess::Read));
    REQUIRE(hasAccess(combined, ResourceAccess::VertexShader));
    REQUIRE_FALSE(hasAccess(combined, ResourceAccess::Write));
}

TEST_CASE("ResourceAccess bitwise operators", "[render_graph][resource]") {
    ResourceAccess a = ResourceAccess::Read;
    ResourceAccess b = ResourceAccess::Write;
    
    ResourceAccess combined = a | b;
    REQUIRE(static_cast<u16>(combined) == 3);
    
    ResourceAccess intersection = combined & ResourceAccess::Read;
    REQUIRE(static_cast<u16>(intersection) == 1);
    
    ResourceAccess acc = ResourceAccess::None;
    acc |= ResourceAccess::FragmentShader;
    REQUIRE(hasAccess(acc, ResourceAccess::FragmentShader));
}

// ============================================================================
// Resource Handle Tests
// ============================================================================

TEST_CASE("RGResourceHandle default construction", "[render_graph][handle]") {
    RGResourceHandle handle;
    REQUIRE_FALSE(handle.isValid());
    REQUIRE(handle.index == ~0u);
    REQUIRE(handle.version == 0);
}

TEST_CASE("RGResourceHandle invalid factory", "[render_graph][handle]") {
    auto handle = RGResourceHandle::invalid();
    REQUIRE_FALSE(handle.isValid());
}

TEST_CASE("RGResourceHandle equality", "[render_graph][handle]") {
    RGResourceHandle a;
    a.index = 5;
    a.version = 1;
    
    RGResourceHandle b;
    b.index = 5;
    b.version = 1;
    
    RGResourceHandle c;
    c.index = 5;
    c.version = 2;
    
    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("RGTextureHandle inherits from RGResourceHandle", "[render_graph][handle]") {
    RGTextureHandle tex;
    REQUIRE_FALSE(tex.isValid());
    
    auto invalid = RGTextureHandle::invalid();
    REQUIRE_FALSE(invalid.isValid());
}

TEST_CASE("RGBufferHandle inherits from RGResourceHandle", "[render_graph][handle]") {
    RGBufferHandle buf;
    REQUIRE_FALSE(buf.isValid());
    
    auto invalid = RGBufferHandle::invalid();
    REQUIRE_FALSE(invalid.isValid());
}

// ============================================================================
// Texture Description Tests
// ============================================================================

TEST_CASE("RGTextureDesc default construction", "[render_graph][desc]") {
    RGTextureDesc desc;
    REQUIRE(desc.width == 0);
    REQUIRE(desc.height == 0);
    REQUIRE(desc.depth == 1);
    REQUIRE(desc.mipLevels == 1);
    REQUIRE(desc.arrayLayers == 1);
    REQUIRE(desc.format == TextureFormat::RGBA8_UNORM);
    REQUIRE_FALSE(desc.isRenderTarget);
    REQUIRE_FALSE(desc.isDepthStencil);
}

TEST_CASE("RGTextureDesc::renderTarget factory", "[render_graph][desc]") {
    auto desc = RGTextureDesc::renderTarget("Test", 1920, 1080);
    
    REQUIRE(desc.name == "Test");
    REQUIRE(desc.width == 1920);
    REQUIRE(desc.height == 1080);
    REQUIRE(desc.format == TextureFormat::RGBA8_UNORM);
    REQUIRE(desc.isRenderTarget);
    REQUIRE_FALSE(desc.isDepthStencil);
}

TEST_CASE("RGTextureDesc::depthBuffer factory", "[render_graph][desc]") {
    auto desc = RGTextureDesc::depthBuffer("Depth", 1920, 1080);
    
    REQUIRE(desc.name == "Depth");
    REQUIRE(desc.width == 1920);
    REQUIRE(desc.height == 1080);
    REQUIRE(desc.format == TextureFormat::D32_FLOAT);
    REQUIRE(desc.isRenderTarget);
    REQUIRE(desc.isDepthStencil);
}

TEST_CASE("RGTextureDesc::gBuffer factory", "[render_graph][desc]") {
    auto desc = RGTextureDesc::gBuffer("GBuffer", 1920, 1080, TextureFormat::RGBA16_FLOAT);
    
    REQUIRE(desc.name == "GBuffer");
    REQUIRE(desc.width == 1920);
    REQUIRE(desc.height == 1080);
    REQUIRE(desc.format == TextureFormat::RGBA16_FLOAT);
    REQUIRE(desc.isRenderTarget);
}

TEST_CASE("RGTextureDesc::computeSizeBytes", "[render_graph][desc]") {
    RGTextureDesc desc;
    desc.width = 1024;
    desc.height = 1024;
    desc.format = TextureFormat::RGBA8_UNORM;
    
    // 1024 * 1024 * 4 bytes = 4 MB
    REQUIRE(desc.computeSizeBytes() == 4 * 1024 * 1024);
}

// ============================================================================
// Buffer Description Tests
// ============================================================================

TEST_CASE("RGBufferDesc default construction", "[render_graph][desc]") {
    RGBufferDesc desc;
    REQUIRE(desc.size == 0);
    REQUIRE(desc.usage == BufferUsage::Storage);
    REQUIRE_FALSE(desc.cpuReadable);
}

TEST_CASE("RGBufferDesc::uniform factory", "[render_graph][desc]") {
    auto desc = RGBufferDesc::uniform("UBO", 256);
    
    REQUIRE(desc.name == "UBO");
    REQUIRE(desc.size == 256);
    REQUIRE(desc.usage == BufferUsage::Uniform);
}

TEST_CASE("RGBufferDesc::storage factory", "[render_graph][desc]") {
    auto desc = RGBufferDesc::storage("SSBO", 1024);
    
    REQUIRE(desc.name == "SSBO");
    REQUIRE(desc.size == 1024);
    REQUIRE(desc.usage == BufferUsage::Storage);
}

TEST_CASE("RGBufferDesc::indirect factory", "[render_graph][desc]") {
    auto desc = RGBufferDesc::indirect("Indirect", 512);
    
    REQUIRE(desc.name == "Indirect");
    REQUIRE(desc.size == 512);
    REQUIRE(desc.usage == BufferUsage::IndirectBuffer);
}

// ============================================================================
// Pass Type Tests
// ============================================================================

TEST_CASE("RGPassType enum values", "[render_graph][pass]") {
    REQUIRE(static_cast<u8>(RGPassType::Graphics) == 0);
    REQUIRE(static_cast<u8>(RGPassType::Compute) == 1);
    REQUIRE(static_cast<u8>(RGPassType::Transfer) == 2);
    REQUIRE(static_cast<u8>(RGPassType::Present) == 3);
    REQUIRE(static_cast<u8>(RGPassType::AsyncCompute) == 4);
    REQUIRE(static_cast<u8>(RGPassType::RayTracing) == 5);
}

TEST_CASE("getPassTypeName returns correct names", "[render_graph][pass]") {
    REQUIRE(std::string(getPassTypeName(RGPassType::Graphics)) == "Graphics");
    REQUIRE(std::string(getPassTypeName(RGPassType::Compute)) == "Compute");
    REQUIRE(std::string(getPassTypeName(RGPassType::Transfer)) == "Transfer");
    REQUIRE(std::string(getPassTypeName(RGPassType::Present)) == "Present");
    REQUIRE(std::string(getPassTypeName(RGPassType::AsyncCompute)) == "AsyncCompute");
    REQUIRE(std::string(getPassTypeName(RGPassType::RayTracing)) == "RayTracing");
}

TEST_CASE("RGPassFlags values", "[render_graph][pass]") {
    REQUIRE(static_cast<u8>(RGPassFlags::None) == 0);
    REQUIRE(static_cast<u8>(RGPassFlags::NoCulling) == 1);
    REQUIRE(static_cast<u8>(RGPassFlags::AsyncCapable) == 2);
    REQUIRE(static_cast<u8>(RGPassFlags::ForceSerial) == 4);
    REQUIRE(static_cast<u8>(RGPassFlags::SideEffects) == 8);
}

TEST_CASE("RGPassFlags bitwise operators", "[render_graph][pass]") {
    RGPassFlags flags = RGPassFlags::NoCulling | RGPassFlags::SideEffects;
    REQUIRE(hasFlag(flags, RGPassFlags::NoCulling));
    REQUIRE(hasFlag(flags, RGPassFlags::SideEffects));
    REQUIRE_FALSE(hasFlag(flags, RGPassFlags::AsyncCapable));
}

// ============================================================================
// Pass Data Tests
// ============================================================================

TEST_CASE("RGPassData default construction", "[render_graph][pass]") {
    RGPassData pass;
    REQUIRE(pass.name.empty());
    REQUIRE(pass.type == RGPassType::Graphics);
    REQUIRE(pass.flags == RGPassFlags::None);
    REQUIRE(pass.reads.empty());
    REQUIRE(pass.writes.empty());
    REQUIRE(pass.colorTargets.empty());
    REQUIRE_FALSE(pass.depthTarget.isValid());
    REQUIRE(pass.executionOrder == -1);
    REQUIRE_FALSE(pass.culled);
}

TEST_CASE("RGPassData helper methods", "[render_graph][pass]") {
    RGPassData pass;
    
    REQUIRE_FALSE(pass.hasColorTargets());
    REQUIRE_FALSE(pass.hasDepthTarget());
    REQUIRE(pass.isGraphics());
    REQUIRE_FALSE(pass.isCompute());
    
    pass.type = RGPassType::Compute;
    REQUIRE_FALSE(pass.isGraphics());
    REQUIRE(pass.isCompute());
    
    RGTextureHandle tex;
    tex.index = 1;
    tex.version = 1;
    pass.colorTargets.push_back(tex);
    REQUIRE(pass.hasColorTargets());
    
    pass.depthTarget = tex;
    REQUIRE(pass.hasDepthTarget());
}

// ============================================================================
// Render Graph Tests
// ============================================================================

TEST_CASE("RenderGraph construction", "[render_graph][graph]") {
    RenderGraph graph;
    REQUIRE(graph.getPassCount() == 0);
    REQUIRE(graph.getResourceCount() == 0);
    REQUIRE_FALSE(graph.isCompiled());
}

TEST_CASE("RenderGraph createTexture", "[render_graph][graph]") {
    RenderGraph graph;
    
    auto desc = RGTextureDesc::renderTarget("Test", 1920, 1080);
    auto handle = graph.createTexture(desc);
    
    REQUIRE(handle.isValid());
    REQUIRE(graph.getResourceCount() == 1);
    
    const auto& res = graph.getResource(handle);
    REQUIRE(res.name == "Test");
    REQUIRE(res.isTexture());
    REQUIRE(res.isTransient);
    REQUIRE_FALSE(res.isImported);
}

TEST_CASE("RenderGraph createBuffer", "[render_graph][graph]") {
    RenderGraph graph;
    
    auto desc = RGBufferDesc::uniform("UBO", 256);
    auto handle = graph.createBuffer(desc);
    
    REQUIRE(handle.isValid());
    REQUIRE(graph.getResourceCount() == 1);
    
    const auto& res = graph.getResource(handle);
    REQUIRE(res.name == "UBO");
    REQUIRE(res.isBuffer());
    REQUIRE(res.isTransient);
}

TEST_CASE("RenderGraph importTexture", "[render_graph][graph]") {
    RenderGraph graph;
    
    TextureHandle external(123);
    auto desc = RGTextureDesc::renderTarget("External", 1920, 1080);
    auto handle = graph.importTexture("External", external, desc);
    
    REQUIRE(handle.isValid());
    const auto& res = graph.getResource(handle);
    REQUIRE(res.isImported);
    REQUIRE_FALSE(res.isTransient);
}

TEST_CASE("RenderGraph importBackBuffer", "[render_graph][graph]") {
    RenderGraph graph;
    
    TextureHandle backBuffer(1);
    auto handle = graph.importBackBuffer(backBuffer, 1920, 1080);
    
    REQUIRE(handle.isValid());
    REQUIRE(graph.getBackBuffer() == handle);
}

TEST_CASE("RenderGraph addGraphicsPass", "[render_graph][graph]") {
    RenderGraph graph;
    
    bool setupCalled = false;
    bool executeCalled = false;
    
    graph.addGraphicsPass("TestPass",
        [&](RenderGraphBuilder& builder) {
            setupCalled = true;
        },
        [&](RenderGraphContext& ctx) {
            executeCalled = true;
        });
    
    REQUIRE(setupCalled);
    REQUIRE(graph.getPassCount() == 1);
    
    const auto& pass = graph.getPass(0);
    REQUIRE(pass.name == "TestPass");
    REQUIRE(pass.type == RGPassType::Graphics);
}

TEST_CASE("RenderGraph addComputePass", "[render_graph][graph]") {
    RenderGraph graph;
    
    graph.addComputePass("ComputePass",
        [](RenderGraphBuilder& builder) {},
        [](RenderGraphContext& ctx) {});
    
    REQUIRE(graph.getPassCount() == 1);
    REQUIRE(graph.getPass(0).type == RGPassType::Compute);
}

TEST_CASE("RenderGraph addTransferPass", "[render_graph][graph]") {
    RenderGraph graph;
    
    graph.addTransferPass("TransferPass",
        [](RenderGraphBuilder& builder) {},
        [](RenderGraphContext& ctx) {});
    
    REQUIRE(graph.getPassCount() == 1);
    REQUIRE(graph.getPass(0).type == RGPassType::Transfer);
}

TEST_CASE("RenderGraph compile and execute", "[render_graph][graph]") {
    RenderGraph graph;
    
    // Import back buffer
    TextureHandle backBuffer(1);
    auto bb = graph.importBackBuffer(backBuffer, 1920, 1080);
    
    bool executed = false;
    
    graph.addGraphicsPass("FinalPass",
        [&](RenderGraphBuilder& builder) {
            builder.setRenderTarget(0, bb);
            builder.setFlags(RGPassFlags::NoCulling);
        },
        [&](RenderGraphContext& ctx) {
            executed = true;
        });
    
    REQUIRE_FALSE(graph.isCompiled());
    
    graph.compile();
    REQUIRE(graph.isCompiled());
    
    graph.execute();
    REQUIRE(executed);
}

TEST_CASE("RenderGraph reset", "[render_graph][graph]") {
    RenderGraph graph;
    
    graph.createTexture(RGTextureDesc::renderTarget("Test", 100, 100));
    graph.addGraphicsPass("Pass",
        [](RenderGraphBuilder& builder) {},
        [](RenderGraphContext& ctx) {});
    
    REQUIRE(graph.getPassCount() == 1);
    REQUIRE(graph.getResourceCount() == 1);
    
    graph.reset();
    
    REQUIRE(graph.getPassCount() == 0);
    REQUIRE(graph.getResourceCount() == 0);
    REQUIRE_FALSE(graph.isCompiled());
}

TEST_CASE("RenderGraph dependency tracking", "[render_graph][graph]") {
    RenderGraph graph;
    
    // Create a texture
    RGTextureHandle tex;
    
    // Pass 1 writes to texture
    graph.addGraphicsPass("WritePass",
        [&](RenderGraphBuilder& builder) {
            tex = builder.createTexture(RGTextureDesc::renderTarget("RT", 100, 100));
            builder.setRenderTarget(0, tex);
        },
        [](RenderGraphContext& ctx) {});
    
    // Pass 2 reads from texture
    graph.addGraphicsPass("ReadPass",
        [&](RenderGraphBuilder& builder) {
            builder.read(tex, ResourceAccess::FragmentShader);
        },
        [](RenderGraphContext& ctx) {});
    
    graph.compile();
    
    // ReadPass should depend on WritePass
    const auto& readPass = graph.getPass(1);
    REQUIRE(readPass.dependencies.size() == 1);
    REQUIRE(readPass.dependencies[0] == 0);
}

TEST_CASE("RenderGraph execution order", "[render_graph][graph]") {
    RenderGraph graph;
    
    std::vector<int> executionOrder;
    RGTextureHandle tex;
    
    // Pass 1
    graph.addGraphicsPass("Pass1",
        [&](RenderGraphBuilder& builder) {
            tex = builder.createTexture(RGTextureDesc::renderTarget("RT", 100, 100));
            builder.setRenderTarget(0, tex);
            builder.setFlags(RGPassFlags::NoCulling);
        },
        [&](RenderGraphContext& ctx) {
            executionOrder.push_back(1);
        });
    
    // Pass 2 depends on Pass 1
    graph.addGraphicsPass("Pass2",
        [&](RenderGraphBuilder& builder) {
            builder.read(tex);
            builder.setFlags(RGPassFlags::NoCulling);
        },
        [&](RenderGraphContext& ctx) {
            executionOrder.push_back(2);
        });
    
    graph.compile();
    graph.execute();
    
    REQUIRE(executionOrder.size() == 2);
    REQUIRE(executionOrder[0] == 1);
    REQUIRE(executionOrder[1] == 2);
}

TEST_CASE("RenderGraph dump output", "[render_graph][graph]") {
    RenderGraph graph;
    
    graph.createTexture(RGTextureDesc::renderTarget("RT", 100, 100));
    graph.addGraphicsPass("TestPass",
        [](RenderGraphBuilder& builder) {},
        [](RenderGraphContext& ctx) {});
    
    std::string dump = graph.dump();
    
    REQUIRE(dump.find("Render Graph") != std::string::npos);
    REQUIRE(dump.find("Passes: 1") != std::string::npos);
    REQUIRE(dump.find("Resources: 1") != std::string::npos);
    REQUIRE(dump.find("TestPass") != std::string::npos);
}

TEST_CASE("RenderGraph exportGraphViz output", "[render_graph][graph]") {
    RenderGraph graph;
    
    graph.createTexture(RGTextureDesc::renderTarget("RT", 100, 100));
    graph.addGraphicsPass("TestPass",
        [](RenderGraphBuilder& builder) {},
        [](RenderGraphContext& ctx) {});
    
    std::string dot = graph.exportGraphViz();
    
    REQUIRE(dot.find("digraph RenderGraph") != std::string::npos);
    REQUIRE(dot.find("TestPass") != std::string::npos);
}

// ============================================================================
// Render Graph Builder Tests
// ============================================================================

TEST_CASE("RenderGraphBuilder setViewport", "[render_graph][builder]") {
    RenderGraph graph;
    
    graph.addGraphicsPass("Pass",
        [](RenderGraphBuilder& builder) {
            builder.setViewport(Viewport(1920.0f, 1080.0f));
        },
        [](RenderGraphContext& ctx) {});
    
    const auto& pass = graph.getPass(0);
    REQUIRE(pass.viewport.width == Approx(1920.0f));
    REQUIRE(pass.viewport.height == Approx(1080.0f));
}

TEST_CASE("RenderGraphBuilder setScissor", "[render_graph][builder]") {
    RenderGraph graph;
    
    graph.addGraphicsPass("Pass",
        [](RenderGraphBuilder& builder) {
            builder.setScissor(Scissor(1920, 1080));
        },
        [](RenderGraphContext& ctx) {});
    
    const auto& pass = graph.getPass(0);
    REQUIRE(pass.scissor.width == 1920);
    REQUIRE(pass.scissor.height == 1080);
}

TEST_CASE("RenderGraphBuilder setFlags", "[render_graph][builder]") {
    RenderGraph graph;
    
    graph.addGraphicsPass("Pass",
        [](RenderGraphBuilder& builder) {
            builder.setFlags(RGPassFlags::NoCulling | RGPassFlags::SideEffects);
        },
        [](RenderGraphContext& ctx) {});
    
    const auto& pass = graph.getPass(0);
    REQUIRE(hasFlag(pass.flags, RGPassFlags::NoCulling));
    REQUIRE(hasFlag(pass.flags, RGPassFlags::SideEffects));
}

// ============================================================================
// GBuffer Setup Tests
// ============================================================================

TEST_CASE("GBufferSetup::create", "[render_graph][helpers]") {
    RenderGraph graph;
    
    GBufferSetup gbuffer;
    
    graph.addGraphicsPass("GBufferPass",
        [&](RenderGraphBuilder& builder) {
            gbuffer = GBufferSetup::create(builder, 1920, 1080);
        },
        [](RenderGraphContext& ctx) {});
    
    REQUIRE(gbuffer.albedo.isValid());
    REQUIRE(gbuffer.normal.isValid());
    REQUIRE(gbuffer.material.isValid());
    REQUIRE(gbuffer.depth.isValid());
    
    const auto& pass = graph.getPass(0);
    REQUIRE(pass.colorTargets.size() == 3);
    REQUIRE(pass.hasDepthTarget());
}

// ============================================================================
// Shadow Map Setup Tests
// ============================================================================

TEST_CASE("ShadowMapSetup::create", "[render_graph][helpers]") {
    RenderGraph graph;
    
    ShadowMapSetup shadow;
    
    graph.addGraphicsPass("ShadowPass",
        [&](RenderGraphBuilder& builder) {
            shadow = ShadowMapSetup::create(builder, 4096);
        },
        [](RenderGraphContext& ctx) {});
    
    REQUIRE(shadow.shadowMap.isValid());
    REQUIRE(shadow.resolution == 4096);
    
    const auto& pass = graph.getPass(0);
    REQUIRE(pass.hasDepthTarget());
}

// ============================================================================
// Post Process Setup Tests
// ============================================================================

TEST_CASE("PostProcessSetup::create", "[render_graph][helpers]") {
    RenderGraph graph;
    
    RGTextureHandle input;
    PostProcessSetup pp;
    
    // Create input texture first
    graph.addGraphicsPass("PrePass",
        [&](RenderGraphBuilder& builder) {
            input = builder.createTexture(
                RGTextureDesc::renderTarget("Input", 1920, 1080));
            builder.setRenderTarget(0, input);
        },
        [](RenderGraphContext& ctx) {});
    
    // Post process pass
    graph.addGraphicsPass("PostProcess",
        [&](RenderGraphBuilder& builder) {
            pp = PostProcessSetup::create(builder, input, "Output", 1920, 1080);
        },
        [](RenderGraphContext& ctx) {});
    
    REQUIRE(pp.input.isValid());
    REQUIRE(pp.output.isValid());
}
