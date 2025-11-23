/*
 * test_annotations.c - Test suite for null-safe headers
 *
 * This file intentionally contains null-safety errors that should be
 * caught by cbang but will compile with regular clang.
 *
 * Compile with regular clang (should succeed):
 *   clang -I../include test_annotations.c -o test_annotations
 *
 * Compile with cbang (should report errors):
 *   cbang -I../include -fsyntax-only test_annotations.c
 */

#include "string.h"
#include "stdlib.h"
#include "stdio.h"

/* Test 1: malloc returns nullable */
void test_malloc_nullable(void) {
    char* ptr = malloc(100);
    /* Should error: dereferencing nullable pointer */
    *ptr = 'x';  /* ERROR expected */
}

void test_malloc_safe(void) {
    char* ptr = malloc(100);
    if (ptr) {
        *ptr = 'x';  /* OK */
        free(ptr);
    }
}

/* Test 2: strlen requires nonnull */
void test_strlen_nonnull(char* str) {
    /* Should error: passing nullable to nonnull */
    size_t len = strlen(str);  /* ERROR expected */
}

void test_strlen_safe(char* str) {
    if (str) {
        size_t len = strlen(str);  /* OK */
    }
}

/* Test 3: strchr returns nullable */
void test_strchr_nullable(void) {
    const char* str = "hello";
    char* found = strchr(str, 'l');
    /* Should error: dereferencing nullable */
    *found = 'X';  /* ERROR expected */
}

void test_strchr_safe(void) {
    const char* str = "hello";
    char* found = strchr(str, 'l');
    if (found) {
        *found = 'X';  /* OK */
    }
}

/* Test 4: fopen returns nullable */
void test_fopen_nullable(void) {
    FILE* fp = fopen("test.txt", "r");
    /* Should error: passing nullable to nonnull */
    char buf[100];
    fgets(buf, sizeof(buf), fp);  /* ERROR expected */
    fclose(fp);  /* ERROR expected */
}

void test_fopen_safe(void) {
    FILE* fp = fopen("test.txt", "r");
    if (fp) {
        char buf[100];
        fgets(buf, sizeof(buf), fp);  /* OK */
        fclose(fp);  /* OK */
    }
}

/* Test 5: strcpy requires both nonnull */
void test_strcpy_nonnull(char* dest, char* src) {
    /* Should error: both might be null */
    strcpy(dest, src);  /* ERROR expected */
}

void test_strcpy_safe(char* dest, char* src) {
    if (dest && src) {
        strcpy(dest, src);  /* OK */
    }
}

/* Test 6: free accepts nullable */
void test_free_nullable(void) {
    char* ptr = malloc(100);
    free(ptr);  /* OK - free accepts nullable */

    ptr = NULL;
    free(ptr);  /* OK - NULL is explicitly allowed */
}

/* Test 7: getenv returns nullable */
void test_getenv_nullable(void) {
    char* path = getenv("PATH");
    /* Should error: passing nullable to nonnull */
    size_t len = strlen(path);  /* ERROR expected */
}

void test_getenv_safe(void) {
    char* path = getenv("PATH");
    if (path) {
        size_t len = strlen(path);  /* OK */
    }
}

/* Test 8: memcpy requires all nonnull */
void test_memcpy_nonnull(void* dest, void* src) {
    /* Should error: both might be null */
    memcpy(dest, src, 10);  /* ERROR expected */
}

void test_memcpy_safe(void* dest, void* src) {
    if (dest && src) {
        memcpy(dest, src, 10);  /* OK */
    }
}

/* Test 9: printf format must be nonnull */
void test_printf_nonnull(char* fmt) {
    /* Should error: format might be null */
    printf(fmt);  /* ERROR expected */
}

void test_printf_safe(char* fmt) {
    if (fmt) {
        printf(fmt);  /* OK */
    }
    /* String literals are always nonnull */
    printf("Hello\n");  /* OK */
}

/* Test 10: Early return narrowing */
void test_early_return(char* str) {
    if (!str) return;

    /* After early return, str is nonnull */
    size_t len = strlen(str);  /* OK */
}

/* Main just calls safe versions to ensure the file compiles */
int main(void) {
    char buffer[100] = "test";

    printf("Running null-safe header tests...\n");

    test_malloc_safe();
    test_strlen_safe(buffer);
    test_strchr_safe();
    test_fopen_safe();
    test_strcpy_safe(buffer, "hello");
    test_free_nullable();
    test_getenv_safe();

    char src[10] = "src";
    char dst[10];
    test_memcpy_safe(dst, src);

    test_printf_safe("test");
    test_early_return(buffer);

    printf("All safe tests passed!\n");
    printf("\nNote: Run with cbang to see errors in unsafe test functions.\n");

    return 0;
}
