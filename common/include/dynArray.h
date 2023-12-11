/**
 * @file dynArray.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Dynamic array and functions.
 * @version 0.1
 * @date 11.12.2023.
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef ASTEROIDS_STRUCTURES_H
#define ASTEROIDS_STRUCTURES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * @brief A simple dynamic array implementation.
 *
 */
typedef struct dyn_array_s {
    void **data;
    size_t size;
    size_t capacity;
    size_t elementSize;
} DynArray;

/**
 * @brief Create a new dynamic array.
 *
 * @param capacity Initial capacity.
 * @return DynArray* Pointer to the new dynamic array.
 */
DynArray *newDynArray(size_t capacity, size_t elementSize);

/**
 * @brief Destroy a dynamic array.
 *
 * @param dynArray Dynamic array to destroy.
 */
void destroyDynArray(DynArray *dynArray);

/**
 * @brief Add an element to a dynamic array.
 *
 * @param dynArray Dynamic array to add to.
 * @param element Element to add.
 * @return size_t Index of the added element.
 */
size_t dynArrayAdd(DynArray *dynArray, void *element);

/**
 * @brief Remove an element from a dynamic array.
 *
 * @param dynArray Dynamic array to remove from.
 * @param index Index of the element to remove.
 */
void dynArrayRemove(DynArray *dynArray, size_t index);

/**
 * @brief Get an element from a dynamic array.
 *
 * @param dynArray Dynamic array to get from.
 * @param index Index of the element to get.
 * @return void* Pointer to the element.
 */
void *dynArrayGet(DynArray *dynArray, size_t index);

/**
 * @brief Get the size of a dynamic array.
 *
 * @param dynArray Dynamic array to get the size of.
 * @return size_t Size of the dynamic array.
 */
size_t dynArraySize(DynArray *dynArray);

/**
 * @brief Get the capacity of a dynamic array.
 *
 * @param dynArray Dynamic array to get the capacity of.
 * @return size_t Capacity of the dynamic array.
 */
size_t dynArrayCapacity(DynArray *dynArray);

/**
 * @brief Get the last element of a dynamic array.
 *
 * @param dynArray Dynamic array to get the last element of.
 * @return void* Pointer to the last element.
 */
void *dynArrayLast(DynArray *dynArray);

/**
 * @brief Get the first element of a dynamic array.
 *
 * @param dynArray Dynamic array to get the first element of.
 * @return void* Pointer to the first element.
 */
void *dynArrayFirst(DynArray *dynArray);

/**
 * @brief Clear a dynamic array.
 *
 * @param dynArray Pointer to the dynamic array to clear.
 */
void dynArrayClear(DynArray *dynArray);

/**
 * @brief Apply a function to each element of a dynamic array.
 * 
 * @param dynArray Dynamic array to apply the function to.
 * @param foreach Function to apply.
 */
void dynArrayForeach(DynArray *dynArray, void (*foreach)(void *element));

/**
 * @brief Filter a dynamic array.
 *
 * @param dynArray Dynamic array to filter.
 * @param filter Filter function.
 * @return DynArray* Filtered dynamic array.
 */
DynArray *dynArrayFilter(DynArray *dynArray, int (*filter)(void *element));

/**
 * @brief Map a dynamic array.
 *
 * @param dynArray Dynamic array to map.
 * @param map Map function.
 * @return DynArray* Mapped dynamic array.
 */
DynArray *dynArrayMap(DynArray *dynArray, void *(*map)(void *element));

/**
 * @brief Reduce a dynamic array.
 *
 * @param dynArray Dynamic array to reduce.
 * @param reduce Reduce function.
 * @param initial Initial value.
 * @return void* Reduced value.
 */
void *dynArrayReduce(DynArray *dynArray, void *(*reduce)(void *accumulator, void *element), void *initial);

/**
 * @brief Find an element in a dynamic array.
 *
 * @param dynArray Dynamic array to find in.
 * @param find Find function.
 * @return void* Pointer to the found element.
 */
void *dynArrayFind(DynArray *dynArray, int (*find)(void *element));

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // ASTEROIDS_STRUCTURES_H
