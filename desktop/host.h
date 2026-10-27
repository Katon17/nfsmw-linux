#pragma once
// What the Linux host's parts share: the window, the app's lifecycle and the event routing
#include <cstdint>

union SDL_Event;
struct SDL_Window;

namespace host {

SDL_Window* Window();
// the app is in front with a window to draw to; the renderer and the game wait while it is not
bool Foreground();
void WaitForeground();
// a change to the window's size, rotation or presence: incremented on the main thread, the renderer waits for a new
// surface when its copy is behind
uint32_t SurfaceEpoch();
// the window's drawable size in pixels (SDL_GetWindowSizeInPixels)
void WindowPixels(int& width, int& height);
}  // namespace host

// controllers through SDL (input.cpp)
namespace input {
struct Pad;
void OnPadEvent(const SDL_Event& e);
}  // namespace input

// the touch controls and keyboard/mouse (touch.cpp)
namespace touch {
void Init();
bool OnEvent(const SDL_Event& e);
void Apply(int port, input::Pad& pad);
// render thread, in the ImGui frame: the controls over the game
void Draw();
// any touch on the screen in the last few seconds: the controls show
bool Shown();
// main thread, each turn of the event loop: the tilt steering's sensor (gyro.cpp)
void Pump();
// a controller was used: the game's hints name its buttons again, not the touch controls
void PadUsed();
// the layout's settings in the game's options (src/common/menu.cpp), saved at once
uint32_t ControlsOpacity();  // percent
void StepOpacity(int step);
uint32_t ControlsSize();  // percent of each control's own size
void StepSize(int step);
const char* StickModeName();  // FLOATING or FIXED
void StepStickMode(int step);
const char* SteeringName();  // ARROWS, WHEEL or TILT: what steers in the RACING layout
void StepSteering(int step);
bool TiltSteering();         // any thread: STEERING is TILT
void EditLayout();  // the controls over the screen to drag where they go
void LoadArt();     // main thread, once ImGui's context is made: the controls' art into its font atlas
}  // namespace touch

// steering by tilting the phone (gyro.cpp), main thread
namespace gyro {
void OnEvent(const SDL_Event& e);
void Pump();     // the accelerometer open while STEERING is TILT and the app in front, closed otherwise
float Steer();   // any thread: -1 a full lock left to 1 right, 0 while off
}  // namespace gyro

// ImGui's input from SDL's events (ui.cpp), main thread, under app::UiLock
namespace ui {
void OnEvent(const SDL_Event& e);
// render thread, before ImGui::NewFrame: the display size and the frame time
void NewFrame(float width, float height);
// render thread, under app::UiLock, before the backend's NewFrame: the game's font into the atlas once it is loaded;
// true when the atlas is to be uploaded again
bool ApplyGameFont();
}  // namespace ui
