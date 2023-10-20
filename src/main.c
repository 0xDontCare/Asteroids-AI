#include <pthread.h>
#include <raylib.h>
#include <stdio.h>
#include <sys/mman.h>

int main(void) {
    const int windowWidth = 1024;
    const int windowHeight = 768;
    Vector2 movingSpeed = (Vector2){200, 200};
    Rectangle testingBox = (Rectangle){20, 20, 100, 100};

    // create window and unlock maximum framerate
    InitWindow(windowWidth, windowHeight, "Testing window");
    SetTargetFPS(0);

    // logic timing variables
    const double fixedTimeStep = 1.0 / 60.0;
    double accumulator = 0.0;
    double currentTime = GetTime();

    while (!WindowShouldClose()) {
        // update timing
        double newTime = GetTime();
        double frameTime = newTime - currentTime;
        if (frameTime > 0.25) { // note: max frame time to avoid spiral of death
            frameTime = 0.25;
        }
        currentTime = newTime;
        accumulator += frameTime;

        char fpsText[20];
        sprintf(fpsText, "FPS: %f", 1.0 / frameTime);
        while (accumulator >= fixedTimeStep) {
            // game logic updates here
            if (testingBox.x + testingBox.width >= windowWidth || testingBox.x <= 0) {
                movingSpeed.x *= -1;
            } else if (testingBox.y + testingBox.height >= windowHeight || testingBox.y <= 0) {
                movingSpeed.y *= -1;
            }
            testingBox.x += movingSpeed.x * fixedTimeStep;
            testingBox.y += movingSpeed.y * fixedTimeStep;

            accumulator -= fixedTimeStep;
        }

        // update rendering
        BeginDrawing();
        ClearBackground(BLACK);

        DrawRectangleLines(0, 0, windowWidth, windowHeight, WHITE);
        DrawRectangleRec(testingBox, RED);
        DrawText(fpsText, 10, 10, 20, WHITE);

        EndDrawing();

        // post-rendering logic (free resources, etc.)

    }

    CloseWindow();
    return 0;
}