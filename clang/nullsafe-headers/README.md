# Null-Safe C Standard Library Headers

Nullability-annotated C standard library headers using Clang's `_Nonnull` and `_Nullable` attributes.

## Overview

These headers provide compile-time null-safety checking for C programs by annotating the standard library with nullability information. They work with **any version of Clang** (not just cbang) but are most effective when combined with flow-sensitive null-safety analysis.

### What Problem Does This Solve?

The C standard library doesn't specify which pointers can be NULL. This leads to bugs like:

```c
char* str = getenv("PATH");  // Can return NULL
strlen(str);                  // Crash if str is NULL!
```

With these headers, Clang can catch such bugs at compile time:

```c
char* str = getenv("PATH");   // str has type: char* _Nullable
strlen(str);                   // ERROR: passing _Nullable to _Nonnull parameter

// Fixed version:
if (str) {
    strlen(str);               // OK: str narrowed to _Nonnull in this branch
}
```

## Features

- ✅ **Standards-compliant**: Uses only standard Clang attributes
- ✅ **Zero runtime overhead**: Purely compile-time checking
- ✅ **Drop-in replacement**: Same API as standard headers
- ✅ **No reimplementation**: Just declarations, links to system libc
- ✅ **Well-documented**: Comments explain null-safety contracts

## Files

| Header | Lines | Functions | Description |
|--------|-------|-----------|-------------|
| `string.h` | 183 | 30+ | String manipulation and memory operations |
| `stdlib.h` | 222 | 40+ | Memory allocation, conversions, utilities |
| `stdio.h` | 235 | 50+ | File I/O and formatted input/output |

## Usage

### Basic Usage

```c
// Include null-safe headers instead of system headers
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

void safe_string_copy(const char* source) {
    if (source) {  // Check before use
        char* dest = malloc(strlen(source) + 1);
        if (dest) {  // Check malloc return
            strcpy(dest, source);
            free(dest);
        }
    }
}
```

Compile with:

```bash
clang -I/path/to/nullsafe-headers/include mycode.c
```

### With cbang (C! compiler)

For full flow-sensitive null checking:

```bash
cbang -I/path/to/nullsafe-headers/include mycode.c
```

### Integration with Build Systems

**Makefile:**
```makefile
CFLAGS += -I$(LLVM_PROJECT)/clang/nullsafe-headers/include
```

**CMake:**
```cmake
include_directories(${LLVM_PROJECT}/clang/nullsafe-headers/include)
```

**Buck2/Bazel:**
```python
compiler_flags = [
    "-I$(location //clang/nullsafe-headers:headers)",
]
```

## Design Principles

### Conservative Defaults

By default in cbang, all pointers are assumed `_Nullable` (can be NULL). These headers explicitly mark:

- **`_Nonnull`**: Functions that require non-null pointers (UB if passed NULL)
- **`_Nullable`**: Functions that explicitly allow NULL (like `free()`)
- **Unmarked**: Returns that can be NULL (like `malloc()`)

### Key Annotations

**Functions requiring non-null inputs:**
```c
size_t strlen(const char* _Nonnull s);           // UB if NULL
void* _Nonnull memcpy(void* _Nonnull dest,       // UB if either NULL
                      const void* _Nonnull src,
                      size_t n);
```

**Functions returning nullable:**
```c
void* _Nullable malloc(size_t size);             // Returns NULL on failure
char* _Nullable strchr(const char* _Nonnull s,   // Returns NULL if not found
                       int c);
FILE* _Nullable fopen(const char* _Nonnull name, // Returns NULL on error
                      const char* _Nonnull mode);
```

**Functions accepting NULL:**
```c
void free(void* _Nullable ptr);                  // NULL is explicitly allowed
int fflush(FILE* _Nullable stream);              // NULL means "flush all"
```

## Examples

### Memory Safety

```c
#include "stdlib.h"
#include "string.h"

void process_data(size_t size) {
    char* buffer = malloc(size);  // Returns _Nullable

    // ERROR: buffer might be NULL
    buffer[0] = 'x';

    // CORRECT: Check first
    if (buffer) {
        buffer[0] = 'x';
        free(buffer);  // free() accepts _Nullable
    }
}
```

### String Operations

```c
#include "string.h"

void copy_string(char* dest, const char* src) {
    // ERROR: both might be NULL
    strcpy(dest, src);

    // CORRECT: Check both
    if (dest && src) {
        strcpy(dest, src);
    }
}
```

### File I/O

```c
#include "stdio.h"

void read_file(const char* filename) {
    FILE* fp = fopen(filename, "r");  // Returns _Nullable

    // ERROR: fp might be NULL
    char buf[100];
    fgets(buf, sizeof(buf), fp);

    // CORRECT: Check first
    if (fp) {
        fgets(buf, sizeof(buf), fp);
        fclose(fp);
    }
}
```

## Compatibility

### Clang Versions

These headers work with any Clang version that supports nullability attributes (Clang 3.7+):

- Clang 3.7+: Basic support
- Clang 9.0+: Improved diagnostics
- cbang: Full flow-sensitive analysis

### C Standards

- ✅ C89/C90
- ✅ C99
- ✅ C11
- ✅ C17/C18
- ✅ C23 (draft)

### Operating Systems

Works with any libc implementation:
- ✅ GNU libc (Linux)
- ✅ musl libc (Alpine, embedded)
- ✅ BSD libc (FreeBSD, macOS)
- ✅ Microsoft CRT (Windows with Clang)

## Testing

Run the test suite:

```bash
cd tests/
clang -I../include test_annotations.c -o test
./test
```

For cbang-specific tests:

```bash
cbang -I../include -fsyntax-only test_annotations.c
# Should report errors for unsafe code
```

## Implementation Status

### Completed ✅
- [x] `string.h` - String manipulation and memory operations
- [x] `stdlib.h` - Memory allocation, conversions, utilities
- [x] `stdio.h` - File I/O and formatted I/O

### Planned
- [ ] `time.h` - Date and time functions
- [ ] `signal.h` - Signal handling
- [ ] `setjmp.h` - Non-local jumps
- [ ] `math.h` - Mathematical functions (few pointers)
- [ ] POSIX extensions (optional)

## Contributing

When adding annotations:

1. **Identify null requirements**: Can the function accept NULL? Does it return NULL?
2. **Mark `_Nonnull`**: Only when NULL would be undefined behavior
3. **Mark `_Nullable`**: When NULL is explicitly allowed as input
4. **Document**: Add comments explaining special cases
5. **Test**: Verify against real libc implementation

### Testing Your Annotations

```bash
# Compile against real libc to verify correctness
clang -I../include test.c -o test
./test

# Check with cbang for safety
cbang -I../include -fsyntax-only test.c
```

## FAQ

### Q: Does this require cbang?

**A:** No! These headers work with any Clang. However, cbang provides better diagnostics with flow-sensitive analysis.

### Q: Do I need to recompile libc?

**A:** No! These are just declarations. Your program still links to the system libc.

### Q: What about performance?

**A:** Zero runtime overhead. All checks are compile-time only.

### Q: Can I use this with C++?

**A:** Yes, but C++ has better alternatives (references, smart pointers). These headers are C-focused.

### Q: What about third-party libraries?

**A:** For third-party libraries, use `-isystem` to suppress warnings from their headers, or annotate them separately.

### Q: How do I suppress warnings?

**A:** Use `-Wno-nullability-*` flags or `-isystem` for third-party code:

```bash
clang -I./nullsafe-headers/include \
      -isystem /usr/include/python3.9 \
      mycode.c
```

## License

These headers contain only function declarations from the C standard library specification.
They are provided under the same license as LLVM/Clang (Apache 2.0 with LLVM exceptions).

## See Also

- [Clang Nullability Attributes](https://clang.llvm.org/docs/AttributeReference.html#nullability-attributes)
- [cbang (C!) Documentation](../README.md)
- [LLVM Project](https://llvm.org/)
