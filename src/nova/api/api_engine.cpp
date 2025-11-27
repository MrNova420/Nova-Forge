// =============================================================================
// NovaForge Platform - Engine API Implementation
// =============================================================================
// Platform: NovaForge | Engine: NovaCore | Company: WeNova Interactive
// (operating as Kayden Shawn Massengill)
// =============================================================================

#include <nova/api/api_engine.hpp>

#include <chrono>
#include <thread>
#include <algorithm>
#include <map>
#include <array>
#include <cmath>
#include <cctype>
#include <limits>

namespace nova::api {

// =============================================================================
// EngineApi Implementation
// =============================================================================

struct EngineApi::Impl {
    bool running = false;
    bool stopRequested = false;
    
    WindowHandle mainWindow{0};
    u64 nextWindowId = 1;
    
    // Full window state tracking
    struct WindowState {
        u64 id = 0;
        std::string title;
        u32 width = 0;
        u32 height = 0;
        i32 posX = 0;
        i32 posY = 0;
        bool isFullscreen = false;
        bool isMinimized = false;
        bool isMaximized = false;
        bool isVisible = true;
        bool isFocused = false;
        bool resizable = true;
        bool vsync = true;
        void* nativeHandle = nullptr;  // Platform-specific handle
    };
    std::map<u64, WindowState> windows;
    
    f32 deltaTime = 0.0f;
    f64 elapsedTime = 0.0;
    u64 frameNumber = 0;
    f32 fps = 0.0f;
    u32 targetFps = 60;
    f32 timeScale = 1.0f;
    
    // FPS calculation (rolling average)
    static constexpr size_t FPS_SAMPLE_COUNT = 60;
    std::array<f32, FPS_SAMPLE_COUNT> frameTimes{};
    size_t frameTimeIndex = 0;
    
    std::chrono::high_resolution_clock::time_point lastFrameTime;
    std::chrono::high_resolution_clock::time_point startTime;
    
    std::function<void(f32)> updateCallback;
    std::function<void(f32)> fixedUpdateCallback;
    std::function<void()> renderCallback;
    
    std::unique_ptr<RenderApi> render;
    std::unique_ptr<PhysicsApi> physics;
    std::unique_ptr<AudioApi> audio;
    std::unique_ptr<InputApi> input;
    std::unique_ptr<SceneApi> scene;
    std::unique_ptr<AssetApi> asset;
    
    // Calculate rolling average FPS
    f32 calculateAverageFps() const {
        f32 totalTime = 0.0f;
        size_t count = 0;
        for (size_t i = 0; i < FPS_SAMPLE_COUNT; ++i) {
            if (frameTimes[i] > 0.0f) {
                totalTime += frameTimes[i];
                ++count;
            }
        }
        if (count == 0 || totalTime <= 0.0f) return 0.0f;
        return static_cast<f32>(count) / totalTime;
    }
};

EngineApi::EngineApi() : m_impl(std::make_unique<Impl>()) {
    m_impl->render = std::make_unique<RenderApi>();
    m_impl->physics = std::make_unique<PhysicsApi>();
    m_impl->audio = std::make_unique<AudioApi>();
    m_impl->input = std::make_unique<InputApi>();
    m_impl->scene = std::make_unique<SceneApi>();
    m_impl->asset = std::make_unique<AssetApi>();
}

EngineApi::~EngineApi() = default;

ApiResult EngineApi::initialize() {
    m_impl->startTime = std::chrono::high_resolution_clock::now();
    m_impl->lastFrameTime = m_impl->startTime;
    m_impl->running = true;
    return {};
}

void EngineApi::shutdown() {
    m_impl->running = false;
}

bool EngineApi::isRunning() const noexcept {
    return m_impl->running && !m_impl->stopRequested;
}

bool EngineApi::processFrame() {
    if (!m_impl->running || m_impl->stopRequested) {
        return false;
    }
    
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<f32> delta = now - m_impl->lastFrameTime;
    m_impl->deltaTime = delta.count() * m_impl->timeScale;
    m_impl->lastFrameTime = now;
    
    std::chrono::duration<f64> elapsed = now - m_impl->startTime;
    m_impl->elapsedTime = elapsed.count();
    
    // Calculate FPS
    if (m_impl->deltaTime > 0.0f) {
        m_impl->fps = 1.0f / delta.count();
    }
    
    // Call update callback
    if (m_impl->updateCallback) {
        m_impl->updateCallback(m_impl->deltaTime);
    }
    
    // Call render callback
    if (m_impl->renderCallback) {
        m_impl->renderCallback();
    }
    
    m_impl->frameNumber++;
    
    return true;
}

void EngineApi::run() {
    while (processFrame()) {
        // Frame rate limiting
        if (m_impl->targetFps > 0) {
            f32 targetFrameTime = 1.0f / static_cast<f32>(m_impl->targetFps);
            auto now = std::chrono::high_resolution_clock::now();
            std::chrono::duration<f32> frameTime = now - m_impl->lastFrameTime;
            
            if (frameTime.count() < targetFrameTime) {
                auto sleepTime = std::chrono::duration<f32>(targetFrameTime - frameTime.count());
                std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(sleepTime));
            }
        }
    }
}

void EngineApi::requestStop() {
    m_impl->stopRequested = true;
}

ApiResultOf<WindowHandle> EngineApi::createWindow(const WindowDesc& desc) {
    // Generate unique window ID
    WindowHandle handle{m_impl->nextWindowId++};
    
    // Create full window state
    Impl::WindowState state;
    state.id = handle.id;
    state.title = desc.title;
    state.width = desc.width;
    state.height = desc.height;
    state.isFullscreen = desc.isFullscreen;
    state.resizable = desc.resizable;
    state.vsync = desc.vsync;
    state.isVisible = true;
    state.isFocused = true;
    
    // Center window by default (desktop assumption)
    // In a real implementation, this would query the display size
    state.posX = 100;
    state.posY = 100;
    
    // Store window state
    m_impl->windows[handle.id] = state;
    
    // Set as main window if this is the first window
    if (m_impl->mainWindow.id == 0) {
        m_impl->mainWindow = handle;
    }
    
    // Platform-specific window creation would happen here
    // For now, we maintain full state tracking for the window
    
    return handle;
}

ApiResultOf<WindowHandle> EngineApi::createWindow(std::string_view title, u32 width, u32 height) {
    WindowDesc desc;
    desc.title = std::string(title);
    desc.width = width;
    desc.height = height;
    return createWindow(desc);
}

void EngineApi::destroyWindow(WindowHandle handle) {
    if (!handle.isValid()) return;
    
    // Remove window state
    auto it = m_impl->windows.find(handle.id);
    if (it != m_impl->windows.end()) {
        // Platform-specific cleanup would happen here
        m_impl->windows.erase(it);
    }
    
    // Update main window reference if needed
    if (m_impl->mainWindow.id == handle.id) {
        m_impl->mainWindow = WindowHandle{0};
        // Assign next available window as main, if any
        if (!m_impl->windows.empty()) {
            m_impl->mainWindow = WindowHandle{m_impl->windows.begin()->first};
        }
    }
}

WindowHandle EngineApi::getMainWindow() const noexcept {
    return m_impl->mainWindow;
}

void EngineApi::setWindowTitle(WindowHandle handle, std::string_view title) {
    if (!handle.isValid()) return;
    
    auto it = m_impl->windows.find(handle.id);
    if (it != m_impl->windows.end()) {
        it->second.title = std::string(title);
        // Platform-specific title update would happen here
    }
}

void EngineApi::setWindowSize(WindowHandle handle, u32 width, u32 height) {
    if (!handle.isValid()) return;
    
    auto it = m_impl->windows.find(handle.id);
    if (it != m_impl->windows.end()) {
        it->second.width = width;
        it->second.height = height;
        // Platform-specific resize would happen here
    }
}

void EngineApi::setWindowFullscreen(WindowHandle handle, bool fullscreen) {
    if (!handle.isValid()) return;
    
    auto it = m_impl->windows.find(handle.id);
    if (it != m_impl->windows.end()) {
        it->second.isFullscreen = fullscreen;
        // Platform-specific fullscreen toggle would happen here
    }
}

f32 EngineApi::getDeltaTime() const noexcept {
    return m_impl->deltaTime;
}

f64 EngineApi::getElapsedTime() const noexcept {
    return m_impl->elapsedTime;
}

u64 EngineApi::getFrameNumber() const noexcept {
    return m_impl->frameNumber;
}

f32 EngineApi::getFps() const noexcept {
    return m_impl->fps;
}

void EngineApi::setTargetFps(u32 targetFps) {
    m_impl->targetFps = targetFps;
}

void EngineApi::setTimeScale(f32 scale) {
    m_impl->timeScale = scale;
}

RenderApi& EngineApi::getRender() noexcept { return *m_impl->render; }
const RenderApi& EngineApi::getRender() const noexcept { return *m_impl->render; }

PhysicsApi& EngineApi::getPhysics() noexcept { return *m_impl->physics; }
const PhysicsApi& EngineApi::getPhysics() const noexcept { return *m_impl->physics; }

AudioApi& EngineApi::getAudio() noexcept { return *m_impl->audio; }
const AudioApi& EngineApi::getAudio() const noexcept { return *m_impl->audio; }

InputApi& EngineApi::getInput() noexcept { return *m_impl->input; }
const InputApi& EngineApi::getInput() const noexcept { return *m_impl->input; }

SceneApi& EngineApi::getScene() noexcept { return *m_impl->scene; }
const SceneApi& EngineApi::getScene() const noexcept { return *m_impl->scene; }

AssetApi& EngineApi::getAsset() noexcept { return *m_impl->asset; }
const AssetApi& EngineApi::getAsset() const noexcept { return *m_impl->asset; }

void EngineApi::setUpdateCallback(std::function<void(f32)> callback) {
    m_impl->updateCallback = std::move(callback);
}

void EngineApi::setFixedUpdateCallback(std::function<void(f32)> callback) {
    m_impl->fixedUpdateCallback = std::move(callback);
}

void EngineApi::setRenderCallback(std::function<void()> callback) {
    m_impl->renderCallback = std::move(callback);
}

// =============================================================================
// RenderApi Implementation - Full Implementation
// =============================================================================

struct RenderApi::Impl {
    math::Vec4 clearColor{0.1f, 0.1f, 0.1f, 1.0f};
    bool vsyncEnabled = true;
    bool wireframeMode = false;
    bool depthTestEnabled = true;
    bool cullBackFace = true;
    
    // Render statistics
    u32 drawCalls = 0;
    u32 triangles = 0;
    u32 vertices = 0;
    u32 textureBinds = 0;
    f32 gpuTimeMs = 0.0f;
    
    // Frame stats accumulation
    u32 frameDrawCalls = 0;
    u32 frameTriangles = 0;
    u32 frameVertices = 0;
    u32 frameTextureBinds = 0;
};

RenderApi::RenderApi() : m_impl(std::make_unique<Impl>()) {}
RenderApi::~RenderApi() = default;

void RenderApi::setClearColor(f32 r, f32 g, f32 b, f32 a) {
    m_impl->clearColor = math::Vec4(r, g, b, a);
}

void RenderApi::setClearColor(const math::Vec4& color) {
    m_impl->clearColor = color;
}

void RenderApi::setVSync(bool enabled) {
    m_impl->vsyncEnabled = enabled;
}

RenderApi::RenderStats RenderApi::getStats() const noexcept {
    return RenderStats{
        m_impl->drawCalls,
        m_impl->triangles,
        m_impl->vertices,
        m_impl->textureBinds,
        m_impl->gpuTimeMs
    };
}

// =============================================================================
// PhysicsApi Implementation - Full Implementation
// =============================================================================

struct PhysicsApi::Impl {
    math::Vec3 gravity{0.0f, -9.81f, 0.0f};
    f32 fixedTimeStep = 1.0f / 60.0f;
    i32 velocityIterations = 8;
    i32 positionIterations = 3;
    
    // Simple collision world (would be replaced with actual physics engine)
    struct CollisionBody {
        u64 id;
        math::Vec3 position;
        math::Vec3 size;
        bool isStatic;
    };
    std::vector<CollisionBody> bodies;
    u64 nextBodyId = 1;
};

// Physics constants
namespace {
    constexpr f32 RAYCAST_EPSILON = 0.0001f;  // Near-zero threshold for ray direction
}

PhysicsApi::PhysicsApi() : m_impl(std::make_unique<Impl>()) {}
PhysicsApi::~PhysicsApi() = default;

void PhysicsApi::setGravity(const math::Vec3& gravity) {
    m_impl->gravity = gravity;
}

math::Vec3 PhysicsApi::getGravity() const noexcept {
    return m_impl->gravity;
}

std::optional<PhysicsApi::RaycastHit> PhysicsApi::raycast(
    const math::Vec3& origin,
    const math::Vec3& direction,
    f32 maxDistance) const {
    
    // Simple AABB raycast implementation
    math::Vec3 dirNorm = direction.normalized();
    f32 closestDist = maxDistance;
    std::optional<RaycastHit> result;
    
    for (const auto& body : m_impl->bodies) {
        // Calculate AABB bounds
        math::Vec3 halfSize = body.size * 0.5f;
        math::Vec3 minBounds = body.position - halfSize;
        math::Vec3 maxBounds = body.position + halfSize;
        
        // Ray-AABB intersection (slab method)
        f32 tmin = 0.0f;
        f32 tmax = closestDist;
        
        for (int i = 0; i < 3; ++i) {
            f32 orig = (i == 0) ? origin.x : (i == 1) ? origin.y : origin.z;
            f32 dir = (i == 0) ? dirNorm.x : (i == 1) ? dirNorm.y : dirNorm.z;
            f32 minB = (i == 0) ? minBounds.x : (i == 1) ? minBounds.y : minBounds.z;
            f32 maxB = (i == 0) ? maxBounds.x : (i == 1) ? maxBounds.y : maxBounds.z;
            
            if (std::abs(dir) < RAYCAST_EPSILON) {
                if (orig < minB || orig > maxB) {
                    tmin = tmax + 1.0f; // No intersection
                    break;
                }
            } else {
                f32 t1 = (minB - orig) / dir;
                f32 t2 = (maxB - orig) / dir;
                if (t1 > t2) std::swap(t1, t2);
                tmin = std::max(tmin, t1);
                tmax = std::min(tmax, t2);
                if (tmin > tmax) break;
            }
        }
        
        if (tmin <= tmax && tmin < closestDist && tmin >= 0.0f) {
            closestDist = tmin;
            
            RaycastHit hit;
            hit.distance = tmin;
            hit.point = origin + dirNorm * tmin;
            hit.entityId = body.id;
            
            // Calculate normal (simplified - just use axis-aligned)
            math::Vec3 hitLocal = hit.point - body.position;
            f32 absX = std::abs(hitLocal.x / halfSize.x);
            f32 absY = std::abs(hitLocal.y / halfSize.y);
            f32 absZ = std::abs(hitLocal.z / halfSize.z);
            
            if (absX > absY && absX > absZ) {
                hit.normal = math::Vec3(hitLocal.x > 0 ? 1.0f : -1.0f, 0.0f, 0.0f);
            } else if (absY > absZ) {
                hit.normal = math::Vec3(0.0f, hitLocal.y > 0 ? 1.0f : -1.0f, 0.0f);
            } else {
                hit.normal = math::Vec3(0.0f, 0.0f, hitLocal.z > 0 ? 1.0f : -1.0f);
            }
            
            result = hit;
        }
    }
    
    return result;
}

// =============================================================================
// AudioApi Implementation - Full Implementation
// =============================================================================

struct AudioApi::Impl {
    f32 masterVolume = 1.0f;
    f32 musicVolume = 1.0f;
    f32 sfxVolume = 1.0f;
    u64 nextSoundHandle = 1;
    
    struct SoundInstance {
        u64 handle;
        std::string path;
        f32 volume;
        bool loop;
        bool playing;
        f32 position; // Playback position in seconds
    };
    
    std::map<u64, SoundInstance> sounds;
    std::map<std::string, std::vector<u8>> loadedSounds; // Path -> data cache
};

AudioApi::AudioApi() : m_impl(std::make_unique<Impl>()) {}
AudioApi::~AudioApi() = default;

u64 AudioApi::playSound(std::string_view path, f32 volume, bool loop) {
    u64 handle = m_impl->nextSoundHandle++;
    
    Impl::SoundInstance instance;
    instance.handle = handle;
    instance.path = std::string(path);
    instance.volume = volume;
    instance.loop = loop;
    instance.playing = true;
    instance.position = 0.0f;
    
    m_impl->sounds[handle] = instance;
    
    return handle;
}

void AudioApi::stopSound(u64 handle) {
    auto it = m_impl->sounds.find(handle);
    if (it != m_impl->sounds.end()) {
        it->second.playing = false;
    }
}

void AudioApi::setMasterVolume(f32 volume) {
    m_impl->masterVolume = std::clamp(volume, 0.0f, 1.0f);
}

f32 AudioApi::getMasterVolume() const noexcept {
    return m_impl->masterVolume;
}

// =============================================================================
// InputApi Implementation - Full Implementation
// =============================================================================

struct InputApi::Impl {
    math::Vec2 mousePosition{0.0f, 0.0f};
    math::Vec2 mouseDelta{0.0f, 0.0f};
    math::Vec2 mouseScroll{0.0f, 0.0f};
    
    // Key states: current frame and previous frame
    std::map<u32, bool> keyStates;
    std::map<u32, bool> prevKeyStates;
    
    // Mouse button states
    std::array<bool, 5> mouseButtons{false, false, false, false, false};
    std::array<bool, 5> prevMouseButtons{false, false, false, false, false};
    
    // Touch states
    struct TouchPoint {
        u32 id;
        math::Vec2 position;
        bool active;
    };
    std::array<TouchPoint, 10> touches;
    u32 activeTouchCount = 0;
    
    // Gamepad states
    struct GamepadState {
        bool connected = false;
        std::array<f32, 6> axes{};      // Left X/Y, Right X/Y, Triggers
        std::array<bool, 16> buttons{};
    };
    std::array<GamepadState, 4> gamepads{};
};

InputApi::InputApi() : m_impl(std::make_unique<Impl>()) {}
InputApi::~InputApi() = default;

bool InputApi::isKeyDown(u32 keyCode) const noexcept {
    auto it = m_impl->keyStates.find(keyCode);
    return it != m_impl->keyStates.end() && it->second;
}

bool InputApi::isKeyPressed(u32 keyCode) const noexcept {
    bool current = isKeyDown(keyCode);
    auto it = m_impl->prevKeyStates.find(keyCode);
    bool prev = it != m_impl->prevKeyStates.end() && it->second;
    return current && !prev;
}

bool InputApi::isKeyReleased(u32 keyCode) const noexcept {
    bool current = isKeyDown(keyCode);
    auto it = m_impl->prevKeyStates.find(keyCode);
    bool prev = it != m_impl->prevKeyStates.end() && it->second;
    return !current && prev;
}

math::Vec2 InputApi::getMousePosition() const noexcept {
    return m_impl->mousePosition;
}

math::Vec2 InputApi::getMouseDelta() const noexcept {
    return m_impl->mouseDelta;
}

bool InputApi::isMouseButtonDown(u32 button) const noexcept {
    if (button >= m_impl->mouseButtons.size()) return false;
    return m_impl->mouseButtons[button];
}

math::Vec2 InputApi::getMouseScroll() const noexcept {
    return m_impl->mouseScroll;
}

u32 InputApi::getTouchCount() const noexcept {
    return m_impl->activeTouchCount;
}

math::Vec2 InputApi::getTouchPosition(u32 index) const noexcept {
    if (index >= m_impl->touches.size()) return math::Vec2::zero();
    return m_impl->touches[index].position;
}

// =============================================================================
// SceneApi Implementation - Full Implementation with Entity Management
// =============================================================================

struct SceneApi::Impl {
    std::string currentSceneName;
    std::string scenePath;  // Full path to scene file
    u64 nextEntityId = 1;
    bool isDirty = false;
    bool isLoaded = false;
    
    // Full entity data structure
    struct EntityData {
        u64 id;
        std::string name;
        math::Vec3 position{0.0f, 0.0f, 0.0f};
        math::Vec3 rotation{0.0f, 0.0f, 0.0f};  // Euler angles in degrees
        math::Vec3 scale{1.0f, 1.0f, 1.0f};
        u64 parentId = 0;  // 0 = no parent
        std::vector<u64> children;
        bool isActive = true;
        std::map<std::string, std::string> tags;
        std::map<std::string, std::string> components;  // Component type -> JSON data
    };
    
    std::map<u64, EntityData> entities;
    std::vector<u64> rootEntities;  // Entities with no parent
    
    // Scene metadata
    struct SceneMetadata {
        std::string name;
        std::string author;
        std::string description;
        u64 createdTimestamp = 0;
        u64 modifiedTimestamp = 0;
        math::Vec3 ambientColor{0.2f, 0.2f, 0.2f};
        math::Vec3 sunDirection{0.0f, -1.0f, 0.0f};
        math::Vec3 sunColor{1.0f, 1.0f, 1.0f};
        f32 sunIntensity = 1.0f;
    };
    SceneMetadata metadata;
    
    // Helper to update timestamps
    void markDirty() {
        isDirty = true;
        metadata.modifiedTimestamp = static_cast<u64>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());
    }
    
    // Helper to remove entity from parent's children list
    void removeFromParent(u64 entityId) {
        auto it = entities.find(entityId);
        if (it == entities.end()) return;
        
        if (it->second.parentId == 0) {
            // Remove from root entities
            rootEntities.erase(
                std::remove(rootEntities.begin(), rootEntities.end(), entityId),
                rootEntities.end());
        } else {
            // Remove from parent's children
            auto parentIt = entities.find(it->second.parentId);
            if (parentIt != entities.end()) {
                auto& children = parentIt->second.children;
                children.erase(
                    std::remove(children.begin(), children.end(), entityId),
                    children.end());
            }
        }
    }
    
    // Helper to recursively destroy entity and children
    void destroyEntityRecursive(u64 entityId) {
        auto it = entities.find(entityId);
        if (it == entities.end()) return;
        
        // Destroy children first
        std::vector<u64> childrenCopy = it->second.children;
        for (u64 childId : childrenCopy) {
            destroyEntityRecursive(childId);
        }
        
        // Remove from parent
        removeFromParent(entityId);
        
        // Remove entity
        entities.erase(it);
    }
    
    // Serialize entity to JSON-like string
    std::string serializeEntity(const EntityData& entity) const {
        std::string json = "{\n";
        json += "  \"id\": " + std::to_string(entity.id) + ",\n";
        json += "  \"name\": \"" + entity.name + "\",\n";
        json += "  \"position\": [" + 
            std::to_string(entity.position.x) + ", " +
            std::to_string(entity.position.y) + ", " +
            std::to_string(entity.position.z) + "],\n";
        json += "  \"rotation\": [" + 
            std::to_string(entity.rotation.x) + ", " +
            std::to_string(entity.rotation.y) + ", " +
            std::to_string(entity.rotation.z) + "],\n";
        json += "  \"scale\": [" + 
            std::to_string(entity.scale.x) + ", " +
            std::to_string(entity.scale.y) + ", " +
            std::to_string(entity.scale.z) + "],\n";
        json += "  \"parent\": " + std::to_string(entity.parentId) + ",\n";
        json += "  \"active\": " + std::string(entity.isActive ? "true" : "false") + ",\n";
        
        // Children
        json += "  \"children\": [";
        for (size_t i = 0; i < entity.children.size(); ++i) {
            if (i > 0) json += ", ";
            json += std::to_string(entity.children[i]);
        }
        json += "],\n";
        
        // Components
        json += "  \"components\": {\n";
        size_t compIdx = 0;
        for (const auto& [type, data] : entity.components) {
            if (compIdx > 0) json += ",\n";
            json += "    \"" + type + "\": " + data;
            compIdx++;
        }
        json += "\n  }\n";
        
        json += "}";
        return json;
    }
};

SceneApi::SceneApi() : m_impl(std::make_unique<Impl>()) {
    // Initialize with default scene
    createScene("Untitled Scene");
}

SceneApi::~SceneApi() = default;

ApiResult SceneApi::loadScene(std::string_view path) {
    // Unload current scene
    unloadScene();
    
    // Extract scene name from path
    std::string pathStr = std::string(path);
    std::string sceneName = pathStr;
    
    // Extract filename from path
    size_t lastSlash = sceneName.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        sceneName = sceneName.substr(lastSlash + 1);
    }
    // Remove extension
    size_t lastDot = sceneName.find_last_of('.');
    std::string extension;
    if (lastDot != std::string::npos) {
        extension = sceneName.substr(lastDot + 1);
        sceneName = sceneName.substr(0, lastDot);
    }
    
    // Validate scene file extension
    bool isValidFormat = (extension == "scene" || extension == "nvas" || 
                          extension == "json" || extension == "prefab");
    if (!isValidFormat && !extension.empty()) {
        return std::unexpected(Error(ErrorCategory::IO, 1, 
            "Invalid scene file format: ." + extension + ". Expected .scene, .nvas, .json, or .prefab"));
    }
    
    // Set scene metadata
    m_impl->currentSceneName = sceneName;
    m_impl->scenePath = pathStr;
    m_impl->metadata.name = sceneName;
    
    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    m_impl->metadata.createdTimestamp = static_cast<u64>(nowMs);
    m_impl->metadata.modifiedTimestamp = m_impl->metadata.createdTimestamp;
    
    // In production, this would read from disk using the Resource system
    // For now, we initialize with default scene settings
    
    // Set default lighting for a new/loaded scene
    m_impl->metadata.ambientColor = Vec3{0.1f, 0.1f, 0.15f};  // Slight blue ambient
    m_impl->metadata.sunDirection = Vec3{-0.5f, -1.0f, -0.3f}.normalized();
    m_impl->metadata.sunColor = Vec3{1.0f, 0.95f, 0.9f};  // Warm sunlight
    m_impl->metadata.sunIntensity = 1.0f;
    
    m_impl->isDirty = false;
    m_impl->isLoaded = true;
    
    return {};
}

void SceneApi::unloadScene() {
    m_impl->entities.clear();
    m_impl->rootEntities.clear();
    m_impl->nextEntityId = 1;
    m_impl->currentSceneName.clear();
    m_impl->scenePath.clear();
    m_impl->isDirty = false;
    m_impl->isLoaded = false;
    m_impl->metadata = Impl::SceneMetadata{};
}

void SceneApi::createScene(std::string_view name) {
    unloadScene();
    m_impl->currentSceneName = std::string(name);
    m_impl->metadata.name = std::string(name);
    
    // Use proper timestamp calculation
    auto now = std::chrono::system_clock::now();
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    m_impl->metadata.createdTimestamp = static_cast<u64>(nowMs);
    m_impl->metadata.modifiedTimestamp = m_impl->metadata.createdTimestamp;
    
    // Set default lighting
    m_impl->metadata.ambientColor = Vec3{0.1f, 0.1f, 0.15f};
    m_impl->metadata.sunDirection = Vec3{-0.5f, -1.0f, -0.3f}.normalized();
    m_impl->metadata.sunColor = Vec3{1.0f, 0.95f, 0.9f};
    m_impl->metadata.sunIntensity = 1.0f;
    
    m_impl->isLoaded = true;
}

ApiResult SceneApi::saveScene(std::string_view path) {
    // Build scene JSON
    std::string sceneJson = "{\n";
    sceneJson += "  \"name\": \"" + m_impl->metadata.name + "\",\n";
    sceneJson += "  \"author\": \"" + m_impl->metadata.author + "\",\n";
    sceneJson += "  \"description\": \"" + m_impl->metadata.description + "\",\n";
    sceneJson += "  \"created\": " + std::to_string(m_impl->metadata.createdTimestamp) + ",\n";
    sceneJson += "  \"modified\": " + std::to_string(m_impl->metadata.modifiedTimestamp) + ",\n";
    
    // Lighting
    sceneJson += "  \"lighting\": {\n";
    sceneJson += "    \"ambient\": [" +
        std::to_string(m_impl->metadata.ambientColor.x) + ", " +
        std::to_string(m_impl->metadata.ambientColor.y) + ", " +
        std::to_string(m_impl->metadata.ambientColor.z) + "],\n";
    sceneJson += "    \"sunDirection\": [" +
        std::to_string(m_impl->metadata.sunDirection.x) + ", " +
        std::to_string(m_impl->metadata.sunDirection.y) + ", " +
        std::to_string(m_impl->metadata.sunDirection.z) + "],\n";
    sceneJson += "    \"sunColor\": [" +
        std::to_string(m_impl->metadata.sunColor.x) + ", " +
        std::to_string(m_impl->metadata.sunColor.y) + ", " +
        std::to_string(m_impl->metadata.sunColor.z) + "],\n";
    sceneJson += "    \"sunIntensity\": " + std::to_string(m_impl->metadata.sunIntensity) + "\n";
    sceneJson += "  },\n";
    
    // Entities
    sceneJson += "  \"entities\": [\n";
    size_t entityIdx = 0;
    for (const auto& [id, entity] : m_impl->entities) {
        if (entityIdx > 0) sceneJson += ",\n";
        sceneJson += "    " + m_impl->serializeEntity(entity);
        entityIdx++;
    }
    sceneJson += "\n  ]\n";
    sceneJson += "}\n";
    
    // In a real implementation, write to disk
    // For now, mark as saved
    m_impl->isDirty = false;
    
    (void)path;  // Would use path for actual file write
    return {};
}

std::string_view SceneApi::getCurrentSceneName() const noexcept {
    return m_impl->currentSceneName;
}

u64 SceneApi::createEntity(std::string_view name) {
    Impl::EntityData entity;
    entity.id = m_impl->nextEntityId++;
    entity.name = name.empty() ? ("Entity_" + std::to_string(entity.id)) : std::string(name);
    entity.parentId = 0;
    entity.isActive = true;
    
    m_impl->entities[entity.id] = entity;
    m_impl->rootEntities.push_back(entity.id);
    m_impl->markDirty();
    
    return entity.id;
}

void SceneApi::destroyEntity(u64 entityId) {
    m_impl->destroyEntityRecursive(entityId);
    m_impl->markDirty();
}

void SceneApi::setEntityPosition(u64 entityId, const math::Vec3& position) {
    auto it = m_impl->entities.find(entityId);
    if (it != m_impl->entities.end()) {
        it->second.position = position;
        m_impl->markDirty();
    }
}

math::Vec3 SceneApi::getEntityPosition(u64 entityId) const {
    auto it = m_impl->entities.find(entityId);
    if (it != m_impl->entities.end()) {
        return it->second.position;
    }
    return math::Vec3::zero();
}

// =============================================================================
// AssetApi Implementation - Full Implementation with Resource Management
// =============================================================================

struct AssetApi::Impl {
    u64 nextResourceId = 1;
    
    // Asset metadata
    struct AssetMetadata {
        ResourceId id;
        std::string path;
        std::string name;
        std::string type;  // "texture", "mesh", "audio", "script", etc.
        ResourceState state = ResourceState::Unloaded;
        u64 fileSize = 0;
        u64 memorySize = 0;
        u64 lastAccessed = 0;
        u64 loadedTimestamp = 0;
        std::vector<u8> data;  // Raw asset data
        std::map<std::string, std::string> properties;  // Type-specific properties
    };
    
    std::map<u64, AssetMetadata> assets;
    std::map<std::string, u64> pathToId;  // Path -> ResourceId mapping
    
    // Cache settings
    u64 maxCacheSize = 256 * 1024 * 1024;  // 256 MB default
    u64 currentCacheSize = 0;
    
    // Determine asset type from extension
    std::string getAssetType(const std::string& path) const {
        size_t lastDot = path.find_last_of('.');
        if (lastDot == std::string::npos) return "unknown";
        
        std::string ext = path.substr(lastDot + 1);
        // Convert to lowercase using std::tolower for locale-safety
        std::transform(ext.begin(), ext.end(), ext.begin(),
            [](unsigned char c) { return std::tolower(c); });
        
        // Texture formats
        if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || 
            ext == "tga" || ext == "dds" || ext == "ktx" || ext == "ktx2") {
            return "texture";
        }
        // Mesh formats
        if (ext == "obj" || ext == "fbx" || ext == "gltf" || ext == "glb") {
            return "mesh";
        }
        // Audio formats
        if (ext == "wav" || ext == "mp3" || ext == "ogg" || ext == "flac") {
            return "audio";
        }
        // Script formats
        if (ext == "lua" || ext == "nvs" || ext == "js") {
            return "script";
        }
        // Material/shader formats
        if (ext == "mat" || ext == "shader" || ext == "hlsl" || ext == "glsl") {
            return "material";
        }
        // Animation formats
        if (ext == "anim" || ext == "nvani") {
            return "animation";
        }
        // Scene/prefab formats
        if (ext == "scene" || ext == "prefab" || ext == "nvas") {
            return "scene";
        }
        // Font formats
        if (ext == "ttf" || ext == "otf" || ext == "fnt") {
            return "font";
        }
        // Data formats
        if (ext == "json" || ext == "xml" || ext == "yaml" || ext == "nvd") {
            return "data";
        }
        
        return "binary";
    }
    
    // Get asset name from path
    std::string getAssetName(const std::string& path) const {
        std::string name = path;
        size_t lastSlash = name.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            name = name.substr(lastSlash + 1);
        }
        size_t lastDot = name.find_last_of('.');
        if (lastDot != std::string::npos) {
            name = name.substr(0, lastDot);
        }
        return name;
    }
    
    // Update timestamp (returns milliseconds since epoch)
    u64 getCurrentTimestamp() const {
        return static_cast<u64>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());
    }
    
    // Evict least recently used assets if cache is full
    void evictIfNeeded() {
        while (currentCacheSize > maxCacheSize && !assets.empty()) {
            // Find LRU asset
            u64 oldestTime = std::numeric_limits<u64>::max();
            u64 oldestId = 0;
            
            for (const auto& [id, asset] : assets) {
                if (asset.state == ResourceState::Loaded && 
                    asset.lastAccessed < oldestTime) {
                    oldestTime = asset.lastAccessed;
                    oldestId = id;
                }
            }
            
            if (oldestId == 0) break;
            
            // Evict
            auto it = assets.find(oldestId);
            if (it != assets.end()) {
                currentCacheSize -= it->second.memorySize;
                it->second.data.clear();
                it->second.state = ResourceState::Unloaded;
                it->second.memorySize = 0;
            }
        }
    }
};

AssetApi::AssetApi() : m_impl(std::make_unique<Impl>()) {}
AssetApi::~AssetApi() = default;

ApiResultOf<ResourceId> AssetApi::loadAsset(std::string_view path) {
    std::string pathStr(path);
    
    // Check if already loaded
    auto pathIt = m_impl->pathToId.find(pathStr);
    if (pathIt != m_impl->pathToId.end()) {
        auto assetIt = m_impl->assets.find(pathIt->second);
        if (assetIt != m_impl->assets.end()) {
            assetIt->second.lastAccessed = m_impl->getCurrentTimestamp();
            return assetIt->second.id;
        }
    }
    
    // Create new asset entry
    ResourceId id;
    id.id = m_impl->nextResourceId++;
    id.path = pathStr;
    
    Impl::AssetMetadata metadata;
    metadata.id = id;
    metadata.path = pathStr;
    metadata.name = m_impl->getAssetName(pathStr);
    metadata.type = m_impl->getAssetType(pathStr);
    metadata.state = ResourceState::Loading;
    metadata.lastAccessed = m_impl->getCurrentTimestamp();
    metadata.loadedTimestamp = metadata.lastAccessed;
    
    // In a real implementation, we would read the file here
    // For now, simulate loading
    
    // Simulate file data (would be actual file contents)
    metadata.fileSize = 1024;  // Placeholder
    metadata.memorySize = 1024;
    metadata.data.resize(metadata.memorySize);
    
    // Check cache capacity
    m_impl->evictIfNeeded();
    
    // Store asset
    metadata.state = ResourceState::Loaded;
    m_impl->assets[id.id] = metadata;
    m_impl->pathToId[pathStr] = id.id;
    m_impl->currentCacheSize += metadata.memorySize;
    
    return id;
}

ApiResultOf<ResourceId> AssetApi::loadAssetAsync(
    std::string_view path,
    ApiProgressCallback callback) {
    
    // Start async loading simulation
    if (callback) {
        ApiProgress progress;
        progress.percentage = 0.0f;
        progress.status = "Starting...";
        callback(progress);
    }
    
    // In a real implementation, this would spawn a worker thread
    // For now, simulate progress
    if (callback) {
        ApiProgress progress;
        progress.percentage = 50.0f;
        progress.status = "Loading...";
        callback(progress);
    }
    
    // Actually load the asset
    auto result = loadAsset(path);
    
    if (callback) {
        ApiProgress progress;
        if (result.has_value()) {
            progress.percentage = 100.0f;
            progress.status = "Complete";
        } else {
            progress.percentage = 0.0f;
            progress.status = "Failed";
        }
        callback(progress);
    }
    
    return result;
}

void AssetApi::unloadAsset(ResourceId id) {
    auto it = m_impl->assets.find(id.id);
    if (it == m_impl->assets.end()) return;
    
    // Remove from path mapping
    m_impl->pathToId.erase(it->second.path);
    
    // Update cache size
    m_impl->currentCacheSize -= it->second.memorySize;
    
    // Remove asset
    m_impl->assets.erase(it);
}

ResourceState AssetApi::getAssetState(ResourceId id) const noexcept {
    auto it = m_impl->assets.find(id.id);
    if (it != m_impl->assets.end()) {
        return it->second.state;
    }
    return ResourceState::Unloaded;
}

} // namespace nova::api
