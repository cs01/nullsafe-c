# Clang with Strict Nullability

Bringing TypeScript/Kotlin-style null safety to C with flow-sensitive type narrowing.

**Strict nullability** extends Clang's existing `_Nonnull` and `_Nullable` annotations with intelligent flow analysis. When you write `if (p)`, the compiler knows `p` is non-null in that branch. All pointers are nullable by default unless explicitly marked `_Nonnull`. Catch null-pointer bugs at compile time—with zero runtime overhead.

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

This fork adds flow-sensitive nullability analysis to Clang while remaining 100% compatible with standard C. It includes all of Clang's features plus enhanced nullability checking in both the compiler and the `clangd` language server.

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

Strict nullability adds **flow-sensitive analysis** to Clang's semantic analyzer. When you write `if (p)`, the compiler tracks that `p` is non-null within that branch. Unlike Swift, TypeScript, or Kotlin where this is built into the language, we layer it onto C's type system—giving you modern null safety while keeping C's simplicity.

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

The `clang/nullsafe-headers/` directory contains nullability-annotated standard library headers. These work with any Clang version but shine when combined with this fork's flow analysis.

### Quick Start

```c
// Use null-safe headers
#include "string.h"
#include "stdlib.h"

void safe_code(const char* input) {
    if (!input) return;  // Early return for null check

    // After the check, strict nullability knows input is non-null
    size_t len = strlen(input);  // OK
    char* copy = malloc(len + 1);

    if (copy) {  // Check malloc result
        strcpy(copy, input);  // OK - both non-null
        free(copy);
    }
}
```

Compile with:
```bash
clang -I/path/to/llvm-project/clang/nullsafe-headers/include mycode.c
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
