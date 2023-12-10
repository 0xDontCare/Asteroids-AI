/**
 * @file programGame.c
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Main game program. Depending on how it is started, it can take input from keyboard or from external programs in shared memory.
 * @version 0.2
 * @date 07.11.2023.
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <raylib.h>   // graphics library
#include <raymath.h>  // math library
#include <stdio.h>    // standard input/output library
#include <time.h>     // time library

#include "asteroidsObjects.h"     // game entities and components
#include "asteroidsShared.h"      // shared memory interfaces and functions
#include "asteroidsStructures.h"  // general data structures
#include "commonUtility.h"        // smaller common utility functions which don't have much to do with the game itself
#include "xString.h"              // string library

// TODO: replace asteroidsStructures.h with modules from xcFramework (since xString.h is already used)

int main(int argc, char *argv[]) {
    /* Command line flags:
     * 0x01 - help
     * 0x02 - version
     * 0x04 - standalone mode
     * 0x08 - headless mode
     * 0x10 - use neural network (+2 parameters)
     * 0x20 - managed mode (+3 parameters)
     */
    short cmd_flags = 0;
    char *cmd_shInputName = NULL, *cmd_shOutputName = NULL, *cmd_shStateName = NULL;  // shared memory access names

    // runtime states <- can be replaced with command line flags
    int run_headless = 0;      // headless mode (no graphics)
    int run_standalone = 0;    // standalone mode (no external managing program present)
    int run_neural = 0;        // neural network mode (input and output on shared memory)
    int run_windowActive = 0;  // window active (used to check if window is open)

    // parsing command line arguments
    if (argc > 1) {
        int i;
        for (i = 1; i < argc; i++) {
            xString *tmpString = xString_fromCString(argv[i]);
            if (xString_isEqualCString(tmpString, "-h") || xString_isEqualCString(tmpString, "--help")) {
                cmd_flags |= 0x01;
            } else if (xString_isEqualCString(tmpString, "-v") || xString_isEqualCString(tmpString, "--version")) {
                cmd_flags |= 0x02;
            } else if (xString_isEqualCString(tmpString, "-s") || xString_isEqualCString(tmpString, "--standalone")) {
                cmd_flags |= 0x04;
            } else if (xString_isEqualCString(tmpString, "-H") || xString_isEqualCString(tmpString, "--headless")) {
                cmd_flags |= 0x08;
            } else if (xString_isEqualCString(tmpString, "-n") || xString_isEqualCString(tmpString, "--neural")) {
                if (i + 2 >= argc) break;

                cmd_flags |= 0x10;
                cmd_shInputName = argv[i + 1];
                cmd_shOutputName = argv[i + 2];
                i += 2;
            } else if (xString_isEqualCString(tmpString, "-m") || xString_isEqualCString(tmpString, "--managed")) {
                if (i + 3 >= argc) break;

                cmd_flags |= 0x20;
                cmd_shInputName = argv[i + 1];
                cmd_shOutputName = argv[i + 2];
                cmd_shStateName = argv[i + 3];
                i += 3;
            } else {
                printf("Unknown command line argument: %s\n", argv[i]);
                printf("Use %s --help for more information.\n", argv[0]);
                xString_free(tmpString);
                return 1;
            }
            xString_free(tmpString);
        }
        if (i != argc) {
            printf("Invalid command line arguments.\n");
            printf("Use %s --help for more information.\n", argv[0]);
            return 1;
        }
    } else {
        // assuming standalone mode with no external input by default
        cmd_flags |= 0x04;
    }

    // check for flag conflicts
    if ((cmd_flags & 0x04 && cmd_flags & 0x08) ||     // standalone and headless mode cannot be used at the same time
        (cmd_flags & 0x04 && cmd_flags & 0x20) ||     // standalone and managed mode cannot be used at the same time
        (cmd_flags & 0x01 && cmd_flags & 0xFE) ||     // help flag is exclusive to all other flags
        (cmd_flags & 0x02 && cmd_flags & 0xFD) ||     // version flag is exclusive to all other flags
        (cmd_flags & 0x08 && !(cmd_flags & 0x20)) ||  // headless mode requires managed mode
        (cmd_flags & 0x10 && cmd_flags & 0x20)) {     // neural network mode and managed mode cannot be used at the same time (managed mode already implies neural network mode)
        printf("Invalid command line arguments.\n");
        printf("Use %s --help for more information.\n", argv[0]);
        return 1;
    }

    // parse set command flags
    if (cmd_flags & 0x01) {
        printf("Usage: %s [OPTIONS]\n", argv[0]);
        printf("Options:\n");
        printf("  -h, --help\t\t\t\t\tPrint this help message and exit.\n");
        printf("  -v, --version\t\t\t\t\tPrint version information and exit.\n");
        printf("  -s, --standalone\t\t\t\tRun game in standalone mode (no external manager program).\n");
        printf("  -H, --headless\t\t\t\tRun game in headless mode (no window). Use together with --managed\n");
        printf("  -n, --neural <input> <output>\t\t\tRun game in neural network mode (input and output shared memory names).\n");
        printf("  -m, --managed <input> <output> <state>\tRun game in managed mode (input, output and state shared memory names).\n");
        return 0;
    } else if (cmd_flags & 0x02) {
        printf("Program:\t\tAsteroids-game\n");
        printf("Version:\t\tDEV (P1.1)\n");
        printf("Compiler version:\t%s\n", __VERSION__);
        printf("Raylib version:\t\t%s\n", RAYLIB_VERSION);
        printf("Compiled on %s at %s\n", __DATE__, __TIME__);
        return 0;
    }
    if (cmd_flags & 0x04) {
        printf("Running in standalone mode.\n");
        run_standalone = 1;
    }
    if (cmd_flags & 0x08) {
        printf("Running in headless mode.\n");
        run_headless = 1;
    }
    if (cmd_flags & 0x10) {
        printf("Neural network access enabled.\n");
        printf("Input shared memory name: %s\n", cmd_shInputName);
        printf("Output shared memory name: %s\n", cmd_shOutputName);
        run_neural = 1;
    }
    if (cmd_flags & 0x20) {
        printf("Running in managed mode.\n");
        printf("Input shared memory name: %s\n", cmd_shInputName);
        printf("Output shared memory name: %s\n", cmd_shOutputName);
        printf("State shared memory name: %s\n", cmd_shStateName);
    }
    printf("Command line arguments parsed successfully.\n");

    // flag arguments (shared memory names) should be only alphanumeric strings
    if (cmd_flags & 0x30) {
        if (!cu_CStringIsAlphanumeric(cmd_shInputName) || !cu_CStringIsAlphanumeric(cmd_shOutputName) || !cu_CStringIsAlphanumeric(cmd_shStateName)) {
            printf("Shared memory names can only contain alphanumeric characters.\n");
            return 1;
        } else {
            printf("Shared memory names are valid.\n");
        }
    }
    // TODO: apply new command line arguments to the rest of the program
    printf("Starting game...\n");
    return 0;

    // game window dimensions (if window resizes, playing dimensions stay same)
    const int windowWidth = 1024;
    const int windowHeight = 768;
    const char *windowName = "Asteroids";

    // input controls
    int input_forward = 0;
    int input_right = 0;
    int input_left = 0;
    int input_shoot = 0;
    int input_exit = 0;
    int input_toggleHeadless = 0;
    // TODO: replace multiple input variables with bitfield (one day hopefuly)

    // game constants
    const float base_playerAcceleration = 500.f;

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

    // TODO: depending on run mode, allocate, connect to or skip creating shared memory
    struct sharedInput_s *sharedInput = NULL;
    struct sharedOutput_s *sharedOutput = NULL;
    struct sharedState_s *sharedState = NULL;

    // if game is running standalone and without neural network, sharing memory is not needed
    if (cmd_flags & 0x04 && cmd_flags & 0x10) {
        // create shared input and output
        sharedInput = as_allocateSharedInput(cmd_shInputName);
        sharedOutput = as_allocateSharedOutput(cmd_shOutputName);

        // initialize shared input and output
        as_lockSharedInput(sharedInput);
        as_initSharedInput(sharedInput);
        as_unlockSharedInput(sharedInput);
        as_lockSharedOutput(sharedOutput);
        as_initSharedOutput(sharedOutput);
        as_unlockSharedOutput(sharedOutput);

        // update shared output
        as_lockSharedOutput(sharedOutput);
        sharedOutput->playerPosX = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->position.x;
        sharedOutput->playerPosY = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->position.y;
        sharedOutput->playerRotation = ((ComponentRotation *)dynArrayGet(RotationComponents, player->rotationID))->rotation;
        sharedOutput->playerSpeedX = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->velocity.x;
        sharedOutput->playerSpeedY = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->velocity.y;
        as_unlockSharedOutput(sharedOutput);
    }
    // else if in managed mode, connect to shared memory and set own values
    else if (cmd_flags & 0x20) {
        // connect to shared memory locations
        sharedInput = as_connectSharedInput(cmd_shInputName);
        sharedOutput = as_connectSharedOutput(cmd_shOutputName);
        sharedState = as_connectSharedState(cmd_shStateName);

        // set own values to shared state
        as_lockSharedState(sharedState);
        sharedState->state_gameAlive = 1;
        sharedState->game_gameLevel = 1;
        sharedState->game_gameScore = 0;
        sharedState->game_gameTime = 0.0;
        sharedState->game_runHeadless = run_headless;
        as_unlockSharedState(sharedState);

        // update shared output
        as_lockSharedOutput(sharedOutput);
        sharedOutput->playerPosX = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->position.x;
        sharedOutput->playerPosY = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->position.y;
        sharedOutput->playerRotation = ((ComponentRotation *)dynArrayGet(RotationComponents, player->rotationID))->rotation;
        sharedOutput->playerSpeedX = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->velocity.x;
        sharedOutput->playerSpeedY = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->velocity.y;
        as_unlockSharedOutput(sharedOutput);
    }

    // create window and unlock maximum framerate (which will be regulated by other means)
    if (!run_headless) {
        InitWindow(windowWidth, windowHeight, windowName);
        SetTargetFPS(0);
        run_windowActive = 1;
    }

    // logic timing variables
    struct timespec currentTime;
    const double fixedTimeStep = 1.0 / 60.0;
    double accumulator = 0.0;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    double avgFrameTime = 0.0;

    // main game loop
    while (input_exit) {
        // pre-rendering logic (load resources, update physics, etc.)

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
            if (run_headless && !run_windowActive) {
                as_lockSharedInput(sharedInput);
                input_forward = sharedInput->isKeyDownW;
                input_left = sharedInput->isKeyDownA;
                input_right = sharedInput->isKeyDownD;
                input_shoot = sharedInput->isKeyDownSpace;
                as_unlockSharedInput(sharedInput);
            } else if (run_windowActive) {
                input_forward = IsKeyDown(KEY_W);
                input_left = IsKeyDown(KEY_A);
                input_right = IsKeyDown(KEY_D);
                input_shoot = IsKeyDown(KEY_SPACE);
            }

            // input-based component updates
            if (IsKeyDown(KEY_H)) run_headless = 1;
            if (input_forward) {
                float rotation = ((ComponentRotation *)dynArrayGet(RotationComponents, player->rotationID))->rotation;
                Vector2 acceleration = Vector2Scale((Vector2){cosf(rotation), sinf(rotation)}, base_playerAcceleration);
                ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->acceleration = acceleration;
            } else {
                Vector2 acceleration = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->velocity;
                acceleration = Vector2Scale(acceleration, -0.01f / fixedTimeStep);
                ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->acceleration = acceleration;
            }
            if (input_left) {
                ((ComponentRotation *)dynArrayGet(RotationComponents, player->rotationID))->rotationSpeed = -3.f;
            } else if (input_right) {
                ((ComponentRotation *)dynArrayGet(RotationComponents, player->rotationID))->rotationSpeed = 3.f;
            } else
                ((ComponentRotation *)dynArrayGet(RotationComponents, player->rotationID))->rotationSpeed = 0.f;

            // headless mode update
            if (run_headless && run_windowActive) {
                CloseWindow();
                run_windowActive = 0;
            } else if (!run_headless && !run_windowActive) {
                InitWindow(windowWidth, windowHeight, windowName);
                SetTargetFPS(0);
                run_windowActive = 1;
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
        if (run_windowActive) {
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
        as_lockSharedOutput(sharedOutput);
        sharedOutput->playerPosX = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->position.x;
        sharedOutput->playerPosY = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->position.y;
        sharedOutput->playerRotation = ((ComponentRotation *)dynArrayGet(RotationComponents, player->rotationID))->rotation;
        sharedOutput->playerSpeedX = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->velocity.x;
        sharedOutput->playerSpeedY = ((ComponentMotion *)dynArrayGet(MotionComponents, player->movementID))->velocity.y;
        as_unlockSharedOutput(sharedOutput);
    }

    if (run_windowActive) {
        CloseWindow();
        run_windowActive = 0;
    }

    // close shared memory
    as_freeSharedInput(sharedInput, "sharedInput");
    as_freeSharedOutput(sharedOutput, "sharedOutput");

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
