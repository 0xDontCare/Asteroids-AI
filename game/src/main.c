#include "main.h"  // game enums, structs, constant definitions, etc.

#include <raylib.h>   // graphics library
#include <raymath.h>  // math library
#include <stdio.h>    // standard input/output library
#include <stdlib.h>   // standard library (malloc, free, etc.)
#include <time.h>     // time library (game logic timer and random seed)

#include "commonUtility.h"  // smaller utility functions which don't belong in any standalone module
#include "sharedMemory.h"   //shared memory interfaces and functions (IPC)
#include "xArray.h"         // dynamic array library
#include "xString.h"        // safer string library (dynamic allocation, length tracking, etc.)

//------------------------------------------------------------------------------------
// program globals

static const int screenWidth = 1024;
static const int screenHeight = 768;

static const double fixedTimeStep = 1.0 / 60.0;  // 60 FPS
static double accumulator = 0.0;
static struct timespec currentTime = {0};
static struct timespec startTime = {0};  // time when game started (used for calculating total game time)

static unsigned short flags_runtime = RUNTIME_NONE;
static unsigned short flags_cmd = CMD_FLAG_NONE;
static unsigned short flags_input = INPUT_NONE;

static char *cmd_shInputName = NULL;
static char *cmd_shOutputName = NULL;
static char *cmd_shStateName = NULL;
static struct sharedInput_s *shInput = NULL;
static struct sharedOutput_s *shOutput = NULL;
static struct sharedState_s *shState = NULL;

static bool gameOver = false;
static bool pause = false;
static unsigned int score = 0;
static unsigned short levelsCleared = 0;

// NOTE: Defined triangle is isosceles with common angles of 70 degrees.
static float shipHeight = 0.0f;

static Player player = {0};
static Shoot shoot[PLAYER_MAX_SHOOTS] = {0};
static xArray *asteroids = NULL;
static Vector2 closestAsteroid = {0};
static float distanceFront = 0.0f;

static double fireCooldown = 0.0;
static int destroyedMeteorsCount = 0;

//------------------------------------------------------------------------------------
// local function declarations

static inline void OpenSharedMemory(void);    // connect to shared memory (or create if standalone-neural mode)
static inline void CloseSharedMemory(void);   // disconnect from shared memory (or destroy if standalone-neural mode)
static inline void UpdateSharedState(void);   // update state flags in shared memory
static inline void UpdateSharedInput(void);   // get input from shared memory
static inline void UpdateSharedOutput(void);  // update output in shared memory
static inline float AsteroidRadius(int x);    // get asteroiFd radius from size class
static inline void PregenAsteroids(void);     // pre-generate asteroids (and clear any existing ones)
static void InitGame(void);                   // initialize game
static void UpdateGame(void);                 // update game (one time step)
static void DrawGame(void);                   // draw game (one frame)
static void UnloadGame(void);                 // unload game (free dynamic structures, shared memory, etc.)

//------------------------------------------------------------------------------------
// program entry point (main)

int main(int argc, char *argv[]) {
    // Parsing command line arguments
    //--------------------------------------------------------------------------------------
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

                flags_cmd |= (CMD_FLAG_MANAGED | CMD_FLAG_HEADLESS);  // managed mode starts headless initially
                cmd_shInputName = argv[i + 1];
                cmd_shOutputName = argv[i + 2];
                cmd_shStateName = argv[i + 3];
                i += 3;

                printf("[DEBUG] Starting managed with input: %s, output: %s, state: %s\n", cmd_shInputName, cmd_shOutputName, cmd_shStateName);
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
        // assuming basic standalone mode by default (user plays the game without any external programs)
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

    // parse set command flags (help and version; others affect later execution)
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
        if (!cu_CStringIsAlphanumeric(cmd_shInputName + 1) ||
            !cu_CStringIsAlphanumeric(cmd_shOutputName + 1) ||
            !cu_CStringIsAlphanumeric(cmd_shStateName + 1)) {
            // +1 to skip first character (which is a forward slash)
            printf("ERROR: Shared memory names can only contain alphanumeric characters.\n");
            return 1;
        }
    }

    // Game initialization (window, screen, objects, timer, etc.)
    //---------------------------------------------------------
    SetTraceLogLevel(LOG_WARNING);
    if (!(flags_cmd & CMD_FLAG_HEADLESS)) {
        InitWindow(screenWidth, screenHeight, "Asteroids");
        SetTargetFPS(0);
        flags_runtime |= RUNTIME_WINDOW_ACTIVE;
    }

    InitGame();
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    startTime = currentTime;
    //--------------------------------------------------------------------------------------

    // Main game loop
    //--------------------------------------------------------------------------------------
    while (!(flags_runtime & RUNTIME_EXIT))  // Detect window close button or ESC key
    {
        // Update timer
        struct timespec newTime = {0};
        clock_gettime(CLOCK_MONOTONIC, &newTime);
        double frameTime = (newTime.tv_sec - currentTime.tv_sec) + (newTime.tv_nsec - currentTime.tv_nsec) / 1000000000.0;
        if (frameTime > 0.25) frameTime = 0.25;  // NOTE: maximum accumulated time to avoid spiral of death
        currentTime = newTime;
        accumulator += frameTime;

        // Update logic (fixed time step)
        while (accumulator >= fixedTimeStep) {
            UpdateGame();
            accumulator -= fixedTimeStep;
        }

        // Draw game
        DrawGame();
    }
    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadGame();  // Unload dynamically loaded data (textures, sounds, models...)

    CloseWindow();  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// local function definitions

// open shared memory (or create if standalone-neural mode)
inline void OpenSharedMemory(void) {
    if (flags_cmd & CMD_FLAG_MANAGED) {
        // connect to already existing shared memory
        shInput = sm_connectSharedInput(cmd_shInputName);
        shOutput = sm_connectSharedOutput(cmd_shOutputName);
        shState = sm_connectSharedState(cmd_shStateName);

        if (shInput == NULL || shOutput == NULL || shState == NULL) {
            printf("ERROR: Failed to connect to shared memory.\n");
            exit(1);
        }
    } else if (flags_cmd & CMD_FLAG_STANDALONE && flags_cmd & CMD_FLAG_USE_NEURAL) {
        // create new shared memory
        shInput = sm_allocateSharedInput(cmd_shInputName);
        shOutput = sm_allocateSharedOutput(cmd_shOutputName);

        if (shInput == NULL || shOutput == NULL) {
            printf("ERROR: Failed to create shared memory.\n");
            exit(1);
        }

        // initialize shared memory values
        sm_initSharedInput(shInput);
        sm_initSharedOutput(shOutput);
    }
    return;
}

inline void CloseSharedMemory(void) {
    if (flags_cmd & CMD_FLAG_MANAGED) {
        // disconnect from shared memory
        sm_disconnectSharedInput(shInput);
        sm_disconnectSharedOutput(shOutput);
        sm_disconnectSharedState(shState);
    } else if (flags_cmd & CMD_FLAG_STANDALONE && flags_cmd & CMD_FLAG_USE_NEURAL) {
        // destroy shared memory
        sm_freeSharedInput(shInput, cmd_shInputName);
        sm_freeSharedOutput(shOutput, cmd_shOutputName);
    }

    // clear dangling pointers
    shInput = NULL;
    shOutput = NULL;
    shState = NULL;
    return;
}

inline void UpdateSharedState(void) {
    if (flags_cmd & CMD_FLAG_MANAGED) {
        // update shared state memory
        sm_lockSharedState(shState);
        shState->game_isOver = gameOver;
        shState->game_isPaused = pause;
        shState->game_gameScore = score;
        shState->game_gameLevel = levelsCleared;
        shState->game_gameTime = (currentTime.tv_sec - startTime.tv_sec);

        if (shState->control_gameExit || !shState->state_managerAlive) {
            // game should exit
            shState->state_gameAlive = false;
            flags_runtime |= RUNTIME_EXIT;
        }
        sm_unlockSharedState(shState);
    }
    return;
}

inline void UpdateSharedInput(void) {
    if (flags_cmd & CMD_FLAG_USE_NEURAL || flags_cmd & CMD_FLAG_MANAGED) {
        // update shared input memory
        sm_lockSharedInput(shInput);
        flags_input = INPUT_NONE;
        flags_input |= shInput->isKeyDownW ? INPUT_W : 0;
        flags_input |= shInput->isKeyDownA ? INPUT_A : 0;
        flags_input |= shInput->isKeyDownD ? INPUT_D : 0;
        flags_input |= shInput->isKeyDownSpace ? INPUT_SPACE : 0;
        sm_unlockSharedInput(shInput);
    }
    return;
}

inline void UpdateSharedOutput(void) {
    if (flags_cmd & CMD_FLAG_USE_NEURAL || flags_cmd & CMD_FLAG_MANAGED) {
        // update shared output memory
        sm_lockSharedOutput(shOutput);
        shOutput->playerPosX = player.position.x;
        shOutput->playerPosY = player.position.y;
        shOutput->playerRotation = player.rotation;
        shOutput->playerSpeedX = player.speed.x;
        shOutput->playerSpeedY = player.speed.y;
        shOutput->distanceFront = distanceFront;
        shOutput->closestAsteroidPosX = closestAsteroid.x;
        shOutput->closestAsteroidPosY = closestAsteroid.y;
        sm_unlockSharedOutput(shOutput);
    }
    return;
}

inline float AsteroidRadius(int x) {
    // function is obtained by polynomial interpolation of points (1, 5), (2, 10), (3, 20)
    return (float)(5.0f / 2.0f * (x * x - x) + 5.0f);
}

// pre-generate asteroids (and clear any existing ones from previous level)
inline void PregenAsteroids(void) {
    // clear old asteroids (if any)
    xArray_clear(asteroids);

    // generate new asteroids
    for (int i = 0; i < ASTEROID_BASE_GENERATION_COUNT + levelsCleared; i++) {
        // allocating new asteroid
        Meteor *newAsteroid = malloc(sizeof(Meteor));
        if (newAsteroid == NULL) {
            printf("ERROR: Failed to allocate memory for new asteroid.\n");
            exit(1);
        }

        // setting asteroid properties
        int posx, posy;
        bool validRange = false;

        newAsteroid->sizeClass = 3;  // all asteroids are large initially
        while (!validRange) {
            if ((posx > screenWidth / 2 - 150 && posx < screenWidth / 2 + 150) || (fabsf(player.position.x - posx) < 50.f)) {
                // asteroid is too close to screen edge or player
                posx = GetRandomValue(0, screenWidth);
            } else {
                validRange = true;
            }
        }
        validRange = false;
        while (!validRange) {
            if ((posy > screenHeight / 2 - 150 && posy < screenHeight / 2 + 150) || (fabsf(player.position.y - posy) < 50.f)) {
                // asteroid is too close to screen edge or player
                posy = GetRandomValue(0, screenHeight);
            } else {
                validRange = true;
            }
        }
        float randomAngle = GetRandomValue(0, 360) * DEG2RAD;

        newAsteroid->position = (Vector2){posx, posy};
        newAsteroid->speed = Vector2Scale((Vector2){cosf(randomAngle), sinf(randomAngle)}, ASTEROID_SPEED);
        newAsteroid->radius = AsteroidRadius(newAsteroid->sizeClass + 2);
        newAsteroid->active = true;
        newAsteroid->color = WHITE;

        // adding asteroid to array
        xArray_push(asteroids, (void *)newAsteroid);
    }
}

// initialize game variables
void InitGame(void) {
    pause = false;
    score = 0;
    levelsCleared = 0;

    // initialization of asteroids array
    if ((asteroids = xArray_new()) == NULL) {
        printf("ERROR: Failed to allocate asteroids array.\n");
        exit(1);
    }

    // initialization of shared memory depending on run mode
    if (flags_cmd & CMD_FLAG_MANAGED || flags_cmd & CMD_FLAG_USE_NEURAL) {
        OpenSharedMemory();
    }

    // initialization of player
    shipHeight = (PLAYER_BASE_SIZE / 2) / tanf(20 * DEG2RAD);
    player.position = (Vector2){screenWidth / 2, screenHeight / 2 - shipHeight / 2};
    player.speed = (Vector2){0, 0};
    player.acceleration = (Vector2){0, 0};
    player.rotation = -(PI / 2);
    player.collider = (Vector3){player.position.x - sinf(player.rotation) * (shipHeight / 2.5f), player.position.y - sinf(player.rotation) * (shipHeight / 2.5f), 12};
    player.color = WHITE;
    destroyedMeteorsCount = 0;

    // initialization of bullets
    for (int i = 0; i < PLAYER_MAX_SHOOTS; i++) {
        shoot[i].position = (Vector2){0, 0};
        shoot[i].speed = (Vector2){0, 0};
        shoot[i].radius = 2;
        shoot[i].active = false;
        shoot[i].lifeSpawn = 0;
        shoot[i].color = WHITE;
    }

    // initialization of asteroids
    PregenAsteroids();

    startTime = currentTime; // NOTE: added to avoid game time not being reset after game restart
}

// update logic (one time step)
void UpdateGame(void) {
    // update input flags (depending on run mode)
    if (flags_cmd & CMD_FLAG_MANAGED || flags_cmd & CMD_FLAG_USE_NEURAL) {
        UpdateSharedInput();
    } else if (flags_runtime & RUNTIME_WINDOW_ACTIVE) {
        flags_input |= IsKeyDown(KEY_W) ? INPUT_W : 0;
        flags_input |= IsKeyDown(KEY_A) ? INPUT_A : 0;
        flags_input |= IsKeyDown(KEY_D) ? INPUT_D : 0;
        flags_input |= IsKeyDown(KEY_SPACE) ? INPUT_SPACE : 0;
        flags_input |= IsKeyPressed(KEY_P) ? INPUT_PAUSE : 0;
        flags_input |= IsKeyPressed(KEY_ENTER) ? INPUT_ENTER : 0;
        flags_input |= IsKeyDown(KEY_ESCAPE) ? INPUT_EXIT : 0;
        flags_runtime |= IsKeyDown(KEY_ESCAPE) ? RUNTIME_EXIT : 0;
    }

    if (!gameOver) {
        if (flags_input & INPUT_PAUSE) pause = !pause;

        if (!pause) {
            // Player logic: rotation
            if (flags_input & INPUT_A) player.rotation -= PLAYER_BASE_ROTATION * fixedTimeStep;
            if (flags_input & INPUT_D) player.rotation += PLAYER_BASE_ROTATION * fixedTimeStep;

            // Player logic: acceleration
            if (flags_input & INPUT_W) {
                player.acceleration = Vector2Scale((Vector2){cosf(player.rotation), sinf(player.rotation)}, PLAYER_BASE_ACCELERATION);
            } else {
                // decelerate to 0.99f of current speed
                player.acceleration = Vector2Scale(player.speed, -0.01f / fixedTimeStep);
            }

            // Player logic: speed
            player.speed = Vector2Add(player.speed, Vector2Scale(player.acceleration, fixedTimeStep));

            // Player logic: movement
            player.position = Vector2Add(player.position, Vector2Scale(player.speed, fixedTimeStep));

            // Collision logic: player vs walls
            if (player.position.x > screenWidth + shipHeight)
                player.position.x = -(shipHeight);
            else if (player.position.x < -(shipHeight))
                player.position.x = screenWidth + shipHeight;
            if (player.position.y > (screenHeight + shipHeight))
                player.position.y = -(shipHeight);
            else if (player.position.y < -(shipHeight))
                player.position.y = screenHeight + shipHeight;

            // Player shoot cooldown logic
            if (fireCooldown > 0.0f) fireCooldown -= 1.0f * fixedTimeStep;

            // Player shoot logic
            if (flags_input & INPUT_SPACE && fireCooldown <= 0.0f) {
                for (int i = 0; i < PLAYER_MAX_SHOOTS; i++) {
                    if (!shoot[i].active) {
                        shoot[i].position = (Vector2){player.position.x + cosf(player.rotation) * (shipHeight), player.position.y + sinf(player.rotation) * (shipHeight)};
                        shoot[i].active = true;
                        shoot[i].speed = Vector2Scale((Vector2){cosf(player.rotation), sinf(player.rotation)}, BULLET_SPEED);
                        shoot[i].rotation = player.rotation;
                        fireCooldown = FIRE_COOLDOWN;
                        break;
                    }
                }
            }

            // Shoot life timer
            for (int i = 0; i < PLAYER_MAX_SHOOTS; i++) {
                if (shoot[i].active) shoot[i].lifeSpawn++;
            }

            // Shot logic
            for (int i = 0; i < PLAYER_MAX_SHOOTS; i++) {
                if (shoot[i].active) {
                    // Movement
                    shoot[i].position = Vector2Add(shoot[i].position, Vector2Scale(shoot[i].speed, fixedTimeStep));

                    // Collision logic: shoot vs walls
                    if (shoot[i].position.x > screenWidth + shoot[i].radius) {
                        shoot[i].active = false;
                        shoot[i].lifeSpawn = 0;
                    } else if (shoot[i].position.x < 0 - shoot[i].radius) {
                        shoot[i].active = false;
                        shoot[i].lifeSpawn = 0;
                    }
                    if (shoot[i].position.y > screenHeight + shoot[i].radius) {
                        shoot[i].active = false;
                        shoot[i].lifeSpawn = 0;
                    } else if (shoot[i].position.y < 0 - shoot[i].radius) {
                        shoot[i].active = false;
                        shoot[i].lifeSpawn = 0;
                    }

                    // Life of shoot
                    if (shoot[i].lifeSpawn >= BULLET_LIFETIME) {
                        shoot[i].position = (Vector2){0, 0};
                        shoot[i].speed = (Vector2){0, 0};
                        shoot[i].lifeSpawn = 0;
                        shoot[i].active = false;
                    }
                }
            }

            // asteroid logic
            player.collider = (Vector3){player.position.x + cosf(player.rotation) * (shipHeight / 2.5f), player.position.y + sinf(player.rotation) * (shipHeight / 2.5f), 12};
            for (int i = 0; i < asteroids->size; i++) {
                Meteor *asteroid = (Meteor *)xArray_get(asteroids, i);
                if (!asteroid->active) continue;

                // collision logic: player vs asteroids
                if (CheckCollisionCircles((Vector2){player.collider.x, player.collider.y}, player.collider.z, asteroid->position, asteroid->radius)) {
                    gameOver = true;
                    break;
                }

                // asteroid logic: movement
                asteroid->position = Vector2Add(asteroid->position, Vector2Scale(asteroid->speed, fixedTimeStep));

                // collision logic: asteroid vs walls
                if (asteroid->position.x > screenWidth + asteroid->radius) {
                    asteroid->position.x = -(asteroid->radius);
                } else if (asteroid->position.x < 0 - asteroid->radius) {
                    asteroid->position.x = screenWidth + asteroid->radius;
                }
                if (asteroid->position.y > screenHeight + asteroid->radius) {
                    asteroid->position.y = -(asteroid->radius);
                } else if (asteroid->position.y < 0 - asteroid->radius) {
                    asteroid->position.y = screenHeight + asteroid->radius;
                }
            }

            // Collision logic: bullets vs asteroids
            for (int i = 0; i < PLAYER_MAX_BULLETS; i++) {
                if (!shoot[i].active) continue;

                for (int j = 0; j < asteroids->size; j++) {
                    Meteor *asteroid = (Meteor *)xArray_get(asteroids, j);
                    if (!asteroid->active) continue;

                    if (CheckCollisionCircles(shoot[i].position, shoot[i].radius, asteroid->position, asteroid->radius)) {
                        shoot[i].active = false;
                        shoot[i].lifeSpawn = 0;
                        asteroid->active = false;
                        score += (asteroid->sizeClass == 3) ? 20 : (asteroid->sizeClass == 2) ? 50
                                                                                              : 100;
                        destroyedMeteorsCount++;

                        // spawn smaller asteroids
                        if (asteroid->sizeClass > 1) {
                            for (int k = 0; k < 2; k++) {
                                // allocating new asteroid
                                Meteor *newAsteroid = malloc(sizeof(Meteor));
                                if (newAsteroid == NULL) {
                                    printf("ERROR: Failed to allocate memory for new asteroid.\n");
                                    exit(1);
                                }

                                // setting asteroid properties
                                newAsteroid->sizeClass = asteroid->sizeClass - 1;
                                newAsteroid->position = (Vector2){asteroid->position.x, asteroid->position.y};
                                newAsteroid->speed = (Vector2){sinf(shoot[i].rotation) * ASTEROID_SPEED * (k == 0 ? -(4 - newAsteroid->sizeClass) : (4 - newAsteroid->sizeClass)), cosf(shoot[i].rotation) * ASTEROID_SPEED * (k == 0 ? (4 - newAsteroid->sizeClass) : -(4 - newAsteroid->sizeClass))};
                                newAsteroid->radius = AsteroidRadius(newAsteroid->sizeClass + 2);
                                newAsteroid->active = true;
                                newAsteroid->color = WHITE;

                                // adding asteroid to array
                                xArray_push(asteroids, (void *)newAsteroid);
                            }
                        }
                        break;
                    }
                }
            }

            // calculating closest asteroid
            float minDistance = screenWidth * screenHeight;
            for (int i = 0; i < asteroids->size; i++) {
                Meteor *asteroid = (Meteor *)xArray_get(asteroids, i);
                if (!asteroid->active) continue;

                float tmpDistance = Vector2Distance(player.position, asteroid->position);
                if (tmpDistance < minDistance) {
                    minDistance = tmpDistance;
                    closestAsteroid = asteroid->position;
                }
            }
        }

        // all asteroids destroyed -> next level
        if (destroyedMeteorsCount == asteroids->size) {
            levelsCleared++;
            PregenAsteroids();
            destroyedMeteorsCount = 0;
        }
    } else if (flags_input & INPUT_ENTER) {
        InitGame();
        gameOver = false;
    }

    // update output and state IPC (depending on run mode)
    UpdateSharedOutput();
    UpdateSharedState();

    flags_input &= INPUT_NONE;
}

// draw game (one frame)
void DrawGame(void) {
    BeginDrawing();

    ClearBackground(BLACK);

    if (!gameOver) {
        // Draw spaceship
        Vector2 v1 = {player.position.x + cosf(player.rotation) * (shipHeight), player.position.y + sinf(player.rotation) * (shipHeight)};
        Vector2 v2 = {player.position.x + sinf(player.rotation) * (PLAYER_BASE_SIZE / 2), player.position.y - cosf(player.rotation) * (PLAYER_BASE_SIZE / 2)};
        Vector2 v3 = {player.position.x - sinf(player.rotation) * (PLAYER_BASE_SIZE / 2), player.position.y + cosf(player.rotation) * (PLAYER_BASE_SIZE / 2)};
        DrawTriangleLines(v1, v2, v3, player.color);

        // Draw asteroids
        for (int i = 0; i < asteroids->size; i++) {
            Meteor *asteroid = (Meteor *)xArray_get(asteroids, i);
            if (asteroid->active) {
                DrawCircleLines(asteroid->position.x, asteroid->position.y, asteroid->radius, asteroid->color);
                //DrawCircleV(asteroid->position, asteroid->radius, asteroid->color);
            } else {
                DrawCircleV(asteroid->position, asteroid->radius, Fade(DARKGRAY, 0.3f));
            }
        }

        // Draw shoot
        for (int i = 0; i < PLAYER_MAX_SHOOTS; i++) {
            if (shoot[i].active) DrawCircleV(shoot[i].position, shoot[i].radius, shoot[i].color);
        }

        // DEBUG: Drawing colliders, closest asteroid and distance from player to closest asteroid
        DrawCircleLines(player.collider.x, player.collider.y, player.collider.z, GREEN);
        DrawCircleV(player.position, 5, BLUE);
        DrawCircleV(closestAsteroid, 5, RED);
        DrawText(TextFormat("Distance: %f", distanceFront), 20, 80, 20, WHITE);

        // Draw status (score, levels cleared, time survived)
        DrawText(TextFormat("SCORE: %04i", score), 20, 20, 20, WHITE);
        DrawText(TextFormat("LEVEL: %02i", levelsCleared + 1), 20, 40, 20, WHITE);
        DrawText(TextFormat("TIME: %02i:%02i", (int)(currentTime.tv_sec - startTime.tv_sec) / 60, (int)(currentTime.tv_sec - startTime.tv_sec) % 60), 20, 60, 20, WHITE);

        if (pause) DrawText("GAME PAUSED", screenWidth / 2 - MeasureText("GAME PAUSED", 40) / 2, screenHeight / 2 - 40, 40, WHITE);
    } else {
        DrawText("GAME OVER", GetScreenWidth() / 2 - MeasureText("GAME OVER", 20) / 2, GetScreenHeight() / 2 - 50, 20, WHITE);
        if (flags_runtime & RUNTIME_WINDOW_ACTIVE && !(flags_cmd & CMD_FLAG_USE_NEURAL || flags_cmd & CMD_FLAG_MANAGED)) {
            DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth() / 2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20) / 2, GetScreenHeight() / 2 - 10, 20, WHITE);
        }
    }

    EndDrawing();
}

// unload game variables
void UnloadGame(void) {
    // close shared memory (if any)
    CloseSharedMemory();

    // clear all dynamic structures
    xArray_clear(asteroids);
    xArray_free(asteroids);
}
