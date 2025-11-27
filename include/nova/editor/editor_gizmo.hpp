/**
 * @file editor_gizmo.hpp
 * @brief NovaCore Editor™ - Transform Gizmos and Visual Handles
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 * 
 * Implements the 3D gizmo system for visual manipulation of entities.
 * Supports move, rotate, and scale gizmos with both mouse and touch input.
 * The gizmo system is designed for mobile-first interaction with
 * appropriate hit areas and visual feedback.
 */

#pragma once

#include "editor_types.hpp"
#include <nova/core/ecs/entity.hpp>

#include <memory>
#include <vector>
#include <functional>

namespace nova::editor {

// ============================================================================
// Gizmo Enums
// ============================================================================

/**
 * @brief Gizmo axis or plane
 */
enum class GizmoAxis : u8 {
    None = 0,
    X = 1,
    Y = 2,
    Z = 3,
    XY = 4,       // XY plane
    XZ = 5,       // XZ plane
    YZ = 6,       // YZ plane
    XYZ = 7,      // All axes (uniform scale, free rotate)
    View = 8,     // View-aligned (screen space)
    Center = 9    // Center handle
};

/**
 * @brief Gizmo operation result
 */
enum class GizmoResult : u8 {
    None,         // No interaction
    Hover,        // Hovering over gizmo
    Started,      // Started dragging
    Dragging,     // Currently dragging
    Finished      // Finished dragging
};

// ============================================================================
// Gizmo State
// ============================================================================

/**
 * @brief Current gizmo interaction state
 */
struct GizmoState {
    // Interaction state
    GizmoResult result = GizmoResult::None;
    GizmoAxis activeAxis = GizmoAxis::None;
    GizmoAxis hoveredAxis = GizmoAxis::None;
    
    // Transform deltas (accumulated during drag)
    Vec3 translationDelta;
    Quat rotationDelta;
    Vec3 scaleDelta;
    
    // World space results
    Vec3 worldPosition;
    Quat worldRotation;
    Vec3 worldScale;
    
    // Interaction points
    Vec2 startMousePos;
    Vec2 currentMousePos;
    Vec3 startWorldPos;
    Vec3 currentWorldPos;
    
    // Snap points (if snapping is enabled)
    Vec3 snappedPosition;
    Vec3 snappedRotation;  // Euler angles
    Vec3 snappedScale;
    
    bool isHovering() const { return result == GizmoResult::Hover; }
    bool isDragging() const { return result == GizmoResult::Dragging; }
    bool isActive() const { return result == GizmoResult::Started || result == GizmoResult::Dragging; }
    bool justStarted() const { return result == GizmoResult::Started; }
    bool justFinished() const { return result == GizmoResult::Finished; }
};

// ============================================================================
// Base Gizmo Class
// ============================================================================

/**
 * @brief Base class for all gizmos
 * 
 * Gizmos provide visual handles for manipulating entities in the scene view.
 * Each gizmo type (move, rotate, scale) extends this base class.
 */
class EditorGizmo {
public:
    virtual ~EditorGizmo() = default;
    
    /**
     * @brief Update gizmo state and handle input
     * 
     * @param camera Current editor camera
     * @param mousePos Current mouse/touch position in screen space
     * @param isPressed Whether mouse button/touch is pressed
     * @param selection Current selection
     * @return GizmoState describing the current interaction
     */
    virtual GizmoState update(const EditorCamera& camera,
                              const Vec2& mousePos,
                              bool isPressed,
                              const Selection& selection) = 0;
    
    /**
     * @brief Draw the gizmo
     * 
     * @param camera Current editor camera
     * @param selection Current selection
     * @param state Current gizmo state
     */
    virtual void draw(const EditorCamera& camera,
                      const Selection& selection,
                      const GizmoState& state) = 0;
    
    /**
     * @brief Get the gizmo type
     */
    virtual TransformTool getType() const = 0;
    
    // Settings access
    void setSettings(const GizmoSettings& settings) { m_settings = settings; }
    const GizmoSettings& getSettings() const { return m_settings; }
    
    void setSnapSettings(const SnapSettings& snap) { m_snapSettings = snap; }
    const SnapSettings& getSnapSettings() const { return m_snapSettings; }
    
    void setTransformSpace(TransformSpace space) { m_space = space; }
    TransformSpace getTransformSpace() const { return m_space; }
    
protected:
    /**
     * @brief Calculate gizmo position from selection
     */
    Vec3 calculateGizmoPosition(const Selection& selection) const;
    
    /**
     * @brief Calculate gizmo orientation
     */
    Quat calculateGizmoOrientation(const Selection& selection) const;
    
    /**
     * @brief Calculate screen-space scale for constant size
     */
    f32 calculateScreenScale(const EditorCamera& camera, const Vec3& worldPos) const;
    
    /**
     * @brief Test ray against axis handle
     */
    bool rayTestAxis(const Ray& ray, const Vec3& origin, const Vec3& direction,
                     f32 length, f32 radius, f32& t) const;
    
    /**
     * @brief Test ray against plane handle
     */
    bool rayTestPlane(const Ray& ray, const Vec3& origin, const Vec3& normal,
                      f32 size, f32& t) const;
    
    /**
     * @brief Apply snapping to value
     */
    f32 applySnap(f32 value, f32 snapValue) const;
    Vec3 applySnap(const Vec3& value, f32 snapValue) const;
    
    /**
     * @brief Get axis color (with highlighting)
     */
    Vec4 getAxisColor(GizmoAxis axis, GizmoAxis hoveredAxis, GizmoAxis activeAxis) const;
    
    GizmoSettings m_settings;
    SnapSettings m_snapSettings;
    TransformSpace m_space = TransformSpace::World;
};

// ============================================================================
// Translation Gizmo
// ============================================================================

/**
 * @brief Move/Translation gizmo
 * 
 * Three arrows for single-axis translation
 * Three planes for dual-axis translation
 * Screen-aligned handle for free translation
 */
class TranslationGizmo : public EditorGizmo {
public:
    TranslationGizmo();
    
    GizmoState update(const EditorCamera& camera,
                      const Vec2& mousePos,
                      bool isPressed,
                      const Selection& selection) override;
    
    void draw(const EditorCamera& camera,
              const Selection& selection,
              const GizmoState& state) override;
    
    TransformTool getType() const override { return TransformTool::Move; }
    
private:
    /**
     * @brief Determine which axis is under the cursor
     */
    GizmoAxis hitTest(const Ray& ray, const Vec3& origin, const Quat& orientation, f32 scale);
    
    /**
     * @brief Calculate translation on axis
     */
    Vec3 calculateAxisTranslation(const Ray& ray, const Vec3& origin, 
                                  const Vec3& axis, const Vec2& startMouse);
    
    /**
     * @brief Calculate translation on plane
     */
    Vec3 calculatePlaneTranslation(const Ray& ray, const Vec3& origin,
                                   const Vec3& normal);
    
    // Interaction state
    Vec3 m_dragStartPosition;
    Vec3 m_dragCurrentPosition;
    Ray m_dragStartRay;
    Vec3 m_dragPlaneNormal;
    f32 m_dragStartT = 0.0f;
};

// ============================================================================
// Rotation Gizmo
// ============================================================================

/**
 * @brief Rotation gizmo
 * 
 * Three circles for single-axis rotation
 * Screen-aligned circle for view rotation
 * Sphere for free rotation (arcball)
 */
class RotationGizmo : public EditorGizmo {
public:
    RotationGizmo();
    
    GizmoState update(const EditorCamera& camera,
                      const Vec2& mousePos,
                      bool isPressed,
                      const Selection& selection) override;
    
    void draw(const EditorCamera& camera,
              const Selection& selection,
              const GizmoState& state) override;
    
    TransformTool getType() const override { return TransformTool::Rotate; }
    
private:
    /**
     * @brief Determine which axis ring is under the cursor
     */
    GizmoAxis hitTest(const Ray& ray, const Vec3& origin, const Quat& orientation,
                      f32 scale, const Vec3& viewDir);
    
    /**
     * @brief Calculate rotation around axis from mouse movement
     */
    Quat calculateAxisRotation(const Vec2& currentMouse, const Vec2& startMouse,
                               const EditorCamera& camera, const Vec3& axis);
    
    /**
     * @brief Calculate arcball rotation
     */
    Quat calculateArcballRotation(const Vec2& currentMouse, const Vec2& startMouse,
                                  const EditorCamera& camera);
    
    // Interaction state
    Quat m_dragStartRotation;
    Vec2 m_dragStartMouse;
    f32 m_dragStartAngle = 0.0f;
    f32 m_arcballRadius = 1.0f;
};

// ============================================================================
// Scale Gizmo
// ============================================================================

/**
 * @brief Scale gizmo
 * 
 * Three handles for single-axis scaling
 * Center handle for uniform scaling
 * Three planes for dual-axis scaling
 */
class ScaleGizmo : public EditorGizmo {
public:
    ScaleGizmo();
    
    GizmoState update(const EditorCamera& camera,
                      const Vec2& mousePos,
                      bool isPressed,
                      const Selection& selection) override;
    
    void draw(const EditorCamera& camera,
              const Selection& selection,
              const GizmoState& state) override;
    
    TransformTool getType() const override { return TransformTool::Scale; }
    
private:
    /**
     * @brief Determine which handle is under the cursor
     */
    GizmoAxis hitTest(const Ray& ray, const Vec3& origin, const Quat& orientation, f32 scale);
    
    /**
     * @brief Calculate scale factor from mouse movement
     */
    f32 calculateScaleFactor(const Vec2& currentMouse, const Vec2& startMouse,
                             const EditorCamera& camera, const Vec3& axis);
    
    // Interaction state
    Vec3 m_dragStartScale;
    Vec2 m_dragStartMouse;
};

// ============================================================================
// Combined Gizmo
// ============================================================================

/**
 * @brief Combined move/rotate/scale gizmo
 * 
 * All three gizmo types combined into one, with mode
 * selected based on which handle is clicked.
 */
class CombinedGizmo : public EditorGizmo {
public:
    CombinedGizmo();
    
    GizmoState update(const EditorCamera& camera,
                      const Vec2& mousePos,
                      bool isPressed,
                      const Selection& selection) override;
    
    void draw(const EditorCamera& camera,
              const Selection& selection,
              const GizmoState& state) override;
    
    TransformTool getType() const override { return TransformTool::Combined; }
    
    /**
     * @brief Get the currently active sub-gizmo mode
     */
    TransformTool getActiveMode() const { return m_activeMode; }
    
private:
    std::unique_ptr<TranslationGizmo> m_translateGizmo;
    std::unique_ptr<RotationGizmo> m_rotateGizmo;
    std::unique_ptr<ScaleGizmo> m_scaleGizmo;
    TransformTool m_activeMode = TransformTool::Move;
};

// ============================================================================
// Rect Transform Gizmo (2D)
// ============================================================================

/**
 * @brief 2D rectangle transform gizmo
 * 
 * For UI elements and 2D objects:
 * - Corner handles for scaling
 * - Edge handles for single-axis scaling
 * - Center for moving
 * - Rotation handle
 */
class RectGizmo : public EditorGizmo {
public:
    RectGizmo();
    
    GizmoState update(const EditorCamera& camera,
                      const Vec2& mousePos,
                      bool isPressed,
                      const Selection& selection) override;
    
    void draw(const EditorCamera& camera,
              const Selection& selection,
              const GizmoState& state) override;
    
    TransformTool getType() const override { return TransformTool::Rect; }
    
private:
    enum class RectHandle {
        None,
        TopLeft, Top, TopRight,
        Left, Center, Right,
        BottomLeft, Bottom, BottomRight,
        Rotate
    };
    
    RectHandle hitTest(const Vec2& mousePos, const AABB& rect, f32 handleSize);
    
    RectHandle m_activeHandle = RectHandle::None;
    RectHandle m_hoveredHandle = RectHandle::None;
    Vec2 m_dragStartPos;
    AABB m_dragStartRect;
};

// ============================================================================
// Gizmo Manager
// ============================================================================

/**
 * @brief Manages gizmo creation and switching
 */
class GizmoManager {
public:
    GizmoManager();
    
    /**
     * @brief Set the active gizmo type
     */
    void setGizmoType(TransformTool type);
    
    /**
     * @brief Get the active gizmo
     */
    EditorGizmo* getActiveGizmo() { return m_activeGizmo; }
    
    /**
     * @brief Update active gizmo
     */
    GizmoState update(const EditorCamera& camera,
                      const Vec2& mousePos,
                      bool isPressed,
                      const Selection& selection);
    
    /**
     * @brief Draw active gizmo
     */
    void draw(const EditorCamera& camera,
              const Selection& selection,
              const GizmoState& state);
    
    /**
     * @brief Set settings for all gizmos
     */
    void setSettings(const GizmoSettings& settings);
    
    /**
     * @brief Set snap settings for all gizmos
     */
    void setSnapSettings(const SnapSettings& snap);
    
    /**
     * @brief Set transform space for all gizmos
     */
    void setTransformSpace(TransformSpace space);
    
private:
    std::unique_ptr<TranslationGizmo> m_translateGizmo;
    std::unique_ptr<RotationGizmo> m_rotateGizmo;
    std::unique_ptr<ScaleGizmo> m_scaleGizmo;
    std::unique_ptr<CombinedGizmo> m_combinedGizmo;
    std::unique_ptr<RectGizmo> m_rectGizmo;
    
    EditorGizmo* m_activeGizmo = nullptr;
    TransformTool m_activeType = TransformTool::Move;
};

// ============================================================================
// Inline Implementations
// ============================================================================

inline Vec3 EditorGizmo::calculateGizmoPosition(const Selection& selection) const {
    if (selection.isEmpty()) {
        return Vec3::zero();
    }
    return selection.selectionCenter;
}

inline Quat EditorGizmo::calculateGizmoOrientation(const Selection& selection) const {
    if (m_space == TransformSpace::World || selection.isEmpty()) {
        return Quat::identity();
    }
    
    // For local space, use the active entity's rotation
    if (selection.activeEntity.has_value()) {
        // Note: Would get rotation from world
        return Quat::identity();
    }
    
    return Quat::identity();
}

inline f32 EditorGizmo::calculateScreenScale(const EditorCamera& camera, const Vec3& worldPos) const {
    if (!m_settings.screenSpaceScale) {
        return 1.0f;
    }
    
    // Calculate distance from camera
    f32 distance = (worldPos - camera.position).length();
    
    // Scale based on FOV to maintain constant screen size
    f32 fovScale = std::tan(radians(camera.fov * 0.5f));
    return distance * fovScale * m_settings.size * 0.1f;
}

inline f32 EditorGizmo::applySnap(f32 value, f32 snapValue) const {
    if (snapValue <= 0.0f) return value;
    return std::round(value / snapValue) * snapValue;
}

inline Vec3 EditorGizmo::applySnap(const Vec3& value, f32 snapValue) const {
    return Vec3(
        applySnap(value.x, snapValue),
        applySnap(value.y, snapValue),
        applySnap(value.z, snapValue)
    );
}

inline Vec4 EditorGizmo::getAxisColor(GizmoAxis axis, GizmoAxis hoveredAxis, GizmoAxis activeAxis) const {
    // Highlight color takes precedence
    if (axis == activeAxis || axis == hoveredAxis) {
        return m_settings.highlightColor;
    }
    
    switch (axis) {
        case GizmoAxis::X:
        case GizmoAxis::XY:
        case GizmoAxis::XZ:
            return m_settings.xAxisColor;
        case GizmoAxis::Y:
        case GizmoAxis::YZ:
            return m_settings.yAxisColor;
        case GizmoAxis::Z:
            return m_settings.zAxisColor;
        default:
            return Vec4(0.8f, 0.8f, 0.8f, 1.0f);
    }
}

} // namespace nova::editor
