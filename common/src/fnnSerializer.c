#include "fnnSerializer.h"
#include <stdint.h>  // standard integer types for fixed integer width in file format (uint32_t, ...)
#include <stdio.h>   // standard I/O (fprintf, ...)
#include <stdlib.h>  // standard library (for malloc, free, ...)

FnnModel *fnn_new(void)
{
    // memory allocation
    FnnModel *model = (FnnModel *)malloc(sizeof(FnnModel));
    if (model == NULL) {
        fprintf(stderr, "FNN Serializer: Failed to allocate descriptor\n");
        return NULL;
    }

    // initialization
    model->magic = FNN_SERIALIZER_MAGIC;
    model->version = FNN_SERIALIZER_VERSION;
    model->totalWeights = 0;
    model->totalBiases = 0;
    model->layerCount = 0;
    model->neuronCounts = NULL;
    model->activationFunctions = NULL;
    model->weightValues = NULL;
    model->biasValues = NULL;

    return model;
}

void fnn_free(FnnModel *model)
{
    if (model != NULL) {
        free(model->neuronCounts);
        free(model->activationFunctions);
        free(model->weightValues);
        free(model->biasValues);
        free(model);
    }

    return;
}

int32_t fnn_addLayer(FnnModel *model, uint32_t neuronCount, float *weightVals, float *biasVals, FnnActivation_e activationFunction)
{
    // parameter checking
    if (model == NULL || neuronCount == 0 || (weightVals == NULL && model->layerCount > 0) ||
        (biasVals == NULL && model->layerCount > 0)) {
        fprintf(stderr, "FNN Serializer: fnn_addLayer bad arguments");
        return -1;
    }

    // reallocation of neuron counts array
    uint32_t *neuronCounts = (uint32_t *)realloc(model->neuronCounts, (model->layerCount + 1) * sizeof(uint32_t));
    if (neuronCounts == NULL) {
        fprintf(stderr, "FNN Serializer: Failed to expand neuron counters");
        return -1;
    }
    model->neuronCounts = neuronCounts;

    // activation functions, weights and biases are not applicable to first (input) layer
    if (model->layerCount > 0) {
        // reallocation of activation functions array (input layer has no activation function)
        FnnActivation_e *activationFunctions =
            (FnnActivation_e *)realloc(model->activationFunctions, model->layerCount * sizeof(FnnActivation_e));
        if (activationFunctions == NULL) {
            fprintf(stderr, "FNN Searializer: Failed to expand activation functions descriptor");
            return -1;
        }
        model->activationFunctions = activationFunctions;

        // reallocation of weight values array
        float *weights = (float *)realloc(
            model->weightValues, (model->totalWeights + neuronCount * model->neuronCounts[model->layerCount - 1]) * sizeof(float));
        if (weights == NULL) {
            fprintf(stderr, "FNN Serializer: Failed to expand weights descriptor");
            return -1;
        }
        model->weightValues = weights;

        // reallocation of bias values array
        float *biases = (float *)realloc(model->biasValues, (model->totalBiases + neuronCount) * sizeof(float));
        if (biases == NULL) {
            fprintf(stderr, "FNN Serializer: Failed to expand bias descriptors");
            return -1;
        }
        model->biasValues = biases;
    }

    // value copy
    model->neuronCounts[model->layerCount] = neuronCount;
    if (model->layerCount > 0) {
        model->activationFunctions[model->layerCount - 1] = activationFunction;
        for (uint32_t i = 0; i < neuronCount * model->neuronCounts[model->layerCount - 1]; i++) {
            model->weightValues[model->totalWeights + i] = weightVals[i];
        }
        for (uint32_t i = 0; i < neuronCount; i++) {
            model->biasValues[model->totalBiases + i] = biasVals[i];
        }

        // update total weights and biases
        model->totalWeights += neuronCount * model->neuronCounts[model->layerCount - 1];
        model->totalBiases += neuronCount;
    }

    // update layer count information
    model->layerCount++;

    return 0;
}

int32_t fnn_serialize(const char *filename, FnnModel *model)
{
    // file opening
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        fprintf(stderr, "FNN Serializer: Failed to open file for writing\n");
        return -1;
    }

    // write size counter
    uint64_t wrSize = 0;

    // calculate total expected size
    uint64_t expectedSize = sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint32_t) +
                            model->layerCount * sizeof(uint32_t) + (model->layerCount - 1) * sizeof(FnnActivation_e) +
                            model->totalWeights * sizeof(float) + model->totalBiases * sizeof(float);

    // write model header
    wrSize += fwrite(&model->magic, sizeof(uint32_t), 1, file) * sizeof(uint32_t);
    wrSize += fwrite(&model->version, sizeof(uint16_t), 1, file) * sizeof(uint16_t);
    wrSize += fwrite(&model->totalWeights, sizeof(uint64_t), 1, file) * sizeof(uint64_t);
    wrSize += fwrite(&model->totalBiases, sizeof(uint64_t), 1, file) * sizeof(uint64_t);
    wrSize += fwrite(&model->layerCount, sizeof(uint32_t), 1, file) * sizeof(uint32_t);

    // write layer descriptors
    wrSize += fwrite(model->neuronCounts, sizeof(uint32_t), model->layerCount, file) * sizeof(uint32_t);
    wrSize += fwrite(model->activationFunctions, sizeof(FnnActivation_e), (model->layerCount - 1), file) * sizeof(FnnActivation_e);

    // write weight and bias values
    wrSize += fwrite(model->weightValues, sizeof(float), model->totalWeights, file) * sizeof(float);
    wrSize += fwrite(model->biasValues, sizeof(float), model->totalBiases, file) * sizeof(float);

    // check if all data was written
    if (wrSize != expectedSize) {
        fprintf(stderr, "FNN Serializer: Failed to write all data to file\n");
        fclose(file);
        return -1;
    }

    // close file
    fclose(file);

    return 0;
}

FnnModel *fnn_deserialize(const char *filename)
{
    // file opening
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "FNN Serializer: Failed to open file for reading\n");
        return NULL;
    }

    // allocate model descriptor
    FnnModel *model = (FnnModel *)malloc(sizeof(FnnModel));
    if (model == NULL) {
        fprintf(stderr, "FNN Serializer: Failed to allocate descriptor\n");
        fclose(file);
        return NULL;
    }

    // read model header
    if (fread(&model->magic, sizeof(uint32_t), 1, file) != 1 || fread(&model->version, sizeof(uint16_t), 1, file) != 1 ||
        fread(&model->totalWeights, sizeof(uint64_t), 1, file) != 1 || fread(&model->totalBiases, sizeof(uint64_t), 1, file) != 1 ||
        fread(&model->layerCount, sizeof(uint32_t), 1, file) != 1) {
        fprintf(stderr, "FNN Serializer: Failed to read model header\n");
        fclose(file);
        free(model);
        return NULL;
    }

    // validate model header
    if (model->magic != FNN_SERIALIZER_MAGIC || model->version != FNN_SERIALIZER_VERSION) {
        fprintf(stderr, "FNN Serializer: Invalid model header\n");
        fclose(file);
        free(model);
        return NULL;
    }
    if (model->layerCount <= 1) {
        fprintf(stderr, "FNN Serializer: Invalid layer count\n");
        fclose(file);
        free(model);
        return NULL;
    }

    // allocate arrays
    model->neuronCounts = (uint32_t *)malloc(model->layerCount * sizeof(uint32_t));
    if (model->neuronCounts == NULL) {
        fprintf(stderr, "FNN Serializer: Failed to allocate neuron counts\n");
        fclose(file);
        free(model);
        return NULL;
    }

    model->activationFunctions = (FnnActivation_e *)malloc((model->layerCount - 1) * sizeof(FnnActivation_e));
    if (model->activationFunctions == NULL) {
        fprintf(stderr, "FNN Serializer: Failed to allocate activation functions\n");
        fclose(file);
        free(model->neuronCounts);
        free(model);
        return NULL;
    }

    model->weightValues = (float *)malloc(model->totalWeights * sizeof(float));
    if (model->weightValues == NULL) {
        fprintf(stderr, "FNN Serializer: Failed to allocate weight values\n");
        fclose(file);
        free(model->activationFunctions);
        free(model->neuronCounts);
        free(model);
        return NULL;
    }

    model->biasValues = (float *)malloc(model->totalBiases * sizeof(float));
    if (model->biasValues == NULL) {
        fprintf(stderr, "FNN Serializer: Failed to allocate bias values\n");
        fclose(file);
        free(model->weightValues);
        free(model->activationFunctions);
        free(model->neuronCounts);
        free(model);
        return NULL;
    }

    // read layer descriptors
    if (fread(model->neuronCounts, sizeof(uint32_t), model->layerCount, file) != model->layerCount ||
        fread(model->activationFunctions, sizeof(FnnActivation_e), model->layerCount - 1, file) != model->layerCount - 1) {
        fprintf(stderr, "FNN Serializer: Failed to read layer descriptors\n");
        fclose(file);
        free(model->biasValues);
        free(model->weightValues);
        free(model->activationFunctions);
        free(model->neuronCounts);
        free(model);
        return NULL;
    }

    // read weight and bias values
    uint64_t readWeights = fread(model->weightValues, sizeof(float), model->totalWeights, file);
    uint64_t readBiases = fread(model->biasValues, sizeof(float), model->totalBiases, file);
    if (readWeights != model->totalWeights || readBiases != model->totalBiases) {
        fprintf(stderr, "FNN Serializer: Failed to read weight and bias values\n");
        fclose(file);
        free(model->biasValues);
        free(model->weightValues);
        free(model->activationFunctions);
        free(model->neuronCounts);
        free(model);
        return NULL;
    }

    // close file
    fclose(file);

    return model;
}
