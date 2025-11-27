/**
 * @file ui_system.cpp
 * @brief Nova UI™ - UI System implementation
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include "nova/core/ui/ui_system.hpp"
#include "nova/core/logging/logging.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>

namespace nova::ui {

using namespace nova::logging;

// ============================================================================
// Widget Static Member
// ============================================================================

u64 Widget::s_nextHandleId = 1;

// ============================================================================
// Theme Implementation
// ============================================================================

ThemeTypography ThemeTypography::defaultTypography() {
    ThemeTypography t;
    
    // Headlines
    t.headline1.fontSize = 96.0f;
    t.headline1.fontWeight = FontWeight::Light;
    t.headline1.letterSpacing = -1.5f;
    
    t.headline2.fontSize = 60.0f;
    t.headline2.fontWeight = FontWeight::Light;
    t.headline2.letterSpacing = -0.5f;
    
    t.headline3.fontSize = 48.0f;
    t.headline3.fontWeight = FontWeight::Normal;
    
    t.headline4.fontSize = 34.0f;
    t.headline4.fontWeight = FontWeight::Normal;
    t.headline4.letterSpacing = 0.25f;
    
    t.headline5.fontSize = 24.0f;
    t.headline5.fontWeight = FontWeight::Normal;
    
    t.headline6.fontSize = 20.0f;
    t.headline6.fontWeight = FontWeight::Medium;
    t.headline6.letterSpacing = 0.15f;
    
    // Subtitles
    t.subtitle1.fontSize = 16.0f;
    t.subtitle1.fontWeight = FontWeight::Normal;
    t.subtitle1.letterSpacing = 0.15f;
    
    t.subtitle2.fontSize = 14.0f;
    t.subtitle2.fontWeight = FontWeight::Medium;
    t.subtitle2.letterSpacing = 0.1f;
    
    // Body
    t.body1.fontSize = 16.0f;
    t.body1.fontWeight = FontWeight::Normal;
    t.body1.letterSpacing = 0.5f;
    
    t.body2.fontSize = 14.0f;
    t.body2.fontWeight = FontWeight::Normal;
    t.body2.letterSpacing = 0.25f;
    
    // Button
    t.button.fontSize = 14.0f;
    t.button.fontWeight = FontWeight::Medium;
    t.button.letterSpacing = 1.25f;
    
    // Caption
    t.caption.fontSize = 12.0f;
    t.caption.fontWeight = FontWeight::Normal;
    t.caption.letterSpacing = 0.4f;
    
    // Overline
    t.overline.fontSize = 10.0f;
    t.overline.fontWeight = FontWeight::Normal;
    t.overline.letterSpacing = 1.5f;
    
    return t;
}

Theme Theme::light() {
    Theme theme;
    theme.name = "light";
    theme.colors = ThemeColors{};
    theme.typography = ThemeTypography::defaultTypography();
    
    // Elevation shadows
    theme.elevation[0] = ShadowStyle{};
    theme.elevation[1] = ShadowStyle{0, 1, 3, 0, Color(0, 0, 0, 0.12f)};
    theme.elevation[2] = ShadowStyle{0, 3, 6, 0, Color(0, 0, 0, 0.16f)};
    theme.elevation[3] = ShadowStyle{0, 10, 20, 0, Color(0, 0, 0, 0.19f)};
    theme.elevation[4] = ShadowStyle{0, 14, 28, 0, Color(0, 0, 0, 0.22f)};
    theme.elevation[5] = ShadowStyle{0, 19, 38, 0, Color(0, 0, 0, 0.25f)};
    
    return theme;
}

Theme Theme::dark() {
    Theme theme;
    theme.name = "dark";
    
    theme.colors.primary = Color::fromHex(0xBB86FC);
    theme.colors.primaryVariant = Color::fromHex(0x3700B3);
    theme.colors.secondary = Color::fromHex(0x03DAC6);
    theme.colors.background = Color::fromHex(0x121212);
    theme.colors.surface = Color::fromHex(0x121212);
    theme.colors.error = Color::fromHex(0xCF6679);
    theme.colors.onPrimary = Color::black();
    theme.colors.onSecondary = Color::black();
    theme.colors.onBackground = Color::white();
    theme.colors.onSurface = Color::white();
    theme.colors.onError = Color::black();
    theme.colors.divider = Color(1, 1, 1, 0.12f);
    theme.colors.disabled = Color(1, 1, 1, 0.38f);
    
    theme.typography = ThemeTypography::defaultTypography();
    
    // Update text colors for dark theme
    theme.typography.headline1.color = Color::white();
    theme.typography.headline2.color = Color::white();
    theme.typography.headline3.color = Color::white();
    theme.typography.headline4.color = Color::white();
    theme.typography.headline5.color = Color::white();
    theme.typography.headline6.color = Color::white();
    theme.typography.subtitle1.color = Color::white();
    theme.typography.subtitle2.color = Color::white();
    theme.typography.body1.color = Color::white();
    theme.typography.body2.color = Color::white();
    theme.typography.button.color = Color::white();
    theme.typography.caption.color = Color::white();
    theme.typography.overline.color = Color::white();
    
    // Elevation shadows (lighter for dark theme)
    theme.elevation[0] = ShadowStyle{};
    theme.elevation[1] = ShadowStyle{0, 1, 3, 0, Color(0, 0, 0, 0.20f)};
    theme.elevation[2] = ShadowStyle{0, 3, 6, 0, Color(0, 0, 0, 0.24f)};
    theme.elevation[3] = ShadowStyle{0, 10, 20, 0, Color(0, 0, 0, 0.28f)};
    theme.elevation[4] = ShadowStyle{0, 14, 28, 0, Color(0, 0, 0, 0.32f)};
    theme.elevation[5] = ShadowStyle{0, 19, 38, 0, Color(0, 0, 0, 0.36f)};
    
    return theme;
}

// ============================================================================
// Widget Implementation
// ============================================================================

Widget::Widget() {
    m_handle.value = s_nextHandleId++;
}

Widget::~Widget() {
    stopAllAnimations();
}

Widget::Widget(Widget&& other) noexcept
    : m_children(std::move(other.m_children))
    , m_style(other.m_style)
    , m_bounds(other.m_bounds)
    , m_layoutDirty(other.m_layoutDirty)
    , m_visible(other.m_visible)
    , m_enabled(other.m_enabled)
    , m_focused(other.m_focused)
    , m_hovered(other.m_hovered)
    , m_pressed(other.m_pressed)
    , m_accessibility(std::move(other.m_accessibility))
    , m_handle(other.m_handle)
    , m_id(std::move(other.m_id))
    , m_parent(other.m_parent)
    , m_animations(std::move(other.m_animations))
    , m_nextAnimationId(other.m_nextAnimationId)
{
    other.m_handle.value = 0;
    other.m_parent = nullptr;
    
    // Update parent pointers of children
    for (auto& child : m_children) {
        child->m_parent = this;
    }
}

Widget& Widget::operator=(Widget&& other) noexcept {
    if (this != &other) {
        m_handle = other.m_handle;
        m_id = std::move(other.m_id);
        m_parent = other.m_parent;
        m_children = std::move(other.m_children);
        m_style = other.m_style;
        m_bounds = other.m_bounds;
        m_layoutDirty = other.m_layoutDirty;
        m_visible = other.m_visible;
        m_enabled = other.m_enabled;
        m_focused = other.m_focused;
        m_hovered = other.m_hovered;
        m_pressed = other.m_pressed;
        m_accessibility = std::move(other.m_accessibility);
        m_animations = std::move(other.m_animations);
        m_nextAnimationId = other.m_nextAnimationId;
        
        other.m_handle.value = 0;
        other.m_parent = nullptr;
        
        // Update parent pointers of children
        for (auto& child : m_children) {
            child->m_parent = this;
        }
    }
    return *this;
}

Widget* Widget::getChildAt(usize index) const {
    return index < m_children.size() ? m_children[index].get() : nullptr;
}

void Widget::addChild(std::unique_ptr<Widget> child) {
    if (child) {
        child->m_parent = this;
        m_children.push_back(std::move(child));
        markLayoutDirty();
    }
}

void Widget::insertChild(usize index, std::unique_ptr<Widget> child) {
    if (child) {
        child->m_parent = this;
        if (index >= m_children.size()) {
            m_children.push_back(std::move(child));
        } else {
            m_children.insert(m_children.begin() + static_cast<i32>(index), std::move(child));
        }
        markLayoutDirty();
    }
}

std::unique_ptr<Widget> Widget::removeChild(Widget* child) {
    auto it = std::find_if(m_children.begin(), m_children.end(),
        [child](const std::unique_ptr<Widget>& ptr) { return ptr.get() == child; });
    
    if (it != m_children.end()) {
        auto removed = std::move(*it);
        removed->m_parent = nullptr;
        m_children.erase(it);
        markLayoutDirty();
        return removed;
    }
    return nullptr;
}

std::unique_ptr<Widget> Widget::removeChildAt(usize index) {
    if (index < m_children.size()) {
        auto removed = std::move(m_children[index]);
        removed->m_parent = nullptr;
        m_children.erase(m_children.begin() + static_cast<i32>(index));
        markLayoutDirty();
        return removed;
    }
    return nullptr;
}

void Widget::clearChildren() {
    for (auto& child : m_children) {
        child->m_parent = nullptr;
    }
    m_children.clear();
    markLayoutDirty();
}

Widget* Widget::findById(const std::string& id) {
    if (m_id == id) {
        return this;
    }
    for (auto& child : m_children) {
        if (auto* found = child->findById(id)) {
            return found;
        }
    }
    return nullptr;
}

Widget* Widget::findByHandle(WidgetHandle handle) {
    if (m_handle == handle) {
        return this;
    }
    for (auto& child : m_children) {
        if (auto* found = child->findByHandle(handle)) {
            return found;
        }
    }
    return nullptr;
}

u32 Widget::getDepth() const noexcept {
    u32 depth = 0;
    const Widget* current = m_parent;
    while (current) {
        depth++;
        current = current->m_parent;
    }
    return depth;
}

bool Widget::isAncestorOf(const Widget* widget) const {
    const Widget* current = widget;
    while (current) {
        if (current->m_parent == this) {
            return true;
        }
        current = current->m_parent;
    }
    return false;
}

void Widget::setStyle(const Style& style) {
    m_style = style;
    markLayoutDirty();
    onStyleChanged();
}

Rect Widget::getContentBounds() const noexcept {
    return Rect(
        m_bounds.x + m_style.padding.left,
        m_bounds.y + m_style.padding.top,
        m_bounds.width - m_style.padding.horizontal(),
        m_bounds.height - m_style.padding.vertical()
    );
}

void Widget::markLayoutDirty() {
    m_layoutDirty = true;
    if (m_parent) {
        m_parent->markLayoutDirty();
    }
}

void Widget::layout(const Rect& availableSpace) {
    if (!m_visible || m_style.display == Display::None) {
        m_bounds = Rect();
        return;
    }
    
    Rect oldBounds = m_bounds;
    
    // Calculate dimensions
    f32 width = availableSpace.width;
    f32 height = availableSpace.height;
    
    // Apply explicit dimensions
    if (!m_style.width.isAuto()) {
        width = m_style.width.resolve(availableSpace.width);
    }
    if (!m_style.height.isAuto()) {
        height = m_style.height.resolve(availableSpace.height);
    }
    
    // Apply min/max constraints
    if (!m_style.minWidth.isAuto()) {
        width = std::max(width, m_style.minWidth.resolve(availableSpace.width));
    }
    if (!m_style.maxWidth.isAuto()) {
        width = std::min(width, m_style.maxWidth.resolve(availableSpace.width));
    }
    if (!m_style.minHeight.isAuto()) {
        height = std::max(height, m_style.minHeight.resolve(availableSpace.height));
    }
    if (!m_style.maxHeight.isAuto()) {
        height = std::min(height, m_style.maxHeight.resolve(availableSpace.height));
    }
    
    // Measure content if auto-sized
    if (m_style.width.isAuto() || m_style.height.isAuto()) {
        Vec2 contentSize = measureContent(width, height);
        if (m_style.width.isAuto()) {
            width = contentSize.x + m_style.padding.horizontal();
        }
        if (m_style.height.isAuto()) {
            height = contentSize.y + m_style.padding.vertical();
        }
    }
    
    // Set position based on position type
    f32 x = availableSpace.x + m_style.margin.left;
    f32 y = availableSpace.y + m_style.margin.top;
    
    if (m_style.position == PositionType::Absolute) {
        if (!m_style.left.isAuto()) {
            x = availableSpace.x + m_style.left.resolve(availableSpace.width);
        } else if (!m_style.right.isAuto()) {
            x = availableSpace.x + availableSpace.width - width - m_style.right.resolve(availableSpace.width);
        }
        if (!m_style.top.isAuto()) {
            y = availableSpace.y + m_style.top.resolve(availableSpace.height);
        } else if (!m_style.bottom.isAuto()) {
            y = availableSpace.y + availableSpace.height - height - m_style.bottom.resolve(availableSpace.height);
        }
    }
    
    m_bounds = Rect(x, y, width, height);
    
    // Layout children
    layoutChildren();
    
    m_layoutDirty = false;
    
    if (m_bounds.x != oldBounds.x || m_bounds.y != oldBounds.y ||
        m_bounds.width != oldBounds.width || m_bounds.height != oldBounds.height) {
        onBoundsChanged();
    }
}

void Widget::setVisible(bool visible) {
    if (m_visible != visible) {
        m_visible = visible;
        markLayoutDirty();
    }
}

void Widget::setEnabled(bool enabled) {
    m_enabled = enabled;
    if (!enabled) {
        m_hovered = false;
        m_pressed = false;
    }
}

void Widget::requestFocus() {
    UISystem::instance().setFocusedWidget(this);
}

void Widget::releaseFocus() {
    if (m_focused) {
        UISystem::instance().clearFocus();
    }
}

bool Widget::containsPoint(Vec2 point) const {
    return m_bounds.contains(point);
}

Widget* Widget::hitTest(Vec2 point) {
    if (!m_visible || !m_style.pointerEvents || !containsPoint(point)) {
        return nullptr;
    }
    
    // Check children in reverse order (front to back)
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if (Widget* hit = (*it)->hitTest(point)) {
            return hit;
        }
    }
    
    return this;
}

AnimationState& Widget::startAnimation(const PropertyAnimation& animation) {
    AnimationState state;
    state.id = m_nextAnimationId++;
    state.isPlaying = true;
    m_animations.emplace_back(animation, state);
    return m_animations.back().second;
}

void Widget::stopAnimation(u64 animationId) {
    m_animations.erase(
        std::remove_if(m_animations.begin(), m_animations.end(),
            [animationId](const auto& pair) { return pair.second.id == animationId; }),
        m_animations.end());
}

void Widget::stopAllAnimations() {
    m_animations.clear();
}

void Widget::updateAnimations(f32 deltaTime) {
    for (auto& [anim, state] : m_animations) {
        if (!state.isPlaying) continue;
        
        state.elapsed += deltaTime;
        
        // Calculate progress
        f32 progress = 0.0f;
        if (state.elapsed >= anim.delay) {
            f32 animTime = state.elapsed - anim.delay;
            progress = std::min(1.0f, animTime / anim.duration);
            
            // Apply easing
            switch (anim.easing) {
                case EasingType::EaseIn:
                    progress = progress * progress;
                    break;
                case EasingType::EaseOut:
                    progress = 1.0f - (1.0f - progress) * (1.0f - progress);
                    break;
                case EasingType::EaseInOut:
                    progress = progress < 0.5f 
                        ? 2.0f * progress * progress 
                        : 1.0f - 2.0f * (1.0f - progress) * (1.0f - progress);
                    break;
                default:
                    break;
            }
            
            if (state.reversing) {
                progress = 1.0f - progress;
            }
        }
        
        state.progress = progress;
        
        // Apply animation value
        f32 value = anim.from + (anim.to - anim.from) * progress;
        
        switch (anim.property) {
            case AnimationProperty::Opacity:
                m_style.opacity = value;
                break;
            case AnimationProperty::ScaleX:
                m_style.scale.x = value;
                break;
            case AnimationProperty::ScaleY:
                m_style.scale.y = value;
                break;
            case AnimationProperty::Rotation:
                m_style.rotation = value;
                break;
            case AnimationProperty::PositionX:
                m_style.translation.x = value;
                break;
            case AnimationProperty::PositionY:
                m_style.translation.y = value;
                break;
            case AnimationProperty::BackgroundColorA:
                m_style.backgroundColor.a = value;
                break;
            default:
                break;
        }
        
        // Check for completion
        if (state.elapsed >= anim.delay + anim.duration) {
            if (anim.autoReverse && !state.reversing) {
                state.reversing = true;
                state.elapsed = anim.delay;
            } else if (anim.repeatCount != 0) {
                if (anim.repeatCount > 0) {
                    state.currentRepeat++;
                    if (state.currentRepeat >= anim.repeatCount) {
                        state.isPlaying = false;
                        state.isComplete = true;
                        continue;
                    }
                }
                state.elapsed = 0;
                state.reversing = false;
            } else {
                state.isPlaying = false;
                state.isComplete = true;
            }
        }
    }
    
    // Remove completed animations
    m_animations.erase(
        std::remove_if(m_animations.begin(), m_animations.end(),
            [](const auto& pair) { return pair.second.isComplete; }),
        m_animations.end());
    
    // Update children
    for (auto& child : m_children) {
        child->updateAnimations(deltaTime);
    }
}

void Widget::onStyleChanged() {}
void Widget::onBoundsChanged() {}

Vec2 Widget::measureContent(f32 /*availableWidth*/, f32 /*availableHeight*/) {
    return Vec2(0, 0);
}

void Widget::layoutChildren() {
    // Default: simple stacking
    Rect contentBounds = getContentBounds();
    for (auto& child : m_children) {
        child->layout(contentBounds);
    }
}

bool Widget::onPointerDown(const PointerEvent& event) {
    m_pressed = true;
    if (m_onPointerDown) {
        m_onPointerDown(event);
    }
    return true;
}

bool Widget::onPointerUp(const PointerEvent& event) {
    m_pressed = false;
    if (m_onPointerUp) {
        m_onPointerUp(event);
    }
    return true;
}

bool Widget::onPointerMove(const PointerEvent& event) {
    if (m_onPointerMove) {
        m_onPointerMove(event);
    }
    return true;
}

void Widget::onPointerEnter(const PointerEvent& event) {
    m_hovered = true;
    if (m_onPointerEnter) {
        m_onPointerEnter(event);
    }
}

void Widget::onPointerLeave(const PointerEvent& event) {
    m_hovered = false;
    m_pressed = false;
    if (m_onPointerLeave) {
        m_onPointerLeave(event);
    }
}

bool Widget::onKeyDown(const KeyEvent& event) {
    if (m_onKeyDown) {
        m_onKeyDown(event);
        return true;
    }
    return false;
}

bool Widget::onKeyUp(const KeyEvent& event) {
    if (m_onKeyUp) {
        m_onKeyUp(event);
        return true;
    }
    return false;
}

void Widget::onFocus(const FocusEvent& event) {
    m_focused = true;
    if (m_onFocus) {
        m_onFocus(event);
    }
}

void Widget::onBlur(const FocusEvent& event) {
    m_focused = false;
    if (m_onBlur) {
        m_onBlur(event);
    }
}

bool Widget::onScroll(const ScrollEvent& event) {
    if (m_onScroll) {
        m_onScroll(event);
        return true;
    }
    return false;
}

// ============================================================================
// Container Implementation
// ============================================================================

Container::Container() {
    m_accessibility.role = AccessibilityRole::None;
}

void Container::layoutChildren() {
    if (m_children.empty()) return;
    
    Rect contentBounds = getContentBounds();
    
    // Simplified flexbox layout
    bool isRow = m_style.flexDirection == FlexDirection::Row || 
                 m_style.flexDirection == FlexDirection::RowReverse;
    bool isReverse = m_style.flexDirection == FlexDirection::RowReverse ||
                     m_style.flexDirection == FlexDirection::ColumnReverse;
    
    f32 mainAxisSize = isRow ? contentBounds.width : contentBounds.height;
    f32 crossAxisSize = isRow ? contentBounds.height : contentBounds.width;
    
    // First pass: calculate total flex and fixed sizes
    f32 totalFlex = 0.0f;
    f32 totalFixed = 0.0f;
    
    std::vector<f32> childMainSizes(m_children.size());
    
    for (usize i = 0; i < m_children.size(); ++i) {
        auto& child = m_children[i];
        if (!child->isVisible()) continue;
        
        const Style& cs = child->getStyle();
        
        if (cs.flexGrow > 0) {
            totalFlex += cs.flexGrow;
        } else {
            // Measure fixed-size child
            Vec2 size = child->measureContent(mainAxisSize, crossAxisSize);
            f32 fixedSize = isRow ? size.x : size.y;
            fixedSize += cs.margin.horizontal() + cs.padding.horizontal();
            totalFixed += fixedSize;
            childMainSizes[i] = fixedSize;
        }
    }
    
    // Distribute remaining space
    f32 remainingSpace = mainAxisSize - totalFixed;
    f32 flexUnit = totalFlex > 0 ? remainingSpace / totalFlex : 0;
    
    // Calculate flex child sizes
    for (usize i = 0; i < m_children.size(); ++i) {
        auto& child = m_children[i];
        if (!child->isVisible()) continue;
        
        const Style& cs = child->getStyle();
        if (cs.flexGrow > 0) {
            childMainSizes[i] = flexUnit * cs.flexGrow;
        }
    }
    
    // Second pass: position children
    f32 mainOffset = isReverse ? mainAxisSize : 0.0f;
    
    for (usize i = 0; i < m_children.size(); ++i) {
        usize idx = isReverse ? m_children.size() - 1 - i : i;
        auto& child = m_children[idx];
        if (!child->isVisible()) continue;
        
        f32 childMainSize = childMainSizes[idx];
        
        if (isReverse) {
            mainOffset -= childMainSize;
        }
        
        Rect childSpace;
        if (isRow) {
            childSpace = Rect(
                contentBounds.x + mainOffset,
                contentBounds.y,
                childMainSize,
                crossAxisSize
            );
        } else {
            childSpace = Rect(
                contentBounds.x,
                contentBounds.y + mainOffset,
                crossAxisSize,
                childMainSize
            );
        }
        
        child->layout(childSpace);
        
        if (!isReverse) {
            mainOffset += childMainSize;
        }
    }
}

// ============================================================================
// Label Implementation
// ============================================================================

Label::Label() {
    m_accessibility.role = AccessibilityRole::None;
}

Label::Label(const std::string& text) : m_text(text) {
    m_accessibility.role = AccessibilityRole::None;
    m_accessibility.label = text;
}

void Label::setText(const std::string& text) {
    m_text = text;
    m_accessibility.label = text;
    markLayoutDirty();
}

Vec2 Label::measureContent(f32 availableWidth, f32 /*availableHeight*/) {
    // Simplified text measurement (would use actual font metrics)
    f32 fontSize = m_style.text.fontSize;
    f32 charWidth = fontSize * 0.6f;  // Approximate
    f32 lineHeight = fontSize * m_style.text.lineHeight;
    
    if (m_text.empty()) {
        return Vec2(0, lineHeight);
    }
    
    // Calculate text bounds
    f32 maxLineWidth = availableWidth > 0 ? availableWidth : 100000.0f;
    f32 currentLineWidth = 0.0f;
    u32 lineCount = 1;
    
    for (char c : m_text) {
        if (c == '\n') {
            lineCount++;
            currentLineWidth = 0;
            continue;
        }
        
        currentLineWidth += charWidth;
        if (currentLineWidth > maxLineWidth) {
            lineCount++;
            currentLineWidth = charWidth;
        }
    }
    
    if (m_maxLines > 0 && lineCount > m_maxLines) {
        lineCount = m_maxLines;
    }
    
    f32 width = std::min(static_cast<f32>(m_text.length()) * charWidth, maxLineWidth);
    f32 height = static_cast<f32>(lineCount) * lineHeight;
    
    return Vec2(width, height);
}

// ============================================================================
// Button Implementation
// ============================================================================

Button::Button() {
    m_accessibility.role = AccessibilityRole::Button;
    m_accessibility.isButton = true;
    
    // Default button style
    m_style.padding = EdgeInsets(16, 8);
    m_style.backgroundColor = Color::fromHex(0x6200EE);
    m_style.border.radii = CornerRadii(4);
    m_style.text.color = Color::white();
    m_style.text.fontWeight = FontWeight::Medium;
}

Button::Button(const std::string& text) : Button() {
    m_text = text;
    m_accessibility.label = text;
}

void Button::setText(const std::string& text) {
    m_text = text;
    m_accessibility.label = text;
    markLayoutDirty();
}

bool Button::onPointerDown(const PointerEvent& event) {
    Widget::onPointerDown(event);
    return true;
}

bool Button::onPointerUp(const PointerEvent& event) {
    Widget::onPointerUp(event);
    
    if (m_enabled && containsPoint(event.position) && m_onClick) {
        m_onClick();
    }
    
    return true;
}

void Button::onPointerEnter(const PointerEvent& event) {
    Widget::onPointerEnter(event);
}

void Button::onPointerLeave(const PointerEvent& event) {
    Widget::onPointerLeave(event);
}

Vec2 Button::measureContent(f32 availableWidth, f32 availableHeight) {
    f32 fontSize = m_style.text.fontSize;
    f32 charWidth = fontSize * 0.6f;
    f32 textWidth = static_cast<f32>(m_text.length()) * charWidth;
    f32 textHeight = fontSize * m_style.text.lineHeight;
    
    (void)availableWidth;
    (void)availableHeight;
    
    return Vec2(textWidth, textHeight);
}

// ============================================================================
// Image Implementation
// ============================================================================

Image::Image() {
    m_accessibility.role = AccessibilityRole::Image;
}

void Image::setSource(const std::string& source) {
    m_source = source;
    
    // Parse image dimensions from source path or attempt to read image header
    // This implementation handles common image formats by reading their headers
    m_naturalSize = Vec2(0, 0);
    
    if (source.empty()) {
        markLayoutDirty();
        return;
    }
    
    // Try to read image dimensions from file header
    std::ifstream file(source, std::ios::binary);
    if (file.is_open()) {
        u8 header[24];
        file.read(reinterpret_cast<char*>(header), 24);
        std::streamsize bytesRead = file.gcount();
        file.close();
        
        if (bytesRead >= 8) {
            // PNG: Check for PNG signature and read IHDR chunk
            if (header[0] == 0x89 && header[1] == 'P' && header[2] == 'N' && header[3] == 'G' &&
                header[4] == 0x0D && header[5] == 0x0A && header[6] == 0x1A && header[7] == 0x0A) {
                if (bytesRead >= 24) {
                    // Width at offset 16-19, Height at offset 20-23 (big endian)
                    u32 width = (static_cast<u32>(header[16]) << 24) | 
                                (static_cast<u32>(header[17]) << 16) |
                                (static_cast<u32>(header[18]) << 8) | 
                                 static_cast<u32>(header[19]);
                    u32 height = (static_cast<u32>(header[20]) << 24) | 
                                 (static_cast<u32>(header[21]) << 16) |
                                 (static_cast<u32>(header[22]) << 8) | 
                                  static_cast<u32>(header[23]);
                    m_naturalSize = Vec2(static_cast<f32>(width), static_cast<f32>(height));
                }
            }
            // JPEG: Check for JPEG signature (FFD8FF)
            else if (header[0] == 0xFF && header[1] == 0xD8 && header[2] == 0xFF) {
                // JPEG dimensions require parsing multiple segments
                // Read through segments to find SOF0/SOF2 marker
                std::ifstream jpegFile(source, std::ios::binary);
                if (jpegFile.is_open()) {
                    jpegFile.seekg(2); // Skip SOI marker
                    while (jpegFile.good()) {
                        u8 marker[2];
                        jpegFile.read(reinterpret_cast<char*>(marker), 2);
                        if (marker[0] != 0xFF) break;
                        
                        // SOF0, SOF1, SOF2 contain dimensions
                        if ((marker[1] >= 0xC0 && marker[1] <= 0xC2)) {
                            u8 sof[7];
                            jpegFile.read(reinterpret_cast<char*>(sof), 7);
                            // Height at offset 3-4, Width at offset 5-6 (big endian)
                            u32 height = (static_cast<u32>(sof[3]) << 8) | static_cast<u32>(sof[4]);
                            u32 width = (static_cast<u32>(sof[5]) << 8) | static_cast<u32>(sof[6]);
                            m_naturalSize = Vec2(static_cast<f32>(width), static_cast<f32>(height));
                            break;
                        }
                        // Skip segment
                        if (marker[1] != 0xD8 && marker[1] != 0xD9 && !(marker[1] >= 0xD0 && marker[1] <= 0xD7)) {
                            u8 len[2];
                            jpegFile.read(reinterpret_cast<char*>(len), 2);
                            u16 segLen = (static_cast<u16>(len[0]) << 8) | static_cast<u16>(len[1]);
                            jpegFile.seekg(segLen - 2, std::ios::cur);
                        }
                    }
                    jpegFile.close();
                }
            }
            // BMP: Check for BMP signature
            else if (header[0] == 'B' && header[1] == 'M' && bytesRead >= 22) {
                // Width at offset 18-21, Height at offset 22-25 (little endian)
                i32 width = static_cast<i32>(header[18]) | 
                           (static_cast<i32>(header[19]) << 8) |
                           (static_cast<i32>(header[20]) << 16) | 
                           (static_cast<i32>(header[21]) << 24);
                // Read more for height
                std::ifstream bmpFile(source, std::ios::binary);
                bmpFile.seekg(22);
                u8 heightBytes[4];
                bmpFile.read(reinterpret_cast<char*>(heightBytes), 4);
                bmpFile.close();
                i32 height = static_cast<i32>(heightBytes[0]) | 
                            (static_cast<i32>(heightBytes[1]) << 8) |
                            (static_cast<i32>(heightBytes[2]) << 16) | 
                            (static_cast<i32>(heightBytes[3]) << 24);
                // Height can be negative for top-down BMP
                m_naturalSize = Vec2(static_cast<f32>(width), static_cast<f32>(std::abs(height)));
            }
            // GIF: Check for GIF signature
            else if ((header[0] == 'G' && header[1] == 'I' && header[2] == 'F' && 
                      header[3] == '8' && (header[4] == '7' || header[4] == '9') && header[5] == 'a') &&
                     bytesRead >= 10) {
                // Width at offset 6-7, Height at offset 8-9 (little endian)
                u16 width = static_cast<u16>(header[6]) | (static_cast<u16>(header[7]) << 8);
                u16 height = static_cast<u16>(header[8]) | (static_cast<u16>(header[9]) << 8);
                m_naturalSize = Vec2(static_cast<f32>(width), static_cast<f32>(height));
            }
        }
    }
    
    // If we couldn't read dimensions, use a default size
    // This is a reasonable default for unknown or procedural images
    if (m_naturalSize.x <= 0 || m_naturalSize.y <= 0) {
        // Default to 256x256 for unknown images
        m_naturalSize = Vec2(256.0f, 256.0f);
    }
    
    markLayoutDirty();
}

Vec2 Image::measureContent(f32 availableWidth, f32 availableHeight) {
    if (m_naturalSize.x <= 0 || m_naturalSize.y <= 0) {
        return Vec2(0, 0);
    }
    
    switch (m_aspectRatio) {
        case AspectRatio::None:
            return m_naturalSize;
            
        case AspectRatio::Fill:
            return Vec2(availableWidth > 0 ? availableWidth : m_naturalSize.x,
                       availableHeight > 0 ? availableHeight : m_naturalSize.y);
            
        case AspectRatio::Contain: {
            if (availableWidth <= 0 || availableHeight <= 0) {
                return m_naturalSize;
            }
            f32 scale = std::min(availableWidth / m_naturalSize.x, 
                                 availableHeight / m_naturalSize.y);
            return Vec2(m_naturalSize.x * scale, m_naturalSize.y * scale);
        }
        
        case AspectRatio::Cover: {
            if (availableWidth <= 0 || availableHeight <= 0) {
                return m_naturalSize;
            }
            f32 scale = std::max(availableWidth / m_naturalSize.x, 
                                 availableHeight / m_naturalSize.y);
            return Vec2(m_naturalSize.x * scale, m_naturalSize.y * scale);
        }
    }
    
    return m_naturalSize;
}

// ============================================================================
// TextInput Implementation
// ============================================================================

TextInput::TextInput() {
    m_accessibility.role = AccessibilityRole::TextBox;
    
    // Default text input style
    m_style.padding = EdgeInsets(12, 8);
    m_style.backgroundColor = Color::white();
    m_style.border = BorderStyle(1, Color::fromHex(0xCCCCCC), 4);
}

void TextInput::setValue(const std::string& value) {
    if (m_value != value) {
        m_value = value;
        m_cursorPosition = value.length();
        if (m_onChange) {
            m_onChange(value);
        }
    }
}

bool TextInput::onPointerDown(const PointerEvent& event) {
    Widget::onPointerDown(event);
    requestFocus();
    return true;
}

bool TextInput::onKeyDown(const KeyEvent& event) {
    if (!m_focused) return false;
    
    if (event.key == "Backspace" && m_cursorPosition > 0) {
        m_value.erase(m_cursorPosition - 1, 1);
        m_cursorPosition--;
        if (m_onChange) m_onChange(m_value);
    } else if (event.key == "Delete" && m_cursorPosition < m_value.length()) {
        m_value.erase(m_cursorPosition, 1);
        if (m_onChange) m_onChange(m_value);
    } else if (event.key == "Left" && m_cursorPosition > 0) {
        m_cursorPosition--;
    } else if (event.key == "Right" && m_cursorPosition < m_value.length()) {
        m_cursorPosition++;
    } else if (event.key == "Home") {
        m_cursorPosition = 0;
    } else if (event.key == "End") {
        m_cursorPosition = m_value.length();
    } else if (event.key == "Enter" && !m_multiline) {
        if (m_onSubmit) m_onSubmit();
    } else if (event.key.length() == 1 && !event.ctrl && !event.alt) {
        m_value.insert(m_cursorPosition, event.key);
        m_cursorPosition++;
        if (m_onChange) m_onChange(m_value);
    }
    
    return true;
}

void TextInput::onFocus(const FocusEvent& event) {
    Widget::onFocus(event);
    m_style.border.color = Color::fromHex(0x6200EE);
}

void TextInput::onBlur(const FocusEvent& event) {
    Widget::onBlur(event);
    m_style.border.color = Color::fromHex(0xCCCCCC);
}

Vec2 TextInput::measureContent(f32 /*availableWidth*/, f32 /*availableHeight*/) {
    f32 fontSize = m_style.text.fontSize;
    f32 height = fontSize * m_style.text.lineHeight;
    
    if (m_multiline) {
        height *= static_cast<f32>(DEFAULT_MULTILINE_LINE_COUNT);
    }
    
    return Vec2(DEFAULT_TEXT_INPUT_MIN_WIDTH, height);
}

// ============================================================================
// ScrollView Implementation
// ============================================================================

ScrollView::ScrollView() {
    m_style.overflow = Overflow::Scroll;
    m_accessibility.role = AccessibilityRole::None;
}

void ScrollView::setScrollOffset(Vec2 offset) {
    // Clamp scroll offset
    offset.x = std::max(0.0f, std::min(offset.x, m_contentSize.x - m_bounds.width));
    offset.y = std::max(0.0f, std::min(offset.y, m_contentSize.y - m_bounds.height));
    
    if (!m_horizontalEnabled) offset.x = 0;
    if (!m_verticalEnabled) offset.y = 0;
    
    m_scrollOffset = offset;
}

void ScrollView::scrollTo(Vec2 position, bool /*animated*/) {
    // In production, would animate smoothly
    setScrollOffset(position);
}

void ScrollView::scrollToWidget(Widget* widget, bool animated) {
    if (!widget || !isAncestorOf(widget)) return;
    
    Rect widgetBounds = widget->getBounds();
    Vec2 targetOffset = m_scrollOffset;
    
    // Scroll horizontally if needed
    if (m_horizontalEnabled) {
        if (widgetBounds.left() < m_bounds.left() + m_scrollOffset.x) {
            targetOffset.x = widgetBounds.left() - m_bounds.left();
        } else if (widgetBounds.right() > m_bounds.right() + m_scrollOffset.x) {
            targetOffset.x = widgetBounds.right() - m_bounds.right();
        }
    }
    
    // Scroll vertically if needed
    if (m_verticalEnabled) {
        if (widgetBounds.top() < m_bounds.top() + m_scrollOffset.y) {
            targetOffset.y = widgetBounds.top() - m_bounds.top();
        } else if (widgetBounds.bottom() > m_bounds.bottom() + m_scrollOffset.y) {
            targetOffset.y = widgetBounds.bottom() - m_bounds.bottom();
        }
    }
    
    scrollTo(targetOffset, animated);
}

void ScrollView::layoutChildren() {
    if (m_children.empty()) return;
    
    // Layout children in a vertical stack
    Rect contentBounds = getContentBounds();
    f32 yOffset = 0;
    f32 maxWidth = 0;
    
    for (auto& child : m_children) {
        Rect childSpace(
            contentBounds.x - m_scrollOffset.x,
            contentBounds.y - m_scrollOffset.y + yOffset,
            contentBounds.width,
            10000  // Large available height
        );
        child->layout(childSpace);
        
        yOffset += child->getBounds().height;
        maxWidth = std::max(maxWidth, child->getBounds().width);
    }
    
    m_contentSize = Vec2(maxWidth, yOffset);
}

bool ScrollView::onScroll(const ScrollEvent& event) {
    Vec2 newOffset = m_scrollOffset - event.delta;
    setScrollOffset(newOffset);
    return true;
}

bool ScrollView::onPointerDown(const PointerEvent& event) {
    m_isDragging = true;
    m_dragStart = event.position;
    m_scrollStart = m_scrollOffset;
    return Widget::onPointerDown(event);
}

bool ScrollView::onPointerMove(const PointerEvent& event) {
    if (m_isDragging) {
        Vec2 delta = m_dragStart - event.position;
        setScrollOffset(m_scrollStart + delta);
    }
    return Widget::onPointerMove(event);
}

bool ScrollView::onPointerUp(const PointerEvent& event) {
    m_isDragging = false;
    return Widget::onPointerUp(event);
}

// ============================================================================
// Checkbox Implementation
// ============================================================================

Checkbox::Checkbox() {
    m_accessibility.role = AccessibilityRole::CheckBox;
    
    m_style.padding = EdgeInsets(8);
}

void Checkbox::setChecked(bool checked) {
    if (m_checked != checked) {
        m_checked = checked;
        m_accessibility.isChecked = checked;
        if (m_onChange) {
            m_onChange(checked);
        }
    }
}

bool Checkbox::onPointerUp(const PointerEvent& event) {
    Widget::onPointerUp(event);
    if (m_enabled && containsPoint(event.position)) {
        toggle();
    }
    return true;
}

bool Checkbox::onKeyDown(const KeyEvent& event) {
    if (m_focused && (event.key == " " || event.key == "Enter")) {
        toggle();
        return true;
    }
    return Widget::onKeyDown(event);
}

Vec2 Checkbox::measureContent(f32 /*availableWidth*/, f32 /*availableHeight*/) {
    f32 checkboxSize = 20.0f;
    f32 fontSize = m_style.text.fontSize;
    f32 charWidth = fontSize * 0.6f;
    
    f32 width = checkboxSize + 8.0f + static_cast<f32>(m_label.length()) * charWidth;
    f32 height = std::max(checkboxSize, fontSize * m_style.text.lineHeight);
    
    return Vec2(width, height);
}

// ============================================================================
// Slider Implementation
// ============================================================================

Slider::Slider() {
    m_accessibility.role = AccessibilityRole::Slider;
    
    m_style.height = Dimension::pixels(40);
}

void Slider::setValue(f32 value) {
    // Clamp to range
    value = std::max(m_min, std::min(m_max, value));
    
    // Snap to step
    if (m_step > 0) {
        value = std::round((value - m_min) / m_step) * m_step + m_min;
    }
    
    if (m_value != value) {
        m_value = value;
        m_accessibility.value = std::to_string(value);
        if (m_onChange) {
            m_onChange(value);
        }
    }
}

bool Slider::onPointerDown(const PointerEvent& event) {
    Widget::onPointerDown(event);
    m_isDragging = true;
    updateValueFromPosition(event.position);
    return true;
}

bool Slider::onPointerMove(const PointerEvent& event) {
    Widget::onPointerMove(event);
    if (m_isDragging) {
        updateValueFromPosition(event.position);
    }
    return true;
}

bool Slider::onPointerUp(const PointerEvent& event) {
    Widget::onPointerUp(event);
    m_isDragging = false;
    return true;
}

Vec2 Slider::measureContent(f32 availableWidth, f32 /*availableHeight*/) {
    return Vec2(availableWidth > 0 ? availableWidth : 200.0f, 24.0f);
}

void Slider::updateValueFromPosition(Vec2 position) {
    f32 trackWidth = m_bounds.width - 16.0f;  // Subtract thumb width
    f32 relativeX = position.x - m_bounds.x - 8.0f;  // Half thumb width
    
    f32 percentage = std::max(0.0f, std::min(1.0f, relativeX / trackWidth));
    f32 newValue = m_min + percentage * (m_max - m_min);
    
    setValue(newValue);
}

// ============================================================================
// ProgressBar Implementation
// ============================================================================

ProgressBar::ProgressBar() {
    m_accessibility.role = AccessibilityRole::ProgressBar;
    
    m_style.height = Dimension::pixels(8);
    m_style.backgroundColor = Color::fromHex(0xE0E0E0);
    m_style.border.radii = CornerRadii(4);
}

void ProgressBar::setProgress(f32 progress) {
    m_progress = std::max(0.0f, std::min(1.0f, progress));
    m_accessibility.value = std::to_string(static_cast<int>(m_progress * 100)) + "%";
}

Vec2 ProgressBar::measureContent(f32 availableWidth, f32 /*availableHeight*/) {
    return Vec2(availableWidth > 0 ? availableWidth : 200.0f, 8.0f);
}

// ============================================================================
// UISystem Implementation
// ============================================================================

UISystem& UISystem::instance() {
    static UISystem instance;
    return instance;
}

UISystem::~UISystem() {
    shutdown();
}

Result<void> UISystem::initialize(f32 screenWidth, f32 screenHeight, f32 pixelRatio) {
    if (m_initialized) {
        return {};
    }
    
    m_screenSize = Vec2(screenWidth, screenHeight);
    m_pixelRatio = pixelRatio;
    m_theme = Theme::light();
    
    // Create root widget
    m_root = std::make_unique<Container>();
    m_root->getStyle().width = Dimension::percent(100);
    m_root->getStyle().height = Dimension::percent(100);
    
    m_initialized = true;
    
    NOVA_LOG_INFO(LogCategory::Core, "UI System initialized ({}x{} @ {}x)", 
                  screenWidth, screenHeight, pixelRatio);
    
    return {};
}

void UISystem::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    m_focusedWidget = nullptr;
    m_hoveredWidget = nullptr;
    m_pressedWidget = nullptr;
    m_root.reset();
    m_widgetRegistry.clear();
    
    m_initialized = false;
    
    NOVA_LOG_INFO(LogCategory::Core, "UI System shutdown");
}

void UISystem::update(f32 deltaTime) {
    if (!m_initialized || !m_root) {
        return;
    }
    
    // Update animations
    m_root->updateAnimations(deltaTime);
    
    // Perform layout if needed
    if (m_root->isLayoutDirty()) {
        performLayout();
    }
}

void UISystem::performLayout() {
    if (!m_initialized || !m_root) {
        return;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    Rect screenRect(0, 0, m_screenSize.x, m_screenSize.y);
    m_root->layout(screenRect);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    m_lastLayoutTimeMs = std::chrono::duration<f32, std::milli>(endTime - startTime).count();
    
    // Count widgets
    m_widgetCount = 0;
    m_visibleWidgetCount = 0;
    std::vector<Widget*> allWidgets;
    collectWidgets(m_root.get(), allWidgets);
    m_widgetCount = static_cast<u32>(allWidgets.size());
    for (Widget* w : allWidgets) {
        if (w->isVisible()) {
            m_visibleWidgetCount++;
        }
    }
}

void UISystem::setScreenSize(f32 width, f32 height) {
    m_screenSize = Vec2(width, height);
    if (m_root) {
        m_root->markLayoutDirty();
    }
}

void UISystem::setPixelRatio(f32 ratio) {
    m_pixelRatio = ratio;
    if (m_root) {
        m_root->markLayoutDirty();
    }
}

void UISystem::setRoot(std::unique_ptr<Widget> root) {
    m_root = std::move(root);
    m_focusedWidget = nullptr;
    m_hoveredWidget = nullptr;
    m_pressedWidget = nullptr;
    
    if (m_root) {
        m_root->getStyle().width = Dimension::percent(100);
        m_root->getStyle().height = Dimension::percent(100);
        m_root->markLayoutDirty();
    }
}

void UISystem::setFocusedWidget(Widget* widget) {
    if (m_focusedWidget == widget) {
        return;
    }
    
    FocusEvent event;
    
    if (m_focusedWidget) {
        event.relatedTarget = widget;
        m_focusedWidget->onBlur(event);
    }
    
    Widget* oldFocused = m_focusedWidget;
    m_focusedWidget = widget;
    
    if (widget) {
        event.relatedTarget = oldFocused;
        widget->onFocus(event);
    }
}

void UISystem::clearFocus() {
    setFocusedWidget(nullptr);
}

void UISystem::focusNext() {
    // Would implement tab navigation
}

void UISystem::focusPrevious() {
    // Would implement shift-tab navigation
}

bool UISystem::handlePointerDown(const PointerEvent& event) {
    if (!m_initialized || !m_root) {
        return false;
    }
    
    Widget* target = hitTest(event.screenPosition);
    if (target) {
        m_pressedWidget = target;
        
        PointerEvent localEvent = event;
        localEvent.position = Vec2(
            event.screenPosition.x - target->getBounds().x,
            event.screenPosition.y - target->getBounds().y
        );
        
        return target->onPointerDown(localEvent);
    }
    
    return false;
}

bool UISystem::handlePointerUp(const PointerEvent& event) {
    if (!m_initialized) {
        return false;
    }
    
    bool handled = false;
    
    if (m_pressedWidget) {
        PointerEvent localEvent = event;
        localEvent.position = Vec2(
            event.screenPosition.x - m_pressedWidget->getBounds().x,
            event.screenPosition.y - m_pressedWidget->getBounds().y
        );
        
        handled = m_pressedWidget->onPointerUp(localEvent);
        m_pressedWidget = nullptr;
    }
    
    return handled;
}

bool UISystem::handlePointerMove(const PointerEvent& event) {
    if (!m_initialized || !m_root) {
        return false;
    }
    
    // Update hovered widget
    updateHoveredWidget(event.screenPosition);
    
    // Send move to pressed widget
    if (m_pressedWidget) {
        PointerEvent localEvent = event;
        localEvent.position = Vec2(
            event.screenPosition.x - m_pressedWidget->getBounds().x,
            event.screenPosition.y - m_pressedWidget->getBounds().y
        );
        
        return m_pressedWidget->onPointerMove(localEvent);
    }
    
    return false;
}

bool UISystem::handleKeyDown(const KeyEvent& event) {
    if (!m_initialized || !m_focusedWidget) {
        return false;
    }
    
    return m_focusedWidget->onKeyDown(event);
}

bool UISystem::handleKeyUp(const KeyEvent& event) {
    if (!m_initialized || !m_focusedWidget) {
        return false;
    }
    
    return m_focusedWidget->onKeyUp(event);
}

bool UISystem::handleTextInput(const TextInputEvent& event) {
    if (!m_initialized || !m_focusedWidget) {
        return false;
    }
    
    // Convert to key events
    for (char c : event.text) {
        KeyEvent keyEvent;
        keyEvent.key = std::string(1, c);
        keyEvent.isDown = true;
        keyEvent.timestamp = event.timestamp;
        m_focusedWidget->onKeyDown(keyEvent);
    }
    
    return true;
}

bool UISystem::handleScroll(const ScrollEvent& event) {
    if (!m_initialized || !m_root) {
        return false;
    }
    
    Widget* target = hitTest(event.position);
    if (target) {
        return target->onScroll(event);
    }
    
    return false;
}

void UISystem::setTheme(const Theme& theme) {
    m_theme = theme;
    if (m_root) {
        m_root->markLayoutDirty();
    }
}

Widget* UISystem::findById(const std::string& id) {
    return m_root ? m_root->findById(id) : nullptr;
}

Widget* UISystem::findByHandle(WidgetHandle handle) {
    auto it = m_widgetRegistry.find(handle.value);
    return (it != m_widgetRegistry.end()) ? it->second : nullptr;
}

Widget* UISystem::hitTest(Vec2 screenPosition) {
    return m_root ? m_root->hitTest(screenPosition) : nullptr;
}

std::vector<Widget*> UISystem::getWidgetsToRender() const {
    std::vector<Widget*> widgets;
    if (m_root) {
        collectWidgets(m_root.get(), widgets);
    }
    return widgets;
}

void UISystem::registerWidget(Widget* widget) {
    if (widget) {
        m_widgetRegistry[widget->getHandle().value] = widget;
    }
}

void UISystem::unregisterWidget(Widget* widget) {
    if (widget) {
        m_widgetRegistry.erase(widget->getHandle().value);
    }
}

void UISystem::collectWidgets(Widget* widget, std::vector<Widget*>& output) const {
    if (!widget) return;
    
    output.push_back(widget);
    
    for (const auto& child : widget->getChildren()) {
        collectWidgets(child.get(), output);
    }
}

void UISystem::updateHoveredWidget(Vec2 position) {
    Widget* newHovered = hitTest(position);
    
    if (newHovered != m_hoveredWidget) {
        PointerEvent event;
        event.screenPosition = position;
        
        if (m_hoveredWidget) {
            event.position = Vec2(
                position.x - m_hoveredWidget->getBounds().x,
                position.y - m_hoveredWidget->getBounds().y
            );
            m_hoveredWidget->onPointerLeave(event);
        }
        
        m_hoveredWidget = newHovered;
        
        if (newHovered) {
            event.position = Vec2(
                position.x - newHovered->getBounds().x,
                position.y - newHovered->getBounds().y
            );
            newHovered->onPointerEnter(event);
        }
    }
}

} // namespace nova::ui
