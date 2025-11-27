/**
 * @file script.hpp
 * @brief NovaCore Script System™ - Main Include Header
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#pragma once

#include "script_types.hpp"
#include "script_engine.hpp"

namespace nova::script {

/**
 * @brief Script system version
 */
namespace version {
    constexpr u32 MAJOR = 1;
    constexpr u32 MINOR = 0;
    constexpr u32 PATCH = 0;
    constexpr const char* STRING = "1.0.0";
}

/**
 * @brief Initialize the script system
 */
inline bool initializeScript() {
    return ScriptEngine::get().initialize();
}

/**
 * @brief Shutdown the script system
 */
inline void shutdownScript() {
    ScriptEngine::get().shutdown();
}

/**
 * @brief Update script engine (call each frame)
 */
inline void updateScript(f32 deltaTime) {
    ScriptEngine::get().update(deltaTime);
}

/**
 * @brief Get the script engine singleton
 */
inline ScriptEngine& getScript() {
    return ScriptEngine::get();
}

// Convenience functions
inline bool loadScript(const std::string& path) {
    return ScriptEngine::get().loadModule(path);
}

inline ScriptValue callScript(const std::string& func, const std::vector<ScriptValue>& args = {}) {
    return ScriptEngine::get().callFunction(func, args);
}

inline void setGlobalVar(const std::string& name, const ScriptValue& value) {
    ScriptEngine::get().setGlobal(name, value);
}

inline ScriptValue getGlobalVar(const std::string& name) {
    return ScriptEngine::get().getGlobal(name);
}

} // namespace nova::script
