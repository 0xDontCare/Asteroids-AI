/**
 * @file xStringIO.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief xString compatible I/O functions.
 * @version 0.1
 * @date 08.05.2024.
 *
 * @copyright Copyright (c) 2024
 *
 * This is an extension module for xString library which acts as a bridge between xString and standard C I/O functions. All
 * functions have prefix `xString_`. The module provides functions for reading and writing strings from/to streams, files, and
 * standard input/output.
 */

#ifndef XSTRINGIO_H
#define XSTRINGIO_H

#include <stdio.h>  // file and stream I/O

#include "xString.h"  // base xString module

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Read a string from a stream.
 *
 * @param stream The stream to read from.
 * @return `xString*`: Pointer to the new string, or `NULL` on error.
 *
 * @note
 * Reading ends at EOF.
 *
 * @warning
 * If the file is large, the function will try to allocate a large
 * amount of memory. Use \ref xString_readBuffered to limit the amount of memory
 * used.
 */
xString *xString_readStream(FILE *stream);

/**
 * @brief Write a string to a stream.
 *
 * @param str The string to write.
 * @param stream The stream to write to.
 * @return `int`: `0` on success, `-1` on error.
 *
 * @note
 * The string is written as is, without any additional characters.
 *
 * @note
 * If the string is empty, the function will succeed without writing
 * anything.
 *
 * @warning
 * If stream is not writable, the function will fail.
 *
 * @warning
 * If the stream is a file, it is expected to be opened for writing at
 * function call.
 *
 */
int xString_writeStream(const xString *str, FILE *stream);

/**
 * @brief Read a line from a stream.
 *
 * @param stream The stream to read from.
 * @return `xString*`: Pointer to the new string, or `NULL` on error.
 *
 * @note
 * Reading ends at the first newline character or EOF.
 *
 * @note
 * The returned string will not contain the newline character.
 */
xString *xString_readLine(FILE *stream);

/**
 * @brief Write a string to a stream followed by a newline character.
 *
 * @param str The string to write.
 * @param stream The stream to write to.
 * @return `int`: `0` on success, `-1` on error.
 *
 * @note
 * The string is written as is, followed by a newline character.
 *
 * @note
 * If the string is empty, the function will succeed and write only the
 * newline character.
 *
 * @warning
 * If stream is not writable, the function will fail.
 *
 * @warning
 * If the stream is a file, it is expected to be opened for writing at
 * function call.
 */
int xString_writeLine(const xString *str, FILE *stream);

/**
 * @brief Read string of a given length from a stream.
 *
 * @param stream The stream to read from.
 * @param numBytes The number of bytes to read.
 * @return `xString*`: Pointer to the new string, or `NULL` on error.
 *
 * @note
 * Reading ends at EOF or when the given number of bytes is read.
 *
 */
xString *xString_readBytes(FILE *stream, size_t numBytes);

/**
 * @brief Read a string from the standard input stream.
 *
 * @return `xString*`: Pointer to the new string, or `NULL` on error.
 *
 * @note This function is xString equivalent to standard C `gets` function.
 *
 * @warning This function has no limit on input length. Use with caution.
 */
xString *xString_readIn(void);

/**
 * @brief Read a string from the standard input stream with a length limit.
 *
 * @param maxLen The maximum length of the string to read.
 * @return `xString*`: Pointer to the new string, or `NULL` on error.
 *
 * @note This function is xString equivalent to standard C `getline` function.
 *
 * @note The function will read at most `maxLen` characters from the standard input.
 *
 * @note Reading will end at EOF, newline character, or when `maxLen` characters are read.
 */
xString *xString_readInSafe(size_t maxLen);

/**
 * @brief Write a string to the standard output stream.
 *
 * @param str The string to write.
 * @return `int`: `0` on success, `-1` on error.
 *
 * @note This function is xString equivalent to standard C `puts` function.
 *
 * @warning If the string is empty, the function will succeed without writing
 * anything.
 */
int xString_writeOut(const xString *str);

#ifdef __cplusplus
}
#endif

#endif  // XSTRINGIO_H
