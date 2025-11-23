/*
 * string.h - Null-safe string operations for C!
 * 
 * This header provides nullability-annotated declarations for the C standard
 * library string functions. It is compatible with both cbang and regular clang.
 * 
 * Usage with cbang:
 *   cbang -I/path/to/cbang/headers mycode.c
 * 
 * Usage with regular clang:
 *   clang -I/path/to/cbang/headers mycode.c  (annotations present but not enforced)
 * 
 * Key differences from standard string.h:
 * - Functions requiring non-null pointers are annotated with _Nonnull
 * - Functions that can return NULL are annotated with _Nullable  
 * - Default (unannotated) pointers are treated as _Nullable by cbang
 */

#ifndef _CBANG_STRING_H
#define _CBANG_STRING_H

#include <stddef.h>  /* for size_t */

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * String Examination Functions
 * All require non-null input strings
 * ======================================================================== */

/* Get length of string (must be non-null and null-terminated) */
size_t strlen(const char* _Nonnull __s);

/* Compare two strings (both must be non-null) */
int strcmp(const char* _Nonnull __s1, const char* _Nonnull __s2);

/* Compare at most n characters (both must be non-null) */
int strncmp(const char* _Nonnull __s1, const char* _Nonnull __s2, size_t __n);

/* Compare strings using locale (both must be non-null) */
int strcoll(const char* _Nonnull __s1, const char* _Nonnull __s2);


/* ========================================================================
 * String Copying Functions
 * All require non-null pointers and return the destination pointer
 * ======================================================================== */

/* Copy string (both must be non-null, dest must have space) */
char* _Nonnull strcpy(char* _Nonnull __restrict __dest, 
                      const char* _Nonnull __restrict __src);

/* Copy at most n characters */
char* _Nonnull strncpy(char* _Nonnull __restrict __dest,
                       const char* _Nonnull __restrict __src,
                       size_t __n);

/* Concatenate strings */
char* _Nonnull strcat(char* _Nonnull __restrict __dest,
                      const char* _Nonnull __restrict __src);

/* Concatenate at most n characters */
char* _Nonnull strncat(char* _Nonnull __restrict __dest,
                       const char* _Nonnull __restrict __src,
                       size_t __n);

/* Transform string for strcoll comparison */
size_t strxfrm(char* _Nonnull __restrict __dest,
               const char* _Nonnull __restrict __src,
               size_t __n);


/* ========================================================================
 * String Searching Functions
 * Input strings must be non-null, but return value is _Nullable
 * (returns NULL if character/substring not found)
 * ======================================================================== */

/* Find first occurrence of character (returns NULL if not found) */
char* _Nullable strchr(const char* _Nonnull __s, int __c);

/* Find last occurrence of character (returns NULL if not found) */
char* _Nullable strrchr(const char* _Nonnull __s, int __c);

/* Find first occurrence of substring (returns NULL if not found) */
char* _Nullable strstr(const char* _Nonnull __haystack,
                       const char* _Nonnull __needle);

/* Find first occurrence of any character from accept string */
char* _Nullable strpbrk(const char* _Nonnull __s,
                        const char* _Nonnull __accept);

/* Get length of prefix matching only characters in accept */
size_t strspn(const char* _Nonnull __s, const char* _Nonnull __accept);

/* Get length of prefix not matching any characters in reject */
size_t strcspn(const char* _Nonnull __s, const char* _Nonnull __reject);


/* ========================================================================
 * String Tokenization
 * Special case: first parameter can be NULL to continue previous tokenization
 * ======================================================================== */

/* 
 * Extract tokens from string.
 * First call: pass string to tokenize
 * Subsequent calls: pass NULL to continue with same string
 * Returns NULL when no more tokens
 */
char* _Nullable strtok(char* _Nullable __s, const char* _Nonnull __delim);


/* ========================================================================
 * Memory Operations
 * All require non-null pointers (undefined behavior if NULL)
 * Functions that return pointers return their first argument (non-null)
 * ======================================================================== */

/* Copy n bytes (pointers must not overlap, both must be non-null) */
void* _Nonnull memcpy(void* _Nonnull __restrict __dest,
                      const void* _Nonnull __restrict __src,
                      size_t __n);

/* Copy n bytes (handles overlapping regions, both must be non-null) */
void* _Nonnull memmove(void* _Nonnull __dest,
                       const void* _Nonnull __src,
                       size_t __n);

/* Set n bytes to value */
void* _Nonnull memset(void* _Nonnull __s, int __c, size_t __n);

/* Compare n bytes */
int memcmp(const void* _Nonnull __s1, const void* _Nonnull __s2, size_t __n);

/* Find first occurrence of byte in memory (returns NULL if not found) */
void* _Nullable memchr(const void* _Nonnull __s, int __c, size_t __n);


/* ========================================================================
 * Error String Functions
 * ======================================================================== */

/* Get description of error number (returns non-null, points to static storage) */
char* _Nonnull strerror(int __errnum);


/* ========================================================================
 * String Duplication (POSIX)
 * Returns NULL if allocation fails
 * ======================================================================== */

#if defined(_POSIX_C_SOURCE) || defined(_GNU_SOURCE)
/* Duplicate string (allocates memory, returns NULL on allocation failure) */
char* _Nullable strdup(const char* _Nonnull __s);

/* Duplicate at most n characters (returns NULL on allocation failure) */
char* _Nullable strndup(const char* _Nonnull __s, size_t __n);
#endif


/* ========================================================================
 * BSD/GNU Extensions (commonly available)
 * ======================================================================== */

#ifdef _GNU_SOURCE
/* Find length of string, at most maxlen */
size_t strnlen(const char* _Nonnull __s, size_t __maxlen);

/* Case-insensitive string comparison */
int strcasecmp(const char* _Nonnull __s1, const char* _Nonnull __s2);

/* Case-insensitive string comparison, at most n characters */
int strncasecmp(const char* _Nonnull __s1, const char* _Nonnull __s2, size_t __n);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _CBANG_STRING_H */
