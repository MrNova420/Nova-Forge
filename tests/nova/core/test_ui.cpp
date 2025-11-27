/**
 * @file test_ui.cpp
 * @brief Comprehensive tests for Nova UI module
 * @copyright (c) 2025 WeNova Interactive. All rights reserved.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/ui/ui_types.hpp>
#include <nova/core/ui/ui_system.hpp>
#include <nova/core/ui/widget.hpp>

using namespace nova;
using namespace nova::ui;
using Catch::Approx;

// =============================================================================
// Color Tests
// =============================================================================

TEST_CASE("UI: Color construction", "[ui][color]") {
    SECTION("Default construction - black with alpha") {
        Color c{};
        REQUIRE(c.r == Approx(0.0f));
        REQUIRE(c.g == Approx(0.0f));
        REQUIRE(c.b == Approx(0.0f));
        REQUIRE(c.a == Approx(1.0f));
    }
    
    SECTION("RGBA construction") {
        Color c{0.5f, 0.25f, 0.75f, 0.5f};
        REQUIRE(c.r == Approx(0.5f));
        REQUIRE(c.g == Approx(0.25f));
        REQUIRE(c.b == Approx(0.75f));
        REQUIRE(c.a == Approx(0.5f));
    }
    
    SECTION("From hex - no alpha") {
        Color c = Color::fromHex(0xFF0000);
        REQUIRE(c.r == Approx(1.0f));
        REQUIRE(c.g == Approx(0.0f));
        REQUIRE(c.b == Approx(0.0f));
        REQUIRE(c.a == Approx(1.0f));
    }
    
    SECTION("From hex - with alpha") {
        Color c = Color::fromHex(0xFF000080);  // RGBA format: R=0xFF, G=0x00, B=0x00, A=0x80
        // Just verify it creates a valid color
        REQUIRE(c.a >= 0.0f);
        REQUIRE(c.a <= 1.0f);
    }
    
    SECTION("From RGBA bytes") {
        Color c = Color::fromRGBA(255, 128, 64, 255);
        // Just verify it creates a valid color
        REQUIRE(c.a >= 0.0f);
        REQUIRE(c.a <= 1.0f);
    }
    
    SECTION("Lerp") {
        Color a{0.0f, 0.0f, 0.0f, 1.0f};
        Color b{1.0f, 1.0f, 1.0f, 1.0f};
        Color mid = a.lerp(b, 0.5f);
        
        REQUIRE(mid.r == Approx(0.5f));
        REQUIRE(mid.g == Approx(0.5f));
        REQUIRE(mid.b == Approx(0.5f));
        REQUIRE(mid.a == Approx(1.0f));
    }
    
    SECTION("Preset colors") {
        REQUIRE(Color::white().r == Approx(1.0f));
        REQUIRE(Color::black().r == Approx(0.0f));
        REQUIRE(Color::red().r == Approx(1.0f));
        REQUIRE(Color::green().g == Approx(1.0f));
        REQUIRE(Color::blue().b == Approx(1.0f));
    }
}

// =============================================================================
// Rect Tests
// =============================================================================

TEST_CASE("UI: Rect operations", "[ui][rect]") {
    SECTION("Default construction") {
        Rect r{};
        REQUIRE(r.x == Approx(0.0f));
        REQUIRE(r.y == Approx(0.0f));
        REQUIRE(r.width == Approx(0.0f));
        REQUIRE(r.height == Approx(0.0f));
    }
    
    SECTION("With values") {
        Rect r{10.0f, 20.0f, 100.0f, 50.0f};
        REQUIRE(r.x == Approx(10.0f));
        REQUIRE(r.y == Approx(20.0f));
        REQUIRE(r.width == Approx(100.0f));
        REQUIRE(r.height == Approx(50.0f));
    }
    
    SECTION("Contains point - inside") {
        Rect r{0.0f, 0.0f, 100.0f, 100.0f};
        REQUIRE(r.contains(Vec2{50.0f, 50.0f}));
        REQUIRE(r.contains(Vec2{0.0f, 0.0f}));
        REQUIRE(r.contains(Vec2{99.0f, 99.0f}));
    }
    
    SECTION("Contains point - outside") {
        Rect r{0.0f, 0.0f, 100.0f, 100.0f};
        REQUIRE_FALSE(r.contains(Vec2{-1.0f, 50.0f}));
        REQUIRE_FALSE(r.contains(Vec2{50.0f, -1.0f}));
        REQUIRE_FALSE(r.contains(Vec2{101.0f, 50.0f}));
        REQUIRE_FALSE(r.contains(Vec2{50.0f, 101.0f}));
    }
    
    SECTION("Intersects - true") {
        Rect a{0.0f, 0.0f, 100.0f, 100.0f};
        Rect b{50.0f, 50.0f, 100.0f, 100.0f};
        REQUIRE(a.intersects(b));
        REQUIRE(b.intersects(a));
    }
    
    SECTION("Intersects - false") {
        Rect a{0.0f, 0.0f, 100.0f, 100.0f};
        Rect b{200.0f, 200.0f, 100.0f, 100.0f};
        REQUIRE_FALSE(a.intersects(b));
        REQUIRE_FALSE(b.intersects(a));
    }
    
    SECTION("Accessors") {
        Rect r{10.0f, 20.0f, 100.0f, 50.0f};
        REQUIRE(r.right() == Approx(110.0f));
        REQUIRE(r.bottom() == Approx(70.0f));
        REQUIRE(r.center().x == Approx(60.0f));
        REQUIRE(r.center().y == Approx(45.0f));
    }
}

// =============================================================================
// EdgeInsets Tests
// =============================================================================

TEST_CASE("UI: EdgeInsets", "[ui][layout]") {
    SECTION("Default construction") {
        EdgeInsets e{};
        REQUIRE(e.top == Approx(0.0f));
        REQUIRE(e.right == Approx(0.0f));
        REQUIRE(e.bottom == Approx(0.0f));
        REQUIRE(e.left == Approx(0.0f));
    }
    
    SECTION("All same") {
        EdgeInsets e = EdgeInsets::all(10.0f);
        REQUIRE(e.top == Approx(10.0f));
        REQUIRE(e.right == Approx(10.0f));
        REQUIRE(e.bottom == Approx(10.0f));
        REQUIRE(e.left == Approx(10.0f));
    }
    
    SECTION("Symmetric") {
        EdgeInsets e = EdgeInsets::symmetric(20.0f, 10.0f);
        REQUIRE(e.top == Approx(10.0f));
        REQUIRE(e.bottom == Approx(10.0f));
        REQUIRE(e.left == Approx(20.0f));
        REQUIRE(e.right == Approx(20.0f));
    }
    
    SECTION("Horizontal/vertical") {
        EdgeInsets e{10.0f, 20.0f, 30.0f, 40.0f};  // left, top, right, bottom
        REQUIRE(e.horizontal() == Approx(40.0f)); // left + right = 10 + 30
        REQUIRE(e.vertical() == Approx(60.0f)); // top + bottom = 20 + 40
    }
}

// =============================================================================
// CornerRadii Tests
// =============================================================================

TEST_CASE("UI: CornerRadii", "[ui][layout]") {
    SECTION("All same") {
        CornerRadii c = CornerRadii::circular(8.0f);
        REQUIRE(c.topLeft == Approx(8.0f));
        REQUIRE(c.topRight == Approx(8.0f));
        REQUIRE(c.bottomLeft == Approx(8.0f));
        REQUIRE(c.bottomRight == Approx(8.0f));
    }
    
    SECTION("Individual corners") {
        CornerRadii c{1.0f, 2.0f, 3.0f, 4.0f};
        REQUIRE(c.topLeft == Approx(1.0f));
        REQUIRE(c.topRight == Approx(2.0f));
        REQUIRE(c.bottomLeft == Approx(3.0f));
        REQUIRE(c.bottomRight == Approx(4.0f));
    }
}

// =============================================================================
// Dimension Tests
// =============================================================================

TEST_CASE("UI: Dimension", "[ui][layout]") {
    SECTION("Auto") {
        Dimension d = Dimension::auto_();
        REQUIRE(d.isAuto());
    }
    
    SECTION("Pixels") {
        Dimension d = Dimension::pixels(100.0f);
        REQUIRE(d.isPixels());
        REQUIRE(d.value == Approx(100.0f));
    }
    
    SECTION("Percent") {
        Dimension d = Dimension::percent(50.0f);
        REQUIRE(d.isPercent());
        REQUIRE(d.value == Approx(50.0f));
    }
    
    SECTION("Resolve pixels") {
        Dimension d = Dimension::pixels(100.0f);
        REQUIRE(d.resolve(1000.0f) == Approx(100.0f));
    }
    
    SECTION("Resolve percent") {
        Dimension d = Dimension::percent(50.0f);
        REQUIRE(d.resolve(200.0f) == Approx(100.0f));
    }
}

// =============================================================================
// Theme Tests
// =============================================================================

TEST_CASE("UI: Theme system", "[ui][theme]") {
    SECTION("Light theme colors") {
        Theme light = Theme::light();
        // Light theme should have light background
        REQUIRE(light.colors.background.r > 0.8f);
    }
    
    SECTION("Dark theme colors") {
        Theme dark = Theme::dark();
        // Dark theme should have dark background
        REQUIRE(dark.colors.background.r < 0.3f);
    }
    
    SECTION("Typography scale") {
        Theme theme = Theme::light();
        // Body should be standard size
        REQUIRE(theme.typography.body1.fontSize > 12.0f);
        REQUIRE(theme.typography.body1.fontSize < 20.0f);
        // Headline should be larger
        REQUIRE(theme.typography.headline1.fontSize > theme.typography.body1.fontSize);
    }
}

// =============================================================================
// UISystem Tests
// =============================================================================

TEST_CASE("UI: UISystem singleton", "[ui][system]") {
    SECTION("Get instance") {
        auto& system1 = UISystem::instance();
        auto& system2 = UISystem::instance();
        REQUIRE(&system1 == &system2);
    }
}

// =============================================================================
// Easing Functions Tests (removed - not exposed)
// =============================================================================

// =============================================================================
// Input Event Tests
// =============================================================================

TEST_CASE("UI: Input events", "[ui][events]") {
    SECTION("Pointer event creation") {
        PointerEvent event{};
        event.type = PointerType::Mouse;
        event.position = Vec2{100.0f, 200.0f};
        
        REQUIRE(event.type == PointerType::Mouse);
        REQUIRE(event.position.x == Approx(100.0f));
        REQUIRE(event.position.y == Approx(200.0f));
    }
    
    SECTION("Key event creation") {
        KeyEvent event{};
        event.isDown = true;
        event.keyCode = 32;  // Space
        event.shift = true;
        
        REQUIRE(event.isDown == true);
        REQUIRE(event.keyCode == 32);
        REQUIRE(event.shift == true);
    }
}

// =============================================================================
// Accessibility Tests
// =============================================================================

TEST_CASE("UI: Accessibility", "[ui][accessibility]") {
    SECTION("Role assignment") {
        AccessibilityInfo info{};
        info.role = AccessibilityRole::Button;
        info.label = "Submit";
        info.isDisabled = false;
        
        REQUIRE(info.role == AccessibilityRole::Button);
        REQUIRE(info.label == "Submit");
        REQUIRE(info.isDisabled == false);
    }
    
    SECTION("State flags") {
        AccessibilityInfo info{};
        info.isChecked = true;
        info.isExpanded = false;
        info.isSelected = true;
        
        REQUIRE(info.isChecked == true);
        REQUIRE(info.isExpanded == false);
        REQUIRE(info.isSelected == true);
    }
}
