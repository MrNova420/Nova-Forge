/**
 * @file component.hpp
 * @brief Component type registration and metadata for NovaCore ECS
 * 
 * Provides compile-time component type identification and runtime
 * component metadata for the archetype-based ECS.
 * 
 * Features:
 * - Compile-time type IDs using template metaprogramming
 * - Runtime component metadata (size, alignment, constructor, destructor)
 * - Support for trivial and non-trivial components
 * - Component type registry for reflection
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "nova/core/types/types.hpp"
#include <typeinfo>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <cstring>
#include <type_traits>
#include <atomic>

namespace nova::ecs {

/// Maximum number of component types supported
constexpr usize MAX_COMPONENT_TYPES = 256;

/**
 * @brief Unique identifier for component types
 * 
 * Each component type gets a unique ID at compile time.
 * IDs are assigned sequentially starting from 0.
 */
using ComponentId = u32;

/// Invalid component ID sentinel
constexpr ComponentId INVALID_COMPONENT_ID = ~0u;

/**
 * @brief Runtime metadata for a component type
 */
struct ComponentInfo {
    /// Unique component type ID
    ComponentId id = INVALID_COMPONENT_ID;
    
    /// Size of the component in bytes
    usize size = 0;
    
    /// Alignment requirement in bytes
    usize alignment = 0;
    
    /// Type name (for debugging/reflection)
    const char* name = nullptr;
    
    /// Hash of the type (for serialization)
    u64 typeHash = 0;
    
    /// True if the component is trivially copyable
    bool isTrivial = false;
    
    /// Function to default-construct a component at a memory location
    std::function<void(void*)> construct = nullptr;
    
    /// Function to destroy a component at a memory location
    std::function<void(void*)> destruct = nullptr;
    
    /// Function to copy-construct a component
    std::function<void(void* dst, const void* src)> copyConstruct = nullptr;
    
    /// Function to move-construct a component
    std::function<void(void* dst, void* src)> moveConstruct = nullptr;
    
    /// Function to copy-assign a component
    std::function<void(void* dst, const void* src)> copyAssign = nullptr;
    
    /// Function to move-assign a component
    std::function<void(void* dst, void* src)> moveAssign = nullptr;
};

/**
 * @brief Global component type ID counter
 */
inline std::atomic<ComponentId> g_nextComponentId{0};

/**
 * @brief Get the next available component type ID
 * @return A unique component type ID
 */
inline ComponentId getNextComponentId() {
    return g_nextComponentId.fetch_add(1, std::memory_order_relaxed);
}

/**
 * @brief Component type traits providing compile-time type ID
 * 
 * Each component type T gets a unique ID through template specialization.
 * The ID is generated lazily on first access.
 */
template<typename T>
struct ComponentType {
    static_assert(std::is_same_v<T, std::remove_cv_t<std::remove_reference_t<T>>>,
                  "Component type must be a non-cv, non-reference type");
    
    /// Get the unique ID for this component type
    static ComponentId id() {
        static const ComponentId s_id = getNextComponentId();
        return s_id;
    }
    
    /// Get component info for this type
    static const ComponentInfo& info() {
        static const ComponentInfo s_info = makeInfo();
        return s_info;
    }
    
private:
    static ComponentInfo makeInfo() {
        ComponentInfo info;
        info.id = id();
        info.size = sizeof(T);
        info.alignment = alignof(T);
        info.name = typeid(T).name();
        info.typeHash = runtimeHash(typeid(T).name());
        info.isTrivial = std::is_trivially_copyable_v<T>;
        
        // Setup construction/destruction functions
        if constexpr (std::is_default_constructible_v<T>) {
            info.construct = [](void* ptr) {
                new (ptr) T();
            };
        }
        
        if constexpr (std::is_destructible_v<T> && !std::is_trivially_destructible_v<T>) {
            info.destruct = [](void* ptr) {
                static_cast<T*>(ptr)->~T();
            };
        } else {
            info.destruct = [](void*) {}; // No-op for trivial types
        }
        
        if constexpr (std::is_copy_constructible_v<T>) {
            info.copyConstruct = [](void* dst, const void* src) {
                new (dst) T(*static_cast<const T*>(src));
            };
        }
        
        if constexpr (std::is_move_constructible_v<T>) {
            info.moveConstruct = [](void* dst, void* src) {
                new (dst) T(std::move(*static_cast<T*>(src)));
            };
        }
        
        if constexpr (std::is_copy_assignable_v<T>) {
            info.copyAssign = [](void* dst, const void* src) {
                *static_cast<T*>(dst) = *static_cast<const T*>(src);
            };
        }
        
        if constexpr (std::is_move_assignable_v<T>) {
            info.moveAssign = [](void* dst, void* src) {
                *static_cast<T*>(dst) = std::move(*static_cast<T*>(src));
            };
        }
        
        return info;
    }
};

/**
 * @brief Helper function to get component ID for a type
 */
template<typename T>
inline ComponentId componentId() {
    return ComponentType<std::remove_cv_t<std::remove_reference_t<T>>>::id();
}

/**
 * @brief Helper function to get component info for a type
 */
template<typename T>
inline const ComponentInfo& componentInfo() {
    return ComponentType<std::remove_cv_t<std::remove_reference_t<T>>>::info();
}

/**
 * @brief Component type signature (set of component IDs)
 * 
 * Used to identify archetypes and query matching.
 * Implemented as a fixed-size bitset for fast operations.
 */
class ComponentMask {
private:
    /// Bit storage (256 bits = 4 x 64-bit words)
    static constexpr usize WORD_COUNT = (MAX_COMPONENT_TYPES + 63) / 64;
    u64 m_bits[WORD_COUNT] = {};
    
public:
    /// Default constructor - empty mask
    constexpr ComponentMask() noexcept = default;
    
    /// Set a component bit
    constexpr void set(ComponentId id) noexcept {
        if (id < MAX_COMPONENT_TYPES) {
            m_bits[id / 64] |= (1ULL << (id % 64));
        }
    }
    
    /// Clear a component bit
    constexpr void clear(ComponentId id) noexcept {
        if (id < MAX_COMPONENT_TYPES) {
            m_bits[id / 64] &= ~(1ULL << (id % 64));
        }
    }
    
    /// Test if a component bit is set
    [[nodiscard]] constexpr bool test(ComponentId id) const noexcept {
        if (id >= MAX_COMPONENT_TYPES) return false;
        return (m_bits[id / 64] & (1ULL << (id % 64))) != 0;
    }
    
    /// Check if this mask contains all components in another mask
    [[nodiscard]] constexpr bool containsAll(const ComponentMask& other) const noexcept {
        for (usize i = 0; i < WORD_COUNT; ++i) {
            if ((m_bits[i] & other.m_bits[i]) != other.m_bits[i]) {
                return false;
            }
        }
        return true;
    }
    
    /// Check if this mask contains any components in another mask
    [[nodiscard]] constexpr bool containsAny(const ComponentMask& other) const noexcept {
        for (usize i = 0; i < WORD_COUNT; ++i) {
            if ((m_bits[i] & other.m_bits[i]) != 0) {
                return true;
            }
        }
        return false;
    }
    
    /// Check if this mask is empty
    [[nodiscard]] constexpr bool empty() const noexcept {
        for (usize i = 0; i < WORD_COUNT; ++i) {
            if (m_bits[i] != 0) return false;
        }
        return true;
    }
    
    /// Count the number of set bits
    [[nodiscard]] constexpr usize count() const noexcept {
        usize result = 0;
        for (usize i = 0; i < WORD_COUNT; ++i) {
            // Portable popcount
            u64 x = m_bits[i];
            while (x) {
                x &= (x - 1);
                ++result;
            }
        }
        return result;
    }
    
    /// Union of two masks
    [[nodiscard]] constexpr ComponentMask operator|(const ComponentMask& other) const noexcept {
        ComponentMask result;
        for (usize i = 0; i < WORD_COUNT; ++i) {
            result.m_bits[i] = m_bits[i] | other.m_bits[i];
        }
        return result;
    }
    
    /// Intersection of two masks
    [[nodiscard]] constexpr ComponentMask operator&(const ComponentMask& other) const noexcept {
        ComponentMask result;
        for (usize i = 0; i < WORD_COUNT; ++i) {
            result.m_bits[i] = m_bits[i] & other.m_bits[i];
        }
        return result;
    }
    
    /// Difference of two masks (this - other)
    [[nodiscard]] constexpr ComponentMask operator-(const ComponentMask& other) const noexcept {
        ComponentMask result;
        for (usize i = 0; i < WORD_COUNT; ++i) {
            result.m_bits[i] = m_bits[i] & ~other.m_bits[i];
        }
        return result;
    }
    
    /// Equality comparison
    [[nodiscard]] constexpr bool operator==(const ComponentMask& other) const noexcept {
        for (usize i = 0; i < WORD_COUNT; ++i) {
            if (m_bits[i] != other.m_bits[i]) return false;
        }
        return true;
    }
    
    [[nodiscard]] constexpr bool operator!=(const ComponentMask& other) const noexcept {
        return !(*this == other);
    }
    
    /// Hash for use in containers
    [[nodiscard]] u64 hash() const noexcept {
        u64 h = 0;
        for (usize i = 0; i < WORD_COUNT; ++i) {
            h ^= m_bits[i] * 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        }
        return h;
    }
    
    /// Get a list of component IDs in this mask
    [[nodiscard]] std::vector<ComponentId> toVector() const {
        std::vector<ComponentId> result;
        for (usize i = 0; i < WORD_COUNT; ++i) {
            u64 word = m_bits[i];
            u32 base = static_cast<u32>(i * 64);
            while (word) {
                u32 bit = static_cast<u32>(__builtin_ctzll(word)); // Count trailing zeros
                result.push_back(base + bit);
                word &= (word - 1); // Clear lowest set bit
            }
        }
        return result;
    }
    
    /// Create a mask from a list of component types
    template<typename... Components>
    static ComponentMask create() {
        ComponentMask mask;
        (mask.set(componentId<Components>()), ...);
        return mask;
    }
};

/**
 * @brief Component registry for runtime type information
 */
class ComponentRegistry {
private:
    std::vector<const ComponentInfo*> m_infos;
    std::unordered_map<u64, ComponentId> m_hashToId;
    std::unordered_map<std::string, ComponentId> m_nameToId;
    
public:
    /// Get the singleton instance
    static ComponentRegistry& instance() {
        static ComponentRegistry s_instance;
        return s_instance;
    }
    
    /// Register a component type
    template<typename T>
    void registerComponent() {
        const auto& info = componentInfo<T>();
        ensureCapacity(info.id + 1);
        m_infos[info.id] = &info;
        m_hashToId[info.typeHash] = info.id;
        m_nameToId[info.name] = info.id;
    }
    
    /// Get component info by ID
    [[nodiscard]] const ComponentInfo* getInfo(ComponentId id) const {
        if (id < m_infos.size()) {
            return m_infos[id];
        }
        return nullptr;
    }
    
    /// Get component ID by type hash
    [[nodiscard]] ComponentId getIdByHash(u64 hash) const {
        auto it = m_hashToId.find(hash);
        return (it != m_hashToId.end()) ? it->second : INVALID_COMPONENT_ID;
    }
    
    /// Get component ID by name
    [[nodiscard]] ComponentId getIdByName(const std::string& name) const {
        auto it = m_nameToId.find(name);
        return (it != m_nameToId.end()) ? it->second : INVALID_COMPONENT_ID;
    }
    
private:
    ComponentRegistry() {
        m_infos.reserve(64);
    }
    
    void ensureCapacity(usize capacity) {
        if (m_infos.size() < capacity) {
            m_infos.resize(capacity, nullptr);
        }
    }
};

/**
 * @brief Helper macro to register a component type
 */
#define NOVA_REGISTER_COMPONENT(Type) \
    nova::ecs::ComponentRegistry::instance().registerComponent<Type>()

} // namespace nova::ecs

// Hash specialization for ComponentMask
namespace std {
    template<>
    struct hash<nova::ecs::ComponentMask> {
        size_t operator()(const nova::ecs::ComponentMask& mask) const noexcept {
            return static_cast<size_t>(mask.hash());
        }
    };
}
