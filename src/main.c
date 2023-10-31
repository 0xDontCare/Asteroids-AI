#include <pthread.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <sys/mman.h>

typedef struct physicsObject_s {
    Vector2 position;
    float rotation;
    Vector2 speed;
    Rectangle hitbox;
} PhysicsObject;

typedef struct player_s {
    PhysicsObject physicsComponent;
    float acceleration;
} PlayerObject;

void DrawPlayer(PlayerObject *player) {
    DrawRectanglePro(player->physicsComponent.hitbox, (Vector2){player->physicsComponent.hitbox.width / 2, player->physicsComponent.hitbox.height / 2}, player->physicsComponent.rotation * RAD2DEG, (Color){255, 0, 0, 255});
}

int main(void) {
    const int windowWidth = 1024;
    const int windowHeight = 768;

    PlayerObject *player = (PlayerObject *)MemAlloc(sizeof(PlayerObject));
    player->physicsComponent.position = (Vector2){windowWidth / 2, windowHeight / 2};
    player->physicsComponent.rotation = 0;
    player->physicsComponent.speed = (Vector2){0, 0};
    player->physicsComponent.hitbox = (Rectangle){player->physicsComponent.position.x, player->physicsComponent.position.y, 50, 50};
    player->acceleration = 500;

    // create window and unlock maximum framerate
    InitWindow(windowWidth, windowHeight, "Testing window");
    SetTargetFPS(0);

    // logic timing variables
    const double fixedTimeStep = 1.0 / 60.0;
    double accumulator = 0.0;
    double currentTime = GetTime();
    double avgFrameTime = 0.0;
    while (!WindowShouldClose()) {
        // update timing
        double newTime = GetTime();
        double frameTime = newTime - currentTime;
        if (frameTime > 0.25) {  // note: max frame time to avoid spiral of death
            frameTime = 0.25;
        }
        currentTime = newTime;
        accumulator += frameTime;

        char fpsText[20];
        avgFrameTime = avgFrameTime * 0.999 + frameTime * 0.001;
        sprintf(fpsText, "FPS: %.2f", 1.0 / avgFrameTime);
        while (accumulator >= fixedTimeStep) {
            // game logic updates here

            if (IsKeyDown(KEY_W)) {
                Vector2 deltaV = (Vector2){player->acceleration * cosf(player->physicsComponent.rotation) * fixedTimeStep, player->acceleration * sinf(player->physicsComponent.rotation) * fixedTimeStep};
                player->physicsComponent.speed = Vector2Add(player->physicsComponent.speed, deltaV);
            } else {
                player->physicsComponent.speed = Vector2Scale(player->physicsComponent.speed, 0.99);
                //if(Vector2Length(player->physicsComponent.speed) < 0.1f) player->physicsComponent.speed = Vector2Zero();
            }
            if (IsKeyDown(KEY_A)) {
                player->physicsComponent.rotation -= 0.1;
            }
            if (IsKeyDown(KEY_D)) {
                player->physicsComponent.rotation += 0.1;
            }
            player->physicsComponent.position = Vector2Add(player->physicsComponent.position, Vector2Scale(player->physicsComponent.speed, fixedTimeStep));
            player->physicsComponent.hitbox.x = player->physicsComponent.position.x;
            player->physicsComponent.hitbox.y = player->physicsComponent.position.y;

            if (player->physicsComponent.position.x < 0) {
                player->physicsComponent.position.x = windowWidth;
            } else if (player->physicsComponent.position.x > windowWidth) {
                player->physicsComponent.position.x = 0;
            }
            if (player->physicsComponent.position.y < 0) {
                player->physicsComponent.position.y = windowHeight;
            } else if (player->physicsComponent.position.y > windowHeight) {
                player->physicsComponent.position.y = 0;
            }

            // end game logic updates

            accumulator -= fixedTimeStep;
        }

        // update rendering
        BeginDrawing();
        ClearBackground(BLACK);

        DrawRectangleLines(0, 0, windowWidth, windowHeight, WHITE);
        DrawPlayer(player);
        DrawText(fpsText, 10, 10, 20, WHITE);

        EndDrawing();

        // post-rendering logic (free resources, etc.)
    }

    CloseWindow();
    MemFree(player);
    return 0;
}