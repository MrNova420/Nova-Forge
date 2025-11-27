/**
 * @file script_engine.hpp
 * @brief NovaCore Script System™ - Main Script Engine
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 * 
 * Central scripting engine providing:
 * - Script compilation and execution
 * - Native API bindings
 * - Hot-reload support
 * - Visual scripting runtime
 * - Debugging and profiling
 */

#pragma once

#include "script_types.hpp"

#include <memory>
#include <queue>
#include <set>

namespace nova::script {

// ============================================================================
// Forward Declarations
// ============================================================================

class ScriptCompiler;
class ScriptVM;
class ScriptDebugger;

// ============================================================================
// Script Engine
// ============================================================================

/**
 * @brief Central scripting engine
 * 
 * The ScriptEngine provides a complete scripting solution for NovaCore:
 * 
 * Usage:
 * @code
 *     auto& script = ScriptEngine::get();
 *     
 *     // Load and execute a script module
 *     script.loadModule("scripts/player.nova");
 *     
 *     // Create an instance
 *     auto player = script.createObject("Player");
 *     
 *     // Call methods
 *     player.callMethod("update", {deltaTime});
 *     
 *     // Register native functions
 *     script.registerFunction("print", [](const auto& args) {
 *         std::cout << args[0].asString() << std::endl;
 *         return ScriptValue();
 *     });
 * @endcode
 */
class ScriptEngine {
public:
    /**
     * @brief Get singleton instance
     */
    static ScriptEngine& get();
    
    /**
     * @brief Initialize the script engine
     * @return true if initialization succeeded
     */
    bool initialize();
    
    /**
     * @brief Shutdown the script engine
     */
    void shutdown();
    
    /**
     * @brief Update script engine (call each frame)
     * @param deltaTime Time since last frame
     */
    void update(f32 deltaTime);
    
    /**
     * @brief Check if engine is initialized
     */
    bool isInitialized() const { return m_initialized; }
    
    // ========================================================================
    // Module Management
    // ========================================================================
    
    /**
     * @brief Load a script module from file
     * @param path Script file path
     * @return true if loaded successfully
     */
    bool loadModule(const std::string& path);
    
    /**
     * @brief Load module from source string
     * @param name Module name
     * @param source Source code
     * @return true if compiled successfully
     */
    bool loadModuleFromSource(const std::string& name, const std::string& source);
    
    /**
     * @brief Unload a module
     */
    void unloadModule(const std::string& name);
    
    /**
     * @brief Unload all modules
     */
    void unloadAllModules();
    
    /**
     * @brief Get a loaded module
     */
    const ScriptModule* getModule(const std::string& name) const;
    
    /**
     * @brief Get all loaded modules
     */
    std::vector<const ScriptModule*> getModules() const;
    
    /**
     * @brief Check if module is loaded
     */
    bool isModuleLoaded(const std::string& name) const;
    
    // ========================================================================
    // Class Management
    // ========================================================================
    
    /**
     * @brief Register a native class
     */
    void registerClass(const ScriptClass& cls);
    
    /**
     * @brief Get a class definition
     */
    const ScriptClass* getClass(const std::string& name) const;
    
    /**
     * @brief Get all registered classes
     */
    std::vector<const ScriptClass*> getClasses() const;
    
    /**
     * @brief Check if class exists
     */
    bool hasClass(const std::string& name) const;
    
    /**
     * @brief Check if class is subclass of another
     */
    bool isSubclassOf(const std::string& derived, const std::string& base) const;
    
    // ========================================================================
    // Object Management
    // ========================================================================
    
    /**
     * @brief Create a script object instance
     * @param className Class name
     * @param args Constructor arguments
     * @return Created object (null on failure)
     */
    std::shared_ptr<ScriptObject> createObject(const std::string& className,
                                               const std::vector<ScriptValue>& args = {});
    
    /**
     * @brief Destroy a script object
     */
    void destroyObject(std::shared_ptr<ScriptObject> obj);
    
    /**
     * @brief Get object by instance ID
     */
    std::shared_ptr<ScriptObject> getObject(u64 instanceId) const;
    
    /**
     * @brief Get all objects of a class type
     */
    std::vector<std::shared_ptr<ScriptObject>> getObjectsOfClass(const std::string& className) const;
    
    // ========================================================================
    // Function Execution
    // ========================================================================
    
    /**
     * @brief Register a global function
     */
    void registerFunction(const std::string& name, NativeFunction func);
    
    /**
     * @brief Register a global function with signature
     */
    void registerFunction(const FunctionSignature& sig, NativeFunction func);
    
    /**
     * @brief Unregister a function
     */
    void unregisterFunction(const std::string& name);
    
    /**
     * @brief Call a global function
     * @param name Function name
     * @param args Arguments
     * @return Return value
     */
    ScriptValue callFunction(const std::string& name, const std::vector<ScriptValue>& args = {});
    
    /**
     * @brief Call a method on an object
     */
    ScriptValue callMethod(std::shared_ptr<ScriptObject> obj,
                          const std::string& method,
                          const std::vector<ScriptValue>& args = {});
    
    /**
     * @brief Check if function exists
     */
    bool hasFunction(const std::string& name) const;
    
    // ========================================================================
    // Global Variables
    // ========================================================================
    
    /**
     * @brief Set a global variable
     */
    void setGlobal(const std::string& name, const ScriptValue& value);
    
    /**
     * @brief Get a global variable
     */
    ScriptValue getGlobal(const std::string& name) const;
    
    /**
     * @brief Check if global exists
     */
    bool hasGlobal(const std::string& name) const;
    
    /**
     * @brief Remove a global variable
     */
    void removeGlobal(const std::string& name);
    
    // ========================================================================
    // Visual Scripting
    // ========================================================================
    
    /**
     * @brief Load a visual script graph
     */
    bool loadGraph(const std::string& path);
    
    /**
     * @brief Save a visual script graph
     */
    bool saveGraph(const std::string& path, const ScriptGraph& graph);
    
    /**
     * @brief Execute a visual script graph
     */
    ScriptValue executeGraph(const ScriptGraph& graph, const std::vector<ScriptValue>& args = {});
    
    /**
     * @brief Compile visual script to bytecode
     */
    bool compileGraph(const ScriptGraph& graph, const std::string& outputPath);
    
    /**
     * @brief Get available node types for visual scripting
     */
    std::vector<NodeType> getAvailableNodeTypes() const;
    
    /**
     * @brief Create a node template
     */
    ScriptNode createNodeTemplate(NodeType type) const;
    
    // ========================================================================
    // Hot Reload
    // ========================================================================
    
    /**
     * @brief Enable/disable hot reload
     */
    void setHotReloadEnabled(bool enabled);
    
    /**
     * @brief Check if hot reload is enabled
     */
    bool isHotReloadEnabled() const { return m_hotReloadEnabled; }
    
    /**
     * @brief Add a directory to watch for changes
     */
    void addWatchDirectory(const std::string& path);
    
    /**
     * @brief Remove a watch directory
     */
    void removeWatchDirectory(const std::string& path);
    
    /**
     * @brief Force reload all modules
     */
    void reloadAll();
    
    /**
     * @brief Reload a specific module
     */
    void reloadModule(const std::string& name);
    
    /**
     * @brief Set reload callback
     */
    void setReloadCallback(ReloadCallback callback);
    
    // ========================================================================
    // Debugging
    // ========================================================================
    
    /**
     * @brief Enable/disable debugger
     */
    void setDebuggerEnabled(bool enabled);
    
    /**
     * @brief Check if debugger is enabled
     */
    bool isDebuggerEnabled() const { return m_debuggerEnabled; }
    
    /**
     * @brief Set breakpoint
     */
    void setBreakpoint(const std::string& file, u32 line);
    
    /**
     * @brief Remove breakpoint
     */
    void removeBreakpoint(const std::string& file, u32 line);
    
    /**
     * @brief Clear all breakpoints
     */
    void clearBreakpoints();
    
    /**
     * @brief Step over (continue to next line)
     */
    void stepOver();
    
    /**
     * @brief Step into (enter function call)
     */
    void stepInto();
    
    /**
     * @brief Step out (exit current function)
     */
    void stepOut();
    
    /**
     * @brief Continue execution
     */
    void continueExecution();
    
    /**
     * @brief Pause execution
     */
    void pauseExecution();
    
    /**
     * @brief Check if execution is paused
     */
    bool isPaused() const;
    
    /**
     * @brief Get current call stack
     */
    std::vector<ScriptLocation> getCallStack() const;
    
    /**
     * @brief Get local variables at stack frame
     */
    std::unordered_map<std::string, ScriptValue> getLocals(u32 stackFrame = 0) const;
    
    /**
     * @brief Evaluate expression in current context
     */
    ScriptValue evaluate(const std::string& expression);
    
    // ========================================================================
    // Error Handling
    // ========================================================================
    
    /**
     * @brief Get last error
     */
    const ScriptError& getLastError() const { return m_lastError; }
    
    /**
     * @brief Check if there was an error
     */
    bool hasError() const { return !m_lastError.message.empty(); }
    
    /**
     * @brief Clear error state
     */
    void clearError();
    
    /**
     * @brief Set error callback
     */
    void setErrorCallback(ScriptErrorCallback callback);
    
    /**
     * @brief Set log callback
     */
    void setLogCallback(ScriptLogCallback callback);
    
    // ========================================================================
    // Performance
    // ========================================================================
    
    /**
     * @brief Get execution statistics
     */
    struct ExecutionStats {
        u64 functionsExecuted = 0;
        u64 instructionsExecuted = 0;
        f64 totalExecutionTime = 0.0;
        u32 objectsCreated = 0;
        u32 objectsDestroyed = 0;
        usize memoryUsed = 0;
    };
    
    ExecutionStats getStats() const { return m_stats; }
    
    /**
     * @brief Reset statistics
     */
    void resetStats();
    
    /**
     * @brief Set execution time limit
     */
    void setExecutionTimeLimit(f32 seconds);
    
    /**
     * @brief Set maximum call stack depth
     */
    void setMaxCallStackDepth(u32 depth);
    
    // ========================================================================
    // API Registration Helpers
    // ========================================================================
    
    /**
     * @brief Begin registering a namespace
     */
    void beginNamespace(const std::string& name);
    
    /**
     * @brief End namespace registration
     */
    void endNamespace();
    
    /**
     * @brief Begin registering a class
     */
    void beginClass(const std::string& name, const std::string& baseClass = "");
    
    /**
     * @brief Register class constructor
     */
    void registerConstructor(NativeFunction ctor);
    
    /**
     * @brief Register class property
     */
    void registerProperty(const std::string& name, ScriptType type,
                         NativeMethod getter, NativeMethod setter = nullptr);
    
    /**
     * @brief Register class method
     */
    void registerMethod(const std::string& name, NativeMethod method);
    
    /**
     * @brief Register static method
     */
    void registerStaticMethod(const std::string& name, NativeFunction func);
    
    /**
     * @brief End class registration
     */
    void endClass();
    
    /**
     * @brief Register an enum type
     */
    void registerEnum(const std::string& name, const std::unordered_map<std::string, i64>& values);
    
private:
    ScriptEngine() = default;
    ~ScriptEngine() = default;
    ScriptEngine(const ScriptEngine&) = delete;
    ScriptEngine& operator=(const ScriptEngine&) = delete;
    
    // Internal methods
    void checkFileChanges();
    void processReloadQueue();
    void reportError(const ScriptError& error);
    
    // State
    bool m_initialized = false;
    
    // Modules
    std::unordered_map<std::string, ScriptModule> m_modules;
    std::set<std::string> m_modulesToReload;
    
    // Classes
    std::unordered_map<std::string, ScriptClass> m_classes;
    
    // Objects
    std::unordered_map<u64, std::shared_ptr<ScriptObject>> m_objects;
    u64 m_nextInstanceId = 1;
    
    // Functions
    std::unordered_map<std::string, NativeFunction> m_functions;
    std::unordered_map<std::string, FunctionSignature> m_functionSignatures;
    
    // Globals
    std::unordered_map<std::string, ScriptValue> m_globals;
    
    // Hot reload
    bool m_hotReloadEnabled = true;
    std::vector<std::string> m_watchDirectories;
    ReloadCallback m_reloadCallback;
    
    // Debugging
    bool m_debuggerEnabled = false;
    std::unique_ptr<ScriptDebugger> m_debugger;
    bool m_isPaused = false;
    
    enum class StepMode { None, Over, Into, Out, Continue };
    StepMode m_stepMode = StepMode::None;
    usize m_targetStackDepth = 0;
    
    struct Breakpoint {
        std::string file;
        u32 line = 0;
        bool enabled = true;
        u64 id = 0;
        std::string condition;
    };
    std::vector<Breakpoint> m_breakpoints;
    u64 m_nextBreakpointId = 1;
    
    std::vector<ScriptLocation> m_callStack;
    std::vector<std::unordered_map<std::string, ScriptValue>> m_localScopes;
    
    // Error handling
    ScriptError m_lastError;
    ScriptErrorCallback m_errorCallback;
    ScriptLogCallback m_logCallback;
    
    // Performance
    ExecutionStats m_stats;
    f32 m_executionTimeLimit = ScriptConfig::EXECUTION_TIME_LIMIT;
    u32 m_maxCallStackDepth = ScriptConfig::MAX_CALL_STACK_DEPTH;
    
    // Registration context
    std::string m_currentNamespace;
    ScriptClass m_currentClass;
    bool m_inClassDef = false;
};

} // namespace nova::script
