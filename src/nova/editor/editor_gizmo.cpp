/**
 * @file editor_gizmo.cpp
 * @brief NovaCore Editor™ - Transform Gizmo Implementation
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#include <nova/editor/editor_gizmo.hpp>
#include <nova/editor/editor_context.hpp>

#include <cmath>
#include <algorithm>

namespace nova::editor {

// ============================================================================
// Translation Gizmo Implementation
// ============================================================================

TranslationGizmo::TranslationGizmo() {
    // Default settings
}

GizmoState TranslationGizmo::update(const EditorCamera& camera,
                                    const Vec2& mousePos,
                                    bool isPressed,
                                    const Selection& selection) {
    GizmoState state;
    state.result = GizmoResult::None;
    
    if (selection.isEmpty()) {
        return state;
    }
    
    Vec3 origin = calculateGizmoPosition(selection);
    Quat orientation = calculateGizmoOrientation(selection);
    f32 scale = calculateScreenScale(camera, origin);
    
    state.worldPosition = origin;
    state.worldRotation = orientation;
    
    // Create ray from mouse position
    Ray ray = camera.screenPointToRay(mousePos, Vec2(camera.projectionMatrix[0][0], 
                                                      camera.projectionMatrix[1][1]));
    
    GizmoAxis hitAxis = hitTest(ray, origin, orientation, scale);
    
    if (!isPressed) {
        // Just hovering
        state.hoveredAxis = hitAxis;
        state.activeAxis = GizmoAxis::None;
        
        if (hitAxis != GizmoAxis::None) {
            state.result = GizmoResult::Hover;
        }
        
        // Reset drag state
        m_dragStartPosition = Vec3::zero();
        return state;
    }
    
    // Mouse is pressed
    if (m_dragStartPosition == Vec3::zero()) {
        // Starting a new drag
        if (hitAxis != GizmoAxis::None) {
            state.result = GizmoResult::Started;
            state.activeAxis = hitAxis;
            
            m_dragStartPosition = origin;
            m_dragStartRay = ray;
            
            // Determine drag plane based on axis
            switch (hitAxis) {
                case GizmoAxis::X:
                    m_dragPlaneNormal = Vec3::up();
                    break;
                case GizmoAxis::Y:
                    m_dragPlaneNormal = Vec3::forward();
                    break;
                case GizmoAxis::Z:
                    m_dragPlaneNormal = Vec3::up();
                    break;
                case GizmoAxis::XY:
                    m_dragPlaneNormal = Vec3::forward();
                    break;
                case GizmoAxis::XZ:
                    m_dragPlaneNormal = Vec3::up();
                    break;
                case GizmoAxis::YZ:
                    m_dragPlaneNormal = Vec3::right();
                    break;
                default:
                    m_dragPlaneNormal = camera.forward();
                    break;
            }
        }
    } else {
        // Continuing drag
        state.result = GizmoResult::Dragging;
        state.activeAxis = hitAxis != GizmoAxis::None ? hitAxis : state.activeAxis;
        
        // Calculate translation based on active axis
        Vec3 translation = Vec3::zero();
        
        Vec3 axisDir = orientation * Vec3::right();
        if (state.activeAxis == GizmoAxis::Y) {
            axisDir = orientation * Vec3::up();
        } else if (state.activeAxis == GizmoAxis::Z) {
            axisDir = orientation * Vec3::forward();
        }
        
        // Project mouse movement onto axis
        f32 t;
        if (rayTestPlane(ray, m_dragStartPosition, m_dragPlaneNormal, 1000.0f, t)) {
            Vec3 hitPoint = ray.origin + ray.direction * t;
            
            switch (state.activeAxis) {
                case GizmoAxis::X:
                    translation.x = (hitPoint - m_dragStartPosition).dot(orientation * Vec3::right());
                    break;
                case GizmoAxis::Y:
                    translation.y = (hitPoint - m_dragStartPosition).dot(orientation * Vec3::up());
                    break;
                case GizmoAxis::Z:
                    translation.z = (hitPoint - m_dragStartPosition).dot(orientation * Vec3::forward());
                    break;
                case GizmoAxis::XY:
                    translation.x = (hitPoint - m_dragStartPosition).dot(orientation * Vec3::right());
                    translation.y = (hitPoint - m_dragStartPosition).dot(orientation * Vec3::up());
                    break;
                case GizmoAxis::XZ:
                    translation.x = (hitPoint - m_dragStartPosition).dot(orientation * Vec3::right());
                    translation.z = (hitPoint - m_dragStartPosition).dot(orientation * Vec3::forward());
                    break;
                case GizmoAxis::YZ:
                    translation.y = (hitPoint - m_dragStartPosition).dot(orientation * Vec3::up());
                    translation.z = (hitPoint - m_dragStartPosition).dot(orientation * Vec3::forward());
                    break;
                default:
                    translation = hitPoint - m_dragStartPosition;
                    break;
            }
            
            // Apply snapping if enabled
            if (m_snapSettings.positionSnap) {
                translation = applySnap(translation, m_snapSettings.positionSnapValue);
            }
            
            state.translationDelta = translation;
            state.worldPosition = m_dragStartPosition + translation;
            state.snappedPosition = state.worldPosition;
        }
    }
    
    return state;
}

void TranslationGizmo::draw(const EditorCamera& camera,
                            const Selection& selection,
                            const GizmoState& state) {
    if (selection.isEmpty()) {
        return;
    }
    
    Vec3 origin = calculateGizmoPosition(selection);
    Quat orientation = calculateGizmoOrientation(selection);
    f32 scale = calculateScreenScale(camera, origin);
    
    // Note: Would use render commands to draw gizmo geometry
    // This would integrate with the Nova GraphicsCore™ render system
    
    // Draw axis arrows
    f32 arrowLength = scale * 1.0f;
    f32 arrowHeadLength = scale * 0.15f;
    f32 arrowHeadRadius = scale * 0.05f;
    
    Vec3 xEnd = origin + orientation * Vec3::right() * arrowLength;
    Vec3 yEnd = origin + orientation * Vec3::up() * arrowLength;
    Vec3 zEnd = origin + orientation * Vec3::forward() * arrowLength;
    
    // X axis (red)
    Vec4 xColor = getAxisColor(GizmoAxis::X, state.hoveredAxis, state.activeAxis);
    // DrawLine(origin, xEnd, xColor);
    // DrawCone(xEnd, orientation * Vec3::right(), arrowHeadLength, arrowHeadRadius, xColor);
    
    // Y axis (green)
    Vec4 yColor = getAxisColor(GizmoAxis::Y, state.hoveredAxis, state.activeAxis);
    // DrawLine(origin, yEnd, yColor);
    // DrawCone(yEnd, orientation * Vec3::up(), arrowHeadLength, arrowHeadRadius, yColor);
    
    // Z axis (blue)
    Vec4 zColor = getAxisColor(GizmoAxis::Z, state.hoveredAxis, state.activeAxis);
    // DrawLine(origin, zEnd, zColor);
    // DrawCone(zEnd, orientation * Vec3::forward(), arrowHeadLength, arrowHeadRadius, zColor);
    
    // Draw plane handles
    f32 planeSize = scale * 0.25f;
    f32 planeOffset = scale * 0.4f;
    
    // XY plane
    Vec4 xyColor = getAxisColor(GizmoAxis::XY, state.hoveredAxis, state.activeAxis);
    // DrawQuad(origin + Vec3(planeOffset, planeOffset, 0), planeSize, xyColor);
    
    // XZ plane
    Vec4 xzColor = getAxisColor(GizmoAxis::XZ, state.hoveredAxis, state.activeAxis);
    // DrawQuad(origin + Vec3(planeOffset, 0, planeOffset), planeSize, xzColor);
    
    // YZ plane
    Vec4 yzColor = getAxisColor(GizmoAxis::YZ, state.hoveredAxis, state.activeAxis);
    // DrawQuad(origin + Vec3(0, planeOffset, planeOffset), planeSize, yzColor);
}

GizmoAxis TranslationGizmo::hitTest(const Ray& ray, const Vec3& origin, 
                                    const Quat& orientation, f32 scale) {
    f32 hitT = std::numeric_limits<f32>::max();
    GizmoAxis hitAxis = GizmoAxis::None;
    
    f32 arrowLength = scale * 1.0f;
    f32 hitRadius = scale * 0.1f;  // Larger for touch input
    
    f32 t;
    
    // Test X axis
    if (rayTestAxis(ray, origin, orientation * Vec3::right(), arrowLength, hitRadius, t)) {
        if (t < hitT) {
            hitT = t;
            hitAxis = GizmoAxis::X;
        }
    }
    
    // Test Y axis
    if (rayTestAxis(ray, origin, orientation * Vec3::up(), arrowLength, hitRadius, t)) {
        if (t < hitT) {
            hitT = t;
            hitAxis = GizmoAxis::Y;
        }
    }
    
    // Test Z axis
    if (rayTestAxis(ray, origin, orientation * Vec3::forward(), arrowLength, hitRadius, t)) {
        if (t < hitT) {
            hitT = t;
            hitAxis = GizmoAxis::Z;
        }
    }
    
    // Test plane handles
    f32 planeSize = scale * 0.25f;
    f32 planeOffset = scale * 0.4f;
    
    // XY plane
    if (rayTestPlane(ray, origin + Vec3(planeOffset, planeOffset, 0), 
                     orientation * Vec3::forward(), planeSize, t)) {
        if (t < hitT) {
            hitT = t;
            hitAxis = GizmoAxis::XY;
        }
    }
    
    // XZ plane
    if (rayTestPlane(ray, origin + Vec3(planeOffset, 0, planeOffset),
                     orientation * Vec3::up(), planeSize, t)) {
        if (t < hitT) {
            hitT = t;
            hitAxis = GizmoAxis::XZ;
        }
    }
    
    // YZ plane
    if (rayTestPlane(ray, origin + Vec3(0, planeOffset, planeOffset),
                     orientation * Vec3::right(), planeSize, t)) {
        if (t < hitT) {
            hitT = t;
            hitAxis = GizmoAxis::YZ;
        }
    }
    
    return hitAxis;
}

bool EditorGizmo::rayTestAxis(const Ray& ray, const Vec3& origin, const Vec3& direction,
                              f32 length, f32 radius, f32& t) const {
    // Ray-cylinder intersection
    Vec3 d = ray.direction;
    Vec3 o = ray.origin - origin;
    Vec3 n = direction;
    
    Vec3 D = d - n * d.dot(n);
    Vec3 O = o - n * o.dot(n);
    
    f32 a = D.dot(D);
    f32 b = 2.0f * D.dot(O);
    f32 c = O.dot(O) - radius * radius;
    
    f32 discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f) return false;
    
    f32 sqrtD = std::sqrt(discriminant);
    f32 t0 = (-b - sqrtD) / (2.0f * a);
    f32 t1 = (-b + sqrtD) / (2.0f * a);
    
    t = t0 > 0.0f ? t0 : t1;
    if (t < 0.0f) return false;
    
    // Check if hit point is within cylinder length
    Vec3 hitPoint = ray.origin + ray.direction * t - origin;
    f32 axisT = hitPoint.dot(direction);
    
    if (axisT < 0.0f || axisT > length) return false;
    
    return true;
}

bool EditorGizmo::rayTestPlane(const Ray& ray, const Vec3& origin, const Vec3& normal,
                               f32 size, f32& t) const {
    f32 denom = ray.direction.dot(normal);
    if (std::abs(denom) < 1e-6f) return false;
    
    t = (origin - ray.origin).dot(normal) / denom;
    if (t < 0.0f) return false;
    
    Vec3 hitPoint = ray.origin + ray.direction * t;
    Vec3 localHit = hitPoint - origin;
    
    // Check if within plane bounds
    if (std::abs(localHit.x) > size && std::abs(localHit.y) > size && std::abs(localHit.z) > size) {
        return false;
    }
    
    return true;
}

// ============================================================================
// Rotation Gizmo Implementation
// ============================================================================

RotationGizmo::RotationGizmo() {
}

GizmoState RotationGizmo::update(const EditorCamera& camera,
                                 const Vec2& mousePos,
                                 bool isPressed,
                                 const Selection& selection) {
    GizmoState state;
    state.result = GizmoResult::None;
    
    if (selection.isEmpty()) {
        return state;
    }
    
    Vec3 origin = calculateGizmoPosition(selection);
    Quat orientation = calculateGizmoOrientation(selection);
    f32 scale = calculateScreenScale(camera, origin);
    
    state.worldPosition = origin;
    state.worldRotation = orientation;
    
    Ray ray = camera.screenPointToRay(mousePos, Vec2(1280, 720)); // Note: Would use actual viewport size
    Vec3 viewDir = (camera.position - origin).normalized();
    
    GizmoAxis hitAxis = hitTest(ray, origin, orientation, scale, viewDir);
    
    if (!isPressed) {
        state.hoveredAxis = hitAxis;
        state.activeAxis = GizmoAxis::None;
        
        if (hitAxis != GizmoAxis::None) {
            state.result = GizmoResult::Hover;
        }
        
        m_dragStartRotation = Quat::identity();
        return state;
    }
    
    if (m_dragStartRotation == Quat::identity()) {
        if (hitAxis != GizmoAxis::None) {
            state.result = GizmoResult::Started;
            state.activeAxis = hitAxis;
            
            m_dragStartRotation = orientation;
            m_dragStartMouse = mousePos;
            m_dragStartAngle = 0.0f;
        }
    } else {
        state.result = GizmoResult::Dragging;
        state.activeAxis = hitAxis != GizmoAxis::None ? hitAxis : state.activeAxis;
        
        // Calculate rotation based on mouse movement
        Vec3 rotationAxis = Vec3::up();
        switch (state.activeAxis) {
            case GizmoAxis::X:
                rotationAxis = orientation * Vec3::right();
                break;
            case GizmoAxis::Y:
                rotationAxis = orientation * Vec3::up();
                break;
            case GizmoAxis::Z:
                rotationAxis = orientation * Vec3::forward();
                break;
            case GizmoAxis::View:
                rotationAxis = viewDir;
                break;
            default:
                break;
        }
        
        Quat rotation = calculateAxisRotation(mousePos, m_dragStartMouse, camera, rotationAxis);
        
        // Apply snapping
        if (m_snapSettings.rotationSnap) {
            Vec3 euler = rotation.toEuler();
            constexpr f32 degToRad = 3.14159265358979323846f / 180.0f;
            euler = applySnap(euler, m_snapSettings.rotationSnapValue * degToRad);
            rotation = Quat::fromEuler(euler);
            state.snappedRotation = euler;
        }
        
        state.rotationDelta = rotation;
        state.worldRotation = m_dragStartRotation * rotation;
    }
    
    return state;
}

void RotationGizmo::draw(const EditorCamera& camera,
                         const Selection& selection,
                         const GizmoState& state) {
    if (selection.isEmpty()) {
        return;
    }
    
    Vec3 origin = calculateGizmoPosition(selection);
    Quat orientation = calculateGizmoOrientation(selection);
    f32 scale = calculateScreenScale(camera, origin);
    f32 radius = scale * 1.0f;
    
    // Note: Would draw rotation rings using render commands
    
    // X axis ring (red, rotates around X)
    Vec4 xColor = getAxisColor(GizmoAxis::X, state.hoveredAxis, state.activeAxis);
    // DrawCircle(origin, orientation * Vec3::right(), radius, xColor);
    
    // Y axis ring (green, rotates around Y)
    Vec4 yColor = getAxisColor(GizmoAxis::Y, state.hoveredAxis, state.activeAxis);
    // DrawCircle(origin, orientation * Vec3::up(), radius, yColor);
    
    // Z axis ring (blue, rotates around Z)
    Vec4 zColor = getAxisColor(GizmoAxis::Z, state.hoveredAxis, state.activeAxis);
    // DrawCircle(origin, orientation * Vec3::forward(), radius, zColor);
    
    // View-aligned ring (white)
    Vec3 viewDir = (camera.position - origin).normalized();
    Vec4 viewColor = getAxisColor(GizmoAxis::View, state.hoveredAxis, state.activeAxis);
    // DrawCircle(origin, viewDir, radius * 1.1f, viewColor);
}

GizmoAxis RotationGizmo::hitTest(const Ray& ray, const Vec3& origin, const Quat& orientation,
                                 f32 scale, const Vec3& viewDir) {
    f32 radius = scale * 1.0f;
    f32 thickness = scale * 0.1f;
    
    // Note: Would implement torus intersection for rotation rings
    // For now, simplified sphere segment test
    
    return GizmoAxis::None;
}

Quat RotationGizmo::calculateAxisRotation(const Vec2& currentMouse, const Vec2& startMouse,
                                          const EditorCamera& camera, const Vec3& axis) {
    Vec2 delta = currentMouse - startMouse;
    f32 angle = delta.x * 0.01f;  // Sensitivity factor
    
    return Quat::fromAxisAngle(axis, angle);
}

Quat RotationGizmo::calculateArcballRotation(const Vec2& currentMouse, const Vec2& startMouse,
                                             const EditorCamera& camera) {
    // Arcball rotation algorithm
    // Note: Would implement full arcball with virtual sphere
    return Quat::identity();
}

// ============================================================================
// Scale Gizmo Implementation
// ============================================================================

ScaleGizmo::ScaleGizmo() {
}

GizmoState ScaleGizmo::update(const EditorCamera& camera,
                              const Vec2& mousePos,
                              bool isPressed,
                              const Selection& selection) {
    GizmoState state;
    state.result = GizmoResult::None;
    state.scaleDelta = Vec3::one();
    
    if (selection.isEmpty()) {
        return state;
    }
    
    Vec3 origin = calculateGizmoPosition(selection);
    Quat orientation = calculateGizmoOrientation(selection);
    f32 scale = calculateScreenScale(camera, origin);
    
    state.worldPosition = origin;
    state.worldRotation = orientation;
    state.worldScale = Vec3::one();
    
    Ray ray = camera.screenPointToRay(mousePos, Vec2(1280, 720));
    
    GizmoAxis hitAxis = hitTest(ray, origin, orientation, scale);
    
    if (!isPressed) {
        state.hoveredAxis = hitAxis;
        state.activeAxis = GizmoAxis::None;
        
        if (hitAxis != GizmoAxis::None) {
            state.result = GizmoResult::Hover;
        }
        
        m_dragStartScale = Vec3::one();
        return state;
    }
    
    if (m_dragStartScale == Vec3::one() && hitAxis == GizmoAxis::None) {
        return state;
    }
    
    if (m_dragStartScale == Vec3::one()) {
        state.result = GizmoResult::Started;
        state.activeAxis = hitAxis;
        
        m_dragStartScale = Vec3::one();
        m_dragStartMouse = mousePos;
    } else {
        state.result = GizmoResult::Dragging;
        state.activeAxis = hitAxis != GizmoAxis::None ? hitAxis : state.activeAxis;
        
        // Calculate scale factor from mouse movement
        Vec2 delta = mousePos - m_dragStartMouse;
        f32 scaleFactor = 1.0f + delta.x * 0.01f;
        
        Vec3 scaleVec = Vec3::one();
        switch (state.activeAxis) {
            case GizmoAxis::X:
                scaleVec.x = scaleFactor;
                break;
            case GizmoAxis::Y:
                scaleVec.y = scaleFactor;
                break;
            case GizmoAxis::Z:
                scaleVec.z = scaleFactor;
                break;
            case GizmoAxis::XYZ:
            case GizmoAxis::Center:
                scaleVec = Vec3(scaleFactor, scaleFactor, scaleFactor);
                break;
            default:
                break;
        }
        
        // Apply snapping
        if (m_snapSettings.scaleSnap) {
            scaleVec = applySnap(scaleVec, m_snapSettings.scaleSnapValue);
        }
        
        state.scaleDelta = scaleVec;
        state.worldScale = scaleVec;
        state.snappedScale = scaleVec;
    }
    
    return state;
}

void ScaleGizmo::draw(const EditorCamera& camera,
                      const Selection& selection,
                      const GizmoState& state) {
    if (selection.isEmpty()) {
        return;
    }
    
    Vec3 origin = calculateGizmoPosition(selection);
    Quat orientation = calculateGizmoOrientation(selection);
    f32 scale = calculateScreenScale(camera, origin);
    
    f32 lineLength = scale * 1.0f;
    f32 cubeSize = scale * 0.1f;
    
    // Note: Would draw scale handles using render commands
    
    // X axis (red)
    Vec4 xColor = getAxisColor(GizmoAxis::X, state.hoveredAxis, state.activeAxis);
    Vec3 xEnd = origin + orientation * Vec3::right() * lineLength;
    // DrawLine(origin, xEnd, xColor);
    // DrawCube(xEnd, cubeSize, xColor);
    
    // Y axis (green)
    Vec4 yColor = getAxisColor(GizmoAxis::Y, state.hoveredAxis, state.activeAxis);
    Vec3 yEnd = origin + orientation * Vec3::up() * lineLength;
    // DrawLine(origin, yEnd, yColor);
    // DrawCube(yEnd, cubeSize, yColor);
    
    // Z axis (blue)
    Vec4 zColor = getAxisColor(GizmoAxis::Z, state.hoveredAxis, state.activeAxis);
    Vec3 zEnd = origin + orientation * Vec3::forward() * lineLength;
    // DrawLine(origin, zEnd, zColor);
    // DrawCube(zEnd, cubeSize, zColor);
    
    // Center cube (uniform scale)
    Vec4 centerColor = getAxisColor(GizmoAxis::Center, state.hoveredAxis, state.activeAxis);
    // DrawCube(origin, cubeSize * 1.5f, centerColor);
}

GizmoAxis ScaleGizmo::hitTest(const Ray& ray, const Vec3& origin, const Quat& orientation, f32 scale) {
    f32 lineLength = scale * 1.0f;
    f32 hitRadius = scale * 0.15f;
    
    f32 hitT = std::numeric_limits<f32>::max();
    GizmoAxis hitAxis = GizmoAxis::None;
    
    f32 t;
    
    // Test axis handles
    if (rayTestAxis(ray, origin, orientation * Vec3::right(), lineLength, hitRadius, t)) {
        if (t < hitT) {
            hitT = t;
            hitAxis = GizmoAxis::X;
        }
    }
    
    if (rayTestAxis(ray, origin, orientation * Vec3::up(), lineLength, hitRadius, t)) {
        if (t < hitT) {
            hitT = t;
            hitAxis = GizmoAxis::Y;
        }
    }
    
    if (rayTestAxis(ray, origin, orientation * Vec3::forward(), lineLength, hitRadius, t)) {
        if (t < hitT) {
            hitT = t;
            hitAxis = GizmoAxis::Z;
        }
    }
    
    // Test center cube
    // Note: Would implement box intersection
    
    return hitAxis;
}

// ============================================================================
// Combined Gizmo Implementation
// ============================================================================

CombinedGizmo::CombinedGizmo() {
    m_translateGizmo = std::make_unique<TranslationGizmo>();
    m_rotateGizmo = std::make_unique<RotationGizmo>();
    m_scaleGizmo = std::make_unique<ScaleGizmo>();
}

GizmoState CombinedGizmo::update(const EditorCamera& camera,
                                 const Vec2& mousePos,
                                 bool isPressed,
                                 const Selection& selection) {
    // Delegate to active sub-gizmo
    switch (m_activeMode) {
        case TransformTool::Move:
            return m_translateGizmo->update(camera, mousePos, isPressed, selection);
        case TransformTool::Rotate:
            return m_rotateGizmo->update(camera, mousePos, isPressed, selection);
        case TransformTool::Scale:
            return m_scaleGizmo->update(camera, mousePos, isPressed, selection);
        default:
            return GizmoState();
    }
}

void CombinedGizmo::draw(const EditorCamera& camera,
                         const Selection& selection,
                         const GizmoState& state) {
    // Draw all three gizmos with reduced opacity for inactive
    m_translateGizmo->draw(camera, selection, state);
    m_rotateGizmo->draw(camera, selection, state);
    m_scaleGizmo->draw(camera, selection, state);
}

// ============================================================================
// Rect Gizmo Implementation
// ============================================================================

RectGizmo::RectGizmo() {
}

GizmoState RectGizmo::update(const EditorCamera& camera,
                             const Vec2& mousePos,
                             bool isPressed,
                             const Selection& selection) {
    GizmoState state;
    // Note: Would implement 2D rect transform gizmo
    return state;
}

void RectGizmo::draw(const EditorCamera& camera,
                     const Selection& selection,
                     const GizmoState& state) {
    // Note: Would draw 2D rectangle with handles
}

// ============================================================================
// Gizmo Manager Implementation
// ============================================================================

GizmoManager::GizmoManager() {
    m_translateGizmo = std::make_unique<TranslationGizmo>();
    m_rotateGizmo = std::make_unique<RotationGizmo>();
    m_scaleGizmo = std::make_unique<ScaleGizmo>();
    m_combinedGizmo = std::make_unique<CombinedGizmo>();
    m_rectGizmo = std::make_unique<RectGizmo>();
    
    m_activeGizmo = m_translateGizmo.get();
    m_activeType = TransformTool::Move;
}

void GizmoManager::setGizmoType(TransformTool type) {
    m_activeType = type;
    
    switch (type) {
        case TransformTool::Move:
            m_activeGizmo = m_translateGizmo.get();
            break;
        case TransformTool::Rotate:
            m_activeGizmo = m_rotateGizmo.get();
            break;
        case TransformTool::Scale:
            m_activeGizmo = m_scaleGizmo.get();
            break;
        case TransformTool::Combined:
            m_activeGizmo = m_combinedGizmo.get();
            break;
        case TransformTool::Rect:
            m_activeGizmo = m_rectGizmo.get();
            break;
        default:
            m_activeGizmo = nullptr;
            break;
    }
}

GizmoState GizmoManager::update(const EditorCamera& camera,
                                const Vec2& mousePos,
                                bool isPressed,
                                const Selection& selection) {
    if (!m_activeGizmo) {
        return GizmoState();
    }
    
    return m_activeGizmo->update(camera, mousePos, isPressed, selection);
}

void GizmoManager::draw(const EditorCamera& camera,
                        const Selection& selection,
                        const GizmoState& state) {
    if (!m_activeGizmo) {
        return;
    }
    
    m_activeGizmo->draw(camera, selection, state);
}

void GizmoManager::setSettings(const GizmoSettings& settings) {
    m_translateGizmo->setSettings(settings);
    m_rotateGizmo->setSettings(settings);
    m_scaleGizmo->setSettings(settings);
    m_combinedGizmo->setSettings(settings);
    m_rectGizmo->setSettings(settings);
}

void GizmoManager::setSnapSettings(const SnapSettings& snap) {
    m_translateGizmo->setSnapSettings(snap);
    m_rotateGizmo->setSnapSettings(snap);
    m_scaleGizmo->setSnapSettings(snap);
    m_combinedGizmo->setSnapSettings(snap);
    m_rectGizmo->setSnapSettings(snap);
}

void GizmoManager::setTransformSpace(TransformSpace space) {
    m_translateGizmo->setTransformSpace(space);
    m_rotateGizmo->setTransformSpace(space);
    m_scaleGizmo->setTransformSpace(space);
    m_combinedGizmo->setTransformSpace(space);
    m_rectGizmo->setTransformSpace(space);
}

} // namespace nova::editor
