#include <cstddef>
#include <string>

namespace input {

struct Action {
    const char* id;
    const char* label;
    const char* defaults;
};

const Action* PadActions(size_t* count) {
    if (count) *count = 0;
    return nullptr;
}

std::string PadBinding(size_t) {
    return "";
}

void PadBind(size_t, const std::string&) {}

void ResetPadBindings() {}

void LoadPadBindings() {}

std::string PressedPadInput() {
    return "";
}

}  // namespace input

namespace host {

void SamplePerf() {}

float CpuLoad() {
    return -1.0f;
}

float GpuLoad() {
    return -1.0f;
}

float CpuTemp() {
    return -1.0f;
}

float GpuTemp() {
    return -1.0f;
}

float BatteryTemp() {
    return -1.0f;
}

}  // namespace host
