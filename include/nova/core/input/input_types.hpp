/**
 * @file input_types.hpp
 * @brief NovaCore Input System™ - Core Type Definitions
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 * 
 * Defines the fundamental types, enums, and structures for the input system.
 * Supports keyboard, mouse, touch, gamepad, and motion input across all platforms.
 * Mobile-first design with comprehensive touch gesture support.
 */

#pragma once

#include <nova/core/types/types.hpp>
#include <nova/core/math/math.hpp>

#include <string>
#include <vector>
#include <array>
#include <functional>
#include <chrono>

namespace nova::input {

using namespace nova;
using namespace nova::math;

// ============================================================================
// Configuration Constants
// ============================================================================

namespace InputConfig {
    // Touch settings
    constexpr u32 MAX_TOUCH_POINTS = 10;
    constexpr f32 TOUCH_TAP_MAX_DURATION = 0.3f;      // Seconds
    constexpr f32 TOUCH_LONG_PRESS_DURATION = 0.5f;   // Seconds
    constexpr f32 TOUCH_DOUBLE_TAP_INTERVAL = 0.3f;   // Seconds
    constexpr f32 TOUCH_DRAG_THRESHOLD = 10.0f;       // Pixels
    constexpr f32 TOUCH_SWIPE_MIN_VELOCITY = 500.0f;  // Pixels/second
    
    // Gamepad settings
    constexpr u32 MAX_GAMEPADS = 4;
    constexpr f32 GAMEPAD_DEADZONE = 0.15f;
    constexpr f32 GAMEPAD_TRIGGER_THRESHOLD = 0.1f;
    
    // Input buffering
    constexpr u32 INPUT_BUFFER_FRAMES = 10;
    constexpr f32 INPUT_BUFFER_TIME = 0.166f;  // ~10 frames at 60 FPS
    
    // Acceleration
    constexpr f32 MOUSE_ACCELERATION = 1.0f;
    constexpr f32 STICK_ACCELERATION = 1.0f;
}

// ============================================================================
// Keyboard Input
// ============================================================================

/**
 * @brief Keyboard key codes
 * 
 * Based on USB HID keyboard scancodes for cross-platform compatibility.
 */
enum class Key : u16 {
    Unknown = 0,
    
    // Letters
    A = 4, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    
    // Numbers
    Num1 = 30, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num0,
    
    // Function keys
    F1 = 58, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24,
    
    // Editing keys
    Return = 40,
    Escape = 41,
    Backspace = 42,
    Tab = 43,
    Space = 44,
    
    // Punctuation
    Minus = 45,
    Equals = 46,
    LeftBracket = 47,
    RightBracket = 48,
    Backslash = 49,
    Semicolon = 51,
    Apostrophe = 52,
    Grave = 53,
    Comma = 54,
    Period = 55,
    Slash = 56,
    
    // Lock keys
    CapsLock = 57,
    ScrollLock = 71,
    NumLock = 83,
    
    // Navigation
    PrintScreen = 70,
    Pause = 72,
    Insert = 73,
    Home = 74,
    PageUp = 75,
    Delete = 76,
    End = 77,
    PageDown = 78,
    Right = 79,
    Left = 80,
    Down = 81,
    Up = 82,
    
    // Numpad
    NumpadDivide = 84,
    NumpadMultiply = 85,
    NumpadMinus = 86,
    NumpadPlus = 87,
    NumpadEnter = 88,
    Numpad1 = 89, Numpad2, Numpad3, Numpad4, Numpad5,
    Numpad6, Numpad7, Numpad8, Numpad9, Numpad0,
    NumpadPeriod = 99,
    NumpadEquals = 103,
    
    // Modifier keys
    LeftControl = 224,
    LeftShift = 225,
    LeftAlt = 226,
    LeftSuper = 227,
    RightControl = 228,
    RightShift = 229,
    RightAlt = 230,
    RightSuper = 231,
    
    // Media keys
    Mute = 127,
    VolumeUp = 128,
    VolumeDown = 129,
    
    // Special
    Menu = 118,
    
    Count = 256
};

/**
 * @brief Modifier key flags
 */
enum class KeyMod : u8 {
    None = 0,
    Shift = 1 << 0,
    Control = 1 << 1,
    Alt = 1 << 2,
    Super = 1 << 3,     // Windows/Cmd key
    CapsLock = 1 << 4,
    NumLock = 1 << 5
};

inline KeyMod operator|(KeyMod a, KeyMod b) {
    return static_cast<KeyMod>(static_cast<u8>(a) | static_cast<u8>(b));
}

inline KeyMod operator&(KeyMod a, KeyMod b) {
    return static_cast<KeyMod>(static_cast<u8>(a) & static_cast<u8>(b));
}

inline bool hasModifier(KeyMod mods, KeyMod test) {
    return (static_cast<u8>(mods) & static_cast<u8>(test)) != 0;
}

// ============================================================================
// Mouse Input
// ============================================================================

/**
 * @brief Mouse button codes
 */
enum class MouseButton : u8 {
    Left = 0,
    Right = 1,
    Middle = 2,
    Button4 = 3,
    Button5 = 4,
    Button6 = 5,
    Button7 = 6,
    Button8 = 7,
    Count = 8
};

/**
 * @brief Mouse cursor mode
 */
enum class CursorMode : u8 {
    Normal,         // Cursor visible and moves freely
    Hidden,         // Cursor hidden but moves freely
    Disabled,       // Cursor hidden and locked to window
    Captured        // Cursor captured by window
};

/**
 * @brief Standard cursor shapes
 */
enum class CursorShape : u8 {
    Arrow,
    IBeam,
    Crosshair,
    Hand,
    ResizeEW,       // East-West resize
    ResizeNS,       // North-South resize
    ResizeNESW,     // Diagonal resize
    ResizeNWSE,     // Diagonal resize
    ResizeAll,      // Move cursor
    NotAllowed,     // Not allowed cursor
    Wait,           // Loading/wait cursor
    Custom          // Custom cursor image
};

// ============================================================================
// Touch Input
// ============================================================================

/**
 * @brief Touch point phase
 */
enum class TouchPhase : u8 {
    Began,          // Finger touched screen
    Moved,          // Finger moved on screen
    Stationary,     // Finger is touching but hasn't moved
    Ended,          // Finger lifted from screen
    Cancelled       // Touch was interrupted
};

/**
 * @brief Recognized touch gestures
 */
enum class GestureType : u8 {
    None,
    Tap,            // Quick tap
    DoubleTap,      // Two quick taps
    LongPress,      // Press and hold
    Pan,            // Single finger drag
    Pinch,          // Two finger pinch (scale)
    Rotate,         // Two finger rotation
    Swipe,          // Quick directional movement
    Edge,           // Swipe from screen edge
    ThreeFingerPan, // Three finger drag
    FourFingerPinch // Four finger gesture
};

/**
 * @brief Swipe direction
 */
enum class SwipeDirection : u8 {
    None,
    Left,
    Right,
    Up,
    Down
};

/**
 * @brief Single touch point data
 */
struct TouchPoint {
    u32 id = 0;                    // Unique touch identifier
    TouchPhase phase = TouchPhase::Began;
    Vec2 position;                 // Current position in screen coordinates
    Vec2 previousPosition;         // Position last frame
    Vec2 startPosition;            // Position when touch began
    Vec2 delta;                    // Movement since last frame
    f32 pressure = 1.0f;           // Pressure (0.0 - 1.0, if supported)
    f32 radius = 1.0f;             // Touch radius in pixels
    f32 startTime = 0.0f;          // Time when touch began
    f32 timestamp = 0.0f;          // Current timestamp
    
    Vec2 velocity() const;         // Calculate velocity
    f32 duration() const;          // Time since touch began
    f32 distance() const;          // Distance from start
    bool isTap() const;            // Quick tap detection
    bool isLongPress() const;      // Long press detection
};

/**
 * @brief Gesture recognition result
 */
struct GestureData {
    GestureType type = GestureType::None;
    Vec2 position;                 // Center position of gesture
    Vec2 delta;                    // Movement delta
    f32 scale = 1.0f;              // Pinch scale factor
    f32 rotation = 0.0f;           // Rotation in radians
    f32 velocity = 0.0f;           // Movement velocity
    SwipeDirection swipeDir = SwipeDirection::None;
    u32 touchCount = 0;            // Number of touches in gesture
    bool isActive = false;         // Gesture is in progress
    bool isComplete = false;       // Gesture just completed
};

// ============================================================================
// Gamepad Input
// ============================================================================

/**
 * @brief Gamepad button codes
 */
enum class GamepadButton : u8 {
    // Face buttons
    A = 0,          // Cross (PlayStation), A (Xbox/Nintendo)
    B = 1,          // Circle (PlayStation), B (Xbox/Nintendo)
    X = 2,          // Square (PlayStation), X (Xbox), Y (Nintendo)
    Y = 3,          // Triangle (PlayStation), Y (Xbox), X (Nintendo)
    
    // Shoulder buttons
    LeftBumper = 4,
    RightBumper = 5,
    
    // Menu buttons
    Back = 6,       // Share/Select
    Start = 7,      // Options/Start
    Guide = 8,      // PS/Xbox/Home button
    
    // Stick buttons
    LeftStick = 9,
    RightStick = 10,
    
    // D-pad
    DPadUp = 11,
    DPadDown = 12,
    DPadLeft = 13,
    DPadRight = 14,
    
    // Extra buttons
    Misc = 15,      // Touchpad click, etc.
    
    Count = 16
};

/**
 * @brief Gamepad axis codes
 */
enum class GamepadAxis : u8 {
    LeftX = 0,      // Left stick X (-1 to 1)
    LeftY = 1,      // Left stick Y (-1 to 1)
    RightX = 2,     // Right stick X (-1 to 1)
    RightY = 3,     // Right stick Y (-1 to 1)
    LeftTrigger = 4,  // Left trigger (0 to 1)
    RightTrigger = 5, // Right trigger (0 to 1)
    Count = 6
};

/**
 * @brief Gamepad type/brand
 */
enum class GamepadType : u8 {
    Unknown,
    Xbox,
    PlayStation,
    Nintendo,
    Generic
};

/**
 * @brief Gamepad state
 */
struct GamepadState {
    bool connected = false;
    GamepadType type = GamepadType::Unknown;
    std::string name;
    
    // Button states (current frame)
    std::array<bool, static_cast<usize>(GamepadButton::Count)> buttons{};
    
    // Axis values (-1 to 1 for sticks, 0 to 1 for triggers)
    std::array<f32, static_cast<usize>(GamepadAxis::Count)> axes{};
    
    // Rumble state
    f32 rumbleLow = 0.0f;   // Low frequency motor (0-1)
    f32 rumbleHigh = 0.0f;  // High frequency motor (0-1)
    
    // Helpers
    Vec2 leftStick() const { return Vec2(axes[0], axes[1]); }
    Vec2 rightStick() const { return Vec2(axes[2], axes[3]); }
    f32 leftTrigger() const { return axes[4]; }
    f32 rightTrigger() const { return axes[5]; }
    
    bool isButtonDown(GamepadButton btn) const {
        return buttons[static_cast<usize>(btn)];
    }
};

// ============================================================================
// Motion/Sensor Input
// ============================================================================

/**
 * @brief Motion sensor type
 */
enum class MotionSensor : u8 {
    Accelerometer,  // Device acceleration
    Gyroscope,      // Device rotation
    Magnetometer,   // Compass heading
    Gravity,        // Gravity vector
    LinearAccel,    // Acceleration without gravity
    Rotation        // Combined rotation (quaternion)
};

/**
 * @brief Motion sensor data
 */
struct MotionData {
    Vec3 acceleration;      // Accelerometer (m/s²)
    Vec3 gravity;           // Gravity vector (m/s²)
    Vec3 linearAcceleration;// Acceleration minus gravity
    Vec3 gyroscope;         // Angular velocity (rad/s)
    Vec3 magnetometer;      // Magnetic field (µT)
    Quat rotation;          // Device orientation
    f32 timestamp = 0.0f;
    
    // Derived values
    f32 pitch() const;      // Pitch angle in radians
    f32 roll() const;       // Roll angle in radians
    f32 yaw() const;        // Yaw angle in radians
    f32 compassHeading() const; // Magnetic north heading
};

// ============================================================================
// Input Events
// ============================================================================

/**
 * @brief Input event type
 */
enum class InputEventType : u8 {
    // Keyboard
    KeyDown,
    KeyUp,
    KeyRepeat,
    TextInput,
    
    // Mouse
    MouseMove,
    MouseButtonDown,
    MouseButtonUp,
    MouseScroll,
    MouseEnter,
    MouseLeave,
    
    // Touch
    TouchBegin,
    TouchMove,
    TouchEnd,
    TouchCancel,
    Gesture,
    
    // Gamepad
    GamepadConnect,
    GamepadDisconnect,
    GamepadButtonDown,
    GamepadButtonUp,
    GamepadAxisMove,
    
    // Motion
    MotionUpdate,
    
    // Window
    FocusGained,
    FocusLost
};

/**
 * @brief Keyboard event data
 */
struct KeyEvent {
    Key key = Key::Unknown;
    KeyMod modifiers = KeyMod::None;
    u32 scancode = 0;
    bool isRepeat = false;
};

/**
 * @brief Text input event data
 */
struct TextEvent {
    u32 codepoint = 0;        // Unicode codepoint
    std::string text;         // UTF-8 text
};

/**
 * @brief Mouse event data
 */
struct MouseEvent {
    MouseButton button = MouseButton::Left;
    Vec2 position;            // Current position
    Vec2 delta;               // Movement delta
    Vec2 scroll;              // Scroll amount (x=horizontal, y=vertical)
    KeyMod modifiers = KeyMod::None;
};

/**
 * @brief Touch event data
 */
struct TouchEvent {
    TouchPoint touch;
    u32 touchCount = 0;       // Total active touches
};

/**
 * @brief Gamepad event data
 */
struct GamepadEvent {
    u32 gamepadIndex = 0;
    GamepadButton button = GamepadButton::A;
    GamepadAxis axis = GamepadAxis::LeftX;
    f32 axisValue = 0.0f;
};

/**
 * @brief Unified input event
 */
struct InputEvent {
    InputEventType type = InputEventType::KeyDown;
    f32 timestamp = 0.0f;
    
    union {
        KeyEvent key;
        TextEvent text;
        MouseEvent mouse;
        TouchEvent touch;
        GamepadEvent gamepad;
        GestureData gesture;
        MotionData motion;
    };
    
    InputEvent() : key{} {}
    ~InputEvent() {}
    
    InputEvent(const InputEvent& other);
    InputEvent& operator=(const InputEvent& other);
};

// ============================================================================
// Input Action Mapping
// ============================================================================

/**
 * @brief Input binding type
 */
enum class BindingType : u8 {
    Key,            // Keyboard key
    MouseButton,    // Mouse button
    MouseAxis,      // Mouse movement
    GamepadButton,  // Gamepad button
    GamepadAxis,    // Gamepad stick/trigger
    Touch,          // Touch input
    Gesture         // Touch gesture
};

/**
 * @brief Single input binding
 */
struct InputBinding {
    BindingType type = BindingType::Key;
    u16 code = 0;           // Key/button code
    f32 scale = 1.0f;       // Value multiplier
    bool inverted = false;  // Invert axis
    KeyMod modifiers = KeyMod::None;  // Required modifiers
    
    static InputBinding key(Key k, KeyMod mods = KeyMod::None);
    static InputBinding mouseButton(MouseButton btn);
    static InputBinding mouseAxisX(f32 scale = 1.0f);
    static InputBinding mouseAxisY(f32 scale = 1.0f);
    static InputBinding gamepadButton(GamepadButton btn);
    static InputBinding gamepadAxis(GamepadAxis axis, f32 scale = 1.0f);
    static InputBinding gesture(GestureType type);
};

/**
 * @brief Input action (named action with multiple bindings)
 */
struct InputAction {
    std::string name;
    std::vector<InputBinding> bindings;
    bool consumeInput = true;
    
    void addBinding(const InputBinding& binding);
    void removeBinding(usize index);
    void clearBindings();
};

/**
 * @brief Input axis (continuous value from -1 to 1)
 */
struct InputAxis {
    std::string name;
    std::vector<InputBinding> positiveBindings;  // Adds to axis value
    std::vector<InputBinding> negativeBindings;  // Subtracts from axis value
    f32 gravity = 3.0f;     // Speed axis returns to 0
    f32 sensitivity = 1.0f; // Multiplier
    f32 deadzone = 0.1f;    // Ignore values below this
    bool snap = true;       // Snap to 0 when changing direction
    
    void addPositive(const InputBinding& binding);
    void addNegative(const InputBinding& binding);
};

// ============================================================================
// Callback Types
// ============================================================================

using InputEventCallback = std::function<bool(const InputEvent&)>;
using ActionCallback = std::function<void(const std::string& action, bool pressed)>;
using AxisCallback = std::function<void(const std::string& axis, f32 value)>;
using GestureCallback = std::function<void(const GestureData&)>;

// ============================================================================
// Inline Implementations
// ============================================================================

inline Vec2 TouchPoint::velocity() const {
    f32 dt = timestamp - startTime;
    if (dt <= 0.0f) return Vec2::zero();
    return (position - startPosition) / dt;
}

inline f32 TouchPoint::duration() const {
    return timestamp - startTime;
}

inline f32 TouchPoint::distance() const {
    return (position - startPosition).length();
}

inline bool TouchPoint::isTap() const {
    return phase == TouchPhase::Ended &&
           duration() < InputConfig::TOUCH_TAP_MAX_DURATION &&
           distance() < InputConfig::TOUCH_DRAG_THRESHOLD;
}

inline bool TouchPoint::isLongPress() const {
    return duration() >= InputConfig::TOUCH_LONG_PRESS_DURATION &&
           distance() < InputConfig::TOUCH_DRAG_THRESHOLD;
}

inline f32 MotionData::pitch() const {
    return rotation.toEuler().x;
}

inline f32 MotionData::roll() const {
    return rotation.toEuler().z;
}

inline f32 MotionData::yaw() const {
    return rotation.toEuler().y;
}

inline InputBinding InputBinding::key(Key k, KeyMod mods) {
    InputBinding b;
    b.type = BindingType::Key;
    b.code = static_cast<u16>(k);
    b.modifiers = mods;
    return b;
}

inline InputBinding InputBinding::mouseButton(MouseButton btn) {
    InputBinding b;
    b.type = BindingType::MouseButton;
    b.code = static_cast<u16>(btn);
    return b;
}

inline InputBinding InputBinding::gamepadButton(GamepadButton btn) {
    InputBinding b;
    b.type = BindingType::GamepadButton;
    b.code = static_cast<u16>(btn);
    return b;
}

inline InputBinding InputBinding::gamepadAxis(GamepadAxis axis, f32 scale) {
    InputBinding b;
    b.type = BindingType::GamepadAxis;
    b.code = static_cast<u16>(axis);
    b.scale = scale;
    return b;
}

inline void InputAction::addBinding(const InputBinding& binding) {
    bindings.push_back(binding);
}

inline void InputAction::removeBinding(usize index) {
    if (index < bindings.size()) {
        bindings.erase(bindings.begin() + static_cast<ptrdiff_t>(index));
    }
}

inline void InputAction::clearBindings() {
    bindings.clear();
}

inline void InputAxis::addPositive(const InputBinding& binding) {
    positiveBindings.push_back(binding);
}

inline void InputAxis::addNegative(const InputBinding& binding) {
    negativeBindings.push_back(binding);
}

} // namespace nova::input
