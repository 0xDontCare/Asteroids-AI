/**
 * @file asteroidsShared.c
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Implementations of functions declared in `asteroidsShared.h` header.
 * @version 0.2
 * @date 12.11.2023.
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "asteroidsShared.h"

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct sharedInput_s *as_allocateSharedInput(const char *sharedMemoryName) {
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

struct sharedInput_s *as_connectSharedInput(const char *sharedMemoryName) {
    int sharedMemoryFd = shm_open(sharedMemoryName, O_RDWR, 0666);
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

void as_initSharedInput(struct sharedInput_s *sharedInput) {
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

void as_freeSharedInput(struct sharedInput_s *sharedInput, const char *sharedMemoryName) {
    pthread_mutex_destroy(&sharedInput->mutex);

    if (munmap(sharedInput, sizeof(struct sharedInput_s)) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    if (shm_unlink(sharedMemoryName) == -1) {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }
}

void as_disconnectSharedInput(struct sharedInput_s *sharedInput) {
    if (munmap(sharedInput, sizeof(struct sharedInput_s)) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
}

void as_lockSharedInput(struct sharedInput_s *sharedInput) {
    pthread_mutex_lock(&sharedInput->mutex);
}

void as_unlockSharedInput(struct sharedInput_s *sharedInput) {
    pthread_mutex_unlock(&sharedInput->mutex);
}

struct sharedOutput_s *as_allocateSharedOutput(const char *sharedMemoryName) {
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

struct sharedOutput_s *as_connectSharedOutput(const char *sharedMemoryName) {
    int sharedMemoryFd = shm_open(sharedMemoryName, O_RDWR, 0666);
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

void as_initSharedOutput(struct sharedOutput_s *sharedOutput) {
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

void as_freeSharedOutput(struct sharedOutput_s *sharedOutput, const char *sharedMemoryName) {
    pthread_mutex_destroy(&sharedOutput->mutex);

    if (munmap(sharedOutput, sizeof(struct sharedOutput_s)) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    if (shm_unlink(sharedMemoryName) == -1) {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }
}

void as_disconnectSharedOutput(struct sharedOutput_s *sharedOutput) {
    if (munmap(sharedOutput, sizeof(struct sharedOutput_s)) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
}

void as_lockSharedOutput(struct sharedOutput_s *sharedOutput) {
    pthread_mutex_lock(&sharedOutput->mutex);
}

void as_unlockSharedOutput(struct sharedOutput_s *sharedOutput) {
    pthread_mutex_unlock(&sharedOutput->mutex);
}

struct sharedState_s *as_allocateSharedState(const char *sharedMemoryName) {
    int sharedMemoryFd = shm_open(sharedMemoryName, O_CREAT | O_RDWR, 0666);
    if (sharedMemoryFd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(sharedMemoryFd, sizeof(struct sharedState_s)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    struct sharedState_s *sharedState = mmap(NULL, sizeof(struct sharedState_s), PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemoryFd, 0);
    if (sharedState == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    return sharedState;
}

struct sharedState_s *as_connectSharedState(const char *sharedMemoryName) {
    int sharedMemoryFd = shm_open(sharedMemoryName, O_RDWR, 0666);
    if (sharedMemoryFd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(sharedMemoryFd, sizeof(struct sharedState_s)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    struct sharedState_s *sharedState = mmap(NULL, sizeof(struct sharedState_s), PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemoryFd, 0);
    if (sharedState == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    return sharedState;
}

void as_freeSharedState(struct sharedState_s *sharedState, const char *sharedMemoryName) {
    pthread_mutex_destroy(&sharedState->mutex);

    if (munmap(sharedState, sizeof(struct sharedState_s)) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    if (shm_unlink(sharedMemoryName) == -1) {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }
}

void as_disconnectSharedState(struct sharedState_s *sharedState) {
    if (munmap(sharedState, sizeof(struct sharedState_s)) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
}

void as_lockSharedState(struct sharedState_s *sharedState) {
    pthread_mutex_lock(&sharedState->mutex);
}

void as_unlockSharedState(struct sharedState_s *sharedState) {
    pthread_mutex_unlock(&sharedState->mutex);
}
