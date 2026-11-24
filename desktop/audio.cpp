// mixer output through an SDL audio stream (AAudio underneath): the 5.1 mix folded to the stereo pair Android plays
#include <SDL3/SDL.h>

#include <algorithm>
#include <atomic>
#include <mutex>
#include <vector>

#include "audio/output.h"
#include "core/log.h"

namespace audio {

namespace {

constexpr float kCentreFold = 0.707f;
constexpr uint32_t kBlock = kRate / 100;  // frames the mixer renders a call: 10 ms

std::atomic<uint32_t> g_mode{kOutputAuto};
std::atomic<bool> g_paused{false};
SDL_AudioStream* g_stream = nullptr;

struct Pump {
    std::function<void(float*, uint32_t)> render;
    std::vector<float> mix = std::vector<float>(kBlock * kChannels);
    std::vector<float> out;  // stereo frames rendered, not yet handed to SDL
};

// L = FL + kCentreFold C + BL and R = FR + kCentreFold C + BR without the sub; mono hears the average of the two.
// Clamped to [-1.0f, 1.0f] to prevent integer wrap-around and DAC digital clipping on loud voice/sound transients.
void Fold(const float* in, uint32_t frames, bool mono, std::vector<float>& out) {
    for (uint32_t i = 0; i < frames; i++) {
        const float* f = in + size_t(i) * kChannels;
        float c = f[Centre] * kCentreFold;
        float l = f[FrontLeft] + c + f[BackLeft] * kCentreFold;
        float r = f[FrontRight] + c + f[BackRight] * kCentreFold;
        if (mono) l = r = (l + r) * 0.5f;
        out.push_back(std::clamp(l, -1.0f, 1.0f));
        out.push_back(std::clamp(r, -1.0f, 1.0f));
    }
}

// Target 60 ms queue cushion in SDL_AudioStream to completely absorb scheduling jitter and prevent buffer underruns
constexpr int kCushionFrames = (kRate * 6) / 100;                  // 60 ms = 2880 frames
constexpr int kCushionBytes = kCushionFrames * 2 * sizeof(float);  // 23,040 bytes

void SDLCALL Feed(void* user, SDL_AudioStream* stream, int additional, int) {
    auto& p = *static_cast<Pump*>(user);
    int queued = std::max(0, SDL_GetAudioStreamQueued(stream));
    int needed = std::max(0, additional) + std::max(0, kCushionBytes - queued);

    while (needed > 0) {
        p.render(p.mix.data(), kBlock);
        Fold(p.mix.data(), kBlock, g_mode == kOutputMono, p.out);
        int bytes = int(p.out.size() * sizeof(float));
        SDL_PutAudioStreamData(stream, p.out.data(), bytes);
        needed -= bytes;
        p.out.clear();
    }
}

}  // namespace

// a 5.1 mix only when the player asks for it: phones and tablets play stereo
bool IsSurround() { return g_mode == kOutputSurround; }

bool IsMono() { return g_mode == kOutputMono; }

void SetOutputMode(uint32_t mode) {
    if (g_mode.exchange(mode) != mode) LOGI("audio: output %u", mode);
}

void SetPaused(bool paused) {
    if (g_paused.exchange(paused) == paused || !g_stream) return;
    if (paused) SDL_PauseAudioStreamDevice(g_stream);
    else SDL_ResumeAudioStreamDevice(g_stream);
}

void StartOutput(std::function<void(float*, uint32_t)> render) {
    if (!SDL_WasInit(SDL_INIT_AUDIO) && !SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        LOGE("audio: SDL audio unavailable, no sound: %s", SDL_GetError());
        return;
    }
    SDL_AudioSpec spec{SDL_AUDIO_F32, 2, int(kRate)};
    auto* pump = new Pump{std::move(render)};
    g_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, Feed, pump);
    if (!g_stream) {
        LOGE("audio: no output device, no sound: %s", SDL_GetError());
        delete pump;
        return;
    }
    LOGI("audio: %s output through %s", IsSurround() ? "5.1 folded to stereo" : "stereo", SDL_GetCurrentAudioDriver());

    // Pre-fill stream with 60 ms cushion so audio starts smoothly without an initial underrun pop
    for (int i = 0; i < 6; i++) {
        pump->render(pump->mix.data(), kBlock);
        Fold(pump->mix.data(), kBlock, g_mode == kOutputMono, pump->out);
        SDL_PutAudioStreamData(g_stream, pump->out.data(), int(pump->out.size() * sizeof(float)));
        pump->out.clear();
    }

    if (!g_paused) SDL_ResumeAudioStreamDevice(g_stream);
}

}  // namespace audio
