#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(xs[0])

uint64_t global_frames[4080] = {0};
size_t global_frames_count = 0;

void callback(void *buffer_data, unsigned int frames)
{
    if (frames > ARRAY_LEN(global_frames)) {
        frames = ARRAY_LEN(global_frames);
    }
    memcpy(global_frames, buffer_data, sizeof(uint64_t)*frames);
    global_frames_count = frames;
}

int main(void) {
    InitWindow(800, 600, "BeatBox");
    SetTargetFPS(60);
    
    InitAudioDevice();
    Music sound = LoadMusicStream("res/crash.mp3");
    assert(sound.stream.sampleSize == 32);
    assert(sound.stream.channels == 2);
        
    PlayMusicStream(sound);
    SetMusicVolume(sound, 0.5f);
    AttachAudioStreamProcessor(sound.stream, callback);
    
    // Main loop
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
            int32_t sample = *(int32_t*)&global_frames[i];
                float t = (float)sample / INT32_MAX; // Scale to [0, 1] for positive values
            if (sample < 0) {
                t = (float)sample / INT32_MIN;   // Scale to [0, 1] for negative values 
            } 
            float y = h/2 - h/2 * t;
            DrawRectangle(i * cell_width, y, cell_width, h / 2 * t, RED);
        }
        
        
        EndDrawing();
    }
    
    CloseWindow();

    return 0;
}

