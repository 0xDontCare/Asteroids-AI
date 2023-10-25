#include "asteroidsShared.h"

#include <pthread.h>
#include <sys/mman.h>

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