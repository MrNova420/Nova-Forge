/**
 * @file system.hpp
 * @brief System interface and scheduling for NovaCore ECS
 * 
 * Implements the Worker part of the Entity-Component-Worker (ECW) pattern.
 * Systems process entities with specific component combinations.
 * 
 * Features:
 * - Type-safe system registration
 * - Automatic dependency ordering
 * - Parallel execution support
 * - System groups for logical organization
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "entity.hpp"
#include "component.hpp"
#include "query.hpp"
#include "nova/core/types/types.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <queue>

namespace nova::ecs {

// Forward declarations
class World;
class SystemScheduler;

/**
 * @brief System execution phase
 */
enum class SystemPhase : u8 {
    PreUpdate,      ///< Before main update (input, events)
    Update,         ///< Main update (gameplay, AI)
    PostUpdate,     ///< After update (cleanup, validation)
    PreRender,      ///< Before rendering (culling, LOD)
    Render,         ///< Rendering commands
    PostRender      ///< After rendering (debug, UI overlay)
};

/**
 * @brief System execution context
 */
struct SystemContext {
    World* world = nullptr;
    f32 deltaTime = 0.0f;
    u64 frameCount = 0;
};

/**
 * @brief Base class for systems
 * 
 * Systems are the "Workers" in ECW - they process entities with specific
 * component combinations. Override update() to implement system logic.
 */
class System {
protected:
    /// System name for debugging
    std::string m_name;
    
    /// Execution phase
    SystemPhase m_phase = SystemPhase::Update;
    
    /// System enabled state
    bool m_enabled = true;
    
    /// Execution order within phase (lower = earlier)
    i32 m_order = 0;
    
    /// Systems this system must run after
    std::vector<std::string> m_dependencies;
    
public:
    /// Virtual destructor
    virtual ~System() = default;
    
    /**
     * @brief Called once when system is registered
     * @param world The world this system belongs to
     */
    virtual void initialize([[maybe_unused]] World& world) {}
    
    /**
     * @brief Called every frame to update the system
     * @param context Execution context with world and timing
     */
    virtual void update(const SystemContext& context) = 0;
    
    /**
     * @brief Called when system is removed
     * @param world The world this system belongs to
     */
    virtual void shutdown([[maybe_unused]] World& world) {}
    
    // Accessors
    [[nodiscard]] const std::string& name() const noexcept { return m_name; }
    [[nodiscard]] SystemPhase phase() const noexcept { return m_phase; }
    [[nodiscard]] bool enabled() const noexcept { return m_enabled; }
    [[nodiscard]] i32 order() const noexcept { return m_order; }
    [[nodiscard]] const std::vector<std::string>& dependencies() const noexcept { return m_dependencies; }
    
    // Mutators
    void setEnabled(bool enabled) noexcept { m_enabled = enabled; }
    void setOrder(i32 order) noexcept { m_order = order; }
    void setPhase(SystemPhase phase) noexcept { m_phase = phase; }
    
    /// Add a dependency (system that must run before this one)
    void addDependency(const std::string& systemName) {
        m_dependencies.push_back(systemName);
    }
    
protected:
    /// Set the system name (call in constructor)
    void setName(std::string name) { m_name = std::move(name); }
};

/**
 * @brief Lambda-based system for quick prototyping
 */
class LambdaSystem : public System {
private:
    std::function<void(const SystemContext&)> m_updateFn;
    
public:
    LambdaSystem(std::string name, std::function<void(const SystemContext&)> updateFn,
                 SystemPhase phase = SystemPhase::Update)
        : m_updateFn(std::move(updateFn)) {
        setName(std::move(name));
        setPhase(phase);
    }
    
    void update(const SystemContext& context) override {
        if (m_updateFn) {
            m_updateFn(context);
        }
    }
};

/**
 * @brief System group for organizing related systems
 */
class SystemGroup {
private:
    std::string m_name;
    std::vector<System*> m_systems;
    bool m_enabled = true;
    
public:
    explicit SystemGroup(std::string name) : m_name(std::move(name)) {}
    
    void add(System* system) {
        m_systems.push_back(system);
    }
    
    void remove(System* system) {
        m_systems.erase(
            std::remove(m_systems.begin(), m_systems.end(), system),
            m_systems.end()
        );
    }
    
    [[nodiscard]] const std::string& name() const noexcept { return m_name; }
    [[nodiscard]] const std::vector<System*>& systems() const noexcept { return m_systems; }
    [[nodiscard]] bool enabled() const noexcept { return m_enabled; }
    
    void setEnabled(bool enabled) noexcept { m_enabled = enabled; }
};

/**
 * @brief Schedules and executes systems
 * 
 * Handles:
 * - System registration and lifecycle
 * - Dependency resolution
 * - Phase-based execution
 * - Parallel execution (future)
 */
class SystemScheduler {
private:
    /// All registered systems
    std::vector<std::unique_ptr<System>> m_systems;
    
    /// Name to system mapping
    std::unordered_map<std::string, System*> m_systemsByName;
    
    /// Systems sorted by phase and order
    std::vector<System*> m_executionOrder;
    
    /// System groups
    std::unordered_map<std::string, SystemGroup> m_groups;
    
    /// Whether execution order needs recalculation
    bool m_dirty = true;
    
public:
    /// Default constructor
    SystemScheduler() = default;
    
    /**
     * @brief Register a system
     * @tparam T System type
     * @param args Constructor arguments
     * @return Pointer to the registered system
     */
    template<typename T, typename... Args>
    T* registerSystem(Args&&... args) {
        static_assert(std::is_base_of_v<System, T>, "T must derive from System");
        
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = system.get();
        
        m_systemsByName[ptr->name()] = ptr;
        m_systems.push_back(std::move(system));
        m_dirty = true;
        
        return ptr;
    }
    
    /**
     * @brief Register a lambda system
     */
    System* registerLambda(std::string name, 
                           std::function<void(const SystemContext&)> updateFn,
                           SystemPhase phase = SystemPhase::Update) {
        return registerSystem<LambdaSystem>(std::move(name), std::move(updateFn), phase);
    }
    
    /**
     * @brief Unregister a system by name
     */
    bool unregisterSystem(const std::string& name) {
        auto it = m_systemsByName.find(name);
        if (it == m_systemsByName.end()) {
            return false;
        }
        
        System* system = it->second;
        m_systemsByName.erase(it);
        
        // Remove from groups
        for (auto& [groupName, group] : m_groups) {
            group.remove(system);
        }
        
        // Remove from systems vector
        m_systems.erase(
            std::remove_if(m_systems.begin(), m_systems.end(),
                          [system](const auto& s) { return s.get() == system; }),
            m_systems.end()
        );
        
        m_dirty = true;
        return true;
    }
    
    /**
     * @brief Get a system by name
     */
    [[nodiscard]] System* getSystem(const std::string& name) {
        auto it = m_systemsByName.find(name);
        return (it != m_systemsByName.end()) ? it->second : nullptr;
    }
    
    /**
     * @brief Get a typed system by name
     */
    template<typename T>
    [[nodiscard]] T* getSystem(const std::string& name) {
        return dynamic_cast<T*>(getSystem(name));
    }
    
    /**
     * @brief Create a system group
     */
    SystemGroup& createGroup(const std::string& name) {
        return m_groups.emplace(name, SystemGroup(name)).first->second;
    }
    
    /**
     * @brief Get a system group
     */
    [[nodiscard]] SystemGroup* getGroup(const std::string& name) {
        auto it = m_groups.find(name);
        return (it != m_groups.end()) ? &it->second : nullptr;
    }
    
    /**
     * @brief Initialize all systems
     */
    void initialize(World& world) {
        rebuildExecutionOrder();
        
        for (System* system : m_executionOrder) {
            system->initialize(world);
        }
    }
    
    /**
     * @brief Execute all systems for a specific phase
     */
    void execute(SystemPhase phase, const SystemContext& context) {
        if (m_dirty) {
            rebuildExecutionOrder();
        }
        
        for (System* system : m_executionOrder) {
            if (system->phase() == phase && system->enabled()) {
                system->update(context);
            }
        }
    }
    
    /**
     * @brief Execute all systems for all phases
     */
    void executeAll(const SystemContext& context) {
        if (m_dirty) {
            rebuildExecutionOrder();
        }
        
        for (System* system : m_executionOrder) {
            if (system->enabled()) {
                system->update(context);
            }
        }
    }
    
    /**
     * @brief Shutdown all systems
     */
    void shutdown(World& world) {
        // Shutdown in reverse order
        for (auto it = m_executionOrder.rbegin(); it != m_executionOrder.rend(); ++it) {
            (*it)->shutdown(world);
        }
    }
    
    /**
     * @brief Get all systems
     */
    [[nodiscard]] const std::vector<std::unique_ptr<System>>& systems() const noexcept {
        return m_systems;
    }
    
    /**
     * @brief Get system count
     */
    [[nodiscard]] usize systemCount() const noexcept {
        return m_systems.size();
    }
    
private:
    /// Rebuild execution order based on phases, order, and dependencies
    void rebuildExecutionOrder() {
        m_executionOrder.clear();
        m_executionOrder.reserve(m_systems.size());
        
        // Group systems by phase
        std::unordered_map<SystemPhase, std::vector<System*>> phaseGroups;
        for (auto& system : m_systems) {
            phaseGroups[system->phase()].push_back(system.get());
        }
        
        // Process each phase in order
        for (SystemPhase phase : {SystemPhase::PreUpdate, SystemPhase::Update, 
                                   SystemPhase::PostUpdate, SystemPhase::PreRender,
                                   SystemPhase::Render, SystemPhase::PostRender}) {
            auto& systems = phaseGroups[phase];
            if (systems.empty()) continue;
            
            // Topological sort within each phase using Kahn's algorithm
            // Build adjacency list and in-degree count
            std::unordered_map<System*, std::vector<System*>> adjacency;
            std::unordered_map<System*, i32> inDegree;
            
            for (System* sys : systems) {
                adjacency[sys] = {};
                inDegree[sys] = 0;
            }
            
            // Build edges: if A depends on B, then B -> A (B must run before A)
            for (System* sys : systems) {
                for (const auto& depName : sys->dependencies()) {
                    auto depIt = m_systemsByName.find(depName);
                    if (depIt != m_systemsByName.end() && depIt->second->phase() == phase) {
                        System* dep = depIt->second;
                        adjacency[dep].push_back(sys);
                        inDegree[sys]++;
                    }
                }
            }
            
            // Kahn's algorithm with priority queue for deterministic ordering
            // Use a min-heap based on (order, name) for stable sorting
            auto compare = [](System* a, System* b) {
                if (a->order() != b->order()) {
                    return a->order() > b->order();  // min-heap, so greater means lower priority
                }
                return a->name() > b->name();
            };
            std::priority_queue<System*, std::vector<System*>, decltype(compare)> ready(compare);
            
            // Start with systems that have no dependencies
            for (System* sys : systems) {
                if (inDegree[sys] == 0) {
                    ready.push(sys);
                }
            }
            
            std::vector<System*> sorted;
            sorted.reserve(systems.size());
            
            while (!ready.empty()) {
                System* current = ready.top();
                ready.pop();
                sorted.push_back(current);
                
                for (System* dependent : adjacency[current]) {
                    inDegree[dependent]--;
                    if (inDegree[dependent] == 0) {
                        ready.push(dependent);
                    }
                }
            }
            
            // Check for cycles (not all systems processed)
            if (sorted.size() != systems.size()) {
                // Cycle detected - fall back to order-based sort for remaining systems
                for (System* sys : systems) {
                    if (std::find(sorted.begin(), sorted.end(), sys) == sorted.end()) {
                        sorted.push_back(sys);
                    }
                }
            }
            
            // Add sorted systems to execution order
            for (System* sys : sorted) {
                m_executionOrder.push_back(sys);
            }
        }
        
        m_dirty = false;
    }
};

/**
 * @brief Macro to create a simple system class
 */
#define NOVA_SYSTEM(SystemName, PhaseValue) \
    class SystemName : public nova::ecs::System { \
    public: \
        SystemName() { \
            setName(#SystemName); \
            setPhase(PhaseValue); \
        } \
        void update(const nova::ecs::SystemContext& context) override

#define NOVA_SYSTEM_END };

} // namespace nova::ecs
