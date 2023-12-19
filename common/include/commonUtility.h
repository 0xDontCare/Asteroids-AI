/**
 * @file commonUtility.h
 * @author 0xDontCare (you@domain.com)
 * @brief Common utility functions for all three programs (manager, game and network). All functions have prefix `cu_`.
 * @version 0.2
 * @date 04.12.2023.
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef COMMONUTILITY_H
#define COMMONUTILITY_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * @brief Check if given C string contains only alphanumeric characters.
 * 
 * @param string Pointer to null-terminated string. 
 * @return 1 if string is alphanumeric, 0 otherwise.
 * 
 * @note If string is NULL, function returns 1 as if it passed the test.
 */
int cu_CStringIsAlphanumeric(char *string);

/**
 * @brief Check if given C string contains only numeric characters.
 * 
 * @param string Pointer to null-terminated string. 
 * @return 1 if string is numeric, 0 otherwise.
 * 
 * @note If string is NULL, function returns 0 as if it failed the test.
 */
int cu_CStringIsNumeric(char *string);

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