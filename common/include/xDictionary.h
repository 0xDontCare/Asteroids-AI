/**
 * @file xDictionary.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Dictionary implementation in xStructures module. Inspired by TreeMap structure in Java.
 * @version 0.1
 * @date 19.12.2023.
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef X_DICTIONARY_H
#define X_DICTIONARY_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Key-value pair structure.
 *
 */
typedef struct xKeyValuePair_s {
    unsigned long long key;  // hash of the key
    void *value;        // pointer to the value
} xKeyValuePair;

/**
 * @brief Dictionary structure.
 *
 */
typedef struct xDictionary_s {
    xKeyValuePair *entries;  // array of key-value pairs (ascending order by key)
    int size;                // number of entries
    int capacity;            // number of entries that can be stored
} xDictionary;

/**
 * @brief
 * Create a new xDictionary object.
 *
 * @return Pointer to newly allocated xDictionary object.
 *
 * @note
 * If function fails to allocate memory, it will return NULL.
 */
xDictionary *xDictionary_new();

/**
 * @brief
 * Free xDictionary object from memory.
 *
 * @param dictionary Target xDictionary object.
 *
 * @note
 * This function does not free the values stored in the dictionary. They should be handled before freeing the dictionary if not needed.
 */
void xDictionary_free(xDictionary *dict);

/**
 * @brief
 * Insert a new key-value pair into the dictionary.
 *
 * @param dictionary Target xDictionary object.
 * @param key Key hash of the new entry.
 * @param value Pointer to the value of the new entry.
 *
 * @warning
 * If the key already exists, the value will be overwritten.
 *
 * @note
 * The key hash function should be implemented manually depending on the key type.
 */
void xDictionary_insert(xDictionary *dict, unsigned long long key, void *value);

/**
 * @brief
 * Get the value of the entry with the given key.
 *
 * @param dictionary Target xDictionary object.
 * @param key Key hash of the entry.
 * @return Pointer to the value of the entry.
 *
 * @note
 * If the key does not exist, the function will return NULL.
 */
void *xDictionary_get(xDictionary *dict, unsigned long long key);

/**
 * @brief
 * Remove the entry with the given key from the dictionary.
 *
 * @param dictionary Target xDictionary object.
 * @param key Key hash of the entry.
 * @return Pointer to the value of the removed entry.
 *
 * @note
 * If the key does not exist, the function will return NULL and do nothing to the dictionary.
 *
 * @note
 * The value of the removed entry is not freed from memory but rather returned to the user. It is up to the user to free it or store it elsewhere.
 */
void *xDictionary_remove(xDictionary *dict, unsigned long long key);

/**
 * @brief
 * Check if the dictionary contains an entry with the given key.
 *
 * @param dictionary Target xDictionary object.
 * @param key Key hash of the entry.
 * @return 1 if the dictionary contains the key, 0 otherwise.
 */
int xDictionary_contains(xDictionary *dict, unsigned long long key);

#ifdef __cplusplus
}
#endif

#endif  // X_DICTIONARY_H
