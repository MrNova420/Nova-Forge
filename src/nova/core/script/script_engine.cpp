/**
 * @file script_engine.cpp
 * @brief NovaCore Script System™ - Main Script Engine Implementation
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#include <nova/core/script/script_engine.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>

namespace nova::script {

namespace fs = std::filesystem;

// ============================================================================
// Singleton
// ============================================================================

ScriptEngine& ScriptEngine::get() {
    static ScriptEngine instance;
    return instance;
}

// ============================================================================
// Initialization
// ============================================================================

bool ScriptEngine::initialize() {
    if (m_initialized) {
        return true;
    }
    
    // Register built-in types
    registerBuiltinMathFunctions();
    registerBuiltinStringFunctions();
    registerBuiltinArrayFunctions();
    registerBuiltinConsoleFunctions();
    
    m_initialized = true;
    return true;
}

void ScriptEngine::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    // Destroy all objects
    m_objects.clear();
    
    // Unload all modules
    unloadAllModules();
    
    // Clear functions
    m_functions.clear();
    m_functionSignatures.clear();
    
    // Clear classes
    m_classes.clear();
    
    // Clear globals
    m_globals.clear();
    
    m_debugger.reset();
    m_initialized = false;
}

void ScriptEngine::update(f32 deltaTime) {
    if (!m_initialized) {
        return;
    }
    
    // Check for hot reload
    if (m_hotReloadEnabled) {
        checkFileChanges();
        processReloadQueue();
    }
}

// ============================================================================
// Module Management
// ============================================================================

bool ScriptEngine::loadModule(const std::string& path) {
    // Check if already loaded
    if (isModuleLoaded(path)) {
        return true;
    }
    
    // Read file
    std::ifstream file(path);
    if (!file.is_open()) {
        ScriptError error;
        error.level = ScriptErrorLevel::Error;
        error.message = "Failed to open file: " + path;
        error.location.file = path;
        reportError(error);
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    
    // Get module name from path
    fs::path filePath(path);
    std::string moduleName = filePath.stem().string();
    
    return loadModuleFromSource(moduleName, source);
}

bool ScriptEngine::loadModuleFromSource(const std::string& name, const std::string& source) {
    ScriptModule module;
    module.name = name;
    module.path = name + ".nova";
    
    // Parse and compile (simplified - would use real parser)
    // For now, just mark as loaded
    module.isLoaded = true;
    
    // Calculate hash for hot-reload detection
    std::hash<std::string> hasher;
    module.hash = hasher(source);
    
    m_modules[name] = module;
    return true;
}

void ScriptEngine::unloadModule(const std::string& name) {
    auto it = m_modules.find(name);
    if (it != m_modules.end()) {
        // Destroy objects from this module
        for (auto objIt = m_objects.begin(); objIt != m_objects.end();) {
            if (objIt->second->classPtr && 
                objIt->second->classPtr->module == name) {
                objIt = m_objects.erase(objIt);
            } else {
                ++objIt;
            }
        }
        
        m_modules.erase(it);
    }
}

void ScriptEngine::unloadAllModules() {
    m_objects.clear();
    m_modules.clear();
}

const ScriptModule* ScriptEngine::getModule(const std::string& name) const {
    auto it = m_modules.find(name);
    return it != m_modules.end() ? &it->second : nullptr;
}

std::vector<const ScriptModule*> ScriptEngine::getModules() const {
    std::vector<const ScriptModule*> result;
    for (const auto& [name, module] : m_modules) {
        result.push_back(&module);
    }
    return result;
}

bool ScriptEngine::isModuleLoaded(const std::string& name) const {
    auto it = m_modules.find(name);
    return it != m_modules.end() && it->second.isLoaded;
}

// ============================================================================
// Class Management
// ============================================================================

void ScriptEngine::registerClass(const ScriptClass& cls) {
    m_classes[cls.name] = cls;
}

const ScriptClass* ScriptEngine::getClass(const std::string& name) const {
    auto it = m_classes.find(name);
    return it != m_classes.end() ? &it->second : nullptr;
}

std::vector<const ScriptClass*> ScriptEngine::getClasses() const {
    std::vector<const ScriptClass*> result;
    for (const auto& [name, cls] : m_classes) {
        result.push_back(&cls);
    }
    return result;
}

bool ScriptEngine::hasClass(const std::string& name) const {
    return m_classes.find(name) != m_classes.end();
}

bool ScriptEngine::isSubclassOf(const std::string& derived, const std::string& base) const {
    if (derived == base) return true;
    
    const ScriptClass* cls = getClass(derived);
    while (cls && !cls->baseClass.empty()) {
        if (cls->baseClass == base) return true;
        cls = getClass(cls->baseClass);
    }
    return false;
}

// ============================================================================
// Object Management
// ============================================================================

std::shared_ptr<ScriptObject> ScriptEngine::createObject(const std::string& className,
                                                         const std::vector<ScriptValue>& args) {
    const ScriptClass* cls = getClass(className);
    if (!cls) {
        ScriptError error;
        error.level = ScriptErrorLevel::Error;
        error.message = "Unknown class: " + className;
        reportError(error);
        return nullptr;
    }
    
    if (cls->isAbstract) {
        ScriptError error;
        error.level = ScriptErrorLevel::Error;
        error.message = "Cannot instantiate abstract class: " + className;
        reportError(error);
        return nullptr;
    }
    
    auto obj = std::make_shared<ScriptObject>();
    obj->classPtr = cls;
    obj->instanceId = m_nextInstanceId++;
    
    // Initialize properties with defaults
    for (const auto& prop : cls->properties) {
        obj->properties[prop.name] = prop.defaultValue;
    }
    
    // Call constructor if exists
    if (cls->constructor) {
        cls->constructor(args);
    }
    
    m_objects[obj->instanceId] = obj;
    m_stats.objectsCreated++;
    
    return obj;
}

void ScriptEngine::destroyObject(std::shared_ptr<ScriptObject> obj) {
    if (!obj) return;
    
    // Call destructor if exists
    if (obj->classPtr && obj->classPtr->destructor) {
        obj->classPtr->destructor({});
    }
    
    m_objects.erase(obj->instanceId);
    m_stats.objectsDestroyed++;
}

std::shared_ptr<ScriptObject> ScriptEngine::getObject(u64 instanceId) const {
    auto it = m_objects.find(instanceId);
    return it != m_objects.end() ? it->second : nullptr;
}

std::vector<std::shared_ptr<ScriptObject>> ScriptEngine::getObjectsOfClass(const std::string& className) const {
    std::vector<std::shared_ptr<ScriptObject>> result;
    for (const auto& [id, obj] : m_objects) {
        if (obj->classPtr && obj->classPtr->name == className) {
            result.push_back(obj);
        }
    }
    return result;
}

// ============================================================================
// Function Execution
// ============================================================================

void ScriptEngine::registerFunction(const std::string& name, NativeFunction func) {
    std::string fullName = m_currentNamespace.empty() ? name : m_currentNamespace + "." + name;
    m_functions[fullName] = func;
}

void ScriptEngine::registerFunction(const FunctionSignature& sig, NativeFunction func) {
    std::string fullName = m_currentNamespace.empty() ? sig.name : m_currentNamespace + "." + sig.name;
    m_functions[fullName] = func;
    m_functionSignatures[fullName] = sig;
}

void ScriptEngine::unregisterFunction(const std::string& name) {
    m_functions.erase(name);
    m_functionSignatures.erase(name);
}

ScriptValue ScriptEngine::callFunction(const std::string& name, const std::vector<ScriptValue>& args) {
    auto it = m_functions.find(name);
    if (it == m_functions.end()) {
        ScriptError error;
        error.level = ScriptErrorLevel::Error;
        error.message = "Unknown function: " + name;
        reportError(error);
        return ScriptValue();
    }
    
    auto startTime = std::chrono::steady_clock::now();
    
    try {
        ScriptValue result = it->second(args);
        
        auto endTime = std::chrono::steady_clock::now();
        f64 elapsed = std::chrono::duration<f64>(endTime - startTime).count();
        
        m_stats.functionsExecuted++;
        m_stats.totalExecutionTime += elapsed;
        
        return result;
    } catch (const std::exception& e) {
        ScriptError error;
        error.level = ScriptErrorLevel::Error;
        error.message = std::string("Exception in ") + name + ": " + e.what();
        reportError(error);
        return ScriptValue();
    }
}

ScriptValue ScriptEngine::callMethod(std::shared_ptr<ScriptObject> obj,
                                     const std::string& method,
                                     const std::vector<ScriptValue>& args) {
    if (!obj || !obj->classPtr) {
        ScriptError error;
        error.level = ScriptErrorLevel::Error;
        error.message = "Cannot call method on null object";
        reportError(error);
        return ScriptValue();
    }
    
    // Look for native method
    auto it = obj->classPtr->nativeMethods.find(method);
    if (it != obj->classPtr->nativeMethods.end()) {
        return it->second(obj->nativePtr, args);
    }
    
    ScriptError error;
    error.level = ScriptErrorLevel::Error;
    error.message = "Unknown method: " + obj->classPtr->name + "." + method;
    reportError(error);
    return ScriptValue();
}

bool ScriptEngine::hasFunction(const std::string& name) const {
    return m_functions.find(name) != m_functions.end();
}

// ============================================================================
// Global Variables
// ============================================================================

void ScriptEngine::setGlobal(const std::string& name, const ScriptValue& value) {
    m_globals[name] = value;
}

ScriptValue ScriptEngine::getGlobal(const std::string& name) const {
    auto it = m_globals.find(name);
    return it != m_globals.end() ? it->second : ScriptValue();
}

bool ScriptEngine::hasGlobal(const std::string& name) const {
    return m_globals.find(name) != m_globals.end();
}

void ScriptEngine::removeGlobal(const std::string& name) {
    m_globals.erase(name);
}

// ============================================================================
// Visual Scripting
// ============================================================================

bool ScriptEngine::loadGraph(const std::string& path) {
    // Load graph file
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        m_lastError.code = ScriptErrorCode::FileNotFound;
        m_lastError.message = "Failed to open graph file: " + path;
        return false;
    }
    
    // Read file contents
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    if (content.empty()) {
        m_lastError.code = ScriptErrorCode::ParseError;
        m_lastError.message = "Empty graph file: " + path;
        return false;
    }
    
    // Simple JSON-like parsing for graph format
    // Format: { "nodes": [...], "connections": [...], "entry": id }
    // In production, would use a proper JSON library
    
    // For now, validate that file contains expected structure markers
    if (content.find("nodes") == std::string::npos ||
        content.find("connections") == std::string::npos) {
        m_lastError.code = ScriptErrorCode::ParseError;
        m_lastError.message = "Invalid graph format: " + path;
        return false;
    }
    
    return true;
}

bool ScriptEngine::saveGraph(const std::string& path, const ScriptGraph& graph) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        m_lastError.code = ScriptErrorCode::RuntimeError;
        m_lastError.message = "Failed to create graph file: " + path;
        return false;
    }
    
    // Serialize graph to JSON-like format
    file << "{\n";
    file << "  \"name\": \"" << graph.name << "\",\n";
    file << "  \"entry\": " << graph.entryNodeId << ",\n";
    
    // Serialize nodes
    file << "  \"nodes\": [\n";
    for (usize i = 0; i < graph.nodes.size(); ++i) {
        const auto& node = graph.nodes[i];
        file << "    { \"id\": " << node.id 
             << ", \"type\": " << static_cast<int>(node.type)
             << ", \"name\": \"" << node.name << "\""
             << ", \"x\": " << node.position.x
             << ", \"y\": " << node.position.y
             << " }";
        if (i < graph.nodes.size() - 1) file << ",";
        file << "\n";
    }
    file << "  ],\n";
    
    // Serialize connections
    file << "  \"connections\": [\n";
    for (usize i = 0; i < graph.connections.size(); ++i) {
        const auto& conn = graph.connections[i];
        file << "    { \"from_node\": " << conn.fromNode
             << ", \"from_pin\": " << conn.fromPin
             << ", \"to_node\": " << conn.toNode
             << ", \"to_pin\": " << conn.toPin
             << " }";
        if (i < graph.connections.size() - 1) file << ",";
        file << "\n";
    }
    file << "  ]\n";
    
    file << "}\n";
    
    return file.good();
}

ScriptValue ScriptEngine::executeGraph(const ScriptGraph& graph, const std::vector<ScriptValue>& args) {
    // Find entry node
    const ScriptNode* entryNode = graph.findNode(graph.entryNodeId);
    if (!entryNode) {
        m_lastError.code = ScriptErrorCode::RuntimeError;
        m_lastError.message = "Graph has no entry node";
        return ScriptValue();
    }
    
    // Create execution context
    std::unordered_map<u64, ScriptValue> nodeOutputs;
    std::vector<u64> executionStack;
    executionStack.push_back(graph.entryNodeId);
    
    // Execute nodes in topological order
    while (!executionStack.empty()) {
        u64 currentNodeId = executionStack.back();
        executionStack.pop_back();
        
        const ScriptNode* currentNode = graph.findNode(currentNodeId);
        if (!currentNode) continue;
        
        // Execute node based on type
        ScriptValue result;
        switch (currentNode->type) {
            case NodeType::Entry:
                // Entry node just passes through to connected nodes
                break;
                
            case NodeType::Return:
                // Return node - get input value and return it
                if (!currentNode->inputs.empty()) {
                    // Find connection to this input
                    for (const auto& conn : graph.connections) {
                        if (conn.toNode == currentNodeId && conn.toPin == 0) {
                            auto it = nodeOutputs.find(conn.fromNode);
                            if (it != nodeOutputs.end()) {
                                return it->second;
                            }
                        }
                    }
                }
                return result;
                
            case NodeType::Constant:
                // Store constant value
                nodeOutputs[currentNodeId] = currentNode->data;
                break;
                
            case NodeType::MathOp:
                // Execute math operation on inputs
                {
                    ScriptValue a, b;
                    // Get input values from connected nodes
                    for (const auto& conn : graph.connections) {
                        if (conn.toNode == currentNodeId) {
                            auto it = nodeOutputs.find(conn.fromNode);
                            if (it != nodeOutputs.end()) {
                                if (conn.toPin == 0) a = it->second;
                                else if (conn.toPin == 1) b = it->second;
                            }
                        }
                    }
                    // Perform operation based on node name
                    if (currentNode->name == "Add") {
                        result = ScriptValue(a.toFloat() + b.toFloat());
                    } else if (currentNode->name == "Subtract") {
                        result = ScriptValue(a.toFloat() - b.toFloat());
                    } else if (currentNode->name == "Multiply") {
                        result = ScriptValue(a.toFloat() * b.toFloat());
                    } else if (currentNode->name == "Divide") {
                        f64 divisor = b.toFloat();
                        result = ScriptValue(divisor != 0.0 ? a.toFloat() / divisor : 0.0);
                    }
                    nodeOutputs[currentNodeId] = result;
                }
                break;
                
            default:
                // Other node types - store empty result
                nodeOutputs[currentNodeId] = ScriptValue();
                break;
        }
        
        // Find and queue connected output nodes (for exec flow)
        for (const auto& conn : graph.connections) {
            if (conn.fromNode == currentNodeId) {
                // Check if this is an exec connection
                if (!currentNode->outputs.empty() && 
                    currentNode->outputs[conn.fromPin].type == PinType::Exec) {
                    executionStack.push_back(conn.toNode);
                }
            }
        }
    }
    
    return ScriptValue();
}

bool ScriptEngine::compileGraph(const ScriptGraph& graph, const std::string& outputPath) {
    // Validate graph
    if (graph.nodes.empty()) {
        m_lastError.code = ScriptErrorCode::CompileError;
        m_lastError.message = "Cannot compile empty graph";
        return false;
    }
    
    // Find entry node
    const ScriptNode* entryNode = graph.findNode(graph.entryNodeId);
    if (!entryNode) {
        m_lastError.code = ScriptErrorCode::CompileError;
        m_lastError.message = "Graph has no entry node";
        return false;
    }
    
    // Open output file
    std::ofstream file(outputPath, std::ios::binary);
    if (!file.is_open()) {
        m_lastError.code = ScriptErrorCode::RuntimeError;
        m_lastError.message = "Failed to create output file: " + outputPath;
        return false;
    }
    
    // Write bytecode header
    const char magic[] = "NVGR"; // Nova Graph bytecode
    file.write(magic, 4);
    
    u32 version = 1;
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    
    u32 nodeCount = static_cast<u32>(graph.nodes.size());
    file.write(reinterpret_cast<const char*>(&nodeCount), sizeof(nodeCount));
    
    u32 connCount = static_cast<u32>(graph.connections.size());
    file.write(reinterpret_cast<const char*>(&connCount), sizeof(connCount));
    
    // Write entry node ID
    file.write(reinterpret_cast<const char*>(&graph.entryNodeId), sizeof(graph.entryNodeId));
    
    // Write nodes
    for (const auto& node : graph.nodes) {
        file.write(reinterpret_cast<const char*>(&node.id), sizeof(node.id));
        u8 nodeType = static_cast<u8>(node.type);
        file.write(reinterpret_cast<const char*>(&nodeType), sizeof(nodeType));
        
        // Write name length and name
        u16 nameLen = static_cast<u16>(node.name.size());
        file.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
        file.write(node.name.c_str(), nameLen);
    }
    
    // Write connections
    for (const auto& conn : graph.connections) {
        file.write(reinterpret_cast<const char*>(&conn.fromNode), sizeof(conn.fromNode));
        file.write(reinterpret_cast<const char*>(&conn.fromPin), sizeof(conn.fromPin));
        file.write(reinterpret_cast<const char*>(&conn.toNode), sizeof(conn.toNode));
        file.write(reinterpret_cast<const char*>(&conn.toPin), sizeof(conn.toPin));
    }
    
    return file.good();
}

std::vector<NodeType> ScriptEngine::getAvailableNodeTypes() const {
    return {
        NodeType::Entry, NodeType::Return, NodeType::Branch, NodeType::Switch,
        NodeType::ForLoop, NodeType::WhileLoop, NodeType::ForEach,
        NodeType::FunctionCall, NodeType::Variable, NodeType::Constant,
        NodeType::GetProperty, NodeType::SetProperty,
        NodeType::MathOp, NodeType::Compare, NodeType::Logic,
        NodeType::Cast, NodeType::Construct
    };
}

ScriptNode ScriptEngine::createNodeTemplate(NodeType type) const {
    ScriptNode node;
    node.type = type;
    
    switch (type) {
        case NodeType::Entry:
            node.name = "Entry";
            node.outputs.push_back({"", PinDirection::Output, PinType::Exec, ScriptType::Void, {}});
            break;
            
        case NodeType::Return:
            node.name = "Return";
            node.inputs.push_back({"", PinDirection::Input, PinType::Exec, ScriptType::Void, {}});
            node.inputs.push_back({"Value", PinDirection::Input, PinType::Data, ScriptType::Any, {}});
            break;
            
        case NodeType::Branch:
            node.name = "Branch";
            node.inputs.push_back({"", PinDirection::Input, PinType::Exec, ScriptType::Void, {}});
            node.inputs.push_back({"Condition", PinDirection::Input, PinType::Data, ScriptType::Bool, {}});
            node.outputs.push_back({"True", PinDirection::Output, PinType::Exec, ScriptType::Void, {}});
            node.outputs.push_back({"False", PinDirection::Output, PinType::Exec, ScriptType::Void, {}});
            break;
            
        case NodeType::FunctionCall:
            node.name = "Function";
            node.inputs.push_back({"", PinDirection::Input, PinType::Exec, ScriptType::Void, {}});
            node.outputs.push_back({"", PinDirection::Output, PinType::Exec, ScriptType::Void, {}});
            node.outputs.push_back({"Return", PinDirection::Output, PinType::Data, ScriptType::Any, {}});
            break;
            
        case NodeType::Variable:
            node.name = "Get Variable";
            node.outputs.push_back({"Value", PinDirection::Output, PinType::Data, ScriptType::Any, {}});
            break;
            
        case NodeType::MathOp:
            node.name = "Math";
            node.inputs.push_back({"A", PinDirection::Input, PinType::Data, ScriptType::Float, {}});
            node.inputs.push_back({"B", PinDirection::Input, PinType::Data, ScriptType::Float, {}});
            node.outputs.push_back({"Result", PinDirection::Output, PinType::Data, ScriptType::Float, {}});
            break;
            
        default:
            node.name = "Node";
            break;
    }
    
    return node;
}

// ============================================================================
// Hot Reload
// ============================================================================

void ScriptEngine::setHotReloadEnabled(bool enabled) {
    m_hotReloadEnabled = enabled;
}

void ScriptEngine::addWatchDirectory(const std::string& path) {
    if (std::find(m_watchDirectories.begin(), m_watchDirectories.end(), path) == m_watchDirectories.end()) {
        m_watchDirectories.push_back(path);
    }
}

void ScriptEngine::removeWatchDirectory(const std::string& path) {
    m_watchDirectories.erase(
        std::remove(m_watchDirectories.begin(), m_watchDirectories.end(), path),
        m_watchDirectories.end()
    );
}

void ScriptEngine::reloadAll() {
    for (auto& [name, module] : m_modules) {
        m_modulesToReload.insert(name);
    }
    processReloadQueue();
}

void ScriptEngine::reloadModule(const std::string& name) {
    m_modulesToReload.insert(name);
    processReloadQueue();
}

void ScriptEngine::setReloadCallback(ReloadCallback callback) {
    m_reloadCallback = std::move(callback);
}

void ScriptEngine::checkFileChanges() {
    for (const auto& dir : m_watchDirectories) {
        if (!fs::exists(dir)) continue;
        
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (!entry.is_regular_file()) continue;
            
            std::string ext = entry.path().extension().string();
            if (ext != ".nova" && ext != ".ns") continue;
            
            std::string moduleName = entry.path().stem().string();
            auto it = m_modules.find(moduleName);
            if (it == m_modules.end()) continue;
            
            // Check modification time
            auto modTime = fs::last_write_time(entry).time_since_epoch().count();
            if (static_cast<u64>(modTime) > it->second.lastModified) {
                m_modulesToReload.insert(moduleName);
                if (m_reloadCallback) {
                    m_reloadCallback(entry.path().string(), ReloadEvent::FileChanged);
                }
            }
        }
    }
}

void ScriptEngine::processReloadQueue() {
    for (const auto& moduleName : m_modulesToReload) {
        auto it = m_modules.find(moduleName);
        if (it != m_modules.end()) {
            std::string path = it->second.path;
            unloadModule(moduleName);
            loadModule(path);
        }
    }
    m_modulesToReload.clear();
}

// ============================================================================
// Debugging
// ============================================================================

void ScriptEngine::setDebuggerEnabled(bool enabled) {
    m_debuggerEnabled = enabled;
    if (enabled && !m_debugger) {
        m_debugger = std::make_unique<ScriptDebugger>();
    }
}

void ScriptEngine::setBreakpoint(const std::string& file, u32 line) {
    Breakpoint bp;
    bp.file = file;
    bp.line = line;
    bp.enabled = true;
    bp.id = m_nextBreakpointId++;
    m_breakpoints.push_back(bp);
}

void ScriptEngine::removeBreakpoint(const std::string& file, u32 line) {
    m_breakpoints.erase(
        std::remove_if(m_breakpoints.begin(), m_breakpoints.end(),
            [&](const Breakpoint& bp) {
                return bp.file == file && bp.line == line;
            }),
        m_breakpoints.end()
    );
}

void ScriptEngine::clearBreakpoints() {
    m_breakpoints.clear();
}

void ScriptEngine::stepOver() {
    if (m_debugger && m_isPaused) {
        m_stepMode = StepMode::Over;
        m_isPaused = false;
    }
}

void ScriptEngine::stepInto() {
    if (m_debugger && m_isPaused) {
        m_stepMode = StepMode::Into;
        m_isPaused = false;
    }
}

void ScriptEngine::stepOut() {
    if (m_debugger && m_isPaused) {
        m_stepMode = StepMode::Out;
        m_targetStackDepth = m_callStack.size() > 0 ? m_callStack.size() - 1 : 0;
        m_isPaused = false;
    }
}

void ScriptEngine::continueExecution() {
    if (m_isPaused) {
        m_stepMode = StepMode::Continue;
        m_isPaused = false;
    }
}

void ScriptEngine::pauseExecution() {
    if (!m_isPaused) {
        m_isPaused = true;
        m_stepMode = StepMode::None;
    }
}

bool ScriptEngine::isPaused() const {
    return m_isPaused;
}

std::vector<ScriptLocation> ScriptEngine::getCallStack() const {
    return m_callStack;
}

std::unordered_map<std::string, ScriptValue> ScriptEngine::getLocals(u32 stackFrame) const {
    if (stackFrame >= m_localScopes.size()) {
        return {};
    }
    return m_localScopes[stackFrame];
}

ScriptValue ScriptEngine::evaluate(const std::string& expression) {
    // Simple expression evaluation
    // Supports: numbers, strings, variables, basic math
    
    if (expression.empty()) {
        return ScriptValue();
    }
    
    // Check if it's a number
    bool isNumber = true;
    bool hasDecimal = false;
    for (usize i = 0; i < expression.size(); ++i) {
        char c = expression[i];
        if (c == '.') {
            if (hasDecimal) { isNumber = false; break; }
            hasDecimal = true;
        } else if (c == '-' && i == 0) {
            continue; // Leading minus is OK
        } else if (!std::isdigit(c)) {
            isNumber = false;
            break;
        }
    }
    
    if (isNumber) {
        if (hasDecimal) {
            return ScriptValue(std::stod(expression));
        } else {
            return ScriptValue(static_cast<i64>(std::stoll(expression)));
        }
    }
    
    // Check if it's a string literal
    if (expression.size() >= 2 && expression.front() == '"' && expression.back() == '"') {
        return ScriptValue(expression.substr(1, expression.size() - 2));
    }
    
    // Check if it's a boolean
    if (expression == "true") return ScriptValue(true);
    if (expression == "false") return ScriptValue(false);
    if (expression == "null" || expression == "nil") return ScriptValue();
    
    // Check if it's a variable (look in local scope first, then globals)
    if (!m_localScopes.empty()) {
        auto& locals = m_localScopes.back();
        auto it = locals.find(expression);
        if (it != locals.end()) {
            return it->second;
        }
    }
    
    auto globalIt = m_globals.find(expression);
    if (globalIt != m_globals.end()) {
        return globalIt->second;
    }
    
    // Unknown expression
    m_lastError.code = ScriptErrorCode::RuntimeError;
    m_lastError.message = "Unknown identifier: " + expression;
    return ScriptValue();
}

// ============================================================================
// Error Handling
// ============================================================================

void ScriptEngine::clearError() {
    m_lastError = ScriptError();
}

void ScriptEngine::setErrorCallback(ScriptErrorCallback callback) {
    m_errorCallback = std::move(callback);
}

void ScriptEngine::setLogCallback(ScriptLogCallback callback) {
    m_logCallback = std::move(callback);
}

void ScriptEngine::reportError(const ScriptError& error) {
    m_lastError = error;
    if (m_errorCallback) {
        m_errorCallback(error);
    }
}

// ============================================================================
// Performance
// ============================================================================

void ScriptEngine::resetStats() {
    m_stats = ExecutionStats();
}

void ScriptEngine::setExecutionTimeLimit(f32 seconds) {
    m_executionTimeLimit = seconds;
}

void ScriptEngine::setMaxCallStackDepth(u32 depth) {
    m_maxCallStackDepth = depth;
}

// ============================================================================
// API Registration Helpers
// ============================================================================

void ScriptEngine::beginNamespace(const std::string& name) {
    if (m_currentNamespace.empty()) {
        m_currentNamespace = name;
    } else {
        m_currentNamespace += "." + name;
    }
}

void ScriptEngine::endNamespace() {
    auto pos = m_currentNamespace.rfind('.');
    if (pos != std::string::npos) {
        m_currentNamespace = m_currentNamespace.substr(0, pos);
    } else {
        m_currentNamespace.clear();
    }
}

void ScriptEngine::beginClass(const std::string& name, const std::string& baseClass) {
    m_currentClass = ScriptClass();
    m_currentClass.name = name;
    m_currentClass.baseClass = baseClass;
    m_inClassDef = true;
}

void ScriptEngine::registerConstructor(NativeFunction ctor) {
    if (m_inClassDef) {
        m_currentClass.constructor = ctor;
    }
}

void ScriptEngine::registerProperty(const std::string& name, ScriptType type,
                                   NativeMethod getter, NativeMethod setter) {
    if (m_inClassDef) {
        ScriptProperty prop;
        prop.name = name;
        prop.type = type;
        prop.getter = getter;
        prop.setter = setter;
        prop.isReadonly = !setter;
        m_currentClass.properties.push_back(prop);
    }
}

void ScriptEngine::registerMethod(const std::string& name, NativeMethod method) {
    if (m_inClassDef) {
        m_currentClass.nativeMethods[name] = method;
    }
}

void ScriptEngine::registerStaticMethod(const std::string& name, NativeFunction func) {
    if (m_inClassDef) {
        std::string fullName = m_currentClass.name + "." + name;
        m_functions[fullName] = func;
    }
}

void ScriptEngine::endClass() {
    if (m_inClassDef) {
        registerClass(m_currentClass);
        m_currentClass = ScriptClass();
        m_inClassDef = false;
    }
}

void ScriptEngine::registerEnum(const std::string& name, const std::unordered_map<std::string, i64>& values) {
    for (const auto& [enumName, value] : values) {
        std::string fullName = name + "." + enumName;
        setGlobal(fullName, ScriptValue(value));
    }
}

// ============================================================================
// Built-in Functions
// ============================================================================

void ScriptEngine::registerBuiltinMathFunctions() {
    beginNamespace("Math");
    
    registerFunction("sin", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.empty()) return ScriptValue(0.0);
        return ScriptValue(std::sin(args[0].asFloat()));
    });
    
    registerFunction("cos", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.empty()) return ScriptValue(0.0);
        return ScriptValue(std::cos(args[0].asFloat()));
    });
    
    registerFunction("tan", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.empty()) return ScriptValue(0.0);
        return ScriptValue(std::tan(args[0].asFloat()));
    });
    
    registerFunction("sqrt", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.empty()) return ScriptValue(0.0);
        return ScriptValue(std::sqrt(args[0].asFloat()));
    });
    
    registerFunction("abs", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.empty()) return ScriptValue(0.0);
        return ScriptValue(std::abs(args[0].asFloat()));
    });
    
    registerFunction("floor", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.empty()) return ScriptValue(0.0);
        return ScriptValue(std::floor(args[0].asFloat()));
    });
    
    registerFunction("ceil", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.empty()) return ScriptValue(0.0);
        return ScriptValue(std::ceil(args[0].asFloat()));
    });
    
    registerFunction("round", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.empty()) return ScriptValue(0.0);
        return ScriptValue(std::round(args[0].asFloat()));
    });
    
    registerFunction("min", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.size() < 2) return args.empty() ? ScriptValue(0.0) : args[0];
        return ScriptValue(std::min(args[0].asFloat(), args[1].asFloat()));
    });
    
    registerFunction("max", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.size() < 2) return args.empty() ? ScriptValue(0.0) : args[0];
        return ScriptValue(std::max(args[0].asFloat(), args[1].asFloat()));
    });
    
    registerFunction("clamp", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.size() < 3) return args.empty() ? ScriptValue(0.0) : args[0];
        return ScriptValue(std::clamp(args[0].asFloat(), args[1].asFloat(), args[2].asFloat()));
    });
    
    registerFunction("lerp", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.size() < 3) return args.empty() ? ScriptValue(0.0) : args[0];
        f64 a = args[0].asFloat();
        f64 b = args[1].asFloat();
        f64 t = args[2].asFloat();
        return ScriptValue(a + (b - a) * t);
    });
    
    registerFunction("pow", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.size() < 2) return ScriptValue(0.0);
        return ScriptValue(std::pow(args[0].asFloat(), args[1].asFloat()));
    });
    
    registerFunction("log", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.empty()) return ScriptValue(0.0);
        return ScriptValue(std::log(args[0].asFloat()));
    });
    
    registerFunction("exp", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.empty()) return ScriptValue(0.0);
        return ScriptValue(std::exp(args[0].asFloat()));
    });
    
    // Constants
    setGlobal("Math.PI", ScriptValue(3.14159265358979323846));
    setGlobal("Math.E", ScriptValue(2.71828182845904523536));
    setGlobal("Math.TAU", ScriptValue(6.28318530717958647692));
    
    endNamespace();
}

void ScriptEngine::registerBuiltinStringFunctions() {
    beginNamespace("String");
    
    registerFunction("length", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.empty() || !args[0].isString()) return ScriptValue(static_cast<i64>(0));
        return ScriptValue(static_cast<i64>(args[0].asString().length()));
    });
    
    registerFunction("upper", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.empty() || !args[0].isString()) return ScriptValue("");
        std::string s = args[0].asString();
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        return ScriptValue(s);
    });
    
    registerFunction("lower", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.empty() || !args[0].isString()) return ScriptValue("");
        std::string s = args[0].asString();
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return ScriptValue(s);
    });
    
    registerFunction("contains", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.size() < 2) return ScriptValue(false);
        return ScriptValue(args[0].asString().find(args[1].asString()) != std::string::npos);
    });
    
    registerFunction("startsWith", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.size() < 2) return ScriptValue(false);
        const auto& s = args[0].asString();
        const auto& prefix = args[1].asString();
        return ScriptValue(s.rfind(prefix, 0) == 0);
    });
    
    registerFunction("endsWith", [](const std::vector<ScriptValue>& args) -> ScriptValue {
        if (args.size() < 2) return ScriptValue(false);
        const auto& s = args[0].asString();
        const auto& suffix = args[1].asString();
        if (suffix.size() > s.size()) return ScriptValue(false);
        return ScriptValue(s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0);
    });
    
    endNamespace();
}

void ScriptEngine::registerBuiltinArrayFunctions() {
    beginNamespace("Array");
    
    // Array functions would go here
    
    endNamespace();
}

void ScriptEngine::registerBuiltinConsoleFunctions() {
    registerFunction("print", [this](const std::vector<ScriptValue>& args) -> ScriptValue {
        std::string output;
        for (const auto& arg : args) {
            if (arg.isString()) {
                output += arg.asString();
            } else if (arg.isInt()) {
                output += std::to_string(arg.asInt());
            } else if (arg.isFloat()) {
                output += std::to_string(arg.asFloat());
            } else if (arg.isBool()) {
                output += arg.asBool() ? "true" : "false";
            }
        }
        if (m_logCallback) {
            m_logCallback(output);
        }
        return ScriptValue();
    });
    
    registerFunction("log", [this](const std::vector<ScriptValue>& args) -> ScriptValue {
        return callFunction("print", args);
    });
    
    registerFunction("warn", [this](const std::vector<ScriptValue>& args) -> ScriptValue {
        ScriptError warning;
        warning.level = ScriptErrorLevel::Warning;
        if (!args.empty() && args[0].isString()) {
            warning.message = args[0].asString();
        }
        reportError(warning);
        return ScriptValue();
    });
    
    registerFunction("error", [this](const std::vector<ScriptValue>& args) -> ScriptValue {
        ScriptError error;
        error.level = ScriptErrorLevel::Error;
        if (!args.empty() && args[0].isString()) {
            error.message = args[0].asString();
        }
        reportError(error);
        return ScriptValue();
    });
}

// ============================================================================
// ScriptValue Operators
// ============================================================================

bool ScriptValue::operator==(const ScriptValue& other) const {
    if (type != other.type) return false;
    
    switch (type) {
        case ScriptType::Void: return true;
        case ScriptType::Bool: return asBool() == other.asBool();
        case ScriptType::Int: return asInt() == other.asInt();
        case ScriptType::Float: return asFloat() == other.asFloat();
        case ScriptType::String: return asString() == other.asString();
        case ScriptType::Vec2: return asVec2() == other.asVec2();
        case ScriptType::Vec3: return asVec3() == other.asVec3();
        case ScriptType::Vec4: return asVec4() == other.asVec4();
        case ScriptType::Quat: return asQuat() == other.asQuat();
        case ScriptType::Entity: return asEntity() == other.asEntity();
        default: return false;
    }
}

ScriptValue ScriptValue::toBool() const {
    return ScriptValue(asBool());
}

ScriptValue ScriptValue::toInt() const {
    return ScriptValue(asInt());
}

ScriptValue ScriptValue::toFloat() const {
    return ScriptValue(asFloat());
}

ScriptValue ScriptValue::toString() const {
    switch (type) {
        case ScriptType::Void: return ScriptValue("void");
        case ScriptType::Bool: return ScriptValue(asBool() ? "true" : "false");
        case ScriptType::Int: return ScriptValue(std::to_string(asInt()));
        case ScriptType::Float: return ScriptValue(std::to_string(asFloat()));
        case ScriptType::String: return *this;
        default: return ScriptValue(typeName());
    }
}

// ============================================================================
// ScriptObject Methods
// ============================================================================

ScriptValue ScriptObject::getProperty(const std::string& name) const {
    auto it = properties.find(name);
    if (it != properties.end()) {
        return it->second;
    }
    
    // Check for native getter
    if (classPtr) {
        for (const auto& prop : classPtr->properties) {
            if (prop.name == name && prop.getter) {
                return prop.getter(nativePtr, {});
            }
        }
    }
    
    return ScriptValue();
}

void ScriptObject::setProperty(const std::string& name, const ScriptValue& value) {
    // Check for native setter
    if (classPtr) {
        for (const auto& prop : classPtr->properties) {
            if (prop.name == name) {
                if (prop.isReadonly) return;
                if (prop.setter) {
                    prop.setter(nativePtr, {value});
                    return;
                }
            }
        }
    }
    
    properties[name] = value;
}

bool ScriptObject::hasProperty(const std::string& name) const {
    if (properties.find(name) != properties.end()) {
        return true;
    }
    
    if (classPtr) {
        for (const auto& prop : classPtr->properties) {
            if (prop.name == name) return true;
        }
    }
    
    return false;
}

ScriptValue ScriptObject::callMethod(const std::string& name, const std::vector<ScriptValue>& args) {
    if (!classPtr) {
        return ScriptValue();
    }
    
    auto it = classPtr->nativeMethods.find(name);
    if (it != classPtr->nativeMethods.end()) {
        return it->second(nativePtr, args);
    }
    
    return ScriptValue();
}

// ============================================================================
// ScriptError
// ============================================================================

std::string ScriptError::toString() const {
    std::string result;
    
    switch (level) {
        case ScriptErrorLevel::Info: result = "[INFO] "; break;
        case ScriptErrorLevel::Warning: result = "[WARN] "; break;
        case ScriptErrorLevel::Error: result = "[ERROR] "; break;
        case ScriptErrorLevel::Fatal: result = "[FATAL] "; break;
    }
    
    if (!code.empty()) {
        result += code + ": ";
    }
    
    result += message;
    
    if (!location.file.empty()) {
        result += " at " + location.file;
        if (location.line > 0) {
            result += ":" + std::to_string(location.line);
            if (location.column > 0) {
                result += ":" + std::to_string(location.column);
            }
        }
    }
    
    return result;
}

// ============================================================================
// FunctionSignature
// ============================================================================

std::string FunctionSignature::toString() const {
    std::string result = ScriptValue::typeName(returnType);
    result += " " + name + "(";
    
    for (usize i = 0; i < params.size(); ++i) {
        if (i > 0) result += ", ";
        result += ScriptValue::typeName(params[i].type);
        result += " " + params[i].name;
        if (params[i].hasDefault) {
            result += " = ...";
        }
    }
    
    if (isVariadic) {
        if (!params.empty()) result += ", ";
        result += "...";
    }
    
    result += ")";
    
    if (isConst) result += " const";
    
    return result;
}

} // namespace nova::script
