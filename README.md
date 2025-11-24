# Clang with Strict Nullability

A C compiler with opt-in strict null safety, bringing TypeScript/Kotlin-style nullable type checking to C through flow-sensitive analysis.

**Strict nullability mode** extends Clang's existing `_Nonnull` and `_Nullable` annotations with flow-sensitive type narrowing. All pointers are nullable by default unless explicitly marked `_Nonnull`. The compiler catches null-pointer bugs before they crash your program—all without runtime overhead.

This is a fork of Clang that adds flow-sensitive nullability analysis while remaining 100% compatible with existing C code. It has all the power of Clang and the same API, plus IDE support through the enhanced `clangd` binary.

## Usage

**Enable strict nullability (on by default):**
```bash
clang mycode.c                    # Strict nullability ON, warnings only
clang -Werror=nullability mycode.c  # Promote warnings to errors
```

**Disable for legacy code:**
```bash
clang -fno-strict-nullability mycode.c  # Pure standard Clang behavior
```

**Gradual adoption:**
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
- **Flow-sensitive type narrowing**: `if (p)` proves `p` is non-null in that scope
- **Dereference checking**: Errors when dereferencing nullable pointers
- **Type checking**: Through function calls, returns, and assignments
- **Typedef support**: Nullability annotations work with typedefs
- **Early-exit narrowing**: `return`, `goto`, `break`, `continue` are understood
- **Function call invalidation**: Conservative by default - assumes functions have side effects
- **Pure function support**: Use `__attribute__((pure))` or `__attribute__((const))` to preserve narrowing
- **Multi-level pointer narrowing**: Works with `int**`, `int***`, etc.
- **Pointer arithmetic narrowing**: `q = p + 1` preserves narrowing from `p`
- **Standard library annotations**: Nullability-annotated headers (see `clang/nullsafe-headers/`)
- **Real-world compatibility**: Tested on cJSON, SQLite
- **IDE integration**: Enhanced `clangd` with nullability diagnostics

### Future Work
- Direct assignment narrowing (`q = p` should narrow `q` if `p` is narrowed)
- Bounds safety integration (combine with `-fbounds-safety`)
- Additional compiler flags for fine-grained control

## How It Works

This implementation adds **flow-sensitive nullability analysis** directly to Clang's semantic analyzer. Unlike languages like Swift, TypeScript, or Kotlin that have flow analysis built into their type systems from the start, strict nullability brings this capability to C by layering it onto Clang's traditionally flow-insensitive type system. When you write `if (p)`, the compiler tracks that `p` is non-null within that branch and allows you to use it where a non-nullable pointer is expected—all without runtime overhead.

### Function Call Invalidation

By default, **strict nullability assumes all function calls have side effects** and conservatively invalidates narrowing:

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

This is conservative but safe - functions can modify global state, escaped pointers, or have other side effects.

**Pure Functions**: Mark side-effect-free functions with `__attribute__((pure))` or `__attribute__((const))` to preserve narrowing:

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
- `__attribute__((const))`: Function only uses its arguments (e.g., `tolower`, `abs`)
- `__attribute__((pure))`: Function may read global state but doesn't modify it (e.g., `strlen`)

Both preserve narrowing since they can't invalidate pointers.

**Good news**: Many standard library functions already have these attributes! The GNU C library's `ctype.h`, `string.h`, and other headers already mark functions like `tolower()`, `strlen()`, etc. as pure/const, so strict nullability automatically recognizes them without any special configuration.

## Null-Safe C Standard Library

The `clang/nullsafe-headers/` directory contains nullability-annotated versions of the C standard library headers (`string.h`, `stdlib.h`, `stdio.h`). These headers work with **any version of Clang**, not just this fork, but provide the best experience when combined with strict nullability's flow-sensitive analysis.

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
