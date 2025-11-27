/**
 * @file script_types.hpp
 * @brief NovaCore Script System™ - Core Type Definitions
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 * 
 * Defines the fundamental types for the scripting system including:
 * - Script value types
 * - Script function signatures
 * - Script class definitions
 * - Hot-reload support
 * - Visual scripting node types
 */

#pragma once

#include <nova/core/types/types.hpp>
#include <nova/core/math/math.hpp>
#include <nova/core/ecs/entity.hpp>

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <variant>
#include <any>
#include <unordered_map>

namespace nova::script {

using namespace nova::core;
using namespace nova::core::math;

// ============================================================================
// Configuration Constants
// ============================================================================

namespace ScriptConfig {
    constexpr u32 MAX_FUNCTION_PARAMS = 16;
    constexpr u32 MAX_CALL_STACK_DEPTH = 256;
    constexpr u32 MAX_LOCAL_VARIABLES = 128;
    constexpr usize DEFAULT_STACK_SIZE = 1024 * 64;  // 64KB
    constexpr f32 EXECUTION_TIME_LIMIT = 5.0f;       // Seconds
}

// ============================================================================
// Script Value Types
// ============================================================================

/**
 * @brief Primitive script value types
 */
enum class ScriptType : u8 {
    Void,
    Bool,
    Int,
    Float,
    String,
    Vec2,
    Vec3,
    Vec4,
    Quat,
    Color,
    Entity,
    Object,       // Reference to script object
    Array,        // Dynamic array
    Map,          // Key-value map
    Function,     // Function reference
    Any           // Any type (variant)
};

/**
 * @brief Script value variant
 */
struct ScriptValue {
    ScriptType type = ScriptType::Void;
    
    std::variant<
        std::monostate,     // Void
        bool,               // Bool
        i64,                // Int
        f64,                // Float
        std::string,        // String
        Vec2,               // Vec2
        Vec3,               // Vec3
        Vec4,               // Vec4/Color
        Quat,               // Quat
        ecs::Entity,        // Entity
        std::shared_ptr<void>,  // Object/Array/Map/Function
        std::any            // Any
    > data;
    
    // Constructors
    ScriptValue() : type(ScriptType::Void), data(std::monostate{}) {}
    ScriptValue(bool v) : type(ScriptType::Bool), data(v) {}
    ScriptValue(i64 v) : type(ScriptType::Int), data(v) {}
    ScriptValue(i32 v) : type(ScriptType::Int), data(static_cast<i64>(v)) {}
    ScriptValue(f64 v) : type(ScriptType::Float), data(v) {}
    ScriptValue(f32 v) : type(ScriptType::Float), data(static_cast<f64>(v)) {}
    ScriptValue(const std::string& v) : type(ScriptType::String), data(v) {}
    ScriptValue(const char* v) : type(ScriptType::String), data(std::string(v)) {}
    ScriptValue(Vec2 v) : type(ScriptType::Vec2), data(v) {}
    ScriptValue(Vec3 v) : type(ScriptType::Vec3), data(v) {}
    ScriptValue(Vec4 v) : type(ScriptType::Vec4), data(v) {}
    ScriptValue(Quat v) : type(ScriptType::Quat), data(v) {}
    ScriptValue(ecs::Entity v) : type(ScriptType::Entity), data(v) {}
    
    // Type checking
    bool isVoid() const { return type == ScriptType::Void; }
    bool isBool() const { return type == ScriptType::Bool; }
    bool isInt() const { return type == ScriptType::Int; }
    bool isFloat() const { return type == ScriptType::Float; }
    bool isNumber() const { return isInt() || isFloat(); }
    bool isString() const { return type == ScriptType::String; }
    bool isVec2() const { return type == ScriptType::Vec2; }
    bool isVec3() const { return type == ScriptType::Vec3; }
    bool isVec4() const { return type == ScriptType::Vec4; }
    bool isQuat() const { return type == ScriptType::Quat; }
    bool isEntity() const { return type == ScriptType::Entity; }
    bool isObject() const { return type == ScriptType::Object; }
    bool isArray() const { return type == ScriptType::Array; }
    bool isMap() const { return type == ScriptType::Map; }
    bool isFunction() const { return type == ScriptType::Function; }
    
    // Value getters
    bool asBool() const;
    i64 asInt() const;
    f64 asFloat() const;
    const std::string& asString() const;
    Vec2 asVec2() const;
    Vec3 asVec3() const;
    Vec4 asVec4() const;
    Quat asQuat() const;
    ecs::Entity asEntity() const;
    
    // Conversion
    ScriptValue toBool() const;
    ScriptValue toInt() const;
    ScriptValue toFloat() const;
    ScriptValue toString() const;
    
    // Operators
    bool operator==(const ScriptValue& other) const;
    bool operator!=(const ScriptValue& other) const { return !(*this == other); }
    
    // Type name
    static const char* typeName(ScriptType type);
    const char* typeName() const { return typeName(type); }
};

// ============================================================================
// Script Function Types
// ============================================================================

/**
 * @brief Parameter definition
 */
struct ScriptParam {
    std::string name;
    ScriptType type = ScriptType::Any;
    ScriptValue defaultValue;
    bool hasDefault = false;
    bool isOptional = false;
    bool isRef = false;       // Pass by reference
    bool isConst = false;     // Read-only reference
};

/**
 * @brief Function signature
 */
struct FunctionSignature {
    std::string name;
    ScriptType returnType = ScriptType::Void;
    std::vector<ScriptParam> params;
    bool isVariadic = false;
    bool isStatic = false;
    bool isVirtual = false;
    bool isConst = false;
    
    std::string toString() const;
};

/**
 * @brief Native function callback
 */
using NativeFunction = std::function<ScriptValue(const std::vector<ScriptValue>&)>;

/**
 * @brief Native method callback (with this pointer)
 */
using NativeMethod = std::function<ScriptValue(void*, const std::vector<ScriptValue>&)>;

// ============================================================================
// Script Class Types
// ============================================================================

/**
 * @brief Property access mode
 */
enum class PropertyAccess : u8 {
    Public,
    Protected,
    Private
};

/**
 * @brief Script property definition
 */
struct ScriptProperty {
    std::string name;
    ScriptType type = ScriptType::Any;
    ScriptValue defaultValue;
    PropertyAccess access = PropertyAccess::Public;
    bool isReadonly = false;
    bool isStatic = false;
    bool isSerialized = true;
    std::string category;     // For editor grouping
    std::string tooltip;      // For editor tooltip
    
    // Property getter/setter (if native)
    NativeMethod getter;
    NativeMethod setter;
};

/**
 * @brief Script class definition
 */
struct ScriptClass {
    std::string name;
    std::string baseClass;
    std::string module;
    std::string sourcePath;
    
    std::vector<ScriptProperty> properties;
    std::vector<FunctionSignature> methods;
    
    bool isAbstract = false;
    bool isSealed = false;
    bool isComponent = false; // Can be attached to entities
    bool isSingleton = false;
    
    // Native bindings
    NativeFunction constructor;
    NativeFunction destructor;
    std::unordered_map<std::string, NativeMethod> nativeMethods;
};

// ============================================================================
// Script Instance Types
// ============================================================================

/**
 * @brief Script object instance
 */
struct ScriptObject {
    const ScriptClass* classPtr = nullptr;
    std::unordered_map<std::string, ScriptValue> properties;
    void* nativePtr = nullptr;  // For native object binding
    u64 instanceId = 0;
    
    ScriptValue getProperty(const std::string& name) const;
    void setProperty(const std::string& name, const ScriptValue& value);
    bool hasProperty(const std::string& name) const;
    
    ScriptValue callMethod(const std::string& name, const std::vector<ScriptValue>& args);
};

// ============================================================================
// Script Module Types
// ============================================================================

/**
 * @brief Script module (file)
 */
struct ScriptModule {
    std::string name;
    std::string path;
    std::vector<std::string> imports;
    std::vector<std::string> exports;
    std::vector<ScriptClass> classes;
    std::vector<FunctionSignature> functions;
    std::unordered_map<std::string, ScriptValue> constants;
    
    u64 hash = 0;             // Content hash for hot-reload
    u64 lastModified = 0;     // File modification time
    bool isLoaded = false;
    bool needsReload = false;
};

// ============================================================================
// Script Error Types
// ============================================================================

/**
 * @brief Script error severity
 */
enum class ScriptErrorLevel : u8 {
    Info,
    Warning,
    Error,
    Fatal
};

/**
 * @brief Script error location
 */
struct ScriptLocation {
    std::string file;
    u32 line = 0;
    u32 column = 0;
    std::string function;
};

/**
 * @brief Script error/warning
 */
struct ScriptError {
    ScriptErrorLevel level = ScriptErrorLevel::Error;
    std::string message;
    std::string code;         // Error code (e.g., "E001")
    ScriptLocation location;
    std::vector<ScriptLocation> stackTrace;
    
    std::string toString() const;
};

// ============================================================================
// Visual Scripting Types
// ============================================================================

/**
 * @brief Visual script node type
 */
enum class NodeType : u8 {
    // Control flow
    Entry,
    Return,
    Branch,
    Switch,
    Sequence,
    ForLoop,
    WhileLoop,
    ForEach,
    
    // Events
    Event,
    CustomEvent,
    EventDispatch,
    
    // Functions
    FunctionCall,
    FunctionDef,
    Macro,
    
    // Data
    Variable,
    Constant,
    GetProperty,
    SetProperty,
    ArrayGet,
    ArraySet,
    MapGet,
    MapSet,
    
    // Math
    MathOp,
    Compare,
    Logic,
    
    // Conversion
    Cast,
    Make,
    Break,
    
    // Object
    Construct,
    IsValid,
    GetClass,
    
    // Flow control
    Delay,
    WaitFor,
    Parallel,
    
    // Comments
    Comment,
    Reroute
};

/**
 * @brief Visual script pin direction
 */
enum class PinDirection : u8 {
    Input,
    Output
};

/**
 * @brief Visual script pin type
 */
enum class PinType : u8 {
    Exec,         // Execution flow
    Data,         // Data connection
    Delegate      // Event delegate
};

/**
 * @brief Visual script pin
 */
struct ScriptPin {
    std::string name;
    PinDirection direction = PinDirection::Input;
    PinType pinType = PinType::Data;
    ScriptType dataType = ScriptType::Any;
    ScriptValue defaultValue;
    bool isConnected = false;
    bool isHidden = false;
    
    u32 nodeId = 0;       // Owner node
    u32 pinIndex = 0;     // Index on node
    
    // UI
    Vec4 color = Vec4(1, 1, 1, 1);
};

/**
 * @brief Visual script node
 */
struct ScriptNode {
    u32 id = 0;
    NodeType type = NodeType::FunctionCall;
    std::string name;
    std::string category;
    std::string tooltip;
    
    std::vector<ScriptPin> inputs;
    std::vector<ScriptPin> outputs;
    
    // UI position
    Vec2 position;
    Vec2 size;
    bool isCollapsed = false;
    bool isSelected = false;
    Vec4 color = Vec4(0.3f, 0.3f, 0.3f, 1.0f);
    
    // Execution state
    bool isExecuting = false;
    bool hasBreakpoint = false;
};

/**
 * @brief Visual script connection
 */
struct ScriptConnection {
    u32 fromNode = 0;
    u32 fromPin = 0;
    u32 toNode = 0;
    u32 toPin = 0;
};

/**
 * @brief Visual script graph
 */
struct ScriptGraph {
    std::string name;
    std::vector<ScriptNode> nodes;
    std::vector<ScriptConnection> connections;
    std::vector<ScriptProperty> variables;  // Local graph variables
    
    u32 entryNodeId = 0;
    u32 nextNodeId = 1;
    
    // Graph metadata
    std::string description;
    std::string category;
    Vec2 viewPosition;
    f32 viewZoom = 1.0f;
    
    // Add/remove operations
    u32 addNode(NodeType type, const Vec2& position);
    void removeNode(u32 nodeId);
    bool connect(u32 fromNode, u32 fromPin, u32 toNode, u32 toPin);
    void disconnect(u32 fromNode, u32 fromPin, u32 toNode, u32 toPin);
    
    ScriptNode* findNode(u32 id);
    const ScriptNode* findNode(u32 id) const;
};

// ============================================================================
// Hot Reload Types
// ============================================================================

/**
 * @brief Hot reload event type
 */
enum class ReloadEvent : u8 {
    FileChanged,
    FileCreated,
    FileDeleted,
    FileMoved
};

/**
 * @brief Hot reload callback
 */
using ReloadCallback = std::function<void(const std::string& path, ReloadEvent event)>;

// ============================================================================
// Callback Types
// ============================================================================

using ScriptErrorCallback = std::function<void(const ScriptError&)>;
using ScriptLogCallback = std::function<void(const std::string& message)>;

// ============================================================================
// Inline Implementations
// ============================================================================

inline bool ScriptValue::asBool() const {
    if (type == ScriptType::Bool) return std::get<bool>(data);
    if (type == ScriptType::Int) return std::get<i64>(data) != 0;
    if (type == ScriptType::Float) return std::get<f64>(data) != 0.0;
    return false;
}

inline i64 ScriptValue::asInt() const {
    if (type == ScriptType::Int) return std::get<i64>(data);
    if (type == ScriptType::Float) return static_cast<i64>(std::get<f64>(data));
    if (type == ScriptType::Bool) return std::get<bool>(data) ? 1 : 0;
    return 0;
}

inline f64 ScriptValue::asFloat() const {
    if (type == ScriptType::Float) return std::get<f64>(data);
    if (type == ScriptType::Int) return static_cast<f64>(std::get<i64>(data));
    if (type == ScriptType::Bool) return std::get<bool>(data) ? 1.0 : 0.0;
    return 0.0;
}

inline const std::string& ScriptValue::asString() const {
    static std::string empty;
    if (type == ScriptType::String) return std::get<std::string>(data);
    return empty;
}

inline Vec2 ScriptValue::asVec2() const {
    if (type == ScriptType::Vec2) return std::get<Vec2>(data);
    return Vec2::zero();
}

inline Vec3 ScriptValue::asVec3() const {
    if (type == ScriptType::Vec3) return std::get<Vec3>(data);
    return Vec3::zero();
}

inline Vec4 ScriptValue::asVec4() const {
    if (type == ScriptType::Vec4) return std::get<Vec4>(data);
    return Vec4::zero();
}

inline Quat ScriptValue::asQuat() const {
    if (type == ScriptType::Quat) return std::get<Quat>(data);
    return Quat::identity();
}

inline ecs::Entity ScriptValue::asEntity() const {
    if (type == ScriptType::Entity) return std::get<ecs::Entity>(data);
    return ecs::Entity();
}

inline const char* ScriptValue::typeName(ScriptType type) {
    switch (type) {
        case ScriptType::Void: return "void";
        case ScriptType::Bool: return "bool";
        case ScriptType::Int: return "int";
        case ScriptType::Float: return "float";
        case ScriptType::String: return "string";
        case ScriptType::Vec2: return "Vec2";
        case ScriptType::Vec3: return "Vec3";
        case ScriptType::Vec4: return "Vec4";
        case ScriptType::Quat: return "Quat";
        case ScriptType::Color: return "Color";
        case ScriptType::Entity: return "Entity";
        case ScriptType::Object: return "Object";
        case ScriptType::Array: return "Array";
        case ScriptType::Map: return "Map";
        case ScriptType::Function: return "Function";
        case ScriptType::Any: return "any";
    }
    return "unknown";
}

inline u32 ScriptGraph::addNode(NodeType type, const Vec2& position) {
    ScriptNode node;
    node.id = nextNodeId++;
    node.type = type;
    node.position = position;
    nodes.push_back(node);
    return node.id;
}

inline void ScriptGraph::removeNode(u32 nodeId) {
    // Remove connections to/from this node
    connections.erase(
        std::remove_if(connections.begin(), connections.end(),
            [nodeId](const ScriptConnection& c) {
                return c.fromNode == nodeId || c.toNode == nodeId;
            }),
        connections.end()
    );
    
    // Remove node
    nodes.erase(
        std::remove_if(nodes.begin(), nodes.end(),
            [nodeId](const ScriptNode& n) { return n.id == nodeId; }),
        nodes.end()
    );
}

inline bool ScriptGraph::connect(u32 fromNode, u32 fromPin, u32 toNode, u32 toPin) {
    // Validate nodes exist
    auto* srcNode = findNode(fromNode);
    auto* dstNode = findNode(toNode);
    if (!srcNode || !dstNode) return false;
    
    // Add connection
    ScriptConnection conn;
    conn.fromNode = fromNode;
    conn.fromPin = fromPin;
    conn.toNode = toNode;
    conn.toPin = toPin;
    connections.push_back(conn);
    
    return true;
}

inline void ScriptGraph::disconnect(u32 fromNode, u32 fromPin, u32 toNode, u32 toPin) {
    connections.erase(
        std::remove_if(connections.begin(), connections.end(),
            [&](const ScriptConnection& c) {
                return c.fromNode == fromNode && c.fromPin == fromPin &&
                       c.toNode == toNode && c.toPin == toPin;
            }),
        connections.end()
    );
}

inline ScriptNode* ScriptGraph::findNode(u32 id) {
    for (auto& node : nodes) {
        if (node.id == id) return &node;
    }
    return nullptr;
}

inline const ScriptNode* ScriptGraph::findNode(u32 id) const {
    for (const auto& node : nodes) {
        if (node.id == id) return &node;
    }
    return nullptr;
}

} // namespace nova::script
