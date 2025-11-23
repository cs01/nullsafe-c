/*
 * demo.c - Demonstration of null-safe C standard library headers
 *
 * Compile with:
 *   clang -I../include demo.c -o demo
 *   cbang -I../include demo.c -o demo  (for full null checking)
 */

#include "string.h"
#include "stdlib.h"
#include "stdio.h"

/* Example 1: Safe memory allocation */
void example_malloc(void) {
    size_t size = 100;
    char* buffer = malloc(size);  // Returns _Nullable

    /* UNSAFE: buffer might be NULL
    buffer[0] = 'x';  // cbang error: dereferencing nullable pointer
    */

    /* SAFE: Check before use */
    if (buffer) {
        buffer[0] = 'x';
        printf("Allocated buffer at %p\n", (void*)buffer);
        free(buffer);  /* free() accepts _Nullable */
    } else {
        fprintf(stderr, "Allocation failed\n");
    }
}

/* Example 2: String operations */
void example_strings(const char* source) {
    /* UNSAFE: source might be NULL
    size_t len = strlen(source);  // cbang error: passing nullable to nonnull
    */

    /* SAFE: Check first */
    if (source) {
        size_t len = strlen(source);  /* OK: source narrowed to _Nonnull */

        char* dest = malloc(len + 1);
        if (dest) {
            strcpy(dest, source);  /* OK: both _Nonnull in this branch */
            printf("Copied: %s\n", dest);
            free(dest);
        }
    }
}

/* Example 3: String searching */
void example_search(void) {
    const char* text = "Hello, World!";
    char* found = strchr(text, ',');  /* Returns _Nullable */

    /* UNSAFE: found might be NULL
    *found = ';';  // cbang error: dereferencing nullable pointer
    */

    /* SAFE: Check return value */
    if (found) {
        printf("Found comma at position %ld\n", found - text);
    } else {
        printf("Comma not found\n");
    }
}

/* Example 4: File I/O */
void example_file_io(const char* filename) {
    if (!filename) {
        fprintf(stderr, "Filename is required\n");
        return;
    }

    FILE* fp = fopen(filename, "r");  /* Returns _Nullable */

    /* UNSAFE: fp might be NULL
    char buf[100];
    fgets(buf, sizeof(buf), fp);  // cbang error: passing nullable to nonnull
    */

    /* SAFE: Check before use */
    if (fp) {
        char buf[100];
        while (fgets(buf, sizeof(buf), fp)) {  /* fgets returns _Nullable */
            printf("%s", buf);
        }
        fclose(fp);
    } else {
        fprintf(stderr, "Could not open %s\n", filename);
    }
}

/* Example 5: Environment variables */
void example_getenv(void) {
    const char* path = getenv("PATH");  /* Returns _Nullable */

    /* UNSAFE: path might be NULL
    printf("PATH is %zu characters\n", strlen(path));  // cbang error
    */

    /* SAFE: Check first */
    if (path) {
        printf("PATH is %zu characters long\n", strlen(path));
    } else {
        printf("PATH is not set\n");
    }
}

/* Example 6: Early return pattern */
void example_early_return(const char* input) {
    if (!input) {
        fprintf(stderr, "Input cannot be NULL\n");
        return;
    }

    /* After early return, input is narrowed to _Nonnull */
    size_t len = strlen(input);  /* OK - cbang knows input is non-null here */
    printf("Input length: %zu\n", len);
}

/* Example 7: Combining checks */
void example_combined(char* dest, const char* src, size_t max_len) {
    /* Check all parameters in one condition */
    if (dest && src && max_len > 0) {
        /* All narrowed to safe values in this branch */
        size_t src_len = strlen(src);
        if (src_len < max_len) {
            strcpy(dest, src);
        } else {
            strncpy(dest, src, max_len - 1);
            dest[max_len - 1] = '\0';
        }
    }
}

int main(int argc, char** argv) {
    printf("=== Null-Safe C Standard Library Demo ===\n\n");

    printf("Example 1: Memory allocation\n");
    example_malloc();
    printf("\n");

    printf("Example 2: String operations\n");
    example_strings("Hello, World!");
    example_strings(NULL);  /* Safe - function checks */
    printf("\n");

    printf("Example 3: String searching\n");
    example_search();
    printf("\n");

    printf("Example 4: File I/O\n");
    if (argc > 1) {
        example_file_io(argv[1]);
    } else {
        printf("(Skipping file I/O - no filename provided)\n");
    }
    printf("\n");

    printf("Example 5: Environment variables\n");
    example_getenv();
    printf("\n");

    printf("Example 6: Early return pattern\n");
    example_early_return("test");
    example_early_return(NULL);
    printf("\n");

    printf("Example 7: Combined checks\n");
    char buffer[20];
    example_combined(buffer, "short", sizeof(buffer));
    printf("Buffer: %s\n", buffer);
    printf("\n");

    printf("All examples completed successfully!\n");
    return 0;
}
