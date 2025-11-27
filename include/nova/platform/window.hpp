/**
 * @file window.hpp
 * @brief NovaCore Platform™ - Window Management
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 * 
 * Cross-platform window management:
 * - Window creation and destruction
 * - Window state management
 * - Fullscreen modes
 * - Multi-monitor support
 */

#pragma once

#include "platform_types.hpp"

namespace nova::platform {

// ============================================================================
// Window Class
// ============================================================================

/**
 * @brief Cross-platform window class
 */
class Window {
public:
    /**
     * @brief Constructor
     */
    Window();
    
    /**
     * @brief Destructor
     */
    ~Window();
    
    /**
     * @brief Create the window
     * @param desc Window description
     * @return true if creation succeeded
     */
    bool create(const WindowDesc& desc);
    
    /**
     * @brief Destroy the window
     */
    void destroy();
    
    /**
     * @brief Check if window is valid
     */
    bool isValid() const { return m_handle != nullptr; }
    
    /**
     * @brief Process window events
     */
    void processEvents();
    
    /**
     * @brief Present the frame (swap buffers)
     */
    void present();
    
    // ========================================================================
    // Window State
    // ========================================================================
    
    /**
     * @brief Get window handle
     */
    WindowHandle getHandle() const { return m_handle; }
    
    /**
     * @brief Get native window handle (platform-specific)
     */
    void* getNativeHandle() const;
    
    /**
     * @brief Get window title
     */
    const std::string& getTitle() const { return m_title; }
    
    /**
     * @brief Set window title
     */
    void setTitle(const std::string& title);
    
    /**
     * @brief Get window position
     */
    Vec2i getPosition() const { return m_position; }
    
    /**
     * @brief Set window position
     */
    void setPosition(i32 x, i32 y);
    void setPosition(const Vec2i& pos) { setPosition(pos.x, pos.y); }
    
    /**
     * @brief Get window size
     */
    Vec2i getSize() const { return m_size; }
    
    /**
     * @brief Set window size
     */
    void setSize(u32 width, u32 height);
    void setSize(const Vec2i& size) { setSize(size.x, size.y); }
    
    /**
     * @brief Get client area size (excluding borders)
     */
    Vec2i getClientSize() const { return m_clientSize; }
    
    /**
     * @brief Get framebuffer size (for high DPI)
     */
    Vec2i getFramebufferSize() const { return m_framebufferSize; }
    
    /**
     * @brief Get content scale (for high DPI)
     */
    Vec2 getContentScale() const { return m_contentScale; }
    
    // ========================================================================
    // Window State Control
    // ========================================================================
    
    /**
     * @brief Get window state
     */
    WindowState getState() const { return m_state; }
    
    /**
     * @brief Minimize the window
     */
    void minimize();
    
    /**
     * @brief Maximize the window
     */
    void maximize();
    
    /**
     * @brief Restore the window
     */
    void restore();
    
    /**
     * @brief Show the window
     */
    void show();
    
    /**
     * @brief Hide the window
     */
    void hide();
    
    /**
     * @brief Check if window is visible
     */
    bool isVisible() const { return m_visible; }
    
    /**
     * @brief Focus the window
     */
    void focus();
    
    /**
     * @brief Check if window has focus
     */
    bool hasFocus() const { return m_focused; }
    
    /**
     * @brief Check if window should close
     */
    bool shouldClose() const { return m_shouldClose; }
    
    /**
     * @brief Set should close flag
     */
    void setShouldClose(bool close) { m_shouldClose = close; }
    
    // ========================================================================
    // Fullscreen
    // ========================================================================
    
    /**
     * @brief Get fullscreen mode
     */
    FullscreenMode getFullscreenMode() const { return m_fullscreenMode; }
    
    /**
     * @brief Set fullscreen mode
     */
    void setFullscreen(FullscreenMode mode, u32 monitorIndex = 0);
    
    /**
     * @brief Toggle fullscreen
     */
    void toggleFullscreen();
    
    /**
     * @brief Check if in any fullscreen mode
     */
    bool isFullscreen() const { return m_fullscreenMode != FullscreenMode::Windowed; }
    
    // ========================================================================
    // Display
    // ========================================================================
    
    /**
     * @brief Get current monitor index
     */
    u32 getMonitorIndex() const { return m_monitorIndex; }
    
    /**
     * @brief Get display mode
     */
    DisplayMode getDisplayMode() const { return m_displayMode; }
    
    /**
     * @brief Set display mode (for exclusive fullscreen)
     */
    void setDisplayMode(const DisplayMode& mode);
    
    // ========================================================================
    // Misc
    // ========================================================================
    
    /**
     * @brief Set window icon
     */
    void setIcon(const u8* pixels, u32 width, u32 height);
    
    /**
     * @brief Flash the window (get user attention)
     */
    void flash();
    
    /**
     * @brief Request attention (taskbar)
     */
    void requestAttention();
    
    /**
     * @brief Set cursor visible
     */
    void setCursorVisible(bool visible);
    
    /**
     * @brief Check if cursor is visible
     */
    bool isCursorVisible() const { return m_cursorVisible; }
    
    /**
     * @brief Confine cursor to window
     */
    void setConfineCursor(bool confine);
    
    /**
     * @brief Check if cursor is confined
     */
    bool isCursorConfined() const { return m_cursorConfined; }
    
    // ========================================================================
    // Callbacks
    // ========================================================================
    
    void setResizedCallback(WindowResizedCallback callback) { m_resizedCallback = std::move(callback); }
    void setMovedCallback(WindowMovedCallback callback) { m_movedCallback = std::move(callback); }
    void setFocusCallback(WindowFocusCallback callback) { m_focusCallback = std::move(callback); }
    void setCloseCallback(WindowCloseCallback callback) { m_closeCallback = std::move(callback); }
    void setStateCallback(WindowStateCallback callback) { m_stateCallback = std::move(callback); }
    
private:
    // Platform-specific
    void createPlatformWindow(const WindowDesc& desc);
    void destroyPlatformWindow();
    void updateWindowMetrics();
    
    // Handle
    WindowHandle m_handle = nullptr;
    
    // State
    std::string m_title;
    Vec2i m_position;
    Vec2i m_size;
    Vec2i m_clientSize;
    Vec2i m_framebufferSize;
    Vec2 m_contentScale = Vec2(1.0f, 1.0f);
    WindowState m_state = WindowState::Normal;
    FullscreenMode m_fullscreenMode = FullscreenMode::Windowed;
    DisplayMode m_displayMode;
    u32 m_monitorIndex = 0;
    
    bool m_visible = true;
    bool m_focused = false;
    bool m_shouldClose = false;
    bool m_cursorVisible = true;
    bool m_cursorConfined = false;
    
    // Stored windowed state for fullscreen toggle
    Vec2i m_windowedPosition;
    Vec2i m_windowedSize;
    
    // Callbacks
    WindowResizedCallback m_resizedCallback;
    WindowMovedCallback m_movedCallback;
    WindowFocusCallback m_focusCallback;
    WindowCloseCallback m_closeCallback;
    WindowStateCallback m_stateCallback;
};

} // namespace nova::platform
