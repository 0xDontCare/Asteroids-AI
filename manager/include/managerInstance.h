#ifndef MANINSTANCE_H
#define MANINSTANCE_H

#include <inttypes.h>  // standard integer types (for fixed size integers)
#include <unistd.h>    // standard symbolic constants and types (for POSIX OS API)
#include "xArray.h"    // dynamic array structure

#define FITNESS_WEIGHT_SCORE 0.5f
#define FITNESS_WEIGHT_TIME 0.2f
#define FITNESS_WEIGHT_LEVEL 0.3f

#define BREED_CROSSOVER_INDEX 2.0f
#define BREED_MUTATION_RATE 0.1f
#define BREED_MUTATION_STDDEV 0.1f

#define AUTOKILL_TIMEOUT 20  // timeout in seconds before killing instance if no score update happens

enum instanceStatus_e {
    INSTANCE_INACTIVE = 0x00,
    INSTANCE_WAITING = 0x01,
    INSTANCE_RUNNING = 0x02,
    INSTANCE_FINISHED = 0x04,
    INSTANCE_ERRORED = 0x08,
    INSTANCE_ENDED = 0x10,
    INSTANCE_ERRENDED = 0x20
};

/**
 * @brief Manager instance descriptor
 */
typedef struct {
    uint32_t instanceID;           // unique instance ID
    enum instanceStatus_e status;  // status of the instance

    pid_t gamePID;  // game process ID
    pid_t aiPID;    // AI process ID

    int scoreUpdateValue;    // last updated score value
    long scoreUpdateTime;  // time of updating score

    // uint32_t sharedMemoryID;  // shared memory ID
    char shmemInput[255];   // input shared memory key
    char shmemOutput[255];  // output shared memory key
    char shmemStatus[255];  // status shared memory key

    char *modelPath;      // path to the model file
    uint32_t generation;  // generation number
    float fitnessScore;   // fitness score
    uint32_t currSeed;    // index of currently used seed of generation
} managerInstance_t;

/**
 * @brief Initialize instance manager module
 *
 * @return 0 on success, 1 on failure
 */
int32_t mInstancer_init(void);

/**
 * @brief Free all resources allocated by the instance manager module
 *
 */
void mInstancer_cleanup(void);

/**
 * @brief Load latest generation of given population directory
 *
 * @param modelPath Path to the population directory
 * @return 0 on success, 1 on failure
 *
 * @note Generation subdirectories are named as "genX" where X is the generation number starting from 0 (initial generation)
 */
int32_t mInstancer_loadPopulation(const char *modelPath);

/**
 * @brief Start population of instances
 *
 * @return 0 on success, 1 on failure
 *
 * @note For this function to work, population must be loaded first
 *
 * @note This function creates new thread which will manage running loaded instances until all instances are finished or stopped
 */
int32_t mInstancer_startPopulation(void);

/**
 * @brief Stop running population of instances and kill worker thread
 *
 * @return 0 on success, 1 on failure
 */
int32_t mInstancer_stopPopulation(void);

/**
 * @brief Stop running population of instances
 *
 * @return 0 on success, 1 on failure
 *
 * @note This function stops thread responsible for managing running instances and kills any running instances
 */
// int32_t mInstancer_stopPopulation(void);

/**
 * @brief Kill individual instance
 *
 * @param instanceID Instance ID to kill
 * @return 0 on success, 1 on failure
 *
 * @note Not to confuse with PID, instance ID is unique identifier of the instance in the manager
 */
int32_t mInstancer_killIndividual(uint32_t instanceID);

/**
 * @brief Toggle headless mode for given instance
 *
 * @param instanceID Instance ID to toggle headless mode
 * @return 0 on success, 1 on failure
 *
 * @note Headless mode can be toggled only on running instances
 */
int32_t mInstancer_toggleHeadless(uint32_t instanceID);

/**
 * @brief Get instance descriptor information for given ID
 *
 * @param instanceID Instance ID to lookup
 * @return Instance descriptor or NULL if not found
 *
 * @note Descriptor is not a copy, it is a reference to the internal descriptor and should not be modified
 */
const managerInstance_t *mInstancer_get(uint32_t instanceID);

/**
 * @brief Get all instance descriptors
 *
 * @return Pointer to xArray structure containing all instance descriptors
 *
 * @note Array is not a copy, it is a reference to the internal array and should not be modified
 */
const xArray *mInstancer_getAll(void);

/**
 * @brief Set maximum number of parallel instances when running population
 *
 * @param value Maximum number of parallel instances (minimally 1)
 */
void mInstancer_setMaxParallel(uint32_t value);

/**
 * @brief Set maximum number of evolution iterations to run
 *
 * @param value Maximum number of iterations (minimally 1)
 */
void mInstancer_setMaxIterations(uint32_t value);

/**
 * @brief Set number of iterations before reseeding game environment for training
 *
 * @param value Number of iterations (if 0, reseeding is disabled)
 */
void mInstancer_setEpochSize(uint32_t value);

/**
 * @brief Set number of elite individuals to keep in the next generation
 *
 * @param value Number of elite individuals (maximally population size - 1)
 */
void mInstancer_setElitismCount(uint32_t value);

/**
 * @brief Set number of seeds to use for training single generation
 *
 * @param value Number of seeds (minimally 1)
 */
void mInstancer_setSeedCount(uint32_t value);

#endif  // MANINSTANCE_H
