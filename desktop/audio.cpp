// mixer output through an SDL audio stream: decoupled worker thread feeding 5.1 folded to stereo
#include <SDL3/SDL.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

#include "audio/output.h"
#include "core/log.h"

namespace audio {

namespace {

constexpr float kCentreFold = 0.707f;
constexpr float kDownmixScale = 0.75f;    // headroom scaling to prevent flat-top clipping distortion
constexpr uint32_t kBlock = kRate / 100;  // frames the mixer renders a call: 10 ms (480 frames)
// 60 ms buffer cushion (2880 frames = 23,040 bytes)
constexpr int kTargetQueuedBytes = (int(kRate) * 6 / 100) * 2 * int(sizeof(float));

std::atomic<uint32_t> g_mode{kOutputAuto};
std::atomic<bool> g_paused{false};
std::atomic<bool> g_running{false};
SDL_AudioStream* g_stream = nullptr;
std::thread g_audio_thread;

// L = (FL + 0.707 C + 0.707 BL) * scale and R = (FR + 0.707 C + 0.707 BR) * scale.
// Clamped to [-1.0f, 1.0f] to prevent integer wrap-around and DAC digital clipping on loud voice/sound transients.
void Fold(const float* in, uint32_t frames, bool mono, std::vector<float>& out) {
    for (uint32_t i = 0; i < frames; i++) {
        const float* f = in + size_t(i) * kChannels;
        float c = f[Centre] * kCentreFold;
        float l = (f[FrontLeft] + c + f[BackLeft] * kCentreFold) * kDownmixScale;
        float r = (f[FrontRight] + c + f[BackRight] * kCentreFold) * kDownmixScale;
        if (mono) l = r = (l + r) * 0.5f;
        out.push_back(std::clamp(l, -1.0f, 1.0f));
        out.push_back(std::clamp(r, -1.0f, 1.0f));
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
    SDL_SetHint(SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES, "1024");
    SDL_AudioSpec spec{SDL_AUDIO_F32, 2, int(kRate)};
    // Open stream WITHOUT callback: we feed it from our own dedicated thread, decoupling
    // DirectSound mixing and g_lock contention from OS real-time audio driver deadlines.
    g_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
    if (!g_stream) {
        LOGE("audio: no output device, no sound: %s", SDL_GetError());
        return;
    }
    LOGI("audio: %s output through %s (decoupled worker thread, 60ms cushion)",
         IsSurround() ? "5.1 folded to stereo" : "stereo", SDL_GetCurrentAudioDriver());

    g_running = true;
    g_audio_thread = std::thread([render = std::move(render)] {
        std::vector<float> mix(kBlock * kChannels);
        std::vector<float> out;
        out.reserve(kBlock * 2);

        // Pre-fill stream with 60 ms cushion so audio starts smoothly without an initial underrun pop
        for (int i = 0; i < 6; i++) {
            render(mix.data(), kBlock);
            Fold(mix.data(), kBlock, g_mode == kOutputMono, out);
            SDL_PutAudioStreamData(g_stream, out.data(), int(out.size() * sizeof(float)));
            out.clear();
        }

        if (!g_paused) SDL_ResumeAudioStreamDevice(g_stream);

        while (g_running) {
            if (g_paused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            int queued = SDL_GetAudioStreamQueued(g_stream);
            if (queued >= kTargetQueuedBytes) {
                std::this_thread::sleep_for(std::chrono::milliseconds(3));
                continue;
            }

            render(mix.data(), kBlock);
            Fold(mix.data(), kBlock, g_mode == kOutputMono, out);
            SDL_PutAudioStreamData(g_stream, out.data(), int(out.size() * sizeof(float)));
            out.clear();
        }
    });
    g_audio_thread.detach();
}

}  // namespace audio
