#include <pthread.h>
#include <raylib.h>
#include <stdio.h>
#include <sys/mman.h>

int main(void) {
    const int windowWidth = 1024;
    const int windowHeight = 768;

    InitWindow(windowWidth, windowHeight, "Hello window!");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BLACK);

        DrawText("Hello, world!", 10, 10, 20, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}