#include "fnnGenAlgorithm.h"
#include <math.h>           // mathematical functions (for pow, log, sqrt)
#include <stdint.h>         // standard integer types (for uint8_t, uint32_t, ...)
#include <stdlib.h>         // standard library (for malloc, free, rand)
#include <time.h>           // time functions (for getting time)
#include "fnnSerializer.h"  // FNN model implementation and serialization functions

/**
 * @brief Generate random number from normal distribution
 *
 * @param mean Mean value
 * @param stddev Standard deviation
 * @return `float`: Random number from normal distribution
 *
 * @note The function uses Box-Muller transform to generate the random number.
 */
static float normalRandom(float mean, float stddev)
{
    static float n2 = 0.0f;
    static uint8_t n2_cached = 0;
    if (!n2_cached) {
        float x, y, r;
        do {
            x = 2.0f * rand() / (float)RAND_MAX - 1;
            y = 2.0f * rand() / (float)RAND_MAX - 1;

            r = x * x + y * y;
        } while (r == 0.0f || r > 1.0f);

        float d = sqrtf(-2.0f * logf(r) / r);
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

/**
 * @brief Calculate the distance factor (beta) for SBX crossover
 *
 * @param distrIndex Distribution index (eta) for the SBX algorithm
 * @return `float`: Distance factor
 */
static float distanceFactor(float distrIndex)
{
    // check for valid distribution index
    if (distrIndex <= 0) {
        return 1.0f;
    }

    // generate positive random number smaller than 1
    float random = (float)rand() / (float)RAND_MAX;

    // calculate the distance factor
    return (random > 0.5f) ? powf(2.0f * random, 1.0f / (distrIndex + 1.0f))
                           : 1.0f / powf(2.0f * (1.0f - random), 1.0f / (distrIndex + 1.0f));
}

/**
 * @brief Fill array with uniformly distributed random numbers
 *
 * @param array Pointer to the array to fill
 * @param arrLen Length of the array
 * @param min Minimum value for the random numbers
 * @param max Maximum value for the random numbers
 */
static void fnn_fillUniform(float *array, uint64_t arrLen, float min, float max)
{
    // parameter checking
    if (array == NULL || arrLen == 0 || min > max) {
        return;
    }

    srand((unsigned int)time(NULL) ^ (unsigned int)rand());
    for (uint64_t i = 0; i < arrLen; i++) {
        array[i] = ((float)rand() / (float)RAND_MAX) * (max - min) + min;
    }
}

/**
 * @brief Fill array with normally distributed random numbers
 *
 * @param array Pointer to the array to fill
 * @param arrLen Length of the array
 * @param mean Mean value for the normal distribution
 * @param stdDev Standard deviation for the normal distribution
 */
static void fnn_fillNormal(float *array, uint64_t arrLen, float mean, float stdDev)
{
    // parameter checking
    if (array == NULL || arrLen == 0 || stdDev < 0) {
        return;
    }

    // filling array
    srand((unsigned int)time(NULL) ^ (unsigned int)rand());
    for (uint64_t i = 0; i < arrLen; i++) {
        array[i] = normalRandom(mean, stdDev);
    }
}

FnnModel *fnn_generateModel(uint32_t *neuronCounts, FnnActivation_e *activationFunctions, uint32_t layerCount, float weightMin,
                            float weightMax, float biasMean, float biasStddev)
{
    // parameter checking
    if (neuronCounts == NULL || activationFunctions == NULL || layerCount < 2 || weightMin > weightMax || biasStddev < 0) {
        return NULL;
    }

    // memory allocation
    FnnModel *model = fnn_new();

    // generate weights and biases
    float *weights = fnn_generateWeights(neuronCounts, layerCount, weightMin, weightMax);
    float *biases = fnn_generateBiases(neuronCounts, layerCount, biasMean, biasStddev);

    // count up total weights and biases
    for (uint32_t i = 0; i < layerCount - 1; i++) {
        model->totalWeights += neuronCounts[i] * neuronCounts[i + 1];
        model->totalBiases += neuronCounts[i + 1];
    }

    // assign values to the model
    model->layerCount = layerCount;
    model->neuronCounts = neuronCounts;
    model->activationFunctions = activationFunctions;
    model->weightValues = weights;
    model->biasValues = biases;

    return model;
}

FnnModel *fnn_modelBreed(FnnModel *parent1, FnnModel *parent2, float sbxCrossDistrIndex, float mutationRate, float mutationStddev)
{
    // parameter checking
    if (parent1 == NULL || parent2 == NULL || sbxCrossDistrIndex < 0 || mutationRate < 0 || mutationRate > 1 ||
        mutationStddev < 0 || parent1->totalWeights != parent2->totalWeights || parent1->totalBiases != parent2->totalBiases) {
        return NULL;
    }

    // memory allocation
    FnnModel *child = fnn_new();
    if (child == NULL) {
        return NULL;
    }

    // crossover
    if (parent1->weightValues != NULL && parent2->weightValues != NULL) {
        float *childWeights =
            fnn_crossover(parent1->weightValues, parent2->weightValues, parent1->totalWeights, sbxCrossDistrIndex);
        if (childWeights == NULL) {
            fnn_free(child);
            return NULL;
        }
        child->weightValues = childWeights;
    }

    if (parent1->biasValues != NULL && parent2->biasValues != NULL) {
        float *childBiases = fnn_crossover(parent1->biasValues, parent2->biasValues, parent1->totalBiases, sbxCrossDistrIndex);
        if (childBiases == NULL) {
            fnn_free(child);
            return NULL;
        }
        child->biasValues = childBiases;
    }

    // mutation
    if (child->weightValues != NULL) {
        fnn_mutate(child->weightValues, parent1->totalWeights, mutationRate, mutationStddev);
    }

    if (child->biasValues != NULL) {
        fnn_mutate(child->biasValues, parent1->totalBiases, mutationRate, mutationStddev);
    }

    // assign values to the model
    child->layerCount = parent1->layerCount;
    child->totalWeights = parent1->totalWeights;
    child->totalBiases = parent1->totalBiases;

    child->neuronCounts = (uint32_t *)malloc(sizeof(uint32_t) * parent1->layerCount);
    if (child->neuronCounts == NULL) {
        fnn_free(child);
        return NULL;
    }
    for (uint32_t i = 0; i < parent1->layerCount; i++) {
        child->neuronCounts[i] = parent1->neuronCounts[i];
    }
    child->activationFunctions = (FnnActivation_e *)malloc(sizeof(FnnActivation_e) * (parent1->layerCount - 1));
    if (child->activationFunctions == NULL) {
        fnn_free(child);
        return NULL;
    }
    for (uint32_t i = 0; i < parent1->layerCount - 1; i++) {
        child->activationFunctions[i] = parent1->activationFunctions[i];
    }

    return child;
}

float *fnn_generateWeights(const uint32_t *layerNeurons, uint32_t layerCount, float rangeMin, float rangeMax)
{
    // parameter checking
    if (layerNeurons == NULL || layerCount < 2 || rangeMin > rangeMax) {
        return NULL;
    }

    // memory allocation
    uint64_t totalWeights = 0;
    for (uint32_t i = 0; i < layerCount - 1; i++) {
        totalWeights += layerNeurons[i] * layerNeurons[i + 1];
    }
    float *weights = (float *)malloc(totalWeights * sizeof(float));
    if (weights == NULL) {
        return NULL;
    }

    // fill weights with random values
    fnn_fillUniform(weights, totalWeights, rangeMin, rangeMax);

    return weights;
}

float *fnn_generateBiases(const uint32_t *layerNeurons, uint32_t layerCount, float mean, float stddev)
{
    // parameter checking
    if (layerNeurons == NULL || layerCount < 2 || stddev < 0) {
        return NULL;
    }

    // memory allocation
    uint64_t totalBiases = 0;
    for (uint32_t i = 1; i < layerCount; i++) {
        totalBiases += layerNeurons[i];
    }
    float *biases = (float *)malloc(totalBiases * sizeof(float));
    if (biases == NULL) {
        return NULL;
    }

    // fill biases with random values
    fnn_fillNormal(biases, totalBiases, mean, stddev);

    return biases;
}

float *fnn_crossover(const float *parent1, const float *parent2, uint64_t numElements, float distributionIndex)
{
    // parameter checking
    if (parent1 == NULL || parent2 == NULL || numElements == 0 || distributionIndex < 0) {
        return NULL;
    }

    // memory allocation
    float *childWeights = (float *)malloc(numElements * sizeof(float));
    if (childWeights == NULL) {
        return NULL;
    }

    // simulated binary crossover (SBX)
    for (uint64_t i = 0; i < numElements; i++) {
        float x1 = parent1[i];
        float x2 = parent2[i];
        float beta = distanceFactor(distributionIndex);

        float child1 = 0.5f * ((1.0f + beta) * x1 + (1.0f - beta) * x2);
        float child2 = 0.5f * ((1.0f - beta) * x1 + (1.0f + beta) * x2);

        childWeights[i] = (rand() % 2 == 0) ? child1 : child2;
    }

    return childWeights;
}

void fnn_mutate(float *values, uint64_t numElements, float mutationRate, float stddev)
{
    // parameter checking
    if (values == NULL || numElements == 0 || mutationRate < 0 || stddev < 0) {
        return;
    }

    // mutation
    srand((unsigned int)time(NULL) ^ (unsigned int)rand());
    for (uint64_t i = 0; i < numElements; i++) {
        if ((float)rand() / (float)RAND_MAX < mutationRate) {
            values[i] = normalRandom(values[i], stddev);
        }
    }

    return;
}
