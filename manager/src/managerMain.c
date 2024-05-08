#include "managerMain.h"  // manager program related enums, structs, etc.

#include <dirent.h>     // directory operations
#include <pthread.h>    // POSIX threads (worker threads)
#include <stdint.h>     // standard integer types
#include <stdio.h>      // standard input/output
#include <sys/stat.h>   // data returned by the stat() function
#include <sys/types.h>  // unix data types
#include <sys/wait.h>   // waitpid function
#include <time.h>       // time types
#include <unistd.h>     // standard symbolic constants and types

#include "commonUtility.h"    // common utility functions
#include "fnnGenAlgorithm.h"  // genetic algorithm functions for feedforward neural network (FNN) weights and biases
#include "fnnSerializer.h"    // neural network model reading and writing
#include "sharedMemory.h"     // shared memory functions
#include "xArray.h"           // dynamic array and its functions
#include "xDictionary.h"      // treemap and its functions
#include "xString.h"          // string functions
#include "xStringIO.h"        // xString I/O functions

// menu function declarations
static inline void clearscreen(void);              // clear screen
static void mmenu_createPopulation(void);          // generate initial generations
static void mmenu_loadPopulation(void);            // load generations
static void mmenu_startGameAgentPairs(void);       // start game-agent pairs
static void mmenu_overlookRunningInstances(void);  // view running instances
static void mmenu_exitProgram(void);               // exit program
static bool mmmenu_overlookInterpreter(void);      // running instances command sub-menu

// TODO: instance function declarations
static managerInstance_t *instance_new(void);
static void instance_free(managerInstance_t *descriptor);
static int instance_start(managerInstance_t *descriptor);
// static int instance_end(managerInstance_t *descriptor);
// static int instance_status(managerInstance_t *descriptor);

static int removeDirectoryContent(char *folderPath);          // remove all files from directory
static void *instance_starter(xArray *descriptorArray);       // worker thread for automatic instance starting
static void *instance_statusUpdate(xArray *descriptorArray);  // worker thread for automatic instance status updating

// menu entry item
typedef struct {
    const char *name;
    void (*function)(void);
} MenuEntry;

// global variables
uint32_t errorFlag = 0;                                   // error flag
uint32_t randSeed = 0;                                    // random seed
xArray *instanceDescriptors = NULL;                       // array of instance descriptors
xDictionary *instanceShmemIn = NULL;                      // dictionary of input shared memory pointers
xDictionary *instanceShmemOut = NULL;                     // dictionary of output shared memory pointers
xDictionary *instanceShmemStat = NULL;                    // dictionary of status shared memory pointers
pthread_mutex_t globalMutex = PTHREAD_MUTEX_INITIALIZER;  // global mutex for shared memory access
uint32_t maxParallelInstances = 0;                        // maximum number of parallel instances

// worker threads
pthread_t starterThread;  // automatic instance starting
pthread_t statusThread;   // automatic instance status checking (updating descriptors according to shared memory status)

// main (menu selector)
int main(void)
{
    // seed random number generator and generate global random seed
    srand(time(NULL));
    randSeed = (uint32_t)rand();

    // local variables
    xString *inputStr = NULL;

    instanceDescriptors = xArray_new();
    instanceShmemIn = xDictionary_new();
    instanceShmemOut = xDictionary_new();
    instanceShmemStat = xDictionary_new();
    if (instanceDescriptors == NULL || instanceShmemIn == NULL || instanceShmemOut == NULL || instanceShmemStat == NULL) {
        printf("Memory allocation error\n");
        return 1;
    }

    // menu items
    MenuEntry menuEntries[] = {{"Generate initial generations", mmenu_createPopulation},
                               {"Load generations", mmenu_loadPopulation},
                               {"Start game-agent pairs", mmenu_startGameAgentPairs},
                               {"View instance status", mmenu_overlookRunningInstances},
                               {"Exit program", mmenu_exitProgram}};

    // menu loop
    while (errorFlag == 0) {
        clearscreen();

        // print menu
        printf("Menu:\n");
        for (uint64_t i = 0; i < sizeof(menuEntries) / sizeof(MenuEntry); i++) {
            printf("%ld. %s\n", i + 1, menuEntries[i].name);
        }

        // get user input
        uint64_t choice = 0;
        printf("\nEnter your choice: ");
        inputStr = xString_readIn();

        // evaluate user input
        if (inputStr == NULL) {
            perror("xString_readIn() error");
            return 1;
        } else {
            choice = xString_toInt(inputStr);
            xString_free(inputStr);

            if (choice < 1 || choice > (uint64_t)(sizeof(menuEntries) / sizeof(MenuEntry))) {
                printf("Invalid choice. Please try again.\n");
                continue;
            }
        }

        // execute selected function
        menuEntries[choice - 1].function();

        // check if error occured
        if (errorFlag && errorFlag != (uint32_t)-1) {
            fprintf(stderr, "Error occured with code %d\nExiting...\n", errorFlag);
            return (int)errorFlag;
        } else {
            // wait for user input
            printf("\nPress enter to continue...");
            getchar();
        }
    }

    return 0;
}

// clear screen
inline void clearscreen(void) { printf("\033[2J\033[1;1H"); }

// generate initial generations
void mmenu_createPopulation(void)
{
    // local variables
    xString *inputStr = NULL;

    // print submenu title
    clearscreen();
    printf("Generate initial generations\n");

    // print current working directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
        errorFlag = 5;
        return;
    }

    // ask user for folder name (generation name)
    char *folderName = NULL;
    printf("Enter folder name (generation name): ");
    inputStr = xString_readInSafe(26);
    if (inputStr == NULL || xString_isEmpty(inputStr)) {
        perror("xString_readIn() error");
        errorFlag = 1;
        return;
    } else {
        folderName = xString_toCString(inputStr);
        xString_free(inputStr);
    }

    // check if folder with given name already exists
    struct stat st = {0};
    if (stat(folderName, &st) == -1) {
        // create folder
        if (mkdir(folderName, 0700) == -1) {
            perror("mkdir() error");
            errorFlag = 4;
            return;
        }
    } else {
        // ask user if he wants to overwrite existing folder
        printf("Folder with given name already exists. Do you want to overwrite "
               "it? (y/n): ");
        char answer = getchar();
        getchar();  // Consume the newline character
        if (answer == 'y' || answer == 'Y') {
            // delete folder content
            if (removeDirectoryContent(folderName) == -1) {
                perror("removeDirectoryContent() error");
                return;
            }
        } else {
            printf("Operation canceled.\n");
            return;
        }
    }

    // ask user for size of initial generation
    printf("Enter size of initial generation: ");
    inputStr = xString_readInSafe(8);
    if (inputStr == NULL || xString_isEmpty(inputStr)) {
        perror("xString_readIn() error");
        errorFlag = 1;
        return;
    }

    // evaluate user input
    uint32_t genSize = (uint32_t)xString_toInt(inputStr);
    xString_free(inputStr);
    if (genSize <= 1) {
        printf("Invalid size. Please try again.\n");
        return;
    }

    // get hidden layer count
    printf("Enter hidden layer count: ");
    inputStr = xString_readInSafe(8);
    if (inputStr == NULL || xString_isEmpty(inputStr)) {
        perror("xString_readIn() error");
        errorFlag = 1;
        return;
    }

    // evaluate user input
    uint32_t hiddenLayerCount = (uint32_t)xString_toInt(inputStr);
    xString_free(inputStr);
    if (hiddenLayerCount <= 0) {
        printf("Invalid hidden layer count. Please try again.\n");
        return;
    }

    // get size for each hidden layer
    uint32_t *layerSizes = (uint32_t *)malloc((hiddenLayerCount + 2) * sizeof(uint32_t));
    if (layerSizes == NULL) {
        perror("malloc() error");
        errorFlag = 1;
        return;
    }
    layerSizes[0] = 5;                     // game -> input layer
    layerSizes[hiddenLayerCount + 1] = 4;  // output layer -> game
    uint64_t weightsTotal = 0;
    uint64_t biasesTotal = 0;

    for (uint32_t i = 1; i < hiddenLayerCount + 1; i++) {
        printf("Enter size for hidden layer %d: ", i);
        inputStr = xString_readInSafe(8);
        if (inputStr == NULL || xString_isEmpty(inputStr)) {
            perror("xString_readIn() error");
            errorFlag = 1;
            return;
        }

        // evaluate user input
        layerSizes[i] = (uint32_t)xString_toInt(inputStr);
        xString_free(inputStr);
        if (layerSizes[i] <= 0) {
            printf("Invalid size. Please try again.\n");
            return;
        }
        weightsTotal += layerSizes[i - 1] * layerSizes[i];
        biasesTotal += layerSizes[i];
    }
    weightsTotal += layerSizes[hiddenLayerCount] * layerSizes[hiddenLayerCount + 1];
    biasesTotal += layerSizes[hiddenLayerCount + 1];

    // set activation functions for layers (ReLU for hidden layers, sigmoid for
    // output layer) (input layer has no activation function)
    FnnActivation_e *activationFunctions = (FnnActivation_e *)malloc((hiddenLayerCount + 1) * sizeof(FnnActivation_e));
    if (activationFunctions == NULL) {
        perror("malloc() error");
        errorFlag = 1;
        return;
    }
    for (uint32_t i = 0; i < hiddenLayerCount; i++) {
        activationFunctions[i] = FNN_ACTIVATION_RELU;
    }
    activationFunctions[hiddenLayerCount] = FNN_ACTIVATION_SIGMOID;

    // generate random neural netwprk models for initial generation for given
    // parameters
    for (uint32_t i = 0; i < genSize; i++) {
        // create model descriptor
        FnnModel *model = fnn_new();
        if (model == NULL) {
            perror("fnn_new() error");
            errorFlag = 1;
            return;
        }

        // set neuron counts
        model->layerCount = hiddenLayerCount + 2;
        model->neuronCounts = layerSizes;
        model->totalWeights = weightsTotal;
        model->totalBiases = biasesTotal;
        model->activationFunctions = activationFunctions;

        // generate random weights and biases
        model->weightValues = fnn_generateWeights(layerSizes, hiddenLayerCount + 2, -1.0f, 1.0f);
        if (model->weightValues == NULL) {
            perror("fnn_generateWeights() error");
            errorFlag = 1;
            return;
        }
        model->biasValues = fnn_generateBiases(layerSizes, hiddenLayerCount + 2, 0.0f, 1.0f);
        if (model->biasValues == NULL) {
            perror("fnn_generateBiases() error");
            errorFlag = 1;
            return;
        }

        // create file name
        xString *fileName = xString_new();
        xString_appendCString(fileName, folderName);
        xString_appendChar(fileName, '/');
        xString_appendCString(fileName, "model_");
        xString *tmp = xString_fromInt(i);
        xString_appendString(fileName, tmp);
        xString_free(tmp);
        xString_appendCString(fileName, ".fnnm");

        char *fileNameStr = xString_toCString(fileName);
        xString_free(fileName);

        // write model to file
        if (fnn_serialize(fileNameStr, model) != 0) {
            perror("fnn_serialize() error");
            errorFlag = 7;
            return;
        }

        // free memory
        free(fileNameStr);
        free(model->weightValues);
        free(model->biasValues);
        free(model);

        // print progress
        printf("Model %d/%d generated\n", i + 1, genSize);
    }

    // free memory
    free(layerSizes);
    free(activationFunctions);
    free(folderName);

    // print success message
    printf("Initial generation generated successfully\n");
}

void mmenu_loadPopulation(void)
{
    // local variables
    xString *inputStr = NULL;

    // print submenu title
    clearscreen();
    printf("Load population\n");

    // print current working directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
        errorFlag = 5;
        return;
    }

    // ask user for folder name (name of population to load)
    char *folderName = NULL;
    printf("Enter folder name (population name): ");
    inputStr = xString_readInSafe(26);
    if (inputStr == NULL || xString_isEmpty(inputStr)) {
        perror("xString_readIn() error");
        errorFlag = 1;
        return;
    } else {
        folderName = xString_toCString(inputStr);
        xString_free(inputStr);
    }

    // check if folder with given name exists
    struct stat st = {0};
    if (stat(folderName, &st) == -1) {
        printf("Folder with given name does not exist.\n");
        return;
    }

    // clear old descriptors if any
    xArray_forEach(instanceDescriptors, (void (*)(void *))instance_free);
    instanceDescriptors->size = 0;

    // load models from folder
    xString *shmKey = xString_new();
    xString *filePath = xString_new();

    // open directory
    DIR *dir = opendir(folderName);
    if (dir == NULL) {
        perror("opendir() error");
        errorFlag = 2;
        return;
    }

    // read directory content (file extension .fnnm)
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // skip everything that is not a regular file or does not have .fnnm extension at the end
        if (entry->d_type != DT_REG || cu_CStringEndsWith(entry->d_name, ".fnnm") == 0) {
            continue;
        }

        printf("Loading model: %s\n", entry->d_name);

        // create instance descriptor
        managerInstance_t *instance = instance_new();
        if (instance == NULL) {
            perror("instance_new() error");
            errorFlag = 1;
            return;
        }

        // construct file path and add to model array
        xString_remove(filePath, 0, filePath->len);
        xString_appendCString(filePath, folderName);
        xString_appendChar(filePath, '/');
        xString_appendCString(filePath, entry->d_name);
        instance->modelPath = xString_toCString(filePath);

        // construct shared memory keys
        xString_remove(shmKey, 0, shmKey->len);
        xString_appendCString(shmKey, entry->d_name);
        xString_removeLastCString(shmKey, ".fnnm");

        uint64_t hash = (uint64_t)xString_hash(shmKey);
        instance->sharedMemoryID = *((uint32_t *)&hash) ^ *((uint32_t *)&hash + 1);

        xString_appendCString(shmKey, "_in");
        for (int i = 0; i < shmKey->len; i++) {
            instance->shmemInput[i] = (char)shmKey->data[i];
        }
        instance->shmemInput[shmKey->len] = '\0';
        xString_removeLastCString(shmKey, "_in");
        xString_appendCString(shmKey, "_out");
        for (int i = 0; i < shmKey->len; i++) {
            instance->shmemOutput[i] = (char)shmKey->data[i];
        }
        instance->shmemOutput[shmKey->len] = '\0';
        xString_removeLastCString(shmKey, "_out");
        xString_appendCString(shmKey, "_stat");
        for (int i = 0; i < shmKey->len; i++) {
            instance->shmemStatus[i] = (char)shmKey->data[i];
        }
        instance->shmemStatus[shmKey->len] = '\0';

        instance->instanceID = instanceDescriptors->size;
        xArray_push(instanceDescriptors, (void *)instance);

        printf("Loaded model: %s\n", instance->modelPath);
    }

    closedir(dir);
    xString_free(filePath);
    xString_free(shmKey);

    return;
}

void mmenu_startGameAgentPairs(void)
{
    // local variables
    xString *inputStr = NULL;

    // print submenu title
    clearscreen();
    printf("Start game-agent pairs\n");

    // check if descriptor array contains any instances
    if (instanceDescriptors->size == 0) {
        printf("No population loaded. Please load population first.\n");
        return;
    }

    // ask user for maximum number of simultaneous instances
    printf("Enter maximum number of simultaneous instances: ");
    inputStr = xString_readInSafe(8);

    // evaluate user input
    maxParallelInstances = (uint32_t)xString_toInt(inputStr);
    xString_free(inputStr);
    if (maxParallelInstances <= 0) {
        printf("Invalid number of instances. Please try again.\n");
        return;
    } else {
        if (pthread_create(&starterThread, NULL, (void *(*)(void *))instance_starter, (void *)instanceDescriptors) != 0) {
            perror("pthread_create() error");
            errorFlag = 6;
            return;
        }
        // start worker thread for automatic instance status updating
        if (pthread_create(&statusThread, NULL, (void *(*)(void *))instance_statusUpdate, (void *)instanceDescriptors) != 0) {
            perror("pthread_create() error");
            errorFlag = 6;
            return;
        }
    }

    return;
}

void mmenu_overlookRunningInstances(void)
{
    // local variables
    bool runFlag = true;

    while (runFlag && errorFlag == 0) {
        // print submenu title
        clearscreen();
        printf("View running instances\n");

        // check if descriptor array contains any instances
        if (instanceDescriptors->size == 0) {
            printf("No population loaded. Please load population first.\n");
            return;
        }

        // print legend
        printf("Legend:\n");
        printf("ID  | MemID      | Status | Game PID | AI PID | Model path                     | Generation | Fitness score\n");

        // print instances
        pthread_mutex_lock(&globalMutex);
        for (int i = 0; i < instanceDescriptors->size; i++) {
            managerInstance_t *instance = (managerInstance_t *)xArray_get(instanceDescriptors, i);
            printf("%3d | %10u | %6x |   %6d | %6d | %-30s | %10d | %11.2f\n", instance->instanceID, instance->sharedMemoryID,
                   instance->instanceStatus, instance->gamePID, instance->aiPID, instance->modelPath, instance->generation,
                   instance->fitnessScore);
        }
        pthread_mutex_unlock(&globalMutex);

        runFlag = mmmenu_overlookInterpreter();
    }

    return;
}

void mmenu_exitProgram(void)
{
    // end worker threads (if created)
    if (starterThread != 0 && pthread_cancel(starterThread) != 0) {
        perror("pthread_cancel() error");
        errorFlag = 6;
        return;
    }
    if (statusThread != 0 && pthread_cancel(statusThread) != 0) {
        perror("pthread_cancel() error");
        errorFlag = 6;
        return;
    }

    // kill all child processes
    for (int i = 0; i < instanceDescriptors->size; i++) {
        managerInstance_t *instance = (managerInstance_t *)xArray_get(instanceDescriptors, i);
        if (instance->gamePID != -1 && instance->gamePID != 0) {
            kill(instance->gamePID, SIGTERM);
            waitpid(instance->gamePID, NULL, 0);
        }
        if (instance->aiPID != -1 && instance->gamePID != 0) {
            kill(instance->aiPID, SIGTERM);
            waitpid(instance->aiPID, NULL, 0);
        }
    }

    // free memory
    if (instanceDescriptors != NULL) {
        xArray_forEach(instanceDescriptors, (void (*)(void *))instance_free);
        xArray_free(instanceDescriptors);
    }
    if (instanceShmemIn != NULL) {
        xDictionary_free(instanceShmemIn);
    }
    if (instanceShmemOut != NULL) {
        xDictionary_free(instanceShmemOut);
    }
    if (instanceShmemStat != NULL) {
        xDictionary_free(instanceShmemStat);
    }

    // exit program
    printf("Exit requested.\n");
    errorFlag = (uint32_t)-1;
    return;
}

static bool mmmenu_overlookInterpreter(void)
{
    // local variables
    xString *inputStr = NULL;

    // prompt user command
    printf("\nEnter command: ");
    inputStr = xString_readInSafe(26);
    if (inputStr == NULL) {
        perror("xString_readIn() error");
        errorFlag = 1;
        return false;
    } else if (xString_isEmpty(inputStr)) {
        xString_free(inputStr);
        return true;
    }

    xString **cmdargs = xString_splitChar(inputStr, ' ');
    free(inputStr);
    if (cmdargs == NULL) {
        perror("xString_splitChar() error");
        errorFlag = 1;
        return false;
    }

    // evaluate user input
    if (xString_compareCString(cmdargs[0], "exit") == 0 || xString_compareCString(cmdargs[0], "back") == 0) {
        for (int i = 0; cmdargs[i] != NULL; i++) {
            xString_free(cmdargs[i]);
        }
        free(cmdargs);
        return false;
    } else if (xString_compareCString(cmdargs[0], "help") == 0) {
        printf("Available commands:\n");
        printf("\texit/back\t\t- exit sub-menu\n");
        printf("\thelp\t\t\t- show available commands\n");
        printf("\tshscr <instanceID>\t- toggle headless mode of instance\n");
        printf("\tshmem <instanceID>\t- print shared memory of instance\n");
        printf("\tkill <instanceID>\t- kill instance\n");
    } else if (xString_compareCString(cmdargs[0], "shscr") == 0) {
        if (cmdargs[1] == NULL) {
            printf("Invalid command. Type 'help' for available commands.\n");
        } else {
            // get instance descriptor
            int instanceID = xString_toInt(cmdargs[1]);
            if (instanceID < 0 || instanceID >= instanceDescriptors->size) {
                printf("Invalid instance ID. Please try again.\n");
            } else {
                pthread_mutex_lock(&globalMutex);
                managerInstance_t *instance = (managerInstance_t *)xArray_get(instanceDescriptors, instanceID);

                if ((instance->instanceStatus & INSTANCE_RUNNING) == 0) {
                    printf("Instance is not running!\n");
                } else {
                    struct sharedState_s *shmemStat =
                        (struct sharedState_s *)xDictionary_get(instanceShmemStat, (unsigned long long)instance->sharedMemoryID);
                    sm_lockSharedState(shmemStat);
                    shmemStat->game_runHeadless = !shmemStat->game_runHeadless;
                    sm_unlockSharedState(shmemStat);
                }

                pthread_mutex_unlock(&globalMutex);
            }
        }
    } else if (xString_compareCString(cmdargs[0], "shmem") == 0) {
        if (cmdargs[1] == NULL) {
            printf("Invalid command. Type 'help' for available commands.\n");
        } else {
            // get instance descriptor
            int instanceID = xString_toInt(cmdargs[1]);
            if (instanceID < 0 || instanceID >= instanceDescriptors->size) {
                printf("Invalid instance ID. Please try again.\n");
            } else {
                pthread_mutex_lock(&globalMutex);
                managerInstance_t *instance = (managerInstance_t *)xArray_get(instanceDescriptors, instanceID);

                // print shared memory contents
                printf("Shared memory of instance %d:\n", instanceID);

                printf("\tInput shared memory:\n");
                struct sharedInput_s *shmemIn =
                    (struct sharedInput_s *)xDictionary_get(instanceShmemIn, (unsigned long long)instance->sharedMemoryID);
                if (shmemIn == NULL) {
                    printf("\t\tUnavailable\n");
                } else {
                    sm_lockSharedInput(shmemIn);
                    printf("\t\tKEY_W:\t%d\n", shmemIn->isKeyDownW);
                    printf("\t\tKEY_A:\t%d\n", shmemIn->isKeyDownA);
                    printf("\t\tKEY_D:\t%d\n", shmemIn->isKeyDownD);
                    printf("\t\tKEY_SPACE:\t%d\n", shmemIn->isKeyDownSpace);
                    sm_unlockSharedInput(shmemIn);
                }

                printf("\tOutput shared memory:\n");
                struct sharedOutput_s *shmemOut =
                    (struct sharedOutput_s *)xDictionary_get(instanceShmemOut, (unsigned long long)instance->sharedMemoryID);
                if (shmemOut == NULL) {
                    printf("\t\tUnavailable\n");
                } else {
                    sm_lockSharedOutput(shmemOut);
                    printf("\t\tOUT_1:\t%f\n", shmemOut->gameOutput01);
                    printf("\t\tOUT_2:\t%f\n", shmemOut->gameOutput02);
                    printf("\t\tOUT_3:\t%f\n", shmemOut->gameOutput03);
                    printf("\t\tOUT_4:\t%f\n", shmemOut->gameOutput04);
                    printf("\t\tOUT_5:\t%f\n", shmemOut->gameOutput05);
                    sm_unlockSharedOutput(shmemOut);
                }

                printf("\tStatus shared memory:\n");
                struct sharedState_s *shmemStat =
                    (struct sharedState_s *)xDictionary_get(instanceShmemStat, (unsigned long long)instance->sharedMemoryID);
                if (shmemStat == NULL) {
                    printf("\t\tUnavailable\n");
                } else {
                    sm_lockSharedState(shmemStat);
                    printf("\t\tGAME_ALIVE:\t%d\n", shmemStat->state_gameAlive);
                    printf("\t\tMANAGER_ALIVE:\t%d\n", shmemStat->state_managerAlive);
                    printf("\t\tNEURONS_ALIVE:\t%d\n", shmemStat->state_neuronsAlive);
                    printf("\t\tGAME_EXIT:\t%d\n", shmemStat->control_gameExit);
                    printf("\t\tNEURONS_EXIT:\t%d\n", shmemStat->control_neuronsExit);
                    printf("\t\tGAME_OVER:\t%d\n", shmemStat->game_isOver);
                    printf("\t\tGAME_PAUSED:\t%d\n", shmemStat->game_isPaused);
                    printf("\t\tGAME_HEADLESS:\t%d\n", shmemStat->game_runHeadless);
                    printf("\t\tGAME_SCORE:\t%d\n", shmemStat->game_gameScore);
                    printf("\t\tGAME_LEVEL:\t%d\n", shmemStat->game_gameLevel);
                    printf("\t\tGAME_TIME:\t%ld\n", shmemStat->game_gameTime);
                    sm_unlockSharedState(shmemStat);
                }

                pthread_mutex_unlock(&globalMutex);
            }
        }
    } else if (xString_compareCString(cmdargs[0], "kill") == 0) {
        if (cmdargs[1] == NULL) {
            printf("Invalid command. Type 'help' for available commands.\n");
        } else {
            // get instance descriptor
            int instanceID = xString_toInt(cmdargs[1]);
            if (instanceID < 0 || instanceID >= instanceDescriptors->size) {
                printf("Invalid instance ID. Please try again.\n");
            } else {
                pthread_mutex_lock(&globalMutex);
                managerInstance_t *instance = (managerInstance_t *)xArray_get(instanceDescriptors, instanceID);

                // kill game and AI processes
                if (instance->gamePID != -1 && instance->gamePID != 0) {
                    kill(instance->gamePID, SIGTERM);
                    waitpid(instance->gamePID, NULL, 0);
                }
                if (instance->aiPID != -1 && instance->gamePID != 0) {
                    kill(instance->aiPID, SIGTERM);
                    waitpid(instance->aiPID, NULL, 0);
                }

                instance->instanceStatus = INSTANCE_ENDED;
                pthread_mutex_unlock(&globalMutex);
            }
        }
    } else {
        printf("Invalid command. Type 'help' for available commands.\n");
    }

    // free memory
    for (int i = 0; cmdargs[i] != NULL; i++) {
        xString_free(cmdargs[i]);
    }
    free(cmdargs);

    // wait for user input
    printf("\nPress enter to continue...");
    getchar();

    return true;
}

int removeDirectoryContent(char *folderPath)
{
    // local variables
    xArray *files = xArray_new();
    xString *folderPathStr = xString_fromCString(folderPath);
    xString *fileName = xString_new();
    xString *filePath = xString_new();
    char *tmp = NULL;

    // open directory
    DIR *dir = opendir(folderPath);
    if (dir == NULL) {
        perror("opendir() error");
        errorFlag = 2;
        return -1;
    }

    // read directory content
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // skip . and ..
        if (cu_CStringCompare(entry->d_name, ".") == 0 || cu_CStringCompare(entry->d_name, "..") == 0) {
            continue;
        }

        // create file path
        xString_remove(fileName, 0, fileName->len);
        xString_appendCString(fileName, folderPath);
        xString_appendChar(fileName, '/');
        xString_appendCString(fileName, entry->d_name);

        // add file path to array
        tmp = xString_toCString(fileName);
        xArray_push(files, tmp);
    }

    // close directory
    closedir(dir);

    // remove files
    for (int i = 0; i < files->size; i++) {
        // remove file
        if (remove((char *)xArray_get(files, i)) == -1) {
            perror("remove() error");
            errorFlag = 3;
            return -1;
        }
    }

    // free memory
    xArray_forEach(files, free);
    xArray_free(files);
    xString_free(folderPathStr);
    xString_free(fileName);
    xString_free(filePath);

    return 0;
}

void *instance_starter(xArray *descriptorArray)
{
    uint32_t runningInstances = 0;
    int nextStartingInstance = 0;
    bool allEnded;

    while (errorFlag == 0) {
        allEnded = true;

        // lock global mutex (to access instanceDescriptors)
        pthread_mutex_lock(&globalMutex);

        // check if there are any instances to start
        if (nextStartingInstance < descriptorArray->size) {
            // check if maximum number of parallel instances is reached
            if (runningInstances < maxParallelInstances) {
                managerInstance_t *instance = (managerInstance_t *)xArray_get(descriptorArray, nextStartingInstance);

                // create shared memory for instance
                struct sharedInput_s *shmemIn = sm_allocateSharedInput(instance->shmemInput);
                struct sharedOutput_s *shmemOut = sm_allocateSharedOutput(instance->shmemOutput);
                struct sharedState_s *shmemStat = sm_allocateSharedState(instance->shmemStatus);
                if (shmemIn == NULL || shmemOut == NULL || shmemStat == NULL) {
                    perror("sm_allocateShared*() error");
                    instance->instanceStatus = INSTANCE_ERRORED;
                } else {
                    // initialize shared memory values
                    sm_initSharedInput(shmemIn);
                    sm_initSharedOutput(shmemOut);
                    sm_initSharedState(shmemStat);
                    shmemStat->state_managerAlive = true;
                    shmemStat->game_runHeadless = true;

                    // save shared memory pointers to dictionaries
                    xDictionary_insert(instanceShmemIn, (unsigned long long)instance->sharedMemoryID, (void *)shmemIn);
                    xDictionary_insert(instanceShmemOut, (unsigned long long)instance->sharedMemoryID, (void *)shmemOut);
                    xDictionary_insert(instanceShmemStat, (unsigned long long)instance->sharedMemoryID, (void *)shmemStat);
                }

                // start instance
                if (instance_start(instance)) {
                    instance->instanceStatus = INSTANCE_ERRORED;
                } else {
                    instance->instanceStatus = INSTANCE_RUNNING;
                    runningInstances++;
                }
                nextStartingInstance++;
            }
        }

        // check if there are any instances to end
        for (int i = 0; i < descriptorArray->size; i++) {
            managerInstance_t *instance = (managerInstance_t *)xArray_get(descriptorArray, i);
            // if we find at least one instance that has not ended or errored, main loop can't end
            if ((instance->instanceStatus & (INSTANCE_ENDED | INSTANCE_ERRENDED)) == 0) {
                allEnded = false;
            }
            if (instance->instanceStatus & (INSTANCE_FINISHED | INSTANCE_ERRORED)) {
                // wait for game and AI processes to finish
                waitpid(instance->gamePID, NULL, 0);
                waitpid(instance->aiPID, NULL, 0);

                // free shared memory of finished instance
                sm_freeSharedInput(
                    (struct sharedInput_s *)xDictionary_get(instanceShmemIn, (unsigned long long)instance->sharedMemoryID),
                    instance->shmemInput);
                sm_freeSharedOutput(
                    (struct sharedOutput_s *)xDictionary_get(instanceShmemOut, (unsigned long long)instance->sharedMemoryID),
                    instance->shmemOutput);
                sm_freeSharedState(
                    (struct sharedState_s *)xDictionary_get(instanceShmemStat, (unsigned long long)instance->sharedMemoryID),
                    instance->shmemStatus);

                // remove shared memory pointers from dictionaries
                xDictionary_remove(instanceShmemIn, (unsigned long long)instance->sharedMemoryID);
                xDictionary_remove(instanceShmemOut, (unsigned long long)instance->sharedMemoryID);
                xDictionary_remove(instanceShmemStat, (unsigned long long)instance->sharedMemoryID);

                if (instance->instanceStatus & INSTANCE_FINISHED) {
                    instance->instanceStatus = INSTANCE_ENDED;
                } else {
                    instance->instanceStatus = INSTANCE_ERRENDED;
                }
                runningInstances--;
            }
        }

        pthread_mutex_unlock(&globalMutex);

        if (allEnded) {
            break;
        }

        sleep(1);
    }

    return NULL;
}

void *instance_statusUpdate(xArray *descriptorArray)
{
    bool allFinished;
    while (errorFlag == 0) {
        allFinished = true;

        // check if there are descriptors to update
        pthread_mutex_lock(&globalMutex);
        for (int i = 0; i < descriptorArray->size; i++) {
            managerInstance_t *instance = (managerInstance_t *)xArray_get(descriptorArray, i);
            if (!(instance->instanceStatus & (INSTANCE_ENDED | INSTANCE_ERRENDED))) {
                allFinished = false;
            }
            if (instance->instanceStatus == INSTANCE_RUNNING) {
                // check shared memory status
                struct sharedState_s *state =
                    (struct sharedState_s *)xDictionary_get(instanceShmemStat, (unsigned long long)instance->sharedMemoryID);
                if (state == NULL || kill(instance->gamePID, 0) == -1 || kill(instance->aiPID, 0) == -1) {
                    instance->instanceStatus = INSTANCE_ERRORED;
                    kill(instance->gamePID, SIGTERM);
                    kill(instance->aiPID, SIGTERM);
                } else {
                    sm_lockSharedState(state);
                    if (state->game_isOver) {
                        instance->fitnessScore = state->game_gameScore * FITNESS_WEIGHT_SCORE +
                                                 state->game_gameTime * FITNESS_WEIGHT_TIME +
                                                 state->game_gameLevel * FITNESS_WEIGHT_LEVEL;
                        state->control_gameExit = true;
                        state->control_neuronsExit = true;

                        instance->instanceStatus = INSTANCE_FINISHED;
                    }
                    sm_unlockSharedState(state);
                }
            }
        }
        pthread_mutex_unlock(&globalMutex);

        if (allFinished) {
            break;
        }

        sleep(1);
    }

    return NULL;
}

managerInstance_t *instance_new(void)
{
    managerInstance_t *ret = (managerInstance_t *)malloc(sizeof(managerInstance_t));
    if (ret == NULL) {
        return NULL;
    }

    ret->instanceID = instanceDescriptors->size;
    ret->instanceStatus = INSTANCE_INACTIVE;
    ret->gamePID = -1;
    ret->aiPID = -1;
    ret->modelPath = NULL;
    ret->generation = 0;
    ret->fitnessScore = 0;

    return ret;
}

void instance_free(managerInstance_t *descriptor)
{
    if (descriptor == NULL) {
        return;
    }

    free(descriptor->modelPath);
    free(descriptor);
}

int instance_start(managerInstance_t *descriptor)
{
    if (descriptor == NULL || descriptor->modelPath == NULL) {
        printf("Invalid instance descriptor\n");
        return -1;
    }

    // create game process
    pid_t gamePID = fork();
    if (gamePID == -1) {
        perror("fork() error");
        return -1;
    } else if (gamePID == 0) {
        // child process, start game program in managed mode
        char randSeedStr[16];
        sprintf(randSeedStr, "%d", randSeed);
        char *gameArgs[] = {"./bin/game", "-m", descriptor->shmemInput, descriptor->shmemOutput, descriptor->shmemStatus, "-r",
                            randSeedStr,  NULL};
        execv("./bin/game", gameArgs);
    } else {
        // parent process, save game PID to descriptor
        descriptor->gamePID = gamePID;
    }

    // create AI process
    pid_t aiPID = fork();
    if (aiPID == -1) {
        perror("fork() error");
        return -1;
    } else if (aiPID == 0) {
        // child process, start AI program in managed mode with config file
        char randSeedStr[16];
        sprintf(randSeedStr, "%d", randSeed);
        char *aiArgs[] = {"./bin/neurons",         "-m", descriptor->shmemInput, descriptor->shmemOutput,
                          descriptor->shmemStatus, "-l", descriptor->modelPath,  NULL};
        execv("./bin/neurons", aiArgs);
    } else {
        // parent process, save AI PID to descriptor
        descriptor->aiPID = aiPID;
    }

    return 0;
}
