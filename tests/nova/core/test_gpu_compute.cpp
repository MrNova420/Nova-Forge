/**
 * @file test_gpu_compute.cpp
 * @brief Comprehensive tests for GPU Compute Abstraction System
 *
 * Copyright (c) NovaForge Platform - All rights reserved.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/render/gpu_compute.hpp>

using namespace nova;
using Catch::Approx;

// ============================================================================
// Configuration Tests
// ============================================================================

TEST_CASE("ComputeConfig constants are valid", "[gpu_compute][config]") {
    SECTION("Workgroup limits are reasonable") {
        CHECK(ComputeConfig::MAX_WORKGROUP_SIZE_X >= 256);
        CHECK(ComputeConfig::MAX_WORKGROUP_SIZE_Y >= 256);
        CHECK(ComputeConfig::MAX_WORKGROUP_SIZE_Z >= 64);
        CHECK(ComputeConfig::MAX_WORKGROUP_INVOCATIONS >= 256);
    }
    
    SECTION("Workgroup count limits are valid") {
        CHECK(ComputeConfig::MAX_WORKGROUP_COUNT_X >= 65535);
        CHECK(ComputeConfig::MAX_WORKGROUP_COUNT_Y >= 65535);
        CHECK(ComputeConfig::MAX_WORKGROUP_COUNT_Z >= 65535);
    }
    
    SECTION("Resource limits are reasonable") {
        CHECK(ComputeConfig::MAX_COMPUTE_BUFFERS >= 8);
        CHECK(ComputeConfig::MAX_COMPUTE_TEXTURES >= 8);
        CHECK(ComputeConfig::MAX_PUSH_CONSTANT_SIZE >= 128);
        CHECK(ComputeConfig::MAX_SHARED_MEMORY >= 16384);
    }
}

// ============================================================================
// Enumeration Tests
// ============================================================================

TEST_CASE("ComputeQueueType enum values and names", "[gpu_compute][enums]") {
    CHECK(static_cast<u8>(ComputeQueueType::Graphics) == 0);
    CHECK(static_cast<u8>(ComputeQueueType::AsyncCompute) == 1);
    CHECK(static_cast<u8>(ComputeQueueType::Transfer) == 2);
    
    CHECK(std::string(getComputeQueueTypeName(ComputeQueueType::Graphics)) == "Graphics");
    CHECK(std::string(getComputeQueueTypeName(ComputeQueueType::AsyncCompute)) == "AsyncCompute");
    CHECK(std::string(getComputeQueueTypeName(ComputeQueueType::Transfer)) == "Transfer");
}

TEST_CASE("BarrierScope flags and operators", "[gpu_compute][enums]") {
    SECTION("Individual flags") {
        CHECK(static_cast<u8>(BarrierScope::None) == 0);
        CHECK(static_cast<u8>(BarrierScope::Compute) == 1);
        CHECK(static_cast<u8>(BarrierScope::Graphics) == 2);
        CHECK(static_cast<u8>(BarrierScope::Transfer) == 4);
        CHECK(static_cast<u8>(BarrierScope::Host) == 8);
    }
    
    SECTION("OR operator") {
        BarrierScope combined = BarrierScope::Compute | BarrierScope::Graphics;
        CHECK(hasScope(combined, BarrierScope::Compute));
        CHECK(hasScope(combined, BarrierScope::Graphics));
        CHECK_FALSE(hasScope(combined, BarrierScope::Transfer));
    }
    
    SECTION("AND operator") {
        BarrierScope all = BarrierScope::All;
        CHECK((all & BarrierScope::Compute) == BarrierScope::Compute);
    }
    
    SECTION("All scope") {
        CHECK(hasScope(BarrierScope::All, BarrierScope::Compute));
        CHECK(hasScope(BarrierScope::All, BarrierScope::Graphics));
        CHECK(hasScope(BarrierScope::All, BarrierScope::Transfer));
        CHECK(hasScope(BarrierScope::All, BarrierScope::Host));
    }
}

TEST_CASE("BufferAccess flags", "[gpu_compute][enums]") {
    CHECK(static_cast<u8>(BufferAccess::None) == 0);
    CHECK(static_cast<u8>(BufferAccess::Read) == 1);
    CHECK(static_cast<u8>(BufferAccess::Write) == 2);
    CHECK(static_cast<u8>(BufferAccess::ReadWrite) == 3);
    
    BufferAccess rw = BufferAccess::Read | BufferAccess::Write;
    CHECK(rw == BufferAccess::ReadWrite);
    CHECK(hasAccess(rw, BufferAccess::Read));
    CHECK(hasAccess(rw, BufferAccess::Write));
}

TEST_CASE("ComputeBufferType enum values", "[gpu_compute][enums]") {
    CHECK(static_cast<u8>(ComputeBufferType::Storage) == 0);
    CHECK(static_cast<u8>(ComputeBufferType::Uniform) == 1);
    CHECK(static_cast<u8>(ComputeBufferType::Indirect) == 2);
    CHECK(static_cast<u8>(ComputeBufferType::Counter) == 3);
    CHECK(static_cast<u8>(ComputeBufferType::AppendConsume) == 4);
    CHECK(static_cast<u8>(ComputeBufferType::Staging) == 5);
    
    CHECK(std::string(getComputeBufferTypeName(ComputeBufferType::Storage)) == "Storage");
    CHECK(std::string(getComputeBufferTypeName(ComputeBufferType::Counter)) == "Counter");
}

TEST_CASE("ImageAccess enum values", "[gpu_compute][enums]") {
    CHECK(static_cast<u8>(ImageAccess::SampleOnly) == 0);
    CHECK(static_cast<u8>(ImageAccess::LoadOnly) == 1);
    CHECK(static_cast<u8>(ImageAccess::StoreOnly) == 2);
    CHECK(static_cast<u8>(ImageAccess::LoadStore) == 3);
    
    CHECK(std::string(getImageAccessName(ImageAccess::LoadStore)) == "LoadStore");
}

// ============================================================================
// WorkgroupSize Tests
// ============================================================================

TEST_CASE("WorkgroupSize construction and properties", "[gpu_compute][workgroup]") {
    SECTION("Default construction") {
        WorkgroupSize wg;
        CHECK(wg.x == 1);
        CHECK(wg.y == 1);
        CHECK(wg.z == 1);
        CHECK(wg.totalInvocations() == 1);
        CHECK(wg.isValid());
    }
    
    SECTION("1D construction") {
        WorkgroupSize wg(256);
        CHECK(wg.x == 256);
        CHECK(wg.y == 1);
        CHECK(wg.z == 1);
        CHECK(wg.totalInvocations() == 256);
        CHECK(wg.isValid());
    }
    
    SECTION("2D construction") {
        WorkgroupSize wg(16, 16);
        CHECK(wg.x == 16);
        CHECK(wg.y == 16);
        CHECK(wg.z == 1);
        CHECK(wg.totalInvocations() == 256);
        CHECK(wg.isValid());
    }
    
    SECTION("3D construction") {
        WorkgroupSize wg(8, 8, 4);
        CHECK(wg.x == 8);
        CHECK(wg.y == 8);
        CHECK(wg.z == 4);
        CHECK(wg.totalInvocations() == 256);
        CHECK(wg.isValid());
    }
    
    SECTION("Invalid workgroup (too many invocations)") {
        WorkgroupSize wg(64, 64, 1);  // 4096 invocations
        CHECK_FALSE(wg.isValid());
    }
    
    SECTION("Invalid workgroup (zero dimension)") {
        WorkgroupSize wg(0, 1, 1);
        CHECK_FALSE(wg.isValid());
    }
}

TEST_CASE("WorkgroupSize presets", "[gpu_compute][workgroup]") {
    SECTION("Linear presets") {
        auto wg64 = WorkgroupSize::linear64();
        CHECK(wg64.x == 64);
        CHECK(wg64.totalInvocations() == 64);
        
        auto wg128 = WorkgroupSize::linear128();
        CHECK(wg128.x == 128);
        
        auto wg256 = WorkgroupSize::linear256();
        CHECK(wg256.x == 256);
    }
    
    SECTION("Tile presets") {
        auto tile8 = WorkgroupSize::tile8x8();
        CHECK(tile8.x == 8);
        CHECK(tile8.y == 8);
        CHECK(tile8.totalInvocations() == 64);
        
        auto tile16 = WorkgroupSize::tile16x16();
        CHECK(tile16.totalInvocations() == 256);
        
        auto tile32 = WorkgroupSize::tile32x32();
        CHECK(tile32.totalInvocations() == 1024);
    }
    
    SECTION("Volume presets") {
        auto vol4 = WorkgroupSize::volume4x4x4();
        CHECK(vol4.totalInvocations() == 64);
        
        auto vol8 = WorkgroupSize::volume8x8x8();
        CHECK(vol8.totalInvocations() == 512);
    }
}

// ============================================================================
// DispatchArgs Tests
// ============================================================================

TEST_CASE("DispatchArgs construction and calculations", "[gpu_compute][dispatch]") {
    SECTION("Default construction") {
        DispatchArgs args;
        CHECK(args.groupCountX == 1);
        CHECK(args.groupCountY == 1);
        CHECK(args.groupCountZ == 1);
        CHECK(args.totalGroups() == 1);
        CHECK(args.isValid());
    }
    
    SECTION("1D dispatch") {
        DispatchArgs args(100);
        CHECK(args.totalGroups() == 100);
        CHECK(args.totalThreads(WorkgroupSize::linear256()) == 25600);
    }
    
    SECTION("2D dispatch") {
        DispatchArgs args(10, 20);
        CHECK(args.totalGroups() == 200);
    }
    
    SECTION("3D dispatch") {
        DispatchArgs args(5, 10, 4);
        CHECK(args.totalGroups() == 200);
    }
}

TEST_CASE("DispatchArgs factory methods", "[gpu_compute][dispatch]") {
    SECTION("for1D") {
        auto args = DispatchArgs::for1D(1000, 64);
        CHECK(args.groupCountX == 16);  // ceil(1000/64) = 16
        CHECK(args.groupCountY == 1);
        CHECK(args.groupCountZ == 1);
    }
    
    SECTION("for1D exact multiple") {
        auto args = DispatchArgs::for1D(256, 64);
        CHECK(args.groupCountX == 4);
    }
    
    SECTION("for2D") {
        auto args = DispatchArgs::for2D(1920, 1080, WorkgroupSize::tile16x16());
        CHECK(args.groupCountX == 120);  // ceil(1920/16)
        CHECK(args.groupCountY == 68);   // ceil(1080/16)
    }
    
    SECTION("for3D") {
        auto args = DispatchArgs::for3D(64, 64, 64, WorkgroupSize::volume8x8x8());
        CHECK(args.groupCountX == 8);
        CHECK(args.groupCountY == 8);
        CHECK(args.groupCountZ == 8);
    }
}

// ============================================================================
// ComputeBufferDesc Tests
// ============================================================================

TEST_CASE("ComputeBufferDesc factory methods", "[gpu_compute][buffer]") {
    SECTION("Storage buffer") {
        auto desc = ComputeBufferDesc::storage("particles", 1024 * 64, 64);
        CHECK(desc.name == "particles");
        CHECK(desc.type == ComputeBufferType::Storage);
        CHECK(desc.size == 1024 * 64);
        CHECK(desc.stride == 64);
        CHECK(desc.isStructured());
        CHECK(desc.elementCount() == 1024);
    }
    
    SECTION("Uniform buffer") {
        auto desc = ComputeBufferDesc::uniform("constants", 256);
        CHECK(desc.type == ComputeBufferType::Uniform);
        CHECK(desc.access == BufferAccess::Read);
        CHECK_FALSE(desc.isStructured());
    }
    
    SECTION("Indirect buffer") {
        auto desc = ComputeBufferDesc::indirect("dispatch_args", 10);
        CHECK(desc.type == ComputeBufferType::Indirect);
        CHECK(desc.size == 10 * sizeof(IndirectDispatchArgs));
        CHECK(desc.stride == sizeof(IndirectDispatchArgs));
    }
    
    SECTION("Counter buffer") {
        auto desc = ComputeBufferDesc::counter("counters", 4);
        CHECK(desc.type == ComputeBufferType::Counter);
        CHECK(desc.size == 4 * sizeof(u32));
        CHECK(hasAccess(desc.access, BufferAccess::Atomic));
    }
    
    SECTION("AppendConsume buffer") {
        auto desc = ComputeBufferDesc::appendConsume("stream", 10000, 32);
        CHECK(desc.type == ComputeBufferType::AppendConsume);
        CHECK(desc.size == 10000 * 32);
        CHECK(desc.elementCount() == 10000);
    }
    
    SECTION("Staging buffer") {
        auto desc = ComputeBufferDesc::staging("upload", 4096);
        CHECK(desc.type == ComputeBufferType::Staging);
        CHECK(desc.cpuVisible);
        CHECK(desc.persistentMap);
    }
}

// ============================================================================
// MemoryBarrier Tests
// ============================================================================

TEST_CASE("MemoryBarrier factory methods", "[gpu_compute][barrier]") {
    SECTION("computeToCompute") {
        auto barrier = MemoryBarrier::computeToCompute();
        CHECK(barrier.srcScope == BarrierScope::Compute);
        CHECK(barrier.dstScope == BarrierScope::Compute);
        CHECK(barrier.srcAccess == BufferAccess::Write);
        CHECK(barrier.dstAccess == BufferAccess::Read);
    }
    
    SECTION("computeToGraphics") {
        auto barrier = MemoryBarrier::computeToGraphics();
        CHECK(barrier.srcScope == BarrierScope::Compute);
        CHECK(barrier.dstScope == BarrierScope::Graphics);
    }
    
    SECTION("graphicsToCompute") {
        auto barrier = MemoryBarrier::graphicsToCompute();
        CHECK(barrier.srcScope == BarrierScope::Graphics);
        CHECK(barrier.dstScope == BarrierScope::Compute);
    }
    
    SECTION("computeToHost") {
        auto barrier = MemoryBarrier::computeToHost();
        CHECK(barrier.dstScope == BarrierScope::Host);
    }
    
    SECTION("full barrier") {
        auto barrier = MemoryBarrier::full();
        CHECK(barrier.srcScope == BarrierScope::All);
        CHECK(barrier.dstScope == BarrierScope::All);
        CHECK(barrier.srcAccess == BufferAccess::ReadWrite);
        CHECK(barrier.dstAccess == BufferAccess::ReadWrite);
    }
}

// ============================================================================
// SpecializationConstant Tests
// ============================================================================

TEST_CASE("SpecializationConstant creation", "[gpu_compute][shader]") {
    SECTION("Int constant") {
        auto sc = SpecializationConstant::fromInt(0, -42);
        CHECK(sc.id == 0);
        CHECK(sc.intValue == -42);
        CHECK(sc.type == SpecializationConstant::Type::Int);
    }
    
    SECTION("UInt constant") {
        auto sc = SpecializationConstant::fromUInt(1, 256);
        CHECK(sc.id == 1);
        CHECK(sc.uintValue == 256);
        CHECK(sc.type == SpecializationConstant::Type::UInt);
    }
    
    SECTION("Float constant") {
        auto sc = SpecializationConstant::fromFloat(2, 3.14f);
        CHECK(sc.id == 2);
        CHECK(sc.floatValue == Approx(3.14f));
        CHECK(sc.type == SpecializationConstant::Type::Float);
    }
}

// ============================================================================
// GPU Data Structure Size Tests
// ============================================================================

TEST_CASE("GPU structure sizes are correct", "[gpu_compute][structures]") {
    CHECK(sizeof(IndirectDispatchArgs) == 12);
    CHECK(sizeof(GPUDispatchCommand) == 32);
    CHECK(IndirectDispatchArgs::byteSize() == 12);
    CHECK(GPUDispatchCommand::byteSize() == 32);
}

// ============================================================================
// ComputeContext Tests
// ============================================================================

TEST_CASE("ComputeContext basic operations", "[gpu_compute][context]") {
    ComputeContext ctx;
    
    SECTION("Initial state") {
        CHECK(ctx.commandCount() == 0);
        CHECK(ctx.stats().totalDispatches == 0);
    }
    
    SECTION("Dispatch recording") {
        ctx.bindPipeline(1);
        ctx.dispatch(DispatchArgs(10, 10, 1));
        CHECK(ctx.commandCount() == 1);
        CHECK(ctx.stats().totalDispatches == 1);
    }
    
    SECTION("1D dispatch helper") {
        ctx.dispatch1D(1000, 64);
        CHECK(ctx.commandCount() == 1);
    }
    
    SECTION("2D dispatch helper") {
        ctx.dispatch2D(1920, 1080);
        CHECK(ctx.commandCount() == 1);
    }
    
    SECTION("3D dispatch helper") {
        ctx.dispatch3D(64, 64, 64);
        CHECK(ctx.commandCount() == 1);
    }
    
    SECTION("Indirect dispatch") {
        ctx.dispatchIndirect(42, 0);
        CHECK(ctx.stats().indirectDispatches == 1);
    }
    
    SECTION("Barrier insertion") {
        ctx.barrier(MemoryBarrier::computeToCompute());
        CHECK(ctx.stats().barriers == 1);
    }
    
    SECTION("Buffer operations") {
        ctx.copyBuffer(1, 2, 0, 0, 1024);
        ctx.fillBuffer(1, 0, 0, 256);
        CHECK(ctx.commandCount() == 2);
    }
    
    SECTION("Reset") {
        ctx.dispatch(DispatchArgs(1));
        ctx.barrier(MemoryBarrier::full());
        CHECK(ctx.commandCount() == 2);
        
        ctx.reset();
        CHECK(ctx.commandCount() == 0);
        CHECK(ctx.stats().totalDispatches == 0);
    }
}

TEST_CASE("ComputeContext multiple dispatches", "[gpu_compute][context]") {
    ComputeContext ctx;
    
    // Simulate a multi-pass compute workload
    ctx.bindPipeline(1);
    ctx.dispatch(DispatchArgs(100));
    ctx.barrier(MemoryBarrier::computeToCompute());
    
    ctx.bindPipeline(2);
    ctx.dispatch(DispatchArgs(50));
    ctx.barrier(MemoryBarrier::computeToCompute());
    
    ctx.bindPipeline(3);
    ctx.dispatch(DispatchArgs(25));
    
    CHECK(ctx.stats().totalDispatches == 3);
    CHECK(ctx.stats().barriers == 2);
    CHECK(ctx.commandCount() == 5);  // 3 dispatches + 2 barriers
}

// ============================================================================
// ComputeCapabilities Tests
// ============================================================================

TEST_CASE("ComputeCapabilities validation", "[gpu_compute][capabilities]") {
    ComputeCapabilities caps;
    caps.maxWorkgroupSizeX = 1024;
    caps.maxWorkgroupSizeY = 1024;
    caps.maxWorkgroupSizeZ = 64;
    caps.maxWorkgroupInvocations = 1024;
    
    SECTION("Valid workgroup sizes") {
        CHECK(caps.supportsWorkgroupSize(WorkgroupSize::linear256()));
        CHECK(caps.supportsWorkgroupSize(WorkgroupSize::tile16x16()));
        CHECK(caps.supportsWorkgroupSize(WorkgroupSize::volume8x8x8()));
    }
    
    SECTION("Invalid workgroup (too many invocations)") {
        CHECK_FALSE(caps.supportsWorkgroupSize(WorkgroupSize(64, 64, 1)));
    }
}

// ============================================================================
// ComputeManager Tests
// ============================================================================

TEST_CASE("ComputeManager singleton and initialization", "[gpu_compute][manager]") {
    auto& manager = ComputeManager::get();
    
    SECTION("Singleton access") {
        auto& manager2 = ComputeManager::get();
        CHECK(&manager == &manager2);
    }
    
    SECTION("Initialization") {
        CHECK(manager.initialize());
        CHECK(manager.isInitialized());
    }
    
    SECTION("Capabilities") {
        manager.initialize();
        const auto& caps = manager.capabilities();
        CHECK(caps.supportsAsyncCompute);
        CHECK(caps.supportsIndirectDispatch);
        CHECK(caps.supportsAtomics);
    }
}

TEST_CASE("ComputeManager resource creation", "[gpu_compute][manager]") {
    auto& manager = ComputeManager::get();
    manager.initialize();
    
    SECTION("Shader creation and destruction") {
        ComputeShaderDesc desc;
        desc.name = "test_shader";
        desc.spirvCode = {0x07, 0x23, 0x02, 0x03};  // Fake SPIR-V
        desc.workgroupSize = WorkgroupSize::linear256();
        
        u64 handle = manager.createShader(desc);
        CHECK(handle != 0);
        
        manager.destroyShader(handle);
    }
    
    SECTION("Pipeline creation and destruction") {
        ComputePipelineDesc desc;
        desc.name = "test_pipeline";
        desc.shaderHandle = 1;
        
        u64 handle = manager.createPipeline(desc);
        CHECK(handle != 0);
        
        manager.destroyPipeline(handle);
    }
    
    SECTION("Buffer creation and query") {
        auto bufDesc = ComputeBufferDesc::storage("test", 1024, 16);
        u64 handle = manager.createBuffer(bufDesc);
        CHECK(handle != 0);
        
        const auto* desc = manager.getBufferDesc(handle);
        REQUIRE(desc != nullptr);
        CHECK(desc->name == "test");
        CHECK(desc->size == 1024);
        
        manager.destroyBuffer(handle);
        CHECK(manager.getBufferDesc(handle) == nullptr);
    }
}

TEST_CASE("ComputeManager submission", "[gpu_compute][manager]") {
    auto& manager = ComputeManager::get();
    manager.initialize();
    manager.resetStats();
    
    ComputeContext ctx;
    ctx.bindPipeline(1);
    ctx.dispatch(DispatchArgs(100));
    ctx.dispatch(DispatchArgs(50));
    ctx.dispatchIndirect(42);
    
    manager.submit(ctx, ComputeQueueType::AsyncCompute);
    
    CHECK(manager.totalStats().totalDispatches == 2);
    CHECK(manager.totalStats().indirectDispatches == 1);
}

// ============================================================================
// Utility Function Tests
// ============================================================================

TEST_CASE("Utility functions", "[gpu_compute][utility]") {
    SECTION("calculateOptimalWorkgroupSize1D") {
        auto wg = calculateOptimalWorkgroupSize1D(1000, 256);
        CHECK(wg.x == 256);
        CHECK(wg.y == 1);
        CHECK(wg.z == 1);
        
        wg = calculateOptimalWorkgroupSize1D(50, 256);
        CHECK(wg.x <= 64);  // Should reduce for small counts
    }
    
    SECTION("calculateOptimalWorkgroupSize2D") {
        auto wg = calculateOptimalWorkgroupSize2D(1920, 1080, 256);
        CHECK(wg.x > 0);
        CHECK(wg.y > 0);
        CHECK(wg.totalInvocations() <= 256);
    }
    
    SECTION("roundUpToMultiple") {
        CHECK(roundUpToMultiple(100, 64) == 128);
        CHECK(roundUpToMultiple(64, 64) == 64);
        CHECK(roundUpToMultiple(1, 256) == 256);
        CHECK(roundUpToMultiple(0, 64) == 0);
    }
    
    SECTION("alignBufferSize") {
        CHECK(alignBufferSize(1000, 256) == 1024);
        CHECK(alignBufferSize(256, 256) == 256);
        CHECK(alignBufferSize(1, 256) == 256);
    }
    
    SECTION("dispatchGroupsNeeded") {
        CHECK(dispatchGroupsNeeded(1000, 64) == 16);
        CHECK(dispatchGroupsNeeded(64, 64) == 1);
        CHECK(dispatchGroupsNeeded(65, 64) == 2);
    }
}

// ============================================================================
// ComputeProfilingData Tests
// ============================================================================

TEST_CASE("ComputeProfilingData calculations", "[gpu_compute][profiling]") {
    ComputeProfilingData data;
    data.name = "test";
    data.gpuTimeMs = 10.0;
    data.memoryBandwidthBytes = 10ULL * 1024 * 1024 * 1024;  // 10 GB
    
    CHECK(data.throughputGBps() == Approx(1000.0));  // 10GB in 10ms = 1000 GB/s
}

TEST_CASE("ComputeStats reset", "[gpu_compute][stats]") {
    ComputeStats stats;
    stats.totalDispatches = 100;
    stats.barriers = 50;
    stats.totalGpuTimeMs = 16.0;
    
    stats.reset();
    
    CHECK(stats.totalDispatches == 0);
    CHECK(stats.barriers == 0);
    CHECK(stats.totalGpuTimeMs == 0.0);
}
