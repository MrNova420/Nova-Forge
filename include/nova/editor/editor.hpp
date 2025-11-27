/**
 * @file editor.hpp
 * @brief NovaCore Editor™ - Main Editor Include Header
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 * 
 * This is the main include header for the NovaCore Editor system.
 * Include this header to access all editor functionality.
 */

#pragma once

// Core types and configuration
#include "editor_types.hpp"

// Central editor context
#include "editor_context.hpp"

// Undo/Redo command system
#include "editor_command.hpp"

// Panel and window system
#include "editor_panel.hpp"

// Transform gizmos
#include "editor_gizmo.hpp"

namespace nova::editor {

/**
 * @brief NovaCore Editor version information
 */
namespace version {
    constexpr u32 MAJOR = 1;
    constexpr u32 MINOR = 0;
    constexpr u32 PATCH = 0;
    constexpr const char* STRING = "1.0.0";
    constexpr const char* CODENAME = "Genesis";
}

/**
 * @brief Initialize the NovaCore Editor
 * 
 * Call this once at application startup to initialize all editor systems.
 * 
 * @param projectPath Optional path to project to open
 * @return true if initialization succeeded
 */
inline bool initializeEditor(const std::string& projectPath = "") {
    return EditorContext::get().initialize(projectPath);
}

/**
 * @brief Shutdown the NovaCore Editor
 * 
 * Call this at application shutdown to clean up all editor systems.
 */
inline void shutdownEditor() {
    EditorContext::get().shutdown();
}

/**
 * @brief Update the editor (call each frame)
 * @param deltaTime Time since last frame in seconds
 */
inline void updateEditor(f32 deltaTime) {
    EditorContext::get().update(deltaTime);
}

/**
 * @brief Get the editor context singleton
 */
inline EditorContext& getEditorContext() {
    return EditorContext::get();
}

} // namespace nova::editor
