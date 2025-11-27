/**
 * @file window.cpp
 * @brief NovaCore Platform™ - Window Management Implementation
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#include <nova/platform/window.hpp>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
// Would include X11 or Wayland headers
#elif defined(__APPLE__)
// Would include Cocoa headers
#endif

namespace nova::platform {

// ============================================================================
// Constructor/Destructor
// ============================================================================

Window::Window() = default;

Window::~Window() {
    if (m_handle) {
        destroy();
    }
}

// ============================================================================
// Creation/Destruction
// ============================================================================

bool Window::create(const WindowDesc& desc) {
    m_title = desc.title;
    m_size = Vec2i(desc.width, desc.height);
    
    createPlatformWindow(desc);
    
    if (!m_handle) {
        return false;
    }
    
    updateWindowMetrics();
    
    if (hasStyle(desc.style, WindowStyle::Fullscreen)) {
        setFullscreen(desc.fullscreenMode, desc.targetMonitor);
    }
    
    return true;
}

void Window::destroy() {
    destroyPlatformWindow();
    m_handle = nullptr;
}

void Window::processEvents() {
    #if defined(_WIN32)
    MSG msg;
    while (PeekMessage(&msg, static_cast<HWND>(m_handle), 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    #endif
}

void Window::present() {
    // Would swap buffers via graphics API
}

// ============================================================================
// Window State
// ============================================================================

void* Window::getNativeHandle() const {
    return m_handle;
}

void Window::setTitle(const std::string& title) {
    m_title = title;
    #if defined(_WIN32)
    SetWindowTextA(static_cast<HWND>(m_handle), title.c_str());
    #endif
}

void Window::setPosition(i32 x, i32 y) {
    m_position = Vec2i(x, y);
    #if defined(_WIN32)
    SetWindowPos(static_cast<HWND>(m_handle), nullptr, x, y, 0, 0,
                SWP_NOSIZE | SWP_NOZORDER);
    #endif
}

void Window::setSize(u32 width, u32 height) {
    m_size = Vec2i(width, height);
    #if defined(_WIN32)
    RECT rect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    SetWindowPos(static_cast<HWND>(m_handle), nullptr, 0, 0,
                rect.right - rect.left, rect.bottom - rect.top,
                SWP_NOMOVE | SWP_NOZORDER);
    #endif
    updateWindowMetrics();
}

// ============================================================================
// State Control
// ============================================================================

void Window::minimize() {
    #if defined(_WIN32)
    ShowWindow(static_cast<HWND>(m_handle), SW_MINIMIZE);
    #endif
    m_state = WindowState::Minimized;
    if (m_stateCallback) m_stateCallback(m_state);
}

void Window::maximize() {
    #if defined(_WIN32)
    ShowWindow(static_cast<HWND>(m_handle), SW_MAXIMIZE);
    #endif
    m_state = WindowState::Maximized;
    if (m_stateCallback) m_stateCallback(m_state);
}

void Window::restore() {
    #if defined(_WIN32)
    ShowWindow(static_cast<HWND>(m_handle), SW_RESTORE);
    #endif
    m_state = WindowState::Normal;
    if (m_stateCallback) m_stateCallback(m_state);
}

void Window::show() {
    #if defined(_WIN32)
    ShowWindow(static_cast<HWND>(m_handle), SW_SHOW);
    #endif
    m_visible = true;
}

void Window::hide() {
    #if defined(_WIN32)
    ShowWindow(static_cast<HWND>(m_handle), SW_HIDE);
    #endif
    m_visible = false;
}

void Window::focus() {
    #if defined(_WIN32)
    SetForegroundWindow(static_cast<HWND>(m_handle));
    SetFocus(static_cast<HWND>(m_handle));
    #endif
}

// ============================================================================
// Fullscreen
// ============================================================================

void Window::setFullscreen(FullscreenMode mode, u32 monitorIndex) {
    if (mode == m_fullscreenMode) return;
    
    // Store windowed state
    if (m_fullscreenMode == FullscreenMode::Windowed) {
        m_windowedPosition = m_position;
        m_windowedSize = m_size;
    }
    
    m_fullscreenMode = mode;
    m_monitorIndex = monitorIndex;
    
    #if defined(_WIN32)
    HWND hwnd = static_cast<HWND>(m_handle);
    
    if (mode == FullscreenMode::Windowed) {
        // Restore windowed
        SetWindowLong(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
        SetWindowPos(hwnd, nullptr,
                    m_windowedPosition.x, m_windowedPosition.y,
                    m_windowedSize.x, m_windowedSize.y,
                    SWP_NOZORDER | SWP_FRAMECHANGED);
        ShowWindow(hwnd, SW_NORMAL);
    }
    else if (mode == FullscreenMode::Borderless) {
        // Borderless fullscreen
        HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(hMonitor, &mi);
        
        SetWindowLong(hwnd, GWL_STYLE, WS_POPUP);
        SetWindowPos(hwnd, HWND_TOP,
                    mi.rcMonitor.left, mi.rcMonitor.top,
                    mi.rcMonitor.right - mi.rcMonitor.left,
                    mi.rcMonitor.bottom - mi.rcMonitor.top,
                    SWP_FRAMECHANGED);
        ShowWindow(hwnd, SW_MAXIMIZE);
    }
    else if (mode == FullscreenMode::Exclusive) {
        // Exclusive fullscreen (would change display mode)
        // Similar to borderless for now
        setFullscreen(FullscreenMode::Borderless, monitorIndex);
    }
    #endif
    
    m_state = (mode == FullscreenMode::Windowed) ? WindowState::Normal : WindowState::Fullscreen;
    updateWindowMetrics();
    
    if (m_stateCallback) m_stateCallback(m_state);
}

void Window::toggleFullscreen() {
    if (m_fullscreenMode == FullscreenMode::Windowed) {
        setFullscreen(FullscreenMode::Borderless);
    } else {
        setFullscreen(FullscreenMode::Windowed);
    }
}

// ============================================================================
// Display
// ============================================================================

void Window::setDisplayMode(const DisplayMode& mode) {
    m_displayMode = mode;
    
    if (m_fullscreenMode == FullscreenMode::Exclusive) {
        #if defined(_WIN32)
        DEVMODE dm = {};
        dm.dmSize = sizeof(dm);
        dm.dmPelsWidth = mode.width;
        dm.dmPelsHeight = mode.height;
        dm.dmBitsPerPel = mode.bitsPerPixel;
        dm.dmDisplayFrequency = mode.refreshRate;
        dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
        
        ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
        #endif
    }
}

// ============================================================================
// Misc
// ============================================================================

void Window::setIcon(const u8* pixels, u32 width, u32 height) {
    #if defined(_WIN32)
    // Would create HICON from pixel data and set via SendMessage WM_SETICON
    #endif
}

void Window::flash() {
    #if defined(_WIN32)
    FLASHWINFO fi = {};
    fi.cbSize = sizeof(fi);
    fi.hwnd = static_cast<HWND>(m_handle);
    fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
    fi.uCount = 3;
    FlashWindowEx(&fi);
    #endif
}

void Window::requestAttention() {
    #if defined(_WIN32)
    FLASHWINFO fi = {};
    fi.cbSize = sizeof(fi);
    fi.hwnd = static_cast<HWND>(m_handle);
    fi.dwFlags = FLASHW_TRAY;
    fi.uCount = 1;
    FlashWindowEx(&fi);
    #endif
}

void Window::setCursorVisible(bool visible) {
    m_cursorVisible = visible;
    #if defined(_WIN32)
    ShowCursor(visible ? TRUE : FALSE);
    #endif
}

void Window::setConfineCursor(bool confine) {
    m_cursorConfined = confine;
    #if defined(_WIN32)
    if (confine) {
        RECT rect;
        GetClientRect(static_cast<HWND>(m_handle), &rect);
        POINT pt = {rect.left, rect.top};
        ClientToScreen(static_cast<HWND>(m_handle), &pt);
        rect.left = pt.x;
        rect.top = pt.y;
        pt = {rect.right, rect.bottom};
        ClientToScreen(static_cast<HWND>(m_handle), &pt);
        rect.right = pt.x;
        rect.bottom = pt.y;
        ClipCursor(&rect);
    } else {
        ClipCursor(nullptr);
    }
    #endif
}

// ============================================================================
// Platform-Specific
// ============================================================================

#if defined(_WIN32)

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    
    switch (msg) {
        case WM_CLOSE:
            if (window) {
                window->setShouldClose(true);
            }
            return 0;
            
        case WM_SIZE:
            // Resize handling
            return 0;
            
        case WM_MOVE:
            // Move handling
            return 0;
            
        case WM_SETFOCUS:
        case WM_KILLFOCUS:
            // Focus handling
            return 0;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Window::createPlatformWindow(const WindowDesc& desc) {
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    
    // Register window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = "NovaWindowClass";
    RegisterClassEx(&wc);
    
    // Calculate window style
    DWORD style = WS_OVERLAPPEDWINDOW;
    if (hasStyle(desc.style, WindowStyle::Borderless)) {
        style = WS_POPUP;
    }
    if (!hasStyle(desc.style, WindowStyle::Resizable)) {
        style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    }
    if (!hasStyle(desc.style, WindowStyle::Minimizable)) {
        style &= ~WS_MINIMIZEBOX;
    }
    if (!hasStyle(desc.style, WindowStyle::Maximizable)) {
        style &= ~WS_MAXIMIZEBOX;
    }
    
    // Calculate window rect
    RECT rect = {0, 0, static_cast<LONG>(desc.width), static_cast<LONG>(desc.height)};
    AdjustWindowRect(&rect, style, FALSE);
    
    int x = desc.x;
    int y = desc.y;
    if (x < 0) x = CW_USEDEFAULT;
    if (y < 0) y = CW_USEDEFAULT;
    
    // Create window
    HWND hwnd = CreateWindowEx(
        0,
        "NovaWindowClass",
        desc.title.c_str(),
        style,
        x, y,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );
    
    if (hwnd) {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        ShowWindow(hwnd, SW_SHOW);
        m_handle = hwnd;
    }
}

void Window::destroyPlatformWindow() {
    if (m_handle) {
        DestroyWindow(static_cast<HWND>(m_handle));
    }
}

void Window::updateWindowMetrics() {
    if (!m_handle) return;
    
    HWND hwnd = static_cast<HWND>(m_handle);
    
    RECT rect;
    GetWindowRect(hwnd, &rect);
    m_position = Vec2i(rect.left, rect.top);
    m_size = Vec2i(rect.right - rect.left, rect.bottom - rect.top);
    
    GetClientRect(hwnd, &rect);
    m_clientSize = Vec2i(rect.right, rect.bottom);
    m_framebufferSize = m_clientSize;  // Would scale for high DPI
    
    // Get DPI scale
    HDC hdc = GetDC(hwnd);
    int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(hwnd, hdc);
    m_contentScale = Vec2(dpi / 96.0f, dpi / 96.0f);
}

#else

// Linux/macOS/other platforms would have their own implementations
void Window::createPlatformWindow(const WindowDesc& desc) {
    // Platform-specific window creation
}

void Window::destroyPlatformWindow() {
    // Platform-specific window destruction
}

void Window::updateWindowMetrics() {
    // Platform-specific metrics update
}

#endif

} // namespace nova::platform
