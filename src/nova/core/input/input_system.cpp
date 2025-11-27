/**
 * @file input_system.cpp
 * @brief NovaCore Input System™ - Main Input Manager Implementation
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#include <nova/core/input/input_system.hpp>

#include <algorithm>
#include <cmath>

namespace nova::input {

// ============================================================================
// Singleton
// ============================================================================

InputSystem& InputSystem::get() {
    static InputSystem instance;
    return instance;
}

// ============================================================================
// Initialization
// ============================================================================

bool InputSystem::initialize() {
    // Initialize gesture recognizer
    m_gestureRecognizer = std::make_unique<GestureRecognizer>();
    
    // Clear all state
    clearState();
    
    // Check for motion sensors (platform-specific)
    #if defined(NOVA_PLATFORM_ANDROID) || defined(NOVA_PLATFORM_IOS)
    m_hasMotionSensors = true;
    #endif
    
    return true;
}

void InputSystem::shutdown() {
    clearCallbacks();
    clearState();
    m_gestureRecognizer.reset();
}

void InputSystem::update() {
    // Calculate delta time
    f32 currentTime = static_cast<f32>(std::chrono::duration<f32>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    f32 deltaTime = currentTime - m_lastUpdateTime;
    m_lastUpdateTime = currentTime;
    m_currentTime = currentTime;
    
    // Store previous state
    updatePreviousState();
    
    // Reset per-frame values
    m_mouseDelta = Vec2::zero();
    m_mouseScroll = Vec2::zero();
    m_textInput.clear();
    
    // Update subsystems
    updateGestures(deltaTime);
    updateAxes(deltaTime);
    updateBuffer(deltaTime);
    updateRumble(deltaTime);
    
    // Check for action state changes and fire callbacks
    for (const auto& [name, action] : m_actions) {
        bool currentState = isActionDown(name);
        bool previousState = m_previousActionState[name];
        
        if (currentState != previousState) {
            m_actionState[name] = currentState;
            for (const auto& callback : m_actionCallbacks) {
                callback(name, currentState);
            }
            
            // Add to buffer if pressed
            if (currentState) {
                m_inputBuffer.push({name, m_currentTime});
            }
        }
    }
}

void InputSystem::clearState() {
    // Keyboard
    m_keyState.fill(false);
    m_previousKeyState.fill(false);
    m_currentModifiers = KeyMod::None;
    
    // Mouse
    m_mousePosition = Vec2::zero();
    m_previousMousePosition = Vec2::zero();
    m_mouseDelta = Vec2::zero();
    m_mouseScroll = Vec2::zero();
    m_mouseButtonState.fill(false);
    m_previousMouseButtonState.fill(false);
    
    // Touch
    m_touches.clear();
    m_currentGesture = GestureData();
    if (m_gestureRecognizer) {
        m_gestureRecognizer->reset();
    }
    
    // Gamepad
    for (auto& gamepad : m_gamepads) {
        gamepad.buttons.fill(false);
        gamepad.axes.fill(0.0f);
    }
    for (auto& prev : m_previousGamepadState) {
        prev.buttons.fill(false);
        prev.axes.fill(0.0f);
    }
    m_rumbleState.fill({});
    
    // Motion
    m_motionData = MotionData();
    
    // Actions
    for (auto& [name, state] : m_actionState) {
        state = false;
    }
    for (auto& [name, state] : m_previousActionState) {
        state = false;
    }
    
    // Axes
    for (auto& [name, value] : m_axisValues) {
        value = 0.0f;
    }
    for (auto& [name, value] : m_axisRawValues) {
        value = 0.0f;
    }
    
    // Buffer
    while (!m_inputBuffer.empty()) {
        m_inputBuffer.pop();
    }
    
    // Text
    m_textInput.clear();
}

// ============================================================================
// Event Processing
// ============================================================================

void InputSystem::processEvent(const InputEvent& event) {
    // Fire event callbacks first
    for (const auto& callback : m_eventCallbacks) {
        if (callback(event)) {
            return;  // Event consumed
        }
    }
    
    switch (event.type) {
        case InputEventType::KeyDown:
        case InputEventType::KeyUp:
        case InputEventType::KeyRepeat:
            processKeyEvent(event.key, event.type);
            break;
            
        case InputEventType::TextInput:
            m_textInput += event.text.text;
            break;
            
        case InputEventType::MouseMove:
        case InputEventType::MouseButtonDown:
        case InputEventType::MouseButtonUp:
        case InputEventType::MouseScroll:
            processMouseEvent(event.mouse, event.type);
            break;
            
        case InputEventType::TouchBegin:
        case InputEventType::TouchMove:
        case InputEventType::TouchEnd:
        case InputEventType::TouchCancel:
            processTouchEvent(event.touch, event.type);
            break;
            
        case InputEventType::GamepadConnect:
        case InputEventType::GamepadDisconnect:
        case InputEventType::GamepadButtonDown:
        case InputEventType::GamepadButtonUp:
        case InputEventType::GamepadAxisMove:
            processGamepadEvent(event.gamepad, event.type);
            break;
            
        case InputEventType::MotionUpdate:
            processMotionEvent(event.motion);
            break;
            
        case InputEventType::FocusLost:
            clearState();
            break;
            
        default:
            break;
    }
}

void InputSystem::processKeyEvent(const KeyEvent& event, InputEventType type) {
    auto keyIndex = static_cast<usize>(event.key);
    if (keyIndex >= m_keyState.size()) return;
    
    if (type == InputEventType::KeyDown || type == InputEventType::KeyRepeat) {
        m_keyState[keyIndex] = true;
    } else if (type == InputEventType::KeyUp) {
        m_keyState[keyIndex] = false;
    }
    
    m_currentModifiers = event.modifiers;
}

void InputSystem::processMouseEvent(const MouseEvent& event, InputEventType type) {
    switch (type) {
        case InputEventType::MouseMove:
            m_previousMousePosition = m_mousePosition;
            m_mousePosition = event.position;
            m_mouseDelta = m_mouseDelta + event.delta;
            break;
            
        case InputEventType::MouseButtonDown:
            m_mouseButtonState[static_cast<usize>(event.button)] = true;
            break;
            
        case InputEventType::MouseButtonUp:
            m_mouseButtonState[static_cast<usize>(event.button)] = false;
            break;
            
        case InputEventType::MouseScroll:
            m_mouseScroll = m_mouseScroll + event.scroll;
            break;
            
        default:
            break;
    }
}

void InputSystem::processTouchEvent(const TouchEvent& event, InputEventType type) {
    switch (type) {
        case InputEventType::TouchBegin: {
            // Add new touch point
            TouchPoint touch = event.touch;
            touch.startTime = m_currentTime;
            touch.timestamp = m_currentTime;
            m_touches.push_back(touch);
            break;
        }
        
        case InputEventType::TouchMove: {
            // Update existing touch
            for (auto& touch : m_touches) {
                if (touch.id == event.touch.id) {
                    touch.previousPosition = touch.position;
                    touch.position = event.touch.position;
                    touch.delta = touch.position - touch.previousPosition;
                    touch.pressure = event.touch.pressure;
                    touch.timestamp = m_currentTime;
                    touch.phase = TouchPhase::Moved;
                    break;
                }
            }
            break;
        }
        
        case InputEventType::TouchEnd:
        case InputEventType::TouchCancel: {
            // Remove touch
            m_touches.erase(
                std::remove_if(m_touches.begin(), m_touches.end(),
                    [&](const TouchPoint& t) { return t.id == event.touch.id; }),
                m_touches.end()
            );
            break;
        }
        
        default:
            break;
    }
}

void InputSystem::processGamepadEvent(const GamepadEvent& event, InputEventType type) {
    if (event.gamepadIndex >= InputConfig::MAX_GAMEPADS) return;
    
    auto& gamepad = m_gamepads[event.gamepadIndex];
    
    switch (type) {
        case InputEventType::GamepadConnect:
            gamepad.connected = true;
            break;
            
        case InputEventType::GamepadDisconnect:
            gamepad.connected = false;
            gamepad.buttons.fill(false);
            gamepad.axes.fill(0.0f);
            break;
            
        case InputEventType::GamepadButtonDown:
            gamepad.buttons[static_cast<usize>(event.button)] = true;
            break;
            
        case InputEventType::GamepadButtonUp:
            gamepad.buttons[static_cast<usize>(event.button)] = false;
            break;
            
        case InputEventType::GamepadAxisMove: {
            auto axisIndex = static_cast<usize>(event.axis);
            if (axisIndex < gamepad.axes.size()) {
                // Apply deadzone
                f32 value = event.axisValue;
                if (std::abs(value) < InputConfig::GAMEPAD_DEADZONE) {
                    value = 0.0f;
                } else {
                    // Rescale after deadzone
                    f32 sign = value > 0.0f ? 1.0f : -1.0f;
                    value = sign * (std::abs(value) - InputConfig::GAMEPAD_DEADZONE) / 
                            (1.0f - InputConfig::GAMEPAD_DEADZONE);
                }
                gamepad.axes[axisIndex] = value;
            }
            break;
        }
        
        default:
            break;
    }
}

void InputSystem::processMotionEvent(const MotionData& motion) {
    m_motionData = motion;
}

// ============================================================================
// State Update
// ============================================================================

void InputSystem::updatePreviousState() {
    m_previousKeyState = m_keyState;
    m_previousMouseButtonState = m_mouseButtonState;
    m_previousMousePosition = m_mousePosition;
    
    for (u32 i = 0; i < InputConfig::MAX_GAMEPADS; ++i) {
        m_previousGamepadState[i] = m_gamepads[i];
    }
    
    m_previousActionState = m_actionState;
}

void InputSystem::updateGestures(f32 deltaTime) {
    if (!m_gestureRecognizer) return;
    
    m_currentGesture = m_gestureRecognizer->update(m_touches, deltaTime);
    
    if (m_currentGesture.type != GestureType::None) {
        for (const auto& callback : m_gestureCallbacks) {
            callback(m_currentGesture);
        }
    }
}

void InputSystem::updateAxes(f32 deltaTime) {
    for (const auto& [name, axis] : m_axes) {
        // Calculate raw value
        f32 positive = 0.0f;
        f32 negative = 0.0f;
        
        for (const auto& binding : axis.positiveBindings) {
            positive = std::max(positive, evaluateBinding(binding));
        }
        for (const auto& binding : axis.negativeBindings) {
            negative = std::max(negative, evaluateBinding(binding));
        }
        
        f32 rawValue = positive - negative;
        m_axisRawValues[name] = rawValue;
        
        // Apply smoothing
        f32 currentValue = m_axisValues[name];
        f32 targetValue = rawValue * axis.sensitivity;
        
        // Snap to zero when changing direction
        if (axis.snap && std::signbit(targetValue) != std::signbit(currentValue) && targetValue != 0.0f) {
            currentValue = 0.0f;
        }
        
        // Move towards target
        if (targetValue != 0.0f) {
            currentValue = targetValue;  // Instant for digital input
        } else {
            // Gravity returns to zero
            if (currentValue > 0.0f) {
                currentValue = std::max(0.0f, currentValue - axis.gravity * deltaTime);
            } else if (currentValue < 0.0f) {
                currentValue = std::min(0.0f, currentValue + axis.gravity * deltaTime);
            }
        }
        
        // Apply deadzone
        if (std::abs(currentValue) < axis.deadzone) {
            currentValue = 0.0f;
        }
        
        // Clamp
        currentValue = std::clamp(currentValue, -1.0f, 1.0f);
        
        // Fire callback if changed
        if (m_axisValues[name] != currentValue) {
            m_axisValues[name] = currentValue;
            for (const auto& callback : m_axisCallbacks) {
                callback(name, currentValue);
            }
        }
    }
}

void InputSystem::updateBuffer(f32 deltaTime) {
    // Remove expired buffer entries
    while (!m_inputBuffer.empty()) {
        if (m_currentTime - m_inputBuffer.front().timestamp > m_bufferWindow) {
            m_inputBuffer.pop();
        } else {
            break;
        }
    }
}

void InputSystem::updateRumble(f32 deltaTime) {
    for (u32 i = 0; i < InputConfig::MAX_GAMEPADS; ++i) {
        auto& rumble = m_rumbleState[i];
        if (rumble.remaining > 0.0f) {
            rumble.remaining -= deltaTime;
            if (rumble.remaining <= 0.0f) {
                rumble.lowFreq = 0.0f;
                rumble.highFreq = 0.0f;
                // Platform-specific: stop rumble on gamepad i
            }
        }
    }
}

// ============================================================================
// Keyboard Queries
// ============================================================================

bool InputSystem::isKeyDown(Key key) const {
    auto index = static_cast<usize>(key);
    return index < m_keyState.size() && m_keyState[index];
}

bool InputSystem::isKeyPressed(Key key) const {
    auto index = static_cast<usize>(key);
    return index < m_keyState.size() && m_keyState[index] && !m_previousKeyState[index];
}

bool InputSystem::isKeyReleased(Key key) const {
    auto index = static_cast<usize>(key);
    return index < m_keyState.size() && !m_keyState[index] && m_previousKeyState[index];
}

bool InputSystem::isAnyKeyDown() const {
    for (bool state : m_keyState) {
        if (state) return true;
    }
    return false;
}

// ============================================================================
// Mouse Queries
// ============================================================================

bool InputSystem::isMouseButtonDown(MouseButton button) const {
    auto index = static_cast<usize>(button);
    return index < m_mouseButtonState.size() && m_mouseButtonState[index];
}

bool InputSystem::isMouseButtonPressed(MouseButton button) const {
    auto index = static_cast<usize>(button);
    return index < m_mouseButtonState.size() && 
           m_mouseButtonState[index] && !m_previousMouseButtonState[index];
}

bool InputSystem::isMouseButtonReleased(MouseButton button) const {
    auto index = static_cast<usize>(button);
    return index < m_mouseButtonState.size() && 
           !m_mouseButtonState[index] && m_previousMouseButtonState[index];
}

void InputSystem::setCursorMode(CursorMode mode) {
    m_cursorMode = mode;
    // Platform-specific implementation
}

void InputSystem::setCursorShape(CursorShape shape) {
    m_cursorShape = shape;
    // Platform-specific implementation
}

void InputSystem::setCursorPosition(const Vec2& position) {
    m_mousePosition = position;
    // Platform-specific implementation
}

// ============================================================================
// Touch Queries
// ============================================================================

u32 InputSystem::getTouchCount() const {
    return static_cast<u32>(m_touches.size());
}

std::optional<TouchPoint> InputSystem::getTouch(u32 index) const {
    if (index < m_touches.size()) {
        return m_touches[index];
    }
    return std::nullopt;
}

std::optional<TouchPoint> InputSystem::getTouchById(u32 id) const {
    for (const auto& touch : m_touches) {
        if (touch.id == id) {
            return touch;
        }
    }
    return std::nullopt;
}

// ============================================================================
// Gamepad Queries
// ============================================================================

u32 InputSystem::getGamepadCount() const {
    u32 count = 0;
    for (const auto& gamepad : m_gamepads) {
        if (gamepad.connected) ++count;
    }
    return count;
}

bool InputSystem::isGamepadConnected(u32 index) const {
    return index < InputConfig::MAX_GAMEPADS && m_gamepads[index].connected;
}

const GamepadState& InputSystem::getGamepad(u32 index) const {
    static GamepadState empty;
    if (index < InputConfig::MAX_GAMEPADS) {
        return m_gamepads[index];
    }
    return empty;
}

bool InputSystem::isGamepadButtonDown(u32 gamepad, GamepadButton button) const {
    if (gamepad >= InputConfig::MAX_GAMEPADS) return false;
    return m_gamepads[gamepad].isButtonDown(button);
}

bool InputSystem::isGamepadButtonPressed(u32 gamepad, GamepadButton button) const {
    if (gamepad >= InputConfig::MAX_GAMEPADS) return false;
    auto index = static_cast<usize>(button);
    return m_gamepads[gamepad].buttons[index] && !m_previousGamepadState[gamepad].buttons[index];
}

bool InputSystem::isGamepadButtonReleased(u32 gamepad, GamepadButton button) const {
    if (gamepad >= InputConfig::MAX_GAMEPADS) return false;
    auto index = static_cast<usize>(button);
    return !m_gamepads[gamepad].buttons[index] && m_previousGamepadState[gamepad].buttons[index];
}

f32 InputSystem::getGamepadAxis(u32 gamepad, GamepadAxis axis) const {
    if (gamepad >= InputConfig::MAX_GAMEPADS) return 0.0f;
    auto index = static_cast<usize>(axis);
    if (index >= m_gamepads[gamepad].axes.size()) return 0.0f;
    return m_gamepads[gamepad].axes[index];
}

Vec2 InputSystem::getGamepadLeftStick(u32 gamepad) const {
    if (gamepad >= InputConfig::MAX_GAMEPADS) return Vec2::zero();
    return m_gamepads[gamepad].leftStick();
}

Vec2 InputSystem::getGamepadRightStick(u32 gamepad) const {
    if (gamepad >= InputConfig::MAX_GAMEPADS) return Vec2::zero();
    return m_gamepads[gamepad].rightStick();
}

void InputSystem::setGamepadRumble(u32 gamepad, f32 lowFreq, f32 highFreq, f32 duration) {
    if (gamepad >= InputConfig::MAX_GAMEPADS) return;
    
    m_rumbleState[gamepad].lowFreq = std::clamp(lowFreq, 0.0f, 1.0f);
    m_rumbleState[gamepad].highFreq = std::clamp(highFreq, 0.0f, 1.0f);
    m_rumbleState[gamepad].remaining = duration > 0.0f ? duration : 1000.0f;
    
    // Platform-specific: set rumble on gamepad
}

void InputSystem::stopGamepadRumble(u32 gamepad) {
    if (gamepad >= InputConfig::MAX_GAMEPADS) return;
    
    m_rumbleState[gamepad] = {};
    // Platform-specific: stop rumble
}

// ============================================================================
// Motion Sensors
// ============================================================================

void InputSystem::setMotionSensorsEnabled(bool enabled) {
    m_motionSensorsEnabled = enabled && m_hasMotionSensors;
    // Platform-specific: enable/disable sensors
}

Vec2 InputSystem::getTilt() const {
    return Vec2(m_motionData.pitch(), m_motionData.roll());
}

f32 InputSystem::getShakeIntensity() const {
    return m_motionData.linearAcceleration.length() / 20.0f;  // Normalize to 0-1
}

// ============================================================================
// Action Mapping
// ============================================================================

void InputSystem::registerAction(const std::string& name, const InputBinding& binding) {
    auto& action = m_actions[name];
    action.name = name;
    action.addBinding(binding);
    m_actionState[name] = false;
    m_previousActionState[name] = false;
}

void InputSystem::registerAction(const std::string& name, const std::vector<InputBinding>& bindings) {
    auto& action = m_actions[name];
    action.name = name;
    action.bindings = bindings;
    m_actionState[name] = false;
    m_previousActionState[name] = false;
}

void InputSystem::unregisterAction(const std::string& name) {
    m_actions.erase(name);
    m_actionState.erase(name);
    m_previousActionState.erase(name);
}

bool InputSystem::hasAction(const std::string& name) const {
    return m_actions.find(name) != m_actions.end();
}

bool InputSystem::isActionDown(const std::string& name) const {
    auto it = m_actions.find(name);
    if (it == m_actions.end()) return false;
    
    for (const auto& binding : it->second.bindings) {
        if (evaluateBinding(binding) > 0.5f) {
            return true;
        }
    }
    return false;
}

bool InputSystem::isActionPressed(const std::string& name) const {
    auto stateIt = m_actionState.find(name);
    auto prevIt = m_previousActionState.find(name);
    
    if (stateIt == m_actionState.end() || prevIt == m_previousActionState.end()) {
        return false;
    }
    
    return isActionDown(name) && !prevIt->second;
}

bool InputSystem::isActionReleased(const std::string& name) const {
    auto stateIt = m_actionState.find(name);
    auto prevIt = m_previousActionState.find(name);
    
    if (stateIt == m_actionState.end() || prevIt == m_previousActionState.end()) {
        return false;
    }
    
    return !isActionDown(name) && prevIt->second;
}

f32 InputSystem::getActionValue(const std::string& name) const {
    auto it = m_actions.find(name);
    if (it == m_actions.end()) return 0.0f;
    
    f32 maxValue = 0.0f;
    for (const auto& binding : it->second.bindings) {
        maxValue = std::max(maxValue, evaluateBinding(binding));
    }
    return maxValue;
}

// ============================================================================
// Axis Mapping
// ============================================================================

void InputSystem::registerAxis(const std::string& name, const InputAxis& axis) {
    m_axes[name] = axis;
    m_axes[name].name = name;
    m_axisValues[name] = 0.0f;
    m_axisRawValues[name] = 0.0f;
}

void InputSystem::registerAxis(const std::string& name, Key positive, Key negative) {
    InputAxis axis;
    axis.name = name;
    axis.addPositive(InputBinding::key(positive));
    axis.addNegative(InputBinding::key(negative));
    registerAxis(name, axis);
}

void InputSystem::unregisterAxis(const std::string& name) {
    m_axes.erase(name);
    m_axisValues.erase(name);
    m_axisRawValues.erase(name);
}

f32 InputSystem::getAxis(const std::string& name) const {
    auto it = m_axisValues.find(name);
    return it != m_axisValues.end() ? it->second : 0.0f;
}

f32 InputSystem::getAxisRaw(const std::string& name) const {
    auto it = m_axisRawValues.find(name);
    return it != m_axisRawValues.end() ? it->second : 0.0f;
}

// ============================================================================
// Input Buffering
// ============================================================================

bool InputSystem::isActionBuffered(const std::string& name) const {
    std::queue<BufferedInput> copy = m_inputBuffer;
    while (!copy.empty()) {
        if (copy.front().action == name) {
            return true;
        }
        copy.pop();
    }
    return false;
}

bool InputSystem::consumeBufferedAction(const std::string& name) {
    std::queue<BufferedInput> newBuffer;
    bool found = false;
    
    while (!m_inputBuffer.empty()) {
        if (!found && m_inputBuffer.front().action == name) {
            found = true;
            m_inputBuffer.pop();
        } else {
            newBuffer.push(m_inputBuffer.front());
            m_inputBuffer.pop();
        }
    }
    
    m_inputBuffer = std::move(newBuffer);
    return found;
}

void InputSystem::clearBuffer() {
    while (!m_inputBuffer.empty()) {
        m_inputBuffer.pop();
    }
}

void InputSystem::setBufferWindow(f32 seconds) {
    m_bufferWindow = std::max(0.0f, seconds);
}

// ============================================================================
// Callbacks
// ============================================================================

void InputSystem::addEventCallback(InputEventCallback callback) {
    m_eventCallbacks.push_back(std::move(callback));
}

void InputSystem::addActionCallback(ActionCallback callback) {
    m_actionCallbacks.push_back(std::move(callback));
}

void InputSystem::addAxisCallback(AxisCallback callback) {
    m_axisCallbacks.push_back(std::move(callback));
}

void InputSystem::addGestureCallback(GestureCallback callback) {
    m_gestureCallbacks.push_back(std::move(callback));
}

void InputSystem::clearCallbacks() {
    m_eventCallbacks.clear();
    m_actionCallbacks.clear();
    m_axisCallbacks.clear();
    m_gestureCallbacks.clear();
}

// ============================================================================
// Text Input
// ============================================================================

void InputSystem::startTextInput() {
    m_textInputActive = true;
    // Platform-specific: show virtual keyboard
}

void InputSystem::stopTextInput() {
    m_textInputActive = false;
    // Platform-specific: hide virtual keyboard
}

void InputSystem::setTextInputRect(const Vec2& position, const Vec2& size) {
    // Platform-specific: set text input area for virtual keyboard positioning
}

// ============================================================================
// Platform Integration
// ============================================================================

void InputSystem::setWindowHandle(void* handle) {
    m_windowHandle = handle;
}

std::string InputSystem::getClipboardText() const {
    // Platform-specific implementation
    return "";
}

void InputSystem::setClipboardText(const std::string& text) {
    // Platform-specific implementation
}

// ============================================================================
// Binding Evaluation
// ============================================================================

f32 InputSystem::evaluateBinding(const InputBinding& binding) const {
    // Check required modifiers
    if (binding.modifiers != KeyMod::None) {
        if ((static_cast<u8>(m_currentModifiers) & static_cast<u8>(binding.modifiers)) != 
            static_cast<u8>(binding.modifiers)) {
            return 0.0f;
        }
    }
    
    f32 value = 0.0f;
    
    switch (binding.type) {
        case BindingType::Key:
            value = isKeyDown(static_cast<Key>(binding.code)) ? 1.0f : 0.0f;
            break;
            
        case BindingType::MouseButton:
            value = isMouseButtonDown(static_cast<MouseButton>(binding.code)) ? 1.0f : 0.0f;
            break;
            
        case BindingType::MouseAxis:
            if (binding.code == 0) value = m_mouseDelta.x;
            else if (binding.code == 1) value = m_mouseDelta.y;
            break;
            
        case BindingType::GamepadButton:
            value = isGamepadButtonDown(0, static_cast<GamepadButton>(binding.code)) ? 1.0f : 0.0f;
            break;
            
        case BindingType::GamepadAxis:
            value = getGamepadAxis(0, static_cast<GamepadAxis>(binding.code));
            break;
            
        case BindingType::Touch:
            value = isTouching() ? 1.0f : 0.0f;
            break;
            
        case BindingType::Gesture:
            value = (m_currentGesture.type == static_cast<GestureType>(binding.code) && 
                    m_currentGesture.isActive) ? 1.0f : 0.0f;
            break;
    }
    
    // Apply scale and inversion
    value *= binding.scale;
    if (binding.inverted) value = -value;
    
    return value;
}

// ============================================================================
// Key Names
// ============================================================================

std::string InputSystem::getKeyName(Key key) {
    switch (key) {
        case Key::A: return "A";
        case Key::B: return "B";
        case Key::C: return "C";
        case Key::D: return "D";
        case Key::E: return "E";
        case Key::F: return "F";
        case Key::G: return "G";
        case Key::H: return "H";
        case Key::I: return "I";
        case Key::J: return "J";
        case Key::K: return "K";
        case Key::L: return "L";
        case Key::M: return "M";
        case Key::N: return "N";
        case Key::O: return "O";
        case Key::P: return "P";
        case Key::Q: return "Q";
        case Key::R: return "R";
        case Key::S: return "S";
        case Key::T: return "T";
        case Key::U: return "U";
        case Key::V: return "V";
        case Key::W: return "W";
        case Key::X: return "X";
        case Key::Y: return "Y";
        case Key::Z: return "Z";
        case Key::Space: return "Space";
        case Key::Return: return "Enter";
        case Key::Escape: return "Escape";
        case Key::Tab: return "Tab";
        case Key::LeftShift: return "Left Shift";
        case Key::RightShift: return "Right Shift";
        case Key::LeftControl: return "Left Ctrl";
        case Key::RightControl: return "Right Ctrl";
        case Key::LeftAlt: return "Left Alt";
        case Key::RightAlt: return "Right Alt";
        default: return "Unknown";
    }
}

std::string InputSystem::getMouseButtonName(MouseButton button) {
    switch (button) {
        case MouseButton::Left: return "Left Mouse";
        case MouseButton::Right: return "Right Mouse";
        case MouseButton::Middle: return "Middle Mouse";
        default: return "Mouse " + std::to_string(static_cast<int>(button) + 1);
    }
}

std::string InputSystem::getGamepadButtonName(GamepadButton button, GamepadType type) {
    switch (button) {
        case GamepadButton::A: 
            return type == GamepadType::PlayStation ? "Cross" : "A";
        case GamepadButton::B:
            return type == GamepadType::PlayStation ? "Circle" : "B";
        case GamepadButton::X:
            return type == GamepadType::PlayStation ? "Square" : "X";
        case GamepadButton::Y:
            return type == GamepadType::PlayStation ? "Triangle" : "Y";
        case GamepadButton::LeftBumper: return "LB";
        case GamepadButton::RightBumper: return "RB";
        case GamepadButton::Back: return "Back";
        case GamepadButton::Start: return "Start";
        case GamepadButton::Guide: return "Guide";
        case GamepadButton::LeftStick: return "L3";
        case GamepadButton::RightStick: return "R3";
        case GamepadButton::DPadUp: return "D-Pad Up";
        case GamepadButton::DPadDown: return "D-Pad Down";
        case GamepadButton::DPadLeft: return "D-Pad Left";
        case GamepadButton::DPadRight: return "D-Pad Right";
        default: return "Button " + std::to_string(static_cast<int>(button));
    }
}

// ============================================================================
// Gesture Recognizer Implementation
// ============================================================================

GestureRecognizer::GestureRecognizer() {
    m_enabledGestures.fill(true);
}

GestureData GestureRecognizer::update(const std::vector<TouchPoint>& touches, f32 deltaTime) {
    GestureData result;
    result.touchCount = static_cast<u32>(touches.size());
    
    if (touches.empty()) {
        reset();
        return result;
    }
    
    // Calculate center of all touches
    Vec2 center = Vec2::zero();
    for (const auto& touch : touches) {
        center = center + touch.position;
    }
    center = center / static_cast<f32>(touches.size());
    result.position = center;
    
    // Single touch gestures
    if (touches.size() == 1) {
        const auto& touch = touches[0];
        
        // Check for tap
        if (isGestureEnabled(GestureType::Tap) && touch.isTap()) {
            result = detectTap(touches);
        }
        // Check for long press
        else if (isGestureEnabled(GestureType::LongPress) && touch.isLongPress()) {
            result = detectLongPress(touches);
        }
        // Check for pan/swipe
        else if (touch.distance() > InputConfig::TOUCH_DRAG_THRESHOLD) {
            if (isGestureEnabled(GestureType::Pan)) {
                result = detectPan(touches);
            }
        }
    }
    // Two finger gestures
    else if (touches.size() == 2) {
        if (isGestureEnabled(GestureType::Pinch)) {
            result = detectPinch(touches);
        }
        if (result.type == GestureType::None && isGestureEnabled(GestureType::Rotate)) {
            result = detectRotate(touches);
        }
    }
    
    return result;
}

void GestureRecognizer::reset() {
    m_currentType = GestureType::None;
    m_gestureStart = Vec2::zero();
    m_gestureCenter = Vec2::zero();
    m_initialDistance = 0.0f;
    m_initialAngle = 0.0f;
    m_gestureStartTime = 0.0f;
    m_gestureBegan = false;
}

void GestureRecognizer::setGestureEnabled(GestureType type, bool enabled) {
    if (static_cast<usize>(type) < m_enabledGestures.size()) {
        m_enabledGestures[static_cast<usize>(type)] = enabled;
    }
}

bool GestureRecognizer::isGestureEnabled(GestureType type) const {
    if (static_cast<usize>(type) < m_enabledGestures.size()) {
        return m_enabledGestures[static_cast<usize>(type)];
    }
    return false;
}

GestureData GestureRecognizer::detectTap(const std::vector<TouchPoint>& touches) {
    GestureData result;
    result.type = GestureType::Tap;
    result.position = touches[0].position;
    result.isComplete = true;
    
    // Check for double tap
    f32 currentTime = touches[0].timestamp;
    Vec2 posDiff = touches[0].position - m_lastTapPosition;
    f32 distSq = posDiff.x * posDiff.x + posDiff.y * posDiff.y;
    f32 thresholdSq = InputConfig::TOUCH_DRAG_THRESHOLD * InputConfig::TOUCH_DRAG_THRESHOLD;
    if (currentTime - m_lastTapTime < InputConfig::TOUCH_DOUBLE_TAP_INTERVAL &&
        distSq < thresholdSq) {
        result.type = GestureType::DoubleTap;
    }
    
    m_lastTapTime = currentTime;
    m_lastTapPosition = touches[0].position;
    
    return result;
}

GestureData GestureRecognizer::detectLongPress(const std::vector<TouchPoint>& touches) {
    GestureData result;
    result.type = GestureType::LongPress;
    result.position = touches[0].position;
    result.isActive = true;
    return result;
}

GestureData GestureRecognizer::detectPan(const std::vector<TouchPoint>& touches) {
    const auto& touch = touches[0];
    
    GestureData result;
    result.type = GestureType::Pan;
    result.position = touch.position;
    result.delta = touch.delta;
    result.velocity = touch.velocity().length();
    result.isActive = true;
    
    // Check for swipe on end
    if (touch.phase == TouchPhase::Ended) {
        result.isComplete = true;
        
        if (result.velocity > InputConfig::TOUCH_SWIPE_MIN_VELOCITY) {
            result.type = GestureType::Swipe;
            
            Vec2 dir = (touch.position - touch.startPosition).normalized();
            if (std::abs(dir.x) > std::abs(dir.y)) {
                result.swipeDir = dir.x > 0 ? SwipeDirection::Right : SwipeDirection::Left;
            } else {
                result.swipeDir = dir.y > 0 ? SwipeDirection::Down : SwipeDirection::Up;
            }
        }
    }
    
    return result;
}

GestureData GestureRecognizer::detectPinch(const std::vector<TouchPoint>& touches) {
    if (touches.size() < 2) return GestureData();
    
    Vec2 p1 = touches[0].position;
    Vec2 p2 = touches[1].position;
    f32 currentDistance = (p2 - p1).length();
    
    if (!m_gestureBegan) {
        m_gestureBegan = true;
        m_initialDistance = currentDistance;
        m_gestureCenter = (p1 + p2) * 0.5f;
    }
    
    GestureData result;
    result.type = GestureType::Pinch;
    result.position = (p1 + p2) * 0.5f;
    result.scale = m_initialDistance > 0.0f ? currentDistance / m_initialDistance : 1.0f;
    result.isActive = true;
    
    return result;
}

GestureData GestureRecognizer::detectRotate(const std::vector<TouchPoint>& touches) {
    if (touches.size() < 2) return GestureData();
    
    Vec2 p1 = touches[0].position;
    Vec2 p2 = touches[1].position;
    Vec2 delta = p2 - p1;
    f32 currentAngle = std::atan2(delta.y, delta.x);
    
    if (!m_gestureBegan) {
        m_gestureBegan = true;
        m_initialAngle = currentAngle;
    }
    
    GestureData result;
    result.type = GestureType::Rotate;
    result.position = (p1 + p2) * 0.5f;
    result.rotation = currentAngle - m_initialAngle;
    result.isActive = true;
    
    return result;
}

} // namespace nova::input
