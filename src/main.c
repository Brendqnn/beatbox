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

void fft(complex double* x, int N) {
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

    // Convert the audio data to complex double values
    complex double complex_data[frames];
    for (size_t i = 0; i < frames; i++) {
        // Assuming 32-bit float audio samples
        float sample = ((float *)buffer_data)[i];
        complex_data[i] = sample;
    }
    
    fft(complex_data, frames);

    for (size_t i = 0; i < frames; i++) {
        global_frames[i] = cabs(complex_data[i]);
    }
    
    global_frames_count = frames;
}

int main(void) {
    InitWindow(1200, 800, "beatbox");
    SetTargetFPS(60);
    
    InitAudioDevice();
    Music sound = LoadMusicStream("Max B.mp3");
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
        
        BeginDrawing();
        ClearBackground(CLITERAL(Color) {0x18, 0x18, 0x18, 0xFF});
        float cell_width = (float)w/global_frames_count;
        for (size_t i = 0; i < global_frames_count; ++i) {
            complex double data = global_frames[i];
            float magnitude = cabs(data); // Get the magnitude of the complex value

            // Visualize the magnitude, not as a 32-bit integer
            float x = i * cell_width;
            float height = h * magnitude;

            DrawRectangle(x, h - height, cell_width, height, RED);
        }
        EndDrawing();
    }
    CloseWindow();

    return 0;
}

