#include <stdio.h>
#include <raylib.h>


int main(void) {
    const int win_width = 800;
    const int win_height = 600;

    InitWindow(win_width, win_height, "BeatBox");

    InitAudioDevice();
    Sound sound = LoadSound("res/crash.mp3");
    PlaySound(sound);

    SetSoundVolume(sound, 0.1f);
    

    SetTargetFPS(60);
    
    // Main loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
                        
        DrawText("Hello, Raylib!", 10, 10, 20, DARKGRAY);
        EndDrawing();
    }
    
    CloseWindow();

    return 0;
}

