#include <pthread.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <time.h>

#include "asteroidsObjects.h"
#include "asteroidsShared.h"
#include "asteroidsStructures.h"

int main(void) {
    // game window dimensions (if window resizes, playing dimensions stay same)
    const int windowWidth = 1024;
    const int windowHeight = 768;
    const char *windowName = "Testing window";
    int headlessRun = 0, windowInitialized = 0;
    //int exitState = 0;

    // game constants
    const float base_playerAcceleration = 500;

    // creating a dynamic arrays to store components
    DynArray *MotionComponents = newDynArray(10, sizeof(ComponentMotion));
    DynArray *RotationComponents = newDynArray(10, sizeof(ComponentRotation));
    DynArray *CollisionComponents = newDynArray(10, sizeof(ComponentCollision));
    DynArray *LifetimeComponents = newDynArray(10, sizeof(ComponentLifeTime));

    // allocating player components and adding them to the components array
    PlayerObject *player = (PlayerObject *)MemAlloc(sizeof(PlayerObject));
    player->movementID = dynArrayAdd(MotionComponents, MemAlloc(sizeof(ComponentMotion)));
    player->rotationID = dynArrayAdd(RotationComponents, MemAlloc(sizeof(ComponentRotation)));
    player->hitboxID = dynArrayAdd(CollisionComponents, MemAlloc(sizeof(ComponentCollision)));

    // initializing player components
    ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->position = (Vector2){windowWidth / 2, windowHeight / 2};
    ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->velocity = (Vector2){0, 0};
    ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->acceleration = (Vector2){0, 0};
    ((ComponentRotation *)dynArrayGet(RotationComponents, player->rotationID))->rotation = 0;
    ((ComponentRotation *)dynArrayGet(RotationComponents, player->rotationID))->rotationSpeed = 0;
    ((ComponentCollision *)dynArrayGet(CollisionComponents, player->hitboxID))->hitbox = (Vector2){50, 50};

    // creating game memory interfaces
    struct sharedInput_s *sharedInput = allocateSharedInput("sharedInput");
    lockSharedInput(sharedInput);
    initSharedInput(sharedInput);
    unlockSharedInput(sharedInput);
    struct sharedOutput_s *sharedOutput = allocateSharedOutput("sharedOutput");
    lockSharedOutput(sharedOutput);
    initSharedOutput(sharedOutput);
    unlockSharedOutput(sharedOutput);

    // update game interfaces
    lockSharedOutput(sharedOutput);
    sharedOutput->playerPosX = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->position.x;
    sharedOutput->playerPosY = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->position.y;
    sharedOutput->playerRotation = ((ComponentRotation *)dynArrayGet(RotationComponents, player->rotationID))->rotation;
    sharedOutput->playerSpeedX = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->velocity.x;
    sharedOutput->playerSpeedY = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->velocity.y;
    unlockSharedOutput(sharedOutput);

    // create window and unlock maximum framerate (which will be regulated by other means)
    if (!headlessRun) {
        InitWindow(windowWidth, windowHeight, windowName);
        SetTargetFPS(0);
        windowInitialized = 1;
    }

    // logic timing variables
    struct timespec currentTime;
    const double fixedTimeStep = 1.0 / 60.0;
    double accumulator = 0.0;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    double avgFrameTime = 0.0;

    // main game loop
    while (!IsKeyPressed(KEY_ESCAPE)) {
        // update timing
        struct timespec newTime;
        clock_gettime(CLOCK_MONOTONIC, &newTime);
        double frameTime = (newTime.tv_sec - currentTime.tv_sec) + (newTime.tv_nsec - currentTime.tv_nsec) / 1e9;
        if (frameTime > 0.25) {  // note: max frame time to avoid spiral of death
            frameTime = 0.25;
        }
        currentTime = newTime;
        accumulator += frameTime;

        // create FPS counter string
        char fpsText[20];
        avgFrameTime = avgFrameTime * 0.99 + frameTime * 0.01;
        sprintf(fpsText, "FPS: %.2f", 1.0 / avgFrameTime);

        // game logic loop
        while (accumulator >= fixedTimeStep) {
            // pre-rendering logic (load resources, update physics, etc.)

            // input-based component updates
            if (IsKeyDown(KEY_H)) headlessRun = 1;
            if (IsKeyDown(KEY_W)) {
                float rotation = ((ComponentRotation *)dynArrayGet(RotationComponents, player->rotationID))->rotation;
                Vector2 acceleration = Vector2Scale((Vector2){cosf(rotation), sinf(rotation)}, base_playerAcceleration);
                ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->acceleration = acceleration;
            } else {
                Vector2 acceleration = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->velocity;
                acceleration = Vector2Scale(acceleration, -0.01f / fixedTimeStep);
                ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->acceleration = acceleration;
            }
            if (IsKeyDown(KEY_A)) {
                ((ComponentRotation *)dynArrayGet(RotationComponents, player->rotationID))->rotationSpeed = -3.f;
            } else if (IsKeyDown(KEY_D)) {
                ((ComponentRotation *)dynArrayGet(RotationComponents, player->rotationID))->rotationSpeed = 3.f;
            } else
                ((ComponentRotation *)dynArrayGet(RotationComponents, player->rotationID))->rotationSpeed = 0.f;

            // headless mode update
            if (headlessRun && windowInitialized) {
                CloseWindow();
                windowInitialized = 0;
            } else if (!headlessRun && !windowInitialized) {
                InitWindow(windowWidth, windowHeight, windowName);
                SetTargetFPS(0);
                windowInitialized = 1;
            }

            // time-based component updates
            for (size_t i = 0; i < MotionComponents->size; i++) {
                ComponentMotion *motion = (ComponentMotion *)dynArrayGet(MotionComponents, i);

                // updating velocity and position
                motion->velocity = Vector2Add(motion->velocity, Vector2Scale(motion->acceleration, fixedTimeStep));
                motion->position = Vector2Add(motion->position, Vector2Scale(motion->velocity, fixedTimeStep));

                // position wrapping
                if (motion->position.x >= windowWidth)
                    motion->position.x -= windowWidth;
                else if (motion->position.x < 0)
                    motion->position.x += windowWidth;
                if (motion->position.y >= windowHeight)
                    motion->position.y -= windowHeight;
                else if (motion->position.y < 0)
                    motion->position.y += windowHeight;
            }
            for (size_t i = 0; i < RotationComponents->size; i++) {
                ComponentRotation *rotation = (ComponentRotation *)dynArrayGet(RotationComponents, i);

                // updating rotation
                rotation->rotation += rotation->rotationSpeed * fixedTimeStep;
            }
            for (size_t i = 0; i < LifetimeComponents->size; i++) {
                ComponentLifeTime *tmpLifetime = (ComponentLifeTime *)dynArrayGet(LifetimeComponents, i);
                if (tmpLifetime->isAlive) {
                    tmpLifetime->lifeTime -= fixedTimeStep;
                    if (tmpLifetime->lifeTime <= 0) {
                        tmpLifetime->isAlive = 0;
                    }
                }
            }

            // end game logic updates
            accumulator -= fixedTimeStep;
        }

        // update rendering
        if (windowInitialized) {
            BeginDrawing();
            ClearBackground(BLACK);

            // game objects rendering
            DrawPlayer((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID), (ComponentRotation *)dynArrayGet(RotationComponents, player->rotationID), (ComponentCollision *)dynArrayGet(CollisionComponents, player->hitboxID));

            // overlay rendering
            DrawRectangleLines(0, 0, windowWidth, windowHeight, WHITE);  // game border
            DrawText(fpsText, 10, 10, 20, WHITE);                        // FPS counter

            EndDrawing();
        }

        // post-rendering logic (free resources, etc.)

        // update shared outputs
        lockSharedOutput(sharedOutput);
        sharedOutput->playerPosX = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->position.x;
        sharedOutput->playerPosY = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->position.y;
        sharedOutput->playerRotation = ((ComponentRotation *)dynArrayGet(RotationComponents, player->rotationID))->rotation;
        sharedOutput->playerSpeedX = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->velocity.x;
        sharedOutput->playerSpeedY = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->velocity.y;
        unlockSharedOutput(sharedOutput);
    }

    if (windowInitialized) {
        CloseWindow();
        windowInitialized = 0;
    }

    // close shared memory
    destroySharedInput(sharedInput);
    destroySharedOutput(sharedOutput);

    // free all allocated memory
    MemFree(player);
    dynArrayForeach(MotionComponents, MemFree);
    dynArrayForeach(RotationComponents, MemFree);
    dynArrayForeach(CollisionComponents, MemFree);
    dynArrayForeach(LifetimeComponents, MemFree);
    destroyDynArray(MotionComponents);
    destroyDynArray(RotationComponents);
    destroyDynArray(CollisionComponents);
    destroyDynArray(LifetimeComponents);

    // exit program
    return 0;
}
