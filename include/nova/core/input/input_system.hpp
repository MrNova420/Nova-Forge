/**
 * @file input_system.hpp
 * @brief NovaCore Input System™ - Main Input Manager
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 * 
 * Central input management system handling all input sources:
 * - Keyboard and mouse input
 * - Multi-touch with gesture recognition
 * - Gamepad with rumble support
 * - Motion sensors (accelerometer, gyroscope)
 * - Action mapping and input buffering
 */

#pragma once

#include "input_types.hpp"

#include <memory>
#include <unordered_map>
#include <queue>
#include <optional>

namespace nova::input {

// ============================================================================
// Forward Declarations
// ============================================================================

class GestureRecognizer;
class InputMapper;

// ============================================================================
// Input System
// ============================================================================

/**
 * @brief Central input management system
 * 
 * The InputSystem is the main interface for all input handling in NovaCore.
 * It provides:
 * - Raw input state queries
 * - Event-based input handling
 * - Action/axis mapping for game logic
 * - Touch gesture recognition
 * - Input buffering for fighting games
 * - Platform-agnostic interface
 * 
 * Usage:
 * @code
 *     auto& input = InputSystem::get();
 *     
 *     // Raw input
 *     if (input.isKeyDown(Key::Space)) {
 *         player.jump();
 *     }
 *     
 *     // Action mapping
 *     input.registerAction("Jump", InputBinding::key(Key::Space));
 *     input.registerAction("Jump", InputBinding::gamepadButton(GamepadButton::A));
 *     
 *     if (input.isActionPressed("Jump")) {
 *         player.jump();
 *     }
 * @endcode
 */
class InputSystem {
public:
    /**
     * @brief Get singleton instance
     */
    static InputSystem& get();
    
    /**
     * @brief Initialize the input system
     * @return true if initialization succeeded
     */
    bool initialize();
    
    /**
     * @brief Shutdown the input system
     */
    void shutdown();
    
    /**
     * @brief Update input state (call at start of frame)
     */
    void update();
    
    /**
     * @brief Process an input event from the platform layer
     */
    void processEvent(const InputEvent& event);
    
    /**
     * @brief Clear all input state (e.g., when window loses focus)
     */
    void clearState();
    
    // ========================================================================
    // Keyboard Input
    // ========================================================================
    
    /** @brief Check if key is currently held down */
    bool isKeyDown(Key key) const;
    
    /** @brief Check if key was just pressed this frame */
    bool isKeyPressed(Key key) const;
    
    /** @brief Check if key was just released this frame */
    bool isKeyReleased(Key key) const;
    
    /** @brief Check if any key is currently held */
    bool isAnyKeyDown() const;
    
    /** @brief Get current modifier keys */
    KeyMod getModifiers() const { return m_currentModifiers; }
    
    /** @brief Check if modifier is active */
    bool hasModifier(KeyMod mod) const {
        return input::hasModifier(m_currentModifiers, mod);
    }
    
    // ========================================================================
    // Mouse Input
    // ========================================================================
    
    /** @brief Get current mouse position in screen coordinates */
    Vec2 getMousePosition() const { return m_mousePosition; }
    
    /** @brief Get mouse movement since last frame */
    Vec2 getMouseDelta() const { return m_mouseDelta; }
    
    /** @brief Get mouse scroll delta (x=horizontal, y=vertical) */
    Vec2 getMouseScroll() const { return m_mouseScroll; }
    
    /** @brief Check if mouse button is held down */
    bool isMouseButtonDown(MouseButton button) const;
    
    /** @brief Check if mouse button was just pressed */
    bool isMouseButtonPressed(MouseButton button) const;
    
    /** @brief Check if mouse button was just released */
    bool isMouseButtonReleased(MouseButton button) const;
    
    /** @brief Set cursor mode */
    void setCursorMode(CursorMode mode);
    
    /** @brief Get current cursor mode */
    CursorMode getCursorMode() const { return m_cursorMode; }
    
    /** @brief Set cursor shape */
    void setCursorShape(CursorShape shape);
    
    /** @brief Set cursor position (if supported) */
    void setCursorPosition(const Vec2& position);
    
    // ========================================================================
    // Touch Input
    // ========================================================================
    
    /** @brief Get number of active touch points */
    u32 getTouchCount() const;
    
    /** @brief Get touch point by index */
    std::optional<TouchPoint> getTouch(u32 index) const;
    
    /** @brief Get touch point by ID */
    std::optional<TouchPoint> getTouchById(u32 id) const;
    
    /** @brief Get all active touch points */
    const std::vector<TouchPoint>& getTouches() const { return m_touches; }
    
    /** @brief Check if screen is being touched */
    bool isTouching() const { return !m_touches.empty(); }
    
    /** @brief Get current recognized gesture */
    const GestureData& getCurrentGesture() const { return m_currentGesture; }
    
    /** @brief Check if a specific gesture type is active */
    bool isGestureActive(GestureType type) const {
        return m_currentGesture.type == type && m_currentGesture.isActive;
    }
    
    // ========================================================================
    // Gamepad Input
    // ========================================================================
    
    /** @brief Get number of connected gamepads */
    u32 getGamepadCount() const;
    
    /** @brief Check if gamepad is connected */
    bool isGamepadConnected(u32 index) const;
    
    /** @brief Get gamepad state */
    const GamepadState& getGamepad(u32 index) const;
    
    /** @brief Check if gamepad button is held */
    bool isGamepadButtonDown(u32 gamepad, GamepadButton button) const;
    
    /** @brief Check if gamepad button was just pressed */
    bool isGamepadButtonPressed(u32 gamepad, GamepadButton button) const;
    
    /** @brief Check if gamepad button was just released */
    bool isGamepadButtonReleased(u32 gamepad, GamepadButton button) const;
    
    /** @brief Get gamepad axis value (-1 to 1 for sticks, 0 to 1 for triggers) */
    f32 getGamepadAxis(u32 gamepad, GamepadAxis axis) const;
    
    /** @brief Get left stick as normalized vector */
    Vec2 getGamepadLeftStick(u32 gamepad) const;
    
    /** @brief Get right stick as normalized vector */
    Vec2 getGamepadRightStick(u32 gamepad) const;
    
    /** @brief Set gamepad rumble (0-1 for each motor) */
    void setGamepadRumble(u32 gamepad, f32 lowFreq, f32 highFreq, f32 duration = 0.0f);
    
    /** @brief Stop all rumble on gamepad */
    void stopGamepadRumble(u32 gamepad);
    
    // ========================================================================
    // Motion Sensors
    // ========================================================================
    
    /** @brief Check if motion sensors are available */
    bool hasMotionSensors() const { return m_hasMotionSensors; }
    
    /** @brief Enable/disable motion sensors */
    void setMotionSensorsEnabled(bool enabled);
    
    /** @brief Get current motion data */
    const MotionData& getMotionData() const { return m_motionData; }
    
    /** @brief Get device tilt as normalized vector */
    Vec2 getTilt() const;
    
    /** @brief Get shake intensity (0-1) */
    f32 getShakeIntensity() const;
    
    // ========================================================================
    // Action Mapping
    // ========================================================================
    
    /** @brief Register an action with a single binding */
    void registerAction(const std::string& name, const InputBinding& binding);
    
    /** @brief Register an action with multiple bindings */
    void registerAction(const std::string& name, const std::vector<InputBinding>& bindings);
    
    /** @brief Unregister an action */
    void unregisterAction(const std::string& name);
    
    /** @brief Check if action exists */
    bool hasAction(const std::string& name) const;
    
    /** @brief Check if action is currently active (button held) */
    bool isActionDown(const std::string& name) const;
    
    /** @brief Check if action was just triggered (button pressed) */
    bool isActionPressed(const std::string& name) const;
    
    /** @brief Check if action was just released */
    bool isActionReleased(const std::string& name) const;
    
    /** @brief Get action value (0 or 1 for buttons, analog for axes) */
    f32 getActionValue(const std::string& name) const;
    
    // ========================================================================
    // Axis Mapping
    // ========================================================================
    
    /** @brief Register an axis */
    void registerAxis(const std::string& name, const InputAxis& axis);
    
    /** @brief Quick register axis with positive/negative keys */
    void registerAxis(const std::string& name, Key positive, Key negative);
    
    /** @brief Unregister an axis */
    void unregisterAxis(const std::string& name);
    
    /** @brief Get axis value (-1 to 1) */
    f32 getAxis(const std::string& name) const;
    
    /** @brief Get raw axis value without smoothing */
    f32 getAxisRaw(const std::string& name) const;
    
    // ========================================================================
    // Input Buffering
    // ========================================================================
    
    /** @brief Check if action was pressed within buffer window */
    bool isActionBuffered(const std::string& name) const;
    
    /** @brief Consume buffered action (removes from buffer) */
    bool consumeBufferedAction(const std::string& name);
    
    /** @brief Clear input buffer */
    void clearBuffer();
    
    /** @brief Set buffer window in seconds */
    void setBufferWindow(f32 seconds);
    
    // ========================================================================
    // Event Callbacks
    // ========================================================================
    
    /** @brief Add input event callback (called for all events) */
    void addEventCallback(InputEventCallback callback);
    
    /** @brief Add action callback */
    void addActionCallback(ActionCallback callback);
    
    /** @brief Add axis callback */
    void addAxisCallback(AxisCallback callback);
    
    /** @brief Add gesture callback */
    void addGestureCallback(GestureCallback callback);
    
    /** @brief Remove all callbacks */
    void clearCallbacks();
    
    // ========================================================================
    // Text Input
    // ========================================================================
    
    /** @brief Start text input mode (shows virtual keyboard on mobile) */
    void startTextInput();
    
    /** @brief Stop text input mode */
    void stopTextInput();
    
    /** @brief Check if text input is active */
    bool isTextInputActive() const { return m_textInputActive; }
    
    /** @brief Set text input rectangle (for virtual keyboard positioning) */
    void setTextInputRect(const Vec2& position, const Vec2& size);
    
    /** @brief Get text entered this frame */
    const std::string& getTextInput() const { return m_textInput; }
    
    // ========================================================================
    // Platform Integration
    // ========================================================================
    
    /** @brief Set the platform window handle */
    void setWindowHandle(void* handle);
    
    /** @brief Get clipboard text */
    std::string getClipboardText() const;
    
    /** @brief Set clipboard text */
    void setClipboardText(const std::string& text);
    
    // ========================================================================
    // Utilities
    // ========================================================================
    
    /** @brief Get key name for display */
    static std::string getKeyName(Key key);
    
    /** @brief Get mouse button name for display */
    static std::string getMouseButtonName(MouseButton button);
    
    /** @brief Get gamepad button name for display */
    static std::string getGamepadButtonName(GamepadButton button, GamepadType type = GamepadType::Generic);
    
private:
    InputSystem() = default;
    ~InputSystem() = default;
    InputSystem(const InputSystem&) = delete;
    InputSystem& operator=(const InputSystem&) = delete;
    
    // Event processing
    void processKeyEvent(const KeyEvent& event, InputEventType type);
    void processMouseEvent(const MouseEvent& event, InputEventType type);
    void processTouchEvent(const TouchEvent& event, InputEventType type);
    void processGamepadEvent(const GamepadEvent& event, InputEventType type);
    void processMotionEvent(const MotionData& motion);
    
    // State update
    void updatePreviousState();
    void updateGestures(f32 deltaTime);
    void updateAxes(f32 deltaTime);
    void updateBuffer(f32 deltaTime);
    void updateRumble(f32 deltaTime);
    
    // Action/axis evaluation
    f32 evaluateBinding(const InputBinding& binding) const;
    
    // Keyboard state
    std::array<bool, static_cast<usize>(Key::Count)> m_keyState{};
    std::array<bool, static_cast<usize>(Key::Count)> m_previousKeyState{};
    KeyMod m_currentModifiers = KeyMod::None;
    
    // Mouse state
    Vec2 m_mousePosition;
    Vec2 m_previousMousePosition;
    Vec2 m_mouseDelta;
    Vec2 m_mouseScroll;
    std::array<bool, static_cast<usize>(MouseButton::Count)> m_mouseButtonState{};
    std::array<bool, static_cast<usize>(MouseButton::Count)> m_previousMouseButtonState{};
    CursorMode m_cursorMode = CursorMode::Normal;
    CursorShape m_cursorShape = CursorShape::Arrow;
    
    // Touch state
    std::vector<TouchPoint> m_touches;
    GestureData m_currentGesture;
    std::unique_ptr<GestureRecognizer> m_gestureRecognizer;
    
    // Gamepad state
    std::array<GamepadState, InputConfig::MAX_GAMEPADS> m_gamepads{};
    std::array<GamepadState, InputConfig::MAX_GAMEPADS> m_previousGamepadState{};
    
    // Rumble tracking
    struct RumbleState {
        f32 lowFreq = 0.0f;
        f32 highFreq = 0.0f;
        f32 remaining = 0.0f;
    };
    std::array<RumbleState, InputConfig::MAX_GAMEPADS> m_rumbleState{};
    
    // Motion sensors
    bool m_hasMotionSensors = false;
    bool m_motionSensorsEnabled = false;
    MotionData m_motionData;
    
    // Action mapping
    std::unordered_map<std::string, InputAction> m_actions;
    std::unordered_map<std::string, bool> m_actionState;
    std::unordered_map<std::string, bool> m_previousActionState;
    
    // Axis mapping
    std::unordered_map<std::string, InputAxis> m_axes;
    std::unordered_map<std::string, f32> m_axisValues;
    std::unordered_map<std::string, f32> m_axisRawValues;
    
    // Input buffering
    struct BufferedInput {
        std::string action;
        f32 timestamp;
    };
    std::queue<BufferedInput> m_inputBuffer;
    f32 m_bufferWindow = InputConfig::INPUT_BUFFER_TIME;
    f32 m_currentTime = 0.0f;
    
    // Callbacks
    std::vector<InputEventCallback> m_eventCallbacks;
    std::vector<ActionCallback> m_actionCallbacks;
    std::vector<AxisCallback> m_axisCallbacks;
    std::vector<GestureCallback> m_gestureCallbacks;
    
    // Text input
    bool m_textInputActive = false;
    std::string m_textInput;
    
    // Platform
    void* m_windowHandle = nullptr;
    
    // Timing
    f32 m_lastUpdateTime = 0.0f;
};

// ============================================================================
// Gesture Recognizer
// ============================================================================

/**
 * @brief Touch gesture recognition engine
 */
class GestureRecognizer {
public:
    GestureRecognizer();
    
    /**
     * @brief Update gesture recognition
     * @param touches Current touch points
     * @param deltaTime Frame delta time
     * @return Recognized gesture data
     */
    GestureData update(const std::vector<TouchPoint>& touches, f32 deltaTime);
    
    /**
     * @brief Reset gesture recognition state
     */
    void reset();
    
    /**
     * @brief Enable/disable specific gesture types
     */
    void setGestureEnabled(GestureType type, bool enabled);
    
    /**
     * @brief Check if gesture type is enabled
     */
    bool isGestureEnabled(GestureType type) const;
    
private:
    // Gesture detection
    GestureData detectTap(const std::vector<TouchPoint>& touches);
    GestureData detectLongPress(const std::vector<TouchPoint>& touches);
    GestureData detectPan(const std::vector<TouchPoint>& touches);
    GestureData detectPinch(const std::vector<TouchPoint>& touches);
    GestureData detectRotate(const std::vector<TouchPoint>& touches);
    GestureData detectSwipe(const std::vector<TouchPoint>& touches);
    
    // State
    GestureType m_currentType = GestureType::None;
    Vec2 m_gestureStart;
    Vec2 m_gestureCenter;
    f32 m_initialDistance = 0.0f;
    f32 m_initialAngle = 0.0f;
    f32 m_gestureStartTime = 0.0f;
    bool m_gestureBegan = false;
    
    // Tap detection
    Vec2 m_lastTapPosition;
    f32 m_lastTapTime = -1.0f;
    u32 m_tapCount = 0;
    
    // Enabled gestures
    std::array<bool, 10> m_enabledGestures;
};

} // namespace nova::input
