/**
 * @file test_editor.cpp
 * @brief Unit tests for NovaCore Editor System Types
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/editor/editor_types.hpp>

using namespace nova;
using namespace nova::editor;
using Catch::Approx;

// ============================================================================
// Editor Configuration Tests
// ============================================================================

TEST_CASE("Editor Types - EditorConfig", "[editor][types]") {
    SECTION("Auto-save settings") {
        REQUIRE(EditorConfig::AUTO_SAVE_INTERVAL_SECONDS == 60);
        REQUIRE(EditorConfig::MAX_UNDO_HISTORY_DEPTH == 100);
        REQUIRE(EditorConfig::MAX_RECENT_PROJECTS == 10);
    }
    
    SECTION("Viewport settings") {
        REQUIRE(EditorConfig::DEFAULT_CAMERA_SPEED == Approx(10.0f));
        REQUIRE(EditorConfig::DEFAULT_GRID_SIZE == Approx(1.0f));
        REQUIRE(EditorConfig::DEFAULT_GIZMO_SIZE == Approx(1.0f));
        REQUIRE(EditorConfig::DEFAULT_VIEWPORT_FPS_LIMIT == 60);
    }
    
    SECTION("Touch input settings") {
        REQUIRE(EditorConfig::TOUCH_DRAG_THRESHOLD == Approx(10.0f));
        REQUIRE(EditorConfig::TOUCH_LONG_PRESS_TIME == Approx(0.5f));
        REQUIRE(EditorConfig::TOUCH_DOUBLE_TAP_TIME == Approx(0.3f));
        REQUIRE(EditorConfig::TOUCH_PINCH_SENSITIVITY == Approx(1.0f));
        REQUIRE(EditorConfig::TOUCH_ROTATION_SENSITIVITY == Approx(1.0f));
    }
    
    SECTION("Performance budgets") {
        REQUIRE(EditorConfig::TARGET_FRAME_TIME_MS == Approx(16.67f));
        REQUIRE(EditorConfig::UI_UPDATE_BUDGET_MS == Approx(2.0f));
        REQUIRE(EditorConfig::GIZMO_BUDGET_MS == Approx(1.0f));
    }
}

TEST_CASE("Editor Types - EditorState Enum", "[editor][types]") {
    SECTION("All editor states are defined") {
        REQUIRE(static_cast<u8>(EditorState::Initializing) == 0);
        REQUIRE(static_cast<u8>(EditorState::Loading) == 1);
        REQUIRE(static_cast<u8>(EditorState::Ready) == 2);
        REQUIRE(static_cast<u8>(EditorState::Playing) == 3);
        REQUIRE(static_cast<u8>(EditorState::Paused) == 4);
        REQUIRE(static_cast<u8>(EditorState::Compiling) == 5);
        REQUIRE(static_cast<u8>(EditorState::Building) == 6);
        REQUIRE(static_cast<u8>(EditorState::Exporting) == 7);
        REQUIRE(static_cast<u8>(EditorState::Error) == 8);
    }
}

TEST_CASE("Editor Types - EditMode Enum", "[editor][types]") {
    SECTION("All edit modes are defined") {
        REQUIRE(static_cast<u8>(EditMode::Scene) == 0);
        REQUIRE(static_cast<u8>(EditMode::Prefab) == 1);
        REQUIRE(static_cast<u8>(EditMode::Animation) == 2);
        REQUIRE(static_cast<u8>(EditMode::Material) == 3);
        REQUIRE(static_cast<u8>(EditMode::Terrain) == 4);
        REQUIRE(static_cast<u8>(EditMode::ParticleSystem) == 5);
        REQUIRE(static_cast<u8>(EditMode::UICanvas) == 6);
        REQUIRE(static_cast<u8>(EditMode::Code) == 7);
    }
}

TEST_CASE("Editor Types - TransformTool Enum", "[editor][types]") {
    SECTION("Transform tools") {
        REQUIRE(static_cast<u8>(TransformTool::None) == 0);
        REQUIRE(static_cast<u8>(TransformTool::Move) == 1);
        REQUIRE(static_cast<u8>(TransformTool::Rotate) == 2);
        REQUIRE(static_cast<u8>(TransformTool::Scale) == 3);
        REQUIRE(static_cast<u8>(TransformTool::Rect) == 4);
        REQUIRE(static_cast<u8>(TransformTool::Combined) == 5);
    }
}

TEST_CASE("Editor Types - TransformSpace Enum", "[editor][types]") {
    SECTION("Transform spaces") {
        REQUIRE(static_cast<u8>(TransformSpace::World) == 0);
        REQUIRE(static_cast<u8>(TransformSpace::Local) == 1);
        REQUIRE(static_cast<u8>(TransformSpace::View) == 2);
        REQUIRE(static_cast<u8>(TransformSpace::Parent) == 3);
    }
}

TEST_CASE("Editor Types - PivotMode Enum", "[editor][types]") {
    SECTION("Pivot modes") {
        REQUIRE(static_cast<u8>(PivotMode::Center) == 0);
        REQUIRE(static_cast<u8>(PivotMode::Pivot) == 1);
        REQUIRE(static_cast<u8>(PivotMode::Active) == 2);
        REQUIRE(static_cast<u8>(PivotMode::Custom) == 3);
    }
}

TEST_CASE("Editor Types - ViewMode Enum", "[editor][types]") {
    SECTION("View modes") {
        REQUIRE(static_cast<u8>(ViewMode::Shaded) == 0);
        REQUIRE(static_cast<u8>(ViewMode::Wireframe) == 1);
        REQUIRE(static_cast<u8>(ViewMode::Unlit) == 2);
        REQUIRE(static_cast<u8>(ViewMode::Normals) == 3);
        REQUIRE(static_cast<u8>(ViewMode::UVChecker) == 4);
        REQUIRE(static_cast<u8>(ViewMode::Lightmap) == 5);
        REQUIRE(static_cast<u8>(ViewMode::Overdraw) == 6);
        REQUIRE(static_cast<u8>(ViewMode::Mipmap) == 7);
        REQUIRE(static_cast<u8>(ViewMode::DepthOnly) == 8);
        REQUIRE(static_cast<u8>(ViewMode::Albedo) == 9);
        REQUIRE(static_cast<u8>(ViewMode::Metallic) == 10);
        REQUIRE(static_cast<u8>(ViewMode::Roughness) == 11);
        REQUIRE(static_cast<u8>(ViewMode::AmbientOcclusion) == 12);
        REQUIRE(static_cast<u8>(ViewMode::Emission) == 13);
    }
}

TEST_CASE("Editor Types - ProjectionMode Enum", "[editor][types]") {
    SECTION("Projection modes") {
        REQUIRE(static_cast<u8>(ProjectionMode::Perspective) == 0);
        REQUIRE(static_cast<u8>(ProjectionMode::Orthographic) == 1);
    }
}

TEST_CASE("Editor Types - SnapMode Enum", "[editor][types]") {
    SECTION("Snap modes") {
        REQUIRE(static_cast<u8>(SnapMode::None) == 0);
        REQUIRE(static_cast<u8>(SnapMode::Grid) == 1);
        REQUIRE(static_cast<u8>(SnapMode::Surface) == 2);
        REQUIRE(static_cast<u8>(SnapMode::Vertex) == 3);
        REQUIRE(static_cast<u8>(SnapMode::Edge) == 4);
        REQUIRE(static_cast<u8>(SnapMode::Pivot) == 5);
    }
}

TEST_CASE("Editor Types - DockPosition Enum", "[editor][types]") {
    SECTION("Dock positions") {
        REQUIRE(static_cast<u8>(DockPosition::None) == 0);
        REQUIRE(static_cast<u8>(DockPosition::Left) == 1);
        REQUIRE(static_cast<u8>(DockPosition::Right) == 2);
        REQUIRE(static_cast<u8>(DockPosition::Top) == 3);
        REQUIRE(static_cast<u8>(DockPosition::Bottom) == 4);
        REQUIRE(static_cast<u8>(DockPosition::Center) == 5);
        REQUIRE(static_cast<u8>(DockPosition::TopLeft) == 6);
        REQUIRE(static_cast<u8>(DockPosition::TopRight) == 7);
    }
}

TEST_CASE("Editor Types - AssetType Enum", "[editor][types]") {
    SECTION("Asset types") {
        REQUIRE(static_cast<u8>(AssetType::Unknown) == 0);
        REQUIRE(static_cast<u8>(AssetType::Scene) == 1);
        REQUIRE(static_cast<u8>(AssetType::Prefab) == 2);
        REQUIRE(static_cast<u8>(AssetType::Material) == 3);
        REQUIRE(static_cast<u8>(AssetType::Texture) == 4);
        REQUIRE(static_cast<u8>(AssetType::Mesh) == 5);
        REQUIRE(static_cast<u8>(AssetType::Animation) == 6);
        REQUIRE(static_cast<u8>(AssetType::AnimationController) == 7);
        REQUIRE(static_cast<u8>(AssetType::Audio) == 8);
        REQUIRE(static_cast<u8>(AssetType::Script) == 9);
    }
}

TEST_CASE("Editor Types - TouchGesture Enum", "[editor][types]") {
    SECTION("Touch gestures") {
        REQUIRE(static_cast<u8>(TouchGesture::None) == 0);
        REQUIRE(static_cast<u8>(TouchGesture::Tap) == 1);
        REQUIRE(static_cast<u8>(TouchGesture::DoubleTap) == 2);
        REQUIRE(static_cast<u8>(TouchGesture::LongPress) == 3);
        REQUIRE(static_cast<u8>(TouchGesture::Drag) == 4);
        REQUIRE(static_cast<u8>(TouchGesture::TwoFingerDrag) == 5);
        REQUIRE(static_cast<u8>(TouchGesture::Pinch) == 6);
        REQUIRE(static_cast<u8>(TouchGesture::Rotate) == 7);
        REQUIRE(static_cast<u8>(TouchGesture::ThreeFingerSwipe) == 8);
    }
}
