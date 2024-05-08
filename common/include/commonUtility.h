/**
 * @file commonUtility.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Common utility functions for all three programs (manager, game and network). All functions have prefix `cu_`.
 * @version 0.3
 * @date 08.05.2024.
 *
 * @copyright All rights reserved (c) 2024
 *
 */

#ifndef COMMONUTILITY_H
#define COMMONUTILITY_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * @brief Lexicographically compare two C strings.
 *
 * @param str1 pointer to first null-terminated string.
 * @param str2 pointer to second null-terminated string.
 * @return 0 if strings are equal, negative value if str1 is less than str2, positive value if str1 is greater than str2.
 */
int cu_CStringCompare(const char *str1, const char *str2);

/**
 * @brief Check if given C string ends with given suffix.
 *
 * @param string Pointer to null-terminated string containing the string to check.
 * @param suffix Pointer to null-terminated string containing the suffix to search.
 * @return 1 if string ends with suffix, 0 otherwise.
 *
 * @note If string or suffix is NULL, function returns 0 as if it failed the test.
 */
int cu_CStringEndsWith(const char *string, const char *suffix);

/**
 * @brief Check if given C string contains only alphanumeric characters.
 *
 * @param string Pointer to null-terminated string.
 * @return 1 if string is alphanumeric, 0 otherwise.
 *
 * @note If string is NULL, function returns 1 as if it passed the test.
 */
int cu_CStringIsAlphanumeric(const char *string);

/**
 * @brief Check if given C string contains only numeric characters.
 *
 * @param string Pointer to null-terminated string.
 * @return 1 if string is numeric, 0 otherwise.
 *
 * @note If string is NULL, function returns 0 as if it failed the test.
 */
int cu_CStringIsNumeric(const char *string);

/**
 * @brief Trim newline character from the end of the string.
 *
 * @param string Pointer to null-terminated string.
 */
void cu_CStringTrimNewline(char *string);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // COMMONUTILITY_H
