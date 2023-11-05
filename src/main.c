#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <complex.h>
#include <math.h>

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])

uint64_t global_frames[2048] = {0};
size_t global_frames_count = 0;
 
static void fft(float in[], size_t stride, float complex out[], size_t n)
{
    assert(n > 0);

    if (n == 1) {
        out[0] = in[0];
        return;
    }

    fft(in, stride * 2, out, n / 2);
    fft(in + stride, stride * 2, out + n / 2, n / 2);

    for (size_t k = 0; k < n / 2; ++k) {
        float t = (float)k / n;
        float complex v = cexpf(-2.0 * I * PI * t) * out[k + n / 2];
        float complex e = out[k];
        out[k] = e + v;
        out[k + n / 2] = e - v;
    }
}

void callback(void *buffer_data, unsigned int frames)
{
    if (frames > ARRAY_LEN(global_frames)) {
        frames = ARRAY_LEN(global_frames);
    }

    float audio_data[frames];
    for (size_t i = 0; i < frames; i++) {
        audio_data[i] = ((float *)buffer_data)[i];
    }

    // Apply the Hann window (Hanning window) - https://en.wikipedia.org/wiki/Hann_function
    for (size_t i = 0; i < frames; i++) {
        float t = (float)i / (frames - 1);
        float hann = 0.5 - 0.5 * cosf(2 * PI * t);
        audio_data[i] *= hann;
    }

    float complex complex_data[frames];
    fft(audio_data, 1, complex_data, frames);

    for (size_t i = 0; i < frames; i++) {
        global_frames[i] = cabs(complex_data[i]);
    }

    global_frames_count = frames;
}

int main(void) {
    InitWindow(1200, 800, "beatbox");
    SetTargetFPS(60);

    InitAudioDevice();
    Music sound = LoadMusicStream("Beatiful Love.mp3");
    assert(sound.stream.sampleSize == 32);
    assert(sound.stream.channels == 2);

    PlayMusicStream(sound);
    SetMusicVolume(sound, 1.5f);
    AttachAudioStreamProcessor(sound.stream, callback);

    while (!WindowShouldClose()) {
        UpdateMusicStream(sound);
        if (IsKeyPressed(KEY_SPACE)) {
            if (IsMusicStreamPlaying(sound)) {
                PauseMusicStream(sound);
            } else {
                ResumeMusicStream(sound);
            }
        }

        int w = GetRenderWidth();
        int h = GetRenderHeight();

        float height_scale = 10.0f;

        BeginDrawing();
        ClearBackground(CLITERAL(Color) {0x18, 0x18, 0x18, 0xFF});
        float cell_width = (float)w/global_frames_count;
        float cell_height = (float)h/global_frames_count;
        for (size_t i = 0; i < global_frames_count; ++i) {
            complex double data = global_frames[i];
            double amp = cabs(data);
            float magnitude = cabs(data); // Get the magnitude of the complex value
            float x = i * cell_width;
            float y = h - amp * height_scale;
            DrawRectangle(x, y, cell_width, amp * height_scale, RED);
        }
        EndDrawing();
    }
    CloseWindow();

    return 0;
}

