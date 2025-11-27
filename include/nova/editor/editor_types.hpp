/**
 * @file editor_types.hpp
 * @brief NovaCore Editor™ - Core Type Definitions
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 * 
 * Defines the fundamental types, enums, and structures used throughout
 * the NovaCore Editor system. This includes selection, undo/redo,
 * tool modes, and editor state management.
 */

#pragma once

#include <nova/core/types/types.hpp>
#include <nova/core/math/math.hpp>
#include <nova/core/ecs/entity.hpp>
#include <nova/core/physics/physics_types.hpp>

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <optional>
#include <variant>

namespace nova::editor {

// Bring in types from nova namespaces
using namespace nova;
using namespace nova::math;
using nova::physics::AABB;
using nova::physics::Ray;

// ============================================================================
// Forward Declarations
// ============================================================================

class EditorContext;
class EditorPanel;
class EditorWindow;
class EditorCommand;
class EditorTool;
class EditorGizmo;
class SelectionManager;
class UndoRedoManager;
class AssetBrowser;
class SceneHierarchy;
class Inspector;
class ConsolePanel;
class ProjectManager;

// ============================================================================
// Editor Configuration Constants
// ============================================================================

namespace EditorConfig {
    // Auto-save settings
    constexpr u32 AUTO_SAVE_INTERVAL_SECONDS = 60;
    constexpr u32 MAX_UNDO_HISTORY_DEPTH = 100;
    constexpr u32 MAX_RECENT_PROJECTS = 10;
    
    // Viewport settings
    constexpr f32 DEFAULT_CAMERA_SPEED = 10.0f;
    constexpr f32 DEFAULT_GRID_SIZE = 1.0f;
    constexpr f32 DEFAULT_GIZMO_SIZE = 1.0f;
    constexpr u32 DEFAULT_VIEWPORT_FPS_LIMIT = 60;
    
    // Touch input settings
    constexpr f32 TOUCH_DRAG_THRESHOLD = 10.0f;      // Pixels
    constexpr f32 TOUCH_LONG_PRESS_TIME = 0.5f;      // Seconds
    constexpr f32 TOUCH_DOUBLE_TAP_TIME = 0.3f;      // Seconds
    constexpr f32 TOUCH_PINCH_SENSITIVITY = 1.0f;
    constexpr f32 TOUCH_ROTATION_SENSITIVITY = 1.0f;
    
    // Performance budgets
    constexpr f32 TARGET_FRAME_TIME_MS = 16.67f;     // 60 FPS
    constexpr f32 UI_UPDATE_BUDGET_MS = 2.0f;        // Max time for UI
    constexpr f32 GIZMO_BUDGET_MS = 1.0f;            // Max time for gizmos
    constexpr f32 SELECTION_UPDATE_MS = 0.5f;        // Max selection update time
}

// ============================================================================
// Editor State Enums
// ============================================================================

/**
 * @brief Current state of the editor application
 */
enum class EditorState : u8 {
    Initializing,       // Editor is starting up
    Loading,            // Loading project/assets
    Ready,              // Ready for user input
    Playing,            // Game is playing (play mode)
    Paused,             // Game is paused
    Compiling,          // Scripts/shaders compiling
    Building,           // Building project
    Exporting,          // Exporting assets/project
    Error               // Error state
};

/**
 * @brief Current editing mode
 */
enum class EditMode : u8 {
    Scene,              // Scene editing mode
    Prefab,             // Editing a prefab
    Animation,          // Animation editing mode
    Material,           // Material editing mode
    Terrain,            // Terrain editing mode
    ParticleSystem,     // Particle system editing
    UICanvas,           // UI canvas editing
    Code                // Code editing mode
};

/**
 * @brief Active transform tool
 */
enum class TransformTool : u8 {
    None,               // No tool active
    Move,               // Translation tool (W)
    Rotate,             // Rotation tool (E)
    Scale,              // Scale tool (R)
    Rect,               // 2D rectangle tool (T)
    Combined            // All transforms (Y)
};

/**
 * @brief Transform space for tools
 */
enum class TransformSpace : u8 {
    World,              // World coordinate system
    Local,              // Local entity space
    View,               // Camera view space
    Parent              // Parent entity space
};

/**
 * @brief Pivot mode for transformations
 */
enum class PivotMode : u8 {
    Center,             // Pivot at selection center
    Pivot,              // Use entity's pivot point
    Active,             // Pivot at active (last selected) entity
    Custom              // User-defined pivot position
};

/**
 * @brief Scene view rendering mode
 */
enum class ViewMode : u8 {
    Shaded,             // Full PBR lighting
    Wireframe,          // Mesh topology
    Unlit,              // No lighting
    Normals,            // Normal visualization
    UVChecker,          // UV distortion
    Lightmap,           // Baked lighting
    Overdraw,           // Pixel overdraw heatmap
    Mipmap,             // Mipmap levels
    DepthOnly,          // Depth buffer
    Albedo,             // Base color only
    Metallic,           // Metallic channel
    Roughness,          // Roughness channel
    AmbientOcclusion,   // AO channel
    Emission            // Emission channel
};

/**
 * @brief Camera projection mode
 */
enum class ProjectionMode : u8 {
    Perspective,        // Standard perspective
    Orthographic        // Orthographic projection
};

/**
 * @brief Snap mode for transformations
 */
enum class SnapMode : u8 {
    None,               // No snapping
    Grid,               // Snap to grid
    Surface,            // Snap to surfaces
    Vertex,             // Snap to vertices
    Edge,               // Snap to edges
    Pivot               // Snap to pivots
};

/**
 * @brief Panel dock position
 */
enum class DockPosition : u8 {
    None,               // Floating
    Left,               // Left side
    Right,              // Right side
    Top,                // Top
    Bottom,             // Bottom
    Center,             // Center (tabbed)
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

/**
 * @brief Asset type classification
 */
enum class AssetType : u8 {
    Unknown,
    Scene,              // .nscene
    Prefab,             // .nprefab
    Material,           // .nmat
    Texture,            // .ntex, .png, .jpg, etc.
    Mesh,               // .nmesh, .gltf, .fbx
    Animation,          // .nanim
    AnimationController,// .nanimctrl
    Audio,              // .naudio, .wav, .ogg, .mp3
    Script,             // .nscript, .lua
    Shader,             // .nshader, .glsl
    Font,               // .nfont, .ttf
    ParticleSystem,     // .nparticle
    Terrain,            // .nterrain
    UILayout,           // .nui
    LocalizationTable,  // .nloc
    PhysicsMaterial,    // .nphysmat
    NavMesh,            // .nnavmesh
    Folder              // Directory
};

/**
 * @brief Touch gesture types for mobile editing
 */
enum class TouchGesture : u8 {
    None,
    Tap,                // Single finger tap
    DoubleTap,          // Double tap
    LongPress,          // Long press
    Drag,               // Single finger drag
    TwoFingerDrag,      // Two finger pan
    Pinch,              // Pinch zoom
    Rotate,             // Two finger rotation
    ThreeFingerSwipe    // Three finger gesture
};

// ============================================================================
// Editor Data Structures
// ============================================================================

/**
 * @brief Selection info for a single entity
 */
struct SelectionInfo {
    ecs::Entity entity;
    Vec3 localBounds;          // Local space bounds
    Vec3 worldCenter;          // World space center
    f32 distance;              // Distance from camera
    bool isLocked = false;     // Entity is locked (can't modify)
    bool isHidden = false;     // Entity is hidden in editor
    
    SelectionInfo() = default;
    SelectionInfo(ecs::Entity e) : entity(e) {}
};

/**
 * @brief Current selection state
 */
struct Selection {
    std::vector<SelectionInfo> entities;
    std::optional<ecs::Entity> activeEntity;  // Last selected (pivot reference)
    Vec3 selectionCenter;                      // Center of selection bounds
    AABB selectionBounds;                      // Combined bounds
    
    bool isEmpty() const { return entities.empty(); }
    usize count() const { return entities.size(); }
    bool contains(ecs::Entity entity) const;
    void add(ecs::Entity entity);
    void remove(ecs::Entity entity);
    void clear();
    void set(ecs::Entity entity);
    void setMultiple(const std::vector<ecs::Entity>& entities);
    void updateBounds();
};

/**
 * @brief Camera state for scene view
 */
struct EditorCamera {
    Vec3 position = Vec3(0.0f, 5.0f, -10.0f);
    Quat rotation = Quat::identity();
    f32 fov = 60.0f;
    f32 nearPlane = 0.1f;
    f32 farPlane = 10000.0f;
    f32 orthoSize = 10.0f;
    ProjectionMode projection = ProjectionMode::Perspective;
    
    // Movement state
    f32 moveSpeed = EditorConfig::DEFAULT_CAMERA_SPEED;
    f32 lookSensitivity = 0.5f;
    f32 zoomSensitivity = 1.0f;
    bool isFlying = false;
    
    // Computed matrices (updated each frame)
    Mat4 viewMatrix;
    Mat4 projectionMatrix;
    Mat4 viewProjectionMatrix;
    
    void updateMatrices(f32 aspectRatio);
    Vec3 forward() const { return rotation * Vec3::forward(); }
    Vec3 right() const { return rotation * Vec3::right(); }
    Vec3 up() const { return rotation * Vec3::up(); }
    
    Ray screenPointToRay(const Vec2& screenPoint, const Vec2& screenSize) const;
    Vec3 worldToScreen(const Vec3& worldPos, const Vec2& screenSize) const;
    Vec3 screenToWorld(const Vec2& screenPos, f32 depth, const Vec2& screenSize) const;
};

/**
 * @brief Grid visualization settings
 */
struct GridSettings {
    bool visible = true;
    f32 size = EditorConfig::DEFAULT_GRID_SIZE;
    u32 subdivisions = 10;
    Vec4 primaryColor = Vec4(0.3f, 0.3f, 0.3f, 1.0f);
    Vec4 secondaryColor = Vec4(0.2f, 0.2f, 0.2f, 0.5f);
    f32 fadeDistance = 100.0f;
    bool showAxisColors = true;  // X=red, Y=green, Z=blue
};

/**
 * @brief Snap settings for transformations
 */
struct SnapSettings {
    bool positionSnap = false;
    bool rotationSnap = false;
    bool scaleSnap = false;
    bool surfaceSnap = false;
    bool vertexSnap = false;
    
    f32 positionSnapValue = 0.25f;
    f32 rotationSnapValue = 15.0f;
    f32 scaleSnapValue = 0.1f;
};

/**
 * @brief Gizmo display settings
 */
struct GizmoSettings {
    f32 size = EditorConfig::DEFAULT_GIZMO_SIZE;
    f32 handleSize = 0.05f;       // Size of interaction handles
    f32 planeSize = 0.3f;         // Size of plane handles
    f32 screenSpaceScale = true;  // Constant screen size
    f32 selectionOpacity = 0.8f;
    Vec4 xAxisColor = Vec4(1.0f, 0.2f, 0.2f, 1.0f);
    Vec4 yAxisColor = Vec4(0.2f, 1.0f, 0.2f, 1.0f);
    Vec4 zAxisColor = Vec4(0.2f, 0.2f, 1.0f, 1.0f);
    Vec4 highlightColor = Vec4(1.0f, 0.9f, 0.0f, 1.0f);
};

/**
 * @brief Touch input state for mobile editing
 */
struct TouchState {
    u32 touchCount = 0;
    Vec2 touchPositions[10];      // Up to 10 touch points
    Vec2 previousPositions[10];
    bool touching[10] = {false};
    
    // Gesture detection
    TouchGesture currentGesture = TouchGesture::None;
    f32 gestureStartTime = 0.0f;
    Vec2 gestureStartPos;
    f32 pinchStartDistance = 0.0f;
    f32 rotationStartAngle = 0.0f;
    
    // Computed gesture values
    Vec2 panDelta;                // Two-finger pan amount
    f32 pinchDelta = 0.0f;        // Pinch zoom amount
    f32 rotationDelta = 0.0f;     // Two-finger rotation amount
    
    void update();
    f32 getPinchDistance() const;
    f32 getRotationAngle() const;
    Vec2 getAveragePosition() const;
};

/**
 * @brief Editor viewport state
 */
struct ViewportState {
    u32 width = 0;
    u32 height = 0;
    Vec2 position;                // Screen position of viewport
    EditorCamera camera;
    ViewMode viewMode = ViewMode::Shaded;
    bool showGrid = true;
    bool showGizmos = true;
    bool showIcons = true;
    bool showStats = false;
    bool isFocused = false;
    bool isHovered = false;
    
    // Picking
    ecs::Entity hoveredEntity;
    Vec3 hoveredWorldPos;
    
    f32 getAspectRatio() const {
        return height > 0 ? static_cast<f32>(width) / static_cast<f32>(height) : 1.0f;
    }
};

/**
 * @brief Undo/Redo command base structure
 */
struct CommandInfo {
    std::string name;
    std::string description;
    std::chrono::system_clock::time_point timestamp;
    bool isMergeable = false;
    u64 mergeId = 0;
};

/**
 * @brief Project information
 */
struct ProjectInfo {
    std::string name;
    std::string path;
    std::string version;
    std::chrono::system_clock::time_point lastOpened;
    std::chrono::system_clock::time_point created;
    bool isValid = false;
    std::string thumbnailPath;
    
    // Project settings
    std::string defaultScene;
    std::string companyName;
    std::string productName;
    std::string bundleIdentifier;
};

/**
 * @brief Asset metadata for browser
 */
struct AssetInfo {
    std::string name;
    std::string path;
    std::string extension;
    AssetType type = AssetType::Unknown;
    u64 size = 0;
    std::chrono::system_clock::time_point modifiedTime;
    bool isImported = false;
    bool hasThumbnail = false;
    std::string thumbnailPath;
    std::vector<std::string> dependencies;
    std::vector<std::string> labels;
};

/**
 * @brief Console message entry
 */
struct ConsoleMessage {
    enum class Type : u8 {
        Info,
        Warning,
        Error,
        Assert,
        Exception
    };
    
    Type type = Type::Info;
    std::string message;
    std::string stackTrace;
    std::string file;
    u32 line = 0;
    std::chrono::system_clock::time_point timestamp;
    u32 count = 1;              // For collapsed duplicates
    bool isCollapsed = false;
};

/**
 * @brief Editor notification
 */
struct EditorNotification {
    enum class Type : u8 {
        Info,
        Success,
        Warning,
        Error,
        Progress
    };
    
    Type type = Type::Info;
    std::string title;
    std::string message;
    f32 progress = 0.0f;        // For progress type
    f32 duration = 3.0f;        // Display duration
    f32 elapsed = 0.0f;
    bool dismissible = true;
    std::function<void()> onClick;
};

// ============================================================================
// Editor Theme
// ============================================================================

/**
 * @brief Color scheme for editor UI
 */
struct EditorTheme {
    std::string name = "Dark";
    
    // Background colors
    Vec4 windowBackground = Vec4(0.15f, 0.15f, 0.15f, 1.0f);
    Vec4 panelBackground = Vec4(0.18f, 0.18f, 0.18f, 1.0f);
    Vec4 headerBackground = Vec4(0.12f, 0.12f, 0.12f, 1.0f);
    Vec4 viewportBackground = Vec4(0.1f, 0.1f, 0.1f, 1.0f);
    
    // Text colors
    Vec4 textPrimary = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    Vec4 textSecondary = Vec4(0.7f, 0.7f, 0.7f, 1.0f);
    Vec4 textDisabled = Vec4(0.4f, 0.4f, 0.4f, 1.0f);
    Vec4 textLink = Vec4(0.3f, 0.6f, 1.0f, 1.0f);
    
    // Accent colors
    Vec4 accent = Vec4(0.26f, 0.59f, 0.98f, 1.0f);      // Blue
    Vec4 accentHover = Vec4(0.36f, 0.69f, 1.0f, 1.0f);
    Vec4 accentPressed = Vec4(0.16f, 0.49f, 0.88f, 1.0f);
    
    // Status colors
    Vec4 success = Vec4(0.2f, 0.8f, 0.3f, 1.0f);
    Vec4 warning = Vec4(1.0f, 0.8f, 0.2f, 1.0f);
    Vec4 error = Vec4(1.0f, 0.3f, 0.3f, 1.0f);
    Vec4 info = Vec4(0.3f, 0.6f, 1.0f, 1.0f);
    
    // Selection colors
    Vec4 selection = Vec4(0.26f, 0.59f, 0.98f, 0.3f);
    Vec4 selectionBorder = Vec4(0.26f, 0.59f, 0.98f, 1.0f);
    Vec4 hover = Vec4(0.3f, 0.3f, 0.3f, 1.0f);
    
    // Border and separator
    Vec4 border = Vec4(0.25f, 0.25f, 0.25f, 1.0f);
    Vec4 separator = Vec4(0.2f, 0.2f, 0.2f, 1.0f);
    
    // Input fields
    Vec4 inputBackground = Vec4(0.1f, 0.1f, 0.1f, 1.0f);
    Vec4 inputBorder = Vec4(0.25f, 0.25f, 0.25f, 1.0f);
    Vec4 inputFocusBorder = Vec4(0.26f, 0.59f, 0.98f, 1.0f);
    
    // Font settings
    f32 fontSizeSmall = 11.0f;
    f32 fontSizeNormal = 13.0f;
    f32 fontSizeLarge = 16.0f;
    f32 fontSizeHeading = 18.0f;
    
    // Layout settings
    f32 itemSpacing = 8.0f;
    f32 windowPadding = 8.0f;
    f32 framePadding = 4.0f;
    f32 borderRadius = 4.0f;
    f32 scrollbarSize = 14.0f;
    
    static EditorTheme dark();
    static EditorTheme light();
    static EditorTheme highContrast();
};

// ============================================================================
// Editor Callback Types
// ============================================================================

// Selection callbacks
using SelectionChangedCallback = std::function<void(const Selection&)>;
using EntitySelectedCallback = std::function<void(ecs::Entity)>;
using EntityDeselectedCallback = std::function<void(ecs::Entity)>;

// Command callbacks
using CommandExecutedCallback = std::function<void(const CommandInfo&)>;
using UndoCallback = std::function<void(const CommandInfo&)>;
using RedoCallback = std::function<void(const CommandInfo&)>;

// Asset callbacks
using AssetImportedCallback = std::function<void(const AssetInfo&)>;
using AssetDeletedCallback = std::function<void(const std::string& path)>;
using AssetRenamedCallback = std::function<void(const std::string& oldPath, const std::string& newPath)>;

// Project callbacks
using ProjectOpenedCallback = std::function<void(const ProjectInfo&)>;
using ProjectClosedCallback = std::function<void()>;
using ProjectSavedCallback = std::function<void(const ProjectInfo&)>;

// Scene callbacks
using SceneLoadedCallback = std::function<void(const std::string& scenePath)>;
using SceneSavedCallback = std::function<void(const std::string& scenePath)>;
using PlayModeChangedCallback = std::function<void(bool isPlaying)>;

// ============================================================================
// Inline Implementations
// ============================================================================

inline bool Selection::contains(ecs::Entity entity) const {
    for (const auto& info : entities) {
        if (info.entity == entity) return true;
    }
    return false;
}

inline void Selection::add(ecs::Entity entity) {
    if (!contains(entity)) {
        entities.emplace_back(entity);
        activeEntity = entity;
        updateBounds();
    }
}

inline void Selection::remove(ecs::Entity entity) {
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
            [entity](const SelectionInfo& info) { return info.entity == entity; }),
        entities.end()
    );
    if (activeEntity == entity) {
        activeEntity = entities.empty() ? std::nullopt : std::optional(entities.back().entity);
    }
    updateBounds();
}

inline void Selection::clear() {
    entities.clear();
    activeEntity = std::nullopt;
    selectionCenter = Vec3::zero();
    selectionBounds = AABB();
}

inline void Selection::set(ecs::Entity entity) {
    clear();
    add(entity);
}

inline void Selection::setMultiple(const std::vector<ecs::Entity>& newEntities) {
    clear();
    for (auto entity : newEntities) {
        add(entity);
    }
}

inline void Selection::updateBounds() {
    if (entities.empty()) {
        selectionCenter = Vec3::zero();
        selectionBounds = AABB();
        return;
    }
    
    Vec3 minPoint(std::numeric_limits<f32>::max());
    Vec3 maxPoint(std::numeric_limits<f32>::lowest());
    
    for (const auto& info : entities) {
        minPoint = minPoint.min(info.worldCenter - info.localBounds * 0.5f);
        maxPoint = maxPoint.max(info.worldCenter + info.localBounds * 0.5f);
    }
    
    selectionBounds.min = minPoint;
    selectionBounds.max = maxPoint;
    selectionCenter = (minPoint + maxPoint) * 0.5f;
}

inline void EditorCamera::updateMatrices(f32 aspectRatio) {
    viewMatrix = Mat4::lookAt(position, position + forward(), Vec3::up());
    
    if (projection == ProjectionMode::Perspective) {
        projectionMatrix = Mat4::perspective(radians(fov), aspectRatio, nearPlane, farPlane);
    } else {
        f32 halfWidth = orthoSize * aspectRatio;
        f32 halfHeight = orthoSize;
        projectionMatrix = Mat4::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, nearPlane, farPlane);
    }
    
    viewProjectionMatrix = projectionMatrix * viewMatrix;
}

inline EditorTheme EditorTheme::dark() {
    return EditorTheme();  // Default is dark theme
}

inline EditorTheme EditorTheme::light() {
    EditorTheme theme;
    theme.name = "Light";
    theme.windowBackground = Vec4(0.95f, 0.95f, 0.95f, 1.0f);
    theme.panelBackground = Vec4(0.92f, 0.92f, 0.92f, 1.0f);
    theme.headerBackground = Vec4(0.85f, 0.85f, 0.85f, 1.0f);
    theme.viewportBackground = Vec4(0.7f, 0.7f, 0.7f, 1.0f);
    theme.textPrimary = Vec4(0.1f, 0.1f, 0.1f, 1.0f);
    theme.textSecondary = Vec4(0.3f, 0.3f, 0.3f, 1.0f);
    theme.textDisabled = Vec4(0.5f, 0.5f, 0.5f, 1.0f);
    theme.border = Vec4(0.7f, 0.7f, 0.7f, 1.0f);
    theme.separator = Vec4(0.75f, 0.75f, 0.75f, 1.0f);
    theme.inputBackground = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    theme.hover = Vec4(0.85f, 0.85f, 0.85f, 1.0f);
    return theme;
}

inline EditorTheme EditorTheme::highContrast() {
    EditorTheme theme;
    theme.name = "High Contrast";
    theme.windowBackground = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    theme.panelBackground = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    theme.headerBackground = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    theme.viewportBackground = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    theme.textPrimary = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    theme.textSecondary = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    theme.accent = Vec4(0.0f, 1.0f, 1.0f, 1.0f);
    theme.border = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    theme.selection = Vec4(1.0f, 1.0f, 0.0f, 0.5f);
    theme.selectionBorder = Vec4(1.0f, 1.0f, 0.0f, 1.0f);
    return theme;
}

} // namespace nova::editor
