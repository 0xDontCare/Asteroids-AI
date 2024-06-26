/**
 * @file fnnSerializer.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Feedforward Neural Network Serializer module
 * @version 0.2
 * @date 08.05.2024.
 *
 * @copyright All rights reserved (c) 2024
 *
 * Module declares structure and set of functions used for serializing and deserializing feedforward neural network model to custom
 * binary format. All functions have prefix `fnn_`.
 * Binary format is defined as follows:
 * - Magic number (4 bytes): 0x4D4E4E46 (FNNM)
 * - Format version number (2 bytes): 0x0002 (0.02)
 * - Total weights (8 bytes): Total number of weights in the model
 * - Total biases (8 bytes): Total number of biases in the model
 * - Layer count (4 bytes): Number of layers in the model
 * - Neuron counts (4 bytes * layer count): Number of neurons in each layer
 * - Activation functions (4 bytes * layer count): Activation function for each layer except the input layer
 * - Weight values (4 bytes * total weights): Weights for each neuron in each layer
 * - Bias values (4 bytes * total biases): Biases for each neuron in each layer
 */

#ifndef FNN_SERIALIZER_H
#define FNN_SERIALIZER_H

#include <stdint.h>

#define FNN_SERIALIZER_MAGIC 0x4D4E4E46  // "FNNM"
#define FNN_SERIALIZER_VERSION 0x0002    // 0.02

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Activation functions for the FNN layer
 *
 */
typedef enum {
    FNN_ACTIVATION_NONE = 0,  // f(x) = x (pass through)
    FNN_ACTIVATION_SIGMOID = 1,
    FNN_ACTIVATION_RELU = 2,
    FNN_ACTIVATION_TANH = 3
} FnnActivation_e;

/**
 * @brief Feedforward neural network model descriptor
 *
 */
typedef struct {
    uint32_t magic;
    uint16_t version;
    uint64_t totalWeights;
    uint64_t totalBiases;
    uint32_t layerCount;
    uint32_t *neuronCounts;
    FnnActivation_e *activationFunctions;
    float *weightValues;
    float *biasValues;
} FnnModel;

/**
 * @brief Create empty FNN model object
 * @return `FnnModel*`: Pointer to the FNN model if successful, NULL on failure
 *
 */
FnnModel *fnn_new(void);

/**
 * @brief Free FNN model from memory
 *
 * @param model FNN model to free
 */
void fnn_free(FnnModel *model);

/**
 * @brief Add layer to FNN model
 *
 * @param model FNN model to add layer to
 * @param neuronCount Number of neurons in the layer
 * @param weightCount Number of weights in the layer
 * @param weightVals Weights for the layer
 * @param activationFunction Activation function used for the layer
 * @return `int32_t`: 0 on success, -1 on failure
 */
int32_t fnn_addLayer(FnnModel *model, uint32_t neuronCount, float *weightVals, float *biasVals, FnnActivation_e activationFunction);

/**
 * @brief Serialize FNN model to buffer
 *
 * @param filename Filename to store the serialized model to
 * @param model FNN model to serialize
 * @return `int32_t`: 0 on success, -1 on failure
 */
int32_t fnn_serialize(const char *filename, FnnModel *model);

/**
 * @brief Deserialize FNN model from buffer
 *
 * @param filename File containing the serialized model
 * @return `FnnModel*` Pointer to the deserialized FNN model, NULL on failure
 */
FnnModel *fnn_deserialize(const char *filename);

#ifdef __cplusplus
}
#endif

#endif  // FNN_SERIALIZER_H
