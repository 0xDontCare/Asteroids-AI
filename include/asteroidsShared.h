/**
 * @file asteroidsCommon.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Shared memory module for asteroids game
 * @version 0.1
 * @date 21.10.2023.
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef ASTEROIDS_SHARED_H
#define ASTEROIDS_SHARED_H

#ifdef __cplusplus
extern "C" {
#endif

extern struct game_s;

#include <pthread.h>
#include <sys/mman.h>
#include <stdio.h>

struct sharedMemory_s {
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

struct sharedMemory_s *allocateSharedMemory() {
    struct sharedMemory_s *sharedMemory = (struct sharedMemory_s *)mmap(NULL, sizeof(struct sharedMemory_s), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sharedMemory == NULL) {
        perror("Failed to allocate shared memory");
        exit(1);
    } else {
        return sharedMemory;
    }
}

void initSharedMemory(struct sharedMemory_s *sharedMemory) {
    pthread_mutexattr_t mutexAttribute;
    pthread_mutexattr_init(&mutexAttribute);
    pthread_mutexattr_setpshared(&mutexAttribute, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&sharedMemory->mutex, &mutexAttribute);
    pthread_mutexattr_destroy(&mutexAttribute);
    sharedMemory->playerPosX = 0.f;
    sharedMemory->playerPosY = 0.f;
    sharedMemory->playerRotation = 0.f;
    sharedMemory->playerSpeedX = 0.f;
    sharedMemory->playerSpeedY = 0.f;
    sharedMemory->distanceFront = 0.f;
    sharedMemory->closestAsteroidPosX = 0.f;
    sharedMemory->closestAsteroidPosY = 0.f;
}

void destroySharedMemory(struct sharedMemory_s *sharedMemory) {
    pthread_mutex_destroy(&sharedMemory->mutex);
    munmap(sharedMemory, sizeof(struct sharedMemory_s));
}

inline void lockSharedMemory(struct sharedMemory_s *sharedMemory) {
    pthread_mutex_lock(&sharedMemory->mutex);
}

inline void unlockSharedMemory(struct sharedMemory_s *sharedMemory) {
    pthread_mutex_unlock(&sharedMemory->mutex);
}

void updateSharedMemory(struct sharedMemory_s *sharedMemory, struct game_s *gameData) {
    lockSharedMemory(sharedMemory);
    sharedMemory->playerPosX = gameData->player->physicsComponent.position.x;
    sharedMemory->playerPosY = gameData->player->physicsComponent.position.y;
    sharedMemory->playerRotation = gameData->player->physicsComponent.rotation;
    sharedMemory->playerSpeedX = gameData->player->physicsComponent.speed.x;
    sharedMemory->playerSpeedY = gameData->player->physicsComponent.speed.y;
    sharedMemory->distanceFront = gameData->distanceFront;
    sharedMemory->closestAsteroidPosX = gameData->closestAsteroidPosX;
    sharedMemory->closestAsteroidPosY = gameData->closestAsteroidPosY;
    unlockSharedMemory(sharedMemory);
}

#ifdef __cplusplus
}
#endif

#endif  // ASTEROIDS_SHARED_H
