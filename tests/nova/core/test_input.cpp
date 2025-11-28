/**
 * @file test_input.cpp
 * @brief Unit tests for NovaCore Input System
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/input/input.hpp>

using namespace nova;
using namespace nova::input;
using namespace nova::math;
using Catch::Approx;

// ============================================================================
// Input Types Tests
// ============================================================================

TEST_CASE("Input Types - Key Enum", "[input][types]") {
    SECTION("Letter keys") {
        REQUIRE(static_cast<u16>(Key::A) == 4);
        REQUIRE(static_cast<u16>(Key::Z) == 29);
    }
    
    SECTION("Number keys") {
        REQUIRE(static_cast<u16>(Key::Num1) == 30);
        REQUIRE(static_cast<u16>(Key::Num0) == 39);
    }
    
    SECTION("Function keys") {
        REQUIRE(static_cast<u16>(Key::F1) == 58);
        REQUIRE(static_cast<u16>(Key::F12) == 69);
    }
    
    SECTION("Special keys") {
        REQUIRE(static_cast<u16>(Key::Return) == 40);
        REQUIRE(static_cast<u16>(Key::Escape) == 41);
        REQUIRE(static_cast<u16>(Key::Space) == 44);
    }
}

TEST_CASE("Input Types - MouseButton Enum", "[input][types]") {
    SECTION("All mouse buttons are defined") {
        REQUIRE(static_cast<u8>(MouseButton::Left) == 0);
        REQUIRE(static_cast<u8>(MouseButton::Right) == 1);
        REQUIRE(static_cast<u8>(MouseButton::Middle) == 2);
        REQUIRE(static_cast<u8>(MouseButton::Button4) == 3);
        REQUIRE(static_cast<u8>(MouseButton::Button5) == 4);
    }
}

TEST_CASE("Input Types - TouchPhase Enum", "[input][types]") {
    SECTION("All touch phases are defined") {
        REQUIRE(static_cast<u8>(TouchPhase::Began) == 0);
        REQUIRE(static_cast<u8>(TouchPhase::Moved) == 1);
        REQUIRE(static_cast<u8>(TouchPhase::Stationary) == 2);
        REQUIRE(static_cast<u8>(TouchPhase::Ended) == 3);
        REQUIRE(static_cast<u8>(TouchPhase::Cancelled) == 4);
    }
}

TEST_CASE("Input Types - GestureType Enum", "[input][types]") {
    SECTION("All gesture types are defined") {
        REQUIRE(static_cast<u8>(GestureType::None) == 0);
        REQUIRE(static_cast<u8>(GestureType::Tap) == 1);
        REQUIRE(static_cast<u8>(GestureType::DoubleTap) == 2);
        REQUIRE(static_cast<u8>(GestureType::LongPress) == 3);
        REQUIRE(static_cast<u8>(GestureType::Pan) == 4);
        REQUIRE(static_cast<u8>(GestureType::Pinch) == 5);
        REQUIRE(static_cast<u8>(GestureType::Rotate) == 6);
        REQUIRE(static_cast<u8>(GestureType::Swipe) == 7);
    }
}

TEST_CASE("Input Types - InputConfig", "[input][types]") {
    SECTION("Touch configuration") {
        REQUIRE(InputConfig::MAX_TOUCH_POINTS == 10);
        REQUIRE(InputConfig::TOUCH_TAP_MAX_DURATION == Approx(0.3f));
        REQUIRE(InputConfig::TOUCH_LONG_PRESS_DURATION == Approx(0.5f));
        REQUIRE(InputConfig::TOUCH_DOUBLE_TAP_INTERVAL == Approx(0.3f));
        REQUIRE(InputConfig::TOUCH_DRAG_THRESHOLD == Approx(10.0f));
        REQUIRE(InputConfig::TOUCH_SWIPE_MIN_VELOCITY == Approx(500.0f));
    }
    
    SECTION("Gamepad configuration") {
        REQUIRE(InputConfig::MAX_GAMEPADS == 4);
        REQUIRE(InputConfig::GAMEPAD_DEADZONE == Approx(0.15f));
        REQUIRE(InputConfig::GAMEPAD_TRIGGER_THRESHOLD == Approx(0.1f));
    }
}

TEST_CASE("Input Types - CursorMode Enum", "[input][types]") {
    SECTION("Cursor modes") {
        REQUIRE(static_cast<u8>(CursorMode::Normal) == 0);
        REQUIRE(static_cast<u8>(CursorMode::Hidden) == 1);
        REQUIRE(static_cast<u8>(CursorMode::Disabled) == 2);
        REQUIRE(static_cast<u8>(CursorMode::Captured) == 3);
    }
}

TEST_CASE("Input Types - TouchPoint", "[input][types]") {
    SECTION("Default touch point") {
        TouchPoint touch;
        REQUIRE(touch.id == 0);
    }
    
    SECTION("Touch point with values") {
        TouchPoint touch;
        touch.id = 1;
        touch.position = Vec2(100.0f, 200.0f);
        touch.phase = TouchPhase::Moved;
        touch.pressure = 0.8f;
        
        REQUIRE(touch.id == 1);
        REQUIRE(touch.position.x == Approx(100.0f));
        REQUIRE(touch.position.y == Approx(200.0f));
        REQUIRE(touch.phase == TouchPhase::Moved);
        REQUIRE(touch.pressure == Approx(0.8f));
    }
}
