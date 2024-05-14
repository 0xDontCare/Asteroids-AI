/**
 * @file xString.h
 * @author 0xDontCare (https://github.com/0xDontCare)
 * @brief Safe string library module for xcFramework (and effectively successor
 * to xStringWorks module).
 * @version 0.18
 * @date 08.05.2024.
 * @copyright All rights reserved (c) 2024
 *
 * Library implements safer string type and respective functions for C.
 * All functionality from xStringWorks module is remplemented here (with minor
 * differences in edge case handling) with addition of memory safe container for
 * string data instead of working with basic C strings. Some algorithms have
 * also been tweaked (or changed completly) so functions should also be more
 * optimized.
 */

#ifndef XSTRING_H
#define XSTRING_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 * xString object consisting of string data, its length and allocated memory
 * (maximum length).
 *
 * @remark
 * Unlike regular C strings, xString data is not null terminated. Instead its
 * length is given through "len" attribute.
 *
 * @warning
 * "len" and "cap" attributes should not be directly modified as all other
 * functions within xString depend on that information and change them as
 * needed. It is best to treat them as read-only unless worked with properly.
 *
 */
typedef struct xString_s {
    unsigned char *data;
    int len;
    int cap;
} xString;

/**
 * @brief
 * Allocates and initializes new xString object (in heap).
 *
 * @note
 * Initially string capacity is 0, which means the string data itself is not
 * allocated but should be allocated separately.
 *
 * @return
 * Pointer to newly created xString object if successfull, null pointer if
 * failed.
 *
 * @see \ref xString_append(), \ref xString_Insert(), \ref xString_optimize(),
 * \ref xString_free()
 *
 */
xString *xString_new(void);

/**
 * @brief
 * Frees xString object and its data from memory.
 *
 * @note
 * Dangling pointer left behind should be handled manually.
 *
 * @param str xString object to be freed from memory.
 *
 * @see \ref xString_new(), \ref xString_optimize()
 *
 */
void xString_free(xString *str);

/**
 * @brief
 * Optimizes xString object by removing unused memory.
 *
 * @param str xString object to be optimized.
 *
 * @note
 * If function fails (due to memory allocation fail), string will not be
 * optimized and will be left in larger block of memory.
 *
 * @see \ref xString_free(), \ref xString_append()
 *
 */
void xString_optimize(xString *str);

/**
 * @brief Preallocate string buffer to target size.
 *
 * @param str xString object to be preallocated.
 * @param size Target size of preallocated buffer.
 *
 * @note
 * If new buffer allocation fails, old buffer will be left untouched.
 *
 * @warning
 * Previous string data will be lost along with old buffer. Make copy if needed.
 */
void xString_preallocate(xString *str, int size);

/**
 * @brief
 * Append data block of some size to xString object.
 * Currently the main way of putting data into xString object.
 *
 * @param str Modified xString object.
 * @param data Appended data block location.
 * @param len Block size.
 *
 * @note
 * If string data is unallocated or not large enough, new memory block will be
 * automatically allocated.
 *
 * @note
 * Due to way memory is extended to new size (by doubling old capacity),
 * allocated memory block may be larger than actual string data length. In this
 * case, it would be suggested to optimize string if needed.
 *
 * @note
 * If appended data pointer is NULL or length value is invalid (0 or negative),
 * xString object will be left untouched.
 *
 * @see \ref xString_insert(), \ref xString_optimize()
 *
 */
void xString_append(xString *str, unsigned char *data, int len);

/**
 * @brief
 * Append character to xString object.
 *
 * @param str Modified xString object.
 * @param c Appended character.
 *
 * @note
 * This function is specialized case of xString_append() function.
 * All of its notes and warnings are applicable here too.
 *
 * @see \ref xString_append(), \ref xString_optimize()
 *
 */
void xString_appendChar(xString *str, unsigned char c);

/**
 * @brief
 * Append another xString object to xString object.
 *
 * @param str Modified xString object.
 * @param str2 Appended xString object.
 *
 * @note
 * This function is specialized case of xString_append() function.
 * All of its notes and warnings are applicable here too.
 *
 * @see \ref xString_append(), \ref xString_optimize()
 *
 */
void xString_appendString(xString *str, xString *str2);

/**
 * @brief
 * Append C string to xString object.
 *
 * @param str Modified xString object.
 * @param cstr Appended C string.
 *
 * @note
 * Null-terminator at the end of C string is ignored and is not appended to
 * xString data.
 *
 * @note
 * This function is specialized case of xString_append() function.
 * All of its notes and warnings are applicable here too.
 *
 * @see \ref xString_append(), \ref xString_optimize()
 *
 */
void xString_appendCString(xString *str, const char *cstr);

/**
 * @brief
 * Create deep copy of already existing xString object.
 *
 * @param str Original xString object.
 * @return Copy of given xString object if succeeded, various outputs if fails
 * (read note).
 *
 * @note
 * If function fails (because of inabillity to allocate needed memory), it will
 * return NULL if object is unable to be allocated, xString with NULL data if
 * string data could not be allocated or partial copy of string data if
 * allocated memory could not be extended. Partially copied strings can be
 * easily detected if original and copy have mismatching lengths while other two
 * cases can be detected by checking if not NULL.
 *
 * @remark
 * If original xString object is not optimized before copy, the copy may also
 * have unoptimized memory size for its content.
 *
 */
xString *xString_copy(const xString *str);

/**
 * @brief
 * Create substring of xString object.
 *
 * @param str Input xString object.
 * @param start Starting index.
 * @param end Ending index.
 * @return Pointer to xString object containing selected substring of input
 * xString object.
 *
 * @note
 * Selected interval includes characters on starting index and excludes one on
 * the ending index.
 *
 * @remark
 * Output xString object data may be unoptimized.
 *
 */
xString *xString_substring(const xString *str, int start, int end);

/**
 * @brief
 * Finds first match of data block in xString object.
 *
 * @param str Input xString object.
 * @param data Pointer to data block.
 * @param len Data block size.
 * @return Index of first occurence of data block in xString data, -1 if no
 * matches found or invalid arguments given.
 *
 * @note
 * If function fails to allocate memory for the LPS array used internally, it
 * will fail and return -1.
 *
 * @remark
 * Search is done by utilizing Knuth-Morris-Pratt algorithm resulting in O(m +
 * n) time complexity rather than less efficient brute force.
 *
 */
int xString_find(xString *str, unsigned char *data, int len);

/**
 * @brief
 * Find first occurence of matching character in xString object.
 *
 * @param str Input xString object.
 * @param c Searched character.
 * @return Index of first occurence of character in xString data, -1 if not
 * found.
 *
 * @remark
 * Unlike other pattern finding functions in this module, this function uses
 * O(n) pass through whole string until matching character is found.
 *
 */
int xString_findChar(xString *str, unsigned char c);

/**
 * @brief
 * Find first occurence of another xString object in xString object.
 *
 * @param str Input xString object.
 * @param str2 Searched xString object.
 * @return Index of first occurence of another xString object in xString object,
 * -1 if not found or invalid arguments given.
 *
 * @note
 * This function is specialized case of xString_find() function.
 * All of its notes and warnings are applicable here too.
 *
 */
int xString_findString(xString *str, xString *str2);

/**
 * @brief
 * Find first occurence of C string in xString object.
 *
 * @param str Input xString object.
 * @param cstr Input C string.
 * @return Index of first occurence of C string in xString object, -1 if not
 * found or invalid arguments given.
 *
 * @note
 * This function is specialized case of xString_find() function.
 * All of its notes and warnings are applicable here too.
 *
 */
int xString_findCString(xString *str, char *cstr);

/**
 * @brief
 * Find last occurence of matching data block in xString object.
 *
 * @param str Input xString object.
 * @param data Pointer to data block.
 * @param len Data block size.
 * @return Index of last occurence of data block in xString object, -1 if not
 * found or invalid arguments given.
 *
 * @note
 * If function fails to allocate memory for the LPS array used internally, it
 * will fail and return -1.
 *
 * @remark
 * Search is done by utilizing slightly modified Knuth-Morris-Pratt algorithm
 * resulting in O(m + n) time complexity rather than less efficient brute force.
 *
 */
int xString_findLast(xString *str, unsigned char *data, int len);

/**
 * @brief
 * Find last occurence of character in xString object.
 *
 * @param str Input xString object.
 * @param c Searched character.
 * @return Index of last occurence of character in xString object, -1 if not
 * found.
 *
 * @remark
 * Unlike other pattern finding functions in this module, this function uses
 * O(n) pass through whole string until matching character is found.
 *
 */
int xString_findLastChar(xString *str, unsigned char c);

/**
 * @brief
 * Find last occurence of another xString object in xString object.
 *
 * @param str Input xString object.
 * @param str2 Searched xString object.
 * @return Index of last occurence of another xString object in xString object,
 * -1 if not found or invalid arguments given.
 *
 * @note
 * This function is specialized case of xString_findLast() function.
 * All of its notes and warnings are applicable here too.
 *
 */
int xString_findLastString(xString *str, xString *str2);

/**
 * @brief
 * Find last occurence of C string in xString object.
 *
 * @param str Input xString object.
 * @param cstr Searched C string.
 * @return Index of last occurence of C string in xString object, -1 if not
 * found or invalid arguments given.
 *
 * @note
 * This function is specialized case of xString_findLast() function.
 * All of its notes and warnings are applicable here too.
 *
 */
int xString_findLastCString(xString *str, char *cstr);

/**
 * @brief
 * Find all occurences of data block in xString object.
 *
 * @param str Input xString object.
 * @param data Pointer to data block.
 * @param len Data block size.
 * @return Array of indices containing all occurences of data block in xString
 * object terminated with -1.
 *
 * @warning
 * If no match is found or invalid arguments are given, function will still
 * return array with single element (-1). Freeing the array is handled by user.
 *
 * @note
 * If memory allocation fails (whether for the internal LPS array or returning
 * indices array), function will return NULL instead.
 *
 * @remark
 * Function utilizes modified Knuth-Morris-Pratt algorithm for more time
 * efficient pattern searching than brute force search.
 *
 * @remark
 * This function only searches for non-overlapping matches. If overlapping
 * matches are preferred, check out \ref xString_findAll_overlapping() function
 * instead.
 *
 */
int *xString_findAll(const xString *str, unsigned char *data, int len);

/**
 * @brief
 * Find all occurences of character in xString object.
 *
 * @param str Input xString object.
 * @param c Input character.
 * @return Array of indices of all occurences of character in xString object
 * terminated with -1.
 *
 * @warning
 * If no matches are found, function will still allocate array with space for
 * terminating -1. Freeing the array is left to user for handling.
 *
 * @note
 * If memory allocation for indices array fails, function will return NULL.
 *
 * @remark
 * Function searches for matching character by passing through whole string
 * twice (first time is counting for allocation after which all matches are
 * added to array).
 *
 */
int *xString_findAllChar(xString *str, unsigned char c);

/**
 * @brief
 * Find all occurences of another xString object in xString object.
 *
 * @param str Input xString object.
 * @param str2 Searched xString object.
 * @return Array of indices of all occurences of another xString object in
 * xString object terminated with -1.
 *
 * @note
 * This function is specialized case of \ref xString_findAll() function.
 * All of its notes and warnings are applicable here too.
 *
 * @see \ref xString_findAll()
 *
 */
int *xString_findAllString(xString *str, xString *str2);

/**
 * @brief
 * Find all occurences of C string in xString object.
 *
 * @param str Input xString object.
 * @param cstr Searched C string.
 * @return Array of indices of all occurences of C string in xString object
 * terminated with -1.
 *
 * @note
 * This function is specialized case of \ref xString_findAll() function.
 * All of its notes and warnings are applicable here too.
 *
 * @see \ref xString_findAll()
 *
 */
int *xString_findAllCString(xString *str, char *cstr);

/**
 * @brief
 * Find all occurences of data block in xString object (with overlapping).
 *
 * @param str Input xString object.
 * @param data Pointer to data block.
 * @param len Data block size.
 * @return Array of indices of all occurences of data block in xString object
 * terminated with -1.
 *
 * @warning
 * If no match is found or invalid arguments are given, function will still
 * create and return array with single element (-1). Freeing the array is
 * handled by user.
 *
 * @note
 * If memory allocation fails (whether for the internal LPS array or returning
 * indices array), function will return NULL instead.
 *
 * @remark
 * Function utilizes modified Knuth-Morris-Pratt algorithm for more time
 * efficient pattern searching than brute force search.
 *
 * @remark
 * This function only searches for ALL matches, even overlapping ones. If
 * overlapping matches are unwanted, check out \ref xString_findAll() function
 * instead.
 *
 */
int *xString_findAll_overlapping(xString *str, unsigned char *data, int len);

/**
 * @brief
 * Find all occurences of another xString object in xString object
 * (overlapping).
 *
 * @param str Input xString object.
 * @param str2 Searched xString object.
 * @return Array of indices of all occurences of another xString object in
 * xString object terminated with -1.
 *
 * @note
 * This function is specialized case of \ref xString_findAll_overlapping()
 * function. All of its notes and warnings are applicable here too.
 *
 * @see \ref xString_findAll_overlapping()
 *
 */
int *xString_findAllString_overlapping(xString *str, xString *str2);

/**
 * @brief
 * Find all occurences of C string in xString object (overlapping).
 *
 * @param str Input xString object.
 * @param cstr Searched C string.
 * @return Array of indices of all occurences of C string in xString object
 * terminated with -1.
 *
 * @note
 * This function is specialized case of \ref xString_findAll_overlapping()
 * function. All of its notes and warnings are applicable here too.
 *
 * @see \ref xString_findAll_overlapping()
 *
 */
int *xString_findAllCString_overlapping(xString *str, char *cstr);

/**
 * @brief
 * Count occurences of data block in xString object.
 *
 * @param str Input xString object.
 * @param data Pointer to data block.
 * @param len Data block size.
 * @return Number of occurences of data block in xString object.
 *
 * @note
 * If invalid arguments are given, such as passing NULL as data pointer or
 * negative length, function will return 0 as if no matches are found.
 *
 * @note
 * If function fails to create LPS array for internal use, it will return 0
 * matches.
 *
 * @remark
 * Function makes use of Knuth-Morris-Pratt algorithm for more time efficient
 * searching rather than brute force searching.
 *
 * @remark
 * This function counts only non-overlapping matches of pattern in string. If
 * overlapping matches are preferred, check out \ref xString_count_overlapping()
 * function instead.
 *
 */
int xString_count(xString *str, unsigned char *data, int len);

/**
 * @brief
 * Count occurences of character in xString object.
 *
 * @param str Input xString object.
 * @param c Input character.
 * @return Number of occurences of character in xString object.
 *
 * @remark
 * Function counts character occurences by utilizing basic O(n) pass through
 * whole string.
 */
int xString_countChar(xString *str, unsigned char c);

/**
 * @brief
 * Count occurences of another xString object in xString object.
 *
 * @param str Input xString object.
 * @param str2 Searched xString object.
 * @return Number of occurences of another xString object in xString object.
 *
 * @note
 * This function is specialized case of \ref xString_count() function.
 * All of its notes and warnings are applicable here too.
 *
 * @see \ref xString_count()
 *
 */
int xString_countString(xString *str, xString *str2);

/**
 * @brief
 * Count occurences of C string in xString object.
 *
 * @param str Input xString object.
 * @param cstr Searched C string.
 * @return Number of occurences of C string in xString object.
 *
 * @note
 * This function is specialized case of \ref xString_count() function.
 * All of its notes and warnings are applicable here too.
 *
 * @see \ref xString_count()
 *
 */
int xString_countCString(xString *str, char *cstr);

/**
 * @brief
 * Count occurrences of data block in xString object (overlapping matches
 * included).
 *
 * @param str Input xString object.
 * @param data Pointer to data block.
 * @param len Data block size.
 * @return Number of occurrences of data block in xString object.
 *
 * @note
 * If invalid arguments are given, such as passing NULL as data pointer or
 * negative length, function will return 0 as if no matches are found.
 *
 * @note
 * If function fails to create LPS array for internal use, it will return 0
 * matches.
 *
 * @remark
 * Function makes use of Knuth-Morris-Pratt algorithm for more time efficient
 * searching rather than brute force searching.
 *
 * @remark
 * This function counts ALL matches of pattern in string (even overlapping
 * ones). If overlapping matches are unwanted, check out \ref xString_count()
 * function instead.
 *
 */
int xString_count_overlapping(xString *str, unsigned char *data, int len);

/**
 * @brief
 * Count occurrences of another xString object in xString object (overlapping
 * matches included).
 *
 * @param str Input xString object.
 * @param str2 Searched xString object.
 * @return int Number of occurrences of another xString object in xString
 * object.
 *
 * @note
 * This function is specialized case of \ref xString_count_overlapping()
 * function. All of its notes and warnings are applicable here too.
 *
 * @see \ref xString_count_overlapping()
 *
 */
int xString_countString_overlapping(xString *str, xString *str2);

/**
 * @brief
 * Count occurrences of C string in xString object (overlapping matches
 * included).
 *
 * @param str Input xString object.
 * @param cstr Searched C string.
 * @return int Number of occurrences of C string in xString object.
 *
 * @note
 * This function is specialized case of \ref xString_count_overlapping()
 * function. All of its notes and warnings are applicable here too.
 *
 * @see \ref xString_count_overlapping()
 *
 */
int xString_countCString_overlapping(xString *str, char *cstr);

/**
 * @brief Replace first occurrence data block in xString object.
 *
 * @param str Modified xString object.
 * @param data Pointer to data block.
 * @param len Data block size.
 * @param data2 Pointer to replacement data block.
 * @param len2 Replacement data block size.
 */
void xString_replaceFirst(xString *str, unsigned char *data, int len, unsigned char *data2, int len2);

/**
 * @brief Replace first occurrence of given character in xString object.
 *
 * @param str Modified xString object.
 * @param c Input character.
 * @param c2 Replacement character.
 */
void xString_replaceFirstChar(xString *str, unsigned char c, unsigned char c2);

/**
 * @brief Replace first occurrence of another xString object in xString object.
 *
 * @param str Modified xString object.
 * @param str2 Searched xString object.
 * @param str3 Replacement xString object.
 */
void xString_replaceFirstString(xString *str, xString *str2, xString *str3);

/**
 * @brief Replace first occurrence of C string in xString object.
 *
 * @param str Modified xString object.
 * @param cstr Searched C string.
 * @param cstr2 Replacement C string.
 */
void xString_replaceFirstCString(xString *str, char *cstr, char *cstr2);

/**
 * @brief Replace last occurrence of matched data block in xString object.
 *
 * @param str Modified xString object.
 * @param data Pointer to data block.
 * @param len Data block size.
 * @param data2 Pointer to replacement data block.
 * @param len2 Replacement data block size.
 */
void xString_replaceLast(xString *str, unsigned char *data, int len, unsigned char *data2, int len2);

/**
 * @brief Replace last occurrence of matched character in xString object.
 *
 * @param str Modified xString object.
 * @param c Input character.
 * @param c2 Replacement character.
 */
void xString_replaceLastChar(xString *str, unsigned char c, unsigned char c2);

/**
 * @brief Replace last occurrence of matched xString object in xString object.
 *
 * @param str Modified xString object.
 * @param str2 Searched xString object.
 * @param str3 Replacement xString object.
 */
void xString_replaceLastString(xString *str, xString *str2, xString *str3);

/**
 * @brief Replace last occurrence of matched C string in xString object.
 *
 * @param str Modified xString object.
 * @param cstr Searched C string.
 * @param cstr2 Replacement C string.
 */
void xString_replaceLastCString(xString *str, char *cstr, char *cstr2);

/**
 * @brief Replace all occurences of data block in xString object.
 *
 * @param str Modified xString object.
 * @param data Pointer to data block.
 * @param len Data block size.
 * @param data2 Pointer to replacement data block.
 * @param len2 Replacement data block size.
 */
void xString_replace(xString *str, unsigned char *data, int len, unsigned char *data2, int len2);

/**
 * @brief Replace all occurences of character in xString object.
 *
 * @param str Modified xString object.
 * @param c Input character.
 * @param c2 Replacement character.
 */
void xString_replaceChar(xString *str, unsigned char c, unsigned char c2);

/**
 * @brief Replace all occurences of another xString object in xString object.
 *
 * @param str Modified xString object.
 * @param str2 Searched xString object.
 * @param str3 Replacement xString object.
 */
void xString_replaceString(xString *str, xString *str2, xString *str3);

/**
 * @brief Replace all occurences of C string in xString object.
 *
 * @param str Modified xString object.
 * @param cstr Searched C string.
 * @param cstr2 Replacement C string.
 */
void xString_replaceCString(xString *str, char *cstr, char *cstr2);

/**
 * @brief Remove segment of data from xString object.
 *
 * @param str Modified xString object.
 * @param start Starting index.
 * @param end Ending index.
 */
void xString_remove(xString *str, int start, int end);

/**
 * @brief Remove first occurence of data block from xString object.
 *
 * @param str Modified xString object.
 * @param data Pointer to data block.
 * @param len Data block size.
 */
void xString_removeFirstBlock(xString *str, unsigned char *data, int len);

/**
 * @brief Remove last occurence of data block from xString object.
 *
 * @param str Modified xString object.
 * @param data Pointer to data block.
 * @param len Data block size.
 */
void xString_removeLastBlock(xString *str, unsigned char *data, int len);

/**
 * @brief Remove all occurences of data block from xString object.
 *
 * @param str Modified xString object.
 * @param data Pointer to data block.
 * @param len Data block size.
 */
void xString_removeAllBlock(xString *str, unsigned char *data, int len);

/**
 * @brief Remove first occurrence of character from xString object.
 *
 * @param str Modified xString object.
 * @param c Input character.
 */
void xString_removeFirstChar(xString *str, unsigned char c);

/**
 * @brief Remove last occurrence of character from xString object.
 *
 * @param str Modified xString object.
 * @param c Input character.
 */
void xString_removeLastChar(xString *str, unsigned char c);

/**
 * @brief Remove all occurences of character from xString object.
 *
 * @param str Modified xString object.
 * @param c Input character.
 */
void xString_removeAllChar(xString *str, unsigned char c);

/**
 * @brief Remove first occurrence of another xString object from xString object.
 *
 * @param str Modified xString object.
 * @param str2 Searched xString object.
 */
void xString_removeFirstString(xString *str, xString *str2);

/**
 * @brief Remove last occurrence of another xString object from xString object.
 *
 * @param str Modified xString object.
 * @param str2 Searched xString object.
 */
void xString_removeLastString(xString *str, xString *str2);

/**
 * @brief Remove all occurences of another xString object from xString object.
 *
 * @param str Modified xString object.
 * @param str2 Searched xString object.
 */
void xString_removeAllString(xString *str, xString *str2);

/**
 * @brief Remove first occurrence of C string from xString object.
 *
 * @param str Modified xString object.
 * @param cstr Searched C string.
 */
void xString_removeFirstCString(xString *str, char *cstr);

/**
 * @brief Remove last occurrence of C string from xString object.
 *
 * @param str Modified xString object.
 * @param cstr Searched C string.
 */
void xString_removeLastCString(xString *str, char *cstr);

/**
 * @brief Remove all occurences of C string from xString object.
 *
 * @param str Modified xString object.
 * @param cstr Searched C string.
 */
void xString_removeAllCString(xString *str, char *cstr);

/**
 * @brief Remove all whitespace characters from xString object.
 *
 * @param str Modified xString object.
 */
void xString_removeAllWhitespace(xString *str);

/**
 * @brief Remove all digits from xString object.
 *
 * @param str Modified xString object.
 */
void xString_removeAllDigits(xString *str);

/**
 * @brief Remove all letters from xString object.
 *
 * @param str Modified xString object.
 */
void xString_removeAllLetters(xString *str);

/**
 * @brief Remove all uppercase letters from xString object.
 *
 * @param str Modified xString object.
 */
void xString_removeAllUppercase(xString *str);

/**
 * @brief Remove all lowercase letters from xString object.
 *
 * @param str Modified xString object.
 */
void xString_removeAllLowercase(xString *str);

/**
 * @brief Remove all special characters and symbols from xString object.
 *
 * @param str Modified xString object.
 */
void xString_removeAllSpecial(xString *str);

/**
 * @brief Remove all newlines from xString object.
 *
 * @param str Modified xString object.
 */
void xString_removeAllNewlines(xString *str);

/**
 * @brief Check if xString object is empty.
 *
 * @param str Input xString object.
 * @return int 1 if xString object is empty, 0 otherwise.
 */
int xString_isEmpty(const xString *str);

/**
 * @brief Lexicographically compare two xString objects.
 *
 * @param str First xString object.
 * @param str2 Second xString object.
 * @return int 0 if xString objects are equal, 1 if first xString object is
 * greater, -1 if second xString object is greater.
 */
int xString_compare(const xString *str, const xString *str2);

/**
 * @brief Lexicographically compare xString object and C string.
 *
 * @param str xString object.
 * @param cstr C string.
 * @return int 0 if xString object and C string are equal, 1 if xString object
 * is greater, -1 if C string is greater.
 */
int xString_compareCString(const xString *str, const char *cstr);

/**
 * @brief Lexicographically compare two xString objects (case insensitive).
 *
 * @param str First xString object.
 * @param str2 Second xString object.
 * @return int 0 if xString objects are equal, 1 if first xString object is
 * greater, -1 if second xString object is greater.
 */
int xString_compareIgnoreCase(const xString *str, const xString *str2);

/**
 * @brief Lexicographically compare xString object and C string (case
 * insensitive).
 *
 * @param str xString object.
 * @param cstr C string.
 * @return int 0 if xString object and C string are equal, 1 if xString object
 * is greater, -1 if C string is greater.
 */
int xString_compareIgnoreCaseCString(const xString *str, const char *cstr);

/**
 * @brief Convert xString object to C string.
 *
 * @param str Input xString object.
 * @return char* Pointer to C string.
 */
char *xString_toCString(const xString *str);

/**
 * @brief Convert xString object content to lowercase.
 *
 * @param str Modified xString object.
 */
void xString_toLowercase(xString *str);

/**
 * @brief Convert xString object content to uppercase.
 *
 * @param str Modified xString object.
 */
void xString_toUppercase(xString *str);

/**
 * @brief Reverse xString object content.
 *
 * @param str Modified xString object.
 */
void xString_reverse(xString *str);

/**
 * @brief Trim whitespace characters from xString object.
 *
 * @param str Modified xString object.
 */
void xString_trim(xString *str);

/**
 * @brief Trim whitespace characters from beginning of xString object.
 *
 * @param str Modified xString object.
 */
void xString_trimLeft(xString *str);

/**
 * @brief Trim whitespace characters from end of xString object.
 *
 * @param str Modified xString object.
 */
void xString_trimRight(xString *str);

/**
 * @brief Pad xString object with characters from left side.
 *
 * @param str Modified xString object.
 * @param c Padding character.
 * @param len Padding length.
 */
void xString_padLeft(xString *str, unsigned char c, int len);

/**
 * @brief Pad xString object with characters from right side.
 *
 * @param str Modified xString object.
 * @param c Padding character.
 * @param len Padding length.
 */
void xString_padRight(xString *str, unsigned char c, int len);

/**
 * @brief Pad xString object with characters from both sides.
 *
 * @param str Modified xString object.
 * @param c Padding character.
 * @param len Padding length.
 */
void xString_padBoth(xString *str, unsigned char c, int len);

/**
 * @brief Insert data block into xString object at specified index.
 *
 * @param str Modified xString object.
 * @param data Pointer to data block.
 * @param len Data block size.
 * @param index Insertion index.
 */
void xString_insert(xString *str, unsigned char *data, int len, int index);

/**
 * @brief Insert character into xString object at specified index.
 *
 * @param str Modified xString object.
 * @param c Inserted character.
 * @param index Insertion index.
 */
void xString_insertChar(xString *str, unsigned char c, int index);

/**
 * @brief Insert another xString object into xString object at specified index.
 *
 * @param str Modified xString object.
 * @param str2 Inserted xString object.
 * @param index Insertion index.
 */
void xString_insertString(xString *str, xString *str2, int index);

/**
 * @brief Insert C string into xString object at specified index.
 *
 * @param str Modified xString object.
 * @param cstr Inserted C string.
 * @param index Insertion index.
 */
void xString_insertCString(xString *str, char *cstr, int index);

/**
 * @brief Split xString object into array of xString objects by data block.
 *
 * @param str Input xString object.
 * @param data Pointer to data block.
 * @param len Data block size.
 * @return xString* Array of xString objects.
 */
xString **xString_split(const xString *str, unsigned char *data, int len);

/**
 * @brief Split xString object into array of xString objects by character.
 *
 * @param str Input xString object.
 * @param c Input character.
 * @return xString* Array of xString objects.
 */
xString **xString_splitChar(const xString *str, unsigned char c);

/**
 * @brief Split xString object into array of xString objects by another xString
 * object.
 *
 * @param str Input xString object.
 * @param str2 Input xString object.
 * @return xString* Array of xString objects.
 */
xString **xString_splitString(const xString *str, xString *str2);

/**
 * @brief Split xString object into array of xString objects by C string.
 *
 * @param str Input xString object.
 * @param cstr Input C string.
 * @return xString* Array of xString objects.
 */
xString **xString_splitCString(const xString *str, char *cstr);

/**
 * @brief Convert xString object to integer.
 *
 * @param str Input xString object.
 * @return int Integer value.
 */
int xString_toInt(xString *str);

/**
 * @brief Convert xString object to float.
 *
 * @param str Input xString object.
 * @return Float value.
 */
float xString_toFloat(xString *str);

/**
 * @brief Convert xString object to double.
 *
 * @param str Input xString object.
 * @return Double value.
 */
double xString_toDouble(xString *str);

/**
 * @brief Convert xString object to long.
 *
 * @param str Input xString object.
 * @return Long value.
 */
long xString_toLong(xString *str);

/**
 * @brief Convert integer to xString object.
 *
 * @param num Input integer.
 * @return Pointer to xString object, NULL if failed.
 */
xString *xString_fromInt(int num);

/**
 * @brief Convert float to xString object.
 *
 * @param num Input float.
 * @return Pointer to created xString object, NULL if failed.
 */
xString *xString_fromFloat(float num);

/**
 * @brief Convert double to xString object.
 *
 * @param num Input double.
 * @return Pointer to xString object, NULL if failed.
 */
xString *xString_fromDouble(double num);

/**
 * @brief Convert long to xString object.
 *
 * @param num Input long.
 * @return Pointer to xString object, NULL if failed.
 */
xString *xString_fromLong(long num);

/**
 * @brief Convert C string to xString object.
 *
 * @param cstr Pointer to C string.
 * @return Pointer to xString object, NULL if failed.
 *
 * @note
 * Null-terminator at the end of C string is ignored and is not appended to
 * xString data.
 */
xString *xString_fromCString(const char *cstr);

/**
 * @brief Check if xString object and data block are equal.
 *
 * @param str Pointer to xString object.
 * @param data Pointer to data block.
 * @param len Data block size.
 * @return 1 if string and data block are equal, 0 otherwise.
 */
int xString_isEqual(xString *str, unsigned char *data, int len);

/**
 * @brief Check if two xString objects are equal.
 *
 * @param str Pointer to xString object.
 * @param str2 Pointer to another xString object.
 * @return 1 if strings are equal, 0 otherwise.
 */
int xString_isEqualString(xString *str, xString *str2);

/**
 * @brief Check if xString object and C string are equal.
 *
 * @param str Pointer to xString object.
 * @param cstr Pointer to C string.
 * @return 1 if strings are equal, 0 otherwise.
 */
int xString_isEqualCString(xString *str, char *cstr);

/**
 * @brief Calculate hash value of xString object.
 *
 * @param str Pointer to xString object.
 * @return Hash value.
 *
 * @note Function uses FNV-1a algorithm for calculating hash value of string
 * data.
 */
unsigned long long xString_hash(xString *str);

#ifdef __cplusplus
}
#endif
#endif  // XSTRING_H
