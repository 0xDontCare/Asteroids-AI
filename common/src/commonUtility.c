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

int cu_CStringCompare(const char *str1, const char *str2)
{
    if (str1 == NULL || str2 == NULL) {
        return 0;
    }

    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i]) {
            return str1[i] - str2[i];
        }
        i++;
    }

    return str1[i] - str2[i];
}

int cu_CStringEndsWith(const char *string, const char *suffix)
{
    if (string == NULL || suffix == NULL) {
        return 0;
    }

    int stringLength = 0;
    int suffixLength = 0;
    while (string[stringLength] != '\0') {
        stringLength++;
    }
    while (suffix[suffixLength] != '\0') {
        suffixLength++;
    }

    if (suffixLength > stringLength) {
        return 0;
    }

    int i = 0;
    while (suffix[i] != '\0') {
        if (string[stringLength - suffixLength + i] != suffix[i]) {
            return 0;
        }
        i++;
    }

    return 1;
}

int cu_CStringIsAlphanumeric(const char *string)
{
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

int cu_CStringIsNumeric(const char *string)
{
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

void cu_CStringTrimNewline(char *string)
{
    if (string == NULL) {
        return;
    }

    int i = 0;
    while (string[i] != '\0') {
        if (string[i] == '\n') {
            string[i] = '\0';
            return;
        }
        i++;
    }
}
