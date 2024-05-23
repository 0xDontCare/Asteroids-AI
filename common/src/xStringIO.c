#include "xStringIO.h"
#include <stdio.h>    // standard I/O functions
#include <stdlib.h>   // standard library (for malloc, free, ...)
#include "xString.h"  // base xString library

xString *xString_readStream(FILE *stream)
{
    // check if FILE is readable
    if (stream == NULL || ferror(stream)) {
        return NULL;
    }

    // create a new string
    xString *str = xString_new();
    if (str == NULL) {
        return NULL;
    }

    // calculate read size (end - current position + 1)
    long currentPos = ftell(stream);
    if (currentPos == -1) {
        xString_free(str);
        return NULL;
    }
    if (fseek(stream, 0, SEEK_END) != 0) {
        xString_free(str);
        return NULL;
    }
    long endPos = ftell(stream);
    if (endPos == -1) {
        xString_free(str);
        return NULL;
    }
    if (fseek(stream, currentPos, SEEK_SET) != 0) {
        xString_free(str);
        return NULL;
    }
    size_t size = (size_t)(endPos - currentPos);
    if (size == 0) {
        return str;
    }

    // allocate a buffer
    unsigned char *buffer = (unsigned char *)malloc(size);
    if (buffer == NULL) {
        xString_free(str);
        return NULL;
    }

    // read the file to the buffer
    size_t bytesRead = fread(buffer, sizeof(unsigned char), size, stream);
    if (bytesRead != (unsigned long)size) {
        xString_free(str);
        free(buffer);
        return NULL;
    }

    // append the buffer to the string
    xString_append(str, buffer, bytesRead);

    return str;
}

int xString_writeStream(const xString *str, FILE *stream)
{
    // check if FILE is writable
    if (stream == NULL || ferror(stream)) {
        return -1;
    }

    // check if string is empty
    if (xString_isEmpty(str)) {
        return 0;
    }

    // write the string to the stream
    if (fwrite(str->data, sizeof(unsigned char), str->len, stream) != (unsigned long)str->len) {
        return -1;
    }

    return 0;
}

xString *xString_readLine(FILE *stream)
{
    // check if FILE is readable
    if (stream == NULL || ferror(stream)) {
        return NULL;
    }

    // create a new string
    xString *str = xString_new();
    if (str == NULL) {
        return NULL;
    }

    // read the file to the end of the line (universal newline support)
    int c;
    while ((c = fgetc(stream)) != EOF) {
        if (c == '\n') {
            break;
        }
        xString_appendChar(str, (unsigned char)c);
    }

    // optimize the string
    xString_optimize(str);

    return str;
}

int xString_writeLine(const xString *str, FILE *stream)
{
    // check if FILE is writable
    if (stream == NULL || ferror(stream)) {
        return -1;
    }

    // check if string is empty
    if (xString_isEmpty(str)) {
        return 0;
    }

    // write the string to the stream
    if (fwrite(str->data, sizeof(unsigned char), str->len, stream) != (unsigned long)str->len) {
        return -1;
    }

    // write a newline character
    if (fputc('\n', stream) == EOF) {
        return -1;
    }

    return 0;
}

xString *xString_readBytes(FILE *stream, size_t numBytes)
{
    // check if FILE is readable
    if (stream == NULL || ferror(stream)) {
        return NULL;
    }

    // create a new string
    xString *str = xString_new();
    if (str == NULL) {
        return NULL;
    }

    // allocate a buffer
    unsigned char *buffer = (unsigned char *)malloc(numBytes);
    if (buffer == NULL) {
        xString_free(str);
        return NULL;
    }

    // read `numBytes` bytes from the file
    size_t bytesRead = fread(buffer, sizeof(unsigned char), numBytes, stream);
    if (bytesRead != numBytes) {
        xString_free(str);
        free(buffer);
        return NULL;
    }

    // append the buffer to the string
    xString_append(str, buffer, bytesRead);

    // free the buffer
    free(buffer);

    // optimize the string
    xString_optimize(str);

    return str;
}

xString *xString_readIn(void)
{
    // create a new string
    xString *str = xString_new();
    if (str == NULL) {
        return NULL;
    }

    // read user input from stdin
    int c;
    while (c = getchar(), c != EOF && c != '\n') {
        xString_appendChar(str, (unsigned char)c);
    }

    // optimize the string
    xString_optimize(str);

    return str;
}

xString *xString_readInSafe(size_t maxLen)
{
    // create a new string
    xString *str = xString_new();
    if (str == NULL) {
        return NULL;
    }

    // read user input from stdin
    int c;
    size_t i = 0;
    while (i < maxLen && (c = getchar()) != EOF && c != '\n') {
        xString_appendChar(str, (unsigned char)c);
        i++;
    }

    // optimize the string
    xString_optimize(str);

    return str;
}

int xString_writeOut(const xString *str)
{
    // check if string is empty
    if (xString_isEmpty(str)) {
        return 0;
    }

    // write the string to the standard output
    if (fwrite(str->data, sizeof(unsigned char), str->len, stdout) != (unsigned long)str->len) {
        return -1;
    }

    return 0;
}
