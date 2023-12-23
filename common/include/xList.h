/**
 * @file xList.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Linked list implemenation in xStructures module of xcFramework
 * @version 0.2
 * @date 19.12.2023.
 *
 * @copyright All rights reserved (c) 2023
 *
 */

#ifndef XSTRUCTURES_LIST_H
#define XSTRUCTURES_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

/**
 * @brief
 * Linked list node structure which contains data and pointers to next and previous node. If there is no next or previous node, pointer is NULL.
 *
 */
typedef struct xListNode {
    void *data;
    struct xListNode *next;
    struct xListNode *prev;
} xListNode;

/**
 * @brief
 * Linked list structure which contains head and tail pointers and size of list. If list is empty, head and tail pointers are NULL (and size is 0).
 *
 * @warning
 * "size" attribute should not be directly modified as all other functions within xList might depend on it and will keep it updated as needed.
 * It is best to treat it as read-only attribute unless you know what you are doing.
 *
 */
typedef struct xList {
    xListNode *head;
    xListNode *tail;
    int size;
} xList;

/**
 * @brief
 * Allocate and initialize new xList structure on heap.
 *
 * @return Pointer to new xList structure.
 *
 * @note
 * If function fails to allocate memory, it will return NULL.
 */
xList *xList_new();

/**
 * @brief
 * Free xList object from memory.
 *
 * @param list Pointer to xList object.
 *
 * @note
 * This function will not free nodes in list, only list structure itself.
 * It is recommended to free all nodes first using xList_forEach() function and passing appropriate deallocation function as argument.
 */
void xList_free(xList *list);

/**
 * @brief
 * Push new node with data to start of list.
 *
 * @param list Pointer to xList object.
 * @param data Pointer to data to be stored in new node.
 *
 * @note
 * This function will allocate new node and link pointer of data to it.
 *
 * @warning
 * If anything happens to given data pointer, it will also be reflected in list.
 */
void xList_pushFront(xList *list, void *data);

/**
 * @brief
 * Push new node with data to end of list.
 *
 * @param list Pointer to xList object.
 * @param data Pointer to data to be stored in new node.
 */
void xList_pushBack(xList *list, void *data);

/**
 * @brief
 * Remove first node from list and return its data.
 *
 * @param list Pointer to xList object.
 * @return Pointer to data stored in removed node.
 *
 * @note
 * This function will free node from memory, but data pointer needs to be freed manually after use.
 */
void *xList_popFront(xList *list);

/**
 * @brief
 * Remove last node from list and return its data.
 *
 * @param list Pointer to xList object.
 * @return Pointer to data stored in removed node.
 *
 * @note
 * This function will free node from memory, but data pointer needs to be freed manually after use.
 */
void *xList_popBack(xList *list);

/**
 * @brief
 * Return data stored in first node of list.
 *
 * @param list Pointer to xList object.
 * @return Pointer to data stored in first node.
 *
 * @note
 * Any change to returned pointer will also be reflected in list.
 */
void *xList_front(xList *list);

/**
 * @brief
 * Return data stored in last node of list.
 *
 * @param list Pointer to xList object.
 * @return Pointer to data stored in last node.
 *
 * @note
 * Any change to returned pointer will also be reflected in list.
 */
void *xList_back(xList *list);

/**
 * @brief
 * Insert new node with data at given index.
 *
 * @param list Pointer to xList object.
 * @param index Index at which new node will be inserted.
 * @param data Pointer to data to be stored in new node.
 *
 * @note
 * If index is out of bounds, no action will be taken.
 */
void xList_insert(xList *list, int index, void *data);

/**
 * @brief
 * Get data stored in node at given index.
 *
 * @param list Pointer to xList object.
 * @param index Index of node.
 * @return Pointer to data stored in node.
 *
 * @note
 * If index is out of bounds, NULL will be returned.
 *
 * @note
 * Any change to returned pointer will also be reflected in list.
 */
void *xList_get(xList *list, int index);

/**
 * @brief
 * Remove node at given index and return its data.
 *
 * @param list Pointer to xList object.
 * @param index Index of node.
 * @return Pointer to data stored in removed node.
 *
 * @note
 * This function will free node from memory, but data pointer needs to be freed manually after use.
 *
 * @note
 * If index is out of bounds, NULL will be returned and no nodes will be removed.
 */
void *xList_remove(xList *list, int index);

/**
 * @brief
 * Free all nodes from list.
 *
 * @param list Pointer to xList object.
 *
 * @note
 * This function will not free list structure itself, only nodes.
 *
 * @warning
 * This function will not free data pointers stored in nodes, only nodes themselves.
 * It is recommended to free all data pointers first using xList_forEach() function and passing appropriate deallocation function as argument.
 */
void xList_clear(xList *list);

/**
 * @brief
 * Sort list using given comparator function.
 *
 * @param list Pointer to xList object.
 * @param comparator Comparator function.
 *
 * @note
 * Comparator function should return negative value if first argument is smaller than second, positive value if first argument is larger than second and 0 if both arguments are equal.
 */
void xList_sort(xList *list, int (*comparator)(const void *, const void *));

/**
 * @brief
 * Reverse order of nodes in list.
 *
 * @param list Pointer to xList object.
 */
void xList_reverse(xList *list);

/**
 * @brief
 * Map list to new list using given function.
 *
 * @param list Pointer to xList object.
 * @param func Function to be applied to each node.
 * @return Pointer to new xList object containing mapped data.
 *
 * @note
 * Function should return pointer to new data which will be stored in new list.
 *
 * @note
 * If function fails to allocate memory at any point, NULL will be returned.
 */
xList *xList_map(xList *list, void *(*func)(void *));

/**
 * @brief
 * Filter list using given function.
 *
 * @param list Pointer to xList object.
 * @param func Function which checks if node should be included in new list.
 * @return Pointer to new xList object containing filtered data.
 *
 * @note
 * Function should return 1 if node should be included in new list, 0 otherwise.
 *
 * @note
 * If function fails to allocate memory at any point, NULL will be returned.
 */
xList *xList_filter(xList *list, int (*func)(void *));

/**
 * @brief
 * Reduce list to single value using given function.
 *
 * @param list Pointer to xList object.
 * @param func Function which reduces two nodes to single value.
 * @return Pointer to reduced value.
 *
 * @note
 * Function should return pointer to reduced value if it succeeds, NULL otherwise.
 */
void *xList_reduce(xList *list, void *(*func)(void *, void *));

/**
 * @brief
 * Apply given function to each node in list.
 *
 * @param list Pointer to xList object.
 * @param func Function to be applied to each node.
 */
void xList_forEach(xList *list, void (*func)(void *));

/**
 * @brief
 * Apply given function to each node in list with additional arguments.
 *
 * @param list Pointer to xList object.
 * @param func Function to be applied to each node.
 * @param arg Pointer to argument array.
 *
 * @note
 * Argument array should be terminated with NULL pointer as last element.
 *
 * @warning
 * If argument array is not terminated with NULL pointer, function will not stop at last element and will continue to read memory until it finds NULL pointer.
 */
void xList_forEachArg(xList *list, void (*func)(void *, void **), void **arg);

/**
 * @brief
 * Concatenate two lists into new list.
 *
 * @param list1 Pointer to first xList object.
 * @param list2 Pointer to second xList object.
 * @return Pointer to new xList object containing concatenated data.
 *
 * @note
 * If function fails to allocate memory at any point, NULL will be returned.
 *
 * @note
 * This function will not modify original lists. If they are not needed anymore, they should be freed apropiately.
 */
xList *xList_concat(xList *list1, xList *list2);

/**
 * @brief
 * Return new list containing slice of original list.
 *
 * @param list Pointer to xList object.
 * @param start Start index of slice.
 * @param end End index of slice.
 * @return Pointer to new xList object containing sliced data.
 *
 * @note
 * If start or end index is out of bounds, NULL will be returned.
 *
 * @note
 * The range includes start index and excludes end index.
 *
 * @note
 * If function fails to allocate memory at any point, NULL will be returned.
 *
 * @note
 * This function will not perform deep copy of data pointers. Any change to data pointers in original list will also be reflected in sliced list.
 */
xList *xList_slice(xList *list, int start, int end);

/**
 * @brief
 * Return new list containing copy of original list.
 *
 * @param list Pointer to xList object.
 * @return Pointer to new xList object containing copied data.
 *
 * @note
 * This function will not perform deep copy of data pointers. Any change to data pointers in original list will also be reflected in copied list.
 *
 * @note
 * If function fails to allocate memory at any point, NULL will be returned.
 *
 * @warning
 * This function will not free original list. If it is not needed anymore, it should be freed apropiately.
 */
xList *xList_copy(xList *list);

/**
 * @brief
 * Swap data of two nodes in list.
 *
 * @param list Pointer to xList object.
 * @param index1 Index of first node.
 * @param index2 Index of second node.
 *
 * @note
 * If any of indexes is out of bounds or equal, no action will be taken.
 */
void xList_swap(xList *list, int index1, int index2);

/**
 * @brief
 * Return hash of list using given hash function.
 *
 * @param list Pointer to xList object.
 * @param hash Hash function.
 * @return Hash of list.
 *
 * @note
 * Hash function should return hash of given node data depending on the type the list is storing.
 */
unsigned long long xList_hash(xList *list, unsigned long long (*hash)(void *));

#ifdef __cplusplus
}
#endif

#endif  // XSTRUCTURES_LIST_H