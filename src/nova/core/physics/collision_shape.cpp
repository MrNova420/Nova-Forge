/**
 * @file collision_shape.cpp
 * @brief NovaCore Physics System - Collision Shape Implementations
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include "nova/core/physics/collision_shape.hpp"
#include <cmath>
#include <algorithm>

namespace nova::physics {

// =============================================================================
// CollisionShape Base
// =============================================================================

AABB CollisionShape::getWorldBounds(const Vec3& position, const Quat& orientation) const {
    AABB localBounds = getLocalBounds();
    
    // For axis-aligned shapes with no rotation, just translate
    if (orientation.x == 0.0f && orientation.y == 0.0f && orientation.z == 0.0f) {
        localBounds.min += position;
        localBounds.max += position;
        return localBounds;
    }
    
    // Transform all 8 corners and compute new AABB
    Vec3 corners[8] = {
        Vec3(localBounds.min.x, localBounds.min.y, localBounds.min.z),
        Vec3(localBounds.max.x, localBounds.min.y, localBounds.min.z),
        Vec3(localBounds.min.x, localBounds.max.y, localBounds.min.z),
        Vec3(localBounds.max.x, localBounds.max.y, localBounds.min.z),
        Vec3(localBounds.min.x, localBounds.min.y, localBounds.max.z),
        Vec3(localBounds.max.x, localBounds.min.y, localBounds.max.z),
        Vec3(localBounds.min.x, localBounds.max.y, localBounds.max.z),
        Vec3(localBounds.max.x, localBounds.max.y, localBounds.max.z)
    };
    
    AABB worldBounds;
    worldBounds.min = Vec3(std::numeric_limits<f32>::max());
    worldBounds.max = Vec3(std::numeric_limits<f32>::lowest());
    
    for (const auto& corner : corners) {
        Vec3 worldCorner = orientation * corner + position;
        worldBounds.expandToInclude(worldCorner);
    }
    
    return worldBounds;
}

// =============================================================================
// SphereShape
// =============================================================================

SphereShape::SphereShape(f32 radius)
    : m_radius(radius)
{
}

AABB SphereShape::getLocalBounds() const {
    return AABB::fromCenterExtents(m_localCenter, Vec3(m_radius));
}

MassProperties SphereShape::calculateMassProperties(f32 density) const {
    return MassProperties::sphere(m_radius, density);
}

Vec3 SphereShape::getSupport(const Vec3& direction) const {
    return m_localCenter + direction.normalized() * m_radius;
}

bool SphereShape::raycast(const Ray& ray, RaycastHit& hit) const {
    Vec3 oc = ray.origin - m_localCenter;
    
    f32 a = ray.direction.dot(ray.direction);
    f32 b = 2.0f * oc.dot(ray.direction);
    f32 c = oc.dot(oc) - m_radius * m_radius;
    
    f32 discriminant = b * b - 4.0f * a * c;
    
    if (discriminant < 0.0f) {
        return false;
    }
    
    f32 sqrtD = std::sqrt(discriminant);
    f32 t1 = (-b - sqrtD) / (2.0f * a);
    f32 t2 = (-b + sqrtD) / (2.0f * a);
    
    f32 t = (t1 >= 0.0f) ? t1 : t2;
    
    if (t < 0.0f || t > ray.maxDistance) {
        return false;
    }
    
    hit.hit = true;
    hit.distance = t;
    hit.point = ray.getPoint(t);
    hit.normal = (hit.point - m_localCenter).normalized();
    
    return true;
}

f32 SphereShape::getVolume() const {
    return (4.0f / 3.0f) * PI_F32 * m_radius * m_radius * m_radius;
}

// =============================================================================
// BoxShape
// =============================================================================

BoxShape::BoxShape(const Vec3& halfExtents)
    : m_halfExtents(halfExtents)
{
}

AABB BoxShape::getLocalBounds() const {
    return AABB::fromCenterExtents(m_localCenter, m_halfExtents);
}

MassProperties BoxShape::calculateMassProperties(f32 density) const {
    return MassProperties::box(m_halfExtents, density);
}

Vec3 BoxShape::getSupport(const Vec3& direction) const {
    return m_localCenter + Vec3(
        (direction.x >= 0.0f) ? m_halfExtents.x : -m_halfExtents.x,
        (direction.y >= 0.0f) ? m_halfExtents.y : -m_halfExtents.y,
        (direction.z >= 0.0f) ? m_halfExtents.z : -m_halfExtents.z
    );
}

bool BoxShape::raycast(const Ray& ray, RaycastHit& hit) const {
    Vec3 min = m_localCenter - m_halfExtents;
    Vec3 max = m_localCenter + m_halfExtents;
    
    f32 tmin = 0.0f;
    f32 tmax = ray.maxDistance;
    Vec3 normal = Vec3::zero();
    
    for (int i = 0; i < 3; i++) {
        f32 origin = (i == 0) ? ray.origin.x : (i == 1) ? ray.origin.y : ray.origin.z;
        f32 dir = (i == 0) ? ray.direction.x : (i == 1) ? ray.direction.y : ray.direction.z;
        f32 minVal = (i == 0) ? min.x : (i == 1) ? min.y : min.z;
        f32 maxVal = (i == 0) ? max.x : (i == 1) ? max.y : max.z;
        
        if (std::abs(dir) < PHYSICS_EPSILON) {
            if (origin < minVal || origin > maxVal) {
                return false;
            }
        } else {
            f32 t1 = (minVal - origin) / dir;
            f32 t2 = (maxVal - origin) / dir;
            
            Vec3 n1 = Vec3::zero();
            Vec3 n2 = Vec3::zero();
            if (i == 0) { n1.x = -1; n2.x = 1; }
            else if (i == 1) { n1.y = -1; n2.y = 1; }
            else { n1.z = -1; n2.z = 1; }
            
            if (t1 > t2) {
                std::swap(t1, t2);
                std::swap(n1, n2);
            }
            
            if (t1 > tmin) {
                tmin = t1;
                normal = n1;
            }
            if (t2 < tmax) {
                tmax = t2;
            }
            
            if (tmin > tmax) {
                return false;
            }
        }
    }
    
    if (tmin < 0.0f) {
        return false;
    }
    
    hit.hit = true;
    hit.distance = tmin;
    hit.point = ray.getPoint(tmin);
    hit.normal = normal;
    
    return true;
}

f32 BoxShape::getVolume() const {
    return 8.0f * m_halfExtents.x * m_halfExtents.y * m_halfExtents.z;
}

Vec3 BoxShape::getCorner(u32 index) const {
    return m_localCenter + Vec3(
        ((index & 1) ? m_halfExtents.x : -m_halfExtents.x),
        ((index & 2) ? m_halfExtents.y : -m_halfExtents.y),
        ((index & 4) ? m_halfExtents.z : -m_halfExtents.z)
    );
}

void BoxShape::getCorners(Vec3 corners[8]) const {
    for (u32 i = 0; i < 8; i++) {
        corners[i] = getCorner(i);
    }
}

// =============================================================================
// CapsuleShape
// =============================================================================

CapsuleShape::CapsuleShape(f32 radius, f32 height)
    : m_radius(radius)
    , m_height(height)
{
}

AABB CapsuleShape::getLocalBounds() const {
    f32 halfHeight = m_height * 0.5f + m_radius;
    return AABB::fromCenterExtents(m_localCenter, Vec3(m_radius, halfHeight, m_radius));
}

MassProperties CapsuleShape::calculateMassProperties(f32 density) const {
    return MassProperties::capsule(m_radius, m_height, density);
}

Vec3 CapsuleShape::getSupport(const Vec3& direction) const {
    Vec3 dir = direction.normalized();
    
    // Choose hemisphere based on direction
    Vec3 hemisphereCenter = (dir.y >= 0.0f) ? getTopCenter() : getBottomCenter();
    hemisphereCenter += m_localCenter;
    
    return hemisphereCenter + dir * m_radius;
}

bool CapsuleShape::raycast(const Ray& ray, RaycastHit& hit) const {
    // Test against cylinder portion and two hemispheres
    // Simplified: test against infinite cylinder first, then clamp
    
    Vec3 p1 = m_localCenter + getBottomCenter();
    Vec3 p2 = m_localCenter + getTopCenter();
    Vec3 d = p2 - p1;
    Vec3 m = ray.origin - p1;
    
    f32 md = m.dot(d);
    f32 nd = ray.direction.dot(d);
    f32 dd = d.dot(d);
    
    // Test endcaps (hemispheres)
    if (md < 0.0f && md + nd < 0.0f) {
        // Below bottom cap - test bottom hemisphere
        return sphereRaycast(p1, m_radius, ray, hit);
    }
    if (md > dd && md + nd > dd) {
        // Above top cap - test top hemisphere
        return sphereRaycast(p2, m_radius, ray, hit);
    }
    
    // Test cylinder
    f32 nn = ray.direction.dot(ray.direction);
    f32 mn = m.dot(ray.direction);
    
    f32 a = dd * nn - nd * nd;
    f32 k = m.dot(m) - m_radius * m_radius;
    f32 c = dd * k - md * md;
    
    if (std::abs(a) < PHYSICS_EPSILON) {
        // Ray is parallel to cylinder
        if (c > 0.0f) return false;
        
        if (md < 0.0f) return sphereRaycast(p1, m_radius, ray, hit);
        if (md > dd) return sphereRaycast(p2, m_radius, ray, hit);
        
        hit.hit = true;
        hit.distance = 0.0f;
        return true;
    }
    
    f32 b = dd * mn - nd * md;
    f32 discr = b * b - a * c;
    
    if (discr < 0.0f) return false;
    
    f32 t = (-b - std::sqrt(discr)) / a;
    
    if (t < 0.0f || t > ray.maxDistance) {
        // Try the other intersection
        t = (-b + std::sqrt(discr)) / a;
        if (t < 0.0f || t > ray.maxDistance) return false;
    }
    
    // Check if hit is within cylinder height
    f32 y = md + t * nd;
    
    if (y < 0.0f) {
        return sphereRaycast(p1, m_radius, ray, hit);
    }
    if (y > dd) {
        return sphereRaycast(p2, m_radius, ray, hit);
    }
    
    hit.hit = true;
    hit.distance = t;
    hit.point = ray.getPoint(t);
    
    // Compute normal
    Vec3 axis = d / std::sqrt(dd);
    Vec3 toPoint = hit.point - p1;
    Vec3 projected = p1 + axis * toPoint.dot(axis);
    hit.normal = (hit.point - projected).normalized();
    
    return true;
}

bool CapsuleShape::sphereRaycast(const Vec3& center, f32 radius, const Ray& ray, RaycastHit& hit) const {
    Vec3 oc = ray.origin - center;
    
    f32 a = ray.direction.dot(ray.direction);
    f32 b = 2.0f * oc.dot(ray.direction);
    f32 c = oc.dot(oc) - radius * radius;
    
    f32 discriminant = b * b - 4.0f * a * c;
    
    if (discriminant < 0.0f) return false;
    
    f32 t = (-b - std::sqrt(discriminant)) / (2.0f * a);
    
    if (t < 0.0f) t = (-b + std::sqrt(discriminant)) / (2.0f * a);
    if (t < 0.0f || t > ray.maxDistance) return false;
    
    hit.hit = true;
    hit.distance = t;
    hit.point = ray.getPoint(t);
    hit.normal = (hit.point - center).normalized();
    
    return true;
}

f32 CapsuleShape::getVolume() const {
    f32 cylVolume = PI_F32 * m_radius * m_radius * m_height;
    f32 sphereVolume = (4.0f / 3.0f) * PI_F32 * m_radius * m_radius * m_radius;
    return cylVolume + sphereVolume;
}

// =============================================================================
// CylinderShape
// =============================================================================

CylinderShape::CylinderShape(f32 radius, f32 height)
    : m_radius(radius)
    , m_height(height)
{
}

AABB CylinderShape::getLocalBounds() const {
    return AABB::fromCenterExtents(m_localCenter, Vec3(m_radius, m_height * 0.5f, m_radius));
}

MassProperties CylinderShape::calculateMassProperties(f32 density) const {
    f32 volume = PI_F32 * m_radius * m_radius * m_height;
    f32 mass = volume * density;
    
    f32 iy = 0.5f * mass * m_radius * m_radius;
    f32 ixz = (mass / 12.0f) * (3.0f * m_radius * m_radius + m_height * m_height);
    
    MassProperties props;
    props.mass = mass;
    props.inverseMass = 1.0f / mass;
    props.inertia = Vec3(ixz, iy, ixz);
    props.inverseInertia = Vec3(1.0f / ixz, 1.0f / iy, 1.0f / ixz);
    return props;
}

Vec3 CylinderShape::getSupport(const Vec3& direction) const {
    Vec3 dir = direction;
    
    // Project direction onto XZ plane for radial component
    f32 radialLen = std::sqrt(dir.x * dir.x + dir.z * dir.z);
    
    Vec3 result = m_localCenter;
    
    if (radialLen > PHYSICS_EPSILON) {
        result.x += (dir.x / radialLen) * m_radius;
        result.z += (dir.z / radialLen) * m_radius;
    }
    
    // Vertical component
    result.y += (dir.y >= 0.0f) ? (m_height * 0.5f) : (-m_height * 0.5f);
    
    return result;
}

bool CylinderShape::raycast(const Ray& ray, RaycastHit& hit) const {
    // Similar to capsule but with flat caps
    f32 halfHeight = m_height * 0.5f;
    
    // Test caps first
    if (std::abs(ray.direction.y) > PHYSICS_EPSILON) {
        // Top cap
        f32 t = (m_localCenter.y + halfHeight - ray.origin.y) / ray.direction.y;
        if (t >= 0.0f && t <= ray.maxDistance) {
            Vec3 p = ray.getPoint(t);
            f32 distSq = (p.x - m_localCenter.x) * (p.x - m_localCenter.x) +
                        (p.z - m_localCenter.z) * (p.z - m_localCenter.z);
            if (distSq <= m_radius * m_radius) {
                hit.hit = true;
                hit.distance = t;
                hit.point = p;
                hit.normal = Vec3::up();
                return true;
            }
        }
        
        // Bottom cap
        t = (m_localCenter.y - halfHeight - ray.origin.y) / ray.direction.y;
        if (t >= 0.0f && t <= ray.maxDistance) {
            Vec3 p = ray.getPoint(t);
            f32 distSq = (p.x - m_localCenter.x) * (p.x - m_localCenter.x) +
                        (p.z - m_localCenter.z) * (p.z - m_localCenter.z);
            if (distSq <= m_radius * m_radius) {
                hit.hit = true;
                hit.distance = t;
                hit.point = p;
                hit.normal = Vec3::down();
                return true;
            }
        }
    }
    
    // Test cylinder side
    f32 a = ray.direction.x * ray.direction.x + ray.direction.z * ray.direction.z;
    if (a < PHYSICS_EPSILON) return false;
    
    f32 dx = ray.origin.x - m_localCenter.x;
    f32 dz = ray.origin.z - m_localCenter.z;
    
    f32 b = 2.0f * (dx * ray.direction.x + dz * ray.direction.z);
    f32 c = dx * dx + dz * dz - m_radius * m_radius;
    
    f32 discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f) return false;
    
    f32 t = (-b - std::sqrt(discriminant)) / (2.0f * a);
    if (t < 0.0f) {
        t = (-b + std::sqrt(discriminant)) / (2.0f * a);
    }
    
    if (t < 0.0f || t > ray.maxDistance) return false;
    
    Vec3 p = ray.getPoint(t);
    if (p.y < m_localCenter.y - halfHeight || p.y > m_localCenter.y + halfHeight) {
        return false;
    }
    
    hit.hit = true;
    hit.distance = t;
    hit.point = p;
    hit.normal = Vec3(p.x - m_localCenter.x, 0.0f, p.z - m_localCenter.z).normalized();
    
    return true;
}

f32 CylinderShape::getVolume() const {
    return PI_F32 * m_radius * m_radius * m_height;
}

// =============================================================================
// PlaneShape
// =============================================================================

PlaneShape::PlaneShape(const Vec3& normal, f32 distance)
    : m_normal(normal.normalized())
    , m_distance(distance)
{
}

AABB PlaneShape::getLocalBounds() const {
    // Infinite plane - return very large but finite bounds
    constexpr f32 LARGE = 1e6f;
    return AABB::fromMinMax(Vec3(-LARGE, -LARGE, -LARGE), Vec3(LARGE, LARGE, LARGE));
}

MassProperties PlaneShape::calculateMassProperties(f32 /*density*/) const {
    // Infinite mass for planes
    return MassProperties::infinite();
}

Vec3 PlaneShape::getSupport(const Vec3& direction) const {
    // Planes don't have a meaningful support function
    // Return a point on the plane
    return m_normal * m_distance;
}

bool PlaneShape::raycast(const Ray& ray, RaycastHit& hit) const {
    f32 denom = ray.direction.dot(m_normal);
    
    if (std::abs(denom) < PHYSICS_EPSILON) {
        return false; // Ray is parallel to plane
    }
    
    f32 t = (m_distance - ray.origin.dot(m_normal)) / denom;
    
    if (t < 0.0f || t > ray.maxDistance) {
        return false;
    }
    
    hit.hit = true;
    hit.distance = t;
    hit.point = ray.getPoint(t);
    hit.normal = (denom < 0.0f) ? m_normal : -m_normal;
    
    return true;
}

f32 PlaneShape::signedDistance(const Vec3& point) const {
    return point.dot(m_normal) - m_distance;
}

// =============================================================================
// ConvexHullShape
// =============================================================================

ConvexHullShape::ConvexHullShape(const Vec3* points, u32 pointCount) {
    buildHull(points, pointCount);
}

ConvexHullShape::ConvexHullShape(const std::vector<Vec3>& points) {
    if (!points.empty()) {
        buildHull(points.data(), static_cast<u32>(points.size()));
    }
}

void ConvexHullShape::buildHull(const Vec3* points, u32 pointCount) {
    if (pointCount < 4) {
        // Not enough points for a hull, just store them
        m_vertices.assign(points, points + pointCount);
        if (pointCount > 0) {
            m_localBounds.min = m_vertices[0];
            m_localBounds.max = m_vertices[0];
            for (u32 i = 1; i < pointCount; i++) {
                m_localBounds.expandToInclude(m_vertices[i]);
            }
        }
        return;
    }
    
    // Simplified hull construction - just use the input points
    // A full implementation would use Quickhull algorithm
    m_vertices.assign(points, points + pointCount);
    
    // Compute bounds
    m_localBounds.min = m_vertices[0];
    m_localBounds.max = m_vertices[0];
    for (const auto& v : m_vertices) {
        m_localBounds.expandToInclude(v);
    }
    
    // Compute approximate volume using bounding box
    Vec3 size = m_localBounds.getSize();
    m_volume = size.x * size.y * size.z * 0.5f; // Rough approximation
}

AABB ConvexHullShape::getLocalBounds() const {
    return m_localBounds;
}

MassProperties ConvexHullShape::calculateMassProperties(f32 density) const {
    // Approximate using bounding box
    Vec3 extents = m_localBounds.getExtents();
    return MassProperties::box(extents, density);
}

Vec3 ConvexHullShape::getSupport(const Vec3& direction) const {
    if (m_vertices.empty()) {
        return Vec3::zero();
    }
    
    Vec3 best = m_vertices[0];
    f32 bestDot = best.dot(direction);
    
    for (size_t i = 1; i < m_vertices.size(); i++) {
        f32 dot = m_vertices[i].dot(direction);
        if (dot > bestDot) {
            bestDot = dot;
            best = m_vertices[i];
        }
    }
    
    return best;
}

bool ConvexHullShape::raycast(const Ray& ray, RaycastHit& hit) const {
    // Use SAT-based raycast against convex hull
    // Simplified: test against bounding box first
    AABB bounds = getLocalBounds();
    
    Vec3 tmin = (bounds.min - ray.origin) / ray.direction;
    Vec3 tmax = (bounds.max - ray.origin) / ray.direction;
    
    Vec3 t1 = Vec3(
        std::min(tmin.x, tmax.x),
        std::min(tmin.y, tmax.y),
        std::min(tmin.z, tmax.z)
    );
    Vec3 t2 = Vec3(
        std::max(tmin.x, tmax.x),
        std::max(tmin.y, tmax.y),
        std::max(tmin.z, tmax.z)
    );
    
    f32 near = std::max({t1.x, t1.y, t1.z});
    f32 far = std::min({t2.x, t2.y, t2.z});
    
    if (near > far || far < 0.0f || near > ray.maxDistance) {
        return false;
    }
    
    hit.hit = true;
    hit.distance = near > 0.0f ? near : far;
    hit.point = ray.getPoint(hit.distance);
    
    // Approximate normal
    if (near == t1.x) hit.normal = ray.direction.x < 0 ? Vec3::right() : Vec3::left();
    else if (near == t1.y) hit.normal = ray.direction.y < 0 ? Vec3::up() : Vec3::down();
    else hit.normal = ray.direction.z < 0 ? Vec3::back() : Vec3::forward();
    
    return true;
}

f32 ConvexHullShape::getVolume() const {
    return m_volume;
}

// =============================================================================
// CompoundShape
// =============================================================================

AABB CompoundShape::getLocalBounds() const {
    return m_localBounds;
}

MassProperties CompoundShape::calculateMassProperties(f32 density) const {
    if (m_children.empty()) {
        return MassProperties{};
    }
    
    f32 totalMass = 0.0f;
    Vec3 centerOfMass = Vec3::zero();
    
    // First pass: calculate total mass and center of mass
    for (const auto& child : m_children) {
        MassProperties childProps = child.shape->calculateMassProperties(density);
        Vec3 childCenter = child.localRotation * childProps.centerOfMass + child.localPosition;
        
        totalMass += childProps.mass;
        centerOfMass += childCenter * childProps.mass;
    }
    
    if (totalMass > 0.0f) {
        centerOfMass /= totalMass;
    }
    
    // Second pass: calculate inertia tensor using parallel axis theorem
    Vec3 totalInertia = Vec3::zero();
    for (const auto& child : m_children) {
        MassProperties childProps = child.shape->calculateMassProperties(density);
        Vec3 offset = child.localPosition + child.localRotation * childProps.centerOfMass - centerOfMass;
        f32 offsetSq = offset.lengthSquared();
        
        // Parallel axis theorem
        totalInertia += childProps.inertia + Vec3(childProps.mass) * offsetSq;
    }
    
    MassProperties props;
    props.mass = totalMass;
    props.inverseMass = (totalMass > 0.0f) ? (1.0f / totalMass) : 0.0f;
    props.centerOfMass = centerOfMass;
    props.inertia = totalInertia;
    props.inverseInertia = Vec3(
        (totalInertia.x > 0.0f) ? (1.0f / totalInertia.x) : 0.0f,
        (totalInertia.y > 0.0f) ? (1.0f / totalInertia.y) : 0.0f,
        (totalInertia.z > 0.0f) ? (1.0f / totalInertia.z) : 0.0f
    );
    
    return props;
}

Vec3 CompoundShape::getSupport(const Vec3& direction) const {
    if (m_children.empty()) {
        return Vec3::zero();
    }
    
    Vec3 best = Vec3::zero();
    f32 bestDot = std::numeric_limits<f32>::lowest();
    
    for (const auto& child : m_children) {
        // Transform direction to child space
        Vec3 localDir = child.localRotation.inverse() * direction;
        Vec3 localSupport = child.shape->getSupport(localDir);
        
        // Transform back to compound space
        Vec3 worldSupport = child.localRotation * localSupport + child.localPosition;
        
        f32 dot = worldSupport.dot(direction);
        if (dot > bestDot) {
            bestDot = dot;
            best = worldSupport;
        }
    }
    
    return best;
}

bool CompoundShape::raycast(const Ray& ray, RaycastHit& hit) const {
    bool anyHit = false;
    f32 closestDist = ray.maxDistance;
    
    for (u32 i = 0; i < m_children.size(); i++) {
        const auto& child = m_children[i];
        
        // Transform ray to child space
        Vec3 localOrigin = child.localRotation.inverse() * (ray.origin - child.localPosition);
        Vec3 localDir = child.localRotation.inverse() * ray.direction;
        
        Ray localRay;
        localRay.origin = localOrigin;
        localRay.direction = localDir;
        localRay.maxDistance = closestDist;
        
        RaycastHit localHit;
        if (child.shape->raycast(localRay, localHit)) {
            if (localHit.distance < closestDist) {
                closestDist = localHit.distance;
                hit = localHit;
                hit.point = child.localRotation * localHit.point + child.localPosition;
                hit.normal = child.localRotation * localHit.normal;
                hit.shapeIndex = i;
                anyHit = true;
            }
        }
    }
    
    return anyHit;
}

f32 CompoundShape::getVolume() const {
    f32 total = 0.0f;
    for (const auto& child : m_children) {
        total += child.shape->getVolume();
    }
    return total;
}

u32 CompoundShape::addChild(std::shared_ptr<CollisionShape> shape,
                            const Vec3& localPosition,
                            const Quat& localRotation)
{
    CompoundChild child;
    child.shape = std::move(shape);
    child.localPosition = localPosition;
    child.localRotation = localRotation;
    
    m_children.push_back(std::move(child));
    updateBounds();
    
    return static_cast<u32>(m_children.size() - 1);
}

void CompoundShape::removeChild(u32 index) {
    if (index < m_children.size()) {
        m_children.erase(m_children.begin() + index);
        updateBounds();
    }
}

void CompoundShape::updateBounds() {
    if (m_children.empty()) {
        m_localBounds = AABB{};
        return;
    }
    
    m_localBounds.min = Vec3(std::numeric_limits<f32>::max());
    m_localBounds.max = Vec3(std::numeric_limits<f32>::lowest());
    
    for (const auto& child : m_children) {
        AABB childBounds = child.shape->getWorldBounds(child.localPosition, child.localRotation);
        m_localBounds.expandToInclude(childBounds);
    }
}

} // namespace nova::physics
