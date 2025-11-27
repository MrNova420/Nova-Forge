/**
 * @file editor_panel.hpp
 * @brief NovaCore Editor™ - Base Panel and Window System
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 * 
 * Defines the base classes for editor panels and windows. Panels are
 * dockable UI elements that provide specific functionality (hierarchy,
 * inspector, console, etc.). The panel system supports:
 * - Docking and undocking
 * - Tab grouping
 * - State persistence
 * - Touch and keyboard input
 */

#pragma once

#include "editor_types.hpp"
#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace nova::editor {

// ============================================================================
// Forward Declarations
// ============================================================================

class EditorContext;

// ============================================================================
// Panel Flags
// ============================================================================

/**
 * @brief Flags controlling panel behavior
 */
enum class PanelFlags : u32 {
    None            = 0,
    NoClose         = 1 << 0,      // Cannot be closed
    NoMove          = 1 << 1,      // Cannot be moved
    NoResize        = 1 << 2,      // Cannot be resized
    NoCollapse      = 1 << 3,      // Cannot be collapsed
    NoTitleBar      = 1 << 4,      // No title bar
    NoScrollbar     = 1 << 5,      // No scrollbar
    NoDocking       = 1 << 6,      // Cannot be docked
    AlwaysAutoResize = 1 << 7,     // Auto-resize to content
    NoBackground    = 1 << 8,      // Transparent background
    NoNavigation    = 1 << 9,      // No keyboard navigation
    UnsavedDocument = 1 << 10,     // Show unsaved indicator
    Modal           = 1 << 11,     // Modal dialog
    Popup           = 1 << 12,     // Popup window
    NoFocusOnAppearing = 1 << 13   // Don't focus when appearing
};

inline PanelFlags operator|(PanelFlags a, PanelFlags b) {
    return static_cast<PanelFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

inline bool hasFlag(PanelFlags flags, PanelFlags test) {
    return (static_cast<u32>(flags) & static_cast<u32>(test)) != 0;
}

// ============================================================================
// Base Panel Class
// ============================================================================

/**
 * @brief Base class for all editor panels
 * 
 * Panels are the primary UI elements in the editor. Each panel provides
 * specific functionality (scene hierarchy, inspector, console, etc.).
 * 
 * Panels support:
 * - Docking into the main editor layout
 * - State persistence across sessions
 * - Focus and hover tracking
 * - Keyboard shortcuts
 * - Touch input for mobile
 * 
 * Usage:
 * @code
 *     class MyPanel : public EditorPanel {
 *     public:
 *         MyPanel() : EditorPanel("My Panel", "my_panel") {}
 *         
 *         void onDraw() override {
 *             // Draw panel content
 *         }
 *     };
 * @endcode
 */
class EditorPanel {
public:
    /**
     * @brief Construct a new panel
     * @param title Display title
     * @param id Unique identifier for persistence
     * @param flags Panel behavior flags
     */
    EditorPanel(const std::string& title, 
                const std::string& id,
                PanelFlags flags = PanelFlags::None);
    
    virtual ~EditorPanel() = default;
    
    // ========================================================================
    // Lifecycle
    // ========================================================================
    
    /**
     * @brief Called when panel is first created
     */
    virtual void onInitialize() {}
    
    /**
     * @brief Called when panel is being destroyed
     */
    virtual void onShutdown() {}
    
    /**
     * @brief Called each frame to update panel state
     * @param deltaTime Time since last frame
     */
    virtual void onUpdate(f32 deltaTime) {}
    
    /**
     * @brief Called each frame to draw panel content
     * 
     * This is the main method to override. Draw your panel's UI here.
     */
    virtual void onDraw() = 0;
    
    /**
     * @brief Called when panel becomes visible
     */
    virtual void onShow() {}
    
    /**
     * @brief Called when panel becomes hidden
     */
    virtual void onHide() {}
    
    /**
     * @brief Called when panel gains focus
     */
    virtual void onFocus() {}
    
    /**
     * @brief Called when panel loses focus
     */
    virtual void onBlur() {}
    
    // ========================================================================
    // Input Handling
    // ========================================================================
    
    /**
     * @brief Handle keyboard input
     * @param key Key code
     * @param mods Modifier keys (Shift, Ctrl, Alt)
     * @return true if input was handled
     */
    virtual bool onKeyPressed(int key, int mods) { return false; }
    
    /**
     * @brief Handle keyboard release
     * @param key Key code
     * @param mods Modifier keys
     * @return true if input was handled
     */
    virtual bool onKeyReleased(int key, int mods) { return false; }
    
    /**
     * @brief Handle character input (for text fields)
     * @param codepoint Unicode codepoint
     * @return true if input was handled
     */
    virtual bool onCharInput(u32 codepoint) { return false; }
    
    /**
     * @brief Handle mouse/touch press
     * @param button Button index (0=left, 1=right, 2=middle)
     * @param position Position in panel coordinates
     * @return true if input was handled
     */
    virtual bool onMousePressed(int button, const Vec2& position) { return false; }
    
    /**
     * @brief Handle mouse/touch release
     * @param button Button index
     * @param position Position in panel coordinates
     * @return true if input was handled
     */
    virtual bool onMouseReleased(int button, const Vec2& position) { return false; }
    
    /**
     * @brief Handle mouse/touch move
     * @param position Position in panel coordinates
     * @param delta Movement delta
     * @return true if input was handled
     */
    virtual bool onMouseMoved(const Vec2& position, const Vec2& delta) { return false; }
    
    /**
     * @brief Handle scroll/pinch
     * @param delta Scroll amount (y = vertical, x = horizontal)
     * @return true if input was handled
     */
    virtual bool onScroll(const Vec2& delta) { return false; }
    
    /**
     * @brief Handle drag and drop
     * @param paths Dropped file paths
     * @return true if drop was handled
     */
    virtual bool onDrop(const std::vector<std::string>& paths) { return false; }
    
    /**
     * @brief Handle touch gesture (mobile)
     * @param gesture Gesture type
     * @param state Touch state with positions
     * @return true if gesture was handled
     */
    virtual bool onTouchGesture(TouchGesture gesture, const TouchState& state) { return false; }
    
    // ========================================================================
    // Menu
    // ========================================================================
    
    /**
     * @brief Draw panel-specific menu items
     * 
     * Called when the panel's context menu or menu bar section is shown.
     */
    virtual void onDrawMenu() {}
    
    /**
     * @brief Draw panel-specific toolbar items
     */
    virtual void onDrawToolbar() {}
    
    // ========================================================================
    // State Persistence
    // ========================================================================
    
    /**
     * @brief Save panel state to JSON
     * @param state Output state object (JSON-like)
     */
    virtual void onSaveState(std::unordered_map<std::string, std::string>& state) {}
    
    /**
     * @brief Load panel state from JSON
     * @param state Input state object
     */
    virtual void onLoadState(const std::unordered_map<std::string, std::string>& state) {}
    
    // ========================================================================
    // Accessors
    // ========================================================================
    
    /** @brief Get panel title */
    const std::string& getTitle() const { return m_title; }
    
    /** @brief Set panel title */
    void setTitle(const std::string& title) { m_title = title; }
    
    /** @brief Get unique panel ID */
    const std::string& getId() const { return m_id; }
    
    /** @brief Get panel flags */
    PanelFlags getFlags() const { return m_flags; }
    
    /** @brief Set panel flags */
    void setFlags(PanelFlags flags) { m_flags = flags; }
    
    /** @brief Add panel flags */
    void addFlags(PanelFlags flags) { m_flags = m_flags | flags; }
    
    /** @brief Check if panel is visible */
    bool isVisible() const { return m_isVisible; }
    
    /** @brief Set panel visibility */
    void setVisible(bool visible);
    
    /** @brief Toggle visibility */
    void toggleVisible() { setVisible(!m_isVisible); }
    
    /** @brief Check if panel has focus */
    bool isFocused() const { return m_isFocused; }
    
    /** @brief Request focus for this panel */
    void requestFocus();
    
    /** @brief Check if panel is hovered */
    bool isHovered() const { return m_isHovered; }
    
    /** @brief Get panel position */
    const Vec2& getPosition() const { return m_position; }
    
    /** @brief Set panel position */
    void setPosition(const Vec2& pos) { m_position = pos; }
    
    /** @brief Get panel size */
    const Vec2& getSize() const { return m_size; }
    
    /** @brief Set panel size */
    void setSize(const Vec2& size) { m_size = size; }
    
    /** @brief Get minimum size */
    const Vec2& getMinSize() const { return m_minSize; }
    
    /** @brief Set minimum size */
    void setMinSize(const Vec2& size) { m_minSize = size; }
    
    /** @brief Get maximum size */
    const Vec2& getMaxSize() const { return m_maxSize; }
    
    /** @brief Set maximum size */
    void setMaxSize(const Vec2& size) { m_maxSize = size; }
    
    /** @brief Get dock position */
    DockPosition getDockPosition() const { return m_dockPosition; }
    
    /** @brief Set dock position */
    void setDockPosition(DockPosition pos) { m_dockPosition = pos; }
    
    /** @brief Check if panel is docked */
    bool isDocked() const { return m_dockPosition != DockPosition::None; }
    
protected:
    // Editor context access
    EditorContext& getContext();
    const EditorContext& getContext() const;
    
    // Helper to mark panel as needing redraw
    void markDirty() { m_isDirty = true; }
    
private:
    friend class EditorWindow;
    friend class PanelManager;
    
    std::string m_title;
    std::string m_id;
    PanelFlags m_flags = PanelFlags::None;
    
    bool m_isVisible = true;
    bool m_isFocused = false;
    bool m_isHovered = false;
    bool m_isDirty = true;
    
    Vec2 m_position{0, 0};
    Vec2 m_size{400, 300};
    Vec2 m_minSize{100, 50};
    Vec2 m_maxSize{10000, 10000};
    
    DockPosition m_dockPosition = DockPosition::None;
};

// ============================================================================
// Panel Manager
// ============================================================================

/**
 * @brief Manages all editor panels
 * 
 * Handles panel lifecycle, docking, and rendering order.
 */
class PanelManager {
public:
    /**
     * @brief Initialize the panel manager
     */
    void initialize();
    
    /**
     * @brief Shutdown and clean up all panels
     */
    void shutdown();
    
    /**
     * @brief Update all panels
     * @param deltaTime Time since last frame
     */
    void update(f32 deltaTime);
    
    /**
     * @brief Draw all visible panels
     */
    void draw();
    
    /**
     * @brief Register a panel
     * @param panel Panel to register
     */
    void registerPanel(std::unique_ptr<EditorPanel> panel);
    
    /**
     * @brief Unregister a panel by ID
     * @param id Panel ID
     */
    void unregisterPanel(const std::string& id);
    
    /**
     * @brief Get panel by ID
     * @param id Panel ID
     * @return Panel pointer or nullptr
     */
    EditorPanel* getPanel(const std::string& id);
    
    /**
     * @brief Get panel by type
     * @tparam T Panel type
     * @return Panel pointer or nullptr
     */
    template<typename T>
    T* getPanel();
    
    /**
     * @brief Get all registered panels
     */
    const std::vector<std::unique_ptr<EditorPanel>>& getPanels() const { return m_panels; }
    
    /**
     * @brief Get focused panel
     */
    EditorPanel* getFocusedPanel() { return m_focusedPanel; }
    
    /**
     * @brief Set focused panel
     */
    void setFocusedPanel(EditorPanel* panel);
    
    /**
     * @brief Save panel layout
     * @param name Layout name
     */
    void saveLayout(const std::string& name);
    
    /**
     * @brief Load panel layout
     * @param name Layout name
     */
    void loadLayout(const std::string& name);
    
    /**
     * @brief Reset to default layout
     */
    void resetLayout();
    
    /**
     * @brief Get available layout names
     */
    std::vector<std::string> getLayoutNames() const;
    
private:
    std::vector<std::unique_ptr<EditorPanel>> m_panels;
    EditorPanel* m_focusedPanel = nullptr;
    std::string m_currentLayout = "Default";
};

// ============================================================================
// Common Panel Types
// ============================================================================

/**
 * @brief Scene hierarchy panel showing entity tree
 */
class HierarchyPanel : public EditorPanel {
public:
    HierarchyPanel();
    
    void onDraw() override;
    void onUpdate(f32 deltaTime) override;
    bool onKeyPressed(int key, int mods) override;
    bool onDrop(const std::vector<std::string>& paths) override;
    
    // Hierarchy-specific
    void expandAll();
    void collapseAll();
    void scrollToEntity(ecs::Entity entity);
    
private:
    void drawEntityNode(ecs::Entity entity, int depth);
    void handleContextMenu(ecs::Entity entity);
    
    std::string m_searchFilter;
    std::unordered_set<u64> m_expandedEntities;
    bool m_showHidden = false;
};

/**
 * @brief Inspector panel showing selected entity properties
 */
class InspectorPanel : public EditorPanel {
public:
    InspectorPanel();
    
    void onDraw() override;
    void onUpdate(f32 deltaTime) override;
    
    // Inspector-specific
    void lockToEntity(ecs::Entity entity);
    void unlock();
    bool isLocked() const { return m_isLocked; }
    
private:
    void drawEntityHeader();
    void drawTransformComponent();
    void drawComponent(const std::string& name, void* component);
    void drawAddComponentButton();
    
    ecs::Entity m_lockedEntity;
    bool m_isLocked = false;
    bool m_debugMode = false;
};

/**
 * @brief Console panel for log messages
 */
class ConsolePanelView : public EditorPanel {
public:
    ConsolePanelView();
    
    void onDraw() override;
    
    // Console-specific
    void clear();
    void scrollToBottom();
    void setFilter(ConsoleMessage::Type type, bool show);
    
private:
    bool m_showInfo = true;
    bool m_showWarnings = true;
    bool m_showErrors = true;
    bool m_collapse = true;
    bool m_autoScroll = true;
    std::string m_searchFilter;
};

/**
 * @brief Asset browser panel
 */
class AssetBrowserPanel : public EditorPanel {
public:
    AssetBrowserPanel();
    
    void onDraw() override;
    void onUpdate(f32 deltaTime) override;
    bool onDrop(const std::vector<std::string>& paths) override;
    
    // Browser-specific
    void navigateTo(const std::string& path);
    void refresh();
    void setViewMode(bool gridView);
    
private:
    void drawToolbar();
    void drawFolderTree();
    void drawAssetGrid();
    void drawAssetList();
    void handleAssetContextMenu(const AssetInfo& asset);
    
    std::string m_currentPath;
    std::vector<AssetInfo> m_assets;
    std::string m_searchFilter;
    bool m_gridView = true;
    f32 m_thumbnailSize = 64.0f;
    std::string m_selectedAsset;
};

/**
 * @brief Scene viewport panel
 */
class SceneViewPanel : public EditorPanel {
public:
    SceneViewPanel();
    
    void onDraw() override;
    void onUpdate(f32 deltaTime) override;
    bool onMousePressed(int button, const Vec2& position) override;
    bool onMouseReleased(int button, const Vec2& position) override;
    bool onMouseMoved(const Vec2& position, const Vec2& delta) override;
    bool onScroll(const Vec2& delta) override;
    bool onKeyPressed(int key, int mods) override;
    bool onTouchGesture(TouchGesture gesture, const TouchState& state) override;
    
private:
    void drawToolbar();
    void drawGizmos();
    void drawGrid();
    void drawSelectionOutline();
    void handleNavigation(f32 deltaTime);
    void handlePicking(const Vec2& screenPos);
    
    // Camera navigation state
    bool m_isNavigating = false;
    bool m_isOrbiting = false;
    bool m_isPanning = false;
    Vec2 m_lastMousePos;
    
    // Gizmo interaction
    bool m_isDraggingGizmo = false;
    int m_activeGizmoAxis = -1;
    Vec3 m_gizmoStartPos;
};

/**
 * @brief Game preview panel
 */
class GameViewPanel : public EditorPanel {
public:
    GameViewPanel();
    
    void onDraw() override;
    void onUpdate(f32 deltaTime) override;
    
    // Game view specific
    void setResolution(u32 width, u32 height);
    void setAspectRatio(f32 aspect);
    void setMaximizeOnPlay(bool maximize);
    
private:
    void drawToolbar();
    void drawStatsOverlay();
    
    u32 m_targetWidth = 1920;
    u32 m_targetHeight = 1080;
    f32 m_aspectRatio = 16.0f / 9.0f;
    bool m_freeAspect = false;
    bool m_maximizeOnPlay = true;
    bool m_showStats = false;
};

// ============================================================================
// Template Implementations
// ============================================================================

template<typename T>
T* PanelManager::getPanel() {
    for (auto& panel : m_panels) {
        if (auto* typed = dynamic_cast<T*>(panel.get())) {
            return typed;
        }
    }
    return nullptr;
}

} // namespace nova::editor
