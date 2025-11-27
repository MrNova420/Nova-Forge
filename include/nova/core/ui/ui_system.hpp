/**
 * @file ui_system.hpp
 * @brief Nova UI™ - UI System manager
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "widget.hpp"
#include "ui_types.hpp"

#include <memory>
#include <unordered_map>

namespace nova::ui {

/**
 * @brief UI System manager
 * 
 * Manages the UI widget tree, handles input routing, layout,
 * rendering, and theming.
 */
class UISystem {
public:
    /// Get singleton instance
    [[nodiscard]] static UISystem& instance();
    
    /**
     * @brief Initialize the UI system
     * @param screenWidth Initial screen width
     * @param screenHeight Initial screen height
     * @param pixelRatio Device pixel ratio
     * @return Result indicating success or failure
     */
    [[nodiscard]] Result<void> initialize(f32 screenWidth, f32 screenHeight, f32 pixelRatio = 1.0f);
    
    /**
     * @brief Shutdown the UI system
     */
    void shutdown();
    
    /**
     * @brief Check if initialized
     */
    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }
    
    /**
     * @brief Update the UI system
     * @param deltaTime Time since last update in seconds
     */
    void update(f32 deltaTime);
    
    /**
     * @brief Perform layout pass
     */
    void performLayout();
    
    // =========================================================================
    // Screen
    // =========================================================================
    
    /// Get screen size
    [[nodiscard]] Vec2 getScreenSize() const noexcept { return m_screenSize; }
    
    /// Set screen size
    void setScreenSize(f32 width, f32 height);
    
    /// Get pixel ratio
    [[nodiscard]] f32 getPixelRatio() const noexcept { return m_pixelRatio; }
    
    /// Set pixel ratio
    void setPixelRatio(f32 ratio);
    
    // =========================================================================
    // Root Widget
    // =========================================================================
    
    /// Get root widget
    [[nodiscard]] Widget* getRoot() noexcept { return m_root.get(); }
    [[nodiscard]] const Widget* getRoot() const noexcept { return m_root.get(); }
    
    /// Set root widget
    void setRoot(std::unique_ptr<Widget> root);
    
    // =========================================================================
    // Focus Management
    // =========================================================================
    
    /// Get focused widget
    [[nodiscard]] Widget* getFocusedWidget() noexcept { return m_focusedWidget; }
    
    /// Set focused widget
    void setFocusedWidget(Widget* widget);
    
    /// Clear focus
    void clearFocus();
    
    /// Move focus to next focusable widget
    void focusNext();
    
    /// Move focus to previous focusable widget
    void focusPrevious();
    
    // =========================================================================
    // Input Handling
    // =========================================================================
    
    /// Handle pointer down event
    bool handlePointerDown(const PointerEvent& event);
    
    /// Handle pointer up event
    bool handlePointerUp(const PointerEvent& event);
    
    /// Handle pointer move event
    bool handlePointerMove(const PointerEvent& event);
    
    /// Handle key down event
    bool handleKeyDown(const KeyEvent& event);
    
    /// Handle key up event
    bool handleKeyUp(const KeyEvent& event);
    
    /// Handle text input event
    bool handleTextInput(const TextInputEvent& event);
    
    /// Handle scroll event
    bool handleScroll(const ScrollEvent& event);
    
    // =========================================================================
    // Theme
    // =========================================================================
    
    /// Get current theme
    [[nodiscard]] const Theme& getTheme() const noexcept { return m_theme; }
    
    /// Set theme
    void setTheme(const Theme& theme);
    
    // =========================================================================
    // Widget Lookup
    // =========================================================================
    
    /// Find widget by ID
    [[nodiscard]] Widget* findById(const std::string& id);
    
    /// Find widget by handle
    [[nodiscard]] Widget* findByHandle(WidgetHandle handle);
    
    /// Hit test at screen position
    [[nodiscard]] Widget* hitTest(Vec2 screenPosition);
    
    // =========================================================================
    // Rendering (called by renderer)
    // =========================================================================
    
    /// Get widgets to render (in render order)
    [[nodiscard]] std::vector<Widget*> getWidgetsToRender() const;
    
    // =========================================================================
    // Statistics
    // =========================================================================
    
    /// Get total widget count
    [[nodiscard]] u32 getWidgetCount() const noexcept { return m_widgetCount; }
    
    /// Get visible widget count
    [[nodiscard]] u32 getVisibleWidgetCount() const noexcept { return m_visibleWidgetCount; }
    
    /// Get last layout time in milliseconds
    [[nodiscard]] f32 getLastLayoutTimeMs() const noexcept { return m_lastLayoutTimeMs; }
    
private:
    UISystem() = default;
    ~UISystem();
    
    UISystem(const UISystem&) = delete;
    UISystem& operator=(const UISystem&) = delete;
    
    bool m_initialized = false;
    
    Vec2 m_screenSize;
    f32 m_pixelRatio = 1.0f;
    
    std::unique_ptr<Widget> m_root;
    Widget* m_focusedWidget = nullptr;
    Widget* m_hoveredWidget = nullptr;
    Widget* m_pressedWidget = nullptr;
    
    Theme m_theme;
    
    // Widget registry for fast lookup
    std::unordered_map<u64, Widget*> m_widgetRegistry;
    
    // Statistics
    u32 m_widgetCount = 0;
    u32 m_visibleWidgetCount = 0;
    f32 m_lastLayoutTimeMs = 0.0f;
    
    void registerWidget(Widget* widget);
    void unregisterWidget(Widget* widget);
    void collectWidgets(Widget* widget, std::vector<Widget*>& output) const;
    void updateHoveredWidget(Vec2 position);
};

// ============================================================================
// Convenience Functions
// ============================================================================

/// Initialize UI system
inline Result<void> initializeUI(f32 screenWidth, f32 screenHeight, f32 pixelRatio = 1.0f) {
    return UISystem::instance().initialize(screenWidth, screenHeight, pixelRatio);
}

/// Shutdown UI system
inline void shutdownUI() {
    UISystem::instance().shutdown();
}

/// Get UI root widget
inline Widget* getUIRoot() {
    return UISystem::instance().getRoot();
}

/// Set UI root widget
inline void setUIRoot(std::unique_ptr<Widget> root) {
    UISystem::instance().setRoot(std::move(root));
}

/// Get current theme
inline const Theme& getTheme() {
    return UISystem::instance().getTheme();
}

/// Set theme
inline void setTheme(const Theme& theme) {
    UISystem::instance().setTheme(theme);
}

} // namespace nova::ui
