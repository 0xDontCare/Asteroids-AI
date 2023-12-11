/**
 * @file commonUtility.c
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Implementations of functions declared in commonUtility.h
 * @version 0.2
 * @date 04.12.2023.
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "commonUtility.h"

#include <stddef.h>

int cu_CStringIsAlphanumeric(char *string) {
    if (string == NULL) {
        return 1;
    }

    int i = 0;
    while (string[i] != '\0') {
        if (string[i] < '0' || (string[i] > '9' && string[i] < 'A') || (string[i] > 'Z' && string[i] < 'a') || string[i] > 'z') {
            return 0;
        }
        i++;
    }
    return 1;
}

int cu_CStringIsNumeric(char *string) {
    if (string == NULL) {
        return 0;
    }

    int i = 0;
    while (string[i] != '\0') {
        if (string[i] < '0' || string[i] > '9') {
            return 0;
        }
        i++;
    }
    return 1;
}