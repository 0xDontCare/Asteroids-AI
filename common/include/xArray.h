/**
 * @file xArray.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Dynamic array implementation in xStructures module.
 * @version 0.01a
 * @date 12.05.2024.
 *
 * @copyright All rights reserved (c) 2024
 *
 * Module declares dynamic array structure and functions for managing it. All functions have prefix `xArray_`.
 */

#ifndef XSTRUCTURES_ARRAY_H
#define XSTRUCTURES_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

/**
 * @brief
 * xArray object which contains data, size and capacity.
 *
 * @warning
 * "size" and "cap" attributes should not be directly modified as all other functions within xString depend on that information and
 * change them as needed. It is best to treat them as read-only unless treated properly.
 *
 */
typedef struct xArray_s {
    void **data;
    int size;
    int cap;
} xArray;

/**
 * @brief
 * Allocate and initialize new xArray object on heap.
 *
 * @return Pointer to newly allocated xArray object.
 *
 * @note
 * If function fails to allocate memory, it will return NULL.
 *
 */
xArray *xArray_new(void);

/**
 * @brief
 * Free xArray object and its data from memory.
 *
 * @param arr Target xArray object.
 *
 */
void xArray_free(xArray *arr);

/**
 * @brief
 * Resize xArray data to new capacity.
 *
 * @param arr Target xArray object.
 * @param new_cap New capacity to allocate.
 *
 * @note
 * If new_cap is less than current capacity, function will not do anything.
 * If function fails to allocate memory, it will not do anything (old data will be preserved).
 *
 */
void xArray_resize(xArray *arr, int new_cap);

/**
 * @brief
 * Shrink xArray data to fit its size.
 *
 * @param arr Target xArray object.
 *
 * @note
 * If function fails to allocate memory, it will not do anything (old data will be preserved).
 *
 */
void xArray_shrink(xArray *arr);

/**
 * @brief
 * Push item to the end of xArray.
 *
 * @param arr Target xArray object.
 * @param item Item to push.
 *
 * @note
 * If xArray is full, it will be resized to double its capacity.
 *
 */
void xArray_push(xArray *arr, void *item);

/**
 * @brief
 * Pop item from the end of xArray.
 *
 * @param arr Target xArray object.
 * @return Pointer to popped item.
 *
 * @note
 * Returned item is not freed from memory, it is only removed from xArray. It is up to the user to free it.
 *
 */
void *xArray_pop(xArray *arr);

/**
 * @brief
 * Get item from xArray at specified index.
 *
 * @param arr Target xArray object.
 * @param index Index of item to get.
 * @return Pointer to item at specified index.
 *
 * @note
 * If index is out of bounds, function will return NULL.
 *
 * @warning
 * Returned item is not copied, it is the same item that is stored in xArray. It is up to the user to copy it if needed.
 * All changes made to the item will be reflected in xArray.\
 *
 */
void *xArray_get(const xArray *arr, int index);

/**
 * @brief
 * Set item in xArray at specified index.
 *
 * @param arr Target xArray object.
 * @param index Index of item to set.
 * @param item Item data to set.
 *
 * @note
 * If index is out of bounds, function will not do anything.
 *
 * @warning
 * If there is old item at specified index, it will be freed from memory. Make copy if it is still needed.
 *
 */
void xArray_set(xArray *arr, int index, void *item);

/**
 * @brief
 * Remove item from xArray at specified index.
 *
 * @param arr Target xArray object.
 * @param index Index of item to remove.
 *
 * @note
 * If index is out of bounds, function will not do anything.
 *
 * @note
 * Removed item is automatically freed from memory. Make copy if it is still needed.
 *
 * @note
 * After removal, pointer to item at index will be NULL and no shifting will be performed.
 * Check out \ref xArray_shrink() if optimization is required.
 *
 */
void xArray_remove(xArray *arr, int index);

/**
 * @brief
 * Insert item into xArray at specified index.
 *
 * @param arr Target xArray object.
 * @param index Index to insert item at.
 * @param item Item data to insert.
 *
 * @note
 * If index is out of bounds, function will not do anything.
 *
 * @note
 * If xArray is full, it will be resized to double its capacity.
 *
 * @note
 * All items at and after specified index will be shifted to the right.
 *
 */
void xArray_insert(xArray *arr, int index, void *item);

/**
 * @brief
 * Clear all data items from xArray object.
 *
 * @param arr Target xArray object.
 *
 * @note
 * All items will be freed from memory. Make copies if they are still needed.
 *
 * @note
 * After clearing, pointer to all items will be NULL and no shrinking will be performed.
 *
 */
void xArray_clear(xArray *arr);

/**
 * @brief
 * Sort xArray items using specified comparison function.
 *
 * @param arr Target xArray object.
 * @param cmp Pointer to comparator function.
 *
 * @note
 * Comparator function should return negative value if first item precedes second item, positive value if first item follows second
 * item and zero if items are equal.
 *
 */
void xArray_sort(xArray *arr, int (*comparator)(const void *, const void *));

/**
 * @brief
 * Reverse items in xArray object.
 *
 * @param arr Target xArray object.
 *
 */
void xArray_reverse(xArray *arr);

/**
 * @brief
 * Create new xArray object with items mapped using specified function.
 *
 * @param arr Target xArray object.
 * @param func Pointer to mapping function.
 * @return Pointer to newly allocated xArray object containing mapped items.
 *
 * @note
 * Items in newly created array are allocated on heap and should be managed by user.
 * Making any changes to them will not be reflected in original array.
 *
 */
xArray *xArray_map(xArray *arr, void *(*func)(void *));

/**
 * @brief
 * Filter items in xArray object using specified test function.
 *
 * @param arr Target xArray object.
 * @param test Pointer to test function.
 * @return Pointer to newly allocated xArray object containing filtered items.
 *
 * @note
 * Test function should return non-zero value if item passes test, zero otherwise.
 *
 * @note
 * Items in newly created array are not copied but referenced from original array.
 * Making any changes to them will be reflected in original array.
 *
 */
xArray *xArray_filter(xArray *arr, int (*test)(void *));

/**
 * @brief
 * Reduce items in xArray object using specified function.
 *
 * @param arr Target xArray object.
 * @param func Pointer to reduction function.
 * @return Pointer to result of reduction.
 *
 */
void *xArray_reduce(xArray *arr, void *(*func)(void *, void *));

/**
 * @brief
 * Iterate over items in xArray object using specified function.
 *
 * @param arr Target xArray object.
 * @param func Pointer to iteration function.
 *
 */
void xArray_forEach(xArray *arr, void (*func)(void *));

/**
 * @brief
 * Concatenate two xArray objects into new xArray object.
 *
 * @param arr1 Target xArray object.
 * @param arr2 xArray object to concatenate.
 *
 */
void xArray_concat(xArray *arr1, xArray *arr2);

/**
 * @brief
 * Slice segment of xArray object into new xArray object.
 *
 * @param arr Target xArray object.
 * @param start Start index of slice.
 * @param end End index of slice.
 * @return Pointer to newly allocated xArray object containing items within index range.
 *
 */
xArray *xArray_slice(xArray *arr, int start, int end);

/**
 * @brief
 * Fill xArray object with specified item.
 *
 * @param arr Target xArray object.
 * @param item Item to fill xArray with.
 * @param start Start index of fill.
 * @param end End index of fill.
 *
 */
void xArray_fill(xArray *arr, void *item, int start, int end);

/**
 * @brief
 * Create shallow copy of xArray object.
 *
 * @param arr Target xArray object.
 * @return Pointer to newly allocated xArray object containing copied items.
 *
 * @note
 * Items in newly created array are not copied but referenced from original array.
 * Making any changes to them will be reflected in original array.
 *
 */
xArray *xArray_copy(xArray *arr);

/**
 * @brief
 * Swap two items in xArray object.
 *
 * @param arr Target xArray object.
 * @param index1 Index of first item.
 * @param index2 Index of second item.
 *
 */
void xArray_swap(xArray *arr, int index1, int index2);

#ifdef __cplusplus
}
#endif

#endif  // XSTRUCTURES_ARRAY_H
