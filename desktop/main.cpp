// The Linux desktop host (black-linux): SDL's window and event loop, the settings and folders, the app's lifecycle;
// derived from Black for Android's host (MIT, Rangga Casper). The game is described by app::kGame
// (src/common/game_info.cpp)
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cwchar>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include <imgui.h>
#include <toml++/toml.hpp>

#include "app/app.h"
#include "build_info.h"
#include "common/options.h"
#include "core/input.h"
#include "core/log.h"
#include "host.h"
#include "image_symbols.h"
#include "kernel/kernel.h"

namespace {

SDL_Window* g_window = nullptr;
std::atomic<bool> g_foreground{true};
std::atomic<uint32_t> g_surface_epoch{0};
std::mutex g_foreground_lock;
std::condition_variable g_foreground_cv;

std::string Utf8(const wchar_t* w) {
    std::string s;
    for (; w && *w; w++) s += *w < 0x80 ? char(*w) : '?';
    return s;
}

// the first line of a file under /proc or /etc that starts with key, after the key and its separator
std::string FileField(const char* path, const char* key) {
    std::ifstream f(path);
    size_t n = std::strlen(key);
    for (std::string line; std::getline(f, line);) {
        if (line.compare(0, n, key)) continue;
        size_t at = line.find_first_not_of(" \t:=\"", n);
        std::string v = at == std::string::npos ? std::string() : line.substr(at);
        while (!v.empty() && (v.back() == '"' || v.back() == ' ')) v.pop_back();
        return v;
    }
    return "unknown";
}

void SetForeground(bool on) {
    {
        std::lock_guard lock(g_foreground_lock);
        if (on && !g_foreground) g_surface_epoch++;
        g_foreground = on;
    }
    g_foreground_cv.notify_all();
    LOGI("app: %s", on ? "in front" : "in the background");
}

// the window's size once it is up, else the desktop's display mode: the window is not up yet when the settings load
app::Mode ScreenMode() {
    if (g_window) {
        int w = 0, h = 0;
        host::WindowPixels(w, h);
        if (w > 0 && h > 0) return {uint32_t(w), uint32_t(h)};
    }
    const SDL_DisplayMode* m = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());
    if (!m) return {1920, 1080};
    return {uint32_t(m->w * m->pixel_density), uint32_t(m->h * m->pixel_density)};
}

float RefreshRate() {
    SDL_DisplayID d = g_window ? SDL_GetDisplayForWindow(g_window) : SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* m = SDL_GetCurrentDisplayMode(d);
    return m && m->refresh_rate > 1 ? m->refresh_rate : 60.0f;
}

}  // namespace

namespace host {

SDL_Window* Window() { return g_window; }
bool Foreground() { return g_foreground; }
uint32_t SurfaceEpoch() { return g_surface_epoch; }

void WaitForeground() {
    std::unique_lock lock(g_foreground_lock);
    g_foreground_cv.wait(lock, [] { return g_foreground.load(); });
}

void WindowPixels(int& w, int& h) {
    w = h = 0;
    if (g_window) SDL_GetWindowSizeInPixels(g_window, &w, &h);
}

}  // namespace host

namespace app {

namespace {

const char* kModes[] = {"fullscreen", "borderless", "windowed"};
// the defaults a settings file was written under: 2 has the frame rate unlimited and vsync off, which a file from
// before takes once, its player's own choices kept from then on
constexpr int kDefaults = 2;

std::filesystem::path File() { return ExeDir() / kGame.settings; }

toml::table Table() {
    const Settings& s = Get();
    toml::table t{{"display_mode", kModes[uint32_t(s.display_mode)]},
                  {"width", int64_t(s.width)},
                  {"height", int64_t(s.height)},
                  {"render_percent", int64_t(s.render_percent)},
                  {"fps_cap", int64_t(s.fps_cap)},
                  {"vsync", s.vsync},
                  {"anisotropic_filtering", int64_t(s.aniso)},
                  {"join_ip", s.join_ip},
                  {"defaults", int64_t(kDefaults)}};
    for (const Option& o : kGame.options) {
        if (o.flag) t.insert(o.key, *o.flag);
        else t.insert(o.key, int64_t(*o.value));
    }
    if (kGame.save) kGame.save(&t);
    return t;
}

// a laptop's integrated GPU (an HD 620's) renders the console's frame at about 720p on a first run
uint32_t FirstRunRenderPercent() {
    const SDL_DisplayMode* m = SDL_WasInit(SDL_INIT_VIDEO) ? SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay()) : nullptr;
    int h = m ? int(m->h * m->pixel_density) : 0;
    if (h <= 800) return 100;
    return uint32_t(std::clamp(int(std::lround(72000.0 / h)), 34, 100));
}

// vsync on and the frame rate up to the display's: cooler and quieter on a laptop, no tearing
void PhoneFrameDefaults(Settings& s) {
    s.fps_cap = 0;
    s.vsync = true;
}

}  // namespace

// the settings a laptop starts with, for a first run and the options' restore
Settings DefaultSettings() {
    Settings s;
    s.display_mode = DisplayMode::Fullscreen;
    s.render_percent = FirstRunRenderPercent();
    PhoneFrameDefaults(s);
    return s;
}

Settings& Get() {
    static Settings s = DefaultSettings();
    return s;
}

void Load() {
    Settings& s = Get();
    toml::table t;
    try {
        t = toml::parse_file(File().string());
    } catch (...) {
        s.render_percent = FirstRunRenderPercent();  // the display is known by now
        return;
    }
    std::string mode = t["display_mode"].value_or(std::string(kModes[uint32_t(s.display_mode)]));
    for (uint32_t i = 0; i < 3; i++)
        if (mode == kModes[i]) s.display_mode = DisplayMode(i);
    s.width = t["width"].value_or(s.width);
    s.height = t["height"].value_or(s.height);
    s.render_percent = t["render_percent"].value_or(s.render_percent);
    s.fps_cap = t["fps_cap"].value_or(s.fps_cap);
    s.vsync = t["vsync"].value_or(s.vsync);
    s.aniso = t["anisotropic_filtering"].value_or(s.aniso);
    s.join_ip = t["join_ip"].value_or(s.join_ip);
    if (t["defaults"].value_or(int64_t(0)) < kDefaults) PhoneFrameDefaults(s);
    for (const Option& o : kGame.options) {
        if (o.flag) *o.flag = t[o.key].value_or(*o.flag);
        else *o.value = t[o.key].value_or(*o.value);
    }
    if (kGame.load) kGame.load(&t);
}

void Save() {
    std::ofstream f(File());
    f << Table() << "\n";
}

void LogSettings() {
    std::ostringstream text;
    text << Table();
    std::istringstream lines(text.str());
    LOGR("Settings:");
    for (std::string line; std::getline(lines, line);)
        if (!line.empty()) LOGR("  %s", line.c_str());
}

#ifndef NFSMW_COMMIT
#define NFSMW_COMMIT "0.1.0"
#endif

void LogSystem() {
    std::time_t now = std::time(nullptr);
    char date[32] = {};
    std::strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    LOGR("%ls (%s), %s, build %s", kGame.title, IMAGE_XBE, date, NFSMW_COMMIT);
    LOGR("Path: %s", ExeDir().c_str());
    utsname u{};
    uname(&u);
    LOGR("Device: %s %s, %s, Linux %s, video %s", FileField("/sys/class/dmi/id/sys_vendor", "").c_str(),
         FileField("/sys/class/dmi/id/product_version", "").c_str(), FileField("/etc/os-release", "PRETTY_NAME").c_str(),
         u.release, SDL_GetCurrentVideoDriver() ? SDL_GetCurrentVideoDriver() : "none");
    LOGR("CPU: %s (%d threads)", FileField("/proc/cpuinfo", "model name").c_str(), SDL_GetNumLogicalCPUCores());
    LOGR("RAM: %.1f GB", SDL_GetSystemRAM() / 1024.0);
    Mode desk = Desktop();
    LOGR("Display: %ux%u, %.0f Hz", desk.width, desk.height, RefreshRate());
    LogSettings();
}

// the display mode setting onto the window, on the main thread (SDL's window calls belong there)
void ApplyDisplay() {
    SDL_Event e{};
    e.type = SDL_EVENT_USER;
    SDL_PushEvent(&e);
}

Mode Desktop() {
    SDL_DisplayID d = g_window ? SDL_GetDisplayForWindow(g_window) : SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* m = SDL_GetDesktopDisplayMode(d);
    if (!m) return {1920, 1080};
    return {uint32_t(m->w * m->pixel_density), uint32_t(m->h * m->pixel_density)};
}

Mode Output() {
    // the window is the output: the render resolution follows its size, fullscreen or not
    return ScreenMode();
}

bool Widescreen() {
    Mode m = Output();
    return m.width * 2 >= m.height * 3;  // 3:2 or wider
}

// SSAA renders twice as wide and high, the present averaging each 2x2 back to a pixel
float RenderScale(uint32_t guest_height) {
    float ssaa = game::Opt().anti_aliasing == 2 ? 2.0f : 1.0f;
    float h = Output().height * (Get().render_percent / 100.0f) * ssaa;
    return guest_height ? std::max(h / guest_height, 0.1f) : 1.0f;
}

// the player's data folder: settings, saves (hdd), logs, the shader cache and the game files' default place.
// NFSMW_HOME, else $XDG_DATA_HOME/nfsmw-linux, else ~/.local/share/nfsmw-linux
std::filesystem::path ExeDir() {
    static const std::filesystem::path dir = [] {
        namespace fs = std::filesystem;
        fs::path p;
        if (const char* b = std::getenv("NFSMW_HOME"); b && *b) p = b;
        else if (const char* x = std::getenv("XDG_DATA_HOME"); x && *x) p = fs::path(x) / "nfsmw-linux";
        else if (const char* h = std::getenv("HOME"); h && *h) p = fs::path(h) / ".local/share/nfsmw-linux";
        else p = ".";
        std::error_code ec;
        fs::create_directories(p, ec);
        return p;
    }();
    return dir;
}

int Instance() { return 0; }

void* Window() { return g_window; }

// no splash and no relaunch into another executable
bool OpenSplash() { return false; }
void SplashFirstFrame() {}
void CloseSplash() {}
void OpenGameWindow(bool) {}
bool StartHandover() { return false; }
bool HandingOver() { return false; }
void FinishHandover() {}

std::recursive_mutex& UiLock() {
    static std::recursive_mutex m;
    return m;
}

void Pace() {
    using clock = std::chrono::steady_clock;
    static clock::time_point next = clock::now();
    uint32_t cap = Get().fps_cap;
    auto now = clock::now();
    if (!cap) {
        next = now;
        return;
    }
    auto period = std::chrono::nanoseconds(1000000000ull / cap);
    next += period;
    if (next < now - period) next = now;
    while (clock::now() + std::chrono::milliseconds(2) < next) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    while (clock::now() < next) std::this_thread::yield();
}

}  // namespace app

namespace {

void InitUi() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
    io.BackendPlatformName = "linux_sdl3";
}

// fullscreen is the desktop's mode (no mode switch), borderless the same, windowed 1280x720 or the settings' size
void ApplyWindowMode() {
    if (!g_window) return;
    const app::Settings& s = app::Get();
    if (s.display_mode == app::DisplayMode::Windowed) {
        SDL_SetWindowFullscreen(g_window, false);
        SDL_SetWindowBordered(g_window, true);
        int w = s.width ? int(s.width) : 1280, h = s.height ? int(s.height) : 720;
        SDL_SetWindowSize(g_window, w, h);
        SDL_SetWindowPosition(g_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    } else {
        SDL_SetWindowFullscreenMode(g_window, nullptr);
        SDL_SetWindowFullscreen(g_window, true);
    }
    LOGI("app: %s", s.display_mode == app::DisplayMode::Windowed ? "windowed" : "fullscreen");
}

void Dispatch(const SDL_Event& e) {
    switch (e.type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_TERMINATING:
            logx::Flush();
            kernel::Shutdown(0);
            break;
        case SDL_EVENT_WINDOW_MINIMIZED:
        case SDL_EVENT_WINDOW_HIDDEN:
            SetForeground(false);
            break;
        case SDL_EVENT_WINDOW_RESTORED:
        case SDL_EVENT_WINDOW_SHOWN:
        case SDL_EVENT_WINDOW_MAXIMIZED:
            SetForeground(true);
            break;
        case SDL_EVENT_USER:
            ApplyWindowMode();
            return;
        case SDL_EVENT_KEY_DOWN:
            // F11 or Alt+Enter: fullscreen and a window, kept in the settings
            if (!e.key.repeat && (e.key.scancode == SDL_SCANCODE_F11 ||
                                  (e.key.scancode == SDL_SCANCODE_RETURN && (e.key.mod & SDL_KMOD_ALT)))) {
                app::Settings& s = app::Get();
                s.display_mode = s.display_mode == app::DisplayMode::Windowed ? app::DisplayMode::Fullscreen
                                                                              : app::DisplayMode::Windowed;
                app::Save();
                ApplyWindowMode();
                return;
            }
            break;
        case SDL_EVENT_SENSOR_UPDATE:
        case SDL_EVENT_GAMEPAD_SENSOR_UPDATE:
            gyro::OnEvent(e);
            return;
        case SDL_EVENT_GAMEPAD_ADDED:
        case SDL_EVENT_GAMEPAD_REMOVED:
            input::OnPadEvent(e);
            break;
        default:
            break;
    }
    if (touch::OnEvent(e)) return;
    std::lock_guard lock(app::UiLock());
    ui::OnEvent(e);
}

}  // namespace

int main(int, char**) {
    SDL_SetAppMetadata("Black (Linux)", "1.0", "io.github.blacklinux.Black");
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
    SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "0");
    SDL_SetHint(SDL_HINT_APP_ID, "black-linux");  // the .desktop file's name: the panel shows its icon
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO)) {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    SDL_InitSubSystem(SDL_INIT_SENSOR);  // a controller's gyroscope, if any; not needed
    logx::Init((app::ExeDir() / "logs").string(), app::kGame.log);
    app::Load();
    if (app::kGame.init) app::kGame.init();
    app::LogSystem();

    std::string title = Utf8(app::kGame.title);
    const app::Settings& set = app::Get();
    bool windowed = set.display_mode == app::DisplayMode::Windowed || std::getenv("BLACK_WINDOWED");
    int ww = windowed && set.width ? int(set.width) : 1280, wh = windowed && set.height ? int(set.height) : 720;
    g_window = SDL_CreateWindow(title.c_str(), ww, wh,
                                (windowed ? SDL_WINDOW_RESIZABLE : SDL_WINDOW_FULLSCREEN) | SDL_WINDOW_VULKAN |
                                    SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!g_window) {
        LOGE("app: no window: %s", SDL_GetError());
        return 1;
    }

    app::GameFiles game;
    if (!app::FindGame(game)) {
        logx::Flush();
        return 1;
    }

    InitUi();
    input::Init();
    if (app::kGame.window) app::kGame.window(g_window);
    if (!kernel::Boot(std::move(game.xbe), IMAGE_XBE, game.dir.string(), (app::ExeDir() / "hdd").string())) {
        std::string text = "The game failed to start, see the log in " + (app::ExeDir() / "logs").string();
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, Utf8(app::kGame.short_title).c_str(), text.c_str(), g_window);
        return 1;
    }

    for (;;) {
        SDL_Event e;
        if (SDL_WaitEventTimeout(&e, 100)) {
            Dispatch(e);
            while (SDL_PollEvent(&e)) Dispatch(e);
        }
        touch::Pump();
    }
}
