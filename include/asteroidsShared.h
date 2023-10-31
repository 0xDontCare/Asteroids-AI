/**
 * @file asteroidsShared.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Shared memory structures and functions for external interaction with Asteroids game.
 * @version 0.1
 * @date 31.10.2023.
 *
 * @copyright Copyright (c) 2023
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

struct sharedInput_s *allocateSharedInput(const char *sharedMemoryName);
void initSharedInput(struct sharedInput_s *sharedInput);
void destroySharedInput(struct sharedInput_s *sharedInput);
void unloadSharedInput(struct sharedInput_s *sharedInput);
void lockSharedInput(struct sharedInput_s *sharedInput);
void unlockSharedInput(struct sharedInput_s *sharedInput);

struct sharedOutput_s *allocateSharedOutput(const char *sharedMemoryName);
void initSharedOutput(struct sharedOutput_s *sharedOutput);
void destroySharedOutput(struct sharedOutput_s *sharedOutput);
void unloadSharedOutput(struct sharedOutput_s *sharedOutput);
void lockSharedOutput(struct sharedOutput_s *sharedOutput);
void unlockSharedOutput(struct sharedOutput_s *sharedOutput);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // ASTEROIDS_SHARED_H