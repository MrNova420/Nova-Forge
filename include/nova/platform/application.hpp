/**
 * @file application.hpp
 * @brief NovaCore Platform™ - Application Framework
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 * 
 * Cross-platform application framework providing:
 * - Application lifecycle management
 * - Window management
 * - System information
 * - Platform-agnostic entry point
 */

#pragma once

#include "platform_types.hpp"
#include "window.hpp"

#include <memory>

namespace nova::platform {

// ============================================================================
// Application Class
// ============================================================================

/**
 * @brief Cross-platform application base class
 * 
 * Override this class to create your game/application:
 * 
 * @code
 *     class MyGame : public Application {
 *     public:
 *         bool onInit() override {
 *             // Initialize game
 *             return true;
 *         }
 *         
 *         void onUpdate(f32 deltaTime) override {
 *             // Update game logic
 *         }
 *         
 *         void onRender() override {
 *             // Render game
 *         }
 *         
 *         void onShutdown() override {
 *             // Cleanup
 *         }
 *     };
 *     
 *     NOVA_APP(MyGame)
 * @endcode
 */
class Application {
public:
    /**
     * @brief Get the singleton application instance
     */
    static Application& get();
    
    /**
     * @brief Constructor
     */
    Application();
    
    /**
     * @brief Virtual destructor
     */
    virtual ~Application();
    
    /**
     * @brief Initialize the application
     * @param desc Application description
     * @return true if initialization succeeded
     */
    bool initialize(const AppDesc& desc);
    
    /**
     * @brief Run the main loop
     * @return Exit code
     */
    int run();
    
    /**
     * @brief Shutdown the application
     */
    void shutdown();
    
    /**
     * @brief Request application exit
     * @param exitCode Exit code to return
     */
    void quit(int exitCode = 0);
    
    /**
     * @brief Check if running
     */
    bool isRunning() const { return m_state == AppState::Running; }
    
    /**
     * @brief Get application state
     */
    AppState getState() const { return m_state; }
    
    /**
     * @brief Get application description
     */
    const AppDesc& getDesc() const { return m_desc; }
    
    /**
     * @brief Get main window
     */
    Window* getMainWindow() const { return m_mainWindow.get(); }
    
    /**
     * @brief Get frame information
     */
    const FrameInfo& getFrameInfo() const { return m_frameInfo; }
    
    /**
     * @brief Get system information
     */
    const SystemInfo& getSystemInfo() const { return m_systemInfo; }
    
    // ========================================================================
    // Time Control
    // ========================================================================
    
    /**
     * @brief Set time scale (0 = paused, 1 = normal)
     */
    void setTimeScale(f32 scale);
    
    /**
     * @brief Get time scale
     */
    f32 getTimeScale() const { return m_frameInfo.timeScale; }
    
    /**
     * @brief Set target FPS
     */
    void setTargetFPS(u32 fps);
    
    /**
     * @brief Get target FPS
     */
    u32 getTargetFPS() const { return m_targetFPS; }
    
    /**
     * @brief Enable/disable VSync
     */
    void setVSync(bool enabled);
    
    /**
     * @brief Check if VSync is enabled
     */
    bool isVSyncEnabled() const { return m_vsync; }
    
    // ========================================================================
    // Window Management
    // ========================================================================
    
    /**
     * @brief Create a new window
     */
    std::unique_ptr<Window> createWindow(const WindowDesc& desc);
    
    /**
     * @brief Get monitors
     */
    const std::vector<MonitorInfo>& getMonitors() const { return m_systemInfo.monitors; }
    
    /**
     * @brief Get primary monitor
     */
    const MonitorInfo* getPrimaryMonitor() const;
    
    // ========================================================================
    // Paths
    // ========================================================================
    
    /**
     * @brief Get special folder path
     */
    std::string getSpecialFolderPath(SpecialFolder folder) const;
    
    /**
     * @brief Get application data path
     */
    std::string getAppDataPath() const;
    
    /**
     * @brief Get save game path
     */
    std::string getSaveGamePath() const;
    
    // ========================================================================
    // System Functions
    // ========================================================================
    
    /**
     * @brief Open URL in browser
     */
    void openURL(const std::string& url);
    
    /**
     * @brief Show native message box
     */
    enum class MessageBoxType { Info, Warning, Error, Question };
    enum class MessageBoxButtons { Ok, OkCancel, YesNo, YesNoCancel };
    int showMessageBox(const std::string& title,
                      const std::string& message,
                      MessageBoxType type = MessageBoxType::Info,
                      MessageBoxButtons buttons = MessageBoxButtons::Ok);
    
    /**
     * @brief Copy text to clipboard
     */
    void setClipboardText(const std::string& text);
    
    /**
     * @brief Get text from clipboard
     */
    std::string getClipboardText() const;
    
    // ========================================================================
    // Event Callbacks
    // ========================================================================
    
    /**
     * @brief Set app state change callback
     */
    void setAppStateCallback(AppStateCallback callback);
    
    /**
     * @brief Set low memory warning callback
     */
    void setLowMemoryCallback(LowMemoryCallback callback);
    
protected:
    // ========================================================================
    // Override Points
    // ========================================================================
    
    /**
     * @brief Called during initialization
     * @return true if initialization succeeded
     */
    virtual bool onInit() { return true; }
    
    /**
     * @brief Called each frame for game logic
     * @param deltaTime Time since last frame in seconds
     */
    virtual void onUpdate(f32 deltaTime) {}
    
    /**
     * @brief Called for fixed timestep updates (physics)
     * @param fixedDeltaTime Fixed timestep (typically 1/60)
     */
    virtual void onFixedUpdate(f32 fixedDeltaTime) {}
    
    /**
     * @brief Called each frame for rendering
     */
    virtual void onRender() {}
    
    /**
     * @brief Called after render for ImGui/debug UI
     */
    virtual void onGUI() {}
    
    /**
     * @brief Called during shutdown
     */
    virtual void onShutdown() {}
    
    /**
     * @brief Called when app state changes
     */
    virtual void onStateChange(AppState newState) {}
    
    /**
     * @brief Called on low memory warning (mobile)
     */
    virtual void onLowMemory() {}
    
    /**
     * @brief Called on window resize
     */
    virtual void onResize(u32 width, u32 height) {}
    
    /**
     * @brief Called on window focus change
     */
    virtual void onFocusChange(bool focused) {}
    
private:
    // Internal methods
    void initSystems();
    void shutdownSystems();
    void processEvents();
    void updateFrame();
    void renderFrame();
    void updateFrameInfo();
    void gatherSystemInfo();
    
    // Singleton
    static Application* s_instance;
    
    // State
    AppDesc m_desc;
    AppState m_state = AppState::Stopped;
    int m_exitCode = 0;
    bool m_shouldQuit = false;
    
    // Window
    std::unique_ptr<Window> m_mainWindow;
    
    // Timing
    FrameInfo m_frameInfo;
    u32 m_targetFPS = 60;
    bool m_vsync = true;
    f64 m_targetFrameTime = 1.0 / 60.0;
    f64 m_lastFrameTime = 0.0;
    f64 m_fixedTimestep = 1.0 / 60.0;
    f64 m_fixedTimeAccumulator = 0.0;
    
    // System info
    SystemInfo m_systemInfo;
    
    // Callbacks
    AppStateCallback m_appStateCallback;
    LowMemoryCallback m_lowMemoryCallback;
};

// ============================================================================
// Entry Point Macro
// ============================================================================

/**
 * @brief Main entry point macro
 * 
 * Use this macro to define the application entry point:
 * @code
 *     NOVA_APP(MyGameClass)
 * @endcode
 */
#define NOVA_APP(AppClass) \
    std::unique_ptr<nova::platform::Application> createApplication() { \
        return std::make_unique<AppClass>(); \
    } \
    int main(int argc, char** argv) { \
        auto app = createApplication(); \
        nova::platform::AppDesc desc; \
        if (app->initialize(desc)) { \
            return app->run(); \
        } \
        return 1; \
    }

/**
 * @brief Extended entry point with custom desc
 */
#define NOVA_APP_DESC(AppClass, DescSetup) \
    std::unique_ptr<nova::platform::Application> createApplication() { \
        return std::make_unique<AppClass>(); \
    } \
    int main(int argc, char** argv) { \
        auto app = createApplication(); \
        nova::platform::AppDesc desc; \
        DescSetup; \
        if (app->initialize(desc)) { \
            return app->run(); \
        } \
        return 1; \
    }

} // namespace nova::platform
