#include "xDictionary.h"

#include <stdlib.h>

xDictionary *xDictionary_new() {
    xDictionary *dict = NULL;
    if ((dict = malloc(sizeof(xDictionary)))) {
        dict->size = 0;
        dict->capacity = 0;
        dict->entries = NULL;
    }
    return dict;
}

void xDictionary_free(xDictionary *dict) {
    if (dict) {
        if (dict->entries) {
            free(dict->entries);
        }
        free(dict);
    }
}

void xDictionary_insert(xDictionary *dict, unsigned long long key, void *value) {
    if (dict == NULL) {
        return;
    }

    if (dict->size == dict->capacity) {
        dict->capacity = dict->capacity == 0 ? 1 : dict->capacity * 2;
        dict->entries = realloc(dict->entries, sizeof(xKeyValuePair) * dict->capacity);
    }

    // binary search for position to insert the new entry
    int left = 0;
    int right = dict->size - 1;
    int middle = 0;
    while (left <= right) {
        middle = (left + right) / 2;
        if (key < dict->entries[middle].key) {
            right = middle - 1;
        } else if (key > dict->entries[middle].key) {
            left = middle + 1;
        } else {
            // key already exists, overwrite the value
            dict->entries[middle].value = value;
            return;
        }
    }

    // shift entries to the right
    for (int i = dict->size - 1; i >= left; i--) {
        dict->entries[i + 1] = dict->entries[i];
    }

    // insert the new entry
    dict->entries[left].key = key;
    dict->entries[left].value = value;
    dict->size++;

    return;
}

void *xDictionary_get(xDictionary *dict, unsigned long long key) {
    if (dict == NULL) {
        return NULL;
    }

    // binary search for the key
    int left = 0;
    int right = dict->size - 1;
    int middle = 0;
    while (left <= right) {
        middle = (left + right) / 2;
        if (key < dict->entries[middle].key) {
            right = middle - 1;
        } else if (key > dict->entries[middle].key) {
            left = middle + 1;
        } else {
            // key found
            return dict->entries[middle].value;
        }
    }

    // key not found
    return NULL;
}

void *xDictionary_remove(xDictionary *dict, unsigned long long key) {
    if (dict == NULL) {
        return NULL;
    }

    // binary search for the key
    int left = 0;
    int right = dict->size - 1;
    int middle = 0;
    while (left <= right) {
        middle = (left + right) / 2;
        if (key < dict->entries[middle].key) {
            right = middle - 1;
        } else if (key > dict->entries[middle].key) {
            left = middle + 1;
        } else {
            // key found
            void *value = dict->entries[middle].value;

            // shift entries to the left
            for (int i = middle; i < dict->size - 1; i++) {
                dict->entries[i] = dict->entries[i + 1];
            }

            dict->size--;
            return value;
        }
    }

    // key not found
    return NULL;
}

int xDictionary_contains(xDictionary *dict, unsigned long long key) {
    if (dict == NULL) {
        return 0;
    }

    // binary search for the key
    int left = 0;
    int right = dict->size - 1;
    int middle = 0;
    while (left <= right) {
        middle = (left + right) / 2;
        if (key < dict->entries[middle].key) {
            right = middle - 1;
        } else if (key > dict->entries[middle].key) {
            left = middle + 1;
        } else {
            // key found
            return 1;
        }
    }

    // key not found
    return 0;
}
