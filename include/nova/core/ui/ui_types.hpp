/**
 * @file ui_types.hpp
 * @brief Nova UI™ - Core UI types and structures
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * Production-grade UI system supporting mobile-first responsive layouts,
 * touch interactions, and 60 FPS performance.
 * 
 * Features:
 * - Flexbox-based layout system
 * - Touch/mouse/gamepad input handling
 * - Theming and styling
 * - Animation support
 * - Accessibility features
 * - RTL language support
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 * @see NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md for full technical specifications
 */

#pragma once

#include "nova/core/types/types.hpp"
#include "nova/core/math/vec2.hpp"
#include "nova/core/math/vec4.hpp"

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <optional>
#include <variant>
#include <unordered_map>

namespace nova::ui {

using nova::u8;
using nova::u16;
using nova::u32;
using nova::u64;
using nova::i32;
using nova::f32;
using nova::usize;
using nova::math::Vec2;
using nova::math::Vec4;

// ============================================================================
// Forward Declarations
// ============================================================================

class Widget;
class Container;
class Button;
class Label;
class TextInput;
class Image;
class ScrollView;
class ListView;
class Canvas;
class UISystem;

// ============================================================================
// UI Constants
// ============================================================================

/// Default font size in points
constexpr f32 DEFAULT_FONT_SIZE = 14.0f;

/// Default line height multiplier
constexpr f32 DEFAULT_LINE_HEIGHT = 1.4f;

/// Default animation duration in seconds
constexpr f32 DEFAULT_ANIMATION_DURATION = 0.3f;

/// Default text input minimum width in pixels
constexpr f32 DEFAULT_TEXT_INPUT_MIN_WIDTH = 200.0f;

/// Default multiline text input line count
constexpr u32 DEFAULT_MULTILINE_LINE_COUNT = 3;

/// Touch slop for determining drag vs tap (in pixels)
constexpr f32 TOUCH_SLOP = 8.0f;

/// Long press duration in seconds
constexpr f32 LONG_PRESS_DURATION = 0.5f;

/// Double tap timeout in seconds
constexpr f32 DOUBLE_TAP_TIMEOUT = 0.3f;

/// Maximum widget tree depth
constexpr u32 MAX_WIDGET_DEPTH = 64;

// ============================================================================
// Color Types
// ============================================================================

/**
 * @brief RGBA color with float components [0-1]
 */
struct Color {
    f32 r = 0.0f;
    f32 g = 0.0f;
    f32 b = 0.0f;
    f32 a = 1.0f;
    
    constexpr Color() noexcept = default;
    constexpr Color(f32 r_, f32 g_, f32 b_, f32 a_ = 1.0f) noexcept
        : r(r_), g(g_), b(b_), a(a_) {}
    
    /// Create from 0-255 integer values
    static constexpr Color fromRGBA(u8 r, u8 g, u8 b, u8 a = 255) noexcept {
        return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }
    
    /// Create from hex value (0xRRGGBB or 0xRRGGBBAA)
    static constexpr Color fromHex(u32 hex) noexcept {
        if (hex > 0xFFFFFF) {
            // 0xRRGGBBAA format
            return Color(
                static_cast<f32>((hex >> 24) & 0xFF) / 255.0f,
                static_cast<f32>((hex >> 16) & 0xFF) / 255.0f,
                static_cast<f32>((hex >> 8) & 0xFF) / 255.0f,
                static_cast<f32>(hex & 0xFF) / 255.0f
            );
        } else {
            // 0xRRGGBB format
            return Color(
                static_cast<f32>((hex >> 16) & 0xFF) / 255.0f,
                static_cast<f32>((hex >> 8) & 0xFF) / 255.0f,
                static_cast<f32>(hex & 0xFF) / 255.0f,
                1.0f
            );
        }
    }
    
    /// Lerp between two colors
    [[nodiscard]] constexpr Color lerp(const Color& other, f32 t) const noexcept {
        return Color(
            r + (other.r - r) * t,
            g + (other.g - g) * t,
            b + (other.b - b) * t,
            a + (other.a - a) * t
        );
    }
    
    /// Convert to Vec4
    [[nodiscard]] constexpr Vec4 toVec4() const noexcept {
        return Vec4(r, g, b, a);
    }
    
    // Predefined colors
    static constexpr Color transparent() noexcept { return Color(0, 0, 0, 0); }
    static constexpr Color white() noexcept { return Color(1, 1, 1, 1); }
    static constexpr Color black() noexcept { return Color(0, 0, 0, 1); }
    static constexpr Color red() noexcept { return Color(1, 0, 0, 1); }
    static constexpr Color green() noexcept { return Color(0, 1, 0, 1); }
    static constexpr Color blue() noexcept { return Color(0, 0, 1, 1); }
    static constexpr Color yellow() noexcept { return Color(1, 1, 0, 1); }
    static constexpr Color cyan() noexcept { return Color(0, 1, 1, 1); }
    static constexpr Color magenta() noexcept { return Color(1, 0, 1, 1); }
    static constexpr Color gray() noexcept { return Color(0.5f, 0.5f, 0.5f, 1); }
};

// ============================================================================
// Geometry Types
// ============================================================================

/**
 * @brief 2D rectangle with position and size
 */
struct Rect {
    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 width = 0.0f;
    f32 height = 0.0f;
    
    constexpr Rect() noexcept = default;
    constexpr Rect(f32 x_, f32 y_, f32 w_, f32 h_) noexcept
        : x(x_), y(y_), width(w_), height(h_) {}
    
    /// Create from position and size
    static constexpr Rect fromPosSize(Vec2 pos, Vec2 size) noexcept {
        return Rect(pos.x, pos.y, size.x, size.y);
    }
    
    /// Create from two corners
    static constexpr Rect fromCorners(Vec2 topLeft, Vec2 bottomRight) noexcept {
        return Rect(topLeft.x, topLeft.y, 
                    bottomRight.x - topLeft.x, 
                    bottomRight.y - topLeft.y);
    }
    
    [[nodiscard]] constexpr Vec2 position() const noexcept { return Vec2(x, y); }
    [[nodiscard]] constexpr Vec2 size() const noexcept { return Vec2(width, height); }
    [[nodiscard]] constexpr Vec2 center() const noexcept { 
        return Vec2(x + width * 0.5f, y + height * 0.5f); 
    }
    [[nodiscard]] constexpr f32 left() const noexcept { return x; }
    [[nodiscard]] constexpr f32 right() const noexcept { return x + width; }
    [[nodiscard]] constexpr f32 top() const noexcept { return y; }
    [[nodiscard]] constexpr f32 bottom() const noexcept { return y + height; }
    
    /// Check if point is inside rect
    [[nodiscard]] constexpr bool contains(Vec2 point) const noexcept {
        return point.x >= x && point.x < x + width &&
               point.y >= y && point.y < y + height;
    }
    
    /// Check if rects intersect
    [[nodiscard]] constexpr bool intersects(const Rect& other) const noexcept {
        return x < other.x + other.width && x + width > other.x &&
               y < other.y + other.height && y + height > other.y;
    }
    
    /// Get intersection of two rects
    [[nodiscard]] constexpr Rect intersection(const Rect& other) const noexcept {
        f32 l = x > other.x ? x : other.x;
        f32 t = y > other.y ? y : other.y;
        f32 r = (x + width) < (other.x + other.width) ? (x + width) : (other.x + other.width);
        f32 b = (y + height) < (other.y + other.height) ? (y + height) : (other.y + other.height);
        return Rect(l, t, r > l ? r - l : 0, b > t ? b - t : 0);
    }
    
    /// Expand rect by amount
    [[nodiscard]] constexpr Rect expand(f32 amount) const noexcept {
        return Rect(x - amount, y - amount, width + amount * 2, height + amount * 2);
    }
    
    /// Expand rect by different amounts per side
    [[nodiscard]] constexpr Rect expand(f32 l, f32 t, f32 r, f32 b) const noexcept {
        return Rect(x - l, y - t, width + l + r, height + t + b);
    }
};

/**
 * @brief Edge insets (margin/padding)
 */
struct EdgeInsets {
    f32 left = 0.0f;
    f32 top = 0.0f;
    f32 right = 0.0f;
    f32 bottom = 0.0f;
    
    constexpr EdgeInsets() noexcept = default;
    constexpr EdgeInsets(f32 all) noexcept 
        : left(all), top(all), right(all), bottom(all) {}
    constexpr EdgeInsets(f32 horizontal, f32 vertical) noexcept
        : left(horizontal), top(vertical), right(horizontal), bottom(vertical) {}
    constexpr EdgeInsets(f32 l, f32 t, f32 r, f32 b) noexcept
        : left(l), top(t), right(r), bottom(b) {}
    
    [[nodiscard]] constexpr f32 horizontal() const noexcept { return left + right; }
    [[nodiscard]] constexpr f32 vertical() const noexcept { return top + bottom; }
    
    static constexpr EdgeInsets zero() noexcept { return EdgeInsets(0); }
    static constexpr EdgeInsets all(f32 value) noexcept { return EdgeInsets(value); }
    static constexpr EdgeInsets symmetric(f32 h, f32 v) noexcept { return EdgeInsets(h, v); }
};

/**
 * @brief Corner radii for rounded rectangles
 */
struct CornerRadii {
    f32 topLeft = 0.0f;
    f32 topRight = 0.0f;
    f32 bottomLeft = 0.0f;
    f32 bottomRight = 0.0f;
    
    constexpr CornerRadii() noexcept = default;
    constexpr CornerRadii(f32 all) noexcept
        : topLeft(all), topRight(all), bottomLeft(all), bottomRight(all) {}
    constexpr CornerRadii(f32 tl, f32 tr, f32 bl, f32 br) noexcept
        : topLeft(tl), topRight(tr), bottomLeft(bl), bottomRight(br) {}
    
    static constexpr CornerRadii zero() noexcept { return CornerRadii(0); }
    static constexpr CornerRadii circular(f32 radius) noexcept { return CornerRadii(radius); }
};

// ============================================================================
// Layout Enums
// ============================================================================

/**
 * @brief Flexbox direction
 */
enum class FlexDirection : u8 {
    Row,            ///< Main axis horizontal (left to right)
    RowReverse,     ///< Main axis horizontal (right to left)
    Column,         ///< Main axis vertical (top to bottom)
    ColumnReverse   ///< Main axis vertical (bottom to top)
};

/**
 * @brief Flexbox wrap behavior
 */
enum class FlexWrap : u8 {
    NoWrap,         ///< Single line
    Wrap,           ///< Multi-line (top to bottom / left to right)
    WrapReverse     ///< Multi-line (bottom to top / right to left)
};

/**
 * @brief Main axis alignment (justify-content)
 */
enum class JustifyContent : u8 {
    FlexStart,      ///< Pack items at start
    FlexEnd,        ///< Pack items at end
    Center,         ///< Pack items at center
    SpaceBetween,   ///< Distribute items evenly (first at start, last at end)
    SpaceAround,    ///< Distribute items evenly (equal space around)
    SpaceEvenly     ///< Distribute items evenly (equal space between)
};

/**
 * @brief Cross axis alignment (align-items)
 */
enum class AlignItems : u8 {
    FlexStart,      ///< Align to start
    FlexEnd,        ///< Align to end
    Center,         ///< Align to center
    Baseline,       ///< Align to baseline
    Stretch         ///< Stretch to fill
};

/**
 * @brief Self alignment (align-self)
 */
enum class AlignSelf : u8 {
    Auto,           ///< Inherit from parent
    FlexStart,
    FlexEnd,
    Center,
    Baseline,
    Stretch
};

/**
 * @brief Content alignment for multi-line containers (align-content)
 */
enum class AlignContent : u8 {
    FlexStart,
    FlexEnd,
    Center,
    SpaceBetween,
    SpaceAround,
    Stretch
};

/**
 * @brief Position type
 */
enum class PositionType : u8 {
    Relative,       ///< Normal flow position
    Absolute        ///< Absolute position relative to parent
};

/**
 * @brief Display type
 */
enum class Display : u8 {
    Flex,           ///< Flexbox container
    None            ///< Hidden
};

/**
 * @brief Overflow behavior
 */
enum class Overflow : u8 {
    Visible,        ///< Show overflow
    Hidden,         ///< Clip overflow
    Scroll          ///< Scrollable
};

/**
 * @brief Text alignment
 */
enum class TextAlign : u8 {
    Left,
    Center,
    Right,
    Justify
};

/**
 * @brief Text overflow behavior
 */
enum class TextOverflow : u8 {
    Clip,           ///< Clip text
    Ellipsis,       ///< Show "..."
    Fade            ///< Fade out
};

/**
 * @brief Font weight
 */
enum class FontWeight : u16 {
    Thin = 100,
    ExtraLight = 200,
    Light = 300,
    Normal = 400,
    Medium = 500,
    SemiBold = 600,
    Bold = 700,
    ExtraBold = 800,
    Black = 900
};

/**
 * @brief Font style
 */
enum class FontStyle : u8 {
    Normal,
    Italic,
    Oblique
};

// ============================================================================
// Dimension Types
// ============================================================================

/**
 * @brief Dimension value (can be pixels, percent, or auto)
 */
struct Dimension {
    enum class Unit : u8 {
        Auto,       ///< Auto size
        Pixels,     ///< Fixed pixel size
        Percent     ///< Percentage of parent
    };
    
    f32 value = 0.0f;
    Unit unit = Unit::Auto;
    
    constexpr Dimension() noexcept = default;
    constexpr Dimension(f32 v, Unit u) noexcept : value(v), unit(u) {}
    
    [[nodiscard]] constexpr bool isAuto() const noexcept { return unit == Unit::Auto; }
    [[nodiscard]] constexpr bool isPixels() const noexcept { return unit == Unit::Pixels; }
    [[nodiscard]] constexpr bool isPercent() const noexcept { return unit == Unit::Percent; }
    
    /// Resolve dimension to pixels given parent size
    [[nodiscard]] constexpr f32 resolve(f32 parentSize) const noexcept {
        switch (unit) {
            case Unit::Pixels: return value;
            case Unit::Percent: return value * parentSize / 100.0f;
            default: return 0.0f;
        }
    }
    
    static constexpr Dimension auto_() noexcept { return Dimension(0, Unit::Auto); }
    static constexpr Dimension pixels(f32 px) noexcept { return Dimension(px, Unit::Pixels); }
    static constexpr Dimension percent(f32 pct) noexcept { return Dimension(pct, Unit::Percent); }
};

// ============================================================================
// Style Types
// ============================================================================

/**
 * @brief Border style
 */
struct BorderStyle {
    f32 width = 0.0f;
    Color color = Color::black();
    CornerRadii radii;
    
    constexpr BorderStyle() noexcept = default;
    constexpr BorderStyle(f32 w, Color c, f32 radius = 0.0f) noexcept
        : width(w), color(c), radii(radius) {}
};

/**
 * @brief Shadow style
 */
struct ShadowStyle {
    f32 offsetX = 0.0f;
    f32 offsetY = 2.0f;
    f32 blur = 4.0f;
    f32 spread = 0.0f;
    Color color = Color(0, 0, 0, 0.25f);
    bool inset = false;
    
    constexpr ShadowStyle() noexcept = default;
    constexpr ShadowStyle(f32 ox, f32 oy, f32 b, f32 s, Color c, bool ins = false) noexcept
        : offsetX(ox), offsetY(oy), blur(b), spread(s), color(c), inset(ins) {}
};

/**
 * @brief Text style
 */
struct TextStyle {
    std::string fontFamily = "default";
    f32 fontSize = DEFAULT_FONT_SIZE;
    FontWeight fontWeight = FontWeight::Normal;
    FontStyle fontStyle = FontStyle::Normal;
    Color color = Color::black();
    f32 lineHeight = DEFAULT_LINE_HEIGHT;
    f32 letterSpacing = 0.0f;
    TextAlign textAlign = TextAlign::Left;
    TextOverflow overflow = TextOverflow::Clip;
    bool underline = false;
    bool strikethrough = false;
};

/**
 * @brief Complete widget style
 */
struct Style {
    // Layout
    Display display = Display::Flex;
    PositionType position = PositionType::Relative;
    FlexDirection flexDirection = FlexDirection::Row;
    FlexWrap flexWrap = FlexWrap::NoWrap;
    JustifyContent justifyContent = JustifyContent::FlexStart;
    AlignItems alignItems = AlignItems::Stretch;
    AlignContent alignContent = AlignContent::Stretch;
    AlignSelf alignSelf = AlignSelf::Auto;
    
    // Dimensions
    Dimension width = Dimension::auto_();
    Dimension height = Dimension::auto_();
    Dimension minWidth = Dimension::auto_();
    Dimension minHeight = Dimension::auto_();
    Dimension maxWidth = Dimension::auto_();
    Dimension maxHeight = Dimension::auto_();
    
    // Flex
    f32 flexGrow = 0.0f;
    f32 flexShrink = 1.0f;
    Dimension flexBasis = Dimension::auto_();
    
    // Spacing
    EdgeInsets margin;
    EdgeInsets padding;
    
    // Position (for absolute positioning)
    Dimension left = Dimension::auto_();
    Dimension top = Dimension::auto_();
    Dimension right = Dimension::auto_();
    Dimension bottom = Dimension::auto_();
    
    // Visual
    Color backgroundColor = Color::transparent();
    BorderStyle border;
    ShadowStyle shadow;
    f32 opacity = 1.0f;
    Overflow overflow = Overflow::Visible;
    
    // Transform
    Vec2 transformOrigin = Vec2(0.5f, 0.5f);
    Vec2 scale = Vec2(1.0f, 1.0f);
    f32 rotation = 0.0f;
    Vec2 translation = Vec2(0.0f, 0.0f);
    
    // Text (for text widgets)
    TextStyle text;
    
    // Interaction
    bool pointerEvents = true;  ///< Whether widget receives pointer events
    
    // Z-index
    i32 zIndex = 0;
};

// ============================================================================
// Input Types
// ============================================================================

/**
 * @brief Pointer type (mouse, touch, pen)
 */
enum class PointerType : u8 {
    Mouse,
    Touch,
    Pen
};

/**
 * @brief Pointer button
 */
enum class PointerButton : u8 {
    None = 0,
    Primary = 1,    ///< Left mouse / primary touch
    Secondary = 2,  ///< Right mouse
    Middle = 4,     ///< Middle mouse
    Back = 8,       ///< Mouse back button
    Forward = 16    ///< Mouse forward button
};

/**
 * @brief Pointer event data
 */
struct PointerEvent {
    PointerType type = PointerType::Mouse;
    u32 pointerId = 0;
    Vec2 position;          ///< Position in widget coordinates
    Vec2 screenPosition;    ///< Position in screen coordinates
    Vec2 delta;             ///< Movement since last event
    PointerButton button = PointerButton::None;
    u8 buttons = 0;         ///< Bitmask of pressed buttons
    f32 pressure = 1.0f;    ///< Pressure (0-1) for touch/pen
    f32 tiltX = 0.0f;       ///< Tilt for pen
    f32 tiltY = 0.0f;
    bool isPrimary = true;
    u32 timestamp = 0;
    
    [[nodiscard]] bool isTouch() const noexcept { return type == PointerType::Touch; }
    [[nodiscard]] bool isMouse() const noexcept { return type == PointerType::Mouse; }
    [[nodiscard]] bool isPen() const noexcept { return type == PointerType::Pen; }
};

/**
 * @brief Key event data
 */
struct KeyEvent {
    u32 keyCode = 0;
    u32 scanCode = 0;
    std::string key;        ///< Key name (e.g., "Enter", "a")
    bool isDown = false;
    bool isRepeat = false;
    bool shift = false;
    bool ctrl = false;
    bool alt = false;
    bool meta = false;      ///< Cmd on Mac, Win on Windows
    u32 timestamp = 0;
};

/**
 * @brief Text input event
 */
struct TextInputEvent {
    std::string text;
    u32 timestamp = 0;
};

/**
 * @brief Scroll event
 */
struct ScrollEvent {
    Vec2 delta;
    Vec2 position;
    bool isInertial = false;
    u32 timestamp = 0;
};

/**
 * @brief Focus event
 */
struct FocusEvent {
    Widget* relatedTarget = nullptr;  ///< Widget losing/gaining focus
    u32 timestamp = 0;
};

/**
 * @brief Gesture type
 */
enum class GestureType : u8 {
    Tap,
    DoubleTap,
    LongPress,
    Pan,
    Pinch,
    Rotate,
    Swipe
};

/**
 * @brief Gesture state
 */
enum class GestureState : u8 {
    Possible,
    Began,
    Changed,
    Ended,
    Cancelled,
    Failed
};

/**
 * @brief Gesture event
 */
struct GestureEvent {
    GestureType type = GestureType::Tap;
    GestureState state = GestureState::Ended;
    Vec2 position;
    Vec2 velocity;
    f32 scale = 1.0f;       ///< For pinch
    f32 rotation = 0.0f;    ///< For rotate (radians)
    u32 tapCount = 1;
    u32 timestamp = 0;
};

// ============================================================================
// Callbacks
// ============================================================================

/// Pointer event callback
using PointerCallback = std::function<void(const PointerEvent&)>;

/// Key event callback
using KeyCallback = std::function<void(const KeyEvent&)>;

/// Text input callback
using TextInputCallback = std::function<void(const TextInputEvent&)>;

/// Scroll callback
using ScrollCallback = std::function<void(const ScrollEvent&)>;

/// Focus callback
using FocusCallback = std::function<void(const FocusEvent&)>;

/// Gesture callback
using GestureCallback = std::function<void(const GestureEvent&)>;

/// Generic callback (e.g., for button press)
using ActionCallback = std::function<void()>;

/// Value change callback
template<typename T>
using ValueChangeCallback = std::function<void(const T&)>;

// ============================================================================
// Widget Handle
// ============================================================================

/**
 * @brief Handle to a widget
 */
struct WidgetHandle {
    u64 value = 0;
    
    [[nodiscard]] constexpr bool isValid() const noexcept { return value != 0; }
    constexpr explicit operator bool() const noexcept { return isValid(); }
    
    constexpr bool operator==(WidgetHandle other) const noexcept { return value == other.value; }
    constexpr bool operator!=(WidgetHandle other) const noexcept { return value != other.value; }
};

// ============================================================================
// Animation Types
// ============================================================================

/**
 * @brief Easing function type
 */
enum class EasingType : u8 {
    Linear,
    EaseIn,
    EaseOut,
    EaseInOut,
    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad,
    EaseInCubic,
    EaseOutCubic,
    EaseInOutCubic,
    EaseInElastic,
    EaseOutElastic,
    EaseInOutElastic,
    EaseInBounce,
    EaseOutBounce,
    EaseInOutBounce
};

/**
 * @brief Animation property to animate
 */
enum class AnimationProperty : u8 {
    Opacity,
    PositionX,
    PositionY,
    ScaleX,
    ScaleY,
    Rotation,
    BackgroundColorR,
    BackgroundColorG,
    BackgroundColorB,
    BackgroundColorA,
    Width,
    Height
};

/**
 * @brief Single property animation
 */
struct PropertyAnimation {
    AnimationProperty property = AnimationProperty::Opacity;
    f32 from = 0.0f;
    f32 to = 1.0f;
    f32 duration = DEFAULT_ANIMATION_DURATION;
    f32 delay = 0.0f;
    EasingType easing = EasingType::EaseInOut;
    i32 repeatCount = 0;  ///< -1 for infinite
    bool autoReverse = false;
};

/**
 * @brief Animation state
 */
struct AnimationState {
    u64 id = 0;
    f32 progress = 0.0f;
    f32 elapsed = 0.0f;
    bool isPlaying = false;
    bool isComplete = false;
    i32 currentRepeat = 0;
    bool reversing = false;
};

// ============================================================================
// Theme Types
// ============================================================================

/**
 * @brief Theme color set
 */
struct ThemeColors {
    Color primary = Color::fromHex(0x6200EE);
    Color primaryVariant = Color::fromHex(0x3700B3);
    Color secondary = Color::fromHex(0x03DAC6);
    Color secondaryVariant = Color::fromHex(0x018786);
    Color background = Color::white();
    Color surface = Color::white();
    Color error = Color::fromHex(0xB00020);
    Color onPrimary = Color::white();
    Color onSecondary = Color::black();
    Color onBackground = Color::black();
    Color onSurface = Color::black();
    Color onError = Color::white();
    
    // Additional semantic colors
    Color success = Color::fromHex(0x4CAF50);
    Color warning = Color::fromHex(0xFFC107);
    Color info = Color::fromHex(0x2196F3);
    Color divider = Color(0, 0, 0, 0.12f);
    Color disabled = Color(0, 0, 0, 0.38f);
};

/**
 * @brief Theme typography
 */
struct ThemeTypography {
    TextStyle headline1;
    TextStyle headline2;
    TextStyle headline3;
    TextStyle headline4;
    TextStyle headline5;
    TextStyle headline6;
    TextStyle subtitle1;
    TextStyle subtitle2;
    TextStyle body1;
    TextStyle body2;
    TextStyle button;
    TextStyle caption;
    TextStyle overline;
    
    /// Create default typography
    static ThemeTypography defaultTypography();
};

/**
 * @brief Complete theme
 */
struct Theme {
    std::string name = "default";
    ThemeColors colors;
    ThemeTypography typography;
    
    // Spacing scale
    f32 spacing[8] = {0, 4, 8, 16, 24, 32, 48, 64};
    
    // Border radius scale
    f32 borderRadius[4] = {0, 4, 8, 16};
    
    // Shadow presets
    ShadowStyle elevation[6];
    
    /// Create light theme
    static Theme light();
    
    /// Create dark theme
    static Theme dark();
};

// ============================================================================
// Accessibility Types
// ============================================================================

/**
 * @brief Accessibility role
 */
enum class AccessibilityRole : u8 {
    None,
    Button,
    CheckBox,
    ComboBox,
    Dialog,
    Grid,
    GridCell,
    Heading,
    Image,
    Link,
    List,
    ListItem,
    Menu,
    MenuItem,
    ProgressBar,
    RadioButton,
    ScrollBar,
    Slider,
    SpinButton,
    Switch,
    Tab,
    TabList,
    TabPanel,
    TextBox,
    Timer,
    Tooltip,
    Tree,
    TreeItem
};

/**
 * @brief Accessibility properties
 */
struct AccessibilityInfo {
    AccessibilityRole role = AccessibilityRole::None;
    std::string label;
    std::string hint;
    std::string value;
    bool hidden = false;
    bool isButton = false;
    bool isHeader = false;
    bool isSelected = false;
    bool isExpanded = false;
    bool isChecked = false;
    bool isDisabled = false;
    bool isBusy = false;
    i32 positionInSet = -1;
    i32 setSize = -1;
    i32 level = -1;
};

} // namespace nova::ui
