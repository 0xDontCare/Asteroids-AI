#include "commonUtility.h"
#include <stdlib.h>  // standard library (for malloc, free, realloc)

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

int cu_CStringStartsWith(const char *string, const char *prefix)
{
    if (string == NULL || prefix == NULL) {
        return 0;
    }

    for (int i = 0; prefix[i] != '\0'; i++) {
        if (string[i] == '\0') {
            return 0;
        }
        if (string[i] != prefix[i]) {
            return 0;
        }
    }

    return 1;
}

int cu_CStringEndsWith(const char *string, const char *suffix)
{
    if (string == NULL || suffix == NULL) {
        return 0;
    }

    int stringLength = cu_CStringLength(string);
    int suffixLength = cu_CStringLength(suffix);

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
    if (string == NULL || *string == '\0') {
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

unsigned long long cu_CStringHash(const char *string)
{
    unsigned long long hash = 0xcbf29ce484222325;  // FNV offset basis
    while (*string) {
        hash ^= (unsigned char)*string;
        hash *= 0x100000001b3;  // FNV prime
        string++;
    }
    return hash;
}

unsigned int cu_CStringLength(const char *string)
{
    if (string == NULL || *string == '\0') {
        return 0;
    }

    unsigned int len = 0;
    while (string[len++] != '\0')
        ;
    return len - 1;
}

void cu_CStringConcat(char **dest, const char *src)
{
    if (src == NULL) {
        return;
    }
    if (*dest == NULL) {
        // create copy of src string
        unsigned int srcLen = cu_CStringLength(src);
        *dest = (char *)malloc(sizeof(char) * (srcLen + 1));
        if (dest == NULL) {
            return;
        }
        for (int i = 0; src[i] != '\0'; i++) {
            (*dest)[i] = src[i];
        }
        (*dest)[srcLen] = '\0';
    } else {
        // concatenate src string to dest string
        unsigned int destLen = cu_CStringLength(*dest);
        unsigned int srcLen = cu_CStringLength(src);
        *dest = (char *)realloc(*dest, sizeof(char) * (destLen + srcLen + 1));
        if (*dest == NULL) {
            return;
        }
        for (int i = 0; src[i] != '\0'; i++) {
            (*dest)[destLen + i] = src[i];
        }
        (*dest)[destLen + srcLen] = '\0';
    }
}

int cu_CStringToInteger(const char *string)
{
    if (string == NULL || *string == '\0')
        return 0;

    int result = 0;
    int sign = 1;
    if (*string == '-') {
        sign = -1;
        string++;
    }
    while (*string != '\0') {
        if (*string < '0' || *string > '9') {
            break;
        }
        result = result * 10 + (*string - '0');
        string++;
    }
    return result * sign;
}
