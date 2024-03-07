/**
 * @file fnnSerializer.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Feedforward Neural Network Serializer module
 * @version 0.02
 * @date 06.03.2024.
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef FNN_SERIALIZER_H
#define FNN_SERIALIZER_H

#include <stdint.h>

#define FNN_SERIALIZER_MAGIC 0x4D4E4E46  // "FNNM"
#define FNN_SERIALIZER_VERSION 0x0002    // 0.02

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Activation functions for the FNN layer
   *
   */
  typedef enum
  {
    FNN_ACTIVATION_NONE = 0,
    FNN_ACTIVATION_SIGMOID = 1,
    FNN_ACTIVATION_RELU = 2,
    FNN_ACTIVATION_TANH = 3,
    FNN_ACTIVATION_SOFTMAX = 4
  } FnnActivation_e;

  /**
   * @brief Feedforward neural network model descriptor
   *
   */
  typedef struct
  {
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

  /*
   * Serialization process:
   * 1. Create empty FNN model object
   * 2. Set values for `magic`, `version` and `layerCount` fields
   * 3. Based on the `layerCount`, allocate arrays for `neuronCounts` and `activationFunctions`
   * fields
   * 4. Set values for `neuronCounts` and `activationFunctions` fields
   * 5. Based on `neuronCounts`, allocate arrays for `weightValues` and `biasValues` fields
   * 6. Set values for `weightValues` and `biasValues` fields
   * 7. Calculate and set value for `totalParameters` field
   * 8. Write FNN model to file as binary data
   */

  /**
   * @brief Create empty FNN model object
   *
   */
  FnnModel *fnnNew();

  /**
   * @brief Free FNN model from memory
   *
   * @param model FNN model to free
   */
  void fnnFree(FnnModel *model);

  /**
   * @brief Add layer to FNN model
   *
   * @param model FNN model to add layer to
   * @param neuronCount Number of neurons in the layer
   * @param weightCount Number of weights in the layer
   * @param weightVals Weights for the layer
   * @param activationFunction Activation function used for the layer
   * @return 0 on success, -1 on failure
   */
  int32_t fnnAddLayer(FnnModel *model, uint32_t neuronCount, float *weightVals, float *biasVals,
                      FnnActivation_e activationFunction);

  /**
   * @brief Serialize FNN model to buffer
   *
   * @param filename Filename to store the serialized model to
   * @param model FNN model to serialize
   * @return 0 on success, -1 on failure
   */
  int32_t fnnSerialize(const char *filename, FnnModel *model);

  /**
   * @brief Deserialize FNN model from buffer
   *
   * @param filename File containing the serialized model
   * @return Pointer to the deserialized FNN model, NULL on failure
   */
  FnnModel *fnnDeserialize(const char *filename);

#ifdef __cplusplus
}
#endif

#endif  // FNN_SERIALIZER_H