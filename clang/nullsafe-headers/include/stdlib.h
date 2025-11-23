/*
 * stdlib.h - Null-safe standard library for C!
 * 
 * This header provides nullability-annotated declarations for C standard
 * library functions. Compatible with both cbang and regular clang.
 * 
 * Key annotations:
 * - Memory allocation functions return _Nullable (can fail)
 * - String conversion functions require _Nonnull input
 * - Search/sort functions require _Nonnull pointers
 */

#ifndef _CBANG_STDLIB_H
#define _CBANG_STDLIB_H

#include <stddef.h>  /* for size_t, NULL */

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Memory Allocation
 * All return _Nullable - can return NULL on allocation failure
 * ======================================================================== */

/* Allocate memory (returns NULL on failure) */
void* _Nullable malloc(size_t __size);

/* Allocate and zero-initialize memory (returns NULL on failure) */
void* _Nullable calloc(size_t __nmemb, size_t __size);

/* Reallocate memory (returns NULL on failure, original ptr unchanged) */
void* _Nullable realloc(void* _Nullable __ptr, size_t __size);

/* Free allocated memory (NULL is explicitly allowed) */
void free(void* _Nullable __ptr);


/* ========================================================================
 * String Conversion Functions
 * All require non-null input strings
 * ======================================================================== */

/* Convert string to integer */
int atoi(const char* _Nonnull __nptr);

/* Convert string to long */
long atol(const char* _Nonnull __nptr);

/* Convert string to long long */
long long atoll(const char* _Nonnull __nptr);

/* Convert string to double */
double atof(const char* _Nonnull __nptr);

/*
 * Convert string to long with error checking
 * nptr: string to convert (required non-null)
 * endptr: if non-null, stores pointer to first invalid character
 * base: numeric base (2-36, or 0 for auto-detect)
 */
long strtol(const char* _Nonnull __restrict __nptr,
            char* _Nullable * _Nullable __restrict __endptr,
            int __base);

unsigned long strtoul(const char* _Nonnull __restrict __nptr,
                      char* _Nullable * _Nullable __restrict __endptr,
                      int __base);

long long strtoll(const char* _Nonnull __restrict __nptr,
                  char* _Nullable * _Nullable __restrict __endptr,
                  int __base);

unsigned long long strtoull(const char* _Nonnull __restrict __nptr,
                            char* _Nullable * _Nullable __restrict __endptr,
                            int __base);

double strtod(const char* _Nonnull __restrict __nptr,
              char* _Nullable * _Nullable __restrict __endptr);

float strtof(const char* _Nonnull __restrict __nptr,
             char* _Nullable * _Nullable __restrict __endptr);


/* ========================================================================
 * Pseudo-random Number Generation
 * ======================================================================== */

int rand(void);
void srand(unsigned int __seed);


/* ========================================================================
 * Environment Functions
 * ======================================================================== */

/* Get environment variable (returns NULL if not found) */
char* _Nullable getenv(const char* _Nonnull __name);

/* Execute command (NULL means check if shell is available) */
int system(const char* _Nullable __command);


/* ========================================================================
 * Program Termination
 * ======================================================================== */

/* Normal termination */
_Noreturn void exit(int __status);

/* Abnormal termination */
_Noreturn void abort(void);

/* Register function to be called at exit (returns 0 on success) */
int atexit(void (* _Nonnull __func)(void));


/* ========================================================================
 * Searching and Sorting
 * All require non-null pointers
 * ======================================================================== */

/*
 * Binary search
 * Returns pointer to matching element, or NULL if not found
 */
void* _Nullable bsearch(const void* _Nonnull __key,
                        const void* _Nonnull __base,
                        size_t __nmemb,
                        size_t __size,
                        int (* _Nonnull __compar)(const void* _Nonnull,
                                                   const void* _Nonnull));

/*
 * Quick sort (in-place)
 * All pointers must be non-null
 */
void qsort(void* _Nonnull __base,
           size_t __nmemb,
           size_t __size,
           int (* _Nonnull __compar)(const void* _Nonnull,
                                      const void* _Nonnull));


/* ========================================================================
 * Integer Arithmetic
 * ======================================================================== */

int abs(int __x);
long labs(long __x);
long long llabs(long long __x);

typedef struct {
    int quot;
    int rem;
} div_t;

typedef struct {
    long quot;
    long rem;
} ldiv_t;

typedef struct {
    long long quot;
    long long rem;
} lldiv_t;

div_t div(int __numer, int __denom);
ldiv_t ldiv(long __numer, long __denom);
lldiv_t lldiv(long long __numer, long long __denom);


/* ========================================================================
 * Multibyte/Wide Character Conversions
 * ======================================================================== */

/* Get length of multibyte character (returns -1 on error) */
int mblen(const char* _Nullable __s, size_t __n);

/* Convert multibyte to wide character */
int mbtowc(wchar_t* _Nullable __restrict __pwc,
           const char* _Nullable __restrict __s,
           size_t __n);

/* Convert wide character to multibyte */
int wctomb(char* _Nullable __s, wchar_t __wc);

/* Convert multibyte string to wide character string */
size_t mbstowcs(wchar_t* _Nullable __restrict __pwcs,
                const char* _Nullable __restrict __s,
                size_t __n);

/* Convert wide character string to multibyte string */
size_t wcstombs(char* _Nullable __restrict __s,
                const wchar_t* _Nullable __restrict __pwcs,
                size_t __n);


/* ========================================================================
 * POSIX Extensions
 * ======================================================================== */

#if defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE)

/* Make temporary file name (returns NULL on failure) */
char* _Nullable mktemp(char* _Nonnull __template);

/* Create temporary file (returns -1 on failure) */
int mkstemp(char* _Nonnull __template);

/* Get real path (returns NULL on failure) */
char* _Nullable realpath(const char* _Nonnull __restrict __name,
                         char* _Nullable __restrict __resolved);

#endif

#ifdef __cplusplus
}
#endif

#endif /* _CBANG_STDLIB_H */
