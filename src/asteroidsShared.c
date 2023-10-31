#include "asteroidsShared.h"

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct sharedInput_s *allocateSharedInput(const char *sharedMemoryName) {
    int sharedMemoryFd = shm_open(sharedMemoryName, O_CREAT | O_RDWR, 0666);
    if (sharedMemoryFd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(sharedMemoryFd, sizeof(struct sharedInput_s)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    struct sharedInput_s *sharedInput = mmap(NULL, sizeof(struct sharedInput_s), PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemoryFd, 0);
    if (sharedInput == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    return sharedInput;
}

void initSharedInput(struct sharedInput_s *sharedInput) {
    pthread_mutexattr_t mutexAttr;
    pthread_mutexattr_init(&mutexAttr);
    pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&sharedInput->mutex, &mutexAttr);
    pthread_mutexattr_destroy(&mutexAttr);

    sharedInput->isKeyDownW = 0;
    sharedInput->isKeyDownA = 0;
    sharedInput->isKeyDownD = 0;
    sharedInput->isKeyDownSpace = 0;
}

void destroySharedInput(struct sharedInput_s *sharedInput) {
    pthread_mutex_destroy(&sharedInput->mutex);

    if (munmap(sharedInput, sizeof(struct sharedInput_s)) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    if (shm_unlink("sharedInput") == -1) {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }
}

void unloadSharedInput(struct sharedInput_s *sharedInput) {
    if (munmap(sharedInput, sizeof(struct sharedInput_s)) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
}

void lockSharedInput(struct sharedInput_s *sharedInput) {
    pthread_mutex_lock(&sharedInput->mutex);
}

void unlockSharedInput(struct sharedInput_s *sharedInput) {
    pthread_mutex_unlock(&sharedInput->mutex);
}

struct sharedOutput_s *allocateSharedOutput(const char *sharedMemoryName) {
    int sharedMemoryFd = shm_open(sharedMemoryName, O_CREAT | O_RDWR, 0666);
    if (sharedMemoryFd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(sharedMemoryFd, sizeof(struct sharedOutput_s)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    struct sharedOutput_s *sharedOutput = mmap(NULL, sizeof(struct sharedOutput_s), PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemoryFd, 0);
    if (sharedOutput == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    return sharedOutput;
}

void initSharedOutput(struct sharedOutput_s *sharedOutput) {
    pthread_mutexattr_t mutexAttr;
    pthread_mutexattr_init(&mutexAttr);
    pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&sharedOutput->mutex, &mutexAttr);
    pthread_mutexattr_destroy(&mutexAttr);

    sharedOutput->playerPosX = 0;
    sharedOutput->playerPosY = 0;
    sharedOutput->playerRotation = 0;
    sharedOutput->playerSpeedX = 0;
    sharedOutput->playerSpeedY = 0;
    sharedOutput->distanceFront = 0;
    sharedOutput->closestAsteroidPosX = 0;
    sharedOutput->closestAsteroidPosY = 0;
}

void destroySharedOutput(struct sharedOutput_s *sharedOutput) {
    pthread_mutex_destroy(&sharedOutput->mutex);

    if (munmap(sharedOutput, sizeof(struct sharedOutput_s)) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    if (shm_unlink("sharedOutput") == -1) {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }
}

void unloadSharedOutput(struct sharedOutput_s *sharedOutput) {
    if (munmap(sharedOutput, sizeof(struct sharedOutput_s)) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
}

void lockSharedOutput(struct sharedOutput_s *sharedOutput) {
    pthread_mutex_lock(&sharedOutput->mutex);
}

void unlockSharedOutput(struct sharedOutput_s *sharedOutput) {
    pthread_mutex_unlock(&sharedOutput->mutex);
}