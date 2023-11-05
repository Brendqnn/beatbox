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


static void fft(complex double* x, int N) {
    if (N <= 1) return;

    complex double even[N / 2];
    complex double odd[N / 2];

    for (int i = 0; i < N / 2; i++) {
        even[i] = x[i * 2];
        odd[i] = x[i * 2 + 1];
    }

    fft(even, N / 2);
    fft(odd, N / 2);

    for (int k = 0; k < N / 2; k++) {
        complex double T = cexp(-2.0 * I * M_PI * k / N) * odd[k];
        x[k] = even[k] + T;
        x[k + N / 2] = even[k] - T;
    }
}

void callback(void *buffer_data, unsigned int frames)
{
    if (frames > ARRAY_LEN(global_frames)) {
        frames = ARRAY_LEN(global_frames);
    }

    complex double complex_data[frames];
    for (size_t i = 0; i < frames; i++) {
        float sample = ((float *)buffer_data)[i];       
        complex_data[i] = sample;
    }
    
    // Apply the Hann window (Hanning window) - https://en.wikipedia.org/wiki/Hann_function
    for (size_t i = 0; i < frames; i++) {
        float t = (float)i/(frames - 1);
        float hann = 0.5 - 0.5*cosf(2*PI*t);
        complex_data[i] *= hann;
    }

    // Perform FFT
    fft(complex_data, frames);

    for (size_t i = 0; i < frames; i++) {
        global_frames[i] = creal(complex_data[i]);
    }
    
    global_frames_count = frames;
}

int main(void) {
    InitWindow(1200, 800, "beatbox");
    SetTargetFPS(60);
    
    InitAudioDevice();
    Music sound = LoadMusicStream("MASTER PAKKU.mp3");
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

        float x1 = 0;
        float y1, y2;
        
        BeginDrawing();
        ClearBackground(CLITERAL(Color) {0x18, 0x18, 0x18, 0xFF});
        float cell_width = (float)w/global_frames_count;
        for (size_t i = 0; i < global_frames_count; ++i) {
            complex double data = global_frames[i];
            y1 = h / 2 - h / 2 * creal(data);

            if (i > 0) {
                DrawLine(x1, y1, x1 + cell_width, y2, RED);
            }
            x1 += cell_width;
            y2 = y1;
        }
        EndDrawing();
    }
    CloseWindow();

    return 0;
}
