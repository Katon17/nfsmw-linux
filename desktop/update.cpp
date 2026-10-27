// The options menu's update check (black-linux): the project's releases page in the browser
#include <SDL3/SDL.h>

namespace host {

void CheckForUpdates() { SDL_OpenURL("https://github.com/RanggaCasper/Black-Android/releases"); }

}  // namespace host
