/**
 * @file programManager.c
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Managing program. Will be used to run the game and the AI as well as creating and destroying shared memory between them.
 * @version 0.1
 * @date 07.11.2023.
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "main.h"  // enums, structs, etc.

#include <fcntl.h>      // file flags
#include <signal.h>     // kill, SIGTERM
#include <stdio.h>      // input/output
#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // waitpid
#include <time.h>       // time functions
#include <unistd.h>     // execv, fork, etc.

#include "commonUtility.h"  // common utility functions
#include "sharedMemory.h"   // shared memory functions
#include "xArray.h"         // dynamic array and its functions
#include "xString.h"        // string functions

// global ID counters
static unsigned short instanceID = 0;    // instance id counter
static unsigned short generationID = 0;  // generation id counter

// instance descriptor structure
typedef struct instance_s {
    // instance tracking information
    unsigned int id;            // instance id
    unsigned int generationID;  // generation id
    pid_t gamePid;              // game process id
    pid_t aiPid;                // ai process id
    short int flags;            // instance flags

    // instance evaluation information (used later for genetic algorithm)
    unsigned int gameScore;  // score of the game
    double gameDuration;     // duration of the game

    // instance shared memory pointers (created and freed as instance progresses)
    struct sharedInput_s *sharedInput;    // instance shared input
    struct sharedOutput_s *sharedOutput;  // instance shared output
    struct sharedState_s *sharedState;    // instance shared state
} Instance;

// function declarations
Instance *createInstance(unsigned int id, unsigned int generationID, short int flags);
int createSharedMemory(char *sharedInputKey, char *sharedOutputKey, char *sharedStateKey, struct sharedInput_s **sharedInput, struct sharedOutput_s **sharedOutput, struct sharedState_s **sharedState);

// manager entry point
int main(void) {
    short int flags_config = 0;    // flags for setting up the program
    short int input_selector = 0;  // input selector for the menu

    // shared memory key input containers
    char sharedInputKey[255] = {0};
    char sharedOutputKey[255] = {0};
    char sharedStateKey[255] = {0};

    // instance descriptor array
    xArray *instances = xArray_new();

    // TODO: create generation "multimap" (generationID -> instance_list)

    // shared memory access key arrays
    xArray *sharedInputKeys = xArray_new();
    xArray *sharedOutputKeys = xArray_new();
    xArray *sharedStateKeys = xArray_new();

    // main menu screen
    printf("    ___         __                  __  _____________ \n");
    printf("   /   |  _____/ /____  _________  /  |/  / ____/ __ \\\n");
    printf("  / /| | / ___/ __/ _ \\/ ___/ __ \\/ /|_/ / / __/ /_/ /\n");
    printf(" / ___ |(__  ) /_/  __/ /  / /_/ / /  / / /_/ / _, _/ \n");
    printf("/_/  |_/____/\\__/\\___/_/   \\____/_/  /_/\\____/_/ |_|  \n");
    printf("                                                      \n");
    printf("\n");
    printf("Welcome to the AsteroMGR!\n");
    printf("This program will manage the game and the AI instances.\n");
    printf("It will also create and destroy shared memory between them.\n");
    printf("\n");
    printf("Press enter to continue...\n");
    getchar();

    //  printing out the menu
    while ((flags_config & RUNTIME_EXIT) == 0) {
        // system("clear");
        printf("=============================================\n");
        printf("[1] Create instance (game-AI pair)\n");   // creates game-AI pair and starts both
        printf("[2] Create game\n");                      // creates game-only instance and starts it (with all shared memory)
        printf("[3] Create AI\n");                        // creates AI-only instance and starts it (with all shared memory)
        printf("[4] Create instance batch\n");            // creates multiple game-AI pairs and starts them (useful for creating whole generations)
        printf("[5] Destroy instance (game-AI pair)\n");  // destroys game-AI pair and stops both
        printf("[6] Destroy game\n");                     // destroys game-only instance and stops it (with all shared memory)
        printf("[7] Destroy AI\n");                       // destroys AI-only instance and stops it (with all shared memory)
        printf("[8] Destroy instance batch\n");           // destroys multiple game-AI pairs and stops them (useful for destroying whole generations)
        printf("[9] Refresh instances\n");                // refreshes flags of all running instances
        printf("[10] Save instance results\n");           // saves instance results to file
        printf("[11] Save instance batch results\n");     // saves instance batch results to file
        printf("[12] Exit\n");                            // exits the program
        printf("[13] [DEBUG] Print instance states\n");   // prints out the shared state memory of all instances
        printf("\nChoose an option: ");
        scanf("%hd", &input_selector);

        // TODO: create functionality from menu above

        // TODO: add some test input to the game over shared memory and print changes

        if (input_selector > 13 || input_selector < 1) {
            printf("Invalid input!\n\n");
            continue;
        }

        switch (input_selector) {
            case 1:
                printf("=============================================\n");
                printf("Feature not implemented yet!\n");
                break;
            case 2:
                // test access to game executable
                char pwd[255] = {0};
                getcwd(pwd, 255);
                printf("\nCurrent working directory: %s\n", pwd);

                if (access("./bin/game", X_OK) != 0) {
                    printf("\nFailed to access game executable!\n");
                    exit(EXIT_FAILURE);
                }

                // get arguments for shared memory keys
                printf("\n=============================================\n");
                printf("Enter the key for the shared input memory: ");
                getchar();  // Clear the input buffer
                fgets(sharedInputKey, 255, stdin);
                printf("Enter the key for the shared output memory: ");
                fgets(sharedOutputKey, 255, stdin);
                printf("Enter the key for the shared state memory: ");
                fgets(sharedStateKey, 255, stdin);

                // remove newline characters from the end of the strings
                cu_CStringTrimNewline(sharedInputKey);
                cu_CStringTrimNewline(sharedOutputKey);
                cu_CStringTrimNewline(sharedStateKey);

                // validate shared memory keys
                if (!sm_validateSharedMemoryName(sharedInputKey) || !sm_validateSharedMemoryName(sharedOutputKey) || !sm_validateSharedMemoryName(sharedStateKey)) {
                    printf("Invalid shared memory keys!\n\n");
                    continue;
                }

                // append 5-digit number to the end of the shared memory keys (instance ID)
                sprintf(sharedInputKey, "%s%05d", sharedInputKey, instanceID);
                sprintf(sharedOutputKey, "%s%05d", sharedOutputKey, instanceID);
                sprintf(sharedStateKey, "%s%05d", sharedStateKey, instanceID);

                // add shared memory keys to the array
                xArray_push(sharedInputKeys, xString_fromCString(sharedInputKey));
                xArray_push(sharedOutputKeys, xString_fromCString(sharedOutputKey));
                xArray_push(sharedStateKeys, xString_fromCString(sharedStateKey));

                printf("Starting the game...\n");

                // create instance descriptor
                Instance *instance = createInstance(instanceID, generationID, 0);
                if (instance == NULL) {
                    printf("Failed to create instance descriptor!\n");
                    exit(EXIT_FAILURE);
                }

                // create shared memory
                if (!createSharedMemory(sharedInputKey, sharedOutputKey, sharedStateKey, &instance->sharedInput, &instance->sharedOutput, &instance->sharedState)) {
                    printf("Failed to create shared memory!\n");
                    exit(EXIT_FAILURE);
                }

                // add instance to the array
                xArray_push(instances, instance);

                // increment instance ID for next instance
                instanceID++;

                // start the game with --managed <sharedInputKey> <sharedOutputKey> <sharedStateKey>
                char *gameArgs[] = {"./bin/game", "--managed", sharedInputKey, sharedOutputKey, sharedStateKey, NULL};
                pid_t gamePid = fork();

                if (gamePid == 0) {
                    setsid();  // detach from parent process

                    // redirect stdout to /dev/null
                    // TODO: redirect output to runtime log files
                    int devNull = open("/dev/null", O_WRONLY);
                    if (devNull == -1) {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }
                    dup2(devNull, STDOUT_FILENO);
                    close(devNull);

                    execv("./bin/game", gameArgs);  // start the game
                } else if (gamePid > 0) {
                    instance->gamePid = gamePid;  // save game process id to instance descriptor

                    // wait for game to update shared memory
                    int gameReady = 0;
                    while (gameReady == 0) {
                        usleep(500);
                        sm_lockSharedState(instance->sharedState);
                        gameReady = instance->sharedState->state_gameAlive;
                        sm_unlockSharedState(instance->sharedState);
                    }
                } else {
                    printf("Failed to start the game!\n");
                    exit(EXIT_FAILURE);
                }

                printf("Game started!\n");
                break;
            case 3:
                printf("=============================================\n");
                printf("Feature not implemented yet!\n");
                break;
            case 4:
                printf("=============================================\n");
                printf("Feature not implemented yet!\n");
                break;
            case 5:
                printf("=============================================\n");
                printf("Feature not implemented yet!\n");
                break;
            case 6:
                // prompt user to enter instance ID (out of available range)
                printf("=============================================\n");
                printf("Enter the instance ID: ");
                scanf("%hu", &input_selector);

                // check if instance ID is valid
                if (input_selector >= instanceID) {
                    printf("Invalid instance ID!\n\n");
                    continue;
                }

                // get instance descriptor from array
                instance = xArray_get(instances, input_selector);

                // stop the game
                printf("Sending SIGKILL to %d...\n", instance->gamePid);
                if (instance->gamePid != 0) {
                    int status;
                    kill(instance->gamePid, SIGKILL);
                    waitpid(instance->gamePid, &status, 0);  // wait for game to exit

                    instance->gamePid = 0;  // unset game PID

                    // close shared memory
                    char *sharedMemKey = xString_toCString((xString *)xArray_get(sharedInputKeys, instance->id));
                    sm_freeSharedInput(instance->sharedInput, sharedMemKey);
                    free(sharedMemKey);

                    sharedMemKey = xString_toCString((xString *)xArray_get(sharedOutputKeys, instance->id));
                    sm_freeSharedOutput(instance->sharedOutput, sharedMemKey);
                    free(sharedMemKey);

                    sharedMemKey = xString_toCString((xString *)xArray_get(sharedStateKeys, instance->id));
                    sm_freeSharedState(instance->sharedState, sharedMemKey);
                    free(sharedMemKey);

                    printf("Game [IID=%u] stopped!\n", instance->id);
                } else {
                    printf("Instance [IID=%u] has no game running!\n", instance->id);
                }
                break;
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
                printf("=============================================\n");
                printf("Feature not implemented yet!\n");
                continue;
            case 12:
                printf("=============================================\n");
                printf("Exiting...\n");

                // stop all running instances
                for (int i = 0; i < instances->size; i++) {
                    instance = xArray_get(instances, i);

                    // stop the game
                    printf("Sending SIGKILL to %d...\n", instance->gamePid);
                    if (instance->gamePid != 0) {
                        int status;
                        kill(instance->gamePid, SIGKILL);
                        waitpid(instance->gamePid, &status, 0);  // wait for game to exit

                        instance->gamePid = 0;  // unset game PID

                        // close shared memory
                        char *sharedMemKey = xString_toCString((xString *)xArray_get(sharedInputKeys, instance->id));
                        sm_freeSharedInput(instance->sharedInput, sharedMemKey);
                        free(sharedMemKey);

                        sharedMemKey = xString_toCString((xString *)xArray_get(sharedOutputKeys, instance->id));
                        sm_freeSharedOutput(instance->sharedOutput, sharedMemKey);
                        free(sharedMemKey);

                        sharedMemKey = xString_toCString((xString *)xArray_get(sharedStateKeys, instance->id));
                        sm_freeSharedState(instance->sharedState, sharedMemKey);
                        free(sharedMemKey);

                        printf("Game [IID=%u] stopped!\n", instance->id);
                    } else {
                        printf("Instance [IID=%u] has no game running!\n", instance->id);
                    }
                }

                // free all memory
                xArray_forEach(instances, free);
                xArray_free(instances);
                xArray_forEach(sharedInputKeys, (void (*)(void *))xString_free);
                xArray_free(sharedInputKeys);
                xArray_forEach(sharedOutputKeys, (void (*)(void *))xString_free);
                xArray_free(sharedOutputKeys);
                xArray_forEach(sharedStateKeys, (void (*)(void *))xString_free);
                xArray_free(sharedStateKeys);

                exit(EXIT_SUCCESS);
                break;
            default:
                printf("Invalid input!\n\n");
                continue;
        }
    }

    // free all memory
    xArray_forEach(instances, free);
    xArray_free(instances);
    xArray_forEach(sharedInputKeys, (void (*)(void *))xString_free);
    xArray_free(sharedInputKeys);
    xArray_forEach(sharedOutputKeys, (void (*)(void *))xString_free);
    xArray_free(sharedOutputKeys);
    xArray_forEach(sharedStateKeys, (void (*)(void *))xString_free);
    xArray_free(sharedStateKeys);

    return 0;
}

Instance *createInstance(unsigned int id, unsigned int generationID, short int flags) {
    // allocate instance descriptor
    Instance *instance = malloc(sizeof(Instance));
    if (instance == NULL) {
        return NULL;
    }

    // initialize instance tracking information
    instance->id = id;
    instance->generationID = generationID;
    instance->gamePid = 0;
    instance->aiPid = 0;
    instance->flags = flags;

    // initialize instance evaluation information
    instance->gameScore = 0;
    instance->gameDuration = 0.0;

    // initialize instance shared memory pointers
    instance->sharedInput = NULL;
    instance->sharedOutput = NULL;
    instance->sharedState = NULL;

    return instance;
}

int createSharedMemory(char *sharedInputKey, char *sharedOutputKey, char *sharedStateKey, struct sharedInput_s **sharedInput, struct sharedOutput_s **sharedOutput, struct sharedState_s **sharedState) {
    // assume that shared memory keys are valid (validated in main)

    // allocate shared memory for input
    *sharedInput = sm_allocateSharedInput(sharedInputKey);
    if (*sharedInput == NULL) {
        return 0;
    }

    // allocate shared memory for output
    *sharedOutput = sm_allocateSharedOutput(sharedOutputKey);
    if (*sharedOutput == NULL) {
        return 0;
    }

    // allocate shared memory for state
    *sharedState = sm_allocateSharedState(sharedStateKey);
    if (*sharedState == NULL) {
        return 0;
    }

    // initialize shared memory for input
    sm_lockSharedInput(*sharedInput);
    (*sharedInput)->isKeyDownW = 0;
    (*sharedInput)->isKeyDownA = 0;
    (*sharedInput)->isKeyDownD = 0;
    (*sharedInput)->isKeyDownSpace = 0;
    sm_unlockSharedInput(*sharedInput);

    // initialize shared memory for output
    sm_lockSharedOutput(*sharedOutput);
    (*sharedOutput)->playerPosX = 0.0f;
    (*sharedOutput)->playerPosY = 0.0f;
    (*sharedOutput)->playerRotation = 0.0f;
    (*sharedOutput)->playerSpeedX = 0.0f;
    (*sharedOutput)->playerSpeedY = 0.0f;
    (*sharedOutput)->distanceFront = 0.0f;
    (*sharedOutput)->closestAsteroidPosX = 0.0f;
    (*sharedOutput)->closestAsteroidPosY = 0.0f;
    sm_unlockSharedOutput(*sharedOutput);

    // initialize shared memory for state
    sm_lockSharedState(*sharedState);
    (*sharedState)->state_gameAlive = 0;
    (*sharedState)->state_managerAlive = 0;
    (*sharedState)->state_neuronsAlive = 0;
    (*sharedState)->game_isPaused = 0;
    (*sharedState)->game_runHeadless = 0;
    (*sharedState)->game_gameScore = 0;
    (*sharedState)->game_gameLevel = 0;
    (*sharedState)->game_gameTime = 0.0;
    sm_unlockSharedState(*sharedState);

    return 1;
}
