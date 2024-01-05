/**
 * @file main.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Game related enums, structs, etc.
 * @version 0.1
 * @date 11.12.2023.
 *
 * @copyright All rights reserved (c) 2023
 *
 */

#ifndef MAIN_H
#define MAIN_H

#include <raylib.h>

// ------------------------------------------------------------------
// game enum definitions

/* Command line flags:
 * 0x01 - help
 * 0x02 - version
 * 0x04 - standalone mode
 * 0x08 - headless mode
 * 0x10 - use neural network (+2 parameters)
 * 0x20 - managed mode (+3 parameters)
 */
enum cmdFlag_e {
    CMD_FLAG_NONE = 0x00,
    CMD_FLAG_HELP = 0x01,
    CMD_FLAG_VERSION = 0x02,
    CMD_FLAG_STANDALONE = 0x04,
    CMD_FLAG_HEADLESS = 0x08,
    CMD_FLAG_USE_NEURAL = 0x10,
    CMD_FLAG_MANAGED = 0x20,
};

/* Control input flags
 * 0x01 - W
 * 0x02 - A
 * 0x04 - D
 * 0x08 - SPACE
 * 0x10 - EXIT
 */
enum input_e {
    INPUT_NONE = 0x00,
    INPUT_W = 0x01,
    INPUT_A = 0x02,
    INPUT_D = 0x04,
    INPUT_SPACE = 0x08,
    INPUT_PAUSE = 0x10,
    INPUT_ENTER = 0x20,
    INPUT_EXIT = 0x40
};

/* Game runtime flags
 * 0x01 - running
 * 0x02 - paused
 * 0x04 - exit
 * 0x08 - window active
 */
enum gameRuntime_e {
    RUNTIME_NONE = 0x00,
    RUNTIME_RUNNING = 0x01,
    RUNTIME_PAUSED = 0x02,
    RUNTIME_EXIT = 0x04,
    RUNTIME_WINDOW_ACTIVE = 0x08
};

// ------------------------------------------------------------------
// game struct definitions

// player game object descriptor
typedef struct player_s {
    Vector2 position;      // center position
    Vector2 speed;         // velocity
    Vector2 acceleration;  // acceleration
    float rotation;        // rotation
    Vector3 collider;      // triangle collider info
    Color color;           // render color
} Player;
// typedef struct player_s {
//     Vector2 position;
//     Vector2 velocity;
//     float acceleration;
//     float rotation;
//     float rotationSpeed;
//     Vector3 collider;
//     Color color;
// } Player;

// bullet game object descriptor
typedef struct bullet_s {
    Vector2 position;  // center position
    Vector2 speed;     // velocity
    float radius;      // collision radius
    float rotation;    // rotation
    int lifeSpawn;     // lifespan in frames
    bool active;       // is active (not destroyed/dead)
    Color color;       // render color
} Shoot;
// typedef struct bullet_s {
//     Vector2 position;
//     Vector2 velocity;
//     float radius;
//     int lifeTime;
//     bool byPlayer; // for potentially adding enemy ships at some point
//     Color color;
// } Bullet;

// asteroid game object descriptor
typedef struct asteroid_s {
    int sizeClass;     // 1 - small, 2 - medium, 3 - large
    Vector2 position;  // center position
    Vector2 speed;     // velocity
    float radius;      // collision radius
    bool active;       // is active (not destroyed)
    Color color;       // render color
} Meteor;
// typedef struct asteroid_s {
//     Vector2 position;
//     Vector2 velocity;
//     float radius;
//     bool active;
//     Color color;
// } Asteroid;

// game constant definitions
#define PLAYER_BASE_SIZE 20.0f           // player base size in pixels
#define PLAYER_MAX_BULLETS 10            // maximum number of bullets on screen
#define PLAYER_BASE_ACCELERATION 500.0f  // acceleration in pixels per second^2
#define PLAYER_BASE_ROTATION 5.f         // rotation speed in radians
#define PLAYER_SPEED 6.0f
#define PLAYER_MAX_SHOOTS 10

#define ASTEROID_SPEED 100                // asteroid base speed in pixels per second
#define ASTEROID_BASE_GENERATION_COUNT 4  // number of asteroids generated at game start
#define ASTEROID_BASE_SIZE 20.0f          // asteroid base size in pixels
#define ASTEROID_BASE_SPEED 100.0f        // asteroid max speed in pixels per second

#define METEORS_SPEED 50
#define MAX_BIG_METEORS 4
#define MAX_MEDIUM_METEORS 8
#define MAX_SMALL_METEORS 16

#define BULLET_LIFETIME 80   // bullet lifespan in frames
#define BULLET_SPEED 500.0f  // bullet speed in pixels per second
#define FIRE_COOLDOWN 0.15f  // time between shots in seconds

// ------------------------------------------------------------------
// game function declarations
Player* playerInit(void);
void playerFree(Player* player);

#endif  // MAIN_H