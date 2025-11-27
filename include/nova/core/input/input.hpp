/**
 * @file input.hpp
 * @brief NovaCore Input System™ - Main Include Header
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#pragma once

#include "input_types.hpp"
#include "input_system.hpp"

namespace nova::input {

/**
 * @brief Input system version
 */
namespace version {
    constexpr u32 MAJOR = 1;
    constexpr u32 MINOR = 0;
    constexpr u32 PATCH = 0;
    constexpr const char* STRING = "1.0.0";
}

/**
 * @brief Initialize the input system
 */
inline bool initializeInput() {
    return InputSystem::get().initialize();
}

/**
 * @brief Shutdown the input system
 */
inline void shutdownInput() {
    InputSystem::get().shutdown();
}

/**
 * @brief Update input (call each frame before game logic)
 */
inline void updateInput() {
    InputSystem::get().update();
}

/**
 * @brief Get the input system singleton
 */
inline InputSystem& getInput() {
    return InputSystem::get();
}

// Convenience functions
inline bool isKeyDown(Key key) { return InputSystem::get().isKeyDown(key); }
inline bool isKeyPressed(Key key) { return InputSystem::get().isKeyPressed(key); }
inline bool isKeyReleased(Key key) { return InputSystem::get().isKeyReleased(key); }

inline Vec2 getMousePosition() { return InputSystem::get().getMousePosition(); }
inline Vec2 getMouseDelta() { return InputSystem::get().getMouseDelta(); }
inline bool isMouseButtonDown(MouseButton btn) { return InputSystem::get().isMouseButtonDown(btn); }

inline u32 getTouchCount() { return InputSystem::get().getTouchCount(); }
inline bool isTouching() { return InputSystem::get().isTouching(); }

inline bool isActionDown(const std::string& action) { return InputSystem::get().isActionDown(action); }
inline bool isActionPressed(const std::string& action) { return InputSystem::get().isActionPressed(action); }
inline f32 getAxis(const std::string& axis) { return InputSystem::get().getAxis(axis); }

} // namespace nova::input
