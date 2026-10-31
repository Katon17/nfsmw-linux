#include <cstddef>
#include <string>

namespace input {

struct Action {
    const char* id;
    const char* label;
    const char* defaults;
};

__attribute__((weak)) const Action* PadActions(size_t* count) {
    if (count) *count = 0;
    return nullptr;
}

__attribute__((weak)) std::string PadBinding(size_t) {
    return "";
}

__attribute__((weak)) void PadBind(size_t, const std::string&) {}

__attribute__((weak)) void ResetPadBindings() {}

__attribute__((weak)) void LoadPadBindings() {}

__attribute__((weak)) std::string PressedPadInput() {
    return "";
}

}  // namespace input

namespace host {

__attribute__((weak)) void SamplePerf() {}

__attribute__((weak)) float CpuLoad() {
    return -1.0f;
}

__attribute__((weak)) float GpuLoad() {
    return -1.0f;
}

__attribute__((weak)) float CpuTemp() {
    return -1.0f;
}

__attribute__((weak)) float GpuTemp() {
    return -1.0f;
}

__attribute__((weak)) float BatteryTemp() {
    return -1.0f;
}

}  // namespace host

