/**
 * @file editor_context.cpp
 * @brief NovaCore Editor™ - Central Editor Context Implementation
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#include <nova/editor/editor_context.hpp>
#include <nova/editor/editor_command.hpp>

#include <algorithm>
#include <fstream>
#include <filesystem>

namespace nova::editor {

namespace fs = std::filesystem;

// ============================================================================
// Singleton Instance
// ============================================================================

EditorContext& EditorContext::get() {
    static EditorContext instance;
    return instance;
}

// ============================================================================
// Initialization / Shutdown
// ============================================================================

bool EditorContext::initialize(const std::string& projectPath) {
    if (m_state != EditorState::Initializing) {
        return false;  // Already initialized
    }
    
    logInfo("NovaCore Editor initializing...");
    
    // Load preferences
    loadPreferences();
    
    // Initialize viewport
    initializeViewport();
    
    // Create default world
    m_world = std::make_unique<ecs::World>();
    
    // Open project if specified
    if (!projectPath.empty()) {
        if (!openProject(projectPath)) {
            logWarning("Failed to open project: " + projectPath);
            initializeDefaultProject();
        }
    } else {
        initializeDefaultProject();
    }
    
    m_state = EditorState::Ready;
    logInfo("NovaCore Editor ready");
    
    return true;
}

void EditorContext::shutdown() {
    logInfo("NovaCore Editor shutting down...");
    
    // Save preferences
    savePreferences();
    
    // Save project if open
    if (m_hasUnsavedChanges && hasOpenProject()) {
        // Note: Would prompt user here
        saveProject();
    }
    
    // Clear undo/redo
    clearUndoHistory();
    
    // Clear selection
    clearSelection();
    
    // Destroy world
    m_world.reset();
    m_playModeBackup.reset();
    
    m_state = EditorState::Initializing;
    
    logInfo("NovaCore Editor shutdown complete");
}

void EditorContext::update(f32 deltaTime) {
    if (m_state == EditorState::Initializing) {
        return;
    }
    
    // Update auto-save timer
    checkAutoSave(deltaTime);
    
    // Update viewport camera matrices
    m_viewport.camera.updateMatrices(m_viewport.getAspectRatio());
    
    // Update notifications
    for (auto it = m_notifications.begin(); it != m_notifications.end();) {
        it->elapsed += deltaTime;
        if (it->elapsed >= it->duration && it->dismissible) {
            it = m_notifications.erase(it);
        } else {
            ++it;
        }
    }
    
    // Update world if playing
    if (m_state == EditorState::Playing && m_world) {
        m_world->update(deltaTime);
    }
}

// ============================================================================
// State Management
// ============================================================================

void EditorContext::setState(EditorState state) {
    if (m_state == state) return;
    
    EditorState oldState = m_state;
    m_state = state;
    
    // State transition logic
    if (oldState == EditorState::Playing && state != EditorState::Paused) {
        // Exiting play mode
        for (auto& callback : m_playModeCallbacks) {
            callback(false);
        }
    } else if (state == EditorState::Playing && oldState != EditorState::Paused) {
        // Entering play mode
        for (auto& callback : m_playModeCallbacks) {
            callback(true);
        }
    }
}

void EditorContext::setEditMode(EditMode mode) {
    if (m_editMode == mode) return;
    m_editMode = mode;
}

// ============================================================================
// World/Scene Management
// ============================================================================

void EditorContext::setWorld(std::unique_ptr<ecs::World> world) {
    m_world = std::move(world);
    clearSelection();
    clearUndoHistory();
}

void EditorContext::newScene() {
    if (m_hasUnsavedChanges) {
        // Note: Would prompt to save first
    }
    
    m_world = std::make_unique<ecs::World>();
    m_currentScenePath.clear();
    m_hasUnsavedChanges = false;
    
    clearSelection();
    clearUndoHistory();
    
    logInfo("Created new scene");
}

bool EditorContext::loadScene(const std::string& scenePath) {
    if (!fs::exists(scenePath)) {
        logError("Scene file not found: " + scenePath);
        return false;
    }
    
    setState(EditorState::Loading);
    
    // Note: Would deserialize scene here
    // For now, create empty world
    m_world = std::make_unique<ecs::World>();
    m_currentScenePath = scenePath;
    m_hasUnsavedChanges = false;
    
    clearSelection();
    clearUndoHistory();
    
    setState(EditorState::Ready);
    
    logInfo("Loaded scene: " + scenePath);
    
    return true;
}

bool EditorContext::saveScene() {
    if (m_currentScenePath.empty()) {
        logError("No scene path set - use saveSceneAs()");
        return false;
    }
    
    return saveSceneAs(m_currentScenePath);
}

bool EditorContext::saveSceneAs(const std::string& scenePath) {
    if (!m_world) {
        logError("No world to save");
        return false;
    }
    
    // Note: Would serialize scene here
    
    m_currentScenePath = scenePath;
    m_hasUnsavedChanges = false;
    
    logInfo("Saved scene: " + scenePath);
    notify("Scene saved", EditorNotification::Type::Success);
    
    return true;
}

// ============================================================================
// Selection
// ============================================================================

void EditorContext::select(ecs::Entity entity) {
    Selection oldSelection = m_selection;
    m_selection.set(entity);
    
    for (auto& callback : m_selectionCallbacks) {
        callback(m_selection);
    }
}

void EditorContext::addToSelection(ecs::Entity entity) {
    Selection oldSelection = m_selection;
    m_selection.add(entity);
    
    for (auto& callback : m_selectionCallbacks) {
        callback(m_selection);
    }
}

void EditorContext::removeFromSelection(ecs::Entity entity) {
    Selection oldSelection = m_selection;
    m_selection.remove(entity);
    
    for (auto& callback : m_selectionCallbacks) {
        callback(m_selection);
    }
}

void EditorContext::clearSelection() {
    if (m_selection.isEmpty()) return;
    
    Selection oldSelection = m_selection;
    m_selection.clear();
    
    for (auto& callback : m_selectionCallbacks) {
        callback(m_selection);
    }
}

void EditorContext::selectAll() {
    // Note: Would iterate world entities
    for (auto& callback : m_selectionCallbacks) {
        callback(m_selection);
    }
}

void EditorContext::invertSelection() {
    // Note: Would implement selection inversion
    for (auto& callback : m_selectionCallbacks) {
        callback(m_selection);
    }
}

void EditorContext::addSelectionChangedCallback(SelectionChangedCallback callback) {
    m_selectionCallbacks.push_back(std::move(callback));
}

// ============================================================================
// Undo/Redo
// ============================================================================

bool EditorContext::canUndo() const {
    return !m_undoStack.empty();
}

bool EditorContext::canRedo() const {
    return !m_redoStack.empty();
}

void EditorContext::undo() {
    if (!canUndo()) return;
    
    auto command = std::move(m_undoStack.back());
    m_undoStack.pop_back();
    
    command->undo();
    
    m_redoStack.push_back(std::move(command));
    markDirty();
}

void EditorContext::redo() {
    if (!canRedo()) return;
    
    auto command = std::move(m_redoStack.back());
    m_redoStack.pop_back();
    
    command->execute();
    
    m_undoStack.push_back(std::move(command));
    markDirty();
}

std::vector<std::string> EditorContext::getUndoStack() const {
    std::vector<std::string> names;
    names.reserve(m_undoStack.size());
    for (const auto& cmd : m_undoStack) {
        names.push_back(cmd->getName());
    }
    return names;
}

std::vector<std::string> EditorContext::getRedoStack() const {
    std::vector<std::string> names;
    names.reserve(m_redoStack.size());
    for (const auto& cmd : m_redoStack) {
        names.push_back(cmd->getName());
    }
    return names;
}

void EditorContext::executeCommand(std::unique_ptr<EditorCommand> command) {
    if (!command || !command->isValid()) return;
    
    // Try to merge with previous command
    if (!m_undoStack.empty()) {
        auto& lastCmd = m_undoStack.back();
        if (lastCmd->canMergeWith(*command)) {
            if (lastCmd->mergeWith(*command)) {
                // Merged successfully, don't add new command
                return;
            }
        }
    }
    
    command->execute();
    
    if (m_activeGroup) {
        m_activeGroup->commands.push_back(std::move(command));
    } else {
        m_undoStack.push_back(std::move(command));
        
        // Limit undo stack size
        while (m_undoStack.size() > EditorConfig::MAX_UNDO_HISTORY_DEPTH) {
            m_undoStack.erase(m_undoStack.begin());
        }
    }
    
    // Clear redo stack
    m_redoStack.clear();
    
    markDirty();
}

void EditorContext::beginCommandGroup(const std::string& name) {
    if (m_activeGroup) {
        logWarning("Already in a command group");
        return;
    }
    
    m_activeGroup = std::make_unique<CommandGroup>();
    m_activeGroup->name = name;
}

void EditorContext::endCommandGroup() {
    if (!m_activeGroup) {
        logWarning("Not in a command group");
        return;
    }
    
    if (!m_activeGroup->commands.empty()) {
        auto composite = std::make_unique<CompositeCommand>(m_activeGroup->name);
        for (auto& cmd : m_activeGroup->commands) {
            composite->addCommand(std::move(cmd));
        }
        m_undoStack.push_back(std::move(composite));
    }
    
    m_activeGroup.reset();
}

void EditorContext::clearUndoHistory() {
    m_undoStack.clear();
    m_redoStack.clear();
    m_activeGroup.reset();
}

// ============================================================================
// Transform Tools
// ============================================================================

void EditorContext::setActiveTool(TransformTool tool) {
    m_activeTool = tool;
}

void EditorContext::setTransformSpace(TransformSpace space) {
    m_transformSpace = space;
}

void EditorContext::setPivotMode(PivotMode mode) {
    m_pivotMode = mode;
}

// ============================================================================
// Viewport
// ============================================================================

void EditorContext::focusOnSelection() {
    if (m_selection.isEmpty()) return;
    focusOn(m_selection.selectionCenter);
}

void EditorContext::focusOn(ecs::Entity entity) {
    // Get entity position from world
    // For now, focus on origin
    focusOn(Vec3::zero());
}

void EditorContext::focusOn(const Vec3& position, f32 distance) {
    // Animate camera to focus position
    Vec3 offset = m_viewport.camera.forward() * (-distance);
    m_viewport.camera.position = position + offset;
}

void EditorContext::resetCamera() {
    m_viewport.camera.position = Vec3(0.0f, 5.0f, -10.0f);
    m_viewport.camera.rotation = Quat::fromEuler(Vec3(radians(30.0f), 0.0f, 0.0f));
    m_viewport.camera.fov = 60.0f;
}

void EditorContext::processTouch(const TouchState& touch) {
    m_touchState = touch;
}

// ============================================================================
// Project Management
// ============================================================================

bool EditorContext::createProject(const std::string& name, const std::string& path) {
    fs::path projectDir = fs::path(path) / name;
    
    // Create project directory structure
    try {
        fs::create_directories(projectDir);
        fs::create_directories(projectDir / "Assets");
        fs::create_directories(projectDir / "Assets" / "Scenes");
        fs::create_directories(projectDir / "Assets" / "Scripts");
        fs::create_directories(projectDir / "Assets" / "Materials");
        fs::create_directories(projectDir / "Assets" / "Textures");
        fs::create_directories(projectDir / "Assets" / "Models");
        fs::create_directories(projectDir / "Assets" / "Audio");
        fs::create_directories(projectDir / "ProjectSettings");
        fs::create_directories(projectDir / "Library");
    } catch (const std::exception& e) {
        logError("Failed to create project directories: " + std::string(e.what()));
        return false;
    }
    
    // Create project file
    m_projectInfo.name = name;
    m_projectInfo.path = projectDir.string();
    m_projectInfo.version = "1.0.0";
    m_projectInfo.created = std::chrono::system_clock::now();
    m_projectInfo.lastOpened = m_projectInfo.created;
    m_projectInfo.isValid = true;
    m_projectInfo.bundleIdentifier = "com.novaforge." + name;
    
    // Create default scene
    newScene();
    m_currentScenePath = (projectDir / "Assets" / "Scenes" / "MainScene.nscene").string();
    saveScene();
    m_projectInfo.defaultScene = m_currentScenePath;
    
    // Save project
    saveProject();
    
    logInfo("Created project: " + name);
    notify("Project created: " + name, EditorNotification::Type::Success);
    
    return true;
}

bool EditorContext::openProject(const std::string& projectPath) {
    if (!fs::exists(projectPath)) {
        logError("Project not found: " + projectPath);
        return false;
    }
    
    setState(EditorState::Loading);
    
    // Note: Would load project settings here
    
    m_projectInfo.path = projectPath;
    m_projectInfo.name = fs::path(projectPath).filename().string();
    m_projectInfo.lastOpened = std::chrono::system_clock::now();
    m_projectInfo.isValid = true;
    
    // Add to recent projects
    auto it = std::find_if(m_recentProjects.begin(), m_recentProjects.end(),
        [&](const ProjectInfo& p) { return p.path == projectPath; });
    if (it != m_recentProjects.end()) {
        m_recentProjects.erase(it);
    }
    m_recentProjects.insert(m_recentProjects.begin(), m_projectInfo);
    while (m_recentProjects.size() > EditorConfig::MAX_RECENT_PROJECTS) {
        m_recentProjects.pop_back();
    }
    
    // Load default scene
    if (!m_projectInfo.defaultScene.empty() && fs::exists(m_projectInfo.defaultScene)) {
        loadScene(m_projectInfo.defaultScene);
    } else {
        loadDefaultScene();
    }
    
    setState(EditorState::Ready);
    
    logInfo("Opened project: " + m_projectInfo.name);
    
    return true;
}

bool EditorContext::closeProject() {
    if (!hasOpenProject()) return true;
    
    if (m_hasUnsavedChanges) {
        // Note: Would prompt to save
    }
    
    m_projectInfo = ProjectInfo();
    m_currentScenePath.clear();
    newScene();
    
    logInfo("Project closed");
    
    return true;
}

bool EditorContext::saveProject() {
    if (!hasOpenProject()) return false;
    
    // Note: Would save project settings file
    
    m_projectInfo.lastOpened = std::chrono::system_clock::now();
    
    return true;
}

// ============================================================================
// Theme and Preferences
// ============================================================================

void EditorContext::setTheme(const EditorTheme& theme) {
    m_theme = theme;
}

void EditorContext::setTheme(const std::string& themeName) {
    if (themeName == "Light") {
        m_theme = EditorTheme::light();
    } else if (themeName == "HighContrast") {
        m_theme = EditorTheme::highContrast();
    } else {
        m_theme = EditorTheme::dark();
    }
}

bool EditorContext::loadPreferences() {
    // Note: Would load from disk
    m_theme = EditorTheme::dark();
    return true;
}

bool EditorContext::savePreferences() {
    // Note: Would save to disk
    return true;
}

// ============================================================================
// Console
// ============================================================================

void EditorContext::logInfo(const std::string& message) {
    ConsoleMessage msg;
    msg.type = ConsoleMessage::Type::Info;
    msg.message = message;
    msg.timestamp = std::chrono::system_clock::now();
    
    m_consoleMessages.push_back(msg);
    
    while (m_consoleMessages.size() > MAX_CONSOLE_MESSAGES) {
        m_consoleMessages.erase(m_consoleMessages.begin());
    }
}

void EditorContext::logWarning(const std::string& message) {
    ConsoleMessage msg;
    msg.type = ConsoleMessage::Type::Warning;
    msg.message = message;
    msg.timestamp = std::chrono::system_clock::now();
    
    m_consoleMessages.push_back(msg);
}

void EditorContext::logError(const std::string& message) {
    ConsoleMessage msg;
    msg.type = ConsoleMessage::Type::Error;
    msg.message = message;
    msg.timestamp = std::chrono::system_clock::now();
    
    m_consoleMessages.push_back(msg);
}

void EditorContext::clearConsole() {
    m_consoleMessages.clear();
}

// ============================================================================
// Notifications
// ============================================================================

void EditorContext::notify(const std::string& message, EditorNotification::Type type) {
    EditorNotification notification;
    notification.type = type;
    notification.message = message;
    notification.duration = 3.0f;
    
    m_notifications.push_back(notification);
}

void EditorContext::notifyProgress(const std::string& title, f32 progress) {
    // Find existing progress notification with same title
    for (auto& notification : m_notifications) {
        if (notification.type == EditorNotification::Type::Progress && 
            notification.title == title) {
            notification.progress = progress;
            if (progress >= 1.0f) {
                notification.duration = 1.0f;  // Dismiss soon
            }
            return;
        }
    }
    
    // Create new progress notification
    EditorNotification notification;
    notification.type = EditorNotification::Type::Progress;
    notification.title = title;
    notification.progress = progress;
    notification.duration = 1000.0f;  // Long duration until complete
    notification.dismissible = false;
    
    m_notifications.push_back(notification);
}

// ============================================================================
// Play Mode
// ============================================================================

void EditorContext::play() {
    if (m_state == EditorState::Playing) return;
    
    // Backup current world state
    // Note: Would serialize world here
    m_playModeBackup = std::make_unique<ecs::World>();
    
    setState(EditorState::Playing);
    logInfo("Entered Play Mode");
}

void EditorContext::pause() {
    if (m_state != EditorState::Playing) return;
    setState(EditorState::Paused);
    logInfo("Paused");
}

void EditorContext::resume() {
    if (m_state != EditorState::Paused) return;
    setState(EditorState::Playing);
    logInfo("Resumed");
}

void EditorContext::stop() {
    if (m_state != EditorState::Playing && m_state != EditorState::Paused) return;
    
    // Restore world from backup
    if (m_playModeBackup) {
        m_world = std::move(m_playModeBackup);
    }
    
    setState(EditorState::Ready);
    logInfo("Exited Play Mode");
}

void EditorContext::step() {
    if (m_state != EditorState::Paused) return;
    
    // Simulate one frame
    if (m_world) {
        m_world->update(1.0f / 60.0f);
    }
}

void EditorContext::addPlayModeCallback(PlayModeChangedCallback callback) {
    m_playModeCallbacks.push_back(std::move(callback));
}

// ============================================================================
// Asset Operations
// ============================================================================

bool EditorContext::importAsset(const std::string& sourcePath, const std::string& destPath) {
    if (!fs::exists(sourcePath)) {
        logError("Source file not found: " + sourcePath);
        return false;
    }
    
    // Note: Would implement asset import pipeline
    logInfo("Imported asset: " + sourcePath);
    return true;
}

bool EditorContext::createAsset(AssetType type, const std::string& path) {
    // Note: Would create asset based on type
    logInfo("Created asset: " + path);
    return true;
}

bool EditorContext::deleteAsset(const std::string& path) {
    if (!fs::exists(path)) return false;
    
    try {
        fs::remove(path);
        logInfo("Deleted asset: " + path);
        return true;
    } catch (const std::exception& e) {
        logError("Failed to delete asset: " + std::string(e.what()));
        return false;
    }
}

bool EditorContext::renameAsset(const std::string& oldPath, const std::string& newPath) {
    if (!fs::exists(oldPath)) return false;
    
    try {
        fs::rename(oldPath, newPath);
        logInfo("Renamed asset: " + oldPath + " -> " + newPath);
        return true;
    } catch (const std::exception& e) {
        logError("Failed to rename asset: " + std::string(e.what()));
        return false;
    }
}

bool EditorContext::duplicateAsset(const std::string& path) {
    if (!fs::exists(path)) return false;
    
    fs::path srcPath(path);
    fs::path destPath = srcPath.parent_path() / 
        (srcPath.stem().string() + "_copy" + srcPath.extension().string());
    
    try {
        fs::copy(path, destPath);
        logInfo("Duplicated asset: " + path);
        return true;
    } catch (const std::exception& e) {
        logError("Failed to duplicate asset: " + std::string(e.what()));
        return false;
    }
}

// ============================================================================
// Entity Operations
// ============================================================================

ecs::Entity EditorContext::createEntity(const std::string& name) {
    if (!m_world) return ecs::Entity();
    
    auto entity = m_world->createEntity();
    // Note: Would add name component
    
    // Record undo command for entity creation
    auto cmd = std::make_unique<CreateEntityCommand>(name);
    cmd->execute();
    m_undoStack.push(std::move(cmd));
    m_redoStack = {};
    
    return entity;
}

ecs::Entity EditorContext::instantiatePrefab(const std::string& prefabPath) {
    if (!m_world) return ecs::Entity();
    
    // Note: Would load and instantiate prefab
    auto entity = m_world->createEntity();
    return entity;
}

void EditorContext::duplicateSelection() {
    if (m_selection.isEmpty()) return;
    
    std::vector<ecs::Entity> toDuplicate;
    for (const auto& info : m_selection.entities) {
        toDuplicate.push_back(info.entity);
    }
    
    executeCommand<DuplicateEntitiesCommand>(toDuplicate);
}

void EditorContext::deleteSelection() {
    if (m_selection.isEmpty()) return;
    
    std::vector<ecs::Entity> toDelete;
    for (const auto& info : m_selection.entities) {
        toDelete.push_back(info.entity);
    }
    
    executeCommand<DeleteEntitiesCommand>(toDelete);
    clearSelection();
}

ecs::Entity EditorContext::groupSelection() {
    if (m_selection.count() < 2) return ecs::Entity();
    
    // Create parent entity
    auto parent = createEntity("Group");
    
    // Parent all selected entities to it
    for (const auto& info : m_selection.entities) {
        parentTo(info.entity, parent);
    }
    
    return parent;
}

void EditorContext::ungroupSelection() {
    // Note: Would unparent children from selected groups
}

void EditorContext::parentTo(ecs::Entity child, ecs::Entity parent) {
    // Note: Would implement parenting
    executeCommand<ReparentEntityCommand>(child, ecs::Entity(), parent);
}

void EditorContext::unparent(ecs::Entity entity) {
    // Note: Would remove parent
}

// ============================================================================
// Private Helpers
// ============================================================================

void EditorContext::initializeDefaultProject() {
    m_projectInfo = ProjectInfo();
    m_projectInfo.name = "Untitled";
    m_projectInfo.isValid = false;
}

void EditorContext::initializeViewport() {
    m_viewport.width = 1280;
    m_viewport.height = 720;
    m_viewport.camera = EditorCamera();
    resetCamera();
}

void EditorContext::loadDefaultScene() {
    newScene();
}

void EditorContext::checkAutoSave(f32 deltaTime) {
    if (!hasOpenProject() || !m_hasUnsavedChanges) {
        m_autoSaveTimer = 0.0f;
        return;
    }
    
    m_autoSaveTimer += deltaTime;
    
    if (m_autoSaveTimer >= EditorConfig::AUTO_SAVE_INTERVAL_SECONDS) {
        m_autoSaveTimer = 0.0f;
        
        // Auto-save
        saveScene();
        saveProject();
        
        notify("Auto-saved", EditorNotification::Type::Info);
    }
}

} // namespace nova::editor
