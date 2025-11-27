/**
 * @file widget.hpp
 * @brief Nova UI™ - Base Widget class and widget hierarchy
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "ui_types.hpp"
#include "nova/core/types/result.hpp"

#include <vector>
#include <memory>
#include <string>

namespace nova::ui {

/**
 * @brief Base class for all UI widgets
 * 
 * Widgets form a tree hierarchy. Each widget has a style, can receive events,
 * and participates in the layout system.
 */
class Widget {
public:
    Widget();
    virtual ~Widget();
    
    // Disable copy, enable move
    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget(Widget&&) noexcept;
    Widget& operator=(Widget&&) noexcept;
    
    // =========================================================================
    // Identity
    // =========================================================================
    
    /// Get widget handle
    [[nodiscard]] WidgetHandle getHandle() const noexcept { return m_handle; }
    
    /// Get widget ID (user-assigned)
    [[nodiscard]] const std::string& getId() const noexcept { return m_id; }
    
    /// Set widget ID
    void setId(const std::string& id) { m_id = id; }
    
    /// Get widget type name
    [[nodiscard]] virtual const char* getTypeName() const noexcept { return "Widget"; }
    
    // =========================================================================
    // Hierarchy
    // =========================================================================
    
    /// Get parent widget
    [[nodiscard]] Widget* getParent() const noexcept { return m_parent; }
    
    /// Get children
    [[nodiscard]] const std::vector<std::unique_ptr<Widget>>& getChildren() const noexcept { 
        return m_children; 
    }
    
    /// Get child count
    [[nodiscard]] usize getChildCount() const noexcept { return m_children.size(); }
    
    /// Get child at index
    [[nodiscard]] Widget* getChildAt(usize index) const;
    
    /// Add child widget
    void addChild(std::unique_ptr<Widget> child);
    
    /// Insert child at index
    void insertChild(usize index, std::unique_ptr<Widget> child);
    
    /// Remove child by pointer (returns ownership)
    std::unique_ptr<Widget> removeChild(Widget* child);
    
    /// Remove child at index (returns ownership)
    std::unique_ptr<Widget> removeChildAt(usize index);
    
    /// Remove all children
    void clearChildren();
    
    /// Find widget by ID in subtree
    [[nodiscard]] Widget* findById(const std::string& id);
    
    /// Find widget by handle in subtree
    [[nodiscard]] Widget* findByHandle(WidgetHandle handle);
    
    /// Get depth in tree
    [[nodiscard]] u32 getDepth() const noexcept;
    
    /// Check if widget is ancestor of another
    [[nodiscard]] bool isAncestorOf(const Widget* widget) const;
    
    // =========================================================================
    // Layout
    // =========================================================================
    
    /// Get style (mutable)
    [[nodiscard]] Style& getStyle() noexcept { return m_style; }
    
    /// Get style (const)
    [[nodiscard]] const Style& getStyle() const noexcept { return m_style; }
    
    /// Set style
    void setStyle(const Style& style);
    
    /// Get computed layout bounds (after layout pass)
    [[nodiscard]] const Rect& getBounds() const noexcept { return m_bounds; }
    
    /// Get content bounds (bounds minus padding)
    [[nodiscard]] Rect getContentBounds() const noexcept;
    
    /// Mark layout as dirty (needs recalculation)
    void markLayoutDirty();
    
    /// Check if layout is dirty
    [[nodiscard]] bool isLayoutDirty() const noexcept { return m_layoutDirty; }
    
    /// Perform layout (called by layout system)
    void layout(const Rect& availableSpace);
    
    // =========================================================================
    // Visibility & State
    // =========================================================================
    
    /// Check if visible
    [[nodiscard]] bool isVisible() const noexcept { return m_visible; }
    
    /// Set visibility
    void setVisible(bool visible);
    
    /// Check if enabled
    [[nodiscard]] bool isEnabled() const noexcept { return m_enabled; }
    
    /// Set enabled state
    void setEnabled(bool enabled);
    
    /// Check if focused
    [[nodiscard]] bool isFocused() const noexcept { return m_focused; }
    
    /// Check if hovered
    [[nodiscard]] bool isHovered() const noexcept { return m_hovered; }
    
    /// Check if pressed
    [[nodiscard]] bool isPressed() const noexcept { return m_pressed; }
    
    /// Request focus
    void requestFocus();
    
    /// Release focus
    void releaseFocus();
    
    // =========================================================================
    // Events
    // =========================================================================
    
    /// Set pointer down callback
    void setOnPointerDown(PointerCallback callback) { m_onPointerDown = std::move(callback); }
    
    /// Set pointer up callback
    void setOnPointerUp(PointerCallback callback) { m_onPointerUp = std::move(callback); }
    
    /// Set pointer move callback
    void setOnPointerMove(PointerCallback callback) { m_onPointerMove = std::move(callback); }
    
    /// Set pointer enter callback
    void setOnPointerEnter(PointerCallback callback) { m_onPointerEnter = std::move(callback); }
    
    /// Set pointer leave callback
    void setOnPointerLeave(PointerCallback callback) { m_onPointerLeave = std::move(callback); }
    
    /// Set key down callback
    void setOnKeyDown(KeyCallback callback) { m_onKeyDown = std::move(callback); }
    
    /// Set key up callback
    void setOnKeyUp(KeyCallback callback) { m_onKeyUp = std::move(callback); }
    
    /// Set focus callback
    void setOnFocus(FocusCallback callback) { m_onFocus = std::move(callback); }
    
    /// Set blur callback
    void setOnBlur(FocusCallback callback) { m_onBlur = std::move(callback); }
    
    /// Set scroll callback
    void setOnScroll(ScrollCallback callback) { m_onScroll = std::move(callback); }
    
    // =========================================================================
    // Hit Testing
    // =========================================================================
    
    /// Check if point is inside widget
    [[nodiscard]] bool containsPoint(Vec2 point) const;
    
    /// Hit test (find deepest widget at point)
    [[nodiscard]] Widget* hitTest(Vec2 point);
    
    // =========================================================================
    // Accessibility
    // =========================================================================
    
    /// Get accessibility info
    [[nodiscard]] AccessibilityInfo& getAccessibility() noexcept { return m_accessibility; }
    [[nodiscard]] const AccessibilityInfo& getAccessibility() const noexcept { return m_accessibility; }
    
    /// Set accessibility label
    void setAccessibilityLabel(const std::string& label) { m_accessibility.label = label; }
    
    /// Set accessibility hint
    void setAccessibilityHint(const std::string& hint) { m_accessibility.hint = hint; }
    
    // =========================================================================
    // Animation
    // =========================================================================
    
    /// Start animation
    AnimationState& startAnimation(const PropertyAnimation& animation);
    
    /// Stop animation by ID
    void stopAnimation(u64 animationId);
    
    /// Stop all animations
    void stopAllAnimations();
    
    /// Update animations (called by UI system)
    void updateAnimations(f32 deltaTime);
    
    // =========================================================================
    // Measurement (public for layout calculations)
    // =========================================================================
    
    /// Measure preferred content size (can be overridden)
    virtual Vec2 measureContent(f32 availableWidth, f32 availableHeight);
    
protected:
    // =========================================================================
    // Virtual Methods for Subclasses
    // =========================================================================
    
    /// Called when style changes
    virtual void onStyleChanged();
    
    /// Called when bounds change after layout
    virtual void onBoundsChanged();
    
    /// Called to layout children (for containers)
    virtual void layoutChildren();
    
    /// Called when pointer down occurs
    virtual bool onPointerDown(const PointerEvent& event);
    
    /// Called when pointer up occurs
    virtual bool onPointerUp(const PointerEvent& event);
    
    /// Called when pointer moves
    virtual bool onPointerMove(const PointerEvent& event);
    
    /// Called when pointer enters
    virtual void onPointerEnter(const PointerEvent& event);
    
    /// Called when pointer leaves
    virtual void onPointerLeave(const PointerEvent& event);
    
    /// Called when key pressed
    virtual bool onKeyDown(const KeyEvent& event);
    
    /// Called when key released
    virtual bool onKeyUp(const KeyEvent& event);
    
    /// Called when focused
    virtual void onFocus(const FocusEvent& event);
    
    /// Called when focus lost
    virtual void onBlur(const FocusEvent& event);
    
    /// Called when scrolled
    virtual bool onScroll(const ScrollEvent& event);
    
    // =========================================================================
    // Protected members for derived classes
    // =========================================================================
    
    std::vector<std::unique_ptr<Widget>> m_children;
    Style m_style;
    Rect m_bounds;
    bool m_layoutDirty = true;
    bool m_visible = true;
    bool m_enabled = true;
    bool m_focused = false;
    bool m_hovered = false;
    bool m_pressed = false;
    AccessibilityInfo m_accessibility;
    
private:
    friend class UISystem;
    
    WidgetHandle m_handle;
    std::string m_id;
    Widget* m_parent = nullptr;
    
    // Callbacks
    PointerCallback m_onPointerDown;
    PointerCallback m_onPointerUp;
    PointerCallback m_onPointerMove;
    PointerCallback m_onPointerEnter;
    PointerCallback m_onPointerLeave;
    KeyCallback m_onKeyDown;
    KeyCallback m_onKeyUp;
    FocusCallback m_onFocus;
    FocusCallback m_onBlur;
    ScrollCallback m_onScroll;
    
    // Animations
    std::vector<std::pair<PropertyAnimation, AnimationState>> m_animations;
    u64 m_nextAnimationId = 1;
    
    static u64 s_nextHandleId;
};

// ============================================================================
// Common Widget Types
// ============================================================================

/**
 * @brief Container widget (flexbox layout)
 */
class Container : public Widget {
public:
    Container();
    
    [[nodiscard]] const char* getTypeName() const noexcept override { return "Container"; }
    
protected:
    void layoutChildren() override;
};

/**
 * @brief Text label widget
 */
class Label : public Widget {
public:
    Label();
    explicit Label(const std::string& text);
    
    [[nodiscard]] const char* getTypeName() const noexcept override { return "Label"; }
    
    /// Get text content
    [[nodiscard]] const std::string& getText() const noexcept { return m_text; }
    
    /// Set text content
    void setText(const std::string& text);
    
    /// Get max lines (0 = unlimited)
    [[nodiscard]] u32 getMaxLines() const noexcept { return m_maxLines; }
    
    /// Set max lines
    void setMaxLines(u32 lines) { m_maxLines = lines; markLayoutDirty(); }
    
protected:
    Vec2 measureContent(f32 availableWidth, f32 availableHeight) override;
    
private:
    std::string m_text;
    u32 m_maxLines = 0;
};

/**
 * @brief Clickable button widget
 */
class Button : public Widget {
public:
    Button();
    explicit Button(const std::string& text);
    
    [[nodiscard]] const char* getTypeName() const noexcept override { return "Button"; }
    
    /// Get button text
    [[nodiscard]] const std::string& getText() const noexcept { return m_text; }
    
    /// Set button text
    void setText(const std::string& text);
    
    /// Set click callback
    void setOnClick(ActionCallback callback) { m_onClick = std::move(callback); }
    
protected:
    bool onPointerDown(const PointerEvent& event) override;
    bool onPointerUp(const PointerEvent& event) override;
    void onPointerEnter(const PointerEvent& event) override;
    void onPointerLeave(const PointerEvent& event) override;
    Vec2 measureContent(f32 availableWidth, f32 availableHeight) override;
    
private:
    std::string m_text;
    ActionCallback m_onClick;
};

/**
 * @brief Image display widget
 */
class Image : public Widget {
public:
    Image();
    
    [[nodiscard]] const char* getTypeName() const noexcept override { return "Image"; }
    
    /// Set image source (path or URL)
    void setSource(const std::string& source);
    
    /// Get image source
    [[nodiscard]] const std::string& getSource() const noexcept { return m_source; }
    
    /// Set aspect ratio mode
    enum class AspectRatio : u8 {
        Fill,       ///< Stretch to fill
        Contain,    ///< Scale to fit, maintain aspect
        Cover,      ///< Scale to cover, maintain aspect, clip
        None        ///< No scaling
    };
    
    void setAspectRatio(AspectRatio mode) { m_aspectRatio = mode; }
    [[nodiscard]] AspectRatio getAspectRatio() const noexcept { return m_aspectRatio; }
    
protected:
    Vec2 measureContent(f32 availableWidth, f32 availableHeight) override;
    
private:
    std::string m_source;
    AspectRatio m_aspectRatio = AspectRatio::Contain;
    Vec2 m_naturalSize;  // Original image dimensions
};

/**
 * @brief Text input field widget
 */
class TextInput : public Widget {
public:
    TextInput();
    
    [[nodiscard]] const char* getTypeName() const noexcept override { return "TextInput"; }
    
    /// Get text value
    [[nodiscard]] const std::string& getValue() const noexcept { return m_value; }
    
    /// Set text value
    void setValue(const std::string& value);
    
    /// Get placeholder text
    [[nodiscard]] const std::string& getPlaceholder() const noexcept { return m_placeholder; }
    
    /// Set placeholder text
    void setPlaceholder(const std::string& placeholder) { m_placeholder = placeholder; }
    
    /// Check if password mode
    [[nodiscard]] bool isPassword() const noexcept { return m_isPassword; }
    
    /// Set password mode
    void setPassword(bool password) { m_isPassword = password; }
    
    /// Check if multiline
    [[nodiscard]] bool isMultiline() const noexcept { return m_multiline; }
    
    /// Set multiline mode
    void setMultiline(bool multiline) { m_multiline = multiline; markLayoutDirty(); }
    
    /// Set value change callback
    void setOnChange(ValueChangeCallback<std::string> callback) { m_onChange = std::move(callback); }
    
    /// Set submit callback (enter pressed)
    void setOnSubmit(ActionCallback callback) { m_onSubmit = std::move(callback); }
    
protected:
    bool onPointerDown(const PointerEvent& event) override;
    bool onKeyDown(const KeyEvent& event) override;
    void onFocus(const FocusEvent& event) override;
    void onBlur(const FocusEvent& event) override;
    Vec2 measureContent(f32 availableWidth, f32 availableHeight) override;
    
private:
    std::string m_value;
    std::string m_placeholder;
    bool m_isPassword = false;
    bool m_multiline = false;
    usize m_cursorPosition = 0;
    usize m_selectionStart = 0;
    usize m_selectionEnd = 0;
    
    ValueChangeCallback<std::string> m_onChange;
    ActionCallback m_onSubmit;
};

/**
 * @brief Scrollable container widget
 */
class ScrollView : public Widget {
public:
    ScrollView();
    
    [[nodiscard]] const char* getTypeName() const noexcept override { return "ScrollView"; }
    
    /// Get scroll offset
    [[nodiscard]] Vec2 getScrollOffset() const noexcept { return m_scrollOffset; }
    
    /// Set scroll offset
    void setScrollOffset(Vec2 offset);
    
    /// Scroll to position
    void scrollTo(Vec2 position, bool animated = true);
    
    /// Scroll to widget
    void scrollToWidget(Widget* widget, bool animated = true);
    
    /// Enable/disable horizontal scroll
    void setHorizontalScrollEnabled(bool enabled) { m_horizontalEnabled = enabled; }
    [[nodiscard]] bool isHorizontalScrollEnabled() const noexcept { return m_horizontalEnabled; }
    
    /// Enable/disable vertical scroll
    void setVerticalScrollEnabled(bool enabled) { m_verticalEnabled = enabled; }
    [[nodiscard]] bool isVerticalScrollEnabled() const noexcept { return m_verticalEnabled; }
    
    /// Show/hide scroll indicators
    void setShowIndicators(bool show) { m_showIndicators = show; }
    [[nodiscard]] bool getShowIndicators() const noexcept { return m_showIndicators; }
    
protected:
    void layoutChildren() override;
    bool onScroll(const ScrollEvent& event) override;
    bool onPointerDown(const PointerEvent& event) override;
    bool onPointerMove(const PointerEvent& event) override;
    bool onPointerUp(const PointerEvent& event) override;
    
private:
    Vec2 m_scrollOffset;
    Vec2 m_contentSize;
    Vec2 m_velocity;
    bool m_horizontalEnabled = false;
    bool m_verticalEnabled = true;
    bool m_showIndicators = true;
    bool m_isDragging = false;
    Vec2 m_dragStart;
    Vec2 m_scrollStart;
};

/**
 * @brief Checkbox widget
 */
class Checkbox : public Widget {
public:
    Checkbox();
    
    [[nodiscard]] const char* getTypeName() const noexcept override { return "Checkbox"; }
    
    /// Get checked state
    [[nodiscard]] bool isChecked() const noexcept { return m_checked; }
    
    /// Set checked state
    void setChecked(bool checked);
    
    /// Toggle checked state
    void toggle() { setChecked(!m_checked); }
    
    /// Get label text
    [[nodiscard]] const std::string& getLabel() const noexcept { return m_label; }
    
    /// Set label text
    void setLabel(const std::string& label) { m_label = label; markLayoutDirty(); }
    
    /// Set change callback
    void setOnChange(ValueChangeCallback<bool> callback) { m_onChange = std::move(callback); }
    
protected:
    bool onPointerUp(const PointerEvent& event) override;
    bool onKeyDown(const KeyEvent& event) override;
    Vec2 measureContent(f32 availableWidth, f32 availableHeight) override;
    
private:
    bool m_checked = false;
    std::string m_label;
    ValueChangeCallback<bool> m_onChange;
};

/**
 * @brief Slider widget
 */
class Slider : public Widget {
public:
    Slider();
    
    [[nodiscard]] const char* getTypeName() const noexcept override { return "Slider"; }
    
    /// Get current value
    [[nodiscard]] f32 getValue() const noexcept { return m_value; }
    
    /// Set current value
    void setValue(f32 value);
    
    /// Get minimum value
    [[nodiscard]] f32 getMin() const noexcept { return m_min; }
    
    /// Set minimum value
    void setMin(f32 min) { m_min = min; setValue(m_value); }
    
    /// Get maximum value
    [[nodiscard]] f32 getMax() const noexcept { return m_max; }
    
    /// Set maximum value
    void setMax(f32 max) { m_max = max; setValue(m_value); }
    
    /// Get step (0 = continuous)
    [[nodiscard]] f32 getStep() const noexcept { return m_step; }
    
    /// Set step
    void setStep(f32 step) { m_step = step; }
    
    /// Set change callback
    void setOnChange(ValueChangeCallback<f32> callback) { m_onChange = std::move(callback); }
    
protected:
    bool onPointerDown(const PointerEvent& event) override;
    bool onPointerMove(const PointerEvent& event) override;
    bool onPointerUp(const PointerEvent& event) override;
    Vec2 measureContent(f32 availableWidth, f32 availableHeight) override;
    
private:
    f32 m_value = 0.0f;
    f32 m_min = 0.0f;
    f32 m_max = 1.0f;
    f32 m_step = 0.0f;
    bool m_isDragging = false;
    ValueChangeCallback<f32> m_onChange;
    
    void updateValueFromPosition(Vec2 position);
};

/**
 * @brief Progress bar widget
 */
class ProgressBar : public Widget {
public:
    ProgressBar();
    
    [[nodiscard]] const char* getTypeName() const noexcept override { return "ProgressBar"; }
    
    /// Get progress (0-1)
    [[nodiscard]] f32 getProgress() const noexcept { return m_progress; }
    
    /// Set progress (0-1)
    void setProgress(f32 progress);
    
    /// Check if indeterminate mode
    [[nodiscard]] bool isIndeterminate() const noexcept { return m_indeterminate; }
    
    /// Set indeterminate mode
    void setIndeterminate(bool indeterminate) { m_indeterminate = indeterminate; }
    
protected:
    Vec2 measureContent(f32 availableWidth, f32 availableHeight) override;
    
private:
    f32 m_progress = 0.0f;
    bool m_indeterminate = false;
};

} // namespace nova::ui
