#include "fnnLoader.h"
#include <stdint.h>         // universal integer types
#include <stdio.h>          // fprintf (for error messages)
#include <stdlib.h>         // malloc (for memory allocation)
#include "fnnSerializer.h"  // FNN model descriptor
#include "xLinear.h"        // xMatrix objects for layer information
#include "xList.h"          // xList object for storing xMatrix objects in one package for return

int32_t fnn_loadModel(const char *filename, xList *weightMatrices, xList *biasMatrices, xList *activationFunctions)
{
    // checking validity of arguments (only bias matrices can be ignored if
    // unused)
    if (filename == NULL || weightMatrices == NULL || activationFunctions == NULL) {
        fprintf(stderr, "FNN Loader: Invalid arguments\n");
        return -1;
    }

    // load model descriptor from file
    FnnModel *model = fnn_deserialize(filename);
    if (model == NULL) {
        fprintf(stderr, "FNN Loader: Failed to load model from file\n");
        return -1;
    }

    // load weight and bias matrices from descriptor
    float *weightValues = model->weightValues;
    float *biasValues = model->biasValues;

    for (uint32_t i = 0; i < model->layerCount - 1; i++) {
        // create weight matrix
        xMatrix *weightMatrix = xMatrix_new(model->neuronCounts[i], model->neuronCounts[i + 1]);
        if (weightMatrix == NULL) {
            fprintf(stderr, "FNN Loader: Failed to allocate memory for weight matrix\n");
            return -1;
        }
        for (uint32_t j = 0; j < weightMatrix->rows * weightMatrix->cols; j++) {
            weightMatrix->data[j] = *(weightValues++);
        }
        xList_pushBack(weightMatrices, weightMatrix);

        // create bias matrix
        if (biasMatrices != NULL) {
            xMatrix *biasMatrix = xMatrix_new(1, model->neuronCounts[i + 1]);
            if (biasMatrix == NULL) {
                fprintf(stderr, "FNN Loader: Failed to allocate memory for bias matrix\n");
                return -1;
            }
            for (uint32_t j = 0; j < model->neuronCounts[i + 1]; j++) {
                biasMatrix->data[j] = *(biasValues++);
            }
            xList_pushBack(biasMatrices, biasMatrix);
        }

        // allocate and append activation function identifiers
        FnnActivation_e *activationFunction = malloc(sizeof(FnnActivation_e));
        if (activationFunction == NULL) {
            fprintf(stderr, "FNN Loader: Failed to allocate memory for "
                            "activation function\n");
            return -1;
        }
        *activationFunction = model->activationFunctions[i];
        xList_pushBack(activationFunctions, activationFunction);
    }

    // free model descriptor
    fnn_free(model);

    return 0;
}
