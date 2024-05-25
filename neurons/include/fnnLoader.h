/**
 * @file fnnLoader.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Bridge module for loading FNN models from files directly into xList objects containing weight, bias matrices, and
 * activation functions.
 * @version 0.1
 * @date 08.05.2024.
 *
 * @copyright All rights reserved (c) 2024
 *
 */

#ifndef FNN_LOADER_H
#define FNN_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "xList.h"

/**
 * @brief Load FNN model from file into list of xMatrix objects
 *
 * @param filename Path to the file containing data of the FNN model
 * @param weightMatrices Pointer to the list for storing weight matrices
 * @param biasMatrices Pointer to the list for storing bias matrices
 * @param activationFunctions Pointer to the list for storing activation
 * functions
 * @return `int32_t`: 0 if successful, -1 if error occurred
 *
 * @warning Caller is responsible for freeing the memory of the lists and their
 * elements after use.
 *
 * @note It is assumed that the lists are empty and allocated. If not, the
 * function will fail.
 *
 * @note If biasMatrices is NULL, the bias matrices will not be loaded.
 *
 * @note The order of elements in the lists follow the order of inference in the
 * FNN model.
 *
 */
int32_t fnn_loadModel(const char *filename, xList *weightMatrices, xList *biasMatrices, xList *activationFunctions);

#ifdef __cplusplus
}
#endif

#endif  // FNN_LOADER_H
