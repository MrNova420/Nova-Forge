/**
 * @file physics_world.cpp
 * @brief NovaCore Physics System - Physics World Implementation
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include "nova/core/physics/physics_world.hpp"
#include <chrono>
#include <algorithm>

namespace nova::physics {

// =============================================================================
// PhysicsWorld Implementation
// =============================================================================

PhysicsWorld::PhysicsWorld(const PhysicsWorldConfig& config)
    : m_config(config)
{
    // Create broad phase based on configuration
    switch (config.broadphaseType) {
        case PhysicsWorldConfig::BroadphaseType::BruteForce:
            m_broadPhase = std::make_unique<BruteForceBroadPhase>();
            break;
        case PhysicsWorldConfig::BroadphaseType::SortAndSweep:
            // Fall through to BVH for now
        case PhysicsWorldConfig::BroadphaseType::BVH:
        default:
            m_broadPhase = std::make_unique<BVHBroadPhase>();
            break;
    }
    
    // Create narrow phase
    m_narrowPhase = std::make_unique<GJKNarrowPhase>();
    
    // Create constraint solver
    m_solver = std::make_unique<SequentialImpulseSolver>(
        config.velocityIterations, config.positionIterations);
}

PhysicsWorld::~PhysicsWorld() = default;

PhysicsWorld::PhysicsWorld(PhysicsWorld&&) noexcept = default;
PhysicsWorld& PhysicsWorld::operator=(PhysicsWorld&&) noexcept = default;

std::unique_ptr<PhysicsWorld> PhysicsWorld::create(const PhysicsWorldConfig& config) {
    return std::unique_ptr<PhysicsWorld>(new PhysicsWorld(config));
}

void PhysicsWorld::step(f32 deltaTime) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    m_timeAccumulator += deltaTime;
    m_stats.substeps = 0;
    
    while (m_timeAccumulator >= m_config.fixedTimestep) {
        if (m_stats.substeps >= m_config.maxSubsteps) {
            // Too many substeps, skip remaining time
            m_timeAccumulator = 0.0f;
            break;
        }
        
        stepFixed(m_config.fixedTimestep);
        m_timeAccumulator -= m_config.fixedTimestep;
        m_stats.substeps++;
    }
    
    // Calculate interpolation alpha for rendering
    m_interpolationAlpha = m_timeAccumulator / m_config.fixedTimestep;
    
    auto endTime = std::chrono::high_resolution_clock::now();
    m_stats.totalTime = std::chrono::duration<f32, std::milli>(endTime - startTime).count();
}

void PhysicsWorld::stepFixed(f32 fixedDeltaTime) {
    // Store previous state for interpolation
    for (auto& [id, body] : m_bodies) {
        body->storeState();
    }
    
    // Phase 1: Broad phase collision detection
    broadPhase();
    
    // Phase 2: Narrow phase collision detection
    narrowPhase();
    
    // Phase 3: Integrate velocities (apply forces)
    integrate(fixedDeltaTime);
    
    // Phase 4: Solve constraints
    solveConstraints(fixedDeltaTime);
    
    // Phase 5: Update sleep states
    if (m_config.enableSleeping) {
        updateSleepStates(fixedDeltaTime);
    }
    
    // Phase 6: Handle collision callbacks
    handleCallbacks();
}

void PhysicsWorld::broadPhase() {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Update all body bounds in broad phase
    for (auto& [id, body] : m_bodies) {
        m_broadPhase->updateBody(id, body->getWorldBounds());
    }
    
    // Find potentially colliding pairs
    m_potentialPairs.clear();
    m_broadPhase->findPairs(m_potentialPairs);
    
    m_stats.broadPhasePairs = static_cast<u32>(m_potentialPairs.size());
    
    auto endTime = std::chrono::high_resolution_clock::now();
    m_stats.broadPhaseTime = std::chrono::duration<f32, std::milli>(endTime - startTime).count();
}

void PhysicsWorld::narrowPhase() {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Store previous contacts for callback detection
    m_previousContacts = std::move(m_contacts);
    m_contacts.clear();
    
    // Test each potential pair
    for (const auto& [idA, idB] : m_potentialPairs) {
        RigidBody* bodyA = getBody(idA);
        RigidBody* bodyB = getBody(idB);
        
        if (!bodyA || !bodyB) continue;
        
        // Skip if both are static or sleeping
        if (bodyA->isStatic() && bodyB->isStatic()) continue;
        if (bodyA->isSleeping() && bodyB->isSleeping()) continue;
        
        // Skip if they shouldn't collide
        if (!bodyA->shouldCollideWith(*bodyB)) continue;
        
        // Perform narrow phase collision
        ContactManifold manifold;
        if (m_narrowPhase->collide(*bodyA, *bodyB, manifold)) {
            manifold.bodyA = idA;
            manifold.bodyB = idB;
            manifold.friction = std::sqrt(
                bodyA->getMaterial().dynamicFriction * bodyB->getMaterial().dynamicFriction);
            manifold.restitution = std::max(
                bodyA->getMaterial().restitution, bodyB->getMaterial().restitution);
            manifold.isSensor = bodyA->isSensor() || bodyB->isSensor();
            
            m_contacts.push_back(manifold);
        }
    }
    
    m_stats.contactCount = static_cast<u32>(m_contacts.size());
    
    auto endTime = std::chrono::high_resolution_clock::now();
    m_stats.narrowPhaseTime = std::chrono::duration<f32, std::milli>(endTime - startTime).count();
}

void PhysicsWorld::integrate(f32 deltaTime) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (auto& [id, body] : m_bodies) {
        body->integrateVelocities(deltaTime, m_config.gravity);
        body->applyDamping(deltaTime);
        body->clampVelocities();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    m_stats.integrationTime = std::chrono::duration<f32, std::milli>(endTime - startTime).count();
}

void PhysicsWorld::solveConstraints(f32 deltaTime) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Collect active bodies
    std::vector<RigidBody*> activeBodies;
    for (auto& [id, body] : m_bodies) {
        if (body->isActive()) {
            activeBodies.push_back(body.get());
        }
    }
    
    // Solve velocity constraints
    m_solver->solveVelocities(activeBodies, m_contacts, deltaTime);
    
    // Integrate positions
    for (auto* body : activeBodies) {
        body->integratePositions(deltaTime);
    }
    
    // Solve position constraints
    m_solver->solvePositions(activeBodies, m_contacts, deltaTime);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    m_stats.solverTime = std::chrono::duration<f32, std::milli>(endTime - startTime).count();
}

void PhysicsWorld::updateSleepStates(f32 deltaTime) {
    m_stats.activeBodies = 0;
    m_stats.sleepingBodies = 0;
    m_stats.staticBodies = 0;
    
    for (auto& [id, body] : m_bodies) {
        if (body->isStatic()) {
            m_stats.staticBodies++;
        } else if (body->isSleeping()) {
            m_stats.sleepingBodies++;
        } else {
            body->updateSleepTimer(deltaTime);
            m_stats.activeBodies++;
        }
    }
}

void PhysicsWorld::handleCallbacks() {
    if (!m_onCollisionBegin && !m_onCollisionEnd && !m_onCollisionPersist &&
        !m_onTriggerEnter && !m_onTriggerExit) {
        return;
    }
    
    // Build set of current contact pairs
    std::vector<std::pair<BodyId, BodyId>> currentPairs;
    for (const auto& contact : m_contacts) {
        currentPairs.emplace_back(
            std::min(contact.bodyA, contact.bodyB),
            std::max(contact.bodyA, contact.bodyB)
        );
    }
    
    // Build set of previous contact pairs
    std::vector<std::pair<BodyId, BodyId>> previousPairs;
    for (const auto& contact : m_previousContacts) {
        previousPairs.emplace_back(
            std::min(contact.bodyA, contact.bodyB),
            std::max(contact.bodyA, contact.bodyB)
        );
    }
    
    std::sort(currentPairs.begin(), currentPairs.end());
    std::sort(previousPairs.begin(), previousPairs.end());
    
    // Find new contacts (begin)
    for (const auto& contact : m_contacts) {
        auto pair = std::make_pair(
            std::min(contact.bodyA, contact.bodyB),
            std::max(contact.bodyA, contact.bodyB)
        );
        
        if (!std::binary_search(previousPairs.begin(), previousPairs.end(), pair)) {
            if (contact.isSensor && m_onTriggerEnter) {
                m_onTriggerEnter(contact.bodyA, contact.bodyB);
            } else if (!contact.isSensor && m_onCollisionBegin) {
                m_onCollisionBegin(contact.bodyA, contact.bodyB, contact);
            }
        } else if (!contact.isSensor && m_onCollisionPersist) {
            // Existing contact
            m_onCollisionPersist(contact.bodyA, contact.bodyB, contact);
        }
    }
    
    // Find ended contacts
    for (const auto& contact : m_previousContacts) {
        auto pair = std::make_pair(
            std::min(contact.bodyA, contact.bodyB),
            std::max(contact.bodyA, contact.bodyB)
        );
        
        if (!std::binary_search(currentPairs.begin(), currentPairs.end(), pair)) {
            if (contact.isSensor && m_onTriggerExit) {
                m_onTriggerExit(contact.bodyA, contact.bodyB);
            } else if (!contact.isSensor && m_onCollisionEnd) {
                m_onCollisionEnd(contact.bodyA, contact.bodyB, contact);
            }
        }
    }
}

BodyId PhysicsWorld::generateBodyId() {
    return m_nextBodyId++;
}

BodyId PhysicsWorld::createBody(const RigidBodyDesc& desc) {
    BodyId id = generateBodyId();
    
    auto body = std::make_unique<RigidBody>(id, desc);
    
    // Add to broad phase
    m_broadPhase->addBody(id, body->getWorldBounds());
    
    m_bodies[id] = std::move(body);
    
    return id;
}

void PhysicsWorld::destroyBody(BodyId bodyId) {
    auto it = m_bodies.find(bodyId);
    if (it == m_bodies.end()) return;
    
    m_broadPhase->removeBody(bodyId);
    m_bodies.erase(it);
    
    // Remove any contacts involving this body
    m_contacts.erase(
        std::remove_if(m_contacts.begin(), m_contacts.end(),
            [bodyId](const ContactManifold& c) {
                return c.bodyA == bodyId || c.bodyB == bodyId;
            }),
        m_contacts.end()
    );
}

RigidBody* PhysicsWorld::getBody(BodyId bodyId) {
    auto it = m_bodies.find(bodyId);
    return (it != m_bodies.end()) ? it->second.get() : nullptr;
}

const RigidBody* PhysicsWorld::getBody(BodyId bodyId) const {
    auto it = m_bodies.find(bodyId);
    return (it != m_bodies.end()) ? it->second.get() : nullptr;
}

bool PhysicsWorld::hasBody(BodyId bodyId) const {
    return m_bodies.find(bodyId) != m_bodies.end();
}

std::vector<BodyId> PhysicsWorld::getAllBodyIds() const {
    std::vector<BodyId> ids;
    ids.reserve(m_bodies.size());
    for (const auto& [id, body] : m_bodies) {
        ids.push_back(id);
    }
    return ids;
}

bool PhysicsWorld::raycast(const Ray& ray, RaycastHit& hit, RaycastFilter filter) const {
    std::vector<BodyId> candidates;
    m_broadPhase->queryRay(ray, candidates);
    
    hit.hit = false;
    f32 closestDist = ray.maxDistance;
    
    for (BodyId id : candidates) {
        if (filter && !filter(id)) continue;
        
        const RigidBody* body = getBody(id);
        if (!body || !body->getShape()) continue;
        
        // Transform ray to body local space
        Vec3 localOrigin = body->getOrientation().inverse() * (ray.origin - body->getPosition());
        Vec3 localDir = body->getOrientation().inverse() * ray.direction;
        
        Ray localRay;
        localRay.origin = localOrigin;
        localRay.direction = localDir;
        localRay.maxDistance = closestDist;
        
        RaycastHit localHit;
        if (body->getShape()->raycast(localRay, localHit) && localHit.distance < closestDist) {
            closestDist = localHit.distance;
            hit = localHit;
            hit.point = body->getOrientation() * localHit.point + body->getPosition();
            hit.normal = body->getOrientation() * localHit.normal;
            hit.bodyId = id;
        }
    }
    
    return hit.hit;
}

std::vector<RaycastHit> PhysicsWorld::raycastAll(const Ray& ray, u32 maxHits, RaycastFilter filter) const {
    std::vector<RaycastHit> hits;
    
    std::vector<BodyId> candidates;
    m_broadPhase->queryRay(ray, candidates);
    
    for (BodyId id : candidates) {
        if (filter && !filter(id)) continue;
        
        const RigidBody* body = getBody(id);
        if (!body || !body->getShape()) continue;
        
        Vec3 localOrigin = body->getOrientation().inverse() * (ray.origin - body->getPosition());
        Vec3 localDir = body->getOrientation().inverse() * ray.direction;
        
        Ray localRay;
        localRay.origin = localOrigin;
        localRay.direction = localDir;
        localRay.maxDistance = ray.maxDistance;
        
        RaycastHit localHit;
        if (body->getShape()->raycast(localRay, localHit)) {
            localHit.point = body->getOrientation() * localHit.point + body->getPosition();
            localHit.normal = body->getOrientation() * localHit.normal;
            localHit.bodyId = id;
            hits.push_back(localHit);
            
            if (hits.size() >= maxHits) break;
        }
    }
    
    // Sort by distance
    std::sort(hits.begin(), hits.end(),
        [](const RaycastHit& a, const RaycastHit& b) { return a.distance < b.distance; });
    
    return hits;
}

QueryResult PhysicsWorld::queryPoint(const Vec3& point) const {
    QueryResult result;
    
    // Small AABB around point
    AABB aabb = AABB::fromCenterExtents(point, Vec3(PHYSICS_EPSILON));
    m_broadPhase->queryAABB(aabb, result.bodies);
    
    // Filter to actual containment
    result.bodies.erase(
        std::remove_if(result.bodies.begin(), result.bodies.end(),
            [this, &point](BodyId id) {
                const RigidBody* body = getBody(id);
                return !body || !body->getWorldBounds().contains(point);
            }),
        result.bodies.end()
    );
    
    return result;
}

QueryResult PhysicsWorld::queryAABB(const AABB& aabb) const {
    QueryResult result;
    m_broadPhase->queryAABB(aabb, result.bodies);
    return result;
}

QueryResult PhysicsWorld::querySphere(const Vec3& center, f32 radius) const {
    QueryResult result;
    
    AABB aabb = AABB::fromSphere(center, radius);
    m_broadPhase->queryAABB(aabb, result.bodies);
    
    // Filter to actual sphere overlap
    f32 radiusSq = radius * radius;
    result.bodies.erase(
        std::remove_if(result.bodies.begin(), result.bodies.end(),
            [this, &center, radiusSq](BodyId id) {
                const RigidBody* body = getBody(id);
                if (!body) return true;
                
                AABB bounds = body->getWorldBounds();
                Vec3 closest;
                closest.x = std::clamp(center.x, bounds.min.x, bounds.max.x);
                closest.y = std::clamp(center.y, bounds.min.y, bounds.max.y);
                closest.z = std::clamp(center.z, bounds.min.z, bounds.max.z);
                
                return (closest - center).lengthSquared() > radiusSq;
            }),
        result.bodies.end()
    );
    
    return result;
}

QueryResult PhysicsWorld::queryShape(const CollisionShape& shape, const Vec3& position, const Quat& orientation) const {
    QueryResult result;
    
    AABB aabb = shape.getWorldBounds(position, orientation);
    m_broadPhase->queryAABB(aabb, result.bodies);
    
    // Further narrow phase filtering would go here
    
    return result;
}

void PhysicsWorld::debugDraw() const {
    if (!m_debugDrawEnabled || !m_debugDraw.drawLine) return;
    
    // Draw body AABBs
    if (m_debugDraw.drawAABB) {
        for (const auto& [id, body] : m_bodies) {
            Vec4 color;
            if (body->isStatic()) {
                color = Vec4(0.5f, 0.5f, 0.5f, 1.0f); // Gray for static
            } else if (body->isSleeping()) {
                color = Vec4(0.2f, 0.2f, 0.8f, 1.0f); // Blue for sleeping
            } else {
                color = Vec4(0.0f, 1.0f, 0.0f, 1.0f); // Green for active
            }
            
            m_debugDraw.drawAABB(body->getWorldBounds(), color);
        }
    }
    
    // Draw contacts
    if (m_debugDraw.drawContact) {
        for (const auto& contact : m_contacts) {
            m_debugDraw.drawContact(contact);
        }
    }
}

// =============================================================================
// BruteForceBroadPhase Implementation
// =============================================================================

void BruteForceBroadPhase::addBody(BodyId id, const AABB& bounds) {
    m_entries.push_back({id, bounds});
}

void BruteForceBroadPhase::removeBody(BodyId id) {
    m_entries.erase(
        std::remove_if(m_entries.begin(), m_entries.end(),
            [id](const Entry& e) { return e.id == id; }),
        m_entries.end()
    );
}

void BruteForceBroadPhase::updateBody(BodyId id, const AABB& bounds) {
    for (auto& entry : m_entries) {
        if (entry.id == id) {
            entry.bounds = bounds;
            return;
        }
    }
}

void BruteForceBroadPhase::findPairs(std::vector<std::pair<BodyId, BodyId>>& outPairs) {
    outPairs.clear();
    
    for (size_t i = 0; i < m_entries.size(); i++) {
        for (size_t j = i + 1; j < m_entries.size(); j++) {
            if (m_entries[i].bounds.overlaps(m_entries[j].bounds)) {
                outPairs.emplace_back(m_entries[i].id, m_entries[j].id);
            }
        }
    }
}

void BruteForceBroadPhase::queryAABB(const AABB& aabb, std::vector<BodyId>& outBodies) {
    outBodies.clear();
    
    for (const auto& entry : m_entries) {
        if (entry.bounds.overlaps(aabb)) {
            outBodies.push_back(entry.id);
        }
    }
}

void BruteForceBroadPhase::queryRay(const Ray& ray, std::vector<BodyId>& outBodies) {
    outBodies.clear();
    
    for (const auto& entry : m_entries) {
        // Simple ray-AABB intersection test
        Vec3 tmin = (entry.bounds.min - ray.origin) / ray.direction;
        Vec3 tmax = (entry.bounds.max - ray.origin) / ray.direction;
        
        Vec3 t1 = Vec3(std::min(tmin.x, tmax.x), std::min(tmin.y, tmax.y), std::min(tmin.z, tmax.z));
        Vec3 t2 = Vec3(std::max(tmin.x, tmax.x), std::max(tmin.y, tmax.y), std::max(tmin.z, tmax.z));
        
        f32 near = std::max({t1.x, t1.y, t1.z});
        f32 far = std::min({t2.x, t2.y, t2.z});
        
        if (near <= far && far >= 0.0f && near <= ray.maxDistance) {
            outBodies.push_back(entry.id);
        }
    }
}

// =============================================================================
// BVHBroadPhase Implementation
// =============================================================================

BVHBroadPhase::BVHBroadPhase() {
    m_nodes.reserve(1024);
}

BVHBroadPhase::~BVHBroadPhase() = default;

i32 BVHBroadPhase::allocateNode() {
    if (m_freeList >= 0) {
        i32 nodeIndex = m_freeList;
        m_freeList = m_nodes[nodeIndex].leftChild;
        m_nodes[nodeIndex] = BVHNode{};
        return nodeIndex;
    }
    
    m_nodes.push_back(BVHNode{});
    return static_cast<i32>(m_nodes.size() - 1);
}

void BVHBroadPhase::freeNode(i32 nodeIndex) {
    m_nodes[nodeIndex].leftChild = m_freeList;
    m_freeList = nodeIndex;
}

void BVHBroadPhase::addBody(BodyId id, const AABB& bounds) {
    i32 leafIndex = allocateNode();
    m_nodes[leafIndex].bounds = bounds;
    m_nodes[leafIndex].bodyId = id;
    
    m_bodyToNode[id] = leafIndex;
    
    insertLeaf(leafIndex);
}

void BVHBroadPhase::removeBody(BodyId id) {
    auto it = m_bodyToNode.find(id);
    if (it == m_bodyToNode.end()) return;
    
    i32 leafIndex = it->second;
    removeLeaf(leafIndex);
    freeNode(leafIndex);
    
    m_bodyToNode.erase(it);
}

void BVHBroadPhase::updateBody(BodyId id, const AABB& bounds) {
    auto it = m_bodyToNode.find(id);
    if (it == m_bodyToNode.end()) return;
    
    i32 leafIndex = it->second;
    
    // Check if bounds still fit
    if (m_nodes[leafIndex].bounds.contains(bounds)) {
        return; // No update needed
    }
    
    // Remove and reinsert
    removeLeaf(leafIndex);
    m_nodes[leafIndex].bounds = bounds;
    m_nodes[leafIndex].bounds.expand(0.1f); // Add some padding
    insertLeaf(leafIndex);
}

void BVHBroadPhase::insertLeaf(i32 leafIndex) {
    if (m_root < 0) {
        m_root = leafIndex;
        return;
    }
    
    // Find the best sibling
    i32 sibling = m_root;
    while (!m_nodes[sibling].isLeaf()) {
        i32 left = m_nodes[sibling].leftChild;
        i32 right = m_nodes[sibling].rightChild;
        
        f32 leftCost = computeCost(m_nodes[left].bounds, m_nodes[leafIndex].bounds);
        f32 rightCost = computeCost(m_nodes[right].bounds, m_nodes[leafIndex].bounds);
        
        sibling = (leftCost < rightCost) ? left : right;
    }
    
    // Create a new parent
    i32 oldParent = m_nodes[sibling].parent;
    i32 newParent = allocateNode();
    m_nodes[newParent].parent = oldParent;
    m_nodes[newParent].bounds = m_nodes[sibling].bounds;
    m_nodes[newParent].bounds.expandToInclude(m_nodes[leafIndex].bounds);
    
    if (oldParent >= 0) {
        if (m_nodes[oldParent].leftChild == sibling) {
            m_nodes[oldParent].leftChild = newParent;
        } else {
            m_nodes[oldParent].rightChild = newParent;
        }
    } else {
        m_root = newParent;
    }
    
    m_nodes[newParent].leftChild = sibling;
    m_nodes[newParent].rightChild = leafIndex;
    m_nodes[sibling].parent = newParent;
    m_nodes[leafIndex].parent = newParent;
    
    // Walk up and refit
    i32 index = m_nodes[leafIndex].parent;
    while (index >= 0) {
        i32 left = m_nodes[index].leftChild;
        i32 right = m_nodes[index].rightChild;
        
        m_nodes[index].bounds = m_nodes[left].bounds;
        m_nodes[index].bounds.expandToInclude(m_nodes[right].bounds);
        
        index = m_nodes[index].parent;
    }
}

void BVHBroadPhase::removeLeaf(i32 leafIndex) {
    if (leafIndex == m_root) {
        m_root = -1;
        return;
    }
    
    i32 parent = m_nodes[leafIndex].parent;
    i32 grandParent = m_nodes[parent].parent;
    i32 sibling = (m_nodes[parent].leftChild == leafIndex) ?
                  m_nodes[parent].rightChild : m_nodes[parent].leftChild;
    
    if (grandParent >= 0) {
        if (m_nodes[grandParent].leftChild == parent) {
            m_nodes[grandParent].leftChild = sibling;
        } else {
            m_nodes[grandParent].rightChild = sibling;
        }
        m_nodes[sibling].parent = grandParent;
        freeNode(parent);
        
        // Refit ancestors
        i32 index = grandParent;
        while (index >= 0) {
            i32 left = m_nodes[index].leftChild;
            i32 right = m_nodes[index].rightChild;
            
            m_nodes[index].bounds = m_nodes[left].bounds;
            m_nodes[index].bounds.expandToInclude(m_nodes[right].bounds);
            
            index = m_nodes[index].parent;
        }
    } else {
        m_root = sibling;
        m_nodes[sibling].parent = -1;
        freeNode(parent);
    }
    
    m_nodes[leafIndex].parent = -1;
}

f32 BVHBroadPhase::computeCost(const AABB& a, const AABB& b) const {
    AABB combined = a;
    combined.expandToInclude(b);
    return combined.getSurfaceArea();
}

void BVHBroadPhase::findPairs(std::vector<std::pair<BodyId, BodyId>>& outPairs) {
    outPairs.clear();
    
    if (m_root < 0) return;
    
    // Collect all leaves
    std::vector<i32> leaves;
    std::vector<i32> stack;
    stack.push_back(m_root);
    
    while (!stack.empty()) {
        i32 index = stack.back();
        stack.pop_back();
        
        if (m_nodes[index].isLeaf()) {
            leaves.push_back(index);
        } else {
            stack.push_back(m_nodes[index].leftChild);
            stack.push_back(m_nodes[index].rightChild);
        }
    }
    
    // Test all leaf pairs (could be optimized with tree traversal)
    for (size_t i = 0; i < leaves.size(); i++) {
        for (size_t j = i + 1; j < leaves.size(); j++) {
            if (m_nodes[leaves[i]].bounds.overlaps(m_nodes[leaves[j]].bounds)) {
                outPairs.emplace_back(
                    m_nodes[leaves[i]].bodyId,
                    m_nodes[leaves[j]].bodyId
                );
            }
        }
    }
}

void BVHBroadPhase::queryRecursive(i32 nodeIndex, const AABB& aabb, std::vector<BodyId>& outBodies) const {
    if (nodeIndex < 0) return;
    
    if (!m_nodes[nodeIndex].bounds.overlaps(aabb)) return;
    
    if (m_nodes[nodeIndex].isLeaf()) {
        outBodies.push_back(m_nodes[nodeIndex].bodyId);
    } else {
        queryRecursive(m_nodes[nodeIndex].leftChild, aabb, outBodies);
        queryRecursive(m_nodes[nodeIndex].rightChild, aabb, outBodies);
    }
}

void BVHBroadPhase::queryAABB(const AABB& aabb, std::vector<BodyId>& outBodies) {
    outBodies.clear();
    queryRecursive(m_root, aabb, outBodies);
}

void BVHBroadPhase::queryRay(const Ray& ray, std::vector<BodyId>& outBodies) {
    outBodies.clear();
    
    if (m_root < 0) return;
    
    std::vector<i32> stack;
    stack.push_back(m_root);
    
    while (!stack.empty()) {
        i32 index = stack.back();
        stack.pop_back();
        
        const AABB& bounds = m_nodes[index].bounds;
        
        // Ray-AABB test
        Vec3 tmin = (bounds.min - ray.origin) / ray.direction;
        Vec3 tmax = (bounds.max - ray.origin) / ray.direction;
        
        Vec3 t1 = Vec3(std::min(tmin.x, tmax.x), std::min(tmin.y, tmax.y), std::min(tmin.z, tmax.z));
        Vec3 t2 = Vec3(std::max(tmin.x, tmax.x), std::max(tmin.y, tmax.y), std::max(tmin.z, tmax.z));
        
        f32 near = std::max({t1.x, t1.y, t1.z});
        f32 far = std::min({t2.x, t2.y, t2.z});
        
        if (near > far || far < 0.0f || near > ray.maxDistance) continue;
        
        if (m_nodes[index].isLeaf()) {
            outBodies.push_back(m_nodes[index].bodyId);
        } else {
            stack.push_back(m_nodes[index].leftChild);
            stack.push_back(m_nodes[index].rightChild);
        }
    }
}

void BVHBroadPhase::rebuild() {
    // Collect all bodies
    std::vector<std::pair<BodyId, AABB>> bodies;
    for (const auto& [id, nodeIndex] : m_bodyToNode) {
        bodies.emplace_back(id, m_nodes[nodeIndex].bounds);
    }
    
    // Clear tree
    m_nodes.clear();
    m_bodyToNode.clear();
    m_root = -1;
    m_freeList = -1;
    
    // Reinsert all bodies
    for (const auto& [id, bounds] : bodies) {
        addBody(id, bounds);
    }
}

// =============================================================================
// GJKNarrowPhase Implementation
// =============================================================================

bool GJKNarrowPhase::collide(const RigidBody& bodyA, const RigidBody& bodyB, ContactManifold& manifold) {
    if (!bodyA.getShape() || !bodyB.getShape()) return false;
    
    Vec3 simplex[4];
    u32 simplexSize = 0;
    
    if (!gjk(*bodyA.getShape(), bodyA.getPosition(), bodyA.getOrientation(),
             *bodyB.getShape(), bodyB.getPosition(), bodyB.getOrientation(),
             simplex, simplexSize)) {
        return false;
    }
    
    Vec3 normal;
    f32 penetration;
    
    if (!epa(*bodyA.getShape(), bodyA.getPosition(), bodyA.getOrientation(),
             *bodyB.getShape(), bodyB.getPosition(), bodyB.getOrientation(),
             simplex, simplexSize, normal, penetration)) {
        return false;
    }
    
    // Generate contact point
    manifold.pointCount = 1;
    manifold.normal = normal;
    
    // Find contact point on A
    Vec3 localDirA = bodyA.getOrientation().inverse() * (-normal);
    Vec3 localSupportA = bodyA.getShape()->getSupport(localDirA);
    Vec3 worldSupportA = bodyA.getOrientation() * localSupportA + bodyA.getPosition();
    
    manifold.points[0].position = worldSupportA + normal * (penetration * 0.5f);
    manifold.points[0].normal = normal;
    manifold.points[0].penetration = penetration;
    
    return true;
}

Vec3 GJKNarrowPhase::support(const CollisionShape& shapeA, const Vec3& posA, const Quat& rotA,
                             const CollisionShape& shapeB, const Vec3& posB, const Quat& rotB,
                             const Vec3& direction) {
    // Support point on A in direction
    Vec3 localDirA = rotA.inverse() * direction;
    Vec3 localSupportA = shapeA.getSupport(localDirA);
    Vec3 worldSupportA = rotA * localSupportA + posA;
    
    // Support point on B in opposite direction
    Vec3 localDirB = rotB.inverse() * (-direction);
    Vec3 localSupportB = shapeB.getSupport(localDirB);
    Vec3 worldSupportB = rotB * localSupportB + posB;
    
    // Minkowski difference
    return worldSupportA - worldSupportB;
}

bool GJKNarrowPhase::gjk(const CollisionShape& shapeA, const Vec3& posA, const Quat& rotA,
                         const CollisionShape& shapeB, const Vec3& posB, const Quat& rotB,
                         Vec3* simplex, u32& simplexSize) {
    Vec3 direction = posB - posA;
    if (direction.lengthSquared() < PHYSICS_EPSILON) {
        direction = Vec3::right();
    }
    
    simplex[0] = support(shapeA, posA, rotA, shapeB, posB, rotB, direction);
    simplexSize = 1;
    
    direction = -simplex[0];
    
    constexpr u32 MAX_ITERATIONS = 32;
    
    for (u32 iter = 0; iter < MAX_ITERATIONS; iter++) {
        if (direction.lengthSquared() < PHYSICS_EPSILON) {
            return true; // Origin is on the simplex
        }
        
        Vec3 a = support(shapeA, posA, rotA, shapeB, posB, rotB, direction);
        
        if (a.dot(direction) < 0) {
            return false; // No intersection
        }
        
        simplex[simplexSize++] = a;
        
        // Update simplex and direction
        if (simplexSize == 2) {
            // Line case
            Vec3 ab = simplex[0] - simplex[1];
            Vec3 ao = -simplex[1];
            
            if (ab.dot(ao) > 0) {
                direction = ab.cross(ao).cross(ab);
            } else {
                simplex[0] = simplex[1];
                simplexSize = 1;
                direction = ao;
            }
        } else if (simplexSize == 3) {
            // Triangle case
            Vec3 ab = simplex[1] - simplex[2];
            Vec3 ac = simplex[0] - simplex[2];
            Vec3 ao = -simplex[2];
            Vec3 abc = ab.cross(ac);
            
            if (abc.cross(ac).dot(ao) > 0) {
                if (ac.dot(ao) > 0) {
                    simplex[1] = simplex[2];
                    simplexSize = 2;
                    direction = ac.cross(ao).cross(ac);
                } else {
                    simplex[0] = simplex[1];
                    simplex[1] = simplex[2];
                    simplexSize = 2;
                    if (ab.dot(ao) > 0) {
                        direction = ab.cross(ao).cross(ab);
                    } else {
                        simplex[0] = simplex[2];
                        simplexSize = 1;
                        direction = ao;
                    }
                }
            } else if (ab.cross(abc).dot(ao) > 0) {
                simplex[0] = simplex[1];
                simplex[1] = simplex[2];
                simplexSize = 2;
                if (ab.dot(ao) > 0) {
                    direction = ab.cross(ao).cross(ab);
                } else {
                    simplex[0] = simplex[2];
                    simplexSize = 1;
                    direction = ao;
                }
            } else {
                if (abc.dot(ao) > 0) {
                    direction = abc;
                } else {
                    std::swap(simplex[0], simplex[1]);
                    direction = -abc;
                }
            }
        } else if (simplexSize == 4) {
            // Tetrahedron case
            Vec3 ab = simplex[2] - simplex[3];
            Vec3 ac = simplex[1] - simplex[3];
            Vec3 ad = simplex[0] - simplex[3];
            Vec3 ao = -simplex[3];
            
            Vec3 abc = ab.cross(ac);
            Vec3 acd = ac.cross(ad);
            Vec3 adb = ad.cross(ab);
            
            if (abc.dot(ao) > 0) {
                simplex[0] = simplex[1];
                simplex[1] = simplex[2];
                simplex[2] = simplex[3];
                simplexSize = 3;
                direction = abc;
            } else if (acd.dot(ao) > 0) {
                simplex[2] = simplex[3];
                simplexSize = 3;
                direction = acd;
            } else if (adb.dot(ao) > 0) {
                simplex[1] = simplex[0];
                simplex[0] = simplex[2];
                simplex[2] = simplex[3];
                simplexSize = 3;
                direction = adb;
            } else {
                return true; // Origin is inside
            }
        }
    }
    
    return false;
}

bool GJKNarrowPhase::epa(const CollisionShape& shapeA, const Vec3& posA, const Quat& rotA,
                         const CollisionShape& shapeB, const Vec3& posB, const Quat& rotB,
                         Vec3* simplex, u32 simplexSize,
                         Vec3& normal, f32& penetration) {
    // Simplified EPA - just use the last normal from GJK
    // A full EPA implementation would expand the polytope to find the closest face
    
    if (simplexSize < 4) {
        // Expand to tetrahedron
        Vec3 direction = Vec3::right();
        while (simplexSize < 4) {
            Vec3 a = support(shapeA, posA, rotA, shapeB, posB, rotB, direction);
            simplex[simplexSize++] = a;
            
            if (simplexSize == 2) direction = Vec3::up();
            else if (simplexSize == 3) direction = Vec3::forward();
        }
    }
    
    // Calculate normal from tetrahedron
    Vec3 ab = simplex[1] - simplex[0];
    Vec3 ac = simplex[2] - simplex[0];
    normal = ab.cross(ac).normalized();
    
    // Make sure normal points towards origin
    Vec3 ao = -simplex[0];
    if (normal.dot(ao) < 0) {
        normal = -normal;
    }
    
    // Calculate approximate penetration
    penetration = std::abs(simplex[0].dot(normal));
    
    return true;
}

// =============================================================================
// SequentialImpulseSolver Implementation
// =============================================================================

SequentialImpulseSolver::SequentialImpulseSolver(u32 velocityIterations, u32 positionIterations)
    : m_velocityIterations(velocityIterations)
    , m_positionIterations(positionIterations)
{
}

void SequentialImpulseSolver::solveVelocities(std::vector<RigidBody*>& bodies,
                                               std::vector<ContactManifold>& contacts,
                                               f32 deltaTime) {
    // Warm start
    warmStart(bodies, contacts);
    
    // Iterate
    for (u32 iter = 0; iter < m_velocityIterations; iter++) {
        for (auto& contact : contacts) {
            if (contact.isSensor) continue;
            
            RigidBody* bodyA = nullptr;
            RigidBody* bodyB = nullptr;
            
            for (auto* body : bodies) {
                if (body->getId() == contact.bodyA) bodyA = body;
                if (body->getId() == contact.bodyB) bodyB = body;
            }
            
            if (!bodyA || !bodyB) continue;
            
            for (u32 i = 0; i < contact.pointCount; i++) {
                solveVelocityConstraint(bodyA, bodyB, contact.points[i],
                                        contact.normal, contact.friction);
            }
        }
    }
}

void SequentialImpulseSolver::warmStart(std::vector<RigidBody*>& bodies,
                                         std::vector<ContactManifold>& contacts) {
    for (auto& contact : contacts) {
        if (contact.isSensor) continue;
        
        RigidBody* bodyA = nullptr;
        RigidBody* bodyB = nullptr;
        
        for (auto* body : bodies) {
            if (body->getId() == contact.bodyA) bodyA = body;
            if (body->getId() == contact.bodyB) bodyB = body;
        }
        
        if (!bodyA || !bodyB) continue;
        
        for (u32 i = 0; i < contact.pointCount; i++) {
            ContactPoint& cp = contact.points[i];
            
            Vec3 impulse = contact.normal * cp.normalImpulse + cp.tangentImpulse;
            
            Vec3 rA = cp.position - bodyA->getWorldCenterOfMass();
            Vec3 rB = cp.position - bodyB->getWorldCenterOfMass();
            
            if (bodyA->isDynamic()) {
                bodyA->applyImpulse(-impulse);
                bodyA->applyAngularImpulse(-rA.cross(impulse));
            }
            
            if (bodyB->isDynamic()) {
                bodyB->applyImpulse(impulse);
                bodyB->applyAngularImpulse(rB.cross(impulse));
            }
        }
    }
}

void SequentialImpulseSolver::solveVelocityConstraint(RigidBody* bodyA, RigidBody* bodyB,
                                                       ContactPoint& contact,
                                                       const Vec3& normal, f32 friction) {
    Vec3 rA = contact.position - bodyA->getWorldCenterOfMass();
    Vec3 rB = contact.position - bodyB->getWorldCenterOfMass();
    
    // Relative velocity at contact point
    Vec3 relVel = bodyB->getVelocityAtPoint(contact.position) -
                  bodyA->getVelocityAtPoint(contact.position);
    
    // Normal impulse
    f32 vn = relVel.dot(normal);
    
    f32 invMassSum = bodyA->getInverseMass() + bodyB->getInverseMass();
    
    // TODO: Add inertia terms
    f32 effectiveMass = (invMassSum > 0.0f) ? (1.0f / invMassSum) : 0.0f;
    
    f32 lambda = -vn * effectiveMass;
    f32 oldNormalImpulse = contact.normalImpulse;
    contact.normalImpulse = std::max(contact.normalImpulse + lambda, 0.0f);
    lambda = contact.normalImpulse - oldNormalImpulse;
    
    Vec3 impulse = normal * lambda;
    
    if (bodyA->isDynamic()) {
        bodyA->applyImpulse(-impulse);
    }
    if (bodyB->isDynamic()) {
        bodyB->applyImpulse(impulse);
    }
    
    // Friction impulse (simplified)
    relVel = bodyB->getVelocityAtPoint(contact.position) -
             bodyA->getVelocityAtPoint(contact.position);
    
    Vec3 tangent = relVel - normal * relVel.dot(normal);
    f32 tangentLen = tangent.length();
    
    if (tangentLen > PHYSICS_EPSILON) {
        tangent /= tangentLen;
        
        f32 vt = relVel.dot(tangent);
        f32 lambdaT = -vt * effectiveMass;
        
        f32 maxFriction = friction * contact.normalImpulse;
        lambdaT = std::clamp(lambdaT, -maxFriction, maxFriction);
        
        Vec3 frictionImpulse = tangent * lambdaT;
        
        if (bodyA->isDynamic()) {
            bodyA->applyImpulse(-frictionImpulse);
        }
        if (bodyB->isDynamic()) {
            bodyB->applyImpulse(frictionImpulse);
        }
        
        contact.tangentImpulse = tangent * lambdaT;
    }
}

void SequentialImpulseSolver::solvePositions(std::vector<RigidBody*>& bodies,
                                              std::vector<ContactManifold>& contacts,
                                              f32 deltaTime) {
    constexpr f32 SLOP = 0.01f;
    constexpr f32 BAUMGARTE = 0.2f;
    
    for (u32 iter = 0; iter < m_positionIterations; iter++) {
        for (auto& contact : contacts) {
            if (contact.isSensor) continue;
            
            RigidBody* bodyA = nullptr;
            RigidBody* bodyB = nullptr;
            
            for (auto* body : bodies) {
                if (body->getId() == contact.bodyA) bodyA = body;
                if (body->getId() == contact.bodyB) bodyB = body;
            }
            
            if (!bodyA || !bodyB) continue;
            
            for (u32 i = 0; i < contact.pointCount; i++) {
                solvePositionConstraint(bodyA, bodyB, contact.points[i], contact.normal);
            }
        }
    }
}

void SequentialImpulseSolver::solvePositionConstraint(RigidBody* bodyA, RigidBody* bodyB,
                                                       const ContactPoint& contact,
                                                       const Vec3& normal) {
    constexpr f32 SLOP = 0.01f;
    constexpr f32 BAUMGARTE = 0.2f;
    
    f32 separation = contact.penetration - SLOP;
    if (separation >= 0.0f) return;
    
    f32 invMassSum = bodyA->getInverseMass() + bodyB->getInverseMass();
    if (invMassSum <= 0.0f) return;
    
    f32 correction = BAUMGARTE * separation / invMassSum;
    Vec3 correctionVec = normal * correction;
    
    if (bodyA->isDynamic()) {
        Vec3 pos = bodyA->getPosition();
        bodyA->setPosition(pos + correctionVec * bodyA->getInverseMass());
    }
    
    if (bodyB->isDynamic()) {
        Vec3 pos = bodyB->getPosition();
        bodyB->setPosition(pos - correctionVec * bodyB->getInverseMass());
    }
}

} // namespace nova::physics
