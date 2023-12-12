/**
 * @file dynArray.c
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Implementations of array functions declared in dynArray.h
 * @version 0.1
 * @date 11.12.2023.
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "dynArray.h"

#include <stdlib.h>

DynArray *newDynArray(size_t capacity, size_t elementSize) {
    DynArray *dynArray = malloc(sizeof(DynArray));
    dynArray->data = malloc(capacity * elementSize);
    dynArray->size = 0;
    dynArray->capacity = capacity;
    dynArray->elementSize = elementSize;
    return dynArray;
}

void destroyDynArray(DynArray *dynArray) {
    free(dynArray->data);
    free(dynArray);
}

size_t dynArrayAdd(DynArray *dynArray, void *element) {
    if (dynArray->size == dynArray->capacity) {
        dynArray->capacity *= 2;
        dynArray->data = realloc(dynArray->data, dynArray->capacity * dynArray->elementSize);
    }
    dynArray->data[dynArray->size] = element;
    dynArray->size++;
    return dynArray->size - 1;
}

void dynArrayRemove(DynArray *dynArray, size_t index) {
    for (size_t i = index; i < dynArray->size - 1; i++) {
        dynArray->data[i] = dynArray->data[i + 1];
    }
    dynArray->size--;
}

void *dynArrayGet(DynArray *dynArray, size_t index) {
    if(index >= dynArray->size) return NULL; // TODO: error handling (return error code)
    return dynArray->data[index];
}

size_t dynArraySize(DynArray *dynArray) {
    return dynArray->size;
}

size_t dynArrayCapacity(DynArray *dynArray) {
    return dynArray->capacity;
}

void *dynArrayLast(DynArray *dynArray) {
    return dynArray->data[dynArray->size - 1];
}

void *dynArrayFirst(DynArray *dynArray) {
    return dynArray->data[0];
}

void dynArrayClear(DynArray *dynArray) {
    dynArray->size = 0;
}

void dynArrayForeach(DynArray *dynArray, void (*foreach)(void *element)) {
    for (size_t i = 0; i < dynArray->size; i++) {
        foreach(dynArray->data[i]);
    }
}

DynArray *dynArrayFilter(DynArray *dynArray, int (*filter)(void *element)) {
    DynArray *tmpDynArray = newDynArray(dynArray->capacity, dynArray->elementSize);
    for (size_t i = 0; i < dynArray->size; i++) {
        if (filter(dynArray->data[i])) {
            dynArrayAdd(tmpDynArray, dynArray->data[i]);
        }
    }
    return tmpDynArray;
}

DynArray *dynArrayMap(DynArray *dynArray, void *(*map)(void *element)) {
    DynArray *tmpDynArray = newDynArray(dynArray->capacity, dynArray->elementSize);
    for (size_t i = 0; i < dynArray->size; i++) {
        dynArrayAdd(tmpDynArray, map(dynArray->data[i]));
    }
    return tmpDynArray;
}

void *dynArrayReduce(DynArray *dynArray, void *(*reduce)(void *accumulator, void *element), void *initial) {
    void *accumulator = initial;
    for (size_t i = 0; i < dynArray->size; i++) {
        accumulator = reduce(accumulator, dynArray->data[i]);
    }
    return accumulator;
}

void *dynArrayFind(DynArray *dynArray, int (*find)(void *element)) {
    for (size_t i = 0; i < dynArray->size; i++) {
        if (find(dynArray->data[i])) {
            return dynArray->data[i];
        }
    }
    return NULL;
}
