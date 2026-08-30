#ifndef ANDROIDGLINVESTIGATIONS_AUDIOENGINE_H
#define ANDROIDGLINVESTIGATIONS_AUDIOENGINE_H

#include <aaudio/AAudio.h>
#include <cmath>
#include <vector>
#include <mutex>
#include <algorithm>
#include "AndroidOut.h"

enum SoundType {
    SOUND_TAP_BLUE,
    SOUND_TAP_RED,
    SOUND_TAP_GREEN,
    SOUND_TAP_YELLOW,
    SOUND_TAP_WHITE,
    SOUND_BEEP,
    SOUND_GO
};

struct ActiveSound {
    SoundType type;
    float time;        // current time in seconds
    float duration;    // total duration in seconds
    float startFreq;   // starting frequency in Hz
    float endFreq;     // ending frequency in Hz
    float phase;       // sine phase accumulator
    bool active;
};

class AudioEngine {
public:
    static constexpr int SAMPLE_RATE = 44100;
    static constexpr int CHANNEL_COUNT = 2; // Stereo

    AudioEngine() : stream_(nullptr), isPlaying_(false) {
        initAAudio();
    }

    ~AudioEngine() {
        stopAAudio();
    }

    void playTapSound(int state) {
        std::lock_guard<std::mutex> lock(audioMutex_);
        ActiveSound s;
        s.time = 0.0f;
        s.duration = 0.08f;
        s.phase = 0.0f;
        s.active = true;

        if (state == 1) { // Blue
            s.type = SOUND_TAP_BLUE;
            s.startFreq = 650.0f;
            s.endFreq = 280.0f;
        } else if (state == 2) { // Red
            s.type = SOUND_TAP_RED;
            s.startFreq = 850.0f;
            s.endFreq = 380.0f;
        } else if (state == 3) { // Green (Future ready)
            s.type = SOUND_TAP_GREEN;
            s.startFreq = 1050.0f;
            s.endFreq = 480.0f;
        } else if (state == 4) { // Yellow (Future ready)
            s.type = SOUND_TAP_YELLOW;
            s.startFreq = 1250.0f;
            s.endFreq = 620.0f;
        } else { // White (0)
            s.type = SOUND_TAP_WHITE;
            s.startFreq = 480.0f;
            s.endFreq = 200.0f;
        }

        sounds_.push_back(s);
    }

    void playCountdownBeep() {
        std::lock_guard<std::mutex> lock(audioMutex_);
        ActiveSound s;
        s.type = SOUND_BEEP;
        s.time = 0.0f;
        s.duration = 0.12f;
        s.startFreq = 523.25f; // C5
        s.endFreq = 523.25f;
        s.phase = 0.0f;
        s.active = true;
        sounds_.push_back(s);
    }

    void playGoChime() {
        std::lock_guard<std::mutex> lock(audioMutex_);
        ActiveSound s1;
        s1.type = SOUND_GO;
        s1.time = 0.0f;
        s1.duration = 0.24f;
        s1.startFreq = 783.99f; // G5
        s1.endFreq = 1046.50f; // C6
        s1.phase = 0.0f;
        s1.active = true;
        sounds_.push_back(s1);

        ActiveSound s2;
        s2.type = SOUND_GO;
        s2.time = 0.04f;
        s2.duration = 0.24f;
        s2.startFreq = 1046.50f; // C6
        s2.endFreq = 1318.51f; // E6
        s2.phase = 0.0f;
        s2.active = true;
        sounds_.push_back(s2);
    }

private:
    void initAAudio() {
        AAudioStreamBuilder* builder = nullptr;
        aaudio_result_t result = AAudio_createStreamBuilder(&builder);
        if (result != AAUDIO_OK) {
            aout << "Failed to create AAudioStreamBuilder" << std::endl;
            return;
        }

        AAudioStreamBuilder_setSampleRate(builder, SAMPLE_RATE);
        AAudioStreamBuilder_setChannelCount(builder, CHANNEL_COUNT);
        AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_FLOAT);
        AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
        AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_SHARED);
        AAudioStreamBuilder_setDataCallback(builder, dataCallback, this);

        result = AAudioStreamBuilder_openStream(builder, &stream_);
        AAudioStreamBuilder_delete(builder);

        if (result == AAUDIO_OK && stream_ != nullptr) {
            result = AAudioStream_requestStart(stream_);
            if (result == AAUDIO_OK) {
                isPlaying_ = true;
                aout << "AAudio stream started successfully" << std::endl;
            }
        }
    }

    void stopAAudio() {
        if (stream_ != nullptr) {
            AAudioStream_requestStop(stream_);
            AAudioStream_close(stream_);
            stream_ = nullptr;
            isPlaying_ = false;
        }
    }

    static aaudio_data_callback_result_t dataCallback(
            AAudioStream* stream,
            void* userData,
            void* audioData,
            int32_t numFrames) {

        auto* engine = static_cast<AudioEngine*>(userData);
        return engine->renderAudio(static_cast<float*>(audioData), numFrames);
    }

    aaudio_data_callback_result_t renderAudio(float* output, int32_t numFrames) {
        std::fill(output, output + numFrames * CHANNEL_COUNT, 0.0f);

        std::lock_guard<std::mutex> lock(audioMutex_);
        float dt = 1.0f / static_cast<float>(SAMPLE_RATE);

        for (auto& s : sounds_) {
            if (!s.active) continue;

            for (int i = 0; i < numFrames; ++i) {
                if (s.time >= s.duration) {
                    s.active = false;
                    break;
                }

                float tRatio = s.time / s.duration;
                float currentFreq = s.startFreq + tRatio * (s.endFreq - s.startFreq);

                float envelope = std::pow(1.0f - tRatio, 2.0f);
                if (s.type == SOUND_BEEP || s.type == SOUND_GO) {
                    envelope = std::pow(1.0f - tRatio, 1.5f);
                }

                float sample = std::sin(s.phase) * envelope * 0.35f;

                s.phase += 2.0f * 3.14159265f * currentFreq * dt;
                if (s.phase > 2.0f * 3.14159265f) s.phase -= 2.0f * 3.14159265f;

                output[i * 2 + 0] += sample;
                output[i * 2 + 1] += sample;

                s.time += dt;
            }
        }

        sounds_.erase(
            std::remove_if(sounds_.begin(), sounds_.end(),
                           [](const ActiveSound& s) { return !s.active; }),
            sounds_.end());

        return AAUDIO_CALLBACK_RESULT_CONTINUE;
    }

    AAudioStream* stream_;
    bool isPlaying_;
    std::mutex audioMutex_;
    std::vector<ActiveSound> sounds_;
};

#endif // ANDROIDGLINVESTIGATIONS_AUDIOENGINE_H