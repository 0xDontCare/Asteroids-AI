#include "xString.h"

#include <stdlib.h>

xString *xString_new(void)
{
    xString *str = NULL;
    if ((str = malloc(sizeof(xString)))) {
        str->data = NULL;
        str->len = 0;
        str->cap = 0;
    }
    return str;
}

void xString_free(xString *str)
{
    if (str) {
        if (str->data) {
            free(str->data);
        }
        free(str);
    }
}

void xString_optimize(xString *str)
{
    if (str) {
        if (str->len < str->cap) {
            unsigned char *newData = NULL;
            if ((newData = malloc(str->len))) {
                int i;
                for (i = 0; i < str->len; i++) {
                    newData[i] = str->data[i];
                }
                free(str->data);
                str->data = newData;
                str->cap = str->len;
            }
        }
    }
}

void xString_preallocate(xString *str, int len)
{
    if (str && len > 0) {
        unsigned char *newData = NULL;
        if ((newData = malloc(len))) {
            free(str->data);
            str->data = newData;
            str->len = 0;
            str->cap = len;
        }
    }
}

/**
 * @brief Get length of C-style string.
 *
 * @param cstr Pointer to C-style string.
 * @return int Length of C-style string.
 *
 * @note If cstr is NULL, the function returns 0.
 *
 * @note The function returns the length of the string, not including the null
 * terminator.
 *
 * @warning It is important that the string is null-terminated, otherwise the
 * function will continue reading memory until it finds a null terminator or the
 * program crashes.
 */
static int xString_CStringLen(const char *cstr)
{
    int len = 0;
    if (cstr) {
        while (cstr[len] != '\0') {
            len++;
        }
    }
    return len;
}

void xString_append(xString *str, unsigned char *data, int len)
{
    if (str && data && len > 0) {
        if (str->len + len > str->cap) {
            int newCap = str->cap * 2;
            if (newCap < str->len + len) {
                newCap = str->len + len;
            }
            unsigned char *newData = NULL;
            if ((newData = malloc(newCap))) {
                if (str->data) {
                    for (int i = 0; i < str->len; i++) {
                        newData[i] = str->data[i];
                    }
                    free(str->data);
                }
                str->data = newData;
                str->cap = newCap;
            }
        }
        if (str->data) {
            for (int i = 0; i < len; i++) {
                str->data[str->len + i] = data[i];
            }
            str->len += len;
        }
    }
}

void xString_appendChar(xString *str, unsigned char c)
{
    if (str) {
        xString_append(str, &c, 1);
    }
}

void xString_appendString(xString *str, xString *str2)
{
    if (str && str2) {
        xString_append(str, str2->data, str2->len);
    }
}

void xString_appendCString(xString *str, const char *cstr)
{
    if (str && cstr) {
        xString_append(str, (unsigned char *)cstr, xString_CStringLen(cstr));
    }
}

xString *xString_copy(const xString *str)
{
    xString *str2 = NULL;
    if (str) {
        if ((str2 = xString_new())) {
            xString_append(str2, str->data, str->len);
        }
    }
    return str2;
}

xString *xString_substring(const xString *str, int start, int end)
{
    xString *str2 = NULL;
    if ((str2 = xString_new())) {
        if (str && start >= 0 && end >= 0 && start <= end && end <= str->len) {
            xString_append(str2, str->data + start, end - start);
        }
    }
    return str2;
}

/**
 * @brief Create longest prefix suffix array for use in the Knuth-Morris-Pratt
 * algorithm.
 *
 * @param data Pointer to data block.
 * @param len Length of data block.
 * @return int* Pointer to allocated longest prefix suffix array, or NULL if
 * allocation failed.
 *
 * @note The longest prefix suffix array is used in the Knuth-Morris-Pratt
 * algorithm to avoid unnecessary comparisons.
 *
 */
static int *xString_buildLPS(unsigned char *data, int len)
{
    int *lps = NULL;
    if (data && len > 0) {
        if ((lps = malloc(len * sizeof(int)))) {
            lps[0] = 0;
            int i = 1;
            int j = 0;
            while (i < len) {
                if (data[i] == data[j]) {
                    lps[i] = j + 1;
                    i++;
                    j++;
                } else {
                    if (j != 0) {
                        j = lps[j - 1];
                    } else {
                        lps[i] = 0;
                        i++;
                    }
                }
            }
        }
    }
    return lps;
}

/*
static int xString_intPow(int x, int n) {
    int res = 1;

    while (n > 0) {
        if (n & 1) {
            res *= x;
        }
        x *= x;
        n >>= 1;
    }
    return res;
}

static int xString_hashData(unsigned char *data, int len, int startIndex) {
    // hash data for use in Rabin-Karp pattern search algorithm

    int hashValue = 0;

    if (data && len > 0) {
        for (int i = 0; i < len; i++) {
            hashValue = (256 * hashValue + (int)(data[startIndex + i])) % 10007;
        }
    }

    return hashValue;
}

static int xString_rehashData(int hashValue, unsigned char oldByte, unsigned
char newByte, int dataLen) {
    // update hash value for use in the Rabin-Karp algorithm

    if (dataLen <= 0) {
        return 0;
    }

    int maxOrder = 1;

    for (int i = 0; i < dataLen - 1; i++) {
        maxOrder = (maxOrder * 256) % 10007;
    }

    int newHash = (256 * (hashValue - (int)oldByte * maxOrder) + (int)(newByte))
% 10007; if (newHash < 0) { newHash += 10007;
    }

    return newHash;
}
*/

int xString_find(xString *str, unsigned char *data, int len)
{
    // search using the Knuth-Morris-Pratt algorithm

    int index = -1;
    if (str && data && len > 0 && str->len >= len) {
        int *lps = xString_buildLPS(data, len);
        if (lps) {
            int i = 0;
            int j = 0;
            while (i < str->len) {
                if (str->data[i] == data[j]) {
                    i++;
                    j++;
                }
                if (j == len) {
                    index = i - j;
                    break;
                } else if (i < str->len && str->data[i] != data[j]) {
                    if (j != 0) {
                        j = lps[j - 1];
                    } else {
                        i++;
                    }
                }
            }
            free(lps);
        }
    }
    return index;
}

int xString_findChar(xString *str, unsigned char c)
{
    int index = -1;
    if (str) {
        int i;
        for (i = 0; i < str->len; i++) {
            if (str->data[i] == c) {
                index = i;
                break;
            }
        }
    }
    return index;
}

int xString_findString(xString *str, xString *str2) { return xString_find(str, str2->data, str2->len); }

int xString_findCString(xString *str, char *cstr) { return xString_find(str, (unsigned char *)cstr, xString_CStringLen(cstr)); }

int xString_findLast(xString *str, unsigned char *data, int len)
{
    // find last occurrence of substring in string using the Knuth-Morris-Pratt
    // algorithm

    int index = -1;
    int *lps = NULL;

    if (str && data && len > 0 && str->len >= len) {
        lps = xString_buildLPS(data, len);
        if (lps) {
            int i = 0;
            int j = 0;

            while ((str->len - i) >= (len - j)) {
                if (str->data[i] == data[j]) {
                    i++;
                    j++;
                }
                if (j == len) {
                    index = i - j;
                    j = lps[j - 1];
                } else if ((i < str->len) && (str->data[i] != data[j])) {
                    if (j != 0) {
                        j = lps[j - 1];
                    } else {
                        i++;
                    }
                }
            }

            free(lps);
        }
    }

    return index;
}

int xString_findLastChar(xString *str, unsigned char c)
{
    int index = -1;

    if (str) {
        int i;
        for (i = str->len - 1; i >= 0; i--) {
            if (str->data[i] == c) {
                index = i;
                break;
            }
        }
    }
    return index;
}

int xString_findLastString(xString *str, xString *str2) { return xString_findLast(str, str2->data, str2->len); }

int xString_findLastCString(xString *str, char *cstr)
{
    return xString_findLast(str, (unsigned char *)cstr, xString_CStringLen(cstr));
}

int *xString_findAll(const xString *str, unsigned char *data, int len)
{
    int *indices = NULL;
    if (str && data && len > 0 && str->len >= len) {
        int *lps = xString_buildLPS(data, len);
        if (!lps) {
            return NULL;
        }
        int i = 0, j = 0, count = 0, last = -1;
        while (i < str->len) {
            if (str->data[i] == data[j]) {
                i++;
                j++;
            }
            if (j == len) {
                if (last == -1 || i - j > last) {
                    count++;
                    last = i - 1;
                }
                j = lps[j - 1];
            } else if (i < str->len && str->data[i] != data[j]) {
                if (j != 0) {
                    j = lps[j - 1];
                } else {
                    i++;
                }
            }
        }
        indices = malloc((count + 1) * sizeof(int));
        if (!indices) {
            free(lps);
            return NULL;
        }
        i = 0, j = 0, count = 0, last = -1;
        while (i < str->len) {
            if (str->data[i] == data[j]) {
                i++;
                j++;
            }
            if (j == len) {
                if (last == -1 || i - j > last) {
                    indices[count] = i - j;
                    count++;
                    last = i - 1;
                }
                j = lps[j - 1];
            } else if (i < str->len && str->data[i] != data[j]) {
                if (j != 0) {
                    j = lps[j - 1];
                } else {
                    i++;
                }
            }
        }
        indices[count] = -1;
        free(lps);
    } else {
        indices = malloc(sizeof(int));
        if (indices) {
            *indices = -1;
        }
    }
    return indices;
}

int *xString_findAllChar(xString *str, unsigned char c)
{
    int *indices = NULL;
    if (str && str->len > 0) {
        int count = 0;
        for (int i = 0; i < str->len; i++) {
            if (str->data[i] == c) {
                count++;
            }
        }
        if ((indices = malloc((count + 1) * sizeof(int)))) {
            count = 0;
            for (int i = 0; i < str->len; i++) {
                if (str->data[i] == c) {
                    indices[count] = i;
                    count++;
                }
            }
            indices[count] = -1;
        }
    } else {
        if ((indices = malloc(sizeof(int)))) {
            *indices = -1;
        }
    }
    return indices;
}

int *xString_findAllString(xString *str, xString *str2) { return xString_findAll(str, str2->data, str2->len); }

int *xString_findAllCString(xString *str, char *cstr)
{
    return xString_findAll(str, (unsigned char *)cstr, xString_CStringLen(cstr));
}

int *xString_findAll_overlapping(xString *str, unsigned char *data, int len)
{
    int *indices = NULL;
    if (str && data && len > 0 && str->len >= len) {
        int *lps = xString_buildLPS(data, len);
        if (!lps) {
            return NULL;
        }
        int i = 0, j = 0, count = 0;
        while (i < str->len) {
            if (str->data[i] == data[j]) {
                i++;
                j++;
            }
            if (j == len) {
                count++;
                j = lps[j - 1];
            } else if (i < str->len && str->data[i] != data[j]) {
                if (j != 0) {
                    j = lps[j - 1];
                } else {
                    i++;
                }
            }
        }
        indices = malloc((count + 1) * sizeof(int));
        if (!indices) {
            free(lps);
            return NULL;
        }
        i = 0, j = 0, count = 0;
        while (i < str->len) {
            if (str->data[i] == data[j]) {
                i++;
                j++;
            }
            if (j == len) {
                indices[count] = i - j;
                count++;
                j = lps[j - 1];
            } else if (i < str->len && str->data[i] != data[j]) {
                if (j != 0) {
                    j = lps[j - 1];
                } else {
                    i++;
                }
            }
        }
        indices[count] = -1;
        free(lps);
    } else {
        indices = malloc(sizeof(int));
        if (indices) {
            *indices = -1;
        }
    }
    return indices;
}

int *xString_findAllString_overlapping(xString *str, xString *str2)
{
    return xString_findAll_overlapping(str, str2->data, str2->len);
}

int *xString_findAllCString_overlapping(xString *str, char *cstr)
{
    return xString_findAll_overlapping(str, (unsigned char *)cstr, xString_CStringLen(cstr));
}

int xString_count(xString *str, unsigned char *data, int len)
{
    int count = 0;
    if (str && data && len > 0 && str->len >= len) {
        int *lps = xString_buildLPS(data, len);
        if (!lps) {
            return 0;
        }
        int i = 0, j = 0, last = -1;
        while (i < str->len) {
            if (str->data[i] == data[j]) {
                i++;
                j++;
            }
            if (j == len) {
                if (last == -1 || i - j > last) {
                    count++;
                    last = i - 1;
                }
                j = lps[j - 1];
            } else if (i < str->len && str->data[i] != data[j]) {
                if (j != 0) {
                    j = lps[j - 1];
                } else {
                    i++;
                }
            }
        }
        free(lps);
    }
    return count;
}

int xString_countChar(xString *str, unsigned char c)
{
    // count occurrences of character in string using linear pass through string

    int count = 0;

    if (str) {
        for (int i = 0; i < str->len; i++) {
            if (str->data[i] == c) {
                count++;
            }
        }
    }

    return count;
}

int xString_countString(xString *str, xString *str2) { return xString_count(str, str2->data, str2->len); }

int xString_countCString(xString *str, char *cstr) { return xString_count(str, (unsigned char *)cstr, xString_CStringLen(cstr)); }

int xString_count_overlapping(xString *str, unsigned char *data, int len)
{
    // count occurrences of substring in string using Knuth-Morris-Pratt
    // algorithm (overlapping matches included)

    int count = 0;
    if (str && data && len > 0 && str->len >= len) {
        int *lps = xString_buildLPS(data, len);
        if (lps) {
            int i = 0;
            int j = 0;
            while (i < str->len) {
                if (str->data[i] == data[j]) {
                    i++;
                    j++;
                }
                if (j == len) {
                    count++;
                    j = lps[j - 1];
                } else if (i < str->len && str->data[i] != data[j]) {
                    if (j != 0) {
                        j = lps[j - 1];
                    } else {
                        i++;
                    }
                }
            }
            free(lps);
        }
    }
    return count;
}

int xString_countString_overlapping(xString *str, xString *str2) { return xString_count_overlapping(str, str2->data, str2->len); }

int xString_countCString_overlapping(xString *str, char *cstr)
{
    return xString_count_overlapping(str, (unsigned char *)cstr, xString_CStringLen(cstr));
}

void xString_replaceFirst(xString *str, unsigned char *data, int len, unsigned char *data2, int len2)
{
    // replace first occurrence of substring

    if (str && data && data2 && len > 0 && len2 >= 0) {
        int index = xString_find(str, data, len);
        if (index >= 0) {
            int newLen = str->len - len + len2;
            if (newLen > str->cap) {
                int newCap = str->cap * 2;
                if (newCap < newLen) {
                    newCap = newLen;
                }
                unsigned char *newData = NULL;
                if ((newData = malloc(newCap * sizeof(unsigned char)))) {
                    int i;
                    for (i = 0; i < index; i++) {
                        newData[i] = str->data[i];
                    }
                    for (i = 0; i < len2; i++) {
                        newData[index + i] = data2[i];
                    }
                    for (i = index + len; i < str->len; i++) {
                        newData[i + len2 - len] = str->data[i];
                    }
                    free(str->data);
                    str->data = newData;
                    str->len = newLen;
                    str->cap = newCap;
                }
            } else {
                if (len2 > len) {
                    for (int i = newLen - 1; i >= index + len2; i--) {
                        str->data[i] = str->data[i + len - len2];
                    }
                }

                for (int i = 0; i < len2; i++) {
                    str->data[index + i] = data2[i];
                }

                if (len2 < len) {
                    for (int i = index + len2; i <= str->len && i + len - len2 < str->len; i++) {
                        str->data[i] = str->data[i + len - len2];
                    }
                }
                str->len = newLen;
            }
        }
    }
}

void xString_replaceFirstChar(xString *str, unsigned char c, unsigned char c2)
{
    for (int i = 0; i < str->len; i++) {
        if (str->data[i] == c) {
            str->data[i] = c2;
            break;
        }
    }
}

void xString_replaceFirstString(xString *str, xString *str2, xString *str3)
{
    xString_replaceFirst(str, str2->data, str2->len, str3->data, str3->len);
}

void xString_replaceFirstCString(xString *str, char *cstr, char *cstr2)
{
    xString_replaceFirst(str, (unsigned char *)cstr, xString_CStringLen(cstr), (unsigned char *)cstr2, xString_CStringLen(cstr2));
}

void xString_replaceLast(xString *str, unsigned char *data, int len, unsigned char *data2, int len2)
{
    if (str && data && data2 && len > 0 && len2 >= 0) {
        int index = xString_findLast(str, data, len);
        if (index >= 0) {
            int newLen = str->len - len + len2;
            if (newLen > str->cap) {
                int newCap = str->cap * 2;
                if (newCap < newLen) {
                    newCap = newLen;
                }
                unsigned char *newData = NULL;
                if ((newData = malloc(newCap * sizeof(unsigned char)))) {
                    int i;
                    for (i = 0; i < index; i++) {
                        newData[i] = str->data[i];
                    }
                    for (i = 0; i < len2; i++) {
                        newData[index + i] = data2[i];
                    }
                    for (i = index + len; i < str->len; i++) {
                        newData[i + len2 - len] = str->data[i];
                    }
                    free(str->data);
                    str->data = newData;
                    str->len = newLen;
                    str->cap = newCap;
                }
            } else {
                if (len2 > len) {
                    for (int i = newLen - 1; i >= index + len2; i--) {
                        str->data[i] = str->data[i + len - len2];
                    }
                }

                for (int i = 0; i < len2; i++) {
                    str->data[index + i] = data2[i];
                }

                if (len2 < len) {
                    for (int i = index + len2; i <= str->len && i + len - len2 < str->len; i++) {
                        str->data[i] = str->data[i + len - len2];
                    }
                }
                str->len = newLen;
            }
        }
    }
}

void xString_replaceLastChar(xString *str, unsigned char c, unsigned char c2)
{
    for (int i = str->len - 1; i >= 0; i--) {
        if (str->data[i] == c) {
            str->data[i] = c2;
            break;
        }
    }
}

void xString_replaceLastString(xString *str, xString *str2, xString *str3)
{
    xString_replaceLast(str, str2->data, str2->len, str3->data, str3->len);
}

void xString_replaceLastCString(xString *str, char *cstr, char *cstr2)
{
    xString_replaceLast(str, (unsigned char *)cstr, xString_CStringLen(cstr), (unsigned char *)cstr2, xString_CStringLen(cstr2));
}

void xString_replace(xString *str, unsigned char *data, int len, unsigned char *data2, int len2)
{
    // replace all occurrences of substring in string (non-overlapping search)

    if (str && data && data2 && len > 0 && len2 >= 0) {
        int *indices = xString_findAll(str, data, len);
        if (!indices) {
            return;
        }

        int insertsCount = 0;
        while (indices[insertsCount] >= 0) {
            insertsCount++;
        }

        if (insertsCount == 0) {
            free(indices);
            return;
        }

        int lenDiff = len2 - len;
        int newLen = str->len + lenDiff * insertsCount;

        if (newLen == 0) {
            str->len = 0;
            free(indices);
            return;
        }

        if (newLen > str->cap) {
            int newCap = str->cap * 2;
            if (newCap < newLen) {
                newCap = newLen;
            }
            unsigned char *newData = NULL;
            if ((newData = malloc(newCap * sizeof(unsigned char)))) {
                for (int i = 0, j = 0; i < str->len; i++) {
                    if (i == indices[j]) {
                        for (int k = 0; k < len2; k++) {
                            newData[i + k] = data2[k];
                        }
                        i += len - 1;
                        j++;
                    } else {
                        newData[i + lenDiff * j] = str->data[i];
                    }
                }
                free(str->data);
                str->data = newData;
                str->len = newLen;
                str->cap = newCap;
            }
        } else {
            int offset = 0;
            int replacementOffset = 0;

            for (int i = 0; indices[i] >= 0; i++) {
                int index = indices[i] + offset;
                int shift = len - len2;

                while (shift > 0) {
                    str->data[index + replacementOffset] = str->data[index + replacementOffset + shift];
                    shift--;
                }

                for (int j = 0; j < len2; j++) {
                    str->data[index + replacementOffset + j] = data2[j];
                }

                offset += len2 - len;
                replacementOffset += len2;
            }
            str->len = newLen;
        }

        free(indices);
    }
}

void xString_replaceChar(xString *str, unsigned char c, unsigned char c2)
{
    if (str && str->data && str->len > 0) {
        for (int i = 0; i < str->len; i++) {
            if (str->data[i] == c) {
                str->data[i] = c2;
            }
        }
    }
}

void xString_replaceString(xString *str, xString *str2, xString *str3)
{
    xString_replace(str, str2->data, str2->len, str3->data, str3->len);
}

void xString_replaceCString(xString *str, char *cstr, char *cstr2)
{
    xString_replace(str, (unsigned char *)cstr, xString_CStringLen(cstr), (unsigned char *)cstr2, xString_CStringLen(cstr2));
}

void xString_remove(xString *str, int start, int end)
{
    if (str && start >= 0 && end >= 0 && start <= end && end <= str->len) {
        int len = end - start;
        if (len > 0) {
            for (int i = start; i < str->len - len; i++) {
                str->data[i] = str->data[i + len];
            }
            str->len -= len;
        }
    }
}

void xString_removeFirstBlock(xString *str, unsigned char *data, int len)
{
    if (str && data && len > 0 && str->len >= len) {
        int index = xString_find(str, data, len);
        if (index >= 0) {
            xString_remove(str, index, index + len);
        }
    }
}

void xString_removeLastBlock(xString *str, unsigned char *data, int len)
{
    if (str && data && len > 0 && str->len >= len) {
        int index = xString_findLast(str, data, len);
        if (index >= 0) {
            xString_remove(str, index, index + len);
        }
    }
}

void xString_removeAllBlock(xString *str, unsigned char *data, int len)
{
    if (str && data && len > 0 && str->len >= len) {
        int *indices = xString_findAll(str, data, len);
        if (indices) {
            int i = 0;
            while (indices[i] >= 0) {
                xString_remove(str, indices[i] - i * len, indices[i] + len * (1 - i));
                i++;
            }
            free(indices);
        }
    }
}

void xString_removeFirstChar(xString *str, unsigned char c) { xString_removeFirstBlock(str, &c, 1); }

void xString_removeLastChar(xString *str, unsigned char c) { xString_removeLastBlock(str, &c, 1); }

void xString_removeAllChar(xString *str, unsigned char c) { xString_removeAllBlock(str, &c, 1); }

void xString_removeFirstString(xString *str, xString *str2) { xString_removeFirstBlock(str, str2->data, str2->len); }

void xString_removeLastString(xString *str, xString *str2) { xString_removeLastBlock(str, str2->data, str2->len); }

void xString_removeAllString(xString *str, xString *str2) { xString_removeAllBlock(str, str2->data, str2->len); }

void xString_removeFirstCString(xString *str, char *cstr)
{
    xString_removeFirstBlock(str, (unsigned char *)cstr, xString_CStringLen(cstr));
}

void xString_removeLastCString(xString *str, char *cstr)
{
    xString_removeLastBlock(str, (unsigned char *)cstr, xString_CStringLen(cstr));
}

void xString_removeAllCString(xString *str, char *cstr)
{
    xString_removeAllBlock(str, (unsigned char *)cstr, xString_CStringLen(cstr));
}

void xString_removeAllWhitespace(xString *str)
{
    if (str) {
        int i;
        for (i = 0; i < str->len; i++) {
            if (str->data[i] == ' ' || str->data[i] == '\t' || str->data[i] == '\n' || str->data[i] == '\r') {
                xString_remove(str, i, i + 1);
                i--;
            }
        }
    }
}

void xString_removeAllDigits(xString *str)
{
    if (str) {
        int i;
        for (i = 0; i < str->len; i++) {
            if (str->data[i] >= '0' && str->data[i] <= '9') {
                xString_remove(str, i, i + 1);
                i--;
            }
        }
    }
}

void xString_removeAllLetters(xString *str)
{
    if (str) {
        int i;
        for (i = 0; i < str->len; i++) {
            if ((str->data[i] >= 'a' && str->data[i] <= 'z') || (str->data[i] >= 'A' && str->data[i] <= 'Z')) {
                xString_remove(str, i, i + 1);
                i--;
            }
        }
    }
}

void xString_removeAllUppercase(xString *str)
{
    if (str) {
        int i;
        for (i = 0; i < str->len; i++) {
            if (str->data[i] >= 'A' && str->data[i] <= 'Z') {
                xString_remove(str, i, i + 1);
                i--;
            }
        }
    }
}

void xString_removeAllLowercase(xString *str)
{
    if (str) {
        int i;
        for (i = 0; i < str->len; i++) {
            if (str->data[i] >= 'a' && str->data[i] <= 'z') {
                xString_remove(str, i, i + 1);
                i--;
            }
        }
    }
}

void xString_removeAllSpecial(xString *str)
{
    if (str) {
        int i;
        for (i = 0; i < str->len; i++) {
            if ((str->data[i] >= '!' && str->data[i] <= '/') || (str->data[i] >= ':' && str->data[i] <= '@') ||
                (str->data[i] >= '[' && str->data[i] <= '`') || (str->data[i] >= '{' && str->data[i] <= '~')) {
                xString_remove(str, i, i + 1);
                i--;
            }
        }
    }
}

void xString_removeAllNewlines(xString *str)
{
    if (str) {
        int i;
        for (i = 0; i < str->len; i++) {
            if (str->data[i] == '\n' || str->data[i] == '\r') {
                xString_remove(str, i, i + 1);
                i--;
            }
        }
    }
}

int xString_isEmpty(const xString *str) { return str ? str->len == 0 : 1; }

int xString_compare(xString *str, xString *str2)
{
    int result = 0;
    if (str && str2) {
        int i;
        for (i = 0; i < str->len && i < str2->len; i++) {
            if (str->data[i] < str2->data[i]) {
                result = -1;
                break;
            } else if (str->data[i] > str2->data[i]) {
                result = 1;
                break;
            }
        }
        if (result == 0) {
            if (str->len < str2->len) {
                result = -1;
            } else if (str->len > str2->len) {
                result = 1;
            }
        }
    }
    return result;
}

int xString_compareCString(xString *str, char *cstr)
{
    xString *str2 = xString_new();
    xString_appendCString(str2, cstr);
    int result = xString_compare(str, str2);
    xString_free(str2);
    return result;
}

int xString_compareIgnoreCase(xString *str, xString *str2)
{
    int result = 0;
    if (str && str2) {
        int i;
        for (i = 0; i < str->len && i < str2->len; i++) {
            if (str->data[i] >= 'a' && str->data[i] <= 'z') {
                if (str->data[i] - 32 < str2->data[i]) {
                    result = -1;
                    break;
                } else if (str->data[i] - 32 > str2->data[i]) {
                    result = 1;
                    break;
                }
            } else if (str->data[i] >= 'A' && str->data[i] <= 'Z') {
                if (str->data[i] + 32 < str2->data[i]) {
                    result = -1;
                    break;
                } else if (str->data[i] + 32 > str2->data[i]) {
                    result = 1;
                    break;
                }
            } else {
                if (str->data[i] < str2->data[i]) {
                    result = -1;
                    break;
                } else if (str->data[i] > str2->data[i]) {
                    result = 1;
                    break;
                }
            }
        }
        if (result == 0) {
            if (str->len < str2->len) {
                result = -1;
            } else if (str->len > str2->len) {
                result = 1;
            }
        }
    }
    return result;
}

int xString_compareIgnoreCaseCString(xString *str, char *cstr)
{
    xString *str2 = xString_new();
    xString_appendCString(str2, cstr);
    int result = xString_compareIgnoreCase(str, str2);
    xString_free(str2);
    return result;
}

char *xString_toCString(xString *str)
{
    char *cstr = NULL;
    if (str) {
        int cstr_len = 0;
        while (cstr_len < str->len && str->data[cstr_len] != '\0') {
            cstr_len++;
        }
        if ((cstr = malloc(cstr_len + 1))) {
            int i;
            for (i = 0; i < cstr_len; i++) {
                cstr[i] = str->data[i];
            }
            cstr[cstr_len] = '\0';
        }
    }
    return cstr;
}

void xString_toLowercase(xString *str)
{
    if (str) {
        int i;
        for (i = 0; i < str->len; i++) {
            if (str->data[i] >= 'A' && str->data[i] <= 'Z') {
                str->data[i] += 32;
            }
        }
    }
}

void xString_toUppercase(xString *str)
{
    if (str) {
        int i;
        for (i = 0; i < str->len; i++) {
            if (str->data[i] >= 'a' && str->data[i] <= 'z') {
                str->data[i] -= 32;
            }
        }
    }
}

void xString_reverse(xString *str)
{
    if (str) {
        int i;
        for (i = 0; i < str->len / 2; i++) {
            unsigned char temp = str->data[i];
            str->data[i] = str->data[str->len - i - 1];
            str->data[str->len - i - 1] = temp;
        }
    }
}

void xString_trim(xString *str)
{
    xString_trimLeft(str);
    xString_trimRight(str);
}

void xString_trimLeft(xString *str)
{
    if (str) {
        int i;
        for (i = 0; i < str->len; i++) {
            if (str->data[i] != ' ' && str->data[i] != '\t' && str->data[i] != '\n' && str->data[i] != '\r') {
                break;
            }
        }
        xString_remove(str, 0, i);
    }
}

void xString_trimRight(xString *str)
{
    if (str) {
        int i;
        for (i = str->len - 1; i >= 0; i--) {
            if (str->data[i] != ' ' && str->data[i] != '\t' && str->data[i] != '\n' && str->data[i] != '\r') {
                break;
            }
        }
        xString_remove(str, i + 1, str->len);
    }
}

void xString_padLeft(xString *str, unsigned char c, int len)
{
    if (str && len > str->len) {
        int oldLen = str->len;
        int i;
        for (i = 0; i < len - oldLen; i++) {
            xString_insert(str, &c, 1, 0);
        }
    }
}

void xString_padRight(xString *str, unsigned char c, int len)
{
    if (str && len > str->len) {
        int oldLen = str->len;
        int i;
        for (i = 0; i < len - oldLen; i++) {
            xString_append(str, &c, 1);
        }
    }
}

void xString_padBoth(xString *str, unsigned char c, int len)
{
    if (str && len > str->len) {
        int oldLen = str->len;
        int i;
        for (i = 0; i < (len - oldLen) / 2; i++) {
            xString_insert(str, &c, 1, 0);
        }
        oldLen = str->len;
        for (i = 0; i < len - oldLen; i++) {
            xString_append(str, &c, 1);
        }
    }
}

void xString_insert(xString *str, unsigned char *data, int len, int index)
{
    if (str && index >= 0 && index <= str->len && data && len > 0) {
        int newLen = str->len + len;
        if (newLen > str->cap) {
            int newCap = str->cap * 2;
            if (newCap < newLen) {
                newCap = newLen;
            }
            unsigned char *newData = NULL;
            if ((newData = malloc(newCap))) {
                int i;
                for (i = 0; i < index; i++) {
                    newData[i] = str->data[i];
                }
                for (i = 0; i < len; i++) {
                    newData[index + i] = data[i];
                }
                for (i = index; i < str->len; i++) {
                    newData[i + len] = str->data[i];
                }
                free(str->data);
                str->data = newData;
                str->len = newLen;
                str->cap = newCap;
            }
        } else {
            int i;
            for (i = str->len - 1; i >= index; i--) {
                str->data[i + len] = str->data[i];
            }
            for (i = 0; i < len; i++) {
                str->data[index + i] = data[i];
            }
            str->len = newLen;
        }
    }
}

void xString_insertChar(xString *str, unsigned char c, int index) { xString_insert(str, &c, 1, index); }

void xString_insertString(xString *str, xString *str2, int index) { xString_insert(str, str2->data, str2->len, index); }

void xString_insertCString(xString *str, char *cstr, int index)
{
    xString_insert(str, (unsigned char *)cstr, xString_CStringLen(cstr), index);
}

xString **xString_split(const xString *str, unsigned char *data, int len)
{
    xString **str2 = NULL;
    if (str && data && len > 0 && str->len >= len) {
        int *indices = xString_findAll(str, data, len);
        if (!indices || indices[0] == -1) {
            if ((str2 = malloc(2 * sizeof(xString *)))) {
                str2[0] = xString_copy(str);
                str2[1] = NULL;
            }
        } else {
            int splitCtr = 0;
            while (indices[splitCtr] != -1) {
                splitCtr++;
            }
            str2 = malloc((splitCtr + 2) * sizeof(xString *));

            splitCtr = 0;
            int i = 0;
            while (i < str->len) {
                if (indices[splitCtr] == -1) {
                    str2[splitCtr] = xString_substring(str, i, str->len);
                    splitCtr++;
                    break;
                }
                if (i < indices[splitCtr]) {
                    str2[splitCtr] = xString_substring(str, i, indices[splitCtr]);
                } else {
                    str2[splitCtr] = xString_new();
                }
                i = indices[splitCtr] + len;
                splitCtr++;
            }
            str2[splitCtr] = NULL;
        }
    }
    return str2;
}

xString **xString_splitChar(const xString *str, unsigned char c) { return xString_split(str, &c, 1); }

xString **xString_splitString(const xString *str, xString *str2) { return xString_split(str, str2->data, str2->len); }

xString **xString_splitCString(const xString *str, char *cstr)
{
    return xString_split(str, (unsigned char *)cstr, xString_CStringLen(cstr));
}

int xString_toInt(xString *str)
{
    int result = 0;
    int sign = 1;
    if (str && str->len > 0) {
        int i = 0;
        if (str->data[i] == '-') {
            sign = -1;
            i++;
        }
        for (; i < str->len; i++) {
            if (str->data[i] >= '0' && str->data[i] <= '9') {
                result *= 10;
                result += str->data[i] - '0';
            } else {
                break;
            }
        }
    }
    return result * sign;
}

float xString_toFloat(xString *str)
{
    float result = 0;
    int sign = 1;
    if (str && str->len > 0) {
        int i = 0;
        if (str->data[i] == '-') {
            sign = -1;
            i++;
        }
        for (; i < str->len; i++) {
            if (str->data[i] >= '0' && str->data[i] <= '9') {
                result *= 10;
                result += str->data[i] - '0';
            } else if (str->data[i] == '.') {
                break;
            } else {
                return result * sign;
            }
        }
        if (i < str->len) {
            float multiplier = 0.1;
            for (i++; i < str->len; i++) {
                if (str->data[i] >= '0' && str->data[i] <= '9') {
                    result += (str->data[i] - '0') * multiplier;
                    multiplier *= 0.1;
                } else {
                    return result * sign;
                }
            }
        }
    }
    return result * sign;
}

double xString_toDouble(xString *str)
{
    double result = 0;
    int sign = 1;
    if (str && str->len > 0) {
        int i = 0;
        if (str->data[0] == '-') {
            sign = -1;
            i++;
        }
        for (; i < str->len; i++) {
            if (str->data[i] >= '0' && str->data[i] <= '9') {
                result *= 10;
                result += str->data[i] - '0';
            } else if (str->data[i] == '.') {
                break;
            } else {
                return result * sign;
            }
        }
        if (i < str->len) {
            double multiplier = 0.1;
            for (i++; i < str->len; i++) {
                if (str->data[i] >= '0' && str->data[i] <= '9') {
                    result += (str->data[i] - '0') * multiplier;
                    multiplier *= 0.1;
                } else {
                    return result * sign;
                }
            }
        }
    }
    return result * sign;
}

long xString_toLong(xString *str)
{
    long result = 0;
    int sign = 1;
    if (str && str->len > 0) {
        int i = 0;
        if (str->data[i] == '-') {
            sign = -1;
            i++;
        }
        for (; i < str->len; i++) {
            if (str->data[i] >= '0' && str->data[i] <= '9') {
                result *= 10;
                result += str->data[i] - '0';
            } else {
                break;
            }
        }
    }
    return result * sign;
}

xString *xString_fromInt(int i)
{
    xString *str = NULL;
    if ((str = xString_new())) {
        if (i == 0) {
            xString_appendChar(str, '0');
        } else {
            int negative = 0;
            if (i < 0) {
                negative = 1;
                i = -i;
            }
            while (i > 0) {
                xString_insertChar(str, '0' + i % 10, 0);
                i /= 10;
            }
            if (negative) {
                xString_insertChar(str, '-', 0);
            }
        }
    }
    return str;
}

xString *xString_fromFloat(float f)
{
    xString *str = NULL;
    if ((str = xString_new())) {
        // determining sign of number
        int negative = 0;
        if (f < 0) {
            negative = 1;
            f = -f;
        }

        // converting integer part
        int i = (int)f;
        do {
            xString_appendChar(str, (unsigned char)('0' + i % 10));
            i /= 10;
        } while (i > 0);

        // adding sign if negative
        if (negative) {
            xString_appendChar(str, (unsigned char)'-');
        }

        // reversing since all integer part has been read backwards and adding
        // decimal point
        xString_reverse(str);
        xString_appendChar(str, (unsigned char)'.');

        // trimming off integer part
        f = f - (int)f;

        // converting decimal part up to 7 digits
        for (int j = 0; j < 8; j++) {
            f *= 10;
            xString_appendChar(str, (unsigned char)('0' + (int)f));
            f -= (int)f;
            if (f == 0) {
                break;
            }
        }
    }

    return str;
}

xString *xString_fromDouble(double d)
{
    xString *str = NULL;
    if ((str = xString_new())) {
        // determining sign of number
        int negative = 0;
        if (d < 0) {
            negative = 1;
            d = -d;
        }

        // converting integer part
        int i = (int)d;
        do {
            xString_appendChar(str, (unsigned char)('0' + i % 10));
            i /= 10;
        } while (i > 0);

        // adding sign if negative
        if (negative) {
            xString_appendChar(str, (unsigned char)'-');
        }

        // reversing since all integer part has been read backwards and adding
        // decimal point
        xString_reverse(str);
        xString_appendChar(str, (unsigned char)'.');

        // trimming off integer part
        d = d - (int)d;

        // converting decimal part up to 15 digits
        for (int j = 0; j < 16; j++) {
            d *= 10;
            xString_appendChar(str, (unsigned char)('0' + (int)d));
            d -= (int)d;
            if (d == 0) {
                break;
            }
        }
    }

    return str;
}

xString *xString_fromLong(long l)
{
    xString *str = NULL;
    if ((str = xString_new())) {
        if (l == 0) {
            xString_appendChar(str, '0');
        } else {
            int negative = 0;
            if (l < 0) {
                negative = 1;
                l = -l;
            }
            while (l > 0) {
                xString_insertChar(str, '0' + l % 10, 0);
                l /= 10;
            }
            if (negative) {
                xString_insertChar(str, '-', 0);
            }
        }
    }
    return str;
}

xString *xString_fromCString(const char *cstr)
{
    xString *str = NULL;
    if ((str = xString_new())) {
        xString_appendCString(str, cstr);
    }
    return str;
}

int xString_isEqual(xString *str, unsigned char *data, int len)
{
    int result = 0;
    if (str && data && len > 0 && str->len == len) {
        int i;
        for (i = 0; i < len; i++) {
            if (str->data[i] != data[i]) {
                break;
            }
        }
        if (i == len) {
            result = 1;
        }
    }
    return result;
}

int xString_isEqualString(xString *str, xString *str2) { return xString_isEqual(str, str2->data, str2->len); }

int xString_isEqualCString(xString *str, char *cstr)
{
    return xString_isEqual(str, (unsigned char *)cstr, xString_CStringLen(cstr));
}

unsigned long long xString_hash(xString *str)
{
    unsigned long long hash = 0xcbf29ce484222325;  // FNV offset basis
    if (str && str->data && str->len > 0) {
        for (int i = 0; i < str->len; i++) {
            hash ^= (unsigned long long)str->data[i];
            hash *= 0x100000001b3;  // FNV prime
        }
    }
    return hash;
}
