#include <stdio.h>
#include <raylib.h>
#include <complex.h>
#include <math.h>
#include <stdlib.h>


int main(void) {
    InitWindow(800, 800, "BeatBox");
    SetTargetFPS(60);
    
    InitAudioDevice();
    Music sound = LoadMusicStream("res/crash.mp3");
    PlayMusicStream(sound);
    
    // Main loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RED);
        UpdateMusicStream(sound);
        EndDrawing();
    }
    
    CloseWindow();

    return 0;
}

