/**
 * @file application.cpp
 * @brief NovaCore Platform™ - Application Framework Implementation
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#include <nova/platform/application.hpp>

#include <chrono>
#include <thread>
#include <cmath>

#if defined(_WIN32)
#include <windows.h>
#include <shlobj.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <sys/sysctl.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/sysinfo.h>
#endif

namespace nova::platform {

// ============================================================================
// Singleton
// ============================================================================

Application* Application::s_instance = nullptr;

Application& Application::get() {
    return *s_instance;
}

// ============================================================================
// Constructor/Destructor
// ============================================================================

Application::Application() {
    s_instance = this;
}

Application::~Application() {
    if (m_state != AppState::Stopped) {
        shutdown();
    }
    s_instance = nullptr;
}

// ============================================================================
// Initialization
// ============================================================================

bool Application::initialize(const AppDesc& desc) {
    m_desc = desc;
    m_state = AppState::Starting;
    
    // Gather system information
    gatherSystemInfo();
    
    // Initialize timing
    m_targetFPS = desc.targetFPS;
    m_targetFrameTime = 1.0 / m_targetFPS;
    m_vsync = desc.window.vsync;
    
    // Create main window
    m_mainWindow = std::make_unique<Window>();
    if (!m_mainWindow->create(desc.window)) {
        return false;
    }
    
    // Initialize engine systems
    initSystems();
    
    // Call virtual init
    if (!onInit()) {
        shutdown();
        return false;
    }
    
    m_state = AppState::Running;
    if (m_appStateCallback) {
        m_appStateCallback(m_state);
    }
    onStateChange(m_state);
    
    return true;
}

void Application::shutdown() {
    m_state = AppState::Stopping;
    if (m_appStateCallback) {
        m_appStateCallback(m_state);
    }
    onStateChange(m_state);
    
    // Call virtual shutdown
    onShutdown();
    
    // Shutdown systems
    shutdownSystems();
    
    // Destroy window
    if (m_mainWindow) {
        m_mainWindow->destroy();
        m_mainWindow.reset();
    }
    
    m_state = AppState::Stopped;
    if (m_appStateCallback) {
        m_appStateCallback(m_state);
    }
}

// ============================================================================
// Main Loop
// ============================================================================

int Application::run() {
    auto startTime = std::chrono::high_resolution_clock::now();
    m_lastFrameTime = 0.0;
    
    while (!m_shouldQuit && !m_mainWindow->shouldClose()) {
        auto frameStart = std::chrono::high_resolution_clock::now();
        
        // Calculate delta time
        f64 currentTime = std::chrono::duration<f64>(frameStart - startTime).count();
        m_frameInfo.unscaledDeltaTime = currentTime - m_lastFrameTime;
        m_frameInfo.deltaTime = m_frameInfo.unscaledDeltaTime * m_frameInfo.timeScale;
        m_lastFrameTime = currentTime;
        m_frameInfo.totalTime = currentTime;
        
        // Process events
        processEvents();
        
        // Fixed update (physics)
        m_fixedTimeAccumulator += m_frameInfo.deltaTime;
        while (m_fixedTimeAccumulator >= m_fixedTimestep) {
            onFixedUpdate(static_cast<f32>(m_fixedTimestep));
            m_fixedTimeAccumulator -= m_fixedTimestep;
        }
        
        // Update
        updateFrame();
        
        // Render
        renderFrame();
        
        // Update frame info
        updateFrameInfo();
        
        // Frame rate limiting
        if (!m_desc.unlimitedFPS && !m_vsync) {
            auto frameEnd = std::chrono::high_resolution_clock::now();
            f64 frameTime = std::chrono::duration<f64>(frameEnd - frameStart).count();
            
            if (frameTime < m_targetFrameTime) {
                f64 sleepTime = m_targetFrameTime - frameTime;
                std::this_thread::sleep_for(
                    std::chrono::duration<f64>(sleepTime * 0.95));
                
                // Spin for remaining time for accuracy
                while (std::chrono::duration<f64>(
                    std::chrono::high_resolution_clock::now() - frameStart).count() < m_targetFrameTime) {
                    std::this_thread::yield();
                }
            }
        }
        
        m_frameInfo.frameNumber++;
    }
    
    return m_exitCode;
}

void Application::quit(int exitCode) {
    m_exitCode = exitCode;
    m_shouldQuit = true;
}

// ============================================================================
// Time Control
// ============================================================================

void Application::setTimeScale(f32 scale) {
    m_frameInfo.timeScale = std::max(0.0f, scale);
}

void Application::setTargetFPS(u32 fps) {
    m_targetFPS = std::max(1u, fps);
    m_targetFrameTime = 1.0 / m_targetFPS;
}

void Application::setVSync(bool enabled) {
    m_vsync = enabled;
    // Would call graphics API to set vsync
}

// ============================================================================
// Window Management
// ============================================================================

std::unique_ptr<Window> Application::createWindow(const WindowDesc& desc) {
    auto window = std::make_unique<Window>();
    if (window->create(desc)) {
        return window;
    }
    return nullptr;
}

const MonitorInfo* Application::getPrimaryMonitor() const {
    for (const auto& monitor : m_systemInfo.monitors) {
        if (monitor.isPrimary) {
            return &monitor;
        }
    }
    return m_systemInfo.monitors.empty() ? nullptr : &m_systemInfo.monitors[0];
}

// ============================================================================
// Paths
// ============================================================================

std::string Application::getSpecialFolderPath(SpecialFolder folder) const {
    #if defined(_WIN32)
    char path[MAX_PATH];
    KNOWNFOLDERID folderId;
    
    switch (folder) {
        case SpecialFolder::UserData: folderId = FOLDERID_RoamingAppData; break;
        case SpecialFolder::UserDocuments: folderId = FOLDERID_Documents; break;
        case SpecialFolder::UserDesktop: folderId = FOLDERID_Desktop; break;
        case SpecialFolder::UserDownloads: folderId = FOLDERID_Downloads; break;
        case SpecialFolder::UserPictures: folderId = FOLDERID_Pictures; break;
        case SpecialFolder::UserMusic: folderId = FOLDERID_Music; break;
        case SpecialFolder::UserVideos: folderId = FOLDERID_Videos; break;
        case SpecialFolder::AppData: folderId = FOLDERID_RoamingAppData; break;
        case SpecialFolder::LocalAppData: folderId = FOLDERID_LocalAppData; break;
        case SpecialFolder::Cache: folderId = FOLDERID_LocalAppData; break;
        case SpecialFolder::SaveGames: folderId = FOLDERID_SavedGames; break;
        default: return "";
    }
    
    PWSTR wpath;
    if (SHGetKnownFolderPath(folderId, 0, nullptr, &wpath) == S_OK) {
        size_t convertedChars = 0;
        wcstombs_s(&convertedChars, path, MAX_PATH, wpath, _TRUNCATE);
        CoTaskMemFree(wpath);
        return std::string(path);
    }
    return "";
    
    #elif defined(__APPLE__) || defined(__linux__)
    const char* home = getenv("HOME");
    if (!home) return "";
    
    switch (folder) {
        case SpecialFolder::UserData: return std::string(home) + "/.local/share";
        case SpecialFolder::UserDocuments: return std::string(home) + "/Documents";
        case SpecialFolder::UserDesktop: return std::string(home) + "/Desktop";
        case SpecialFolder::UserDownloads: return std::string(home) + "/Downloads";
        case SpecialFolder::UserPictures: return std::string(home) + "/Pictures";
        case SpecialFolder::UserMusic: return std::string(home) + "/Music";
        case SpecialFolder::UserVideos: return std::string(home) + "/Videos";
        case SpecialFolder::Cache: return std::string(home) + "/.cache";
        case SpecialFolder::Temp: return "/tmp";
        default: return "";
    }
    #else
    return "";
    #endif
}

std::string Application::getAppDataPath() const {
    std::string base = getSpecialFolderPath(SpecialFolder::UserData);
    if (base.empty()) return "";
    return base + "/" + m_desc.organization + "/" + m_desc.name;
}

std::string Application::getSaveGamePath() const {
    std::string base = getSpecialFolderPath(SpecialFolder::SaveGames);
    if (base.empty()) {
        base = getSpecialFolderPath(SpecialFolder::UserDocuments);
    }
    if (base.empty()) return "";
    return base + "/" + m_desc.name;
}

// ============================================================================
// System Functions
// ============================================================================

void Application::openURL(const std::string& url) {
    #if defined(_WIN32)
    ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    #elif defined(__APPLE__)
    std::string cmd = "open " + url;
    system(cmd.c_str());
    #elif defined(__linux__)
    std::string cmd = "xdg-open " + url;
    system(cmd.c_str());
    #endif
}

int Application::showMessageBox(const std::string& title,
                                const std::string& message,
                                MessageBoxType type,
                                MessageBoxButtons buttons) {
    #if defined(_WIN32)
    UINT mbType = 0;
    switch (type) {
        case MessageBoxType::Info: mbType |= MB_ICONINFORMATION; break;
        case MessageBoxType::Warning: mbType |= MB_ICONWARNING; break;
        case MessageBoxType::Error: mbType |= MB_ICONERROR; break;
        case MessageBoxType::Question: mbType |= MB_ICONQUESTION; break;
    }
    switch (buttons) {
        case MessageBoxButtons::Ok: mbType |= MB_OK; break;
        case MessageBoxButtons::OkCancel: mbType |= MB_OKCANCEL; break;
        case MessageBoxButtons::YesNo: mbType |= MB_YESNO; break;
        case MessageBoxButtons::YesNoCancel: mbType |= MB_YESNOCANCEL; break;
    }
    return MessageBoxA(nullptr, message.c_str(), title.c_str(), mbType);
    #else
    // Would use platform-specific dialog
    return 0;
    #endif
}

void Application::setClipboardText(const std::string& text) {
    #if defined(_WIN32)
    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
        if (hMem) {
            char* mem = static_cast<char*>(GlobalLock(hMem));
            memcpy(mem, text.c_str(), text.size() + 1);
            GlobalUnlock(hMem);
            SetClipboardData(CF_TEXT, hMem);
        }
        CloseClipboard();
    }
    #endif
}

std::string Application::getClipboardText() const {
    std::string result;
    #if defined(_WIN32)
    if (OpenClipboard(nullptr)) {
        HANDLE hData = GetClipboardData(CF_TEXT);
        if (hData) {
            char* text = static_cast<char*>(GlobalLock(hData));
            if (text) {
                result = text;
                GlobalUnlock(hData);
            }
        }
        CloseClipboard();
    }
    #endif
    return result;
}

// ============================================================================
// Callbacks
// ============================================================================

void Application::setAppStateCallback(AppStateCallback callback) {
    m_appStateCallback = std::move(callback);
}

void Application::setLowMemoryCallback(LowMemoryCallback callback) {
    m_lowMemoryCallback = std::move(callback);
}

// ============================================================================
// Internal Methods
// ============================================================================

void Application::initSystems() {
    // Would initialize all engine systems here
    // input::initializeInput();
    // audio::initializeAudio();
    // script::initializeScript();
    // resource::initializeResources();
}

void Application::shutdownSystems() {
    // Would shutdown all engine systems here
    // resource::shutdownResources();
    // script::shutdownScript();
    // audio::shutdownAudio();
    // input::shutdownInput();
}

void Application::processEvents() {
    if (m_mainWindow) {
        m_mainWindow->processEvents();
    }
    
    // Would process input events
    // input::getInput().update();
}

void Application::updateFrame() {
    auto start = std::chrono::high_resolution_clock::now();
    
    onUpdate(static_cast<f32>(m_frameInfo.deltaTime));
    
    auto end = std::chrono::high_resolution_clock::now();
    m_frameInfo.updateTime = std::chrono::duration<f64, std::milli>(end - start).count();
}

void Application::renderFrame() {
    auto start = std::chrono::high_resolution_clock::now();
    
    onRender();
    onGUI();
    
    if (m_mainWindow) {
        m_mainWindow->present();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    m_frameInfo.renderTime = std::chrono::duration<f64, std::milli>(end - start).count();
}

void Application::updateFrameInfo() {
    m_frameInfo.frameTime = m_frameInfo.updateTime + m_frameInfo.renderTime;
    m_frameInfo.fps = 1.0f / static_cast<f32>(m_frameInfo.unscaledDeltaTime);
    
    // Running average FPS
    static f32 fpsAccum = 0.0f;
    static u32 fpsCount = 0;
    fpsAccum += m_frameInfo.fps;
    fpsCount++;
    
    if (fpsCount >= 60) {
        m_frameInfo.averageFps = fpsAccum / fpsCount;
        fpsAccum = 0.0f;
        fpsCount = 0;
    }
}

void Application::gatherSystemInfo() {
    m_systemInfo.platform = getCurrentPlatform();
    m_systemInfo.architecture = getCurrentArchitecture();
    
    #if defined(_WIN32)
    // OS info
    m_systemInfo.osName = "Windows";
    
    // CPU info
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    m_systemInfo.cpu.coreCount = sysInfo.dwNumberOfProcessors;
    m_systemInfo.cpu.threadCount = sysInfo.dwNumberOfProcessors;
    
    // Memory info
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    GlobalMemoryStatusEx(&memStatus);
    m_systemInfo.memory.totalPhysical = memStatus.ullTotalPhys;
    m_systemInfo.memory.availablePhysical = memStatus.ullAvailPhys;
    m_systemInfo.memory.totalVirtual = memStatus.ullTotalVirtual;
    m_systemInfo.memory.availableVirtual = memStatus.ullAvailVirtual;
    
    #elif defined(__linux__)
    m_systemInfo.osName = "Linux";
    
    // CPU cores
    m_systemInfo.cpu.coreCount = sysconf(_SC_NPROCESSORS_ONLN);
    m_systemInfo.cpu.threadCount = m_systemInfo.cpu.coreCount;
    
    // Memory
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        m_systemInfo.memory.totalPhysical = info.totalram * info.mem_unit;
        m_systemInfo.memory.availablePhysical = info.freeram * info.mem_unit;
    }
    
    #elif defined(__APPLE__)
    m_systemInfo.osName = "macOS";
    
    // CPU cores
    int coreCount;
    size_t size = sizeof(coreCount);
    sysctlbyname("hw.physicalcpu", &coreCount, &size, nullptr, 0);
    m_systemInfo.cpu.coreCount = coreCount;
    
    sysctlbyname("hw.logicalcpu", &coreCount, &size, nullptr, 0);
    m_systemInfo.cpu.threadCount = coreCount;
    
    // Memory
    int64_t mem;
    size = sizeof(mem);
    sysctlbyname("hw.memsize", &mem, &size, nullptr, 0);
    m_systemInfo.memory.totalPhysical = mem;
    #endif
    
    // Monitor info (placeholder - would use platform windowing system)
    MonitorInfo primary;
    primary.name = "Primary Monitor";
    primary.index = 0;
    primary.isPrimary = true;
    primary.size = Vec2i(1920, 1080);
    primary.dpi = 96.0f;
    primary.scale = 1.0f;
    m_systemInfo.monitors.push_back(primary);
}

} // namespace nova::platform
