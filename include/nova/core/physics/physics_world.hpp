/**
 * @file physics_world.hpp
 * @brief NovaCore Physics System - Physics World
 * 
 * Provides the main physics simulation container:
 * - Body management (create, destroy, query)
 * - Collision detection (broad phase + narrow phase)
 * - Constraint solving
 * - Raycasting and shape queries
 * - ECS integration
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "physics_types.hpp"
#include "collision_shape.hpp"
#include "rigid_body.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

namespace nova::physics {

// Forward declarations
class BroadPhase;
class NarrowPhase;
class ConstraintSolver;

/**
 * @brief Collision callback types
 */
using CollisionCallback = std::function<void(BodyId bodyA, BodyId bodyB, const ContactManifold& manifold)>;
using TriggerCallback = std::function<void(BodyId sensor, BodyId other)>;

/**
 * @brief Raycast filter callback
 * @return true to include body in raycast, false to skip
 */
using RaycastFilter = std::function<bool(BodyId bodyId)>;

/**
 * @brief Query result for body queries
 */
struct QueryResult {
    std::vector<BodyId> bodies;
    
    [[nodiscard]] bool empty() const { return bodies.empty(); }
    [[nodiscard]] usize count() const { return bodies.size(); }
};

/**
 * @brief Statistics for physics simulation
 */
struct PhysicsStats {
    /// Number of active bodies
    u32 activeBodies = 0;
    
    /// Number of sleeping bodies
    u32 sleepingBodies = 0;
    
    /// Number of static bodies
    u32 staticBodies = 0;
    
    /// Number of contacts this step
    u32 contactCount = 0;
    
    /// Number of broad phase pairs
    u32 broadPhasePairs = 0;
    
    /// Time spent in broad phase (ms)
    f32 broadPhaseTime = 0.0f;
    
    /// Time spent in narrow phase (ms)
    f32 narrowPhaseTime = 0.0f;
    
    /// Time spent in constraint solver (ms)
    f32 solverTime = 0.0f;
    
    /// Time spent integrating (ms)
    f32 integrationTime = 0.0f;
    
    /// Total simulation time (ms)
    f32 totalTime = 0.0f;
    
    /// Number of substeps this frame
    u32 substeps = 0;
};

/**
 * @brief Main physics simulation world
 * 
 * The PhysicsWorld manages all physics objects and runs the simulation.
 * It should be stepped once per frame with the frame's delta time.
 * 
 * Example usage:
 * @code
 * auto world = PhysicsWorld::create(PhysicsWorldConfig::mobileOptimized());
 * 
 * // Create a ground plane
 * auto groundShape = ShapeFactory::createPlane();
 * auto groundId = world->createBody(RigidBodyDesc::staticBody(groundShape));
 * 
 * // Create a dynamic box
 * auto boxShape = ShapeFactory::createBox(Vec3(0.5f));
 * auto boxDesc = RigidBodyDesc::dynamicBody(boxShape);
 * boxDesc.position = Vec3(0, 5, 0);
 * auto boxId = world->createBody(boxDesc);
 * 
 * // Simulate
 * world->step(1.0f / 60.0f);
 * @endcode
 */
class PhysicsWorld {
public:
    /**
     * @brief Create a physics world
     * @param config World configuration
     * @return Physics world instance
     */
    static std::unique_ptr<PhysicsWorld> create(const PhysicsWorldConfig& config = PhysicsWorldConfig::defaultConfig());
    
    ~PhysicsWorld();
    
    // Non-copyable, movable
    PhysicsWorld(const PhysicsWorld&) = delete;
    PhysicsWorld& operator=(const PhysicsWorld&) = delete;
    PhysicsWorld(PhysicsWorld&&) noexcept;
    PhysicsWorld& operator=(PhysicsWorld&&) noexcept;
    
    // =========================================================================
    // Simulation
    // =========================================================================
    
    /**
     * @brief Step the simulation
     * @param deltaTime Time since last step (seconds)
     * 
     * This will perform multiple fixed timestep substeps if needed
     * to catch up to the accumulated time.
     */
    void step(f32 deltaTime);
    
    /**
     * @brief Perform a single fixed timestep
     * @param fixedDeltaTime Fixed timestep duration
     */
    void stepFixed(f32 fixedDeltaTime);
    
    /**
     * @brief Get the interpolation alpha for rendering
     * @return Interpolation factor [0, 1]
     */
    [[nodiscard]] f32 getInterpolationAlpha() const { return m_interpolationAlpha; }
    
    // =========================================================================
    // Body Management
    // =========================================================================
    
    /**
     * @brief Create a rigid body
     * @param desc Body descriptor
     * @return Body ID (INVALID_BODY_ID on failure)
     */
    BodyId createBody(const RigidBodyDesc& desc);
    
    /**
     * @brief Destroy a rigid body
     * @param bodyId Body to destroy
     */
    void destroyBody(BodyId bodyId);
    
    /**
     * @brief Get a rigid body by ID
     * @param bodyId Body ID
     * @return Pointer to body, or nullptr if not found
     */
    [[nodiscard]] RigidBody* getBody(BodyId bodyId);
    [[nodiscard]] const RigidBody* getBody(BodyId bodyId) const;
    
    /**
     * @brief Check if a body exists
     */
    [[nodiscard]] bool hasBody(BodyId bodyId) const;
    
    /**
     * @brief Get all body IDs
     */
    [[nodiscard]] std::vector<BodyId> getAllBodyIds() const;
    
    /**
     * @brief Get body count
     */
    [[nodiscard]] u32 getBodyCount() const { return static_cast<u32>(m_bodies.size()); }
    
    // =========================================================================
    // Raycasting
    // =========================================================================
    
    /**
     * @brief Cast a ray and get the first hit
     * @param ray Ray to cast
     * @param hit Output hit result
     * @param filter Optional filter callback
     * @return true if something was hit
     */
    bool raycast(const Ray& ray, RaycastHit& hit, RaycastFilter filter = nullptr) const;
    
    /**
     * @brief Cast a ray and get all hits
     * @param ray Ray to cast
     * @param maxHits Maximum hits to return
     * @param filter Optional filter callback
     * @return Vector of hit results (sorted by distance)
     */
    std::vector<RaycastHit> raycastAll(const Ray& ray, u32 maxHits = 100, RaycastFilter filter = nullptr) const;
    
    // =========================================================================
    // Shape Queries
    // =========================================================================
    
    /**
     * @brief Query bodies overlapping a point
     */
    QueryResult queryPoint(const Vec3& point) const;
    
    /**
     * @brief Query bodies overlapping an AABB
     */
    QueryResult queryAABB(const AABB& aabb) const;
    
    /**
     * @brief Query bodies overlapping a sphere
     */
    QueryResult querySphere(const Vec3& center, f32 radius) const;
    
    /**
     * @brief Query bodies overlapping a shape at a transform
     */
    QueryResult queryShape(const CollisionShape& shape, const Vec3& position, const Quat& orientation) const;
    
    // =========================================================================
    // Callbacks
    // =========================================================================
    
    /**
     * @brief Set collision begin callback
     */
    void setOnCollisionBegin(CollisionCallback callback) { m_onCollisionBegin = std::move(callback); }
    
    /**
     * @brief Set collision end callback
     */
    void setOnCollisionEnd(CollisionCallback callback) { m_onCollisionEnd = std::move(callback); }
    
    /**
     * @brief Set collision persist callback
     */
    void setOnCollisionPersist(CollisionCallback callback) { m_onCollisionPersist = std::move(callback); }
    
    /**
     * @brief Set trigger enter callback
     */
    void setOnTriggerEnter(TriggerCallback callback) { m_onTriggerEnter = std::move(callback); }
    
    /**
     * @brief Set trigger exit callback
     */
    void setOnTriggerExit(TriggerCallback callback) { m_onTriggerExit = std::move(callback); }
    
    // =========================================================================
    // Configuration
    // =========================================================================
    
    /**
     * @brief Get gravity
     */
    [[nodiscard]] const Vec3& getGravity() const { return m_config.gravity; }
    
    /**
     * @brief Set gravity
     */
    void setGravity(const Vec3& gravity) { m_config.gravity = gravity; }
    
    /**
     * @brief Get fixed timestep
     */
    [[nodiscard]] f32 getFixedTimestep() const { return m_config.fixedTimestep; }
    
    /**
     * @brief Set fixed timestep
     */
    void setFixedTimestep(f32 timestep) { m_config.fixedTimestep = timestep; }
    
    /**
     * @brief Get configuration
     */
    [[nodiscard]] const PhysicsWorldConfig& getConfig() const { return m_config; }
    
    // =========================================================================
    // Statistics
    // =========================================================================
    
    /**
     * @brief Get simulation statistics
     */
    [[nodiscard]] const PhysicsStats& getStats() const { return m_stats; }
    
    /**
     * @brief Reset statistics
     */
    void resetStats() { m_stats = PhysicsStats{}; }
    
    // =========================================================================
    // Debug
    // =========================================================================
    
    /**
     * @brief Callback for debug drawing
     */
    struct DebugDrawCallbacks {
        std::function<void(const Vec3& start, const Vec3& end, const Vec4& color)> drawLine;
        std::function<void(const Vec3& center, f32 radius, const Vec4& color)> drawSphere;
        std::function<void(const AABB& aabb, const Vec4& color)> drawAABB;
        std::function<void(const ContactManifold& manifold)> drawContact;
    };
    
    /**
     * @brief Set debug draw callbacks
     */
    void setDebugDrawCallbacks(const DebugDrawCallbacks& callbacks) { m_debugDraw = callbacks; }
    
    /**
     * @brief Draw debug visualization
     */
    void debugDraw() const;
    
    /**
     * @brief Enable/disable debug drawing
     */
    void setDebugDrawEnabled(bool enabled) { m_debugDrawEnabled = enabled; }
    [[nodiscard]] bool isDebugDrawEnabled() const { return m_debugDrawEnabled; }

private:
    explicit PhysicsWorld(const PhysicsWorldConfig& config);
    
    // Simulation phases
    void broadPhase();
    void narrowPhase();
    void solveConstraints(f32 deltaTime);
    void integrate(f32 deltaTime);
    void updateSleepStates(f32 deltaTime);
    void handleCallbacks();
    
    // Body ID generation
    BodyId generateBodyId();
    
    // Configuration
    PhysicsWorldConfig m_config;
    
    // Bodies
    std::unordered_map<BodyId, std::unique_ptr<RigidBody>> m_bodies;
    BodyId m_nextBodyId = 1;
    
    // Collision detection
    std::unique_ptr<BroadPhase> m_broadPhase;
    std::unique_ptr<NarrowPhase> m_narrowPhase;
    std::unique_ptr<ConstraintSolver> m_solver;
    
    // Broad phase pairs
    std::vector<std::pair<BodyId, BodyId>> m_potentialPairs;
    
    // Contact manifolds
    std::vector<ContactManifold> m_contacts;
    std::vector<ContactManifold> m_previousContacts;
    
    // Callbacks
    CollisionCallback m_onCollisionBegin;
    CollisionCallback m_onCollisionEnd;
    CollisionCallback m_onCollisionPersist;
    TriggerCallback m_onTriggerEnter;
    TriggerCallback m_onTriggerExit;
    
    // Time accumulator for fixed timestep
    f32 m_timeAccumulator = 0.0f;
    f32 m_interpolationAlpha = 0.0f;
    
    // Statistics
    PhysicsStats m_stats;
    
    // Debug
    DebugDrawCallbacks m_debugDraw;
    bool m_debugDrawEnabled = false;
};

// =============================================================================
// Broad Phase Interface
// =============================================================================

/**
 * @brief Broad phase collision detection interface
 */
class BroadPhase {
public:
    virtual ~BroadPhase() = default;
    
    /**
     * @brief Add a body to the broad phase
     */
    virtual void addBody(BodyId id, const AABB& bounds) = 0;
    
    /**
     * @brief Remove a body from the broad phase
     */
    virtual void removeBody(BodyId id) = 0;
    
    /**
     * @brief Update a body's bounds
     */
    virtual void updateBody(BodyId id, const AABB& bounds) = 0;
    
    /**
     * @brief Find all potentially overlapping pairs
     */
    virtual void findPairs(std::vector<std::pair<BodyId, BodyId>>& outPairs) = 0;
    
    /**
     * @brief Query bodies overlapping an AABB
     */
    virtual void queryAABB(const AABB& aabb, std::vector<BodyId>& outBodies) = 0;
    
    /**
     * @brief Query bodies along a ray
     */
    virtual void queryRay(const Ray& ray, std::vector<BodyId>& outBodies) = 0;
};

/**
 * @brief Simple brute-force broad phase (good for < 100 bodies)
 */
class BruteForceBroadPhase : public BroadPhase {
public:
    void addBody(BodyId id, const AABB& bounds) override;
    void removeBody(BodyId id) override;
    void updateBody(BodyId id, const AABB& bounds) override;
    void findPairs(std::vector<std::pair<BodyId, BodyId>>& outPairs) override;
    void queryAABB(const AABB& aabb, std::vector<BodyId>& outBodies) override;
    void queryRay(const Ray& ray, std::vector<BodyId>& outBodies) override;

private:
    struct Entry {
        BodyId id;
        AABB bounds;
    };
    std::vector<Entry> m_entries;
};

/**
 * @brief BVH (Bounding Volume Hierarchy) broad phase
 */
class BVHBroadPhase : public BroadPhase {
public:
    BVHBroadPhase();
    ~BVHBroadPhase() override;
    
    void addBody(BodyId id, const AABB& bounds) override;
    void removeBody(BodyId id) override;
    void updateBody(BodyId id, const AABB& bounds) override;
    void findPairs(std::vector<std::pair<BodyId, BodyId>>& outPairs) override;
    void queryAABB(const AABB& aabb, std::vector<BodyId>& outBodies) override;
    void queryRay(const Ray& ray, std::vector<BodyId>& outBodies) override;
    
    /**
     * @brief Rebuild the tree (call periodically for dynamic scenes)
     */
    void rebuild();

private:
    struct BVHNode {
        AABB bounds;
        BodyId bodyId = INVALID_BODY_ID;  // Leaf node
        i32 leftChild = -1;
        i32 rightChild = -1;
        i32 parent = -1;
        
        [[nodiscard]] bool isLeaf() const { return leftChild < 0; }
    };
    
    std::vector<BVHNode> m_nodes;
    std::unordered_map<BodyId, i32> m_bodyToNode;
    i32 m_root = -1;
    i32 m_freeList = -1;
    
    i32 allocateNode();
    void freeNode(i32 nodeIndex);
    void insertLeaf(i32 leafIndex);
    void removeLeaf(i32 leafIndex);
    void queryRecursive(i32 nodeIndex, const AABB& aabb, std::vector<BodyId>& outBodies) const;
    void findPairsRecursive(i32 nodeA, i32 nodeB, std::vector<std::pair<BodyId, BodyId>>& outPairs);
    [[nodiscard]] f32 computeCost(const AABB& a, const AABB& b) const;
};

// =============================================================================
// Narrow Phase Interface
// =============================================================================

/**
 * @brief Narrow phase collision detection interface
 */
class NarrowPhase {
public:
    virtual ~NarrowPhase() = default;
    
    /**
     * @brief Test collision between two bodies
     * @param bodyA First body
     * @param bodyB Second body
     * @param manifold Output contact manifold
     * @return true if bodies are colliding
     */
    virtual bool collide(const RigidBody& bodyA, const RigidBody& bodyB, ContactManifold& manifold) = 0;
};

/**
 * @brief GJK/EPA based narrow phase
 */
class GJKNarrowPhase : public NarrowPhase {
public:
    bool collide(const RigidBody& bodyA, const RigidBody& bodyB, ContactManifold& manifold) override;

private:
    // GJK algorithm
    bool gjk(const CollisionShape& shapeA, const Vec3& posA, const Quat& rotA,
             const CollisionShape& shapeB, const Vec3& posB, const Quat& rotB,
             Vec3* simplex, u32& simplexSize);
    
    // EPA algorithm
    bool epa(const CollisionShape& shapeA, const Vec3& posA, const Quat& rotA,
             const CollisionShape& shapeB, const Vec3& posB, const Quat& rotB,
             Vec3* simplex, u32 simplexSize,
             Vec3& normal, f32& penetration);
    
    // Support function for Minkowski difference
    Vec3 support(const CollisionShape& shapeA, const Vec3& posA, const Quat& rotA,
                 const CollisionShape& shapeB, const Vec3& posB, const Quat& rotB,
                 const Vec3& direction);
};

// =============================================================================
// Constraint Solver Interface
// =============================================================================

/**
 * @brief Constraint solver interface
 */
class ConstraintSolver {
public:
    virtual ~ConstraintSolver() = default;
    
    /**
     * @brief Solve velocity constraints
     */
    virtual void solveVelocities(std::vector<RigidBody*>& bodies, 
                                  std::vector<ContactManifold>& contacts,
                                  f32 deltaTime) = 0;
    
    /**
     * @brief Solve position constraints
     */
    virtual void solvePositions(std::vector<RigidBody*>& bodies,
                                 std::vector<ContactManifold>& contacts,
                                 f32 deltaTime) = 0;
};

/**
 * @brief Sequential impulse constraint solver
 */
class SequentialImpulseSolver : public ConstraintSolver {
public:
    explicit SequentialImpulseSolver(u32 velocityIterations = 8, u32 positionIterations = 3);
    
    void solveVelocities(std::vector<RigidBody*>& bodies,
                         std::vector<ContactManifold>& contacts,
                         f32 deltaTime) override;
    
    void solvePositions(std::vector<RigidBody*>& bodies,
                        std::vector<ContactManifold>& contacts,
                        f32 deltaTime) override;
    
    void setVelocityIterations(u32 iterations) { m_velocityIterations = iterations; }
    void setPositionIterations(u32 iterations) { m_positionIterations = iterations; }

private:
    u32 m_velocityIterations;
    u32 m_positionIterations;
    
    void warmStart(std::vector<RigidBody*>& bodies, std::vector<ContactManifold>& contacts);
    void solveVelocityConstraint(RigidBody* bodyA, RigidBody* bodyB, ContactPoint& contact, 
                                  const Vec3& normal, f32 friction);
    void solvePositionConstraint(RigidBody* bodyA, RigidBody* bodyB, const ContactPoint& contact,
                                  const Vec3& normal);
};

} // namespace nova::physics
