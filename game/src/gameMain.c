#include "gameMain.h"  // game enums, structs, constant definitions, etc.

#include <fcntl.h>    // file control options (open, close, etc.)
#include <raylib.h>   // graphics library
#include <raymath.h>  // math library
#include <signal.h>   // signal handling library
#include <stdio.h>    // standard input/output library
#include <stdlib.h>   // standard library (malloc, free, etc.)
#include <time.h>     // time library (game logic timer and random seed)
#include <unistd.h>   // UNIX standard library (fork, exec, etc.)

#include "commonUtility.h"  // smaller utility functions which don't belong in any standalone module
#include "sharedMemory.h"   //shared memory interfaces and functions (IPC)
#include "xArray.h"         // dynamic array library
#include "xString.h"        // safer string library (dynamic allocation, length tracking, etc.)

//------------------------------------------------------------------------------------
// program globals

static const int screenWidth = 1024;
static const int screenHeight = 768;
static float screenDiagonal;

static const double fixedTimeStep = 1.0 / 60.0;  // 60 FPS
static double accumulator = 0.0;
static struct timespec currentTime = {0};
static struct timespec startTime = {0};  // time when game started (used for calculating total game time)

static unsigned short flags_runtime = RUNTIME_NONE;
static unsigned short flags_cmd = CMD_FLAG_NONE;
static unsigned short flags_input = INPUT_NONE;
pid_t pid_neurons = 0;  // process ID of neural network program (used for sending signals if game is managing the network)

static char *cmd_shInputName = NULL;
static char *cmd_shOutputName = NULL;
static char *cmd_shStateName = NULL;
static char *cmd_nmodelPath = NULL;
static struct sharedInput_s *shInput = NULL;
static struct sharedOutput_s *shOutput = NULL;
static struct sharedState_s *shState = NULL;

static bool gameOver = false;
static bool gamePaused = false;
static unsigned int score = 0;
static unsigned short levelsCleared = 0;

// NOTE: Defined triangle is isosceles with common angles of 70 degrees.
static float shipHeight = 0.0f;

static Player player = {0};                      // player object
static Bullet bullet[PLAYER_MAX_BULLETS] = {0};  // array of bullets
static xArray *asteroids = NULL;                 // dynamic array of asteroids

// calculated values for neural network input
static Vector2 closestAsteroid = {0};   // polar coordinates of closest asteroid relative to player
static Vector2 relativeVelocity = {0};  // relative velocity of player and closest asteroid

static double fireCooldown = 0.0;
static int destroyedMeteorsCount = 0;
static int wastedBulletsCount = 0;

//------------------------------------------------------------------------------------
// local function declarations

static inline void OpenSharedMemory(void);    // connect to shared memory (or create if standalone-neural mode)
static inline void CloseSharedMemory(void);   // disconnect from shared memory (or destroy if standalone-neural mode)
static inline void UpdateSharedState(void);   // update state flags in shared memory
static inline void UpdateSharedInput(void);   // get input from shared memory
static inline void UpdateSharedOutput(void);  // update output in shared memory
static inline float AsteroidRadius(int x);    // get asteroiFd radius from size class
static inline void PregenAsteroids(void);     // pre-generate asteroids (and clear any existing ones)
static Vector2 ClosestAsteroid(void);         // get distance and delta-rotation to closest asteroid
static void InitGame(void);                   // initialize game
static void UpdateGame(void);                 // update game (one time step)
static void DrawGame(void);                   // draw game (one frame)
static void UnloadGame(void);                 // unload game (free dynamic structures, shared memory, etc.)

//------------------------------------------------------------------------------------
// program entry point (main)

int main(int argc, char *argv[])
{
    // Seeding PRNG for case if it doesn't get seeded by command line argument
    srand((unsigned int)time(NULL));

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
            } else if (xString_isEqualCString(tmpString, "-m") || xString_isEqualCString(tmpString, "--managed")) {
                if (i + 3 >= argc)
                    break;

                flags_cmd |= (CMD_FLAG_MANAGED | CMD_FLAG_HEADLESS);  // managed mode starts headless initially
                cmd_shInputName = argv[i + 1];
                cmd_shOutputName = argv[i + 2];
                cmd_shStateName = argv[i + 3];
                i += 3;
            } else if (xString_isEqualCString(tmpString, "-nr") || xString_isEqualCString(tmpString, "--neural-random")) {
                flags_cmd |= CMD_FLAG_USE_NEURAL | CMD_FLAG_NEURAL_RANDOM;
                cmd_shInputName = "asteroids0_in";
                cmd_shOutputName = "asteroids0_out";
            } else if (xString_isEqualCString(tmpString, "-nl") || xString_isEqualCString(tmpString, "--neural-load")) {
                if (i + 1 >= argc)
                    break;

                flags_cmd |= CMD_FLAG_USE_NEURAL | CMD_FLAG_NEURAL_FILE;
                cmd_shInputName = "asteroids0_in";
                cmd_shOutputName = "asteroids0_out";
                cmd_nmodelPath = argv[i + 1];
                i += 1;
            } else if (xString_isEqualCString(tmpString, "-r") || xString_isEqualCString(tmpString, "--random")) {
                if (i + 1 > argc || !cu_CStringIsNumeric(argv[i + 1]))
                    break;

                srand((unsigned int)atoi(argv[i + 1]));

                i += 1;
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
    if ((flags_cmd & CMD_FLAG_STANDALONE &&
         flags_cmd & CMD_FLAG_HEADLESS) ||  // standalone and headless mode cannot be used at the same time
        (flags_cmd & CMD_FLAG_STANDALONE &&
         flags_cmd & CMD_FLAG_MANAGED) ||                             // standalone and managed mode cannot be used at the same time
        (flags_cmd & CMD_FLAG_HELP && flags_cmd & ~CMD_FLAG_HELP) ||  // help flag is exclusive to all other flags
        (flags_cmd & CMD_FLAG_VERSION && flags_cmd & ~CMD_FLAG_VERSION) ||     // version flag is exclusive to all other flags
        (flags_cmd & CMD_FLAG_HEADLESS && !(flags_cmd & CMD_FLAG_MANAGED)) ||  // headless mode requires managed mode
        (flags_cmd & CMD_FLAG_USE_NEURAL &&
         flags_cmd & CMD_FLAG_MANAGED)) {  // neural network mode and managed mode cannot be defined at the same time (managed mode
                                           // already implies neural network mode later on)
        printf("ERROR: Invalid command line arguments.\n");
        printf("Use %s --help for more information.\n", argv[0]);
        return 1;
    }

    // if game is started with no arguments, assume standalone mode (player control)
    if ((flags_cmd & (CMD_FLAG_STANDALONE | CMD_FLAG_MANAGED)) == 0) {
        // no run mode specified, defaulting to standalone mode
        flags_cmd |= CMD_FLAG_STANDALONE;
    }

    // parse set command flags (help and version; others affect later execution)
    if (flags_cmd & CMD_FLAG_HELP) {
        printf("Usage: %s [OPTIONS]\n", argv[0]);
        printf("Options:\n");
        printf("  -h, --help\t\t\t\t\tPrint this help message and exit.\n");
        printf("  -v, --version\t\t\t\t\tPrint version information and exit.\n");
        printf("  -s, --standalone\t\t\t\tRun game in standalone mode (no external manager program).\n");
        printf("  -H, --headless\t\t\t\tRun game in headless mode (no window). Use together with --managed\n");
        printf("  -nr, --neural-random\t\t\t\tRun game with randomly initialized neural network.\n");
        printf("  -nl, --neural-load <model>\t\t\tRun game with neural network loaded from .fnnm model file.\n");
        printf(
            "  -m, --managed <input> <output> <state>\tRun game in managed mode (input, output and state shared memory names).\n");
        printf("  -r, --random <seed>\t\t\t\tSet random seed for game initialization.\n");
        return 0;
    } else if (flags_cmd & CMD_FLAG_VERSION) {
        printf("Program:\t\tAsteroids-game\n");
        printf("Version:\t\tDEV (P3.0)\n");
        printf("Compiler version:\t%s\n", __VERSION__);
        printf("Raylib version:\t\t%s\n", RAYLIB_VERSION);
        printf("Compiled on %s at %s\n", __DATE__, __TIME__);
        return 0;
    }

    // flag arguments (shared memory names) should be only alphanumeric strings with underscores
    if (flags_cmd & (CMD_FLAG_MANAGED | CMD_FLAG_USE_NEURAL)) {
        bool validNames = true;
        validNames &= sm_validateSharedMemoryName(cmd_shInputName);
        validNames &= sm_validateSharedMemoryName(cmd_shOutputName);
        if (flags_cmd & CMD_FLAG_MANAGED) {
            validNames &= sm_validateSharedMemoryName(cmd_shStateName);
            flags_cmd |= CMD_FLAG_USE_NEURAL;  // managed mode implies neural network mode
        }

        if (!validNames) {
            printf("ERROR: Shared memory names can only contain alphanumeric characters and underscores.\n");
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

    if (flags_cmd & CMD_FLAG_STANDALONE && flags_cmd & CMD_FLAG_USE_NEURAL) {
        // prepare arguments for neural network program
        char *argvNeural[] = {"./bin/neurons",
                              "-s",
                              cmd_shInputName,
                              cmd_shOutputName,
                              (flags_cmd & CMD_FLAG_NEURAL_FILE) ? "-l" : NULL,
                              (flags_cmd & CMD_FLAG_NEURAL_FILE) ? cmd_nmodelPath : NULL,
                              NULL};

        // fork process for neural network
        pid_neurons = fork();
        if (pid_neurons == 0) {
            // child process
            setsid();                                   // create new session for neural network process
            int devNull = open("/dev/null", O_WRONLY);  // open /dev/null for writing
            if (devNull == -1) {
                printf("ERROR: Failed to open /dev/null for writing.\n");
                exit(1);
            }
            dup2(devNull, STDOUT_FILENO);  // redirect stdout to /dev/null
            close(devNull);                // close /dev/null
            execv(argvNeural[0], argvNeural);
            printf("ERROR: Failed to start neural network program.\n");
            exit(1);
        } else if (pid_neurons < 0) {
            // error
            printf("ERROR: Failed to fork neural network process.\n");
            exit(1);
        }
    }

    //--------------------------------------------------------------------------------------

    // Main game loop
    //--------------------------------------------------------------------------------------
    while (!(flags_runtime & RUNTIME_EXIT))  // Detect window close button or ESC key
    {
        // Toggle window/headless mode
        if (flags_cmd & CMD_FLAG_HEADLESS && flags_runtime & RUNTIME_WINDOW_ACTIVE) {
            CloseWindow();
            flags_runtime &= ~RUNTIME_WINDOW_ACTIVE;
        } else if (!(flags_cmd & CMD_FLAG_HEADLESS) && !(flags_runtime & RUNTIME_WINDOW_ACTIVE)) {
            InitWindow(screenWidth, screenHeight, "Asteroids");
            SetTargetFPS(0);
            flags_runtime |= RUNTIME_WINDOW_ACTIVE;
        }

        // Update timer
        struct timespec newTime = {0};
        clock_gettime(CLOCK_MONOTONIC, &newTime);
        double frameTime = (newTime.tv_sec - currentTime.tv_sec) + (newTime.tv_nsec - currentTime.tv_nsec) / 1000000000.0;
        if (frameTime > 0.25)
            frameTime = 0.25;  // NOTE: maximum accumulated time to avoid spiral of death
        currentTime = newTime;
        accumulator += frameTime;

        // Update logic (fixed time step)
        while (accumulator >= fixedTimeStep) {
            UpdateGame();
            accumulator -= fixedTimeStep;
        }

        // Draw game
        if (flags_runtime & RUNTIME_WINDOW_ACTIVE) {
            DrawGame();
        }
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadGame();  // Unload dynamically loaded data (textures, sounds, models...)

    if (flags_runtime & RUNTIME_WINDOW_ACTIVE) {
        CloseWindow();  // Close window and OpenGL context
    }
    //--------------------------------------------------------------------------------------
    return 0;
}

//------------------------------------------------------------------------------------
// local function definitions

// open shared memory (or create if standalone-neural mode)
static inline void OpenSharedMemory(void)
{
    if (flags_cmd & CMD_FLAG_MANAGED) {
        // connect to already existing shared memory
        shInput = sm_connectSharedInput(cmd_shInputName);
        shOutput = sm_connectSharedOutput(cmd_shOutputName);
        shState = sm_connectSharedState(cmd_shStateName);

        if (shInput == NULL || shOutput == NULL || shState == NULL) {
            printf("ERROR: Failed to connect to shared memory.\n");
            exit(1);
        }

        // set shared state variables
        sm_lockSharedState(shState);
        shState->state_gameAlive = true;
        shState->game_isOver = false;
        shState->game_isPaused = false;
        flags_cmd |= shState->game_runHeadless ? CMD_FLAG_HEADLESS : 0;
        sm_unlockSharedState(shState);
    } else if (flags_cmd & CMD_FLAG_STANDALONE) {
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

static inline void CloseSharedMemory(void)
{
    if (flags_cmd & CMD_FLAG_MANAGED) {
        // disconnect from shared memory
        sm_lockSharedState(shState);
        shState->state_gameAlive = false;
        shState->game_isOver = true;
        sm_unlockSharedState(shState);

        sm_disconnectSharedInput(shInput);
        sm_disconnectSharedOutput(shOutput);
        sm_disconnectSharedState(shState);
    } else if (flags_cmd & CMD_FLAG_STANDALONE) {
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

static inline void UpdateSharedState(void)
{
    if (flags_cmd & CMD_FLAG_MANAGED) {
        // update shared state memory
        sm_lockSharedState(shState);
        shState->game_isOver = gameOver;
        shState->game_isPaused = gamePaused;
        shState->game_gameScore = score;
        shState->game_gameLevel = levelsCleared;
        shState->game_gameTime = (currentTime.tv_sec - startTime.tv_sec);
        flags_cmd &= ~CMD_FLAG_HEADLESS | (shState->game_runHeadless ? CMD_FLAG_HEADLESS : 0);

        if (shState->control_gameExit || !shState->state_managerAlive) {
            // game should exit
            shState->state_gameAlive = false;
            flags_runtime |= RUNTIME_EXIT;
        }
        sm_unlockSharedState(shState);
    }
    return;
}

static inline void UpdateSharedInput(void)
{
    if (flags_cmd & CMD_FLAG_USE_NEURAL) {
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

static inline void UpdateSharedOutput(void)
{
    /*
     * CURRENT GAME OUTPUT VALUES FOR NEURAL NETWORK:
     * 01: Absolute rotation of player in environment [-1, 1]
     * 02: Relative velocity of player to closest asteroid (x) [-1, 1]
     * 03: Relative velocity of player to closest asteroid (y) [-1, 1]
     * 04: Closest asteroid euclidean distance divided by screen diagonal (x) [0, 1]
     * 05: Relative rotation of closest asteroid to player [-1, 1]
     */
    if (flags_cmd & CMD_FLAG_USE_NEURAL) {
        // update shared output memory
        sm_lockSharedOutput(shOutput);
        shOutput->gameOutput01 = player.rotation / PI;
        shOutput->gameOutput02 = relativeVelocity.x / (ASTEROID_SPEED + PLAYER_MAX_SPEED);
        shOutput->gameOutput03 = relativeVelocity.y / (ASTEROID_SPEED + PLAYER_MAX_SPEED);
        shOutput->gameOutput04 = closestAsteroid.x / screenDiagonal;
        shOutput->gameOutput05 = closestAsteroid.y / PI;
        sm_unlockSharedOutput(shOutput);
    }
    return;
}

static inline float AsteroidRadius(int x)
{
    // function is obtained by polynomial interpolation of points (1, 5), (2, 10), (3, 20)
    return (float)(5.0f / 2.0f * (x * x - x) + 5.0f);
}

// pre-generate asteroids (and clear any existing ones from previous level)
static inline void PregenAsteroids(void)
{
    // clear old asteroids (if any)
    xArray_clear(asteroids);

    // generate new asteroids
    for (int i = 0; i < ASTEROID_BASE_GENERATION_COUNT + levelsCleared; i++) {
        // allocating new asteroid
        Asteroid *newAsteroid = malloc(sizeof(Asteroid));
        if (newAsteroid == NULL) {
            printf("ERROR: Failed to allocate memory for new asteroid.\n");
            exit(1);
        }

        // setting asteroid properties
        int posx = 0, posy = 0;
        bool validRange = false;

        newAsteroid->sizeClass = 3;  // all asteroids are large initially
        while (!validRange) {
            if ((posx > screenWidth / 2 - 150 && posx < screenWidth / 2 + 150) || (fabsf(player.position.x - posx) < 20.f)) {
                // asteroid is too close to screen edge or player
                posx = rand() % screenWidth;
            } else {
                validRange = true;
            }
        }
        validRange = false;
        while (!validRange) {
            if ((posy > screenHeight / 2 - 150 && posy < screenHeight / 2 + 150) || (fabsf(player.position.y - posy) < 20.f)) {
                // asteroid is too close to screen edge or player
                posy = rand() % screenHeight;
            } else {
                validRange = true;
            }
        }
        float randomAngle = (rand() & 360) * DEG2RAD;

        newAsteroid->position = (Vector2){posx, posy};
        newAsteroid->speed = Vector2Scale((Vector2){cosf(randomAngle), sinf(randomAngle)}, ASTEROID_SPEED);
        newAsteroid->radius = AsteroidRadius(newAsteroid->sizeClass + 2);
        newAsteroid->active = true;
        newAsteroid->color = WHITE;

        // adding asteroid to array
        xArray_push(asteroids, (void *)newAsteroid);
    }
}

// calculate distance and delta-rotation to closest asteroid
static Vector2 ClosestAsteroid(void)
{
    float minDistance = screenWidth + screenHeight;
    float deltaRotation = 0;

    for (int i = 0; i < asteroids->size; i++) {
        Asteroid *asteroid = (Asteroid *)xArray_get(asteroids, i);
        if (!asteroid->active)
            continue;

        Vector2 positions[9] = {asteroid->position,
                                (Vector2){asteroid->position.x, asteroid->position.y + screenHeight},
                                (Vector2){asteroid->position.x, asteroid->position.y - screenHeight},
                                (Vector2){asteroid->position.x + screenWidth, asteroid->position.y},
                                (Vector2){asteroid->position.x - screenWidth, asteroid->position.y},
                                (Vector2){asteroid->position.x + screenWidth, asteroid->position.y + screenHeight},
                                (Vector2){asteroid->position.x - screenWidth, asteroid->position.y - screenHeight},
                                (Vector2){asteroid->position.x + screenWidth, asteroid->position.y - screenHeight},
                                (Vector2){asteroid->position.x - screenWidth, asteroid->position.y + screenHeight}};

        for (int j = 0; j < (int)(sizeof(positions) / sizeof(Vector2)); j++) {
            float distance = Vector2Distance(player.position, positions[j]);
            if (distance < minDistance) {
                minDistance = distance;
                deltaRotation = atan2f(positions[j].y - player.position.y, positions[j].x - player.position.x) - player.rotation;

                relativeVelocity = Vector2Subtract(asteroid->speed, player.speed);
            }
        }
    }

    return (Vector2){minDistance, deltaRotation};
}

// initialize game variables
static void InitGame(void)
{
    screenDiagonal = sqrtf(screenWidth * screenWidth + screenHeight * screenHeight);
    gamePaused = false;
    score = 0;
    levelsCleared = 0;

    // initialization of asteroids array
    if ((asteroids = xArray_new()) == NULL) {
        printf("ERROR: Failed to allocate asteroids array.\n");
        exit(1);
    }

    // initialization of shared memory depending on run mode
    if (flags_cmd & CMD_FLAG_USE_NEURAL) {
        OpenSharedMemory();
    }

    // initialization of player
    shipHeight = (PLAYER_BASE_SIZE / 2) / tanf(20 * DEG2RAD);
    player.position = (Vector2){screenWidth / 2, screenHeight / 2 - shipHeight / 2};
    player.speed = (Vector2){0, 0};
    player.acceleration = (Vector2){0, 0};
    player.rotation = -(PI / 2);
    player.collider = (Vector3){player.position.x - sinf(player.rotation) * (shipHeight / 2.5f),
                                player.position.y - sinf(player.rotation) * (shipHeight / 2.5f), 12};
    player.color = WHITE;
    destroyedMeteorsCount = 0;

    // initialization of bullets
    for (int i = 0; i < PLAYER_MAX_BULLETS; i++) {
        bullet[i].position = (Vector2){0, 0};
        bullet[i].speed = (Vector2){0, 0};
        bullet[i].radius = 2;
        bullet[i].active = false;
        bullet[i].lifeSpawn = 0;
        bullet[i].color = WHITE;
    }

    // initialization of asteroids
    PregenAsteroids();

    startTime = currentTime;  // NOTE: added to avoid game time not being reset after game restart
}

// update logic (one time step)
static void UpdateGame(void)
{
    // clear input
    flags_input &= INPUT_NONE;

    // update input flags (depending on run mode)
    if ((flags_cmd & CMD_FLAG_USE_NEURAL) && !gameOver) {
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
        if (flags_input & INPUT_PAUSE)
            gamePaused = !gamePaused;

        if (!gamePaused) {
            // Player logic: rotation
            if (flags_input & INPUT_A)
                player.rotation -= PLAYER_BASE_ROTATION * fixedTimeStep;
            if (flags_input & INPUT_D)
                player.rotation += PLAYER_BASE_ROTATION * fixedTimeStep;
            if (player.rotation > PI) {
                player.rotation -= 2 * M_PI;
            } else if (player.rotation < -PI) {
                player.rotation += 2 * PI;
            }

            // Player logic: acceleration
            if (flags_input & INPUT_W) {
                player.acceleration =
                    Vector2Scale((Vector2){cosf(player.rotation), sinf(player.rotation)}, PLAYER_BASE_ACCELERATION);
            } else {
                // decelerate to 0.99f of current speed
                player.acceleration = Vector2Scale(player.speed, -0.01f / fixedTimeStep);
            }

            // Player logic: speed
            player.speed = Vector2Add(player.speed, Vector2Scale(player.acceleration, fixedTimeStep));
            if (Vector2Length(player.speed) > PLAYER_MAX_SPEED) {
                player.speed = Vector2Scale(player.speed, PLAYER_MAX_SPEED / Vector2Length(player.speed));
            }

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

            // Player bullet cooldown logic
            if (fireCooldown > 0.0f)
                fireCooldown -= 1.0f * fixedTimeStep;

            // Player bullet logic
            if (flags_input & INPUT_SPACE && fireCooldown <= 0.0f) {
                for (int i = 0; i < PLAYER_MAX_BULLETS; i++) {
                    if (!bullet[i].active) {
                        bullet[i].position = (Vector2){player.position.x + cosf(player.rotation) * (shipHeight),
                                                       player.position.y + sinf(player.rotation) * (shipHeight)};
                        bullet[i].active = true;
                        bullet[i].speed = Vector2Scale((Vector2){cosf(player.rotation), sinf(player.rotation)}, BULLET_SPEED);
                        bullet[i].rotation = player.rotation;
                        fireCooldown = FIRE_COOLDOWN;
                        break;
                    }
                }
            }

            // Bullet life timer
            for (int i = 0; i < PLAYER_MAX_BULLETS; i++) {
                if (bullet[i].active)
                    bullet[i].lifeSpawn++;
            }

            // bullet logic
            for (int i = 0; i < PLAYER_MAX_BULLETS; i++) {
                if (bullet[i].active) {
                    // bullet movement
                    bullet[i].position = Vector2Add(bullet[i].position, Vector2Scale(bullet[i].speed, fixedTimeStep));

                    // collision logic: bullet vs walls
                    if (bullet[i].position.x > screenWidth) {
                        bullet[i].position.x = 0;
                    } else if (bullet[i].position.x < 0) {
                        bullet[i].position.x = screenWidth;
                    }
                    if (bullet[i].position.y > screenHeight) {
                        bullet[i].position.y = 0;
                    } else if (bullet[i].position.y < 0) {
                        bullet[i].position.y = screenHeight;
                    }

                    // bullet lifetime
                    if (bullet[i].lifeSpawn >= BULLET_LIFETIME) {
                        bullet[i].position = (Vector2){0, 0};
                        bullet[i].speed = (Vector2){0, 0};
                        bullet[i].lifeSpawn = 0;
                        bullet[i].active = false;
                        wastedBulletsCount++;
                    }
                }
            }

            // asteroid logic
            player.collider = (Vector3){player.position.x + cosf(player.rotation) * (shipHeight / 2.5f),
                                        player.position.y + sinf(player.rotation) * (shipHeight / 2.5f), 12};
            for (int i = 0; i < asteroids->size; i++) {
                Asteroid *asteroid = (Asteroid *)xArray_get(asteroids, i);
                if (!asteroid->active)
                    continue;

                // collision logic: player vs asteroids
                if (CheckCollisionCircles((Vector2){player.collider.x, player.collider.y}, player.collider.z, asteroid->position,
                                          asteroid->radius)) {
                    gameOver = true;
                    break;
                }

                // asteroid logic: movement
                asteroid->position = Vector2Add(asteroid->position, Vector2Scale(asteroid->speed, fixedTimeStep));

                // collision logic: asteroid vs walls
                if (asteroid->position.x > screenWidth) {
                    asteroid->position.x = 0;
                } else if (asteroid->position.x < 0) {
                    asteroid->position.x = screenWidth;
                }
                if (asteroid->position.y > screenHeight) {
                    asteroid->position.y = 0;
                } else if (asteroid->position.y < 0) {
                    asteroid->position.y = screenHeight;
                }
            }

            // collision logic: bullets vs asteroids
            for (int i = 0; i < PLAYER_MAX_BULLETS; i++) {
                if (!bullet[i].active)
                    continue;

                for (int j = 0; j < asteroids->size; j++) {
                    Asteroid *asteroid = (Asteroid *)xArray_get(asteroids, j);
                    if (!asteroid->active)
                        continue;

                    if (CheckCollisionCircles(bullet[i].position, bullet[i].radius, asteroid->position, asteroid->radius)) {
                        bullet[i].active = false;
                        bullet[i].lifeSpawn = 0;
                        asteroid->active = false;
                        score += (asteroid->sizeClass == 3) ? 25 : (asteroid->sizeClass == 2) ? 50 : 100;
                        destroyedMeteorsCount++;

                        // spawn smaller asteroids
                        if (asteroid->sizeClass > 1) {
                            for (int k = 0; k < 2; k++) {
                                // allocating new asteroid
                                Asteroid *newAsteroid = malloc(sizeof(Asteroid));
                                if (newAsteroid == NULL) {
                                    printf("ERROR: Failed to allocate memory for new asteroid.\n");
                                    exit(1);
                                }

                                // setting asteroid properties
                                newAsteroid->sizeClass = asteroid->sizeClass - 1;
                                newAsteroid->position = (Vector2){asteroid->position.x, asteroid->position.y};
                                // set new asteroid speeds to be normal to line between collided bullet and asteroid
                                newAsteroid->speed = Vector2Scale(
                                    Vector2Rotate(Vector2Normalize(Vector2Subtract(asteroid->position, bullet[i].position)),
                                                  90 * DEG2RAD),
                                    ASTEROID_SPEED * (k == 0 ? -(4 - newAsteroid->sizeClass) : (4 - newAsteroid->sizeClass)));
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

            // calculate distance and delta-rotation to closest asteroid
            closestAsteroid = ClosestAsteroid();
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

    // kill neural network process if game is managing it and game is over
    if (flags_cmd & CMD_FLAG_STANDALONE && flags_cmd & CMD_FLAG_USE_NEURAL && gameOver && pid_neurons > 0) {
        kill(pid_neurons, SIGTERM);
        pid_neurons = -1;
    }

    // update output and state IPC (depending on run mode)
    UpdateSharedOutput();
    UpdateSharedState();
}

// draw game (one frame)
static void DrawGame(void)
{
    BeginDrawing();

    ClearBackground(BLACK);

    if (!gameOver) {
        // Draw spaceship
        Vector2 v1 = {player.position.x + cosf(player.rotation) * (shipHeight),
                      player.position.y + sinf(player.rotation) * (shipHeight)};
        Vector2 v2 = {player.position.x + sinf(player.rotation) * (PLAYER_BASE_SIZE / 2),
                      player.position.y - cosf(player.rotation) * (PLAYER_BASE_SIZE / 2)};
        Vector2 v3 = {player.position.x - sinf(player.rotation) * (PLAYER_BASE_SIZE / 2),
                      player.position.y + cosf(player.rotation) * (PLAYER_BASE_SIZE / 2)};
        DrawTriangleLines(v1, v2, v3, player.color);

        // Draw asteroids
        for (int i = 0; i < asteroids->size; i++) {
            Asteroid *asteroid = (Asteroid *)xArray_get(asteroids, i);
            if (asteroid->active) {
                DrawCircleLines(asteroid->position.x, asteroid->position.y, asteroid->radius, asteroid->color);
                // DrawCircleV(asteroid->position, asteroid->radius, asteroid->color);
            } else {
                DrawCircleV(asteroid->position, asteroid->radius, Fade(DARKGRAY, 0.3f));
            }
        }

        // Draw bullet
        for (int i = 0; i < PLAYER_MAX_BULLETS; i++) {
            if (bullet[i].active)
                DrawCircleV(bullet[i].position, bullet[i].radius, bullet[i].color);
        }

        // DEBUG: Drawing colliders, line to closest asteroid, etc.
        if (flags_cmd & CMD_FLAG_USE_NEURAL) {
            DrawCircleLines(player.collider.x, player.collider.y, player.collider.z, GREEN);
            DrawCircleV(player.position, 5, BLUE);
            DrawCircle(player.position.x + closestAsteroid.x * cosf(closestAsteroid.y + player.rotation),
                       player.position.y + closestAsteroid.x * sinf(closestAsteroid.y + player.rotation), 5, RED);
            DrawLineEx(player.position,
                       Vector2Add(player.position, (Vector2){closestAsteroid.x * cosf(closestAsteroid.y + player.rotation),
                                                             closestAsteroid.x * sinf(closestAsteroid.y + player.rotation)}),
                       2, RED);
            DrawLineEx(player.position,
                       Vector2Add(player.position, (Vector2){cosf(player.rotation) * 200, sinf(player.rotation) * 200}), 2, GREEN);
        }

        // Draw status (score, levels cleared, time survived)
        DrawText(TextFormat("SCORE: %04i", score), 20, 20, 20, WHITE);
        DrawText(TextFormat("LEVEL: %02i", levelsCleared + 1), 20, 40, 20, WHITE);
        DrawText(TextFormat("TIME: %02i:%02i", (int)(currentTime.tv_sec - startTime.tv_sec) / 60,
                            (int)(currentTime.tv_sec - startTime.tv_sec) % 60),
                 20, 60, 20, WHITE);

        if (flags_cmd & CMD_FLAG_USE_NEURAL) {
            // DEBUG: text status on right side of screen (input and output states for neural network)
            DrawText(TextFormat("INPUT_01: %01i", ((flags_input & INPUT_W) > 0)), screenWidth - 250, 20, 20, WHITE);
            DrawText(TextFormat("INPUT_02: %01i", ((flags_input & INPUT_A) > 0)), screenWidth - 250, 40, 20, WHITE);
            DrawText(TextFormat("INPUT_03: %01i", ((flags_input & INPUT_D) > 0)), screenWidth - 250, 60, 20, WHITE);
            DrawText(TextFormat("INPUT_04: %01i", ((flags_input & INPUT_SPACE) > 0)), screenWidth - 250, 80, 20, WHITE);
            DrawText(TextFormat("OUTPUT_01: %.4f", player.rotation / PI), screenWidth - 250, 120, 20, WHITE);
            DrawText(TextFormat("OUTPUT_02: %.4f", relativeVelocity.x / (ASTEROID_SPEED + PLAYER_MAX_SPEED)), screenWidth - 250,
                     140, 20, WHITE);
            DrawText(TextFormat("OUTPUT_03: %.4f", relativeVelocity.y / (ASTEROID_SPEED + PLAYER_MAX_SPEED)), screenWidth - 250,
                     160, 20, WHITE);
            DrawText(TextFormat("OUTPUT_04: %.4f", closestAsteroid.x / screenDiagonal), screenWidth - 250, 180, 20, WHITE);
            DrawText(TextFormat("OUTPUT_05: %.4f", closestAsteroid.y / PI), screenWidth - 250, 200, 20, WHITE);

            // DEBUG: text status on right side of screen (additional game information for fitness function)
            DrawText(TextFormat("WASTED BULLETS: %04i", wastedBulletsCount), screenWidth - 250, 240, 20, WHITE);
        }

        if (gamePaused)
            DrawText("GAME PAUSED", screenWidth / 2 - MeasureText("GAME PAUSED", 40) / 2, screenHeight / 2 - 40, 40, WHITE);
    } else {
        DrawText("GAME OVER", GetScreenWidth() / 2 - MeasureText("GAME OVER", 20) / 2, GetScreenHeight() / 2 - 50, 20, WHITE);
        if (flags_runtime & RUNTIME_WINDOW_ACTIVE && !(flags_cmd & CMD_FLAG_USE_NEURAL || flags_cmd & CMD_FLAG_MANAGED)) {
            DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth() / 2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20) / 2,
                     GetScreenHeight() / 2 - 10, 20, WHITE);
        }
    }

    EndDrawing();
}

// unload game variables
static void UnloadGame(void)
{
    // close shared memory (if any)
    if (flags_cmd & CMD_FLAG_USE_NEURAL) {
        CloseSharedMemory();
    }

    // clear all dynamic structures
    xArray_clear(asteroids);
    xArray_free(asteroids);
}
