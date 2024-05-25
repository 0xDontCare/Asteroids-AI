#include "xArray.h"
#include <stdlib.h>  // standard library (for malloc, free)

// DISABLED BECAUSE OF ISSUES
/*
static void xArray_quicksort(void *arrayData, int size, int (*comparator)(const void *a, const void *b))
{
    if (size <= 1) {
        return;
    }
    char *p = arrayData;
    char *pivot = p + (size - 1) * sizeof(*p);
    char *j = p;
    for (char *i = p; i < pivot; i += sizeof(*p)) {
        if (comparator(i, pivot) < 0) {
            if (i != j) {
                for (size_t k = 0; k < sizeof(*p); k++) {
                    char tmp = i[k];
                    i[k] = j[k];
                    j[k] = tmp;
                }
            }
            j += sizeof(*p);
        }
    }
    if (j != pivot) {
        for (size_t k = 0; k < sizeof(*p); k++) {
            char tmp = j[k];
            j[k] = pivot[k];
            pivot[k] = tmp;
        }
    }
    xArray_quicksort(p, (j - p) / sizeof(*p), comparator);
    xArray_quicksort(j + sizeof(*p), (pivot - j) / sizeof(*p), comparator);
}
*/

xArray *xArray_new(void)
{
    xArray *arr = NULL;
    if ((arr = malloc(sizeof(xArray)))) {
        arr->size = 0;
        arr->cap = 0;
        arr->data = NULL;
    }
    return arr;
}

void xArray_free(xArray *arr)
{
    if (arr) {
        if (arr->data) {
            free(arr->data);
        }
        free(arr);
    }
}

void xArray_resize(xArray *arr, int new_cap)
{
    if (arr) {
        if (new_cap > arr->cap) {
            void **new_data = malloc(sizeof(void *) * new_cap);
            if (new_data) {
                for (int i = 0; i < arr->size; i++) {
                    new_data[i] = arr->data[i];
                }
                free(arr->data);
                arr->data = new_data;
                arr->cap = new_cap;
            }
        }
    }
}

void xArray_shrink(xArray *arr)
{
    if (arr) {
        if (arr->size < arr->cap) {
            void **new_data = malloc(sizeof(void *) * arr->size);
            if (new_data) {
                for (int i = 0; i < arr->size; i++) {
                    new_data[i] = arr->data[i];
                }
                free(arr->data);
                arr->data = new_data;
                arr->cap = arr->size;
            }
        }
    }
}

void xArray_push(xArray *arr, void *item)
{
    if (arr && item) {
        if (arr->size == arr->cap) {
            int new_cap = arr->cap == 0 ? 1 : arr->cap * 2;
            xArray_resize(arr, new_cap);
        }
        arr->data[arr->size++] = item;
    }
}

void *xArray_pop(xArray *arr)
{
    void *retItem = NULL;
    if (arr && arr->size > 0) {
        arr->size--;
        retItem = arr->data[arr->size];
    }
    return retItem;
}

void *xArray_get(const xArray *arr, int index)
{
    void *retItem = NULL;
    if (arr && index >= 0 && index < arr->size) {
        retItem = arr->data[index];
    }
    return retItem;
}

void xArray_set(xArray *arr, int index, void *item)
{
    if (arr && index >= 0 && index < arr->size && item) {
        arr->data[index] = item;
    }
}

void xArray_remove(xArray *arr, int index)
{
    if (arr && index < arr->size) {
        free(arr->data[index]);
        for (int i = index; i < arr->size - 1; i++) {
            arr->data[i] = arr->data[i + 1];
        }
        arr->size--;
    }
}

void xArray_insert(xArray *arr, int index, void *item)
{
    if (arr && index >= 0 && index <= arr->size && item) {
        if (arr->size == arr->cap) {
            int new_cap = arr->cap == 0 ? 1 : arr->cap * 2;
            xArray_resize(arr, new_cap);
        }
        for (int i = arr->size; i > index; i--) {
            arr->data[i] = arr->data[i - 1];
        }
        arr->data[index] = item;
        arr->size++;
    }
}

void xArray_clear(xArray *arr)
{
    if (arr) {
        for (int i = 0; i < arr->size; i++) {
            free(arr->data[i]);
        }
        arr->size = 0;
    }
}

// TODO: fix issues with quicksort
void xArray_sort(xArray *arr, int (*comparator)(const void *, const void *))
{
    if (!arr || !arr->data || !comparator) {
        return;
    }
    // xArray_quicksort(arr->data, arr->size, comparator);

    for (int i = 0; i < arr->size; i++) {
        for (int j = i + 1; j < arr->size; j++) {
            if (comparator(arr->data[i], arr->data[j]) > 0) {
                xArray_swap(arr, i, j);
            }
        }
    }
}

void xArray_reverse(xArray *arr)
{
    if (arr) {
        for (int i = 0; i < arr->size / 2; i++) {
            void *tmp = arr->data[i];
            arr->data[i] = arr->data[arr->size - i - 1];
            arr->data[arr->size - i - 1] = tmp;
        }
    }
}

xArray *xArray_map(xArray *arr, void *(*func)(void *))
{
    xArray *newArr = NULL;
    if (arr && func) {
        if ((newArr = xArray_new())) {
            for (int i = 0; i < arr->size; i++) {
                xArray_push(newArr, func(arr->data[i]));
            }
        }
    }
    return newArr;
}

xArray *xArray_filter(xArray *arr, int (*test)(void *))
{
    xArray *newArr = NULL;
    if (arr && test) {
        if ((newArr = xArray_new())) {
            for (int i = 0; i < arr->size; i++) {
                if (test(arr->data[i])) {
                    xArray_push(newArr, arr->data[i]);
                }
            }
        }
    }
    return newArr;
}

void *xArray_reduce(xArray *arr, void *(*func)(void *, void *))
{
    void *result = NULL;
    if (arr && func) {
        if (arr->size > 0) {
            result = arr->data[0];
            for (int i = 1; i < arr->size; i++) {
                result = func(result, arr->data[i]);
            }
        }
    }
    return result;
}

void xArray_forEach(xArray *arr, void (*func)(void *))
{
    if (arr && func) {
        for (int i = 0; i < arr->size; i++) {
            func(arr->data[i]);
        }
    }
}

void xArray_concat(xArray *arr1, xArray *arr2)
{
    if (arr1 && arr2) {
        for (int i = 0; i < arr2->size; i++) {
            xArray_push(arr1, arr2->data[i]);
        }
    }
}

xArray *xArray_slice(xArray *arr, int start, int end)
{
    xArray *newArr = NULL;
    if (arr) {
        if ((newArr = xArray_new())) {
            start = (start < 0) ? 0 : start;
            end = (end < start) ? start : ((end > arr->size) ? arr->size : end);

            if (start < end) {
                for (int i = start; i < end; i++) {
                    xArray_push(newArr, arr->data[i]);
                }
            }
        }
    }
    return newArr;
}

void xArray_fill(xArray *arr, void *item, int start, int end)
{
    if (arr && item) {
        start = (start < 0) ? 0 : start;
        end = (end < start) ? start : ((end > arr->size) ? arr->size : end);

        if (start < end) {
            for (int i = start; i < end; i++) {
                arr->data[i] = item;
            }
        }
    }
}

xArray *xArray_copy(xArray *arr)
{
    xArray *newArr = NULL;
    if (arr) {
        if ((newArr = xArray_new())) {
            for (int i = 0; i < arr->size; i++) {
                xArray_push(newArr, arr->data[i]);
            }
        }
    }
    return newArr;
}

void xArray_swap(xArray *arr, int index1, int index2)
{
    if (arr && index1 >= 0 && index1 < arr->size && index2 >= 0 && index2 < arr->size) {
        void *tmp = arr->data[index1];
        arr->data[index1] = arr->data[index2];
        arr->data[index2] = tmp;
    }
}
