/**
 * @file test_ecs.cpp
 * @brief Comprehensive tests for NovaCore ECS (Entity-Component-Worker)
 * 
 * Tests cover:
 * - Entity creation and destruction
 * - Component addition, removal, and access
 * - Archetype management
 * - World operations
 * - System scheduling
 * - Query iteration
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "nova/core/ecs/ecs.hpp"
#include <vector>
#include <algorithm>

using namespace nova;
using namespace nova::ecs;

// ============================================================================
// Test Components
// ============================================================================

struct Position {
    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 z = 0.0f;
    
    Position() = default;
    Position(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}
    
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct Velocity {
    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 z = 0.0f;
    
    Velocity() = default;
    Velocity(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}
};

struct Health {
    i32 current = 100;
    i32 max = 100;
    
    Health() = default;
    Health(i32 current, i32 max) : current(current), max(max) {}
};

struct Name {
    std::string value;
    
    Name() = default;
    explicit Name(std::string v) : value(std::move(v)) {}
};

struct Tag {}; // Empty tag component

// ============================================================================
// Entity Tests
// ============================================================================

TEST_CASE("Entity creation and properties", "[ecs][entity]") {
    SECTION("Default entity is invalid") {
        Entity e;
        REQUIRE_FALSE(e.isValid());
        REQUIRE(e.id() == Entity::INVALID_ID);
    }
    
    SECTION("Entity from index and generation") {
        Entity e(42, 7, Entity::FLAG_ALIVE);
        REQUIRE(e.index() == 42);
        REQUIRE(e.generation() == 7);
        REQUIRE(e.isValid());
    }
    
    SECTION("Entity comparison") {
        Entity a(1, 0, 0);
        Entity b(1, 0, 0);
        Entity c(2, 0, 0);
        
        REQUIRE(a == b);
        REQUIRE(a != c);
        REQUIRE(a < c);
    }
    
    SECTION("Invalid entity constant") {
        Entity invalid = Entity::invalid();
        REQUIRE_FALSE(invalid.isValid());
    }
}

TEST_CASE("EntityManager operations", "[ecs][entity]") {
    EntityManager manager;
    
    SECTION("Create entities") {
        Entity e1 = manager.create();
        Entity e2 = manager.create();
        Entity e3 = manager.create();
        
        REQUIRE(manager.isValid(e1));
        REQUIRE(manager.isValid(e2));
        REQUIRE(manager.isValid(e3));
        REQUIRE(manager.aliveCount() == 3);
        
        // Different indices
        REQUIRE(e1.index() != e2.index());
        REQUIRE(e2.index() != e3.index());
    }
    
    SECTION("Destroy entities") {
        Entity e1 = manager.create();
        Entity e2 = manager.create();
        
        REQUIRE(manager.destroy(e1));
        REQUIRE_FALSE(manager.isValid(e1));
        REQUIRE(manager.isValid(e2));
        REQUIRE(manager.aliveCount() == 1);
        
        // Cannot destroy again
        REQUIRE_FALSE(manager.destroy(e1));
    }
    
    SECTION("Entity recycling with generation") {
        Entity e1 = manager.create();
        u32 oldIndex = e1.index();
        u32 oldGen = e1.generation();
        
        manager.destroy(e1);
        
        Entity e2 = manager.create();
        
        // Same index, different generation
        REQUIRE(e2.index() == oldIndex);
        REQUIRE(e2.generation() == oldGen + 1);
        
        // Old entity reference is now invalid
        REQUIRE_FALSE(manager.isValid(e1));
        REQUIRE(manager.isValid(e2));
    }
    
    SECTION("Lock prevents destruction") {
        Entity e = manager.create();
        
        REQUIRE(manager.lock(e));
        REQUIRE_FALSE(manager.destroy(e));
        REQUIRE(manager.isValid(e));
        
        REQUIRE(manager.unlock(e));
        REQUIRE(manager.destroy(e));
        REQUIRE_FALSE(manager.isValid(e));
    }
    
    SECTION("Clear resets manager") {
        manager.create();
        manager.create();
        manager.create();
        
        REQUIRE(manager.aliveCount() == 3);
        
        manager.clear();
        
        REQUIRE(manager.aliveCount() == 0);
        REQUIRE(manager.capacity() == 0);
    }
}

// ============================================================================
// Component Tests
// ============================================================================

TEST_CASE("Component type identification", "[ecs][component]") {
    SECTION("Unique component IDs") {
        ComponentId posId = componentId<Position>();
        ComponentId velId = componentId<Velocity>();
        ComponentId healthId = componentId<Health>();
        
        REQUIRE(posId != velId);
        REQUIRE(velId != healthId);
        REQUIRE(posId != healthId);
    }
    
    SECTION("Same type returns same ID") {
        ComponentId id1 = componentId<Position>();
        ComponentId id2 = componentId<Position>();
        
        REQUIRE(id1 == id2);
    }
    
    SECTION("Component info accuracy") {
        const ComponentInfo& info = componentInfo<Position>();
        
        REQUIRE(info.size == sizeof(Position));
        REQUIRE(info.alignment == alignof(Position));
        REQUIRE(info.isTrivial == std::is_trivially_copyable_v<Position>);
    }
}

TEST_CASE("ComponentMask operations", "[ecs][component]") {
    SECTION("Set and test bits") {
        ComponentMask mask;
        
        mask.set(0);
        mask.set(5);
        mask.set(63);
        mask.set(64);
        mask.set(127);
        
        REQUIRE(mask.test(0));
        REQUIRE(mask.test(5));
        REQUIRE(mask.test(63));
        REQUIRE(mask.test(64));
        REQUIRE(mask.test(127));
        REQUIRE_FALSE(mask.test(1));
        REQUIRE_FALSE(mask.test(100));
    }
    
    SECTION("Clear bits") {
        ComponentMask mask;
        mask.set(5);
        REQUIRE(mask.test(5));
        
        mask.clear(5);
        REQUIRE_FALSE(mask.test(5));
    }
    
    SECTION("ContainsAll") {
        ComponentMask a;
        a.set(1);
        a.set(2);
        a.set(3);
        
        ComponentMask b;
        b.set(1);
        b.set(2);
        
        REQUIRE(a.containsAll(b));
        REQUIRE_FALSE(b.containsAll(a));
    }
    
    SECTION("ContainsAny") {
        ComponentMask a;
        a.set(1);
        a.set(2);
        
        ComponentMask b;
        b.set(2);
        b.set(3);
        
        REQUIRE(a.containsAny(b));
        
        ComponentMask c;
        c.set(10);
        c.set(11);
        
        REQUIRE_FALSE(a.containsAny(c));
    }
    
    SECTION("Count") {
        ComponentMask mask;
        REQUIRE(mask.count() == 0);
        
        mask.set(1);
        mask.set(50);
        mask.set(100);
        REQUIRE(mask.count() == 3);
    }
    
    SECTION("Create from types") {
        auto mask = ComponentMask::create<Position, Velocity>();
        
        REQUIRE(mask.test(componentId<Position>()));
        REQUIRE(mask.test(componentId<Velocity>()));
        REQUIRE_FALSE(mask.test(componentId<Health>()));
    }
    
    SECTION("Mask operations") {
        ComponentMask a = ComponentMask::create<Position, Velocity>();
        ComponentMask b = ComponentMask::create<Velocity, Health>();
        
        ComponentMask unionMask = a | b;
        REQUIRE(unionMask.test(componentId<Position>()));
        REQUIRE(unionMask.test(componentId<Velocity>()));
        REQUIRE(unionMask.test(componentId<Health>()));
        
        ComponentMask intersection = a & b;
        REQUIRE_FALSE(intersection.test(componentId<Position>()));
        REQUIRE(intersection.test(componentId<Velocity>()));
        REQUIRE_FALSE(intersection.test(componentId<Health>()));
        
        ComponentMask diff = a - b;
        REQUIRE(diff.test(componentId<Position>()));
        REQUIRE_FALSE(diff.test(componentId<Velocity>()));
    }
}

// ============================================================================
// World Tests
// ============================================================================

TEST_CASE("World entity management", "[ecs][world]") {
    World world;
    
    SECTION("Create entity") {
        Entity e = world.createEntity();
        REQUIRE(world.isValid(e));
        REQUIRE(world.entityCount() == 1);
    }
    
    SECTION("Create entity with components") {
        Entity e = world.createEntity(Position{1.0f, 2.0f, 3.0f}, Velocity{0.1f, 0.2f, 0.3f});
        
        REQUIRE(world.isValid(e));
        REQUIRE(world.hasComponent<Position>(e));
        REQUIRE(world.hasComponent<Velocity>(e));
        
        auto* pos = world.getComponent<Position>(e);
        REQUIRE(pos != nullptr);
        REQUIRE(pos->x == 1.0f);
        REQUIRE(pos->y == 2.0f);
        REQUIRE(pos->z == 3.0f);
    }
    
    SECTION("Destroy entity") {
        Entity e = world.createEntity();
        REQUIRE(world.destroyEntity(e));
        REQUIRE_FALSE(world.isValid(e));
        REQUIRE(world.entityCount() == 0);
    }
    
    SECTION("Multiple entities") {
        std::vector<Entity> entities;
        for (int i = 0; i < 100; ++i) {
            entities.push_back(world.createEntity());
        }
        
        REQUIRE(world.entityCount() == 100);
        
        for (const Entity& e : entities) {
            REQUIRE(world.isValid(e));
        }
    }
}

TEST_CASE("World component management", "[ecs][world]") {
    World world;
    
    SECTION("Add component") {
        Entity e = world.createEntity();
        
        Position& pos = world.addComponent<Position>(e, Position{1.0f, 2.0f, 3.0f});
        
        REQUIRE(world.hasComponent<Position>(e));
        REQUIRE(pos.x == 1.0f);
        REQUIRE(pos.y == 2.0f);
        REQUIRE(pos.z == 3.0f);
    }
    
    SECTION("Add multiple components") {
        Entity e = world.createEntity();
        
        world.addComponent<Position>(e, Position{1.0f, 2.0f, 3.0f});
        world.addComponent<Velocity>(e, Velocity{0.1f, 0.2f, 0.3f});
        world.addComponent<Health>(e, Health{50, 100});
        
        REQUIRE(world.hasComponents<Position, Velocity, Health>(e));
    }
    
    SECTION("Get component") {
        Entity e = world.createEntity(Position{5.0f, 10.0f, 15.0f});
        
        auto* pos = world.getComponent<Position>(e);
        REQUIRE(pos != nullptr);
        REQUIRE(pos->x == 5.0f);
        
        // Modify through pointer
        pos->x = 100.0f;
        REQUIRE(world.getComponent<Position>(e)->x == 100.0f);
    }
    
    SECTION("Remove component") {
        Entity e = world.createEntity(Position{}, Velocity{});
        
        REQUIRE(world.hasComponents<Position, Velocity>(e));
        
        REQUIRE(world.removeComponent<Velocity>(e));
        
        REQUIRE(world.hasComponent<Position>(e));
        REQUIRE_FALSE(world.hasComponent<Velocity>(e));
    }
    
    SECTION("Component on invalid entity returns null") {
        Entity invalid;
        REQUIRE(world.getComponent<Position>(invalid) == nullptr);
    }
    
    SECTION("Non-existent component returns null") {
        Entity e = world.createEntity();
        REQUIRE(world.getComponent<Position>(e) == nullptr);
    }
}

TEST_CASE("World iteration", "[ecs][world]") {
    World world;
    
    // Create entities with different component combinations
    for (int i = 0; i < 10; ++i) {
        world.createEntity(Position{static_cast<f32>(i), 0.0f, 0.0f});
    }
    
    for (int i = 0; i < 10; ++i) {
        world.createEntity(Position{static_cast<f32>(i), 0.0f, 0.0f}, Velocity{1.0f, 0.0f, 0.0f});
    }
    
    for (int i = 0; i < 5; ++i) {
        world.createEntity(Velocity{2.0f, 0.0f, 0.0f});
    }
    
    SECTION("Each with single component") {
        int count = 0;
        world.each<Position>([&count](Position& pos) {
            ++count;
            pos.y = 1.0f; // Modify to verify access
        });
        
        REQUIRE(count == 20); // 10 Position-only + 10 Position+Velocity
    }
    
    SECTION("Each with multiple components") {
        int count = 0;
        world.each<Position, Velocity>([&count](Position& pos, Velocity& vel) {
            ++count;
            pos.x += vel.x;
        });
        
        REQUIRE(count == 10); // Only entities with both
    }
    
    SECTION("Each with entity access") {
        std::vector<Entity> entities;
        world.eachWithEntity<Position>([&entities](Entity e, Position& pos) {
            entities.push_back(e);
        });
        
        REQUIRE(entities.size() == 20);
    }
    
    SECTION("Count matching entities") {
        REQUIRE(world.count<Position>() == 20);
        REQUIRE(world.count<Velocity>() == 15);
        REQUIRE(world.count<Position, Velocity>() == 10);
        REQUIRE(world.count<Health>() == 0);
    }
}

TEST_CASE("World frame management", "[ecs][world]") {
    World world;
    
    SECTION("Frame timing") {
        world.beginFrame(0.016f);
        
        REQUIRE(world.deltaTime() == 0.016f);
        REQUIRE(world.frameCount() == 1);
        
        world.endFrame();
        
        world.beginFrame(0.017f);
        REQUIRE(world.frameCount() == 2);
    }
    
    SECTION("Deferred destruction") {
        Entity e1 = world.createEntity();
        Entity e2 = world.createEntity();
        
        world.destroyEntity(e1, false); // Deferred
        
        REQUIRE(world.isValid(e1)); // Still valid during frame
        
        world.endFrame();
        
        REQUIRE_FALSE(world.isValid(e1)); // Destroyed after frame
        REQUIRE(world.isValid(e2));
    }
}

// ============================================================================
// System Tests
// ============================================================================

// Test system that updates position by velocity
class MovementSystem : public System {
public:
    int updateCount = 0;
    
    MovementSystem() {
        setName("MovementSystem");
        setPhase(SystemPhase::Update);
    }
    
    void update(const SystemContext& context) override {
        ++updateCount;
        context.world->each<Position, Velocity>([dt = context.deltaTime](Position& pos, Velocity& vel) {
            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            pos.z += vel.z * dt;
        });
    }
};

TEST_CASE("System scheduling", "[ecs][system]") {
    World world;
    SystemScheduler scheduler;
    
    SECTION("Register and execute system") {
        auto* movement = scheduler.registerSystem<MovementSystem>();
        
        REQUIRE(movement != nullptr);
        REQUIRE(scheduler.systemCount() == 1);
        
        scheduler.initialize(world);
        
        SystemContext context;
        context.world = &world;
        context.deltaTime = 0.016f;
        
        scheduler.executeAll(context);
        
        REQUIRE(movement->updateCount == 1);
    }
    
    SECTION("System affects world") {
        Entity e = world.createEntity(Position{0.0f, 0.0f, 0.0f}, Velocity{10.0f, 0.0f, 0.0f});
        
        scheduler.registerSystem<MovementSystem>();
        scheduler.initialize(world);
        
        SystemContext context;
        context.world = &world;
        context.deltaTime = 1.0f;
        
        scheduler.executeAll(context);
        
        auto* pos = world.getComponent<Position>(e);
        REQUIRE(pos->x == 10.0f);
    }
    
    SECTION("Lambda system") {
        int lambdaCount = 0;
        
        scheduler.registerLambda("TestLambda", [&lambdaCount](const SystemContext& ctx) {
            ++lambdaCount;
        });
        
        scheduler.initialize(world);
        
        SystemContext context;
        context.world = &world;
        context.deltaTime = 0.016f;
        
        scheduler.executeAll(context);
        
        REQUIRE(lambdaCount == 1);
    }
    
    SECTION("System phases") {
        std::vector<std::string> executionOrder;
        
        scheduler.registerLambda("PreUpdate", [&](const SystemContext&) {
            executionOrder.push_back("PreUpdate");
        }, SystemPhase::PreUpdate);
        
        scheduler.registerLambda("Update", [&](const SystemContext&) {
            executionOrder.push_back("Update");
        }, SystemPhase::Update);
        
        scheduler.registerLambda("PostUpdate", [&](const SystemContext&) {
            executionOrder.push_back("PostUpdate");
        }, SystemPhase::PostUpdate);
        
        scheduler.initialize(world);
        
        SystemContext context;
        context.world = &world;
        context.deltaTime = 0.016f;
        
        scheduler.executeAll(context);
        
        REQUIRE(executionOrder.size() == 3);
        REQUIRE(executionOrder[0] == "PreUpdate");
        REQUIRE(executionOrder[1] == "Update");
        REQUIRE(executionOrder[2] == "PostUpdate");
    }
    
    SECTION("Disable system") {
        int count = 0;
        auto* system = scheduler.registerLambda("Test", [&count](const SystemContext&) {
            ++count;
        });
        
        scheduler.initialize(world);
        
        SystemContext context;
        context.world = &world;
        context.deltaTime = 0.016f;
        
        scheduler.executeAll(context);
        REQUIRE(count == 1);
        
        system->setEnabled(false);
        
        scheduler.executeAll(context);
        REQUIRE(count == 1); // Should not increase
    }
}

// ============================================================================
// Query Tests
// ============================================================================

TEST_CASE("Query operations", "[ecs][query]") {
    World world;
    
    // Setup test data
    for (int i = 0; i < 5; ++i) {
        world.createEntity(Position{static_cast<f32>(i), 0.0f, 0.0f});
    }
    for (int i = 0; i < 5; ++i) {
        world.createEntity(Position{}, Velocity{});
    }
    for (int i = 0; i < 5; ++i) {
        world.createEntity(Velocity{});
    }
    
    SECTION("Basic query") {
        auto query = Query<Position>::create(world);
        
        REQUIRE(query.count() == 10); // 5 Position + 5 Position+Velocity
    }
    
    SECTION("Multiple component query") {
        auto query = Query<Position, Velocity>::create(world);
        
        REQUIRE(query.count() == 5);
    }
    
    SECTION("Query iteration") {
        auto query = Query<Position>::create(world);
        
        int count = 0;
        query.each([&count](Position& pos) {
            ++count;
        });
        
        REQUIRE(count == 10);
    }
    
    SECTION("Query with entity") {
        auto query = Query<Position>::create(world);
        
        std::vector<Entity> entities;
        query.eachWithEntity([&entities](Entity e, Position& pos) {
            entities.push_back(e);
        });
        
        REQUIRE(entities.size() == 10);
    }
}

// ============================================================================
// Performance / Stress Tests
// ============================================================================

TEST_CASE("Stress test: Many entities", "[ecs][stress]") {
    World world;
    
    constexpr int ENTITY_COUNT = 10000;
    
    SECTION("Create many entities") {
        std::vector<Entity> entities;
        entities.reserve(ENTITY_COUNT);
        
        for (int i = 0; i < ENTITY_COUNT; ++i) {
            entities.push_back(world.createEntity(
                Position{static_cast<f32>(i), 0.0f, 0.0f},
                Velocity{1.0f, 0.0f, 0.0f}
            ));
        }
        
        REQUIRE(world.entityCount() == ENTITY_COUNT);
        
        // Verify all are valid
        for (const Entity& e : entities) {
            REQUIRE(world.isValid(e));
        }
    }
    
    SECTION("Iterate many entities") {
        for (int i = 0; i < ENTITY_COUNT; ++i) {
            world.createEntity(
                Position{static_cast<f32>(i), 0.0f, 0.0f},
                Velocity{1.0f, 0.0f, 0.0f}
            );
        }
        
        int count = 0;
        world.each<Position, Velocity>([&count](Position& pos, Velocity& vel) {
            pos.x += vel.x;
            ++count;
        });
        
        REQUIRE(count == ENTITY_COUNT);
    }
    
    SECTION("Destroy many entities") {
        std::vector<Entity> entities;
        for (int i = 0; i < ENTITY_COUNT; ++i) {
            entities.push_back(world.createEntity());
        }
        
        REQUIRE(world.entityCount() == ENTITY_COUNT);
        
        for (const Entity& e : entities) {
            world.destroyEntity(e);
        }
        
        REQUIRE(world.entityCount() == 0);
    }
}

// ============================================================================
// Non-trivial Component Tests
// ============================================================================

TEST_CASE("Non-trivial components", "[ecs][component]") {
    World world;
    
    SECTION("String component") {
        Entity e = world.createEntity(Name{"TestEntity"});
        
        auto* name = world.getComponent<Name>(e);
        REQUIRE(name != nullptr);
        REQUIRE(name->value == "TestEntity");
    }
    
    SECTION("String component modification") {
        Entity e = world.createEntity(Name{"Original"});
        
        auto* name = world.getComponent<Name>(e);
        name->value = "Modified";
        
        REQUIRE(world.getComponent<Name>(e)->value == "Modified");
    }
    
    SECTION("String component removal") {
        Entity e = world.createEntity(Name{"ToRemove"}, Position{});
        
        REQUIRE(world.hasComponent<Name>(e));
        REQUIRE(world.removeComponent<Name>(e));
        REQUIRE_FALSE(world.hasComponent<Name>(e));
        REQUIRE(world.hasComponent<Position>(e));
    }
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_CASE("Edge cases", "[ecs][edge]") {
    World world;
    
    SECTION("Empty world operations") {
        REQUIRE(world.entityCount() == 0);
        
        int count = 0;
        world.each<Position>([&count](Position&) { ++count; });
        REQUIRE(count == 0);
    }
    
    SECTION("Single entity with many components") {
        Entity e = world.createEntity(Position{}, Velocity{}, Health{}, Name{"Many"});
        
        REQUIRE(world.hasComponents<Position, Velocity, Health, Name>(e));
    }
    
    SECTION("Add same component twice") {
        Entity e = world.createEntity(Position{1.0f, 0.0f, 0.0f});
        
        // Adding again should update, not duplicate
        world.addComponent<Position>(e, Position{2.0f, 0.0f, 0.0f});
        
        auto* pos = world.getComponent<Position>(e);
        REQUIRE(pos->x == 2.0f);
    }
    
    SECTION("Remove non-existent component") {
        Entity e = world.createEntity();
        
        REQUIRE_FALSE(world.removeComponent<Position>(e));
    }
    
    SECTION("Tag component (zero-size)") {
        Entity e = world.createEntity();
        world.addComponent<Tag>(e);
        
        REQUIRE(world.hasComponent<Tag>(e));
    }
    
    SECTION("Clear and reuse") {
        for (int i = 0; i < 100; ++i) {
            world.createEntity(Position{});
        }
        
        world.clear();
        REQUIRE(world.entityCount() == 0);
        
        // Should work after clear
        Entity e = world.createEntity(Position{});
        REQUIRE(world.isValid(e));
        REQUIRE(world.hasComponent<Position>(e));
    }
}
