# Null-Safe C Standard Library Headers

Nullability-annotated C standard library headers using Clang's `_Nonnull` and `_Nullable` attributes.

## Quick Start

```c
#include "string.h"
#include "stdlib.h"

void example(const char* input) {
    if (!input) return;

    char* copy = malloc(strlen(input) + 1);
    if (copy) {
        strcpy(copy, input);
        free(copy);
    }
}
```

Compile:
```bash
clang -I/path/to/nullsafe-headers/include mycode.c
```

## What's Included

- `string.h` - String manipulation (strlen, strcpy, memcpy, etc.)
- `stdlib.h` - Memory allocation, conversions (malloc, atoi, etc.)
- `stdio.h` - File I/O (fopen, printf, fgets, etc.)

## Examples

### Memory Allocation

```c
void process_data(size_t size) {
    char* buffer = malloc(size);  // Returns _Nullable

    // ERROR with cbang - buffer might be NULL
    buffer[0] = 'x';

    // CORRECT - check first
    if (buffer) {
        buffer[0] = 'x';
        free(buffer);  // free() accepts _Nullable
    }
}
```

### String Operations

```c
void safe_copy(char* dest, const char* src) {
    // ERROR with cbang - both might be NULL, but strcpy requires _Nonnull
    strcpy(dest, src);

    // CORRECT
    if (dest && src) {
        strcpy(dest, src);
    }
}
```

### File I/O

```c
void read_config(const char* filename) {
    FILE* fp = fopen(filename, "r");  // Returns _Nullable

    // ERROR with cbang - fp might be NULL
    char buf[100];
    fgets(buf, sizeof(buf), fp);

    // CORRECT
    if (fp) {
        while (fgets(buf, sizeof(buf), fp)) {
            // Process line
        }
        fclose(fp);
    }
}
```

### Search Functions

```c
void find_and_replace(char* str, char old, char new) {
    char* pos = strchr(str, old);  // Returns _Nullable

    // ERROR with cbang - pos might be NULL
    *pos = new;

    // CORRECT
    while (pos) {
        *pos = new;
        pos = strchr(pos + 1, old);
    }
}
```

## Build System Integration

**Makefile:**
```makefile
CFLAGS += -I$(LLVM_PROJECT)/clang/nullsafe-headers/include
```

**CMake:**
```cmake
include_directories(${LLVM_PROJECT}/clang/nullsafe-headers/include)
```

## Annotating Third-Party Libraries

Want null-safe headers for other libraries? Here's how:

### Popular Libraries to Annotate

- **OpenSSL** - Crypto functions (many return NULL on error)
- **zlib** - Compression (gzopen returns NULL on failure)
- **libcurl** - HTTP client (curl_easy_init returns NULL on failure)
- **SQLite** - Database (sqlite3_open returns status, but many functions return NULL)
- **libpng/libjpeg** - Image processing (create functions return NULL)
- **POSIX** - pthread, socket APIs (many nullable returns)

### How to Annotate

1. **Start with a wrapper header:**
```c
/*
 * curl_nullsafe.h - Null-safe libcurl wrapper
 */
#ifndef CURL_NULLSAFE_H
#define CURL_NULLSAFE_H

#include <curl/curl.h>

/* Override key functions with annotations */
CURL* _Nullable curl_easy_init(void);
CURLcode curl_easy_setopt(CURL* _Nonnull handle, CURLoption option, ...);
void curl_easy_cleanup(CURL* _Nullable handle);
char* _Nullable curl_easy_escape(CURL* _Nonnull handle,
                                  const char* _Nonnull string,
                                  int length);

#endif
```

2. **Focus on common patterns:**
   - Constructor functions → usually return `_Nullable`
   - Destructor/cleanup functions → usually accept `_Nullable`
   - Getters that can fail → return `_Nullable`
   - Required parameters → mark `_Nonnull`

3. **Test against real usage:**
```bash
clang -I. -include curl_nullsafe.h mycode.c
```

## FAQ

**Q: Does this require cbang?**
A: No. Works with any Clang. cbang provides better flow-sensitive analysis.

**Q: Do I need to recompile libc?**
A: No. These are just declarations; links to system libc.

**Q: What about third-party libraries?**
A: Use `-isystem` to suppress warnings:
```bash
clang -I./nullsafe-headers/include -isystem /usr/include/python3.9 mycode.c
```

**Q: Performance overhead?**
A: Zero. All checks are compile-time only.

## Contributing

Guidelines for adding annotations:
1. Mark `_Nonnull` only when NULL causes undefined behavior
2. Mark `_Nullable` when NULL is explicitly allowed as input
3. Unmarked return types default to `_Nullable` (conservative)
4. Test against real libc to verify correctness

## License

Apache 2.0 with LLVM exceptions (same as LLVM/Clang).
