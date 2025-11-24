# C with Strict Nullability: An experimental fork of the Clang compiler

An experimental Clang fork that adds **flow-sensitive null safety** to C, inspired by modern languages like TypeScript, Kotlin, and Rust.

## What This Adds

Modern languages prevent null pointer crashes through **nullable types** and **type narrowing** (also called refinement). When you write `if (p != null)`, the type system understands `p` is non-null in that branch. This catches null pointer dereferences at compile time instead of crashing at runtime.

**This fork brings that safety to C** by extending Clang's existing `_Nonnull` and `_Nullable` annotations with intelligent flow analysis. All pointers are nullable by default unless explicitly marked `_Nonnull`.

## Memory Safety: The Bigger Picture

Null pointer dereferences are just one category of memory safety bugs. Here's where this project fits in the landscape:

| Safety Category | Example Vulnerability | This Project | Rust | Bounds Safety¹ |
|----------------|----------------------|--------------|------|----------------|
| **Null pointer dereferences** | CVE-2019-11932 (WhatsApp) | ✅ **Fixed** | ✅ Fixed | ❌ Not addressed |
| **Buffer overflows** | CVE-2014-0160 (Heartbleed) | ❌ Not addressed | ✅ Fixed | ✅ Fixed |
| **Use-after-free** | CVE-2021-30551 (Chrome) | ❌ Not addressed | ✅ Fixed | ❌ Not addressed |
| **Double-free** | CVE-2019-5786 (Chrome) | ❌ Not addressed | ✅ Fixed | ❌ Not addressed |
| **Uninitialized memory** | CVE-2018-4259 (WebKit) | ❌ Not addressed | ✅ Fixed | ⚠️  Partial |
| **Type confusion** | CVE-2015-0311 (Flash) | ❌ Not addressed | ✅ Fixed | ❌ Not addressed |

<sup>1. Clang's `-fbounds-safety` experimental feature</sup>

### Why This Still Matters

While this project doesn't solve all memory safety issues, **null pointer dereferences are extremely common**:

- **~25% of all CVEs** involve null pointer dereferences ([Microsoft Security Response Center](https://github.com/microsoft/MSRC-Security-Research/blob/master/papers/2019/The%20Memory%20Safety%20Story.pdf))
- Easier to adopt than a full language rewrite (remains 100% compatible with C)
- Can be enabled incrementally (warnings by default, not errors)
- Complements other safety efforts (can combine with `-fbounds-safety`)

**Real-world CVEs this would catch:**
- CVE-2019-11932 (WhatsApp): Null pointer dereference in MP4 parsing
- CVE-2021-3520 (lz4): NULL pointer dereference in LZ4_decompress_safe_continue
- CVE-2020-14409 (SDL2): NULL pointer dereference in audio driver
- *[TODO: Add more specific examples with links]*

---

## Quick Example

```c
void process(int* data) {
    if (data) {
        *data = 42;        // ✓ OK - strict nullability knows data is non-null here
    }
}

void unsafe(int* data) {
    *data = 42;            // ⚠️  Warning - data might be null!
}
```

**Standard Clang/GCC**: Both functions compile without warnings.
**This fork**: The `unsafe` function warns you about the potential null dereference.

This experimental fork of Clang adds flow-sensitive nullability analysis while remaining 100% compatible with standard C. It includes all of Clang's features plus enhanced nullability checking in both the compiler and the `clangd` language server.

**By default, strict nullability is enabled and issues warnings.** You can promote warnings to errors with `-Werror=nullability`, or disable the feature entirely with `-fno-strict-nullability`.

## Usage

**Basic usage (warnings enabled by default):**
```bash
clang mycode.c                          # Warnings for nullable dereferences
```

**Promote warnings to errors:**
```bash
clang -Werror=nullability mycode.c      # Treat nullability issues as errors
```

**Disable strict nullability:**
```bash
clang -fno-strict-nullability mycode.c  # Turn off nullability checking
```

**Gradual adoption (per-file or per-function):**
```c
// Disable warnings for specific files
#pragma clang diagnostic ignored "-Wnullability"

// Or per-function
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability"
void legacy_function(int* p) { ... }
#pragma clang diagnostic pop
```

## Features

- **Nullable-by-default**: All pointers are `_Nullable` unless marked `_Nonnull`
- **Flow-sensitive narrowing**: `if (p)` proves `p` is non-null in that scope
- **Smart invalidation**: Assumes functions have side effects; use `__attribute__((pure))` or `__attribute__((const))` to preserve narrowing
- **Early-exit patterns**: Understands `return`, `goto`, `break`, `continue`
- **Multi-level pointers**: Works with `int**`, `int***`, etc.
- **Pointer arithmetic**: `q = p + 1` preserves narrowing from `p`
- **Type checking**: Through function calls, returns, and assignments
- **Typedef support**: Nullability annotations work seamlessly with typedefs
- **Null-safe headers**: Annotated C standard library in `clang/nullsafe-headers/`
- **IDE integration**: Enhanced `clangd` with real-time nullability diagnostics
- **Real-world tested**: Validated on cJSON, SQLite

### Future Work
- Direct assignment narrowing (`q = p` should narrow `q` if `p` is narrowed)
- Bounds safety integration (combine with `-fbounds-safety`)

## How It Works

Strict nullability adds **flow-sensitive analysis** to Clang's semantic analyzer. When you write `if (p)`, the compiler tracks that `p` is non-null within that branch—just like TypeScript, Swift, or Kotlin do. The difference is we add this to C without changing the language itself.

### Function Call Invalidation

By default, function calls invalidate narrowing because they may have side effects:

```c
void some_function(void);

void example(int* p) {
    if (p) {
        *p = 1;           // OK - p is narrowed to non-null
        some_function();  // Invalidates narrowing
        *p = 2;           // Warning: p is nullable again
    }
}
```

This is conservative but safe—functions can modify global state or escaped pointers.

**Preserve narrowing with pure functions:**

Mark side-effect-free functions with `__attribute__((pure))` or `__attribute__((const))`:

```c
int tolower(int c) __attribute__((const));  // No side effects

void example(char* p) {
    if (p) {
        tolower(*p);  // Pure function - doesn't invalidate
        *p = 'x';     // OK - p is still narrowed
    }
}
```

The difference:
- `__attribute__((const))`: Only uses its arguments (e.g., `tolower`, `abs`)
- `__attribute__((pure))`: May read globals but doesn't modify anything (e.g., `strlen`)

Many standard library functions already have these attributes in GNU libc headers, so strict nullability automatically recognizes them.

## Null-Safe C Standard Library

The `clang/nullsafe-headers/` directory contains nullability-annotated standard library headers. These headers tell the compiler which functions can return NULL and which parameters can be NULL—information missing from system headers.

**The difference:** Functions like `malloc`, `strdup`, `getenv` are annotated to return `_Nullable` pointers (can be NULL), while parameters like `strlen`'s input are marked `_Nonnull` (must not be NULL).

**Without null-safe headers (using system headers):**
```c
#include <string.h>  // System headers - no parameter nullability info

void example(const char* input) {
    char* result = strdup(input);  // No warning - compiler doesn't know strdup needs nonnull
    result[0] = 'x';                // ⚠️  Warning: result might be NULL (all pointers nullable by default)
}
```
You get warnings about dereferencing nullable return values, but not about passing NULL to functions.

**With null-safe headers:**
```c
#include "string.h"  // From clang/nullsafe-headers - strdup parameter is _Nonnull

void example(const char* input) {
    char* result = strdup(input);  // ⚠️  Warning: passing nullable 'input' to nonnull parameter!
    result[0] = 'x';                // ⚠️  Warning: dereferencing nullable pointer!

    // Fix both issues:
    if (input) {
        char* result = strdup(input);  // ✓ OK - input is non-null
        if (result) {
            result[0] = 'x';            // ✓ OK - result is non-null
            free(result);
        }
    }
}
```
Now you get warnings for BOTH nullable returns AND passing NULL to nonnull parameters.

The headers catch both directions:

```c
#include "string.h"

void test_returns(void) {
    char* str = malloc(100);  // malloc returns _Nullable
    str[0] = 'x';             // ⚠️  Warning: str might be NULL!
}

void test_params(char* str) {
    strlen(str);              // ⚠️  Warning: str is nullable, strlen expects nonnull!

    if (str) {
        strlen(str);          // ✓ OK - str is non-null here
    }
}
```

### Quick Start

```bash
# Compile with null-safe headers
clang -Iclang/nullsafe-headers/include mycode.c
```

Example code:
```c
#include "string.h"
#include "stdlib.h"

void safe_code(const char* input) {
    if (!input) return;  // Early return for null check

    // After the check, strict nullability knows input is non-null
    size_t len = strlen(input);  // OK - strlen expects non-null
    char* copy = malloc(len + 1);

    if (copy) {  // Check malloc result (returns _Nullable)
        strcpy(copy, input);  // OK - both non-null
        free(copy);
    }
}
```

See [`clang/nullsafe-headers/README.md`](clang/nullsafe-headers/README.md) for complete documentation.


---

# The LLVM Compiler Infrastructure

This repository contains the source code for LLVM, a toolkit for the
construction of highly optimized compilers, optimizers, and run-time
environments.

The LLVM project has multiple components. The core of the project is
itself called "LLVM". This contains all of the tools, libraries, and header
files needed to process intermediate representations and convert them into
object files. Tools include an assembler, disassembler, bitcode analyzer, and
bitcode optimizer.

C-like languages use the [Clang](https://clang.llvm.org/) frontend. This
component compiles C, C++, Objective-C, and Objective-C++ code into LLVM bitcode
-- and from there into object files, using LLVM.

Other components include:
the [libc++ C++ standard library](https://libcxx.llvm.org),
the [LLD linker](https://lld.llvm.org), and more.

## Getting the Source Code and Building LLVM

Consult the
[Getting Started with LLVM](https://llvm.org/docs/GettingStarted.html#getting-the-source-code-and-building-llvm)
page for information on building and running LLVM.

For information on how to contribute to the LLVM project, please take a look at
the [Contributing to LLVM](https://llvm.org/docs/Contributing.html) guide.

## Getting in touch

Join the [LLVM Discourse forums](https://discourse.llvm.org/), [Discord
chat](https://discord.gg/xS7Z362),
[LLVM Office Hours](https://llvm.org/docs/GettingInvolved.html#office-hours) or
[Regular sync-ups](https://llvm.org/docs/GettingInvolved.html#online-sync-ups).

The LLVM project has adopted a [code of conduct](https://llvm.org/docs/CodeOfConduct.html) for
participants to all modes of communication within the project.
