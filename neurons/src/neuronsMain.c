#include "neuronsMain.h"  // neural network program related enums, structs, etc.

#include <math.h>     // math functions
#include <signal.h>   // signal handling (will be used for graceful exit)
#include <stdbool.h>  // boolean type
#include <stdio.h>    // console input/output
#include <stdlib.h>   // malloc, free, etc.
#include <time.h>     // time functions (for random number generation)

#include "commonUtility.h"  // common utility functions
#include "sharedMemory.h"   // shared memory
#include "xLinear.h"        // matrix operations
#include "xString.h"        // string operations (for parsing command line arguments)

// ----------------------------------------------------------------------------------------------
// global variables

struct sigaction sigact;  // signal action for graceful exit

static char *cmd_configFilename = NULL;  // TODO: parse and load weights/biases from config file
static char *cmd_shInputName = NULL;     // shared input memory name
static char *cmd_shOutputName = NULL;    // shared output memory name
static char *cmd_shStateName = NULL;     // shared state memory name
static struct sharedInput_s *shInput = NULL;
static struct sharedOutput_s *shOutput = NULL;
static struct sharedState_s *shState = NULL;

static unsigned short flags_cmd = CMD_FLAG_NONE;  // command line argument flags
// static unsigned short flags_input;    // game input flags (W, A, D, SPACE)
static unsigned short flags_runtime;  // program runtime flags (running, paused, exit, etc.)

xMatrix *weights1 = NULL;      // input(8) -> hidden layer(32)
xMatrix *weights2 = NULL;      // hidden layer(32) -> output(4)
xMatrix *bias1 = NULL;         // bias for hidden layer (1x32)
xMatrix *bias2 = NULL;         // bias for output layer (1x4)

xMatrix *input = NULL;         // input matrix (1x8)
xMatrix *intermediate = NULL;  // intermediate matrix (1x32)
xMatrix *output = NULL;        // output matrix (1x4)

// ----------------------------------------------------------------------------------------------
// local function declarations

static inline void OpenSharedMemory(void);                       // open shared memory
static inline void CloseSharedMemory(void);                      // close shared memory
static inline void UpdateSharedState(void);                      // update state from shared memory
static inline void UpdateSharedInput(void);                      // update input from shared memory, game input (NN output)
static inline void UpdateSharedOutput(void);                     // update output to shared memory, game output (NN input)
static void fillUniform(xMatrix *mat, float min, float max);     // fill matrix with random values in from uniform distribution
static float normalRandom(float mean, float stddev);             // generate normally distributed random number (using Box-Muller transform)
static void fillNormal(xMatrix *mat, float mean, float stddev);  // fill matrix with random values from normal distribution
static inline float activation(float x);                         // neural network activation function
static inline void InitNeurons(void);                            // initialize neural network
static inline void UpdateNeurons(void);                          // update neural network (one frame)
static inline void UnloadNeurons(void);                          // unload dynamic structures of neural network
static void signalHandler(int signal);                           // signal handler for graceful exit

// ----------------------------------------------------------------------------------------------
// program entry point (main)

int main(int argc, char *argv[]) {
    // parsing command line arguments
    if (argc == 1) {
        printf("No command line arguments provided.\n");
        printf("Use -h or --help for more information.\n");
        return 0;
    } else {
        int i;
        for (i = 1; i < argc; i++) {
            xString *arg = xString_fromCString(argv[i]);
            if (xString_isEqualCString(arg, "-h") || xString_isEqualCString(arg, "--help")) {
                flags_cmd |= CMD_FLAG_HELP;
            } else if (xString_isEqualCString(arg, "-v") || xString_isEqualCString(arg, "--version")) {
                flags_cmd |= CMD_FLAG_VERSION;
            } else if (xString_isEqualCString(arg, "-s") || xString_isEqualCString(arg, "--standalone")) {
                if (i + 2 > argc) break;

                flags_cmd |= CMD_FLAG_STANDALONE;
                cmd_shInputName = argv[i + 1];
                cmd_shOutputName = argv[i + 2];

                // DEBUG: print shared memory names
                printf("shInputName: %s\n", cmd_shInputName);
                printf("shOutputName: %s\n", cmd_shOutputName);

                i += 2;
            } else if (xString_isEqualCString(arg, "-m") || xString_isEqualCString(arg, "--managed")) {
                if (i + 3 > argc) break;

                flags_cmd |= CMD_FLAG_MANAGED;
                cmd_shInputName = argv[i + 1];
                cmd_shOutputName = argv[i + 2];
                cmd_shStateName = argv[i + 3];

                // DEBUG: print shared memory names
                printf("shInputName: %s\n", cmd_shInputName);
                printf("shOutputName: %s\n", cmd_shOutputName);
                printf("shStateName: %s\n", cmd_shStateName);

                i += 3;
            } else if (xString_isEqualCString(arg, "-l") || xString_isEqualCString(arg, "--load")) {
                if (i + 1 > argc) break;

                flags_cmd |= CMD_FLAG_LOADCFG;
                cmd_configFilename = argv[i + 1];

                // DEBUG: print config filename
                printf("configFilename: %s\n", cmd_configFilename);

                i += 1;
            } else {
                printf("ERROR: Unknown command line argument: %s\n", argv[i]);
                printf("Use %s --help for more information.\n", argv[0]);
                xString_free(arg);
                return 1;
            }
            xString_free(arg);
        }
        if (i != argc) {
            printf("ERROR: Invalid command line arguments.\n");
            printf("Use %s --help for more information.\n", argv[0]);
            return 1;
        }
    }

    // check flag conflicts
    if ((flags_cmd & CMD_FLAG_STANDALONE && flags_cmd & CMD_FLAG_MANAGED) ||  // managed mode extends standalone mode
        (flags_cmd & CMD_FLAG_HELP && flags_cmd & ~CMD_FLAG_HELP) ||          // help flag is exclusive
        (flags_cmd & CMD_FLAG_VERSION && flags_cmd & ~CMD_FLAG_VERSION)) {    // version flag is exclusive
        printf("ERROR: Invalid command line arguments.\n");
        printf("Use %s --help for more information.\n", argv[0]);
        return 1;
    }

    // parse help or version flag
    if (flags_cmd & CMD_FLAG_HELP) {
        printf("Usage: %s [OPTIONS]\n", argv[0]);
        printf("Neural network program.\n");
        printf("\n");
        printf("Options:\n");
        printf("  -h, --help\t\t\t\t\tPrint this help message and exit.\n");
        printf("  -v, --version\t\t\t\t\tPrint version information and exit.\n");
        printf("  -s, --standalone <input> <output>\t\tRun in standalone mode.\n");
        printf("  -m, --managed <input> <output> <state>\tRun in managed mode.\n");
        printf("  -l, --load <config>\t\t\t\tLoad configuration file.\n");
        printf("\n");
        printf("Standalone mode:\n");
        printf("  <input>\tShared memory name for input.\n");
        printf("  <output>\tShared memory name for output.\n");
        printf("\n");
        printf("Managed mode:\n");
        printf("  <input>\tShared memory name for input.\n");
        printf("  <output>\tShared memory name for output.\n");
        printf("  <state>\tShared memory name for state.\n");
        printf("\n");
        printf("Configuration file:\n");
        printf("  <config>\tConfiguration file path.\n");
        printf("\n");
        printf("Shared memory name:\n");
        printf("  Shared memory name must start with a slash and contain only alphanumeric characters.\n");
        printf("  Maximum length is 255 characters.\n");
        printf("\n");
        return 0;
    } else if (flags_cmd & CMD_FLAG_VERSION) {
        printf("Program:\t\tAsteroids-Neurons\n");
        printf("Version:\t\tDEV P2.0\n");
        printf("Compiler version:\t%s\n", __VERSION__);
        printf("Compiled on %s at %s\n", __DATE__, __TIME__);
        return 0;
    }

    // flag arguments (shared memory names) should only be alphanumeric strings
    if (flags_cmd & (CMD_FLAG_STANDALONE | CMD_FLAG_MANAGED)) {
        if (!cu_CStringIsAlphanumeric(cmd_shInputName + 1) ||
            !cu_CStringIsAlphanumeric(cmd_shOutputName + 1) ||
            (cmd_shStateName != NULL && !cu_CStringIsAlphanumeric(cmd_shStateName + 1))) {
            printf("ERROR: Invalid shared memory name.\n");
            printf("Use %s --help for more information.\n", argv[0]);
            return 1;
        }
    }

    // initialize neural network, connect to shared memory and register signal handler
    InitNeurons();

    // neural network main loop
    while(!(flags_runtime & RUNTIME_EXIT)) {
        UpdateNeurons();
    }

    // free dynamic structures
    UnloadNeurons();

    return 0;
}

// ----------------------------------------------------------------------------------------------
// local function definitions

// connect to shared memory if in appropriate mode
inline void OpenSharedMemory(void) {
    if (flags_cmd & CMD_FLAG_STANDALONE) {
        shInput = sm_connectSharedInput(cmd_shInputName);
        shOutput = sm_connectSharedOutput(cmd_shOutputName);

        if (shInput == NULL || shOutput == NULL) {
            printf("ERROR: Failed to connect to shared memory.\n");
            exit(1);
        }
    } else if (flags_cmd & CMD_FLAG_MANAGED) {
        shInput = sm_connectSharedInput(cmd_shInputName);
        shOutput = sm_connectSharedOutput(cmd_shOutputName);
        shState = sm_connectSharedState(cmd_shStateName);

        if (shInput == NULL || shOutput == NULL || shState == NULL) {
            printf("ERROR: Failed to connect to shared memory.\n");
            exit(1);
        }
    }

    // shared memory should already be initialized by the game or manager
    return;
}

// disconnect from shared memory if in appropriate mode
inline void CloseSharedMemory(void) {
    if (flags_cmd & CMD_FLAG_STANDALONE) {
        sm_disconnectSharedInput(shInput);
        sm_disconnectSharedOutput(shOutput);
    } else if (flags_cmd & CMD_FLAG_MANAGED) {
        sm_disconnectSharedInput(shInput);
        sm_disconnectSharedOutput(shOutput);
        sm_disconnectSharedState(shState);
    }

    // clear dangling pointers
    shInput = NULL;
    shOutput = NULL;
    shState = NULL;
    return;
}

// update state from shared memory (if in managed mode)
inline void UpdateSharedState(void) {
    if (flags_cmd & CMD_FLAG_MANAGED) {
        sm_lockSharedState(shState);
        if (shState->control_neuronsExit) {
            flags_runtime |= RUNTIME_EXIT;
        }
        sm_unlockSharedState(shState);
    }
    return;
}

// update input to shared memory, game input (NN output)
inline void UpdateSharedInput(void) {
    // check output matrix dimension
    if (output->rows != 1 || output->cols != 4) {
        printf("ERROR: Invalid output matrix dimension.\n");
        exit(1);
    }

    // update values from output matrix
    sm_lockSharedInput(shInput);
    shInput->isKeyDownW = (xMatrix_get(output, 0, 0) > ACTIVATION_THRESHOLD) ? true : false;
    shInput->isKeyDownA = (xMatrix_get(output, 0, 1) > ACTIVATION_THRESHOLD) ? true : false;
    shInput->isKeyDownD = (xMatrix_get(output, 0, 2) > ACTIVATION_THRESHOLD) ? true : false;
    shInput->isKeyDownSpace = (xMatrix_get(output, 0, 3) > ACTIVATION_THRESHOLD) ? true : false;
    sm_unlockSharedInput(shInput);

    return;
}

// update output from shared memory, game output (NN input)
inline void UpdateSharedOutput(void) {
    // check input matrix dimension
    if (input->rows != 1 || input->cols != 8) {
        printf("ERROR: Invalid input matrix dimension.\n");
        exit(1);
    }

    // update values to input matrix
    sm_lockSharedOutput(shOutput);
    xMatrix_set(input, 0, 0, shOutput->playerPosX);
    xMatrix_set(input, 0, 1, shOutput->playerPosY);
    xMatrix_set(input, 0, 2, shOutput->playerRotation);
    xMatrix_set(input, 0, 3, shOutput->playerSpeedX);
    xMatrix_set(input, 0, 4, shOutput->playerSpeedY);
    xMatrix_set(input, 0, 5, shOutput->distanceFront);
    xMatrix_set(input, 0, 6, shOutput->closestAsteroidPosX);
    xMatrix_set(input, 0, 7, shOutput->closestAsteroidPosY);
    sm_unlockSharedOutput(shOutput);

    return;
}

// fill matrix with random values in from uniform distribution
void fillUniform(xMatrix *mat, float min, float max) {
    srand(time(NULL));
    for (int i = 0; i < mat->rows; i++) {
        for (int j = 0; j < mat->cols; j++) {
            xMatrix_set(mat, i, j, min + (max - min) * rand() / RAND_MAX);
        }
    }
}

// fill matrix with random values from normal distribution
void fillNormal(xMatrix *mat, float mean, float stddev) {
    srand(time(NULL));
    for (int i = 0; i < mat->rows; i++) {
        for (int j = 0; j < mat->cols; j++) {
            xMatrix_set(mat, i, j, normalRandom(mean, stddev));
        }
    }
}

// generate normally distributed random number (using Box-Muller transform)
float normalRandom(float mean, float stddev) {
    static float n2 = 0.0;
    static int n2_cached = 0;
    if (!n2_cached) {
        float x, y, r;
        do {
            x = 2.0f * rand() / RAND_MAX - 1;
            y = 2.0f * rand() / RAND_MAX - 1;

            r = x * x + y * y;
        } while (r == 0.0f || r > 1.0f);

        float d = sqrt(-2.0f * log(r) / r);
        float n1 = x * d;
        n2 = y * d;

        float result = n1 * stddev + mean;
        n2_cached = 1;
        return result;
    } else {
        n2_cached = 0;
        return n2 * stddev + mean;
    }
}

// sigmoid activation function
inline float activation(float x) {
    return 1.0f / (1.0f + expf(-x));
}

// initialize neural network program
inline void InitNeurons(void) {
    // initialize matrices for network
    weights1 = xMatrix_new(8, 32);  // input(8) -> hidden layer(32)
    weights2 = xMatrix_new(32, 4);  // hidden layer(32) -> output(4)
    bias1 = xMatrix_new(1, 32);     // bias for hidden layer
    bias2 = xMatrix_new(1, 4);      // bias for output layer
    input = xMatrix_new(1, 8);      // input matrix (1x8)
    intermediate = xMatrix_new(1, 32);
    output = xMatrix_new(1, 4);  // output matrix (1x4)

    // load matrices from file or generate random
    if (flags_cmd & CMD_FLAG_LOADCFG) {
        // TODO: implement parsing and loading matrices from file
    } else {
        // initialize random matrices for network
        fillUniform(weights1, -0.5f, 0.5f);
        fillUniform(weights2, -0.5f, 0.5f);
        fillNormal(bias1, 0.0f, 0.001f);
        fillNormal(bias2, 0.0f, 0.001f);
    }

    // connect to shared memory
    OpenSharedMemory();

    // initialize and register signal handler for graceful exit
    sigact.sa_handler = signalHandler;
    sigact.sa_flags = SA_NODEFER;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);

    // set runtime flags
    flags_runtime |= RUNTIME_RUNNING;

    // report to shared state that program is running
    if (flags_cmd & CMD_FLAG_MANAGED) {
        sm_lockSharedState(shState);
        shState->state_neuronsAlive = true;
        sm_unlockSharedState(shState);
    }

    return;
}

// update neural network (one frame)
inline void UpdateNeurons(void) {
    // update state from shared memory
    UpdateSharedState();

    // update output from shared memory, game output (NN input)
    UpdateSharedOutput();

    // calculate intermediate matrix
    xMatrix_dot(intermediate, input, weights1);
    xMatrix_add(intermediate, intermediate, bias1);

    // apply activation function to intermediate matrix
    for (int i = 0; i < intermediate->rows; i++) {
        for (int j = 0; j < intermediate->cols; j++) {
            xMatrix_set(intermediate, i, j, activation(xMatrix_get(intermediate, i, j)));
        }
    }

    // calculate output matrix
    xMatrix_dot(output, intermediate, weights2);
    xMatrix_add(output, output, bias2);

    // apply activation function to output matrix
    for (int i = 0; i < output->rows; i++) {
        for (int j = 0; j < output->cols; j++) {
            xMatrix_set(output, i, j, activation(xMatrix_get(output, i, j)));
        }
    }

    // update input to shared memory, game input (NN output)
    UpdateSharedInput();
}

// unload dynamic structures of neural network
inline void UnloadNeurons(void) {
    // report to shared state that program is not running
    if (flags_cmd & CMD_FLAG_MANAGED) {
        sm_lockSharedState(shState);
        shState->state_neuronsAlive = false;
        sm_unlockSharedState(shState);
    }

    // disconnect from shared memory
    CloseSharedMemory();

    // free dynamic structures
    xMatrix_free(weights1);
    xMatrix_free(weights2);
    xMatrix_free(bias1);
    xMatrix_free(bias2);
    xMatrix_free(input);
    xMatrix_free(intermediate);
    xMatrix_free(output);

    return;
}

// signal handler for graceful exit (when SIGINT or SIGTERM is received)
void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        flags_runtime |= RUNTIME_EXIT;

        // return back to where program was interrupted
        return;
    }
}
