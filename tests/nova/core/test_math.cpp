// =============================================================================
// NovaCore Engine - Math Module Tests
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// Comprehensive tests for the math library including vectors, matrices,
// quaternions, and transforms.
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/math/math.hpp>
#include <cmath>

using namespace nova;
using namespace nova::math;
using Catch::Approx;

// =============================================================================
// Vec2 Tests
// =============================================================================

TEST_CASE("Vec2 basic operations", "[math][vec2]") {
    SECTION("Default constructor creates zero vector") {
        Vec2 v;
        REQUIRE(v.x == 0.0f);
        REQUIRE(v.y == 0.0f);
    }
    
    SECTION("Scalar constructor") {
        Vec2 v(5.0f);
        REQUIRE(v.x == 5.0f);
        REQUIRE(v.y == 5.0f);
    }
    
    SECTION("Component constructor") {
        Vec2 v(1.0f, 2.0f);
        REQUIRE(v.x == 1.0f);
        REQUIRE(v.y == 2.0f);
    }
    
    SECTION("Static factory methods") {
        REQUIRE(Vec2::zero() == Vec2(0.0f, 0.0f));
        REQUIRE(Vec2::one() == Vec2(1.0f, 1.0f));
        REQUIRE(Vec2::unitX() == Vec2(1.0f, 0.0f));
        REQUIRE(Vec2::unitY() == Vec2(0.0f, 1.0f));
    }
    
    SECTION("Addition") {
        Vec2 a(1.0f, 2.0f);
        Vec2 b(3.0f, 4.0f);
        Vec2 c = a + b;
        REQUIRE(c.x == 4.0f);
        REQUIRE(c.y == 6.0f);
    }
    
    SECTION("Subtraction") {
        Vec2 a(5.0f, 7.0f);
        Vec2 b(2.0f, 3.0f);
        Vec2 c = a - b;
        REQUIRE(c.x == 3.0f);
        REQUIRE(c.y == 4.0f);
    }
    
    SECTION("Multiplication") {
        Vec2 v(2.0f, 3.0f);
        Vec2 scaled = v * 2.0f;
        REQUIRE(scaled.x == 4.0f);
        REQUIRE(scaled.y == 6.0f);
    }
    
    SECTION("Dot product") {
        Vec2 a(1.0f, 0.0f);
        Vec2 b(0.0f, 1.0f);
        REQUIRE(a.dot(b) == 0.0f);
        
        Vec2 c(1.0f, 2.0f);
        Vec2 d(3.0f, 4.0f);
        REQUIRE(c.dot(d) == 11.0f); // 1*3 + 2*4 = 11
    }
    
    SECTION("Length") {
        Vec2 v(3.0f, 4.0f);
        REQUIRE(v.length() == Approx(5.0f));
        REQUIRE(v.lengthSquared() == 25.0f);
    }
    
    SECTION("Normalize") {
        Vec2 v(3.0f, 4.0f);
        Vec2 n = v.normalized();
        REQUIRE(n.length() == Approx(1.0f));
        REQUIRE(n.x == Approx(0.6f));
        REQUIRE(n.y == Approx(0.8f));
    }
}

// =============================================================================
// Vec3 Tests
// =============================================================================

TEST_CASE("Vec3 basic operations", "[math][vec3]") {
    SECTION("Default constructor creates zero vector") {
        Vec3 v;
        REQUIRE(v.x == 0.0f);
        REQUIRE(v.y == 0.0f);
        REQUIRE(v.z == 0.0f);
    }
    
    SECTION("Scalar constructor") {
        Vec3 v(5.0f);
        REQUIRE(v.x == 5.0f);
        REQUIRE(v.y == 5.0f);
        REQUIRE(v.z == 5.0f);
    }
    
    SECTION("Component constructor") {
        Vec3 v(1.0f, 2.0f, 3.0f);
        REQUIRE(v.x == 1.0f);
        REQUIRE(v.y == 2.0f);
        REQUIRE(v.z == 3.0f);
    }
    
    SECTION("Static factory methods") {
        REQUIRE(Vec3::zero() == Vec3(0.0f, 0.0f, 0.0f));
        REQUIRE(Vec3::one() == Vec3(1.0f, 1.0f, 1.0f));
        REQUIRE(Vec3::unitX() == Vec3(1.0f, 0.0f, 0.0f));
        REQUIRE(Vec3::unitY() == Vec3(0.0f, 1.0f, 0.0f));
        REQUIRE(Vec3::unitZ() == Vec3(0.0f, 0.0f, 1.0f));
        REQUIRE(Vec3::up() == Vec3(0.0f, 1.0f, 0.0f));
        REQUIRE(Vec3::forward() == Vec3(0.0f, 0.0f, -1.0f));
        REQUIRE(Vec3::right() == Vec3(1.0f, 0.0f, 0.0f));
    }
    
    SECTION("Addition") {
        Vec3 a(1.0f, 2.0f, 3.0f);
        Vec3 b(4.0f, 5.0f, 6.0f);
        Vec3 c = a + b;
        REQUIRE(c.x == 5.0f);
        REQUIRE(c.y == 7.0f);
        REQUIRE(c.z == 9.0f);
    }
    
    SECTION("Subtraction") {
        Vec3 a(5.0f, 7.0f, 9.0f);
        Vec3 b(2.0f, 3.0f, 4.0f);
        Vec3 c = a - b;
        REQUIRE(c.x == 3.0f);
        REQUIRE(c.y == 4.0f);
        REQUIRE(c.z == 5.0f);
    }
    
    SECTION("Multiplication") {
        Vec3 v(2.0f, 3.0f, 4.0f);
        Vec3 scaled = v * 2.0f;
        REQUIRE(scaled.x == 4.0f);
        REQUIRE(scaled.y == 6.0f);
        REQUIRE(scaled.z == 8.0f);
    }
    
    SECTION("Division") {
        Vec3 v(4.0f, 6.0f, 8.0f);
        Vec3 divided = v / 2.0f;
        REQUIRE(divided.x == 2.0f);
        REQUIRE(divided.y == 3.0f);
        REQUIRE(divided.z == 4.0f);
    }
    
    SECTION("Negation") {
        Vec3 v(1.0f, -2.0f, 3.0f);
        Vec3 n = -v;
        REQUIRE(n.x == -1.0f);
        REQUIRE(n.y == 2.0f);
        REQUIRE(n.z == -3.0f);
    }
}

TEST_CASE("Vec3 dot and cross products", "[math][vec3]") {
    SECTION("Dot product - orthogonal vectors") {
        Vec3 a(1.0f, 0.0f, 0.0f);
        Vec3 b(0.0f, 1.0f, 0.0f);
        REQUIRE(a.dot(b) == 0.0f);
    }
    
    SECTION("Dot product - parallel vectors") {
        Vec3 a(1.0f, 0.0f, 0.0f);
        Vec3 b(2.0f, 0.0f, 0.0f);
        REQUIRE(a.dot(b) == 2.0f);
    }
    
    SECTION("Dot product - general case") {
        Vec3 a(1.0f, 2.0f, 3.0f);
        Vec3 b(4.0f, 5.0f, 6.0f);
        REQUIRE(a.dot(b) == 32.0f); // 1*4 + 2*5 + 3*6 = 32
    }
    
    SECTION("Cross product - standard basis") {
        Vec3 x = Vec3::unitX();
        Vec3 y = Vec3::unitY();
        Vec3 z = Vec3::unitZ();
        
        // X x Y = Z
        Vec3 xy = x.cross(y);
        REQUIRE(xy.x == Approx(0.0f));
        REQUIRE(xy.y == Approx(0.0f));
        REQUIRE(xy.z == Approx(1.0f));
        
        // Y x Z = X
        Vec3 yz = y.cross(z);
        REQUIRE(yz.x == Approx(1.0f));
        REQUIRE(yz.y == Approx(0.0f));
        REQUIRE(yz.z == Approx(0.0f));
        
        // Z x X = Y
        Vec3 zx = z.cross(x);
        REQUIRE(zx.x == Approx(0.0f));
        REQUIRE(zx.y == Approx(1.0f));
        REQUIRE(zx.z == Approx(0.0f));
    }
    
    SECTION("Cross product - anti-commutative") {
        Vec3 a(1.0f, 2.0f, 3.0f);
        Vec3 b(4.0f, 5.0f, 6.0f);
        Vec3 ab = a.cross(b);
        Vec3 ba = b.cross(a);
        
        REQUIRE(ab.x == Approx(-ba.x));
        REQUIRE(ab.y == Approx(-ba.y));
        REQUIRE(ab.z == Approx(-ba.z));
    }
}

TEST_CASE("Vec3 length and normalization", "[math][vec3]") {
    SECTION("Length") {
        Vec3 v(3.0f, 4.0f, 0.0f);
        REQUIRE(v.length() == Approx(5.0f));
        REQUIRE(v.lengthSquared() == 25.0f);
    }
    
    SECTION("Length 3D") {
        Vec3 v(1.0f, 2.0f, 2.0f);
        REQUIRE(v.length() == Approx(3.0f));
    }
    
    SECTION("Normalize") {
        Vec3 v(3.0f, 4.0f, 0.0f);
        Vec3 n = v.normalized();
        REQUIRE(n.length() == Approx(1.0f));
        REQUIRE(n.x == Approx(0.6f));
        REQUIRE(n.y == Approx(0.8f));
        REQUIRE(n.z == Approx(0.0f));
    }
    
    SECTION("Zero vector normalize safety") {
        Vec3 v = Vec3::zero();
        Vec3 n = v.normalized();
        // Should return zero vector or unit vector, not NaN
        REQUIRE(!std::isnan(n.x));
        REQUIRE(!std::isnan(n.y));
        REQUIRE(!std::isnan(n.z));
    }
}

TEST_CASE("Vec3 reflection and projection", "[math][vec3]") {
    SECTION("Reflect horizontal") {
        Vec3 incoming(1.0f, -1.0f, 0.0f);
        Vec3 normal(0.0f, 1.0f, 0.0f);
        Vec3 reflected = incoming.reflect(normal);
        
        REQUIRE(reflected.x == Approx(1.0f));
        REQUIRE(reflected.y == Approx(1.0f));
        REQUIRE(reflected.z == Approx(0.0f));
    }
}

TEST_CASE("Vec3 linear interpolation", "[math][vec3]") {
    SECTION("Lerp at t=0") {
        Vec3 a(0.0f, 0.0f, 0.0f);
        Vec3 b(10.0f, 20.0f, 30.0f);
        Vec3 result = a.lerp(b, 0.0f);
        REQUIRE(result == a);
    }
    
    SECTION("Lerp at t=1") {
        Vec3 a(0.0f, 0.0f, 0.0f);
        Vec3 b(10.0f, 20.0f, 30.0f);
        Vec3 result = a.lerp(b, 1.0f);
        REQUIRE(result.x == Approx(b.x));
        REQUIRE(result.y == Approx(b.y));
        REQUIRE(result.z == Approx(b.z));
    }
    
    SECTION("Lerp at t=0.5") {
        Vec3 a(0.0f, 0.0f, 0.0f);
        Vec3 b(10.0f, 20.0f, 30.0f);
        Vec3 result = a.lerp(b, 0.5f);
        REQUIRE(result.x == Approx(5.0f));
        REQUIRE(result.y == Approx(10.0f));
        REQUIRE(result.z == Approx(15.0f));
    }
}

// =============================================================================
// Vec4 Tests
// =============================================================================

TEST_CASE("Vec4 basic operations", "[math][vec4]") {
    SECTION("Default constructor") {
        Vec4 v;
        REQUIRE(v.x == 0.0f);
        REQUIRE(v.y == 0.0f);
        REQUIRE(v.z == 0.0f);
        REQUIRE(v.w == 0.0f);
    }
    
    SECTION("Component constructor") {
        Vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
        REQUIRE(v.x == 1.0f);
        REQUIRE(v.y == 2.0f);
        REQUIRE(v.z == 3.0f);
        REQUIRE(v.w == 4.0f);
    }
    
    SECTION("From Vec3 with w") {
        Vec3 v3(1.0f, 2.0f, 3.0f);
        Vec4 v4(v3, 1.0f);
        REQUIRE(v4.x == 1.0f);
        REQUIRE(v4.y == 2.0f);
        REQUIRE(v4.z == 3.0f);
        REQUIRE(v4.w == 1.0f);
    }
    
    SECTION("Addition") {
        Vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
        Vec4 b(5.0f, 6.0f, 7.0f, 8.0f);
        Vec4 c = a + b;
        REQUIRE(c.x == 6.0f);
        REQUIRE(c.y == 8.0f);
        REQUIRE(c.z == 10.0f);
        REQUIRE(c.w == 12.0f);
    }
    
    SECTION("Dot product") {
        Vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
        Vec4 b(2.0f, 3.0f, 4.0f, 5.0f);
        // 1*2 + 2*3 + 3*4 + 4*5 = 2 + 6 + 12 + 20 = 40
        REQUIRE(a.dot(b) == 40.0f);
    }
}

// =============================================================================
// Mat3 Tests
// =============================================================================

TEST_CASE("Mat3 basic operations", "[math][mat3]") {
    SECTION("Identity matrix") {
        Mat3 m = Mat3::identity();
        REQUIRE(m.columns[0].x == 1.0f);
        REQUIRE(m.columns[1].y == 1.0f);
        REQUIRE(m.columns[2].z == 1.0f);
        REQUIRE(m.columns[0].y == 0.0f);
    }
    
    SECTION("Matrix-vector multiplication with identity") {
        Mat3 m = Mat3::identity();
        Vec3 v(1.0f, 2.0f, 3.0f);
        Vec3 result = m * v;
        REQUIRE(result.x == Approx(v.x));
        REQUIRE(result.y == Approx(v.y));
        REQUIRE(result.z == Approx(v.z));
    }
    
    SECTION("Scale matrix") {
        Mat3 m = Mat3::scale(Vec3(2.0f, 3.0f, 4.0f));
        Vec3 v(1.0f, 1.0f, 1.0f);
        Vec3 result = m * v;
        REQUIRE(result.x == Approx(2.0f));
        REQUIRE(result.y == Approx(3.0f));
        REQUIRE(result.z == Approx(4.0f));
    }
    
    SECTION("Transpose") {
        Mat3 m;
        m.columns[0] = Vec3(1.0f, 2.0f, 3.0f);
        m.columns[1] = Vec3(4.0f, 5.0f, 6.0f);
        m.columns[2] = Vec3(7.0f, 8.0f, 9.0f);
        
        Mat3 t = m.transposed();
        REQUIRE(t.columns[0].x == 1.0f);
        REQUIRE(t.columns[0].y == 4.0f);
        REQUIRE(t.columns[0].z == 7.0f);
        REQUIRE(t.columns[1].x == 2.0f);
        REQUIRE(t.columns[1].y == 5.0f);
    }
}

// =============================================================================
// Mat4 Tests
// =============================================================================

TEST_CASE("Mat4 basic operations", "[math][mat4]") {
    SECTION("Identity matrix") {
        Mat4 m = Mat4::identity();
        REQUIRE(m.columns[0].x == 1.0f);
        REQUIRE(m.columns[1].y == 1.0f);
        REQUIRE(m.columns[2].z == 1.0f);
        REQUIRE(m.columns[3].w == 1.0f);
        REQUIRE(m.columns[0].y == 0.0f);
    }
    
    SECTION("Translation matrix") {
        Mat4 m = Mat4::translate(Vec3(1.0f, 2.0f, 3.0f));
        Vec3 t = m.translation();
        REQUIRE(t.x == Approx(1.0f));
        REQUIRE(t.y == Approx(2.0f));
        REQUIRE(t.z == Approx(3.0f));
    }
    
    SECTION("Scale matrix") {
        Mat4 m = Mat4::scale(Vec3(2.0f, 3.0f, 4.0f));
        Vec4 v(1.0f, 1.0f, 1.0f, 1.0f);
        Vec4 result = m * v;
        REQUIRE(result.x == Approx(2.0f));
        REQUIRE(result.y == Approx(3.0f));
        REQUIRE(result.z == Approx(4.0f));
    }
    
    SECTION("Matrix multiplication with identity") {
        Mat4 a = Mat4::translate(Vec3(1.0f, 2.0f, 3.0f));
        Mat4 b = Mat4::identity();
        Mat4 c = a * b;
        REQUIRE(c.columns[3].x == Approx(1.0f));
        REQUIRE(c.columns[3].y == Approx(2.0f));
        REQUIRE(c.columns[3].z == Approx(3.0f));
    }
    
    SECTION("Transform point") {
        Mat4 m = Mat4::translate(Vec3(10.0f, 20.0f, 30.0f));
        Vec3 p(1.0f, 2.0f, 3.0f);
        Vec3 transformed = m.transformPoint(p);
        REQUIRE(transformed.x == Approx(11.0f));
        REQUIRE(transformed.y == Approx(22.0f));
        REQUIRE(transformed.z == Approx(33.0f));
    }
    
    SECTION("Transform direction ignores translation") {
        Mat4 m = Mat4::translate(Vec3(10.0f, 20.0f, 30.0f));
        Vec3 d(1.0f, 0.0f, 0.0f);
        Vec3 transformed = m.transformDirection(d);
        REQUIRE(transformed.x == Approx(1.0f));
        REQUIRE(transformed.y == Approx(0.0f));
        REQUIRE(transformed.z == Approx(0.0f));
    }
}

TEST_CASE("Mat4 rotation", "[math][mat4]") {
    SECTION("Rotate around X axis 90 degrees") {
        f32 angle = PI_F32 / 2.0f; // 90 degrees
        Mat4 m = Mat4::rotateX(angle);
        Vec3 v(0.0f, 1.0f, 0.0f); // Up vector
        Vec3 result = m.transformDirection(v);
        
        // After rotating 90 degrees around X, (0,1,0) becomes (0,0,1)
        REQUIRE(result.x == Approx(0.0f).margin(0.001f));
        REQUIRE(result.y == Approx(0.0f).margin(0.001f));
        REQUIRE(result.z == Approx(1.0f).margin(0.001f));
    }
    
    SECTION("Rotate around Y axis 90 degrees") {
        f32 angle = PI_F32 / 2.0f;
        Mat4 m = Mat4::rotateY(angle);
        Vec3 v(1.0f, 0.0f, 0.0f); // Right vector
        Vec3 result = m.transformDirection(v);
        
        // After rotating 90 degrees around Y, (1,0,0) becomes (0,0,-1)
        REQUIRE(result.x == Approx(0.0f).margin(0.001f));
        REQUIRE(result.y == Approx(0.0f).margin(0.001f));
        REQUIRE(result.z == Approx(-1.0f).margin(0.001f));
    }
}

TEST_CASE("Mat4 inverse", "[math][mat4]") {
    SECTION("Inverse of identity is identity") {
        Mat4 m = Mat4::identity();
        Mat4 inv = m.inverse();
        REQUIRE(inv.columns[0].x == Approx(1.0f));
        REQUIRE(inv.columns[1].y == Approx(1.0f));
        REQUIRE(inv.columns[2].z == Approx(1.0f));
        REQUIRE(inv.columns[3].w == Approx(1.0f));
    }
    
    SECTION("Inverse of translation") {
        Mat4 m = Mat4::translate(Vec3(1.0f, 2.0f, 3.0f));
        Mat4 inv = m.inverse();
        Mat4 result = m * inv;
        
        // Should be close to identity
        REQUIRE(result.columns[0].x == Approx(1.0f).margin(0.001f));
        REQUIRE(result.columns[1].y == Approx(1.0f).margin(0.001f));
        REQUIRE(result.columns[2].z == Approx(1.0f).margin(0.001f));
        REQUIRE(result.columns[3].w == Approx(1.0f).margin(0.001f));
    }
    
    SECTION("Inverse of scale") {
        Mat4 m = Mat4::scale(Vec3(2.0f, 4.0f, 8.0f));
        Mat4 inv = m.inverse();
        Mat4 result = m * inv;
        
        REQUIRE(result.columns[0].x == Approx(1.0f).margin(0.001f));
        REQUIRE(result.columns[1].y == Approx(1.0f).margin(0.001f));
        REQUIRE(result.columns[2].z == Approx(1.0f).margin(0.001f));
    }
}

// =============================================================================
// Quaternion Tests
// =============================================================================

TEST_CASE("Quaternion basic operations", "[math][quat]") {
    SECTION("Default constructor is identity") {
        Quat q;
        REQUIRE(q.x == 0.0f);
        REQUIRE(q.y == 0.0f);
        REQUIRE(q.z == 0.0f);
        REQUIRE(q.w == 1.0f);
    }
    
    SECTION("Identity quaternion") {
        Quat q = Quat::identity();
        REQUIRE(q.x == 0.0f);
        REQUIRE(q.y == 0.0f);
        REQUIRE(q.z == 0.0f);
        REQUIRE(q.w == 1.0f);
    }
    
    SECTION("From axis-angle") {
        Vec3 axis(0.0f, 1.0f, 0.0f);
        f32 angle = PI_F32 / 2.0f; // 90 degrees around Y
        Quat q = Quat::fromAxisAngle(axis, angle);
        
        // Should rotate a vector
        Vec3 v(1.0f, 0.0f, 0.0f);
        Vec3 rotated = q * v;
        
        REQUIRE(rotated.x == Approx(0.0f).margin(0.001f));
        REQUIRE(rotated.y == Approx(0.0f).margin(0.001f));
        REQUIRE(rotated.z == Approx(-1.0f).margin(0.001f));
    }
    
    SECTION("Conjugate") {
        Quat q(1.0f, 2.0f, 3.0f, 4.0f);
        Quat c = q.conjugate();
        REQUIRE(c.x == -1.0f);
        REQUIRE(c.y == -2.0f);
        REQUIRE(c.z == -3.0f);
        REQUIRE(c.w == 4.0f);
    }
    
    SECTION("Normalized quaternion has unit length") {
        Quat q(1.0f, 2.0f, 3.0f, 4.0f);
        Quat n = q.normalized();
        f32 length = std::sqrt(n.x*n.x + n.y*n.y + n.z*n.z + n.w*n.w);
        REQUIRE(length == Approx(1.0f).margin(0.001f));
    }
}

TEST_CASE("Quaternion rotation", "[math][quat]") {
    SECTION("Rotate vector around Y axis") {
        Quat q = Quat::fromAxisAngle(Vec3::up(), PI_F32 / 2.0f);
        Vec3 v = Vec3::right();
        Vec3 rotated = q * v;
        
        // Right vector rotated 90 degrees around Y becomes -Z
        REQUIRE(rotated.x == Approx(0.0f).margin(0.001f));
        REQUIRE(rotated.y == Approx(0.0f).margin(0.001f));
        REQUIRE(rotated.z == Approx(-1.0f).margin(0.001f));
    }
    
    SECTION("Rotate vector around X axis") {
        Quat q = Quat::fromAxisAngle(Vec3::right(), PI_F32 / 2.0f);
        Vec3 v = Vec3::up();
        Vec3 rotated = q * v;
        
        // Up vector rotated 90 degrees around X becomes +Z
        REQUIRE(rotated.x == Approx(0.0f).margin(0.001f));
        REQUIRE(rotated.y == Approx(0.0f).margin(0.001f));
        REQUIRE(rotated.z == Approx(1.0f).margin(0.001f));
    }
    
    SECTION("Identity rotation doesn't change vector") {
        Quat q = Quat::identity();
        Vec3 v(1.0f, 2.0f, 3.0f);
        Vec3 rotated = q * v;
        
        REQUIRE(rotated.x == Approx(v.x));
        REQUIRE(rotated.y == Approx(v.y));
        REQUIRE(rotated.z == Approx(v.z));
    }
}

TEST_CASE("Quaternion interpolation", "[math][quat]") {
    SECTION("Slerp at t=0") {
        Quat a = Quat::identity();
        Quat b = Quat::fromAxisAngle(Vec3::up(), PI_F32);
        Quat result = a.slerp(b, 0.0f);
        
        REQUIRE(result.x == Approx(a.x).margin(0.001f));
        REQUIRE(result.y == Approx(a.y).margin(0.001f));
        REQUIRE(result.z == Approx(a.z).margin(0.001f));
        REQUIRE(result.w == Approx(a.w).margin(0.001f));
    }
    
    SECTION("Slerp at t=1") {
        Quat a = Quat::identity();
        Quat b = Quat::fromAxisAngle(Vec3::up(), PI_F32 / 2.0f);
        Quat result = a.slerp(b, 1.0f);
        
        // Result should be equivalent to b (may have different sign)
        Vec3 testVec(1.0f, 0.0f, 0.0f);
        Vec3 fromB = b * testVec;
        Vec3 fromResult = result * testVec;
        
        REQUIRE(fromResult.x == Approx(fromB.x).margin(0.01f));
        REQUIRE(fromResult.y == Approx(fromB.y).margin(0.01f));
        REQUIRE(fromResult.z == Approx(fromB.z).margin(0.01f));
    }
    
    SECTION("Slerp at t=0.5 gives halfway rotation") {
        Quat a = Quat::identity();
        Quat b = Quat::fromAxisAngle(Vec3::up(), PI_F32 / 2.0f); // 90 degrees
        Quat result = a.slerp(b, 0.5f);
        
        // Should be 45 degrees rotation
        Vec3 v(1.0f, 0.0f, 0.0f);
        Vec3 rotated = result * v;
        
        // At 45 degrees, x and z should be approximately equal magnitude
        f32 expected = std::sqrt(2.0f) / 2.0f;
        REQUIRE(rotated.x == Approx(expected).margin(0.01f));
        REQUIRE(rotated.y == Approx(0.0f).margin(0.01f));
        REQUIRE(rotated.z == Approx(-expected).margin(0.01f));
    }
}

TEST_CASE("Quaternion to/from matrix", "[math][quat]") {
    SECTION("To Mat4 and back via Mat3") {
        Quat original = Quat::fromAxisAngle(Vec3(1.0f, 1.0f, 1.0f).normalized(), PI_F32 / 3.0f);
        Mat3 mat = original.toMat3();
        Quat recovered = Quat::fromMatrix(mat);
        
        // Compare by rotating a test vector
        Vec3 testVec(1.0f, 2.0f, 3.0f);
        Vec3 fromOriginal = original * testVec;
        Vec3 fromRecovered = recovered * testVec;
        
        REQUIRE(fromRecovered.x == Approx(fromOriginal.x).margin(0.01f));
        REQUIRE(fromRecovered.y == Approx(fromOriginal.y).margin(0.01f));
        REQUIRE(fromRecovered.z == Approx(fromOriginal.z).margin(0.01f));
    }
}

// =============================================================================
// Transform Tests
// =============================================================================

TEST_CASE("Transform basic operations", "[math][transform]") {
    SECTION("Default transform is identity") {
        Transform t;
        REQUIRE(t.position == Vec3::zero());
        REQUIRE(t.rotation.w == Approx(1.0f));
        REQUIRE(t.scale == Vec3::one());
    }
    
    SECTION("Transform from position") {
        Transform t = Transform::fromPosition(Vec3(1.0f, 2.0f, 3.0f));
        REQUIRE(t.position.x == 1.0f);
        REQUIRE(t.position.y == 2.0f);
        REQUIRE(t.position.z == 3.0f);
        REQUIRE(t.scale == Vec3::one());
    }
    
    SECTION("Transform from scale") {
        Transform t = Transform::fromScale(Vec3(2.0f, 3.0f, 4.0f));
        REQUIRE(t.position == Vec3::zero());
        REQUIRE(t.scale.x == 2.0f);
        REQUIRE(t.scale.y == 3.0f);
        REQUIRE(t.scale.z == 4.0f);
    }
    
    SECTION("Uniform scale") {
        Transform t = Transform::fromScale(5.0f);
        REQUIRE(t.scale.x == 5.0f);
        REQUIRE(t.scale.y == 5.0f);
        REQUIRE(t.scale.z == 5.0f);
    }
}

TEST_CASE("Transform point transformation", "[math][transform]") {
    SECTION("Transform with translation only") {
        Transform t(Vec3(10.0f, 20.0f, 30.0f));
        Vec3 p(1.0f, 2.0f, 3.0f);
        Vec3 result = t.transformPoint(p);
        
        REQUIRE(result.x == Approx(11.0f));
        REQUIRE(result.y == Approx(22.0f));
        REQUIRE(result.z == Approx(33.0f));
    }
    
    SECTION("Transform with scale") {
        Transform t;
        t.scale = Vec3(2.0f, 2.0f, 2.0f);
        Vec3 p(1.0f, 2.0f, 3.0f);
        Vec3 result = t.transformPoint(p);
        
        REQUIRE(result.x == Approx(2.0f));
        REQUIRE(result.y == Approx(4.0f));
        REQUIRE(result.z == Approx(6.0f));
    }
    
    SECTION("Transform direction ignores position and scale") {
        Transform t(Vec3(100.0f, 100.0f, 100.0f));
        t.scale = Vec3(5.0f, 5.0f, 5.0f);
        
        Vec3 d(1.0f, 0.0f, 0.0f);
        Vec3 result = t.transformDirection(d);
        
        // Direction should not be affected by position or scale
        REQUIRE(result.x == Approx(1.0f));
        REQUIRE(result.y == Approx(0.0f));
        REQUIRE(result.z == Approx(0.0f));
    }
}

TEST_CASE("Transform composition", "[math][transform]") {
    SECTION("Compose two translations") {
        Transform a(Vec3(1.0f, 0.0f, 0.0f));
        Transform b(Vec3(0.0f, 2.0f, 0.0f));
        Transform c = a * b;
        
        Vec3 p = Vec3::zero();
        Vec3 result = c.transformPoint(p);
        
        REQUIRE(result.x == Approx(1.0f));
        REQUIRE(result.y == Approx(2.0f));
        REQUIRE(result.z == Approx(0.0f));
    }
    
    SECTION("Inverse transform") {
        Transform t(Vec3(10.0f, 20.0f, 30.0f));
        t.scale = Vec3(2.0f, 2.0f, 2.0f);
        
        Transform inv = t.inverse();
        
        Vec3 p(5.0f, 10.0f, 15.0f);
        Vec3 transformed = t.transformPoint(p);
        Vec3 recovered = inv.transformPoint(transformed);
        
        REQUIRE(recovered.x == Approx(p.x).margin(0.01f));
        REQUIRE(recovered.y == Approx(p.y).margin(0.01f));
        REQUIRE(recovered.z == Approx(p.z).margin(0.01f));
    }
}

TEST_CASE("Transform interpolation", "[math][transform]") {
    SECTION("Lerp at t=0") {
        Transform a(Vec3(0.0f, 0.0f, 0.0f));
        Transform b(Vec3(10.0f, 20.0f, 30.0f));
        Transform result = a.lerp(b, 0.0f);
        
        REQUIRE(result.position == a.position);
    }
    
    SECTION("Lerp at t=1") {
        Transform a(Vec3(0.0f, 0.0f, 0.0f));
        Transform b(Vec3(10.0f, 20.0f, 30.0f));
        Transform result = a.lerp(b, 1.0f);
        
        REQUIRE(result.position.x == Approx(b.position.x));
        REQUIRE(result.position.y == Approx(b.position.y));
        REQUIRE(result.position.z == Approx(b.position.z));
    }
    
    SECTION("Lerp at t=0.5") {
        Transform a(Vec3(0.0f, 0.0f, 0.0f));
        Transform b(Vec3(10.0f, 20.0f, 30.0f));
        Transform result = a.lerp(b, 0.5f);
        
        REQUIRE(result.position.x == Approx(5.0f));
        REQUIRE(result.position.y == Approx(10.0f));
        REQUIRE(result.position.z == Approx(15.0f));
    }
}

// =============================================================================
// Math Common Function Tests
// =============================================================================

TEST_CASE("Math common functions", "[math][common]") {
    SECTION("Clamp") {
        REQUIRE(clamp(5.0f, 0.0f, 10.0f) == 5.0f);
        REQUIRE(clamp(-5.0f, 0.0f, 10.0f) == 0.0f);
        REQUIRE(clamp(15.0f, 0.0f, 10.0f) == 10.0f);
    }
    
    SECTION("Lerp") {
        REQUIRE(lerp(0.0f, 10.0f, 0.0f) == 0.0f);
        REQUIRE(lerp(0.0f, 10.0f, 1.0f) == 10.0f);
        REQUIRE(lerp(0.0f, 10.0f, 0.5f) == 5.0f);
    }
    
    SECTION("Smoothstep") {
        REQUIRE(smoothstep(0.0f, 1.0f, -0.5f) == 0.0f);
        REQUIRE(smoothstep(0.0f, 1.0f, 1.5f) == 1.0f);
        REQUIRE(smoothstep(0.0f, 1.0f, 0.5f) == Approx(0.5f));
    }
    
    SECTION("Sign") {
        REQUIRE(sign(5.0f) == 1.0f);
        REQUIRE(sign(-5.0f) == -1.0f);
        REQUIRE(sign(0.0f) == 0.0f);
    }
    
    SECTION("Radians and degrees") {
        REQUIRE(radians(180.0) == Approx(PI_F64));
        REQUIRE(degrees(PI_F64) == Approx(180.0));
        REQUIRE(radians(90.0f) == Approx(PI_F32 / 2.0f));
        REQUIRE(degrees(PI_F32 / 2.0f) == Approx(90.0f));
    }
}

TEST_CASE("Near equality comparisons", "[math][common]") {
    SECTION("nearEqual for floats") {
        REQUIRE(nearEqual(1.0f, 1.0f));
        REQUIRE(nearEqual(1.0f, 1.0000001f));
        REQUIRE_FALSE(nearEqual(1.0f, 1.01f));
    }
    
    SECTION("nearZero") {
        REQUIRE(nearZero(0.0f));
        REQUIRE(nearZero(0.0000001f));
        REQUIRE_FALSE(nearZero(0.01f));
    }
}
