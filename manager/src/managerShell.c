#include "managerInstance.h"  // instance manager module

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

//------------------------------------------------------------------------------------
// program globals

static xDictionary *commandTable = NULL;  // command lookup table
xString *userInput = NULL;                // user input string

//------------------------------------------------------------------------------------
// local function declarations

static inline void programInit(void);
static inline void programCleanup(void);
static int mkdir_recursive(const char *path, mode_t mode);
static int remove_recursive(const char *path);
static int populationGenerate(const char *populationPath, uint32_t populationSize, uint32_t layerCount, uint32_t *layerSizes);

static int cmd_help(void);
static int cmd_version(void);
static int cmd_populationCreate(void);
static int cmd_populationLoad(void);
static int cmd_generationStart(void);
static int cmd_generationStatus(void);
static int cmd_instanceStatus(void);
static int cmd_instanceKill(void);
static int cmd_instanceShow(void);
static int cmd_clear(void);

//------------------------------------------------------------------------------------
// program entry point (main)

int main(void)
{
    programInit();

    // main loop
    while (commandTable != NULL) {
        printf("AstroMGR> ");
        xString *input = xString_readLine(stdin);
        if (input == NULL) {
            break;
        } else if (xString_isEmpty(input)) {
            xString_free(input);
            continue;
        }
        void *commandPtr = xDictionary_get(commandTable, xString_hash(input));
        if (commandPtr != NULL) {
            int (*command)(void) = (int (*)(void))commandPtr;
            command();
        } else {
            char *inputStr = xString_toCString(input);
            printf("\tUnknown command: %s\n", inputStr);
            free(inputStr);
        }
        xString_free(input);
    }

    // program cleanup is already called on exit command
    return 0;
}

//------------------------------------------------------------------------------------
// local function definitions

inline void programInit(void)
{
    // initialize the random number generator
    srand(time(NULL));

    // initialize the command lookup table
    commandTable = xDictionary_new();
    xDictionary_insert(commandTable, cu_CStringHash("help"), (void *)cmd_help);
    xDictionary_insert(commandTable, cu_CStringHash("version"), (void *)cmd_version);
    xDictionary_insert(commandTable, cu_CStringHash("popcreate"), (void *)cmd_populationCreate);
    xDictionary_insert(commandTable, cu_CStringHash("popload"), (void *)cmd_populationLoad);
    xDictionary_insert(commandTable, cu_CStringHash("genrun"), (void *)cmd_generationStart);
    xDictionary_insert(commandTable, cu_CStringHash("genstat"), (void *)cmd_generationStatus);
    xDictionary_insert(commandTable, cu_CStringHash("inststat"), (void *)cmd_instanceStatus);
    xDictionary_insert(commandTable, cu_CStringHash("instkill"), (void *)cmd_instanceKill);
    xDictionary_insert(commandTable, cu_CStringHash("instmon"), (void *)cmd_instanceShow);
    xDictionary_insert(commandTable, cu_CStringHash("clear"), (void *)cmd_clear);
    xDictionary_insert(commandTable, cu_CStringHash("exit"), (void *)programCleanup);

    // initialize manager instance module
    if (mInstancer_init() != 0) {
        printf("\t[ERR]: Failed to initialize instance manager\n");
        programCleanup();
        exit(1);
    }
}

inline void programCleanup(void)
{
    // cleanup the command lookup table
    xDictionary_free(commandTable);
    commandTable = NULL;

    // cleanup manager instance module
    mInstancer_cleanup();
}

int mkdir_recursive(const char *path, mode_t mode)
{
    // check if path is NULL
    if (path == NULL) {
        return 1;
    }

    // check if the directory already exists
    struct stat st = {0};
    if (stat(path, &st) == 0) {
        return 0;
    }

    // create directory iteratively to target
    char *parentPath = (char *)malloc((cu_CStringLength(path) + 1) * sizeof(char));
    for (uint32_t i = 0; i < cu_CStringLength(path); i++) {
        parentPath[i] = path[i];
        if (path[i + 1] == '/' || path[i + 1] == '\0') {
            parentPath[i + 1] = '\0';
            if (stat(parentPath, &st) == 0)
                continue;
            if (mkdir(parentPath, mode) != 0) {
                perror("\t[ERR]: mkdir() error");
                free(parentPath);
                return 1;
            }
        }
    }
    free(parentPath);

    return 0;
}

int remove_recursive(const char *path)
{
    // check if path is NULL
    if (path == NULL) {
        return 1;
    }

    // get information what is on given path (file, directory, etc.)
    struct stat st = {0};
    if (stat(path, &st) != 0) {
        return 0;
    }

    // remove file
    if (S_ISREG(st.st_mode)) {
        if (remove(path) != 0) {
            perror("\t[ERR]: remove() error");
            return 1;
        }
    } else if (S_ISDIR(st.st_mode)) {
        // remove everything from directory iteratively
        DIR *dir = opendir(path);
        struct dirent *entry = NULL;
        while ((entry = readdir(dir)) != NULL) {
            // skip . and ..
            if (cu_CStringCompare(entry->d_name, ".") == 0 || cu_CStringCompare(entry->d_name, "..") == 0) {
                continue;
            }

            char *entryPath = NULL;
            cu_CStringConcat(&entryPath, path);
            cu_CStringConcat(&entryPath, "/");
            cu_CStringConcat(&entryPath, entry->d_name);

            if (remove_recursive(entryPath) != 0) {
                free(entryPath);
                closedir(dir);
                return 1;
            }
            free(entryPath);
        }
        closedir(dir);

        // remove the directory itself
        if (rmdir(path) != 0) {
            perror("\t[ERR]: rmdir() error");
            return 1;
        }
    } else {
        printf("\t[ERR]: Unknown file type\n");
        return 1;
    }

    return 0;
}

int populationGenerate(const char *populationPath, uint32_t populationSize, uint32_t layerCount, uint32_t *layerSizes)
{
    // create initial generation directory
    char *genPath = NULL;
    cu_CStringConcat(&genPath, populationPath);
    cu_CStringConcat(&genPath, "/gen0");
    if (genPath == NULL) {
        perror("\t[ERR]: malloc() error");
        return 1;
    }
    if (mkdir_recursive(genPath, 0770) != 0) {
        printf("\t[ERR]: failed to create generation directory\n");
        free(genPath);
        return 1;
    }

    // create activation function array for each layer
    FnnActivation_e *activationFunctions = (FnnActivation_e *)malloc((layerCount - 1) * sizeof(FnnActivation_e));
    if (activationFunctions == NULL) {
        perror("\t[ERR]: malloc() error");
        free(genPath);
        return 1;
    }
    for (uint32_t i = 0; i < layerCount - 2; i++) {
        activationFunctions[i] = FNN_ACTIVATION_RELU;
    }
    activationFunctions[layerCount - 2] = FNN_ACTIVATION_SIGMOID;

    // create randmomized model for each individual of population
    for (uint32_t i = 0; i < populationSize; i++) {
        char *modelPath = (char *)malloc((cu_CStringLength(genPath) + 23) * sizeof(char));
        if (modelPath == NULL) {
            perror("\t[ERR]: malloc() error");
            free(genPath);
            return 1;
        }
        sprintf(modelPath, "%s/model_%u.fnnm", genPath, i);

        FnnModel *model = fnn_generateModel(layerSizes, activationFunctions, layerCount, -1.0f, 1.0f, 0.0f, 1.0f);
        if (model == NULL) {
            printf("\t[ERR]: fnn_generateModel() error\n");
            free(modelPath);
            free(genPath);
            free(activationFunctions);
            return 1;
        }

        if (fnn_serialize(modelPath, model) != 0) {
            printf("\t[ERR]: fnn_serialize() error\n");
            free(modelPath);
            free(genPath);
            free(activationFunctions);
            fnn_free(model);
            return 1;
        }

        free(modelPath);
        free(model->weightValues);
        free(model->biasValues);
        free(model);

        printf("\t%u/%u models generated\n", i + 1, populationSize);
    }

    free(genPath);
    free(activationFunctions);

    return 0;
}

int cmd_help(void)
{
    printf("Available commands:\n"
           "\thelp\t- show this help message\n"
           "\tversion\t- show program version\n"
           "\tpopcreate\t- create a new population\n"
           "\tpopload\t- load a population from file\n"
           "\tgenrun\t- start a new generation\n"
           "\tgenstat\t- show generation status\n"
           "\tinststat\t- show instance status\n"
           "\tinstkill\t- kill an instance\n"
           "\tinstmon\t- show instance details\n"
           "\tclear\t- clear the screen\n"
           "\texit\t- exit the program\n"
           "\n");

    return 0;
}

int cmd_version(void)
{
    printf("\tProgram:\t\tAstroMGR\n"
           "\tVersion:\t\tDEV (P3.0)\n"
           "\tCompiler version:\t"__VERSION__
           "\n"
           "\tCompiled on %s at %s\n",
           __DATE__, __TIME__);

    return 0;
}

int cmd_populationCreate(void)
{
    // local variables
    char *popPath = NULL;
    uint32_t popSize = 0;
    uint32_t layerCount = 0;
    uint32_t *layerSizes = NULL;

    // print current working directory
    char cwd[255];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("\tCurrent working directory: %s\n", cwd);
    } else {
        perror("\tgetcwd() error");
        return 1;
    }

    // ask user for population name (directory name)
    printf("\tPopulation root path: ");
    xString *popPathStr = xString_readInSafe(26);
    if (popPathStr == NULL) {
        return 1;
    } else if (xString_isEmpty(popPathStr)) {
        printf("\t[ERR]: Invalid population path\n");
        xString_free(popPathStr);
        return 0;
    }
    popPath = xString_toCString(popPathStr);
    xString_free(popPathStr);

    // check if the directory already exists
    struct stat st = {0};
    if (stat(popPath, &st) == 0) {
        printf("\tDirectory already exists. Overwrite? (y/n): ");
        char overwrite = getchar();
        getchar();  // consume newline
        if (overwrite != 'y' && overwrite != 'Y') {
            printf("\tOperation cancelled\n");
            free(popPath);
            return 0;
        } else {
            // clear the directory
            DIR *dir = opendir(popPath);
            struct dirent *entry = NULL;
            while ((entry = readdir(dir)) != NULL) {
                // skip . and ..
                if (cu_CStringCompare(entry->d_name, ".") == 0 || cu_CStringCompare(entry->d_name, "..") == 0) {
                    continue;
                }

                char *entryPath = NULL;
                cu_CStringConcat(&entryPath, popPath);
                cu_CStringConcat(&entryPath, "/");
                cu_CStringConcat(&entryPath, entry->d_name);

                if (remove_recursive(entryPath) != 0) {
                    printf("\t[ERR]: Failed to remove %s\n", entryPath);
                    free(entryPath);
                    free(popPath);
                    closedir(dir);
                    return 1;
                }
                free(entryPath);
            }
            closedir(dir);
        }
    }

    // ask user for population size
    printf("\tPopulation size: ");
    xString *popSizeStr = xString_readInSafe(6);
    if (popSizeStr == NULL) {
        free(popPath);
        return 1;
    } else if (xString_isEmpty(popSizeStr)) {
        printf("\t[ERR]: Invalid population size\n");
        free(popPath);
        xString_free(popSizeStr);
        return 0;
    }
    popSize = (uint32_t)xString_toInt(popSizeStr);
    xString_free(popSizeStr);

    if (popSize == 0) {
        printf("\t[ERR]: Population size cannot be zero\n");
        free(popPath);
        return 0;
    }

    // ask user for layer count
    printf("\tHidden layer count: ");
    xString *layerCountStr = xString_readInSafe(6);
    if (layerCountStr == NULL) {
        free(popPath);
        xString_free(popSizeStr);
        return 1;
    } else if (xString_isEmpty(layerCountStr)) {
        printf("\t[ERR]: Invalid hidden layer count\n");
        free(popPath);
        xString_free(popSizeStr);
        xString_free(layerCountStr);
        return 0;
    }
    layerCount = (uint32_t)xString_toInt(layerCountStr);
    xString_free(layerCountStr);
    if (layerCount == 0) {
        printf("\t[ERR]: At least one hidden layer is required\n");
        free(popPath);
        return 0;
    }

    // ask user for layer sizes
    layerSizes = (uint32_t *)malloc((layerCount + 2) * sizeof(uint32_t));
    if (layerSizes == NULL) {
        perror("\t[ERR]: malloc() error");
        free(popPath);
        return 1;
    }
    layerSizes[0] = 5;               // input layer size
    layerSizes[layerCount + 1] = 4;  // output layer size
    for (uint32_t i = 1; i <= layerCount; i++) {
        printf("\tHidden layer %u size: ", i);
        xString *layerSizeStr = xString_readInSafe(6);
        if (layerSizeStr == NULL) {
            free(popPath);
            free(layerSizes);
            return 1;
        } else if (xString_isEmpty(layerSizeStr)) {
            printf("\t[ERR]: Invalid hidden layer size\n");
            free(popPath);
            free(layerSizes);
            xString_free(layerSizeStr);
            return 0;
        }
        layerSizes[i] = (uint32_t)xString_toInt(layerSizeStr);
        xString_free(layerSizeStr);
        if (layerSizes[i] == 0) {
            printf("\t[ERR]: Layer size cannot be zero\n");
            free(popPath);
            free(layerSizes);
            return 0;
        }
    }

    populationGenerate(popPath, popSize, layerCount + 2, layerSizes);

    free(popPath);
    free(layerSizes);

    return 0;
}

int cmd_populationLoad(void)
{
    // local variables
    char *popPath = NULL;

    // print current working directory
    char cwd[255];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("\tCurrent working directory: %s\n", cwd);
    } else {
        perror("\tgetcwd() error");
        return 1;
    }

    // ask user for population name (directory name)
    printf("\tPopulation root path: ");
    xString *popPathStr = xString_readInSafe(26);
    if (popPathStr == NULL) {
        return 1;
    } else if (xString_isEmpty(popPathStr)) {
        printf("\t[ERR]: Invalid population path\n");
        xString_free(popPathStr);
        return 0;
    }
    popPath = xString_toCString(popPathStr);
    xString_free(popPathStr);

    if (mInstancer_loadPopulation(popPath) != 0) {
        printf("\t[ERR]: Failed to load population\n");
        free(popPath);
        return 1;
    } else {
        printf("\tPopulation loaded successfully\n");
    }

    free(popPath);

    return 0;
}

int cmd_generationStart(void)
{
    // ask user for max parallel instances and evolution iterations
    printf("\tMax parallel instances: ");
    xString *parallelCountStr = xString_readInSafe(6);
    if (parallelCountStr == NULL) {
        return 1;
    } else if (xString_isEmpty(parallelCountStr)) {
        printf("\t[ERR]: Invalid parallel instance count\n");
        xString_free(parallelCountStr);
        return 0;
    }
    mInstancer_setMaxParallel((uint32_t)xString_toInt(parallelCountStr));
    xString_free(parallelCountStr);

    printf("\tEvolution iterations: ");
    xString *iterationCountStr = xString_readInSafe(6);
    if (iterationCountStr == NULL) {
        return 1;
    } else if (xString_isEmpty(iterationCountStr)) {
        printf("\t[ERR]: Invalid evolution iteration count\n");
        xString_free(iterationCountStr);
        return 0;
    }
    mInstancer_setMaxIterations((uint32_t)xString_toInt(iterationCountStr));
    xString_free(iterationCountStr);

    if (mInstancer_startPopulation() != 0) {
        printf("\t[ERR]: Failed to start generation\n");
        return 1;
    }
    printf("\tStarting loaded generation\n");
    return 0;
}

// TODO: fix segfaulting when called during next generation creation process
int cmd_generationStatus(void)
{
    // get loaded population descriptors
    const xArray *descriptors = mInstancer_getAll();
    if (descriptors == NULL) {
        printf("\t[ERR]: No population loaded\n");
        return 1;
    }

    // print population information
    printf("ID  | MemID      | Status | Game PID | AI PID | Model path                     | Generation | Fitness score\n");
    for (int32_t i = 0; i < descriptors->size; i++) {
        const managerInstance_t *instance = (const managerInstance_t *)xArray_get(descriptors, i);
        printf("%3d | %10u | %6x |   %6d | %6d | %-30s | %10d | %11.2f\n", instance->instanceID, instance->sharedMemoryID,
               instance->status, instance->gamePID, instance->aiPID, instance->modelPath, instance->generation,
               instance->fitnessScore);
    }

    return 0;
}

int cmd_instanceStatus(void)
{
    // ask user for instance ID to lookup
    printf("\tInstance ID: ");
    xString *instanceIDStr = xString_readInSafe(6);
    if (instanceIDStr == NULL) {
        return 1;
    } else if (xString_isEmpty(instanceIDStr)) {
        printf("\t[ERR]: Invalid instance ID\n");
        xString_free(instanceIDStr);
        return 0;
    }
    uint32_t instanceID = (uint32_t)xString_toInt(instanceIDStr);
    xString_free(instanceIDStr);

    const managerInstance_t *instance = mInstancer_get(instanceID);
    if (instance == NULL) {
        printf("\t[ERR]: Instance not found\n");
        return 1;
    }

    printf("\t[ Instance %u ]\n"
           "\tShared memory ID: %u\n"
           "\tStatus: %x\n"
           "\tGame PID: %d\n"
           "\tAI PID: %d\n"
           "\tModel: %s\n"
           "\tGeneration: %d\n"
           "\tFitness score: %.2f\n",
           instance->instanceID, instance->sharedMemoryID, instance->status, instance->gamePID, instance->aiPID,
           instance->modelPath, instance->generation, instance->fitnessScore);

    return 0;
}

int cmd_instanceKill(void)
{
    // ask user for instance ID to kill
    printf("\tInstance ID: ");
    xString *instanceIDStr = xString_readInSafe(6);
    if (instanceIDStr == NULL) {
        return 1;
    } else if (xString_isEmpty(instanceIDStr)) {
        printf("\t[ERR]: Invalid instance ID\n");
        xString_free(instanceIDStr);
        return 0;
    }
    uint32_t instanceID = (uint32_t)xString_toInt(instanceIDStr);
    xString_free(instanceIDStr);

    if (mInstancer_killIndividual(instanceID) != 0) {
        printf("\t[ERR]: Failed to kill instance\n");
        return 1;
    }

    printf("\tInstance %u killed\n", instanceID);

    return 0;
}

int cmd_instanceShow(void)
{
    printf("\tShowing instance details...\n");
    return 0;
}

int cmd_clear(void)
{
    printf("\033[H\033[J");
    return 0;
}
