#include "managerInstance.h"

#include <dirent.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "commonUtility.h"    // common utility functions
#include "fnnGenAlgorithm.h"  // feedforward neural network genetic algorithm functions
#include "fnnSerializer.h"    // feedforward neural network model serialization functions
#include "sharedMemory.h"     // shared memory functions
#include "xArray.h"           // dynamic array structure and functions
#include "xDictionary.h"      // dictionary structure and functions
#include "xString.h"          // dynamic string structure and functions

//------------------------------------------------------------------------------------
// program globals

pthread_mutex_t instancerMutex = PTHREAD_MUTEX_INITIALIZER;  // instance manager mutex
static xArray *descriptors = NULL;                           // array of loaded instance descriptors
static xDictionary *shInDict = NULL;                         // dictionary of shared memory input blocks
static xDictionary *shOutDict = NULL;                        // dictionary of shared memory output blocks
static xDictionary *shStatDict = NULL;                       // dictionary of shared memory status blocks

static uint32_t maxParallel = 0;      // maximum number of parallel instances
static uint32_t maxIterations = 0;    // maximum number of iterations
static uint32_t epochIterations = 0;  // number of iterations before updating seed for game randomization
static uint32_t elitismCount = 0;     // number of best instances to keep in next generation
static uint32_t randSeed = 0;         // random seed for starting entire generation under same conditions
static char *populationDir = NULL;    // path to the loaded population directory

static bool instancesRunning = false;  // flag indicating if instances are running
pthread_t thread_instanceStarter;

//------------------------------------------------------------------------------------
// local function declarations

static managerInstance_t *instance_new(char *modelPath);
static void instance_free(managerInstance_t *instance);
static int instance_compare(const managerInstance_t *a, const managerInstance_t *b);
static int instance_start(uint32_t instanceID);
static void instance_writeReport(const xArray *descriptorArray);
static int instance_nextgen(xArray *descriptorArray);
static void *thr_instanceStarter(void *arg);
static int fCopy(const char *src, const char *dest);

//------------------------------------------------------------------------------------
// public function definitions

int32_t mInstancer_init(void)
{
    //(void)instance_compare;  // suppress unused function warning
    //(void)fCopy;             // suppress unused function warning
    srand((unsigned int)time(NULL) ^ (unsigned int)rand());

    if ((descriptors = xArray_new()) == NULL) {
        return 1;
    }
    if ((shInDict = xDictionary_new()) == NULL) {
        xArray_free(descriptors);
        return 1;
    }
    if ((shOutDict = xDictionary_new()) == NULL) {
        xArray_free(descriptors);
        xDictionary_free(shInDict);
        return 1;
    }
    if ((shStatDict = xDictionary_new()) == NULL) {
        xArray_free(descriptors);
        xDictionary_free(shInDict);
        xDictionary_free(shOutDict);
        return 1;
    }
    return 0;
}

void mInstancer_cleanup(void)
{
    // stop worker thread if running
    mInstancer_stopPopulation();

    pthread_mutex_lock(&instancerMutex);

    // free all shared memory blocks and instance descriptors
    for (int i = 0; i < descriptors->size; i++) {
        managerInstance_t *instance = (managerInstance_t *)xArray_get(descriptors, i);

        struct sharedInput_s *shIn = (struct sharedInput_s *)xDictionary_remove(shInDict, cu_CStringHash(instance->shmemInput));
        if (shIn != NULL) {
            sm_freeSharedInput(shIn, instance->shmemInput);
        }
        struct sharedOutput_s *shOut =
            (struct sharedOutput_s *)xDictionary_remove(shOutDict, cu_CStringHash(instance->shmemOutput));
        if (shOut != NULL) {
            sm_freeSharedOutput(shOut, instance->shmemOutput);
        }
        struct sharedState_s *shStat =
            (struct sharedState_s *)xDictionary_remove(shStatDict, cu_CStringHash(instance->shmemStatus));
        if (shStat != NULL) {
            sm_freeSharedState(shStat, instance->shmemStatus);
        }

        instance_free(instance);
    }

    // free all instancer structures
    xArray_free(descriptors);
    xDictionary_free(shInDict);
    xDictionary_free(shOutDict);
    xDictionary_free(shStatDict);

    descriptors = NULL;
    shInDict = NULL;
    shOutDict = NULL;
    shStatDict = NULL;

    pthread_mutex_unlock(&instancerMutex);
}

int32_t mInstancer_loadPopulation(const char *populationPath)
{
    // if there is already population loaded, clear it from structures
    xArray_forEach(descriptors, (void (*)(void *))instance_free);
    descriptors->size = 0;

    // check if given directory exists and is accessible
    struct stat st = {0};
    if (stat(populationPath, &st) == -1) {
        return 1;
    }

    // open directory and search for `genX` subdirectories
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(populationPath)) == NULL) {
        return 1;
    }

    uint32_t genLast = 0;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_type == DT_DIR) {
            // find generation directory with largest value of X in "genX"
            if (cu_CStringStartsWith(ent->d_name, "gen") && cu_CStringIsNumeric(ent->d_name + 3)) {
                uint32_t gen = (uint32_t)cu_CStringToInteger(ent->d_name + 3);
                if (gen > genLast) {
                    genLast = gen;
                }
            }
        }
    }
    closedir(dir);

    // open last generation directory
    char *genPath = (char *)malloc((cu_CStringLength(populationPath) + 15) * sizeof(char));
    if (genPath == NULL) {
        return 1;
    }
    sprintf(genPath, "%s/gen%u", populationPath, genLast);
    if ((dir = opendir(genPath)) == NULL) {
        free(genPath);
        return 1;
    }

    // load models
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_type != DT_REG || !cu_CStringStartsWith(ent->d_name, "model_") || !cu_CStringEndsWith(ent->d_name, ".fnnm"))
            continue;

        // construct path to model file
        char *modelPath = (char *)malloc((cu_CStringLength(genPath) + cu_CStringLength(ent->d_name) + 2) * sizeof(char));
        if (modelPath == NULL) {
            closedir(dir);
            free(genPath);
            return 1;
        }
        sprintf(modelPath, "%s/%s", genPath, ent->d_name);

        // create instance descriptor
        managerInstance_t *instance = instance_new(modelPath);
        if (instance == NULL) {
            closedir(dir);
            free(genPath);
            return 1;
        }
        instance->generation = genLast;

        // printf("\tLoaded model: %s\n", modelPath);
    }

    closedir(dir);
    free(genPath);

    // if nothing is loaded report failure
    if (descriptors->size == 0) {
        return 1;
    }

    if (populationDir != NULL) {
        free(populationDir);
        populationDir = NULL;
    }
    cu_CStringConcat(&populationDir, populationPath);
    return 0;
}

int32_t mInstancer_startPopulation(void)
{
    // if population is not loaded, or thread is already running, return failure
    if (descriptors == NULL || descriptors->size == 0 || instancesRunning) {
        return 1;
    }

    // clear old thread if left finished but not joined
    pthread_mutex_lock(&instancerMutex);
    if (!instancesRunning && thread_instanceStarter != 0) {
        pthread_join(thread_instanceStarter, NULL);
        thread_instanceStarter = 0;
    }
    pthread_mutex_unlock(&instancerMutex);

    // start worker thread to manage running instances
    pthread_create(&thread_instanceStarter, NULL, thr_instanceStarter, NULL);

    return 0;
}

int32_t mInstancer_stopPopulation(void)
{
    // if thread is not running, return failure
    if (thread_instanceStarter == 0 || !instancesRunning) {
        return 1;
    }

    // stop worker thread
    if (pthread_cancel(thread_instanceStarter) != 0) {
        return 1;
    }
    pthread_join(thread_instanceStarter, NULL);
    thread_instanceStarter = 0;

    // update instances which were running, not started or not evaluated as failed
    pthread_mutex_lock(&instancerMutex);
    for (int i = 0; i < descriptors->size; i++) {
        managerInstance_t *instance = (managerInstance_t *)xArray_get(descriptors, i);
        if (instance->status & (INSTANCE_FINISHED | INSTANCE_RUNNING | INSTANCE_WAITING)) {
            instance->status = INSTANCE_ERRORED;
        }
        struct sharedState_s *shStat = (struct sharedState_s *)xDictionary_get(shStatDict, cu_CStringHash(instance->shmemStatus));
        if (shStat != NULL) {
            sm_lockSharedState(shStat);
            shStat->control_gameExit = true;
            shStat->control_neuronsExit = true;
            sm_unlockSharedState(shStat);

            waitpid(instance->gamePID, NULL, 0);
            waitpid(instance->aiPID, NULL, 0);
        }
    }
    pthread_mutex_unlock(&instancerMutex);

    return 0;
}

int32_t mInstancer_killIndividual(uint32_t instanceID)
{
    // if instancer is not initialized or instanceID is out of bounds, return failure
    if (descriptors == NULL || descriptors->size == 0 || instanceID >= (uint32_t)descriptors->size) {
        return 1;
    }

    pthread_mutex_lock(&instancerMutex);
    managerInstance_t *instance = (managerInstance_t *)xArray_get(descriptors, (int)instanceID);

    // if instance is not running return failure
    if ((instance->status & INSTANCE_RUNNING) == 0) {
        pthread_mutex_unlock(&instancerMutex);
        return 1;
    }

    // terminate game and AI processes
    kill(instance->gamePID, SIGTERM);
    kill(instance->aiPID, SIGTERM);
    instance->status = INSTANCE_ERRORED;

    pthread_mutex_unlock(&instancerMutex);
    return 0;
}

int32_t mInstancer_toggleHeadless(uint32_t instanceID)
{
    // if instancer is not initialized or instanceID is out of bounds, return failure
    if (descriptors == NULL || descriptors->size == 0 || instanceID >= (uint32_t)descriptors->size) {
        return 1;
    }

    pthread_mutex_lock(&instancerMutex);
    managerInstance_t *instance = (managerInstance_t *)xArray_get(descriptors, (int)instanceID);

    // if instance is not running return failure
    if ((instance->status & INSTANCE_RUNNING) == 0) {
        pthread_mutex_unlock(&instancerMutex);
        return 1;
    }

    // toggle headless mode
    struct sharedState_s *shStat = (struct sharedState_s *)xDictionary_get(shStatDict, cu_CStringHash(instance->shmemStatus));
    if (shStat == NULL) {
        pthread_mutex_unlock(&instancerMutex);
        return 1;
    }
    sm_lockSharedState(shStat);
    shStat->game_runHeadless = !shStat->game_runHeadless;
    sm_unlockSharedState(shStat);

    pthread_mutex_unlock(&instancerMutex);
    return 0;
}

const managerInstance_t *mInstancer_get(uint32_t instanceID)
{
    pthread_mutex_lock(&instancerMutex);
    if (instanceID >= (uint32_t)descriptors->size) {
        pthread_mutex_unlock(&instancerMutex);
        return NULL;
    }
    pthread_mutex_unlock(&instancerMutex);
    return (const managerInstance_t *)xArray_get(descriptors, (int)instanceID);
}

const xArray *mInstancer_getAll(void)
{
    pthread_mutex_lock(&instancerMutex);
    if (descriptors == NULL || descriptors->size == 0) {
        pthread_mutex_unlock(&instancerMutex);
        return NULL;
    }
    pthread_mutex_unlock(&instancerMutex);
    return (const xArray *)descriptors;
}

void mInstancer_setMaxParallel(uint32_t value)
{
    if (value < 1) {
        value = 1;
    }

    pthread_mutex_lock(&instancerMutex);
    maxParallel = value;
    pthread_mutex_unlock(&instancerMutex);
}

void mInstancer_setMaxIterations(uint32_t value)
{
    if (value < 1) {
        value = 1;
    }

    pthread_mutex_lock(&instancerMutex);
    maxIterations = value;
    pthread_mutex_unlock(&instancerMutex);
}

void mInstancer_setEpochSize(uint32_t value)
{
    pthread_mutex_lock(&instancerMutex);
    epochIterations = value;
    pthread_mutex_unlock(&instancerMutex);
}

void mInstancer_setElitismCount(uint32_t value)
{
    pthread_mutex_lock(&instancerMutex);
    if (descriptors == NULL || descriptors->size == 0) {
        pthread_mutex_unlock(&instancerMutex);
        return;
    } else if (value > (uint32_t)descriptors->size) {
        value = (uint32_t)descriptors->size - 1;
    }
    elitismCount = value;
    pthread_mutex_unlock(&instancerMutex);
}

//------------------------------------------------------------------------------------
// local function definitions

static managerInstance_t *instance_new(char *modelPath)
{
    // prepare parameters from modelPath
    if (modelPath == NULL) {
        return NULL;
    }
    // unsigned long long pathHash = cu_CStringHash(modelPath);
    const char *filenameStart = modelPath + cu_CStringLength(modelPath);
    while (filenameStart > modelPath && *(filenameStart - 1) != '/') {
        filenameStart--;
    }

    // allocate new instance
    managerInstance_t *instance = (managerInstance_t *)malloc(sizeof(managerInstance_t));
    if (instance == NULL) {
        return NULL;
    }

    // initialize instance values
    instance->instanceID = descriptors->size;
    instance->status = INSTANCE_INACTIVE;
    instance->gamePID = -1;
    instance->aiPID = -1;
    instance->modelPath = modelPath;
    instance->generation = 0;
    instance->fitnessScore = 0.0f;

    // construct shared memory keys
    int32_t i;
    for (i = 0; filenameStart[i] != '.'; i++) {
        instance->shmemInput[i] = filenameStart[i];
        instance->shmemOutput[i] = filenameStart[i];
        instance->shmemStatus[i] = filenameStart[i];
    }
    instance->shmemInput[i] = 'i';
    instance->shmemOutput[i] = 'o';
    instance->shmemStatus[i] = 's';
    instance->shmemInput[i + 1] = '\0';
    instance->shmemOutput[i + 1] = '\0';
    instance->shmemStatus[i + 1] = '\0';

    // add instance to loaded instances
    pthread_mutex_lock(&instancerMutex);
    xArray_push(descriptors, instance);
    pthread_mutex_unlock(&instancerMutex);

    return instance;
}

static void instance_free(managerInstance_t *instance)
{
    if (instance == NULL) {
        return;
    }

    if (instance->modelPath != NULL) {
        free(instance->modelPath);
        instance->modelPath = NULL;
    }

    free(instance);
}

static int instance_compare(const managerInstance_t *a, const managerInstance_t *b)
{
    // compare instances by fitness score in ascending order
    float fitnessA = a->fitnessScore;
    float fitnessB = b->fitnessScore;
    if (fitnessA < fitnessB) {
        return 1;
    } else if (fitnessA > fitnessB) {
        return -1;
    }
    return 0;
}

static int instance_start(uint32_t instanceID)
{
    // if instanceID is out of bounds or structures are not initialized, return failure
    if (descriptors == NULL || descriptors->size == 0 || instanceID >= (uint32_t)descriptors->size) {
        return 1;
    }

    // get instance descriptor
    managerInstance_t *instance = (managerInstance_t *)xArray_get(descriptors, (int)instanceID);

    // if instance is not waiting, return failure
    if ((instance->status & INSTANCE_WAITING) == 0) {
        return 1;
    }

    // create and initialize shared memory blocks
    struct sharedInput_s *shIn = (struct sharedInput_s *)xDictionary_get(shInDict, cu_CStringHash(instance->shmemInput));
    if (shIn == NULL) {
        shIn = sm_allocateSharedInput(instance->shmemInput);
        sm_initSharedInput(shIn);
        xDictionary_insert(shInDict, cu_CStringHash(instance->shmemInput), shIn);
    } else {
        sm_initSharedInput(shIn);
    }

    struct sharedOutput_s *shOut = (struct sharedOutput_s *)xDictionary_get(shOutDict, cu_CStringHash(instance->shmemOutput));
    if (shOut == NULL) {
        shOut = sm_allocateSharedOutput(instance->shmemOutput);
        sm_initSharedOutput(shOut);
        xDictionary_insert(shOutDict, cu_CStringHash(instance->shmemOutput), shOut);
    } else {
        sm_initSharedOutput(shOut);
    }
    struct sharedState_s *shStat = (struct sharedState_s *)xDictionary_get(shStatDict, cu_CStringHash(instance->shmemStatus));
    if (shStat == NULL) {
        shStat = sm_allocateSharedState(instance->shmemStatus);
        sm_initSharedState(shStat);
        xDictionary_insert(shStatDict, cu_CStringHash(instance->shmemStatus), shStat);
    } else {
        sm_initSharedState(shStat);
    }
    shStat->state_managerAlive = true;
    shStat->game_runHeadless = true;

    // start game process
    pid_t gamePID = fork();
    if (gamePID == 0) {
        char randSeedStr[16];
        sprintf(randSeedStr, "%u", randSeed);
        char *gameArgs[] = {"./bin/game",          "-m", instance->shmemInput, instance->shmemOutput,
                            instance->shmemStatus, "-r", randSeedStr,          NULL};
        execv(gameArgs[0], gameArgs);
    } else if (gamePID < 0) {
        instance->status = INSTANCE_ERRORED;
        return 1;
    }
    instance->gamePID = gamePID;

    // start neurons process
    pid_t aiPID = fork();
    if (aiPID == 0) {
        char *aiArgs[] = {"./bin/neurons",       "-m", instance->shmemInput, instance->shmemOutput,
                          instance->shmemStatus, "-l", instance->modelPath,  NULL};
        execv(aiArgs[0], aiArgs);
    } else if (aiPID < 0) {
        kill(gamePID, SIGTERM);
        instance->status = INSTANCE_ERRORED;
        return 1;
    }
    instance->aiPID = aiPID;

    // update instance status
    instance->status = INSTANCE_RUNNING;

    return 0;
}

static void instance_writeReport(const xArray *descriptorArray)
{
    if (descriptorArray == NULL || descriptorArray->size == 0) {
        return;
    }
    pthread_mutex_lock(&instancerMutex);

    // construct report file path
    char *reportPath = (char *)malloc((cu_CStringLength(populationDir) + 12) * sizeof(char));
    if (reportPath == NULL) {
        pthread_mutex_unlock(&instancerMutex);
        return;
    }
    sprintf(reportPath, "%s/report.csv", populationDir);

    // open report file and write needed data
    FILE *reportFile = fopen(reportPath, "r+");
    if (reportFile == NULL) {
        reportFile = fopen(reportPath, "w");
        if (reportFile == NULL) {
            free(reportPath);
            pthread_mutex_unlock(&instancerMutex);
            return;
        }
        fprintf(reportFile, "Instance ID,Exit status,Model path,Generation ID,Game seed,Fitness\n");
    }
    fseek(reportFile, 0, SEEK_END);
    for (int i = 0; i < descriptorArray->size; i++) {
        managerInstance_t *instance = (managerInstance_t *)xArray_get(descriptorArray, i);
        fprintf(reportFile, "%d,%d,%s,%d,%u,%f\n", instance->instanceID, instance->status, instance->modelPath,
                instance->generation, randSeed, instance->fitnessScore);
    }

    pthread_mutex_unlock(&instancerMutex);

    fclose(reportFile);
    free(reportPath);
}

static int instance_nextgen(xArray *descriptorArray)
{
    if (descriptorArray == NULL || descriptorArray->size == 0) {
        return 1;
    }

    // create copy of population directory path
    char *popDir = NULL;
    cu_CStringConcat(&popDir, populationDir);

    // create next generation folder
    char *genDir = (char *)malloc((cu_CStringLength(populationDir) + 15) * sizeof(char));
    if (genDir == NULL) {
        return 1;
    }
    sprintf(genDir, "%s/gen%u", populationDir, ((managerInstance_t *)xArray_get(descriptorArray, 0))->generation + 1);
    mkdir(genDir, 0750);

    uint32_t generationSizeTarget = (uint32_t)descriptorArray->size;

    // copy over two best model files to next generation (and rename to model_0.fnnm and model_1.fnnm)
    pthread_mutex_lock(&instancerMutex);

    xArray_sort(descriptorArray, (int (*)(const void *, const void *))instance_compare);
    for (uint32_t i = 0; i < generationSizeTarget; i++) {
        // construct destination path of model file
        char *modelDest = (char *)malloc((cu_CStringLength(genDir) + 23) * sizeof(char));
        if (modelDest == NULL) {
            pthread_mutex_unlock(&instancerMutex);
            free(genDir);
            return 1;
        }
        sprintf(modelDest, "%s/model_%u.fnnm", genDir, i);

        // copy over two best models from previous generation
        if (i < elitismCount) {
            char *modelPath = ((managerInstance_t *)xArray_get(descriptorArray, i))->modelPath;
            fCopy(modelPath, modelDest);
            free(modelDest);
            continue;
        }

        // select two parents based on weighted random selection of fitness scores
        float fitnessSum = 0.0f;
        for (uint32_t j = 0; j < generationSizeTarget; j++) {
            fitnessSum += ((managerInstance_t *)xArray_get(descriptorArray, j))->fitnessScore;
        }
        float fitnessRand = (float)rand() / (float)RAND_MAX * fitnessSum;
        float fitnessAcc = 0.0f;
        uint32_t parent1 = 0;
        uint32_t parent2 = 0;
        for (uint32_t j = 0; j < generationSizeTarget; j++) {
            fitnessAcc += ((managerInstance_t *)xArray_get(descriptorArray, j))->fitnessScore;
            if (fitnessAcc >= fitnessRand) {
                parent1 = j;
                break;
            }
        }
        fitnessRand = (float)rand() / (float)RAND_MAX * fitnessSum;
        fitnessAcc = 0.0f;
        for (uint32_t j = 0; j < generationSizeTarget; j++) {
            fitnessAcc += ((managerInstance_t *)xArray_get(descriptorArray, j))->fitnessScore;
            if (fitnessAcc >= fitnessRand) {
                parent2 = j;
                break;
            }
        }

        // breed two parents to create new model
        FnnModel *model1 = fnn_deserialize(((managerInstance_t *)xArray_get(descriptorArray, parent1))->modelPath);
        FnnModel *model2 = fnn_deserialize(((managerInstance_t *)xArray_get(descriptorArray, parent2))->modelPath);
        FnnModel *modelNew = fnn_modelBreed(model1, model2, BREED_CROSSOVER_INDEX, BREED_MUTATION_RATE, BREED_MUTATION_STDDEV);
        if (modelNew == NULL) {
            pthread_mutex_unlock(&instancerMutex);
            free(genDir);
            free(modelDest);
            return 1;
        }

        // serialize new model to file
        fnn_serialize(modelDest, modelNew);
        free(modelDest);
        fnn_free(model1);
        fnn_free(model2);
        fnn_free(modelNew);
    }
    pthread_mutex_unlock(&instancerMutex);

    // auto-load next generation
    mInstancer_loadPopulation(popDir);

    free(genDir);
    free(popDir);

    return 0;
}

static void *thr_instanceStarter(void *arg)
{
    (void)arg;  // ignore args

    randSeed = (uint32_t)rand();
    uint32_t parallelMax = maxParallel;
    uint32_t iterationMax = maxIterations;

    pthread_mutex_lock(&instancerMutex);
    instancesRunning = true;
    pthread_mutex_unlock(&instancerMutex);

    for (uint32_t iteration = 0; iteration < iterationMax; iteration++) {
        uint32_t nextStarting = 0;
        uint32_t runningInstances = 0;
        bool allEnded = false;

        if (epochIterations > 0 && iteration % epochIterations == 0) {
            srand((unsigned int)time(NULL) ^ (unsigned int)rand());
            randSeed = (uint32_t)rand();
        }

        // set all loaded instances to waiting state
        pthread_mutex_lock(&instancerMutex);
        for (int i = 0; i < descriptors->size; i++) {
            managerInstance_t *instance = (managerInstance_t *)xArray_get(descriptors, i);
            instance->status = INSTANCE_WAITING;
        }
        pthread_mutex_unlock(&instancerMutex);

        // start instances and wait for them to finish
        while (!allEnded) {
            allEnded = true;
            pthread_mutex_lock(&instancerMutex);

            // start next instance if possible
            if (nextStarting < (uint32_t)descriptors->size && runningInstances < parallelMax) {
                instance_start(nextStarting);
                if (((managerInstance_t *)xArray_get(descriptors, nextStarting))->status == INSTANCE_RUNNING)
                    runningInstances++;
                nextStarting++;
            }

            // update descriptors of finished, errored and running instances
            for (int i = 0; i < descriptors->size; i++) {
                managerInstance_t *instance = (managerInstance_t *)xArray_get(descriptors, i);
                if ((instance->status & (INSTANCE_ENDED | INSTANCE_ERRENDED)) == 0) {
                    allEnded = false;
                }
                if (instance->status & INSTANCE_RUNNING) {
                    allEnded = false;

                    if (kill(instance->gamePID, 0) == -1 || kill(instance->aiPID, 0) == -1) {
                        instance->status = INSTANCE_ERRORED;
                        kill(instance->gamePID, SIGTERM);
                        kill(instance->aiPID, SIGTERM);
                        waitpid(instance->gamePID, NULL, 0);
                        waitpid(instance->aiPID, NULL, 0);
                        runningInstances--;
                    } else {
                        struct sharedState_s *shStat =
                            (struct sharedState_s *)xDictionary_get(shStatDict, cu_CStringHash(instance->shmemStatus));
                        sm_lockSharedState(shStat);
                        if (shStat->game_isOver) {
                            instance->status = INSTANCE_FINISHED;
                            instance->fitnessScore = shStat->game_gameScore * FITNESS_WEIGHT_SCORE +
                                                     shStat->game_gameTime * FITNESS_WEIGHT_TIME +
                                                     shStat->game_gameLevel * FITNESS_WEIGHT_LEVEL;
                            shStat->control_gameExit = true;
                            shStat->control_neuronsExit = true;
                        }
                        sm_unlockSharedState(shStat);
                    }
                }
                if (instance->status & (INSTANCE_FINISHED | INSTANCE_ERRORED)) {
                    // wait for game and AI processes to exit
                    waitpid(instance->gamePID, NULL, 0);
                    waitpid(instance->aiPID, NULL, 0);

                    // update instance status
                    instance->status = (instance->status & INSTANCE_FINISHED) ? INSTANCE_ENDED : INSTANCE_ERRENDED;

                    runningInstances--;
                }
            }

            pthread_mutex_unlock(&instancerMutex);

            // write report and create next generation (if needed) if all instances ended
            if (allEnded) {
                instance_writeReport(descriptors);

                if (instance_nextgen(descriptors) != 0) {
                    break;
                }
            }

            sleep(1);
        }
    }

    pthread_mutex_lock(&instancerMutex);
    instancesRunning = false;
    pthread_mutex_unlock(&instancerMutex);

    return NULL;
}

static int fCopy(const char *src, const char *dest)
{
    int in, out;
    char buffer[4096];
    ssize_t nread;

    in = open(src, O_RDONLY);
    if (in == -1) {
        return 1;
    }

    out = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out == -1) {
        close(in);
        return 1;
    }

    while ((nread = read(in, buffer, sizeof(buffer))) > 0) {
        if (write(out, buffer, nread) != nread) {
            close(in);
            close(out);
            return 1;
        }
    }

    close(in);
    close(out);

    return (nread == -1) ? 1 : 0;
}
