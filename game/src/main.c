/**
 * @file programGame.c
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Main game program. Depending on how it is started, it can take input from keyboard or from external programs in shared memory.
 * @version 0.2
 * @date 11.12.2023.
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "main.h"  // main program header

#include <raylib.h>   // graphics library
#include <raymath.h>  // math library
#include <stdio.h>    // standard input/output library
#include <stdlib.h>   // standard library
#include <time.h>     // time library

#include "commonUtility.h"  // smaller common utility functions which don't have much to do with the game itself
#include "ecsObjects.h"     // game entities and components
#include "sharedMemory.h"   // shared memory interfaces and functions
#include "xArray.h"         // dynamic array library
#include "xString.h"        // string library

// constant game globals
const int windowWidth = 1024;
const int windowHeight = 768;
const char *windowName = "Asteroids";

// game entry point
int main(int argc, char *argv[]) {
    /* Command line flags:
     * 0x01 - help
     * 0x02 - version
     * 0x04 - standalone mode
     * 0x08 - headless mode
     * 0x10 - use neural network (+2 parameters)
     * 0x20 - managed mode (+3 parameters)
     */
    unsigned short flags_cmd = CMD_FLAG_NONE;
    char *cmd_shInputName = NULL, *cmd_shOutputName = NULL, *cmd_shStateName = NULL;  // shared memory access names

    // runtime flags (game state, window state, etc.)
    unsigned short flags_runtime = RUNTIME_NONE;

    // parsing command line arguments
    if (argc > 1) {
        int i;
        for (i = 1; i < argc; i++) {
            xString *tmpString = xString_fromCString(argv[i]);
            if (xString_isEqualCString(tmpString, "-h") || xString_isEqualCString(tmpString, "--help")) {
                flags_cmd |= CMD_FLAG_HELP;
            } else if (xString_isEqualCString(tmpString, "-v") || xString_isEqualCString(tmpString, "--version")) {
                flags_cmd |= CMD_FLAG_VERSION;
            } else if (xString_isEqualCString(tmpString, "-s") || xString_isEqualCString(tmpString, "--standalone")) {
                flags_cmd |= CMD_FLAG_STANDALONE;
            } else if (xString_isEqualCString(tmpString, "-H") || xString_isEqualCString(tmpString, "--headless")) {
                flags_cmd |= CMD_FLAG_HEADLESS;
            } else if (xString_isEqualCString(tmpString, "-n") || xString_isEqualCString(tmpString, "--neural")) {
                if (i + 2 >= argc) break;

                flags_cmd |= CMD_FLAG_USE_NEURAL;
                cmd_shInputName = argv[i + 1];
                cmd_shOutputName = argv[i + 2];
                i += 2;
            } else if (xString_isEqualCString(tmpString, "-m") || xString_isEqualCString(tmpString, "--managed")) {
                if (i + 3 >= argc) break;

                flags_cmd |= CMD_FLAG_MANAGED;
                cmd_shInputName = argv[i + 1];
                cmd_shOutputName = argv[i + 2];
                cmd_shStateName = argv[i + 3];
                i += 3;
            } else {
                printf("ERROR: Unknown command line argument: %s\n", argv[i]);
                printf("Use %s --help for more information.\n", argv[0]);
                xString_free(tmpString);
                return 1;
            }
            xString_free(tmpString);
        }
        if (i != argc) {
            printf("ERROR: Invalid command line arguments.\n");
            printf("Use %s --help for more information.\n", argv[0]);
            return 1;
        }
    } else {
        // assuming basic standalone mode by default
        flags_cmd |= CMD_FLAG_STANDALONE;
    }

    // check for flag conflicts
    if ((flags_cmd & CMD_FLAG_STANDALONE && flags_cmd & CMD_FLAG_HEADLESS) ||  // standalone and headless mode cannot be used at the same time
        (flags_cmd & CMD_FLAG_STANDALONE && flags_cmd & CMD_FLAG_MANAGED) ||   // standalone and managed mode cannot be used at the same time
        (flags_cmd & CMD_FLAG_HELP && flags_cmd & ~CMD_FLAG_HELP) ||           // help flag is exclusive to all other flags
        (flags_cmd & CMD_FLAG_VERSION && flags_cmd & ~CMD_FLAG_VERSION) ||     // version flag is exclusive to all other flags
        (flags_cmd & CMD_FLAG_HEADLESS && !(flags_cmd & CMD_FLAG_MANAGED)) ||  // headless mode requires managed mode
        (flags_cmd & CMD_FLAG_USE_NEURAL && flags_cmd & CMD_FLAG_MANAGED)) {   // neural network mode and managed mode cannot be used at the same time (managed mode already implies neural network mode)
        printf("ERROR: Invalid command line arguments.\n");
        printf("Use %s --help for more information.\n", argv[0]);
        return 1;
    }

    // parse set command flags
    if (flags_cmd & CMD_FLAG_HELP) {
        printf("Usage: %s [OPTIONS]\n", argv[0]);
        printf("Options:\n");
        printf("  -h, --help\t\t\t\t\tPrint this help message and exit.\n");
        printf("  -v, --version\t\t\t\t\tPrint version information and exit.\n");
        printf("  -s, --standalone\t\t\t\tRun game in standalone mode (no external manager program).\n");
        printf("  -H, --headless\t\t\t\tRun game in headless mode (no window). Use together with --managed\n");
        printf("  -n, --neural <input> <output>\t\t\tRun game in neural network mode (input and output shared memory names).\n");
        printf("  -m, --managed <input> <output> <state>\tRun game in managed mode (input, output and state shared memory names).\n");
        return 0;
    } else if (flags_cmd & CMD_FLAG_VERSION) {
        printf("Program:\t\tAsteroids-game\n");
        printf("Version:\t\tDEV (P1.1)\n");
        printf("Compiler version:\t%s\n", __VERSION__);
        printf("Raylib version:\t\t%s\n", RAYLIB_VERSION);
        printf("Compiled on %s at %s\n", __DATE__, __TIME__);
        return 0;
    }

    // flag arguments (shared memory names) should be only alphanumeric strings
    if (flags_cmd & (CMD_FLAG_MANAGED | CMD_FLAG_USE_NEURAL)) {
        if (!cu_CStringIsAlphanumeric(cmd_shInputName) || !cu_CStringIsAlphanumeric(cmd_shOutputName) || !cu_CStringIsAlphanumeric(cmd_shStateName)) {
            printf("ERROR: Shared memory names can only contain alphanumeric characters.\n");
            return 1;
        }
    }
    // TODO: apply new command line arguments to the rest of the program

    // game input flags
    unsigned short flags_input = INPUT_NONE;

    // game constants
    const float base_playerAcceleration = 500.f;

    // entity-component arrays
    xArray *motionComponents = xArray_new();
    xArray *rotationComponents = xArray_new();
    xArray *rectComponents = xArray_new();
    xArray *circleComponents = xArray_new();
    // xArray *lifetimeComponents = xArray_new();

    // allocating player components and adding them to arrays
    PlayerObject *player = (PlayerObject *)malloc(sizeof(PlayerObject));
    xArray_push(motionComponents, malloc(sizeof(ComponentMotion)));
    xArray_push(rotationComponents, malloc(sizeof(ComponentRotation)));
    xArray_push(rectComponents, malloc(sizeof(ComponentCollisionRect)));
    player->movementID = motionComponents->size - 1;
    player->rotationID = rotationComponents->size - 1;
    player->hitboxID = rectComponents->size - 1;

    // allocating asteroid components and adding them to the components array
    xArray *asteroidArray = xArray_new();
    for (size_t i = 0; i < 10; i++) {
        AsteroidObject *tmpAsteroid = (AsteroidObject *)malloc(sizeof(AsteroidObject));
        ComponentMotion *astMotion = (ComponentMotion *)malloc(sizeof(ComponentMotion));
        ComponentRotation *astRotation = (ComponentRotation *)malloc(sizeof(ComponentRotation));
        ComponentCollisionCircle *astCollision = (ComponentCollisionCircle *)malloc(sizeof(ComponentCollisionCircle));

        if (tmpAsteroid == NULL || astMotion == NULL || astRotation == NULL || astCollision == NULL) {
            free(tmpAsteroid);
            free(astMotion);
            free(astRotation);
            free(astCollision);
            break;
        }

        astMotion->position = (Vector2){GetRandomValue(0, windowWidth), GetRandomValue(0, windowHeight)};
        astMotion->velocity = (Vector2){GetRandomValue(-100, 100), GetRandomValue(-100, 100)};
        astMotion->acceleration = (Vector2){0, 0};
        astRotation->rotation = GetRandomValue(-3, 3);
        astRotation->rotationSpeed = GetRandomValue(-10, 10);
        astCollision->radius = GetRandomValue(1, 3) * 10.f;

        xArray_push(motionComponents, astMotion);
        xArray_push(rotationComponents, astRotation);
        xArray_push(circleComponents, astCollision);
        tmpAsteroid->movementID = motionComponents->size - 1;
        tmpAsteroid->rotationID = rotationComponents->size - 1;
        tmpAsteroid->hitboxID = circleComponents->size - 1;
        xArray_push(asteroidArray, tmpAsteroid);
    }

    // initializing player components
    ((ComponentMotion *)xArray_get(motionComponents, player->movementID))->position = (Vector2){windowWidth / 2, windowHeight / 2};
    ((ComponentMotion *)xArray_get(motionComponents, player->movementID))->velocity = (Vector2){0, 0};
    ((ComponentMotion *)xArray_get(motionComponents, player->movementID))->acceleration = (Vector2){0, 0};
    ((ComponentRotation *)xArray_get(rotationComponents, player->rotationID))->rotation = 0;
    ((ComponentRotation *)xArray_get(rotationComponents, player->rotationID))->rotationSpeed = 0;
    ((ComponentCollisionRect *)xArray_get(rectComponents, player->hitboxID))->hitbox = (Vector2){50, 50};

    // TODO: depending on run mode, allocate, connect to or skip creating shared memory
    struct sharedInput_s *sharedInput = NULL;
    struct sharedOutput_s *sharedOutput = NULL;
    struct sharedState_s *sharedState = NULL;

    // if game is running standalone and with neural network, initialize input and output shared memory
    if (flags_cmd & CMD_FLAG_STANDALONE && flags_cmd & CMD_FLAG_USE_NEURAL) {
        // allocate shared input and output
        sharedInput = sm_allocateSharedInput(cmd_shInputName);
        sharedOutput = sm_allocateSharedOutput(cmd_shOutputName);

        // initialize default values to shared input and output
        sm_lockSharedInput(sharedInput);
        sm_initSharedInput(sharedInput);
        sm_unlockSharedInput(sharedInput);

        sm_lockSharedOutput(sharedOutput);
        sm_initSharedOutput(sharedOutput);
        // sharedOutput->playerPosX = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->position.x;
        // sharedOutput->playerPosY = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->position.y;
        // sharedOutput->playerRotation = ((ComponentRotation *)dynArrayGet(RotationComponents, player->rotationID))->rotation;
        // sharedOutput->playerSpeedX = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->velocity.x;
        // sharedOutput->playerSpeedY = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->velocity.y;
        sharedOutput->playerPosX = ((ComponentMotion *)xArray_get(motionComponents, player->movementID))->position.x;
        sharedOutput->playerPosY = ((ComponentMotion *)xArray_get(motionComponents, player->movementID))->position.y;
        sharedOutput->playerRotation = ((ComponentRotation *)xArray_get(rotationComponents, player->rotationID))->rotation;
        sharedOutput->playerSpeedX = ((ComponentMotion *)xArray_get(motionComponents, player->movementID))->velocity.x;
        sharedOutput->playerSpeedY = ((ComponentMotion *)xArray_get(motionComponents, player->movementID))->velocity.y;
        sm_unlockSharedOutput(sharedOutput);

        // printf("Shared memory initialized...\n");
    }
    // if game is managed, connect to shared memory using given keys
    else if (flags_cmd & CMD_FLAG_MANAGED) {
        // connect to shared memory locations
        sharedInput = sm_connectSharedInput(cmd_shInputName);
        sharedOutput = sm_connectSharedOutput(cmd_shOutputName);
        sharedState = sm_connectSharedState(cmd_shStateName);

        // update own shared state values
        sm_lockSharedState(sharedState);
        sharedState->state_gameAlive = 1;
        sharedState->game_gameLevel = 1;
        sharedState->game_gameScore = 0;
        sharedState->game_gameTime = 0.0;
        sharedState->game_runHeadless = (flags_cmd & CMD_FLAG_HEADLESS) != 0;
        sm_unlockSharedState(sharedState);

        // update shared output
        sm_lockSharedOutput(sharedOutput);
        sharedOutput->playerPosX = ((ComponentMotion *)xArray_get(motionComponents, player->movementID))->position.x;
        sharedOutput->playerPosY = ((ComponentMotion *)xArray_get(motionComponents, player->movementID))->position.y;
        sharedOutput->playerRotation = ((ComponentRotation *)xArray_get(rotationComponents, player->rotationID))->rotation;
        sharedOutput->playerSpeedX = ((ComponentMotion *)xArray_get(motionComponents, player->movementID))->velocity.x;
        sharedOutput->playerSpeedY = ((ComponentMotion *)xArray_get(motionComponents, player->movementID))->velocity.y;
        sharedOutput->closestAsteroidPosX = 0;
        sharedOutput->closestAsteroidPosY = 0;
        sharedOutput->distanceFront = 0;
        sm_unlockSharedOutput(sharedOutput);
    }

    // create window and unlock maximum framerate (which will be regulated by other means)
    if (!(flags_cmd & CMD_FLAG_HEADLESS)) {
        InitWindow(windowWidth, windowHeight, windowName);
        SetTargetFPS(0);
        flags_runtime |= RUNTIME_WINDOW_ACTIVE;
    }

    // logic timing variables
    struct timespec currentTime;
    const double fixedTimeStep = 1.0 / 60.0;
    double accumulator = 0.0;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    double avgFrameTime = 0.0;

    // main game loop
    while (!(flags_runtime & RUNTIME_EXIT)) {
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

        // delta-time loop
        while (accumulator >= fixedTimeStep) {
            // update input control variables depending on run mode
            if ((flags_cmd & CMD_FLAG_HEADLESS) && !(flags_runtime & RUNTIME_WINDOW_ACTIVE)) {
                sm_lockSharedInput(sharedInput);
                flags_input |= (sharedInput->isKeyDownW) ? INPUT_W : 0;
                flags_input |= (sharedInput->isKeyDownA) ? INPUT_A : 0;
                flags_input |= (sharedInput->isKeyDownD) ? INPUT_D : 0;
                flags_input |= (sharedInput->isKeyDownSpace) ? INPUT_SPACE : 0;
                sm_unlockSharedInput(sharedInput);
            } else if (flags_runtime & RUNTIME_WINDOW_ACTIVE) {
                flags_input |= IsKeyDown(KEY_W) ? INPUT_W : 0;
                flags_input |= IsKeyDown(KEY_A) ? INPUT_A : 0;
                flags_input |= IsKeyDown(KEY_D) ? INPUT_D : 0;
                flags_input |= IsKeyDown(KEY_SPACE) ? INPUT_SPACE : 0;
            }

            // input-based component updates
            if (IsKeyDown(KEY_ESCAPE)) flags_input |= INPUT_EXIT;
            if (IsKeyDown(KEY_H)) flags_cmd |= CMD_FLAG_HEADLESS;
            if (flags_input & INPUT_W) {
                float rotation = ((ComponentRotation *)xArray_get(rotationComponents, player->rotationID))->rotation;
                Vector2 acceleration = Vector2Scale((Vector2){cosf(rotation), sinf(rotation)}, base_playerAcceleration);
                ((ComponentMotion *)xArray_get(motionComponents, player->movementID))->acceleration = acceleration;
            } else {
                Vector2 acceleration = ((ComponentMotion *)xArray_get(motionComponents, player->movementID))->velocity;
                acceleration = Vector2Scale(acceleration, -0.01f / fixedTimeStep);
                ((ComponentMotion *)xArray_get(motionComponents, player->movementID))->acceleration = acceleration;
            }
            if (flags_input & INPUT_A) {
                ((ComponentRotation *)xArray_get(rotationComponents, player->rotationID))->rotationSpeed = -3.f;
            } else if (flags_input & INPUT_D) {
                ((ComponentRotation *)xArray_get(rotationComponents, player->rotationID))->rotationSpeed = 3.f;
            } else {
                ((ComponentRotation *)xArray_get(rotationComponents, player->rotationID))->rotationSpeed = 0.f;
            }

            // headless mode update
            if ((flags_cmd & CMD_FLAG_HEADLESS) && flags_runtime & RUNTIME_WINDOW_ACTIVE) {
                CloseWindow();
                flags_runtime &= ~RUNTIME_WINDOW_ACTIVE;
            } else if (!(flags_cmd & CMD_FLAG_HEADLESS) && !(flags_runtime & RUNTIME_WINDOW_ACTIVE)) {
                InitWindow(windowWidth, windowHeight, windowName);
                SetTargetFPS(0);
                flags_runtime |= RUNTIME_WINDOW_ACTIVE;
            }

            // time-based component updates
            for (int i = 0; i < motionComponents->size; i++) {
                ComponentMotion *tmpMotion = (ComponentMotion *)xArray_get(motionComponents, i);

                // updating velocity and position
                tmpMotion->velocity = Vector2Add(tmpMotion->velocity, Vector2Scale(tmpMotion->acceleration, fixedTimeStep));
                tmpMotion->position = Vector2Add(tmpMotion->position, Vector2Scale(tmpMotion->velocity, fixedTimeStep));

                // position wrapping
                if (tmpMotion->position.x >= windowWidth)
                    tmpMotion->position.x -= windowWidth;
                else if (tmpMotion->position.x < 0)
                    tmpMotion->position.x += windowWidth;
                if (tmpMotion->position.y >= windowHeight)
                    tmpMotion->position.y -= windowHeight;
                else if (tmpMotion->position.y < 0)
                    tmpMotion->position.y += windowHeight;
            }
            for (int i = 0; i < rotationComponents->size; i++) {
                ComponentRotation *tmpRotation = (ComponentRotation *)xArray_get(rotationComponents, i);

                // updating rotation
                tmpRotation->rotation += tmpRotation->rotationSpeed * fixedTimeStep;
            }
            // for (size_t i = 0; i < LifetimeComponents->size; i++) {
            //     ComponentLifeTime *tmpLifetime = (ComponentLifeTime *)dynArrayGet(LifetimeComponents, i);
            //     if (tmpLifetime->isAlive) {
            //         tmpLifetime->lifeTime -= fixedTimeStep;
            //         if (tmpLifetime->lifeTime <= 0) {
            //             tmpLifetime->isAlive = 0;
            //         }
            //     }
            //
            //     printf("Updated lifetime component %zu:\n", i);
            //     printf("\tLifetime: %f\n", tmpLifetime->lifeTime);
            // }

            // exit key pressed
            if (flags_input & INPUT_EXIT) {
                flags_runtime |= RUNTIME_EXIT;
                break;
            }

            // clear input flags
            flags_input &= INPUT_NONE;

            // end game logic updates
            accumulator -= fixedTimeStep;
        }

        // update rendering (if window is open)
        if (flags_runtime & RUNTIME_WINDOW_ACTIVE) {
            BeginDrawing();
            ClearBackground(BLACK);

            // game objects rendering
            DrawPlayer((ComponentMotion *)xArray_get(motionComponents, player->movementID), (ComponentRotation *)xArray_get(rotationComponents, player->rotationID), (ComponentCollisionRect *)xArray_get(rectComponents, player->hitboxID));

            // drawing asteroids
            for (int i = 0; i < asteroidArray->size; i++) {
                AsteroidObject *tmpAsteroid = (AsteroidObject *)xArray_get(asteroidArray, i);
                DrawAsteroid((ComponentMotion *)xArray_get(motionComponents, tmpAsteroid->movementID), (ComponentRotation *)xArray_get(rotationComponents, tmpAsteroid->rotationID), (ComponentCollisionCircle *)xArray_get(circleComponents, tmpAsteroid->hitboxID));
            }

            // UI rendering
            DrawRectangleLines(0, 0, windowWidth, windowHeight, WHITE);  // game border
            DrawText(fpsText, 10, 10, 20, WHITE);                        // FPS counter

            EndDrawing();
        }

        // post-rendering logic (free resources, etc.)

        // update outputs if sharing memory
        if (flags_cmd & (CMD_FLAG_MANAGED | CMD_FLAG_USE_NEURAL)) {
            sm_lockSharedOutput(sharedOutput);
            sharedOutput->playerPosX = ((ComponentMotion *)xArray_get(motionComponents, player->movementID))->position.x;
            sharedOutput->playerPosY = ((ComponentMotion *)xArray_get(motionComponents, player->movementID))->position.y;
            sharedOutput->playerRotation = ((ComponentRotation *)xArray_get(rotationComponents, player->rotationID))->rotation;
            sharedOutput->playerSpeedX = ((ComponentMotion *)xArray_get(motionComponents, player->movementID))->velocity.x;
            sharedOutput->playerSpeedY = ((ComponentMotion *)xArray_get(motionComponents, player->movementID))->velocity.y;
            sm_unlockSharedOutput(sharedOutput);
        }
    }

    // close window if open
    if (flags_runtime & RUNTIME_WINDOW_ACTIVE) {
        CloseWindow();
        flags_runtime &= ~RUNTIME_WINDOW_ACTIVE;
    }

    // free/close shared memory if used
    if (flags_cmd & CMD_FLAG_STANDALONE && flags_cmd & CMD_FLAG_USE_NEURAL) {
        sm_freeSharedInput(sharedInput, cmd_shInputName);
        sm_freeSharedOutput(sharedOutput, cmd_shOutputName);
    } else if (flags_cmd & CMD_FLAG_MANAGED) {
        sm_disconnectSharedInput(sharedInput);
        sm_disconnectSharedOutput(sharedOutput);
        sm_disconnectSharedState(sharedState);
    }

    // free all allocated memory
    free(player);

    xArray_clear(motionComponents);
    xArray_clear(rotationComponents);
    xArray_clear(rectComponents);
    xArray_clear(circleComponents);
    // xArray_clear(lifetimeComponents);

    xArray_free(motionComponents);
    xArray_free(rotationComponents);
    xArray_free(rectComponents);
    xArray_free(circleComponents);
    // xArray_free(lifetimeComponents);

    // exit program
    return 0;
}