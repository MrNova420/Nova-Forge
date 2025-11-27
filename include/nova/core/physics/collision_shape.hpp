/**
 * @file collision_shape.hpp
 * @brief NovaCore Physics System - Collision Shapes
 * 
 * Provides all collision shape types for the physics system:
 * - Primitive shapes: Sphere, Box, Capsule, Cylinder, Plane
 * - Complex shapes: ConvexHull, TriangleMesh, HeightField
 * - Compound shapes for complex objects
 * 
 * All shapes are designed for mobile-first performance with:
 * - SIMD-optimized intersection tests
 * - Efficient memory layout
 * - Pre-computed support data
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "physics_types.hpp"
#include <memory>
#include <vector>
#include <string>

namespace nova::physics {

// Forward declarations
class CollisionShape;

/**
 * @brief Handle to a collision shape
 */
using ShapeHandle = u32;
constexpr ShapeHandle INVALID_SHAPE_HANDLE = 0;

/**
 * @brief Base class for all collision shapes
 */
class CollisionShape {
public:
    virtual ~CollisionShape() = default;
    
    /**
     * @brief Get the shape type
     */
    [[nodiscard]] virtual ShapeType getType() const = 0;
    
    /**
     * @brief Get the local AABB of the shape
     */
    [[nodiscard]] virtual AABB getLocalBounds() const = 0;
    
    /**
     * @brief Get the world AABB given a transform
     * @param position World position
     * @param orientation World orientation
     * @return World-space AABB
     */
    [[nodiscard]] virtual AABB getWorldBounds(const Vec3& position, const Quat& orientation) const;
    
    /**
     * @brief Calculate mass properties for this shape
     * @param density Material density in kg/m³
     * @return Mass properties
     */
    [[nodiscard]] virtual MassProperties calculateMassProperties(f32 density) const = 0;
    
    /**
     * @brief Get support point in a given direction (for GJK/EPA)
     * @param direction Direction vector (normalized)
     * @return Furthest point in that direction
     */
    [[nodiscard]] virtual Vec3 getSupport(const Vec3& direction) const = 0;
    
    /**
     * @brief Raycast against this shape in local space
     * @param ray Ray in local space
     * @param hit Output hit information
     * @return true if ray intersects shape
     */
    [[nodiscard]] virtual bool raycast(const Ray& ray, RaycastHit& hit) const = 0;
    
    /**
     * @brief Get the volume of this shape
     */
    [[nodiscard]] virtual f32 getVolume() const = 0;
    
    /**
     * @brief Get debug name for this shape
     */
    [[nodiscard]] const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }
    
    /**
     * @brief Get the local center offset
     */
    [[nodiscard]] const Vec3& getLocalCenter() const { return m_localCenter; }
    void setLocalCenter(const Vec3& center) { m_localCenter = center; }

protected:
    std::string m_name;
    Vec3 m_localCenter = Vec3::zero();
};

/**
 * @brief Sphere collision shape
 */
class SphereShape : public CollisionShape {
public:
    /**
     * @brief Create a sphere shape
     * @param radius Sphere radius
     */
    explicit SphereShape(f32 radius);
    
    [[nodiscard]] ShapeType getType() const override { return ShapeType::Sphere; }
    [[nodiscard]] AABB getLocalBounds() const override;
    [[nodiscard]] MassProperties calculateMassProperties(f32 density) const override;
    [[nodiscard]] Vec3 getSupport(const Vec3& direction) const override;
    [[nodiscard]] bool raycast(const Ray& ray, RaycastHit& hit) const override;
    [[nodiscard]] f32 getVolume() const override;
    
    /**
     * @brief Get the radius
     */
    [[nodiscard]] f32 getRadius() const { return m_radius; }
    
    /**
     * @brief Set the radius
     */
    void setRadius(f32 radius) { m_radius = radius; }

private:
    f32 m_radius;
};

/**
 * @brief Box collision shape (axis-aligned in local space)
 */
class BoxShape : public CollisionShape {
public:
    /**
     * @brief Create a box shape
     * @param halfExtents Half-extents of the box
     */
    explicit BoxShape(const Vec3& halfExtents);
    
    [[nodiscard]] ShapeType getType() const override { return ShapeType::Box; }
    [[nodiscard]] AABB getLocalBounds() const override;
    [[nodiscard]] MassProperties calculateMassProperties(f32 density) const override;
    [[nodiscard]] Vec3 getSupport(const Vec3& direction) const override;
    [[nodiscard]] bool raycast(const Ray& ray, RaycastHit& hit) const override;
    [[nodiscard]] f32 getVolume() const override;
    
    /**
     * @brief Get the half-extents
     */
    [[nodiscard]] const Vec3& getHalfExtents() const { return m_halfExtents; }
    
    /**
     * @brief Set the half-extents
     */
    void setHalfExtents(const Vec3& halfExtents) { m_halfExtents = halfExtents; }
    
    /**
     * @brief Get a corner of the box
     * @param index Corner index (0-7)
     * @return Corner position in local space
     */
    [[nodiscard]] Vec3 getCorner(u32 index) const;
    
    /**
     * @brief Get all 8 corners
     * @param corners Output array of 8 corners
     */
    void getCorners(Vec3 corners[8]) const;

private:
    Vec3 m_halfExtents;
};

/**
 * @brief Capsule collision shape (cylinder with hemispherical caps)
 */
class CapsuleShape : public CollisionShape {
public:
    /**
     * @brief Create a capsule shape
     * @param radius Radius of the capsule
     * @param height Height of the cylindrical portion
     */
    CapsuleShape(f32 radius, f32 height);
    
    [[nodiscard]] ShapeType getType() const override { return ShapeType::Capsule; }
    [[nodiscard]] AABB getLocalBounds() const override;
    [[nodiscard]] MassProperties calculateMassProperties(f32 density) const override;
    [[nodiscard]] Vec3 getSupport(const Vec3& direction) const override;
    [[nodiscard]] bool raycast(const Ray& ray, RaycastHit& hit) const override;
    [[nodiscard]] f32 getVolume() const override;
    
    /**
     * @brief Get the radius
     */
    [[nodiscard]] f32 getRadius() const { return m_radius; }
    
    /**
     * @brief Get the height of cylindrical portion
     */
    [[nodiscard]] f32 getHeight() const { return m_height; }
    
    /**
     * @brief Get the total height including caps
     */
    [[nodiscard]] f32 getTotalHeight() const { return m_height + 2.0f * m_radius; }
    
    /**
     * @brief Get the top hemisphere center
     */
    [[nodiscard]] Vec3 getTopCenter() const { return Vec3(0, m_height * 0.5f, 0); }
    
    /**
     * @brief Get the bottom hemisphere center
     */
    [[nodiscard]] Vec3 getBottomCenter() const { return Vec3(0, -m_height * 0.5f, 0); }

private:
    bool sphereRaycast(const Vec3& center, f32 radius, const Ray& ray, RaycastHit& hit) const;
    
    f32 m_radius;
    f32 m_height;
};

/**
 * @brief Cylinder collision shape
 */
class CylinderShape : public CollisionShape {
public:
    /**
     * @brief Create a cylinder shape
     * @param radius Radius of the cylinder
     * @param height Height of the cylinder
     */
    CylinderShape(f32 radius, f32 height);
    
    [[nodiscard]] ShapeType getType() const override { return ShapeType::Cylinder; }
    [[nodiscard]] AABB getLocalBounds() const override;
    [[nodiscard]] MassProperties calculateMassProperties(f32 density) const override;
    [[nodiscard]] Vec3 getSupport(const Vec3& direction) const override;
    [[nodiscard]] bool raycast(const Ray& ray, RaycastHit& hit) const override;
    [[nodiscard]] f32 getVolume() const override;
    
    [[nodiscard]] f32 getRadius() const { return m_radius; }
    [[nodiscard]] f32 getHeight() const { return m_height; }

private:
    f32 m_radius;
    f32 m_height;
};

/**
 * @brief Infinite plane collision shape
 */
class PlaneShape : public CollisionShape {
public:
    /**
     * @brief Create a plane shape
     * @param normal Plane normal (will be normalized)
     * @param distance Distance from origin along normal
     */
    PlaneShape(const Vec3& normal = Vec3::up(), f32 distance = 0.0f);
    
    [[nodiscard]] ShapeType getType() const override { return ShapeType::Plane; }
    [[nodiscard]] AABB getLocalBounds() const override;
    [[nodiscard]] MassProperties calculateMassProperties(f32 density) const override;
    [[nodiscard]] Vec3 getSupport(const Vec3& direction) const override;
    [[nodiscard]] bool raycast(const Ray& ray, RaycastHit& hit) const override;
    [[nodiscard]] f32 getVolume() const override { return 0.0f; } // Infinite
    
    [[nodiscard]] const Vec3& getNormal() const { return m_normal; }
    [[nodiscard]] f32 getDistance() const { return m_distance; }
    
    /**
     * @brief Get signed distance from a point to the plane
     */
    [[nodiscard]] f32 signedDistance(const Vec3& point) const;

private:
    Vec3 m_normal;
    f32 m_distance;
};

/**
 * @brief Convex hull collision shape
 */
class ConvexHullShape : public CollisionShape {
public:
    /**
     * @brief Create a convex hull from a point cloud
     * @param points Array of points
     * @param pointCount Number of points
     */
    ConvexHullShape(const Vec3* points, u32 pointCount);
    
    /**
     * @brief Create a convex hull from a vector of points
     */
    explicit ConvexHullShape(const std::vector<Vec3>& points);
    
    [[nodiscard]] ShapeType getType() const override { return ShapeType::ConvexHull; }
    [[nodiscard]] AABB getLocalBounds() const override;
    [[nodiscard]] MassProperties calculateMassProperties(f32 density) const override;
    [[nodiscard]] Vec3 getSupport(const Vec3& direction) const override;
    [[nodiscard]] bool raycast(const Ray& ray, RaycastHit& hit) const override;
    [[nodiscard]] f32 getVolume() const override;
    
    /**
     * @brief Get the hull vertices
     */
    [[nodiscard]] const std::vector<Vec3>& getVertices() const { return m_vertices; }
    
    /**
     * @brief Get vertex count
     */
    [[nodiscard]] u32 getVertexCount() const { return static_cast<u32>(m_vertices.size()); }

private:
    void buildHull(const Vec3* points, u32 pointCount);
    
    std::vector<Vec3> m_vertices;
    std::vector<u32> m_faces;  // Triangle indices
    AABB m_localBounds;
    f32 m_volume = 0.0f;
};

/**
 * @brief Sub-shape within a compound shape
 */
struct CompoundChild {
    std::shared_ptr<CollisionShape> shape;
    Vec3 localPosition = Vec3::zero();
    Quat localRotation = Quat::identity();
};

/**
 * @brief Compound collision shape (multiple sub-shapes)
 */
class CompoundShape : public CollisionShape {
public:
    CompoundShape() = default;
    
    [[nodiscard]] ShapeType getType() const override { return ShapeType::Compound; }
    [[nodiscard]] AABB getLocalBounds() const override;
    [[nodiscard]] MassProperties calculateMassProperties(f32 density) const override;
    [[nodiscard]] Vec3 getSupport(const Vec3& direction) const override;
    [[nodiscard]] bool raycast(const Ray& ray, RaycastHit& hit) const override;
    [[nodiscard]] f32 getVolume() const override;
    
    /**
     * @brief Add a child shape
     * @param shape Child shape
     * @param localPosition Local position offset
     * @param localRotation Local rotation offset
     * @return Index of the added child
     */
    u32 addChild(std::shared_ptr<CollisionShape> shape,
                 const Vec3& localPosition = Vec3::zero(),
                 const Quat& localRotation = Quat::identity());
    
    /**
     * @brief Remove a child shape
     * @param index Child index
     */
    void removeChild(u32 index);
    
    /**
     * @brief Get child count
     */
    [[nodiscard]] u32 getChildCount() const { return static_cast<u32>(m_children.size()); }
    
    /**
     * @brief Get a child
     */
    [[nodiscard]] const CompoundChild& getChild(u32 index) const { return m_children[index]; }
    
    /**
     * @brief Update the cached bounds
     */
    void updateBounds();

private:
    std::vector<CompoundChild> m_children;
    AABB m_localBounds;
};

// =============================================================================
// Shape Factory
// =============================================================================

/**
 * @brief Factory for creating collision shapes
 */
namespace ShapeFactory {

/**
 * @brief Create a sphere shape
 */
inline std::shared_ptr<SphereShape> createSphere(f32 radius) {
    return std::make_shared<SphereShape>(radius);
}

/**
 * @brief Create a box shape
 */
inline std::shared_ptr<BoxShape> createBox(const Vec3& halfExtents) {
    return std::make_shared<BoxShape>(halfExtents);
}

/**
 * @brief Create a box shape from full dimensions
 */
inline std::shared_ptr<BoxShape> createBoxFromDimensions(const Vec3& dimensions) {
    return std::make_shared<BoxShape>(dimensions * 0.5f);
}

/**
 * @brief Create a capsule shape
 */
inline std::shared_ptr<CapsuleShape> createCapsule(f32 radius, f32 height) {
    return std::make_shared<CapsuleShape>(radius, height);
}

/**
 * @brief Create a cylinder shape
 */
inline std::shared_ptr<CylinderShape> createCylinder(f32 radius, f32 height) {
    return std::make_shared<CylinderShape>(radius, height);
}

/**
 * @brief Create a plane shape
 */
inline std::shared_ptr<PlaneShape> createPlane(const Vec3& normal = Vec3::up(), f32 distance = 0.0f) {
    return std::make_shared<PlaneShape>(normal, distance);
}

/**
 * @brief Create a convex hull shape
 */
inline std::shared_ptr<ConvexHullShape> createConvexHull(const std::vector<Vec3>& points) {
    return std::make_shared<ConvexHullShape>(points);
}

/**
 * @brief Create a compound shape
 */
inline std::shared_ptr<CompoundShape> createCompound() {
    return std::make_shared<CompoundShape>();
}

} // namespace ShapeFactory

} // namespace nova::physics
