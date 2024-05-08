#include "neuronsMain.h"  // neural network program related enums, structs, etc.

#include <math.h>     // math functions
#include <signal.h>   // signal handling (will be used for graceful exit)
#include <stdbool.h>  // boolean type
#include <stdio.h>    // console input/output
#include <stdlib.h>   // malloc, free, etc.
#include <time.h>     // time functions (for random number generation)

#include "commonUtility.h"  // common utility functions
#include "fnnLoader.h"      // feedforward neural network loader (.fnnm file format)
#include "sharedMemory.h"   // shared memory
#include "xLinear.h"        // matrix operations
#include "xList.h"          // list structure and operations
#include "xString.h"        // string operations (for parsing command line arguments)

// ----------------------------------------------------------------------------------------------
// global variables

struct sigaction sigact;  // signal action for graceful exit

static char *cmd_configFilename = NULL;  // path to pre-generated model file
static char *cmd_shInputName = NULL;     // shared input memory name
static char *cmd_shOutputName = NULL;    // shared output memory name
static char *cmd_shStateName = NULL;     // shared state memory name
static struct sharedInput_s *shInput = NULL;
static struct sharedOutput_s *shOutput = NULL;
static struct sharedState_s *shState = NULL;

static unsigned short flags_cmd = CMD_FLAG_NONE;  // command line argument flags
static unsigned short flags_runtime;              // program runtime flags (running, paused, exit, etc.)

xList *weightMatrices = NULL;        // list of weight matrices
xList *biasMatrices = NULL;          // list of bias matrices
xList *intermediateMatrices = NULL;  // list of intermediate matrices (results of each layer)
xList *activationFunctions = NULL;   // list of activation functions for each layer

xMatrix *input = NULL;   // input matrix (1x8)
xMatrix *output = NULL;  // output matrix (1x4)

// ----------------------------------------------------------------------------------------------
// local function declarations

static inline void OpenSharedMemory(void);                    // open shared memory
static inline void CloseSharedMemory(void);                   // close shared memory
static inline void UpdateSharedState(void);                   // update state from shared memory
static inline void UpdateSharedInput(void);                   // update input from shared memory, game input (NN output)
static inline void UpdateSharedOutput(void);                  // update output to shared memory, game output (NN input)
static void fillUniform(xMatrix *mat, float min, float max);  // fill matrix with random values in from uniform distribution
static float normalRandom(float mean, float stddev);  // generate normally distributed random number (using Box-Muller transform)
static void fillNormal(xMatrix *mat, float mean, float stddev);  // fill matrix with random values from normal distribution
static inline float activation_none(float x);                    // neural network pass-through activation function
static inline float activation_sigmoid(float x);                 // neural network sigmoid function
static inline float activation_reLU(float x);                    // neural network reLU activation function
static inline float activation_tanh(float x);                    // neural network tanh activation function
static inline void InitNeurons(void);                            // initialize neural network
static inline void UpdateNeurons(void);                          // update neural network (one frame)
static inline void UnloadNeurons(void);                          // unload dynamic structures of neural network
static void signalHandler(int signal);                           // signal handler for graceful exit

// ----------------------------------------------------------------------------------------------
// activation functions table
float (*activationTable[])(float) = {activation_none, activation_sigmoid, activation_reLU, activation_tanh};

// ----------------------------------------------------------------------------------------------
// program entry point (main)

int main(int argc, char *argv[])
{
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
                if (i + 2 > argc)
                    break;

                flags_cmd |= CMD_FLAG_STANDALONE;
                cmd_shInputName = argv[i + 1];
                cmd_shOutputName = argv[i + 2];

                i += 2;
            } else if (xString_isEqualCString(arg, "-m") || xString_isEqualCString(arg, "--managed")) {
                if (i + 3 > argc)
                    break;

                flags_cmd |= CMD_FLAG_MANAGED;
                cmd_shInputName = argv[i + 1];
                cmd_shOutputName = argv[i + 2];
                cmd_shStateName = argv[i + 3];

                i += 3;
            } else if (xString_isEqualCString(arg, "-l") || xString_isEqualCString(arg, "--load")) {
                if (i + 1 > argc)
                    break;

                flags_cmd |= CMD_FLAG_LOADCFG;
                cmd_configFilename = argv[i + 1];

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
        printf("  -r, --random <seed>\t\t\t\tSet random seed for network initialization.\n");
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
        printf("Version:\t\tDEV P3.0\n");
        printf("Compiler version:\t%s\n", __VERSION__);
        printf("Compiled on %s at %s\n", __DATE__, __TIME__);
        return 0;
    }

    // flag arguments (shared memory names) should only be alphanumeric strings
    if (flags_cmd & (CMD_FLAG_STANDALONE | CMD_FLAG_MANAGED)) {
        bool validNames = true;
        validNames &= sm_validateSharedMemoryName(cmd_shInputName);
        validNames &= sm_validateSharedMemoryName(cmd_shOutputName);
        if (flags_cmd & CMD_FLAG_MANAGED)
            validNames &= sm_validateSharedMemoryName(cmd_shStateName);

        if (!validNames) {
            printf("ERROR: Shared memory names can only contain alphanumeric characters and underscores.\n");
            return 1;
        }
    }

    // initialize neural network, connect to shared memory and register signal handler
    InitNeurons();

    // neural network main loop
    while (!(flags_runtime & RUNTIME_EXIT)) {
        UpdateNeurons();
    }

    // free dynamic structures
    UnloadNeurons();

    return 0;
}

// ----------------------------------------------------------------------------------------------
// local function definitions

// connect to shared memory if in appropriate mode
inline void OpenSharedMemory(void)
{
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
inline void CloseSharedMemory(void)
{
    if (flags_cmd & CMD_FLAG_STANDALONE) {
        // disconnect from shared memory
        sm_disconnectSharedInput(shInput);
        sm_disconnectSharedOutput(shOutput);
    } else if (flags_cmd & CMD_FLAG_MANAGED) {
        // notify shared state that program is not running
        sm_lockSharedState(shState);
        shState->state_neuronsAlive = false;
        sm_unlockSharedState(shState);

        // disconnect from shared memory
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
inline void UpdateSharedState(void)
{
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
inline void UpdateSharedInput(void)
{
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
inline void UpdateSharedOutput(void)
{
    // check input matrix dimension
    if (input->rows != 1 || input->cols != 5) {
        printf("ERROR: Invalid input matrix dimension.\n");
        exit(1);
    }

    // update values to input matrix
    sm_lockSharedOutput(shOutput);
    xMatrix_set(input, 0, 0, shOutput->gameOutput01);
    xMatrix_set(input, 0, 1, shOutput->gameOutput02);
    xMatrix_set(input, 0, 2, shOutput->gameOutput03);
    xMatrix_set(input, 0, 3, shOutput->gameOutput04);
    xMatrix_set(input, 0, 4, shOutput->gameOutput05);
    // xMatrix_set(input, 0, 5, shOutput->gameOutput06);
    // xMatrix_set(input, 0, 6, shOutput->gameOutput07);
    // xMatrix_set(input, 0, 7, shOutput->gameOutput08);
    sm_unlockSharedOutput(shOutput);

    return;
}

// fill matrix with random values in from uniform distribution
void fillUniform(xMatrix *mat, float min, float max)
{
    srand((unsigned int)time(NULL) ^ (unsigned int)rand());
    for (uint32_t i = 0; i < mat->rows; i++) {
        for (uint32_t j = 0; j < mat->cols; j++) {
            xMatrix_set(mat, i, j, min + (max - min) * rand() / (float)RAND_MAX);
        }
    }
}

// fill matrix with random values from normal distribution
void fillNormal(xMatrix *mat, float mean, float stddev)
{
    srand((unsigned int)time(NULL) ^ (unsigned int)rand());
    for (uint32_t i = 0; i < mat->rows; i++) {
        for (uint32_t j = 0; j < mat->cols; j++) {
            xMatrix_set(mat, i, j, normalRandom(mean, stddev));
        }
    }
}

// generate normally distributed random number (using Box-Muller transform)
float normalRandom(float mean, float stddev)
{
    static float n2 = 0.0;
    static int n2_cached = 0;
    if (!n2_cached) {
        float x, y, r;
        do {
            x = 2.0f * rand() / (float)RAND_MAX - 1;
            y = 2.0f * rand() / (float)RAND_MAX - 1;

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

// neural network pass-through activation function
inline float activation_none(float x) { return x; }

// sigmoid sigmoid function
inline float activation_sigmoid(float x) { return 1.0f / (1.0f + expf(-x)); }

// reLU activation function
inline float activation_reLU(float x) { return (x > 0.0f) ? x : 0.0f; }

// tanh activation function
inline float activation_tanh(float x) { return tanhf(x); }

// initialize neural network program
inline void InitNeurons(void)
{
    // initialize structures for holding neural network matrices
    weightMatrices = xList_new();
    biasMatrices = xList_new();
    intermediateMatrices = xList_new();
    activationFunctions = xList_new();

    // load matrices from file or generate random
    if (flags_cmd & CMD_FLAG_LOADCFG && cmd_configFilename != NULL) {
        // try to load model from file
        if (fnn_loadModel(cmd_configFilename, weightMatrices, biasMatrices, activationFunctions) != 0) {
            printf("ERROR: Failed to load model from file.\n");
            exit(1);
        }

        // create intermediate matrices for later use
        xList_pushBack(intermediateMatrices, (void *)xMatrix_new(1, ((xMatrix *)weightMatrices->head->data)->rows));
        for (xListNode *node = biasMatrices->head; node != NULL; node = node->next) {
            xList_pushBack(intermediateMatrices, (void *)xMatrix_new(1, ((xMatrix *)node->data)->cols));
        }
    } else {
        // initialize basic random model for network (5-32-4 architecture)
        xMatrix *tmpMatrix = xMatrix_new(5, 32);
        fillUniform(tmpMatrix, -0.5f, 0.5f);
        xList_pushBack(weightMatrices, (void *)tmpMatrix);

        tmpMatrix = xMatrix_new(32, 4);
        fillUniform(tmpMatrix, -0.5f, 0.5f);
        xList_pushBack(weightMatrices, (void *)tmpMatrix);

        tmpMatrix = xMatrix_new(1, 32);
        fillNormal(tmpMatrix, 0.0f, 0.001f);
        xList_pushBack(biasMatrices, (void *)tmpMatrix);

        tmpMatrix = xMatrix_new(1, 4);
        fillNormal(tmpMatrix, 0.0f, 0.001f);
        xList_pushBack(biasMatrices, (void *)tmpMatrix);

        tmpMatrix = xMatrix_new(1, 5);
        xList_pushBack(intermediateMatrices, (void *)tmpMatrix);

        tmpMatrix = xMatrix_new(1, 32);
        xList_pushBack(intermediateMatrices, (void *)tmpMatrix);

        tmpMatrix = xMatrix_new(1, 4);
        xList_pushBack(intermediateMatrices, (void *)tmpMatrix);

        int *tmpInt = (int *)malloc(sizeof(int));
        *tmpInt = 2;
        xList_pushBack(activationFunctions, (void *)tmpInt);
        tmpInt = (int *)malloc(sizeof(int));
        *tmpInt = 1;
        xList_pushBack(activationFunctions, (void *)tmpInt);
    }

    // shortcut reference to input and output layers
    input = (xMatrix *)intermediateMatrices->head->data;
    output = (xMatrix *)intermediateMatrices->tail->data;

    // validate input and output layer dimension (5 inputs, 4 outputs are MANDATORY for this agent)
    if (input->cols != 5 || output->cols != 4) {
        printf("ERROR: Invalid input/output layer dimension.\n");
        printf("Input layer: %d, Output layer: %d\n", input->cols, output->cols);
        exit(1);
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
inline void UpdateNeurons(void)
{
    // TODO: correct inference to use matrices from lists

    // update state from shared memory
    UpdateSharedState();

    // update output from shared memory, game output (NN input)
    UpdateSharedOutput();

    // calculate intermediate matrices
    for (int i = 0; i < intermediateMatrices->size - 1; i++) {
        // get references to required matrices
        xMatrix *intermediateCurrent = xList_get(intermediateMatrices, i);
        xMatrix *intermediateNext = xList_get(intermediateMatrices, i + 1);
        xMatrix *weightCurrent = xList_get(weightMatrices, i);
        xMatrix *biasCurrent = xList_get(biasMatrices, i);
        float (*activationFunction)(float) = activationTable[*(int *)xList_get(activationFunctions, i)];

        // inference for current layer
        xMatrix_dot(intermediateNext, intermediateCurrent, weightCurrent);
        xMatrix_add(intermediateNext, intermediateNext, biasCurrent);
        for (uint32_t j = 0; j < ((xMatrix *)xList_get(intermediateMatrices, i + 1))->cols; j++) {
            xMatrix_set(intermediateNext, 0, j, activationFunction(xMatrix_get(intermediateNext, 0, j)));
        }
    }

    // update input to shared memory, game input (NN output)
    UpdateSharedInput();
}

// unload dynamic structures of neural network
inline void UnloadNeurons(void)
{
    // report to shared state that program is not running
    if (flags_cmd & CMD_FLAG_MANAGED) {
        sm_lockSharedState(shState);
        shState->state_neuronsAlive = false;
        sm_unlockSharedState(shState);
    }

    // disconnect from shared memory
    CloseSharedMemory();

    // free dynamic structures
    xList_forEach(weightMatrices, (void (*)(void *))xMatrix_free);
    xList_free(weightMatrices);
    xList_forEach(biasMatrices, (void (*)(void *))xMatrix_free);
    xList_free(biasMatrices);
    xList_forEach(intermediateMatrices, (void (*)(void *))xMatrix_free);
    xList_free(intermediateMatrices);
    xList_forEach(activationFunctions, free);
    xList_free(activationFunctions);

    return;
}

// signal handler for graceful exit (when SIGINT or SIGTERM is received)
void signalHandler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM) {
        flags_runtime |= RUNTIME_EXIT;

        // return back to where program was interrupted
        return;
    }
}
