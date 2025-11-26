// =============================================================================
// NovaCore Engine - Memory Allocator Tests
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <cstring>

#include "nova/core/memory/memory.hpp"

using namespace nova;
using namespace nova::memory;

// =============================================================================
// Linear Allocator Tests
// =============================================================================

TEST_CASE("LinearAllocator basic operations", "[memory][linear]") {
    constexpr usize bufferSize = 1024;
    std::vector<u8> buffer(bufferSize);
    LinearAllocator allocator(buffer.data(), bufferSize, "TestLinear");
    
    SECTION("Initial state") {
        REQUIRE(allocator.getAllocatedSize() == 0);
        REQUIRE(allocator.getFreeSpace() == bufferSize);
        REQUIRE(allocator.getCapacity() == bufferSize);
        REQUIRE(std::strcmp(allocator.getName(), "TestLinear") == 0);
    }
    
    SECTION("Simple allocation") {
        void* ptr = allocator.allocate(64);
        REQUIRE(ptr != nullptr);
        REQUIRE(allocator.owns(ptr));
        REQUIRE(allocator.getAllocatedSize() >= 64);
    }
    
    SECTION("Multiple allocations") {
        void* ptr1 = allocator.allocate(32);
        void* ptr2 = allocator.allocate(64);
        void* ptr3 = allocator.allocate(128);
        
        REQUIRE(ptr1 != nullptr);
        REQUIRE(ptr2 != nullptr);
        REQUIRE(ptr3 != nullptr);
        
        // Pointers should be different
        REQUIRE(ptr1 != ptr2);
        REQUIRE(ptr2 != ptr3);
    }
    
    SECTION("Allocation with alignment") {
        void* ptr1 = allocator.allocate(1);  // Offset 1 byte
        void* ptr2 = allocator.allocate(16, 16);  // Should be 16-byte aligned
        
        REQUIRE(ptr1 != nullptr);
        REQUIRE(ptr2 != nullptr);
        REQUIRE(reinterpret_cast<uintptr_t>(ptr2) % 16 == 0);
    }
    
    SECTION("Reset clears all allocations") {
        allocator.allocate(64);
        allocator.allocate(128);
        
        allocator.reset();
        
        REQUIRE(allocator.getAllocatedSize() == 0);
        REQUIRE(allocator.getFreeSpace() == bufferSize);
    }
    
    SECTION("Marker-based reset") {
        void* ptr1 = allocator.allocate(32);
        usize marker = allocator.getMarker();
        void* ptr2 = allocator.allocate(64);
        void* ptr3 = allocator.allocate(128);
        
        REQUIRE(ptr1 != nullptr);
        REQUIRE(ptr2 != nullptr);
        REQUIRE(ptr3 != nullptr);
        
        allocator.resetToMarker(marker);
        
        // Should be back to marker position
        REQUIRE(allocator.getAllocatedSize() == marker);
    }
    
    SECTION("Out of memory returns nullptr") {
        void* ptr = allocator.allocate(bufferSize + 1);
        REQUIRE(ptr == nullptr);
    }
    
    SECTION("Statistics tracking") {
        allocator.allocate(100);
        allocator.allocate(200);
        
        auto stats = allocator.getStats();
        REQUIRE(stats.allocationCount == 2);
        REQUIRE(stats.totalAllocationCount == 2);
        REQUIRE(stats.totalAllocated >= 300);
    }
}

TEST_CASE("ScopedLinearReset RAII", "[memory][linear]") {
    constexpr usize bufferSize = 1024;
    std::vector<u8> buffer(bufferSize);
    LinearAllocator allocator(buffer.data(), bufferSize);
    
    (void)allocator.allocate(64);
    usize markerBefore = allocator.getMarker();
    
    {
        ScopedLinearReset scope(allocator);
        (void)allocator.allocate(128);
        (void)allocator.allocate(256);
        REQUIRE(allocator.getMarker() > markerBefore);
    }
    
    // After scope exit, should be reset to marker
    REQUIRE(allocator.getMarker() == markerBefore);
}

// =============================================================================
// Pool Allocator Tests
// =============================================================================

TEST_CASE("PoolAllocator basic operations", "[memory][pool]") {
    constexpr usize blockSize = 64;
    constexpr usize blockCount = 10;
    constexpr usize bufferSize = blockSize * blockCount;
    std::vector<u8> buffer(bufferSize);
    PoolAllocator allocator(buffer.data(), bufferSize, blockSize, "TestPool");
    
    SECTION("Initial state") {
        REQUIRE(allocator.getBlockSize() == blockSize);
        REQUIRE(allocator.getBlockCount() == blockCount);
        REQUIRE(allocator.getFreeBlockCount() == blockCount);
        REQUIRE(std::strcmp(allocator.getName(), "TestPool") == 0);
    }
    
    SECTION("Allocate and deallocate single block") {
        void* ptr = allocator.allocate(blockSize);
        REQUIRE(ptr != nullptr);
        REQUIRE(allocator.owns(ptr));
        REQUIRE(allocator.getFreeBlockCount() == blockCount - 1);
        
        allocator.deallocate(ptr);
        REQUIRE(allocator.getFreeBlockCount() == blockCount);
    }
    
    SECTION("Allocate all blocks") {
        std::vector<void*> ptrs;
        for (usize i = 0; i < blockCount; ++i) {
            void* ptr = allocator.allocate(blockSize);
            REQUIRE(ptr != nullptr);
            ptrs.push_back(ptr);
        }
        
        // Pool should be exhausted
        void* extra = allocator.allocate(blockSize);
        REQUIRE(extra == nullptr);
        
        // Free all
        for (void* ptr : ptrs) {
            allocator.deallocate(ptr);
        }
        REQUIRE(allocator.getFreeBlockCount() == blockCount);
    }
    
    SECTION("Allocation larger than block size fails") {
        void* ptr = allocator.allocate(blockSize + 1);
        REQUIRE(ptr == nullptr);
    }
    
    SECTION("Reset returns all blocks") {
        for (usize i = 0; i < blockCount / 2; ++i) {
            (void)allocator.allocate(blockSize);
        }
        
        allocator.reset();
        REQUIRE(allocator.getFreeBlockCount() == blockCount);
    }
}

TEST_CASE("TypedPoolAllocator", "[memory][pool]") {
    struct TestObject {
        int value;
        float data[3];
        
        TestObject() : value(0), data{0, 0, 0} {}
        TestObject(int v) : value(v), data{0, 0, 0} {}
    };
    
    constexpr usize objectCount = 10;
    constexpr usize bufferSize = sizeof(TestObject) * objectCount;
    std::vector<u8> buffer(bufferSize);
    memory::TypedPoolAllocator<TestObject> pool(buffer.data(), bufferSize, "TestTypedPool");
    
    SECTION("Create and destroy objects") {
        TestObject* obj = pool.create(42);
        REQUIRE(obj != nullptr);
        REQUIRE(obj->value == 42);
        
        pool.destroy(obj);
    }
    
    SECTION("Multiple objects") {
        std::vector<TestObject*> objects;
        for (int i = 0; i < 5; ++i) {
            objects.push_back(pool.create(i * 10));
        }
        
        for (int i = 0; i < 5; ++i) {
            REQUIRE(objects[i]->value == i * 10);
        }
        
        for (auto* obj : objects) {
            pool.destroy(obj);
        }
    }
}

// =============================================================================
// Stack Allocator Tests
// =============================================================================

TEST_CASE("StackAllocator basic operations", "[memory][stack]") {
    constexpr usize bufferSize = 1024;
    std::vector<u8> buffer(bufferSize);
    StackAllocator allocator(buffer.data(), bufferSize, "TestStack");
    
    SECTION("Initial state") {
        REQUIRE(allocator.getCapacity() == bufferSize);
        REQUIRE(allocator.getFreeSpace() == bufferSize);
        REQUIRE(std::strcmp(allocator.getName(), "TestStack") == 0);
    }
    
    SECTION("Allocate from top") {
        void* ptr1 = allocator.allocateTop(64);
        void* ptr2 = allocator.allocateTop(128);
        
        REQUIRE(ptr1 != nullptr);
        REQUIRE(ptr2 != nullptr);
        REQUIRE(ptr1 < ptr2);  // Top grows upward
    }
    
    SECTION("Allocate from bottom") {
        void* ptr1 = allocator.allocateBottom(64);
        void* ptr2 = allocator.allocateBottom(128);
        
        REQUIRE(ptr1 != nullptr);
        REQUIRE(ptr2 != nullptr);
        REQUIRE(ptr1 > ptr2);  // Bottom grows downward
    }
    
    SECTION("Double-ended allocation") {
        void* top = allocator.allocateTop(256);
        void* bottom = allocator.allocateBottom(256);
        
        REQUIRE(top != nullptr);
        REQUIRE(bottom != nullptr);
        REQUIRE(top < bottom);
    }
    
    SECTION("LIFO deallocation") {
        void* ptr1 = allocator.allocateTop(32);
        usize marker = allocator.getTopMarker();
        void* ptr2 = allocator.allocateTop(64);
        
        allocator.deallocateTop(ptr2);
        REQUIRE(allocator.getTopMarker() == marker);
        
        // Should be able to allocate same space again
        void* ptr3 = allocator.allocateTop(64);
        REQUIRE(ptr3 != nullptr);
    }
    
    SECTION("Marker-based reset") {
        (void)allocator.allocateTop(64);
        usize topMarker = allocator.getTopMarker();
        usize bottomMarker = allocator.getBottomMarker();
        
        (void)allocator.allocateTop(128);
        (void)allocator.allocateBottom(128);
        
        allocator.resetTopToMarker(topMarker);
        allocator.resetBottomToMarker(bottomMarker);
        
        REQUIRE(allocator.getTopMarker() == topMarker);
        REQUIRE(allocator.getBottomMarker() == bottomMarker);
    }
    
    SECTION("Reset clears everything") {
        (void)allocator.allocateTop(256);
        (void)allocator.allocateBottom(256);
        
        allocator.reset();
        
        REQUIRE(allocator.getFreeSpace() == bufferSize);
    }
}

TEST_CASE("ScopedStackReset RAII", "[memory][stack]") {
    constexpr usize bufferSize = 1024;
    std::vector<u8> buffer(bufferSize);
    StackAllocator allocator(buffer.data(), bufferSize);
    
    (void)allocator.allocateTop(64);
    usize topBefore = allocator.getTopMarker();
    usize bottomBefore = allocator.getBottomMarker();
    
    {
        ScopedStackReset scope(allocator);
        (void)allocator.allocateTop(128);
        (void)allocator.allocateBottom(128);
    }
    
    REQUIRE(allocator.getTopMarker() == topBefore);
    REQUIRE(allocator.getBottomMarker() == bottomBefore);
}

// =============================================================================
// Memory Utility Tests
// =============================================================================

TEST_CASE("Aligned allocation", "[memory][utils]") {
    SECTION("16-byte alignment") {
        void* ptr = alignedAlloc(64, 16);
        REQUIRE(ptr != nullptr);
        REQUIRE(reinterpret_cast<uintptr_t>(ptr) % 16 == 0);
        alignedFree(ptr);
    }
    
    SECTION("64-byte alignment (cache line)") {
        void* ptr = alignedAlloc(256, 64);
        REQUIRE(ptr != nullptr);
        REQUIRE(reinterpret_cast<uintptr_t>(ptr) % 64 == 0);
        alignedFree(ptr);
    }
}

TEST_CASE("Memory utility functions", "[memory][utils]") {
    constexpr usize size = 64;
    std::vector<u8> buffer1(size);
    std::vector<u8> buffer2(size);
    
    SECTION("zeroMemory") {
        std::fill(buffer1.begin(), buffer1.end(), 0xFF);
        zeroMemory(buffer1.data(), size);
        
        for (u8 byte : buffer1) {
            REQUIRE(byte == 0);
        }
    }
    
    SECTION("fillMemory") {
        fillMemory(buffer1.data(), 0xAB, size);
        
        for (u8 byte : buffer1) {
            REQUIRE(byte == 0xAB);
        }
    }
    
    SECTION("copyMemory") {
        for (usize i = 0; i < size; ++i) {
            buffer1[i] = static_cast<u8>(i);
        }
        
        copyMemory(buffer2.data(), buffer1.data(), size);
        
        REQUIRE(compareMemory(buffer1.data(), buffer2.data(), size) == 0);
    }
    
    SECTION("moveMemory with overlap") {
        for (usize i = 0; i < size; ++i) {
            buffer1[i] = static_cast<u8>(i);
        }
        
        // Move overlapping region
        moveMemory(buffer1.data() + 16, buffer1.data(), 32);
        
        // First 16 bytes unchanged
        for (usize i = 0; i < 16; ++i) {
            REQUIRE(buffer1[i] == static_cast<u8>(i));
        }
        // Next 32 bytes are copy of original 0-31
        for (usize i = 0; i < 32; ++i) {
            REQUIRE(buffer1[16 + i] == static_cast<u8>(i));
        }
    }
}
