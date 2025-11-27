/**
 * @file editor_context.hpp
 * @brief NovaCore Editor™ - Central Editor Context and State Management
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 * 
 * The EditorContext class is the central hub for all editor state and services.
 * It manages the selection, undo/redo, project state, viewport state, and
 * provides access to all editor subsystems.
 */

#pragma once

#include "editor_types.hpp"
#include <nova/core/ecs/world.hpp>

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <filesystem>

namespace nova::editor {

// ============================================================================
// Forward Declarations
// ============================================================================

class EditorCommand;
class EditorPanel;
class EditorTool;
class SelectionManager;
class UndoRedoManager;
class HotReloadManager;
class AssetDatabase;

// ============================================================================
// Editor Context
// ============================================================================

/**
 * @brief Central editor context managing all editor state and services
 * 
 * EditorContext is a singleton that provides:
 * - Access to the current World (scene)
 * - Selection management
 * - Undo/redo system
 * - Project management
 * - Viewport state
 * - Theme and preferences
 * - Event callbacks
 * 
 * Usage:
 * @code
 *     auto& ctx = EditorContext::get();
 *     ctx.selection().add(entity);
 *     ctx.executeCommand<MoveCommand>(entity, newPosition);
 *     ctx.undo();
 * @endcode
 */
class EditorContext {
public:
    /**
     * @brief Get the singleton instance
     */
    static EditorContext& get();
    
    /**
     * @brief Initialize the editor context
     * @param projectPath Optional path to project to open
     * @return true if initialization succeeded
     */
    bool initialize(const std::string& projectPath = "");
    
    /**
     * @brief Shutdown the editor context
     */
    void shutdown();
    
    /**
     * @brief Update editor state (call each frame)
     * @param deltaTime Time since last frame
     */
    void update(f32 deltaTime);
    
    // ========================================================================
    // State Accessors
    // ========================================================================
    
    /** @brief Get current editor state */
    EditorState getState() const { return m_state; }
    
    /** @brief Set editor state */
    void setState(EditorState state);
    
    /** @brief Get current editing mode */
    EditMode getEditMode() const { return m_editMode; }
    
    /** @brief Set editing mode */
    void setEditMode(EditMode mode);
    
    /** @brief Check if in play mode */
    bool isPlaying() const { return m_state == EditorState::Playing; }
    
    /** @brief Check if paused */
    bool isPaused() const { return m_state == EditorState::Paused; }
    
    // ========================================================================
    // World/Scene Access
    // ========================================================================
    
    /** @brief Get the current world (scene) */
    ecs::World* getWorld() { return m_world.get(); }
    const ecs::World* getWorld() const { return m_world.get(); }
    
    /** @brief Set the current world */
    void setWorld(std::unique_ptr<ecs::World> world);
    
    /** @brief Create a new empty scene */
    void newScene();
    
    /** @brief Load a scene from file */
    bool loadScene(const std::string& scenePath);
    
    /** @brief Save current scene */
    bool saveScene();
    
    /** @brief Save scene to a new path */
    bool saveSceneAs(const std::string& scenePath);
    
    /** @brief Get current scene path */
    const std::string& getScenePath() const { return m_currentScenePath; }
    
    /** @brief Check if scene has unsaved changes */
    bool hasUnsavedChanges() const { return m_hasUnsavedChanges; }
    
    /** @brief Mark scene as having unsaved changes */
    void markDirty() { m_hasUnsavedChanges = true; }
    
    // ========================================================================
    // Selection
    // ========================================================================
    
    /** @brief Get current selection */
    Selection& selection() { return m_selection; }
    const Selection& selection() const { return m_selection; }
    
    /** @brief Select a single entity (clears previous selection) */
    void select(ecs::Entity entity);
    
    /** @brief Add entity to selection */
    void addToSelection(ecs::Entity entity);
    
    /** @brief Remove entity from selection */
    void removeFromSelection(ecs::Entity entity);
    
    /** @brief Clear selection */
    void clearSelection();
    
    /** @brief Select all entities in scene */
    void selectAll();
    
    /** @brief Invert selection */
    void invertSelection();
    
    /** @brief Add selection changed callback */
    void addSelectionChangedCallback(SelectionChangedCallback callback);
    
    // ========================================================================
    // Undo/Redo
    // ========================================================================
    
    /** @brief Check if undo is available */
    bool canUndo() const;
    
    /** @brief Check if redo is available */
    bool canRedo() const;
    
    /** @brief Undo last command */
    void undo();
    
    /** @brief Redo last undone command */
    void redo();
    
    /** @brief Get undo stack description */
    std::vector<std::string> getUndoStack() const;
    
    /** @brief Get redo stack description */
    std::vector<std::string> getRedoStack() const;
    
    /** @brief Execute a command (adds to undo stack) */
    void executeCommand(std::unique_ptr<EditorCommand> command);
    
    /** @brief Execute a command with automatic construction */
    template<typename CommandType, typename... Args>
    void executeCommand(Args&&... args);
    
    /** @brief Begin a command group (multiple commands as one undo) */
    void beginCommandGroup(const std::string& name);
    
    /** @brief End command group */
    void endCommandGroup();
    
    /** @brief Clear undo/redo history */
    void clearUndoHistory();
    
    // ========================================================================
    // Transform Tools
    // ========================================================================
    
    /** @brief Get active transform tool */
    TransformTool getActiveTool() const { return m_activeTool; }
    
    /** @brief Set active transform tool */
    void setActiveTool(TransformTool tool);
    
    /** @brief Get transform space */
    TransformSpace getTransformSpace() const { return m_transformSpace; }
    
    /** @brief Set transform space */
    void setTransformSpace(TransformSpace space);
    
    /** @brief Get pivot mode */
    PivotMode getPivotMode() const { return m_pivotMode; }
    
    /** @brief Set pivot mode */
    void setPivotMode(PivotMode mode);
    
    /** @brief Get snap settings */
    SnapSettings& getSnapSettings() { return m_snapSettings; }
    const SnapSettings& getSnapSettings() const { return m_snapSettings; }
    
    /** @brief Get gizmo settings */
    GizmoSettings& getGizmoSettings() { return m_gizmoSettings; }
    const GizmoSettings& getGizmoSettings() const { return m_gizmoSettings; }
    
    // ========================================================================
    // Viewport
    // ========================================================================
    
    /** @brief Get main viewport state */
    ViewportState& getViewport() { return m_viewport; }
    const ViewportState& getViewport() const { return m_viewport; }
    
    /** @brief Get grid settings */
    GridSettings& getGridSettings() { return m_gridSettings; }
    const GridSettings& getGridSettings() const { return m_gridSettings; }
    
    /** @brief Focus camera on selection */
    void focusOnSelection();
    
    /** @brief Focus camera on entity */
    void focusOn(ecs::Entity entity);
    
    /** @brief Focus camera on world position */
    void focusOn(const Vec3& position, f32 distance = 10.0f);
    
    /** @brief Reset camera to default position */
    void resetCamera();
    
    // ========================================================================
    // Touch Input (Mobile)
    // ========================================================================
    
    /** @brief Get touch state */
    TouchState& getTouchState() { return m_touchState; }
    const TouchState& getTouchState() const { return m_touchState; }
    
    /** @brief Process touch input */
    void processTouch(const TouchState& touch);
    
    // ========================================================================
    // Project Management
    // ========================================================================
    
    /** @brief Get current project info */
    const ProjectInfo& getProjectInfo() const { return m_projectInfo; }
    
    /** @brief Create a new project */
    bool createProject(const std::string& name, const std::string& path);
    
    /** @brief Open a project */
    bool openProject(const std::string& projectPath);
    
    /** @brief Close current project */
    bool closeProject();
    
    /** @brief Save project settings */
    bool saveProject();
    
    /** @brief Get recent projects list */
    const std::vector<ProjectInfo>& getRecentProjects() const { return m_recentProjects; }
    
    /** @brief Check if project is open */
    bool hasOpenProject() const { return m_projectInfo.isValid; }
    
    // ========================================================================
    // Theme and Preferences
    // ========================================================================
    
    /** @brief Get current theme */
    EditorTheme& getTheme() { return m_theme; }
    const EditorTheme& getTheme() const { return m_theme; }
    
    /** @brief Set theme */
    void setTheme(const EditorTheme& theme);
    
    /** @brief Set theme by name (Dark, Light, HighContrast) */
    void setTheme(const std::string& themeName);
    
    /** @brief Load preferences from disk */
    bool loadPreferences();
    
    /** @brief Save preferences to disk */
    bool savePreferences();
    
    // ========================================================================
    // Console
    // ========================================================================
    
    /** @brief Log info message */
    void logInfo(const std::string& message);
    
    /** @brief Log warning message */
    void logWarning(const std::string& message);
    
    /** @brief Log error message */
    void logError(const std::string& message);
    
    /** @brief Get console messages */
    const std::vector<ConsoleMessage>& getConsoleMessages() const { return m_consoleMessages; }
    
    /** @brief Clear console */
    void clearConsole();
    
    // ========================================================================
    // Notifications
    // ========================================================================
    
    /** @brief Show notification */
    void notify(const std::string& message, EditorNotification::Type type = EditorNotification::Type::Info);
    
    /** @brief Show progress notification */
    void notifyProgress(const std::string& title, f32 progress);
    
    /** @brief Get active notifications */
    const std::vector<EditorNotification>& getNotifications() const { return m_notifications; }
    
    // ========================================================================
    // Play Mode
    // ========================================================================
    
    /** @brief Enter play mode */
    void play();
    
    /** @brief Pause play mode */
    void pause();
    
    /** @brief Resume from pause */
    void resume();
    
    /** @brief Stop play mode */
    void stop();
    
    /** @brief Step one frame (when paused) */
    void step();
    
    /** @brief Add play mode callback */
    void addPlayModeCallback(PlayModeChangedCallback callback);
    
    // ========================================================================
    // Asset Operations
    // ========================================================================
    
    /** @brief Import asset from file */
    bool importAsset(const std::string& sourcePath, const std::string& destPath = "");
    
    /** @brief Create new asset */
    bool createAsset(AssetType type, const std::string& path);
    
    /** @brief Delete asset */
    bool deleteAsset(const std::string& path);
    
    /** @brief Rename asset */
    bool renameAsset(const std::string& oldPath, const std::string& newPath);
    
    /** @brief Duplicate asset */
    bool duplicateAsset(const std::string& path);
    
    // ========================================================================
    // Entity Operations
    // ========================================================================
    
    /** @brief Create empty entity */
    ecs::Entity createEntity(const std::string& name = "New Entity");
    
    /** @brief Create entity from prefab */
    ecs::Entity instantiatePrefab(const std::string& prefabPath);
    
    /** @brief Duplicate selected entities */
    void duplicateSelection();
    
    /** @brief Delete selected entities */
    void deleteSelection();
    
    /** @brief Group selected entities */
    ecs::Entity groupSelection();
    
    /** @brief Ungroup selected entities */
    void ungroupSelection();
    
    /** @brief Parent entity to another */
    void parentTo(ecs::Entity child, ecs::Entity parent);
    
    /** @brief Unparent entity */
    void unparent(ecs::Entity entity);
    
private:
    EditorContext() = default;
    ~EditorContext() = default;
    EditorContext(const EditorContext&) = delete;
    EditorContext& operator=(const EditorContext&) = delete;
    
    // Initialization helpers
    void initializeDefaultProject();
    void initializeViewport();
    void loadDefaultScene();
    
    // Auto-save
    void checkAutoSave(f32 deltaTime);
    f32 m_autoSaveTimer = 0.0f;
    
    // State
    EditorState m_state = EditorState::Initializing;
    EditMode m_editMode = EditMode::Scene;
    bool m_hasUnsavedChanges = false;
    
    // World/Scene
    std::unique_ptr<ecs::World> m_world;
    std::unique_ptr<ecs::World> m_playModeBackup;  // Backup before play mode
    std::string m_currentScenePath;
    
    // Selection
    Selection m_selection;
    std::vector<SelectionChangedCallback> m_selectionCallbacks;
    
    // Undo/Redo
    struct CommandGroup {
        std::string name;
        std::vector<std::unique_ptr<EditorCommand>> commands;
    };
    std::vector<std::unique_ptr<EditorCommand>> m_undoStack;
    std::vector<std::unique_ptr<EditorCommand>> m_redoStack;
    std::unique_ptr<CommandGroup> m_activeGroup;
    
    // Transform tools
    TransformTool m_activeTool = TransformTool::Move;
    TransformSpace m_transformSpace = TransformSpace::World;
    PivotMode m_pivotMode = PivotMode::Center;
    SnapSettings m_snapSettings;
    GizmoSettings m_gizmoSettings;
    
    // Viewport
    ViewportState m_viewport;
    GridSettings m_gridSettings;
    
    // Touch input
    TouchState m_touchState;
    
    // Project
    ProjectInfo m_projectInfo;
    std::vector<ProjectInfo> m_recentProjects;
    
    // Theme
    EditorTheme m_theme;
    
    // Console
    std::vector<ConsoleMessage> m_consoleMessages;
    static constexpr usize MAX_CONSOLE_MESSAGES = 1000;
    
    // Notifications
    std::vector<EditorNotification> m_notifications;
    
    // Play mode callbacks
    std::vector<PlayModeChangedCallback> m_playModeCallbacks;
};

// ============================================================================
// Template Implementations
// ============================================================================

template<typename CommandType, typename... Args>
void EditorContext::executeCommand(Args&&... args) {
    executeCommand(std::make_unique<CommandType>(std::forward<Args>(args)...));
}

} // namespace nova::editor
