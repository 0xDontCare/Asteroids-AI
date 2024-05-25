/**
 * @file fnnGenAlgorithm.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Genetic algorithm functions for feedforward neural network (FNN) weights and biases
 * @version 0.1
 * @date 08.05.2024.
 *
 * @copyright All rights reserved (c) 2024
 *
 */

#ifndef FNN_GENALGORITHM_H
#define FNN_GENALGORITHM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>         // standard integer types (uint32_t, ...)
#include "fnnSerializer.h"  // FNN model descriptor and serialization functions

/**
 * @brief Generate randomized feedforward neural network model based on given parameters
 *
 * @param neuronCounts Number of neurons in each layer
 * @param activationFunctions Activation functions for each layer (except the input layer)
 * @param layerCount Total number of layers in the FNN
 * @param weightMin Minimum value weight can have
 * @param weightMax Maximum value weight can have
 * @param biasMean Mean value for the biases
 * @param biasStddev Standard deviation for the biases
 * @return Pointer to generated FNN model descriptor or NULL if the generation failed
 *
 * @note The caller is responsible for freeing the memory.
 *
 * @note Weights are generated as uniformly distributed random numbers in the given range.
 *
 * @note Biases are generated as normally distributed random numbers with set mean and standard deviation.
 */
FnnModel *fnn_generateModel(uint32_t *neuronCounts, FnnActivation_e *activationFunctions, uint32_t layerCount, float weightMin,
                            float weightMax, float biasMean, float biasStddev);

/**
 * @brief Breed two feedforward neural network models to create a new one
 *
 * @param parent1 Pointer to the first parent FNN model
 * @param parent2 Pointer to the second parent FNN model
 * @param sbxCrossDistrIndex Distribution index for the crossover (probability value between 0 and positive infinity)
 * @param mutationRate Rate of mutation (probability value between 0 and 1) for each weight and bias
 * @param mutationStddev Standard deviation for the mutation
 * @return Pointer to descriptor of the new FNN model or NULL if process failed
 *
 * @note The caller is responsible for freeing the memory.
 *
 * @warning Both parent FNN models must have same total number of weights and biases.
 *
 * @note The crossover is done using simulated binary crossover (SBX) algorithm.
 *
 * @note The mutation is done using normal distribution around the current value.
 */
FnnModel *fnn_modelBreed(FnnModel *parent1, FnnModel *parent2, float sbxCrossDistrIndex, float mutationRate, float mutationStddev);

/**
 * @brief Generate random weights for the FNN based on given layer neurons and range
 * @details The weights are generated as uniformly distributed random numbers in the given range.
 *
 * @param layerNeurons Number of neurons in each layer
 * @param layerCount Number of layers in the FNN
 * @param rangeMin Minimum value for the weights
 * @param rangeMax Maximum value for the weights
 * @return `float*`: Pointer to the generated weights or NULL if the generation failed
 *
 * @note The caller is responsible for freeing the memory.
 *
 * @note The rangeMin and rangeMax are inclusive.
 *
 * @warning There needs to be at least 2 layers for the FNN to be valid.
 */
float *fnn_generateWeights(const uint32_t *layerNeurons, uint32_t layerCount, float rangeMin, float rangeMax);

/**
 * @brief Generate random biases for the FNN based on given layer neurons
 * @details The biases are generated as normally distributed random numbers with
 * set mean and standard deviation.
 *
 * @param layerNeurons Number of neurons in each layer
 * @param layerCount Number of layers in the FNN
 * @param mean Mean value for the biases
 * @param stddev Standard deviation for the biases
 * @return `float*`: Pointer to the generated biases or NULL if the generation failed
 *
 * @note The caller is responsible for freeing the memory.
 *
 * @warning There needs to be at least 2 layers for the FNN to be valid.
 */
float *fnn_generateBiases(const uint32_t *layerNeurons, uint32_t layerCount, float mean, float stddev);

/**
 * @brief Crossover operator for two parent arrays
 *
 * @param parent1 Pointer to the first parent array
 * @param parent2 Pointer to the second parent array
 * @param numElements Number of elements in the parent arrays
 * @param distributionIndex Distribution index for the crossover (probability value between 0 and positive infinity)
 * @return `float*`: Pointer to the new array with the crossover result or NULL if the crossover
 *
 * @note The caller is responsible for freeing the memory if parent arrays are not needed anymore.
 *
 * @warning `layerNeurons` and `layerCount` must be the same for both parent FNNs.
 *
 * @note The crossover is done using simulated binary crossover (SBX) algorithm.
 */
float *fnn_crossover(const float *parent1, const float *parent2, uint64_t numElements, float distributionIndex);

/**
 * @brief Mutate the weights of the FNN
 *
 * @param values Pointer to the array of values to mutate
 * @param numElements Number of elements in the array
 * @param mutationRate Rate of mutation (probability value between 0 and 1)
 * @param stddev Standard deviation for the mutation
 *
 * @note The mutation is done using normal distribution around the current value.
 */
void fnn_mutate(float *values, uint64_t numElements, float mutationRate, float stddev);

#ifdef __cplusplus
}
#endif

#endif  // FNN_GENALGORITHM_H
