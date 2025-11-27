// =============================================================================
// NovaForge Platform - Engine API
// =============================================================================
// Platform: NovaForge | Engine: NovaCore | Company: WeNova Interactive
// (operating as Kayden Shawn Massengill)
//
// API for accessing NovaCore engine functionality including rendering,
// physics, audio, input, and more.
// =============================================================================

#pragma once

#include "nova/api/api_types.hpp"
#include <nova/core/math/math.hpp>

namespace nova::api {

// =============================================================================
// Forward Declarations
// =============================================================================

class RenderApi;
class PhysicsApi;
class AudioApi;
class InputApi;
class SceneApi;
class AssetApi;

// =============================================================================
// Window Types
// =============================================================================

/**
 * @brief Window creation parameters
 */
struct WindowDesc {
    std::string title = "NovaForge Window";
    u32 width = 1280;
    u32 height = 720;
    bool isFullscreen = false;
    bool resizable = true;
    bool vsync = true;
    
    /// Create default window
    [[nodiscard]] static WindowDesc defaults() {
        return WindowDesc{};
    }
    
    /// Create fullscreen window
    [[nodiscard]] static WindowDesc fullscreen(std::string_view windowTitle) {
        WindowDesc desc;
        desc.title = std::string(windowTitle);
        desc.isFullscreen = true;
        return desc;
    }
    
    /// Create window with specific size
    [[nodiscard]] static WindowDesc withSize(std::string_view windowTitle, u32 w, u32 h) {
        WindowDesc desc;
        desc.title = std::string(windowTitle);
        desc.width = w;
        desc.height = h;
        return desc;
    }
};

/**
 * @brief Window handle
 */
struct WindowHandle {
    u64 id;
    
    [[nodiscard]] bool isValid() const noexcept { return id != 0; }
    [[nodiscard]] static WindowHandle invalid() { return WindowHandle{0}; }
};

// =============================================================================
// Engine API
// =============================================================================

/**
 * @brief Main engine API for accessing NovaCore functionality
 * 
 * Provides access to:
 * - Window management
 * - Rendering system
 * - Physics system
 * - Audio system
 * - Input system
 * - Scene management
 * - Asset management
 */
class EngineApi {
public:
    EngineApi();
    ~EngineApi();
    
    // Non-copyable, non-movable
    EngineApi(const EngineApi&) = delete;
    EngineApi& operator=(const EngineApi&) = delete;
    EngineApi(EngineApi&&) = delete;
    EngineApi& operator=(EngineApi&&) = delete;
    
    // =========================================================================
    // Lifecycle
    // =========================================================================
    
    /**
     * @brief Initialize the engine
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult initialize();
    
    /**
     * @brief Shutdown the engine
     */
    void shutdown();
    
    /**
     * @brief Check if engine is running
     */
    [[nodiscard]] bool isRunning() const noexcept;
    
    /**
     * @brief Process a single frame
     * @return true to continue, false to quit
     */
    [[nodiscard]] bool processFrame();
    
    /**
     * @brief Run the main loop
     * 
     * This blocks until the engine is stopped.
     */
    void run();
    
    /**
     * @brief Request engine stop
     */
    void requestStop();
    
    // =========================================================================
    // Window Management
    // =========================================================================
    
    /**
     * @brief Create a window
     * @param desc Window description
     * @return Window handle or error
     */
    [[nodiscard]] ApiResultOf<WindowHandle> createWindow(const WindowDesc& desc);
    
    /**
     * @brief Create a window with title and size
     * @param title Window title
     * @param width Window width
     * @param height Window height
     * @return Window handle or error
     */
    [[nodiscard]] ApiResultOf<WindowHandle> createWindow(
        std::string_view title, 
        u32 width, 
        u32 height);
    
    /**
     * @brief Destroy a window
     * @param handle Window handle
     */
    void destroyWindow(WindowHandle handle);
    
    /**
     * @brief Get the main window
     * @return Main window handle or invalid if none
     */
    [[nodiscard]] WindowHandle getMainWindow() const noexcept;
    
    /**
     * @brief Set window title
     * @param handle Window handle
     * @param title New title
     */
    void setWindowTitle(WindowHandle handle, std::string_view title);
    
    /**
     * @brief Set window size
     * @param handle Window handle
     * @param width New width
     * @param height New height
     */
    void setWindowSize(WindowHandle handle, u32 width, u32 height);
    
    /**
     * @brief Set window fullscreen mode
     * @param handle Window handle
     * @param fullscreen true for fullscreen, false for windowed
     */
    void setWindowFullscreen(WindowHandle handle, bool fullscreen);
    
    // =========================================================================
    // Time and Frame Info
    // =========================================================================
    
    /**
     * @brief Get delta time since last frame
     * @return Delta time in seconds
     */
    [[nodiscard]] f32 getDeltaTime() const noexcept;
    
    /**
     * @brief Get total elapsed time
     * @return Elapsed time in seconds
     */
    [[nodiscard]] f64 getElapsedTime() const noexcept;
    
    /**
     * @brief Get current frame number
     * @return Frame number (starts at 0)
     */
    [[nodiscard]] u64 getFrameNumber() const noexcept;
    
    /**
     * @brief Get current frames per second
     * @return FPS
     */
    [[nodiscard]] f32 getFps() const noexcept;
    
    /**
     * @brief Set target FPS
     * @param targetFps Target frames per second (0 = unlimited)
     */
    void setTargetFps(u32 targetFps);
    
    /**
     * @brief Set time scale
     * @param scale Time scale (1.0 = normal, 0.5 = half speed, 2.0 = double speed)
     */
    void setTimeScale(f32 scale);
    
    // =========================================================================
    // Subsystem Access
    // =========================================================================
    
    /**
     * @brief Get render API
     * @return Reference to render API
     */
    [[nodiscard]] RenderApi& getRender() noexcept;
    [[nodiscard]] const RenderApi& getRender() const noexcept;
    
    /**
     * @brief Get physics API
     * @return Reference to physics API
     */
    [[nodiscard]] PhysicsApi& getPhysics() noexcept;
    [[nodiscard]] const PhysicsApi& getPhysics() const noexcept;
    
    /**
     * @brief Get audio API
     * @return Reference to audio API
     */
    [[nodiscard]] AudioApi& getAudio() noexcept;
    [[nodiscard]] const AudioApi& getAudio() const noexcept;
    
    /**
     * @brief Get input API
     * @return Reference to input API
     */
    [[nodiscard]] InputApi& getInput() noexcept;
    [[nodiscard]] const InputApi& getInput() const noexcept;
    
    /**
     * @brief Get scene API
     * @return Reference to scene API
     */
    [[nodiscard]] SceneApi& getScene() noexcept;
    [[nodiscard]] const SceneApi& getScene() const noexcept;
    
    /**
     * @brief Get asset API
     * @return Reference to asset API
     */
    [[nodiscard]] AssetApi& getAsset() noexcept;
    [[nodiscard]] const AssetApi& getAsset() const noexcept;
    
    // =========================================================================
    // Callbacks
    // =========================================================================
    
    /**
     * @brief Set update callback
     * @param callback Function called every frame with delta time
     */
    void setUpdateCallback(std::function<void(f32)> callback);
    
    /**
     * @brief Set fixed update callback
     * @param callback Function called at fixed time intervals
     */
    void setFixedUpdateCallback(std::function<void(f32)> callback);
    
    /**
     * @brief Set render callback
     * @param callback Function called during rendering
     */
    void setRenderCallback(std::function<void()> callback);
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

// =============================================================================
// Render API
// =============================================================================

/**
 * @brief Rendering subsystem API
 */
class RenderApi {
public:
    RenderApi();
    ~RenderApi();
    
    /**
     * @brief Set clear color
     * @param r Red (0-1)
     * @param g Green (0-1)
     * @param b Blue (0-1)
     * @param a Alpha (0-1)
     */
    void setClearColor(f32 r, f32 g, f32 b, f32 a = 1.0f);
    
    /**
     * @brief Set clear color from Vec4
     * @param color RGBA color
     */
    void setClearColor(const math::Vec4& color);
    
    /**
     * @brief Enable/disable VSync
     * @param enabled true to enable VSync
     */
    void setVSync(bool enabled);
    
    /**
     * @brief Get render statistics
     */
    struct RenderStats {
        u32 drawCalls;
        u32 triangles;
        u32 vertices;
        u64 gpuMemoryUsed;
        f32 gpuTime;
    };
    [[nodiscard]] RenderStats getStats() const noexcept;
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

// =============================================================================
// Physics API
// =============================================================================

/**
 * @brief Physics subsystem API
 */
class PhysicsApi {
public:
    PhysicsApi();
    ~PhysicsApi();
    
    /**
     * @brief Set gravity
     * @param gravity Gravity vector
     */
    void setGravity(const math::Vec3& gravity);
    
    /**
     * @brief Get gravity
     * @return Current gravity vector
     */
    [[nodiscard]] math::Vec3 getGravity() const noexcept;
    
    /**
     * @brief Raycast into the physics world
     * @param origin Ray origin
     * @param direction Ray direction
     * @param maxDistance Maximum distance
     * @return Hit result or nullopt if no hit
     */
    struct RaycastHit {
        math::Vec3 point;
        math::Vec3 normal;
        f32 distance;
        u64 entityId;
    };
    [[nodiscard]] std::optional<RaycastHit> raycast(
        const math::Vec3& origin,
        const math::Vec3& direction,
        f32 maxDistance = 1000.0f) const;
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

// =============================================================================
// Audio API
// =============================================================================

/**
 * @brief Audio subsystem API
 */
class AudioApi {
public:
    AudioApi();
    ~AudioApi();
    
    /**
     * @brief Play a sound
     * @param path Sound file path
     * @param volume Volume (0-1)
     * @param loop Whether to loop
     * @return Sound handle
     */
    [[nodiscard]] u64 playSound(std::string_view path, f32 volume = 1.0f, bool loop = false);
    
    /**
     * @brief Stop a sound
     * @param handle Sound handle
     */
    void stopSound(u64 handle);
    
    /**
     * @brief Set master volume
     * @param volume Volume (0-1)
     */
    void setMasterVolume(f32 volume);
    
    /**
     * @brief Get master volume
     * @return Current master volume
     */
    [[nodiscard]] f32 getMasterVolume() const noexcept;
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

// =============================================================================
// Input API
// =============================================================================

/**
 * @brief Input subsystem API
 */
class InputApi {
public:
    InputApi();
    ~InputApi();
    
    /**
     * @brief Check if key is currently down
     * @param keyCode Key code
     * @return true if key is down
     */
    [[nodiscard]] bool isKeyDown(u32 keyCode) const noexcept;
    
    /**
     * @brief Check if key was just pressed this frame
     * @param keyCode Key code
     * @return true if key was pressed
     */
    [[nodiscard]] bool isKeyPressed(u32 keyCode) const noexcept;
    
    /**
     * @brief Check if key was just released this frame
     * @param keyCode Key code
     * @return true if key was released
     */
    [[nodiscard]] bool isKeyReleased(u32 keyCode) const noexcept;
    
    /**
     * @brief Get mouse position
     * @return Mouse position in window coordinates
     */
    [[nodiscard]] math::Vec2 getMousePosition() const noexcept;
    
    /**
     * @brief Get mouse delta since last frame
     * @return Mouse movement delta
     */
    [[nodiscard]] math::Vec2 getMouseDelta() const noexcept;
    
    /**
     * @brief Check if mouse button is down
     * @param button Button index (0 = left, 1 = right, 2 = middle)
     * @return true if button is down
     */
    [[nodiscard]] bool isMouseButtonDown(u32 button) const noexcept;
    
    /**
     * @brief Get mouse scroll delta
     * @return Scroll delta (x = horizontal, y = vertical)
     */
    [[nodiscard]] math::Vec2 getMouseScroll() const noexcept;
    
    /**
     * @brief Get touch count
     * @return Number of active touches
     */
    [[nodiscard]] u32 getTouchCount() const noexcept;
    
    /**
     * @brief Get touch position
     * @param index Touch index
     * @return Touch position
     */
    [[nodiscard]] math::Vec2 getTouchPosition(u32 index) const noexcept;
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

// =============================================================================
// Scene API
// =============================================================================

/**
 * @brief Scene management API
 */
class SceneApi {
public:
    SceneApi();
    ~SceneApi();
    
    /**
     * @brief Load a scene
     * @param path Scene file path
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult loadScene(std::string_view path);
    
    /**
     * @brief Unload current scene
     */
    void unloadScene();
    
    /**
     * @brief Create a new empty scene
     * @param name Scene name
     */
    void createScene(std::string_view name);
    
    /**
     * @brief Save current scene
     * @param path Save path
     * @return Result indicating success or error
     */
    [[nodiscard]] ApiResult saveScene(std::string_view path);
    
    /**
     * @brief Get current scene name
     * @return Scene name
     */
    [[nodiscard]] std::string_view getCurrentSceneName() const noexcept;
    
    /**
     * @brief Create an entity
     * @param name Entity name
     * @return Entity ID
     */
    [[nodiscard]] u64 createEntity(std::string_view name = "Entity");
    
    /**
     * @brief Destroy an entity
     * @param entityId Entity ID
     */
    void destroyEntity(u64 entityId);
    
    /**
     * @brief Set entity position
     * @param entityId Entity ID
     * @param position World position
     */
    void setEntityPosition(u64 entityId, const math::Vec3& position);
    
    /**
     * @brief Get entity position
     * @param entityId Entity ID
     * @return Entity position
     */
    [[nodiscard]] math::Vec3 getEntityPosition(u64 entityId) const;
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

// =============================================================================
// Asset API
// =============================================================================

/**
 * @brief Asset management API
 */
class AssetApi {
public:
    AssetApi();
    ~AssetApi();
    
    /**
     * @brief Load an asset
     * @param path Asset path
     * @return Asset ID or error
     */
    [[nodiscard]] ApiResultOf<ResourceId> loadAsset(std::string_view path);
    
    /**
     * @brief Load an asset asynchronously
     * @param path Asset path
     * @param callback Progress callback
     * @return Asset ID or error
     */
    [[nodiscard]] ApiResultOf<ResourceId> loadAssetAsync(
        std::string_view path,
        ApiProgressCallback callback = nullptr);
    
    /**
     * @brief Unload an asset
     * @param id Asset ID
     */
    void unloadAsset(ResourceId id);
    
    /**
     * @brief Check if asset is loaded
     * @param id Asset ID
     * @return Asset state
     */
    [[nodiscard]] ResourceState getAssetState(ResourceId id) const noexcept;
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nova::api
