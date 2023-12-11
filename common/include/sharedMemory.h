/**
 * @file sharedMemory.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Shared memory structures and functions for interprocess communication between game, manager and neural network programs.
 * @version 0.2
 * @date 11.12.2023.
 *
 * @copyright Copyright (c) 2023
 *
 * Module declares structures and functions used for sharing data between game, manager and neural network programs.
 * All functions have prefix `sm_`.
 *
 */

#ifndef ASTEROIDS_SHARED_H
#define ASTEROIDS_SHARED_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <pthread.h>

struct sharedInput_s {
    pthread_mutex_t mutex;
    int isKeyDownW;
    int isKeyDownA;
    int isKeyDownD;
    int isKeyDownSpace;
};

struct sharedOutput_s {
    pthread_mutex_t mutex;
    float playerPosX;
    float playerPosY;
    float playerRotation;
    float playerSpeedX;
    float playerSpeedY;
    float distanceFront;
    float closestAsteroidPosX;
    float closestAsteroidPosY;
};

struct sharedState_s {
    pthread_mutex_t mutex;  // access mutex for shared state

    int state_gameAlive;     // status if game program is running (activated by game on start)
    int state_managerAlive;  // status if manager program is running (activated by manager on start)
    int state_neuronsAlive;  // status if neural network program is running (activated by NN program on start)

    int game_isPaused;     // status if game is paused (modified by game)
    int game_runHeadless;  // status if game is running headless (modified by manager)
    int game_gameScore;    // current game score (modified by game)
    int game_gameLevel;    // current game level (modified by game)
    double game_gameTime;  // current game time  (modified by game)
};

/**
 * @brief Allocate or connect to shared memory for input.
 *
 * @param sharedMemoryName Name of shared memory to allocate or connect to.
 * @return Pointer to shared memory structure.
 */
struct sharedInput_s *sm_allocateSharedInput(const char *sharedMemoryName);

/**
 * @brief Connect to already existing shared memory with given key string.
 *
 * @note This function tries to connect to already existing shared memory. It will not create it if it doesn't exist.
 *
 * @param sharedMemoryName Pointer to C string containing shared memory key
 * @return Pointer to shared memory structure if success, NULL if fail
 */
struct sharedInput_s *sm_connectSharedInput(const char *sharedMemoryName);

/**
 * @brief Initialize shared memory structure to default values.
 *
 * @param sharedInput Pointer to shared memory structure.
 */
void sm_initSharedInput(struct sharedInput_s *sharedInput);

/**
 * @brief Destroy shared memory structure.
 *
 * @warning This function destroys shared memory for all processes using it.
 *
 * @param sharedInput Pointer to shared memory structure.
 * @param sharedMemoryName Name of shared memory to destroy.
 */
void sm_freeSharedInput(struct sharedInput_s *sharedInput, const char *sharedMemoryName);

/**
 * @brief Unload shared memory structure.
 *
 * @note This function only unloads shared memory for current process. Other processes using it will not be affected.
 *
 * @param sharedInput Pointer to shared memory structure.
 */
void sm_disconnectSharedInput(struct sharedInput_s *sharedInput);

/**
 * @brief Lock shared memory structure.
 *
 * @note Should be used before performing any actions on shared memory structure.
 *
 * @warning Make sure to unlock shared memory structure after performing any actions on it to avoid deadlocks.
 *
 * @param sharedInput Pointer to shared memory structure.
 */
void sm_lockSharedInput(struct sharedInput_s *sharedInput);

/**
 * @brief Unlock shared memory structure.
 *
 * @note Should be used after locking shared memory structure.
 *
 * @param sharedInput Pointer to shared memory structure.
 */
void sm_unlockSharedInput(struct sharedInput_s *sharedInput);

/**
 * @brief Allocate or connect to shared memory for output.
 *
 * @param sharedMemoryName Name of shared memory to allocate or connect to.
 * @return Pointer to shared memory structure.
 */
struct sharedOutput_s *sm_allocateSharedOutput(const char *sharedMemoryName);

/**
 * @brief Connect to existing shared output structure.
 *
 * @param sharedMemoryName Shared memory access string.
 * @return Pointer to shared output structure.
 */
struct sharedOutput_s *sm_connectSharedOutput(const char *sharedMemoryName);

/**
 * @brief Initialize shared memory structure to default values
 *
 * @param sharedOutput Pointer to shared memory structure.
 */
void sm_initSharedOutput(struct sharedOutput_s *sharedOutput);

/**
 * @brief Destroy shared memory structure
 *
 * @param sharedOutput Pointer to shared memory.
 * @param sharedMemoryName Shared memory access string.
 */
void sm_freeSharedOutput(struct sharedOutput_s *sharedOutput, const char *sharedMemoryName);

/**
 * @brief Unload shared memory from current program.
 *
 * @param sharedOutput Pointer to shared memory structure.
 */
void sm_disconnectSharedOutput(struct sharedOutput_s *sharedOutput);

/**
 * @brief Lock shared memory structure.
 *
 * @note Should be used before performing any actions on shared memory.
 *
 * @param sharedOutput Pointer to shared memory structure.
 */
void sm_lockSharedOutput(struct sharedOutput_s *sharedOutput);

/**
 * @brief Unlock access to shared memory structure.
 *
 * @param sharedOutput Pointer to shared memory structure.
 */
void sm_unlockSharedOutput(struct sharedOutput_s *sharedOutput);

/**
 * @brief Allocate or connect to shared memory for shared state.
 *
 * @param sharedMemoryName Name of shared memory to allocate or connect to.
 * @return Pointer to shared memory structure.
 */
struct sharedState_s *sm_allocateSharedState(const char *sharedMemoryName);

/**
 * @brief Connect to existing shared state structure.
 *
 * @param sharedMemoryName Shared memory access string.
 * @return Pointer to shared state structure.
 */
struct sharedState_s *sm_connectSharedState(const char *sharedMemoryName);

/**
 * @brief Destroy shared memory structure.
 *
 * @param sharedState Pointer to shared memory structure.
 * @param sharedMemoryName Shared memory access string.
 */
void sm_freeSharedState(struct sharedState_s *sharedState, const char *sharedMemoryName);

/**
 * @brief Unload shared memory from current program.
 *
 * @param sharedState Pointer to shared memory structure.
 */
void sm_disconnectSharedState(struct sharedState_s *sharedState);

/**
 * @brief Lock shared memory structure.
 *
 * @param sharedState Pointer to shared memory structure.
 */
void sm_lockSharedState(struct sharedState_s *sharedState);

/**
 * @brief Unlock access to shared memory structure.
 *
 * @param sharedState Pointer to shared memory structure.
 */
void sm_unlockSharedState(struct sharedState_s *sharedState);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // ASTEROIDS_SHARED_H
