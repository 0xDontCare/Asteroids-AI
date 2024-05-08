/**
 * @file managerMain.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Manager program related enums, structs, etc.
 * @version 0.3
 * @date 08.05.2024.
 *
 * @copyright All rights reserved (c) 2024
 *
 */

#ifndef MANMAIN_H
#define MANMAIN_H

#include <inttypes.h>
#include <unistd.h>

#define ASTROMGR_VER "0.2"

#define FITNESS_WEIGHT_SCORE 0.5f
#define FITNESS_WEIGHT_TIME 0.2f
#define FITNESS_WEIGHT_LEVEL 0.3f

enum instanceStatus_e {
    INSTANCE_INACTIVE = 0x00,
    INSTANCE_WAITING = 0x01,
    INSTANCE_RUNNING = 0x02,
    INSTANCE_FINISHED = 0x04,
    INSTANCE_ERRORED = 0x08,
    INSTANCE_ENDED = 0x10,
    INSTANCE_ERRENDED = 0x20
};

// instance descriptor which holds all the necessary information about one instance (game-AI pair)
typedef struct {
    uint32_t instanceID;                   // unique instance ID
    enum instanceStatus_e instanceStatus;  // status of the instance

    pid_t gamePID;  // game process ID
    pid_t aiPID;    // AI process ID

    uint32_t sharedMemoryID;  // shared memory ID
    char shmemInput[255];     // input shared memory key
    char shmemOutput[255];    // output shared memory key
    char shmemStatus[255];    // status shared memory key

    char *modelPath;      // path to the model file
    uint32_t generation;  // generation number
    float fitnessScore;   // fitness score
} managerInstance_t;

#endif /* MANMAIN_H */
