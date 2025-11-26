// =============================================================================
// NovaCore Engine - Core Types Test Suite
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// Comprehensive tests for core types, ensuring correctness and type safety.
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "nova/core/types/types.hpp"
#include "nova/core/types/result.hpp"

using namespace nova;
using Catch::Matchers::WithinRel;

// =============================================================================
// Integer Type Tests
// =============================================================================

TEST_CASE("Integer types have correct sizes", "[types][integers]") {
    SECTION("Signed integers") {
        REQUIRE(sizeof(i8) == 1);
        REQUIRE(sizeof(i16) == 2);
        REQUIRE(sizeof(i32) == 4);
        REQUIRE(sizeof(i64) == 8);
    }
    
    SECTION("Unsigned integers") {
        REQUIRE(sizeof(u8) == 1);
        REQUIRE(sizeof(u16) == 2);
        REQUIRE(sizeof(u32) == 4);
        REQUIRE(sizeof(u64) == 8);
    }
}

TEST_CASE("Integer limits are correct", "[types][integers][limits]") {
    SECTION("i8 limits") {
        REQUIRE(limits::I8_MAX == 127);
        REQUIRE(limits::I8_MIN == -128);
    }
    
    SECTION("u8 limits") {
        REQUIRE(limits::U8_MAX == 255);
    }
    
    SECTION("i32 limits") {
        REQUIRE(limits::I32_MAX == 2147483647);
        REQUIRE(limits::I32_MIN == -2147483648);
    }
    
    SECTION("u32 limits") {
        REQUIRE(limits::U32_MAX == 4294967295U);
    }
}

// =============================================================================
// Floating-Point Type Tests
// =============================================================================

TEST_CASE("Floating-point types have correct sizes", "[types][floats]") {
    REQUIRE(sizeof(f32) == 4);
    REQUIRE(sizeof(f64) == 8);
}

TEST_CASE("Mathematical constants are correct", "[types][math]") {
    SECTION("Pi constant") {
        REQUIRE_THAT(math::PI_F32, WithinRel(3.14159265358979323846f, 1e-6f));
        REQUIRE_THAT(math::PI_F64, WithinRel(3.14159265358979323846, 1e-12));
    }
    
    SECTION("Tau constant (2*Pi)") {
        REQUIRE_THAT(math::TAU_F32, WithinRel(6.28318530717958647692f, 1e-6f));
    }
    
    SECTION("Euler's number") {
        REQUIRE_THAT(math::E_F32, WithinRel(2.71828182845904523536f, 1e-6f));
    }
    
    SECTION("Square root of 2") {
        REQUIRE_THAT(math::SQRT2_F32, WithinRel(1.41421356237309504880f, 1e-6f));
    }
    
    SECTION("Golden ratio") {
        REQUIRE_THAT(math::PHI_F32, WithinRel(1.61803398874989484820f, 1e-6f));
    }
    
    SECTION("Degree/radian conversion") {
        REQUIRE_THAT(90.0f * math::DEG_TO_RAD_F32, WithinRel(math::PI_F32 / 2.0f, 1e-6f));
        REQUIRE_THAT(math::PI_F32 * math::RAD_TO_DEG_F32, WithinRel(180.0f, 1e-4f));
    }
}

// =============================================================================
// Handle Type Tests
// =============================================================================

TEST_CASE("Handle type operations", "[types][handle]") {
    struct TestTag {};
    using TestHandle = Handle<TestTag, u32>;
    
    SECTION("Default construction creates invalid handle") {
        TestHandle h;
        REQUIRE_FALSE(h.isValid());
        REQUIRE_FALSE(static_cast<bool>(h));
    }
    
    SECTION("Explicit construction creates valid handle") {
        TestHandle h{42};
        REQUIRE(h.isValid());
        REQUIRE(static_cast<bool>(h));
        REQUIRE(h.get() == 42);
    }
    
    SECTION("Invalid factory creates invalid handle") {
        auto h = TestHandle::invalid();
        REQUIRE_FALSE(h.isValid());
    }
    
    SECTION("Handle comparison works correctly") {
        TestHandle h1{10};
        TestHandle h2{10};
        TestHandle h3{20};
        
        REQUIRE(h1 == h2);
        REQUIRE(h1 != h3);
        REQUIRE(h1 < h3);
    }
    
    SECTION("Different tag types are not comparable") {
        // This should not compile:
        // struct OtherTag {};
        // using OtherHandle = Handle<OtherTag, u32>;
        // TestHandle t{1}; OtherHandle o{1}; bool x = t == o;
        // (compile-time type safety)
        REQUIRE(true); // Placeholder for compile-time check
    }
}

// =============================================================================
// Entity ID Tests
// =============================================================================

TEST_CASE("EntityId operations", "[types][entity]") {
    SECTION("Default construction creates invalid entity") {
        EntityId id;
        REQUIRE_FALSE(id.isValid());
    }
    
    SECTION("Make entity ID from index and version") {
        auto id = makeEntityId(42, 7);
        REQUIRE(id.isValid());
        REQUIRE(entityIndex(id) == 42);
        REQUIRE(entityVersion(id) == 7);
    }
    
    SECTION("Entity ID encodes full 32-bit index range") {
        auto id = makeEntityId(0xFFFFFFFF, 1);
        REQUIRE(entityIndex(id) == 0xFFFFFFFF);
        REQUIRE(entityVersion(id) == 1);
    }
    
    SECTION("Entity ID encodes full 32-bit version range") {
        auto id = makeEntityId(1, 0xFFFFFFFF);
        REQUIRE(entityIndex(id) == 1);
        REQUIRE(entityVersion(id) == 0xFFFFFFFF);
    }
}

// =============================================================================
// Flags Type Tests
// =============================================================================

enum class TestFlags : u32 {
    None = 0,
    A = 1 << 0,
    B = 1 << 1,
    C = 1 << 2,
    All = A | B | C
};
NOVA_ENABLE_BITMASK_OPERATORS(TestFlags)

TEST_CASE("Flags type operations", "[types][flags]") {
    SECTION("Default construction is empty") {
        Flags<TestFlags> flags;
        REQUIRE(flags.isEmpty());
        REQUIRE_FALSE(flags.has(TestFlags::A));
    }
    
    SECTION("Single flag construction") {
        Flags<TestFlags> flags{TestFlags::A};
        REQUIRE(flags.has(TestFlags::A));
        REQUIRE_FALSE(flags.has(TestFlags::B));
    }
    
    SECTION("Set and clear operations") {
        Flags<TestFlags> flags;
        flags.set(TestFlags::A);
        REQUIRE(flags.has(TestFlags::A));
        
        flags.set(TestFlags::B);
        REQUIRE(flags.has(TestFlags::A));
        REQUIRE(flags.has(TestFlags::B));
        
        flags.clear(TestFlags::A);
        REQUIRE_FALSE(flags.has(TestFlags::A));
        REQUIRE(flags.has(TestFlags::B));
    }
    
    SECTION("Toggle operation") {
        Flags<TestFlags> flags{TestFlags::A};
        flags.toggle(TestFlags::A);
        REQUIRE_FALSE(flags.has(TestFlags::A));
        
        flags.toggle(TestFlags::A);
        REQUIRE(flags.has(TestFlags::A));
    }
    
    SECTION("Bitwise operators with enums") {
        auto flags = TestFlags::A | TestFlags::B;
        REQUIRE(flags.has(TestFlags::A));
        REQUIRE(flags.has(TestFlags::B));
        REQUIRE_FALSE(flags.has(TestFlags::C));
    }
}

// =============================================================================
// Memory Literal Tests
// =============================================================================

TEST_CASE("Memory size literals", "[types][literals]") {
    using namespace nova::literals;
    
    SECTION("Kilobyte literal") {
        REQUIRE(1_KB == 1024ULL);
        REQUIRE(4_KB == 4096ULL);
    }
    
    SECTION("Megabyte literal") {
        REQUIRE(1_MB == 1024ULL * 1024ULL);
        REQUIRE(256_MB == 256ULL * 1024ULL * 1024ULL);
    }
    
    SECTION("Gigabyte literal") {
        REQUIRE(1_GB == 1024ULL * 1024ULL * 1024ULL);
        REQUIRE(2_GB == 2ULL * 1024ULL * 1024ULL * 1024ULL);
    }
}

TEST_CASE("Degree literal", "[types][literals]") {
    using namespace nova::literals;
    
    REQUIRE_THAT(90_deg, WithinRel(math::PI_F32 / 2.0f, 1e-6f));
    REQUIRE_THAT(180_deg, WithinRel(math::PI_F32, 1e-6f));
    REQUIRE_THAT(360_deg, WithinRel(math::TAU_F32, 1e-6f));
}

// =============================================================================
// Utility Function Tests
// =============================================================================

TEST_CASE("Alignment utilities", "[types][utils]") {
    SECTION("alignUp") {
        REQUIRE(alignUp(0u, 16u) == 0u);
        REQUIRE(alignUp(1u, 16u) == 16u);
        REQUIRE(alignUp(15u, 16u) == 16u);
        REQUIRE(alignUp(16u, 16u) == 16u);
        REQUIRE(alignUp(17u, 16u) == 32u);
    }
    
    SECTION("alignDown") {
        REQUIRE(alignDown(0u, 16u) == 0u);
        REQUIRE(alignDown(1u, 16u) == 0u);
        REQUIRE(alignDown(15u, 16u) == 0u);
        REQUIRE(alignDown(16u, 16u) == 16u);
        REQUIRE(alignDown(17u, 16u) == 16u);
        REQUIRE(alignDown(31u, 16u) == 16u);
        REQUIRE(alignDown(32u, 16u) == 32u);
    }
}

TEST_CASE("Power of two utilities", "[types][utils]") {
    SECTION("isPowerOfTwo") {
        REQUIRE_FALSE(isPowerOfTwo(0u));
        REQUIRE(isPowerOfTwo(1u));
        REQUIRE(isPowerOfTwo(2u));
        REQUIRE_FALSE(isPowerOfTwo(3u));
        REQUIRE(isPowerOfTwo(4u));
        REQUIRE_FALSE(isPowerOfTwo(5u));
        REQUIRE(isPowerOfTwo(256u));
        REQUIRE_FALSE(isPowerOfTwo(255u));
    }
    
    SECTION("nextPowerOfTwo (32-bit)") {
        REQUIRE(nextPowerOfTwo(0u) == 1u);
        REQUIRE(nextPowerOfTwo(1u) == 1u);
        REQUIRE(nextPowerOfTwo(2u) == 2u);
        REQUIRE(nextPowerOfTwo(3u) == 4u);
        REQUIRE(nextPowerOfTwo(4u) == 4u);
        REQUIRE(nextPowerOfTwo(5u) == 8u);
        REQUIRE(nextPowerOfTwo(255u) == 256u);
        REQUIRE(nextPowerOfTwo(256u) == 256u);
    }
}

TEST_CASE("Hash utilities", "[types][utils]") {
    SECTION("constHash produces consistent results") {
        constexpr auto hash1 = constHash("hello");
        constexpr auto hash2 = constHash("hello");
        constexpr auto hash3 = constHash("world");
        
        REQUIRE(hash1 == hash2);
        REQUIRE(hash1 != hash3);
    }
    
    SECTION("fnv1aHash works at runtime") {
        const char* str1 = "hello";
        const char* str2 = "world";
        
        auto hash1 = fnv1aHash(str1, 5);
        auto hash2 = fnv1aHash(str1, 5);
        auto hash3 = fnv1aHash(str2, 5);
        
        REQUIRE(hash1 == hash2);
        REQUIRE(hash1 != hash3);
    }
    
    SECTION("constHash matches fnv1aHash") {
        constexpr auto constHashResult = constHash("test");
        const char* str = "test";
        auto runtimeHashResult = fnv1aHash(str, 4);
        
        REQUIRE(constHashResult == runtimeHashResult);
    }
}

// =============================================================================
// Result Type Tests
// =============================================================================

TEST_CASE("Result type basic operations", "[types][result]") {
    SECTION("Successful result") {
        Result<int> result = 42;
        REQUIRE(result.has_value());
        REQUIRE(*result == 42);
    }
    
    SECTION("Failed result") {
        Result<int> result = fail(errors::invalidArgument("Test error"));
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error().category() == ErrorCategory::InvalidArgument);
    }
    
    SECTION("ok() helper for value") {
        auto result = ok(42);
        REQUIRE(result.has_value());
        REQUIRE(*result == 42);
    }
    
    SECTION("ok() helper for void") {
        VoidResult result = ok();
        REQUIRE(result.has_value());
    }
}

TEST_CASE("Error type operations", "[types][error]") {
    SECTION("Default error is not an error") {
        Error err;
        REQUIRE_FALSE(err.isError());
        REQUIRE(err.category() == ErrorCategory::None);
    }
    
    SECTION("Error with category and message") {
        Error err{ErrorCategory::IO, "File not found"};
        REQUIRE(err.isError());
        REQUIRE(err.category() == ErrorCategory::IO);
        REQUIRE(err.message() == "File not found");
    }
    
    SECTION("Error factory functions") {
        auto err1 = errors::memory("Out of memory");
        REQUIRE(err1.category() == ErrorCategory::Memory);
        
        auto err2 = errors::notFound("Resource not found");
        REQUIRE(err2.category() == ErrorCategory::NotFound);
        
        auto err3 = errors::graphics("Shader compilation failed", -1);
        REQUIRE(err3.category() == ErrorCategory::Graphics);
        REQUIRE(err3.code() == -1);
    }
    
    SECTION("Error formatting") {
        Error err{ErrorCategory::IO, 42, "Test message"};
        std::string formatted = err.format();
        
        REQUIRE(formatted.find("IO") != std::string::npos);
        REQUIRE(formatted.find("Test message") != std::string::npos);
        REQUIRE(formatted.find("42") != std::string::npos);
    }
}

TEST_CASE("Error category string conversion", "[types][error]") {
    REQUIRE(errorCategoryToString(ErrorCategory::None) == "None");
    REQUIRE(errorCategoryToString(ErrorCategory::Memory) == "Memory");
    REQUIRE(errorCategoryToString(ErrorCategory::IO) == "IO");
    REQUIRE(errorCategoryToString(ErrorCategory::Graphics) == "Graphics");
    REQUIRE(errorCategoryToString(ErrorCategory::Unknown) == "Unknown");
}

// =============================================================================
// Result Composition Tests
// =============================================================================

namespace {
    Result<int> divide(int a, int b) {
        if (b == 0) {
            return fail(errors::invalidArgument("Division by zero"));
        }
        return a / b;
    }
    
    Result<int> multiplyByTwo(int value) {
        if (value > 1000000000) {
            return fail(errors::outOfRange("Value too large"));
        }
        return value * 2;
    }
}

TEST_CASE("Result composition and error propagation", "[types][result]") {
    SECTION("Chained successful operations") {
        auto result = divide(10, 2);
        REQUIRE(result.has_value());
        REQUIRE(*result == 5);
        
        if (result) {
            auto doubled = multiplyByTwo(*result);
            REQUIRE(doubled.has_value());
            REQUIRE(*doubled == 10);
        }
    }
    
    SECTION("Error in first operation") {
        auto result = divide(10, 0);
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error().category() == ErrorCategory::InvalidArgument);
    }
    
    SECTION("Error in second operation") {
        auto result = multiplyByTwo(2000000000);
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error().category() == ErrorCategory::OutOfRange);
    }
}
