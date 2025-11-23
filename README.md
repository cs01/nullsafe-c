# C! Null-Safe C

**cbang** is a drop-in replacement for clang with better null checking, type refinement, and an optional new type `*!` (non-null). Its goal is to fix the 'billion dollar mistake'.

Write `int*!` for non-nullable pointers and `int*` for nullable ones. The compiler catches null-pointer bugs before they crash your program through flow-sensitive type refinement. cbang is 100% compatible with existing C code, making it a drop-in replacement for your existing toolchain.

The cbang compiler is a fork of the clang compiler with the addition of null safety checks and the `*!` syntax. This means it has all the power of clang and the same API. It also means IDE integrations for the new `*!` work with the clangd build of cbang.

## Roadmap

### Completed
- [x] `*!` syntax for non-nullable pointers of all types
- [x] Single-level pointers default to `_Nullable`
- [x] Nullable → non-nullable conversion errors (enabled by default)
- [x] Flow-sensitive type narrowing after null checks (e.g., `if (p)` narrows `p` to non-null)
- [x] **Dereference checking**: Error when dereferencing nullable pointers (`*p` where `p` is `int*`)
- [x] Type checking through function calls and returns
- [x] Typedef support for nullability annotations
- [x] Real-world code compatibility (tested on cJSON, SQLite)
- [x] IDE tooling (use C!'s `clangd` binary)
- [x] **Early-exit narrowing** with `return`, `goto`, `break`, `continue`
- [x] **Function call invalidation**: Conservative by default - all functions assumed to have side effects
- [x] **Pure function support**: Use `__attribute__((pure))` or `__attribute__((const))` to mark side-effect-free functions
- [x] **Multi-level pointer narrowing** (`int**`, `int***`, etc.)
- [x] **Pointer arithmetic narrowing** (`q = p + 1` preserves narrowing)
- [x] **Standard library nullability annotations** (see `clang/nullsafe-headers/`)

### Future Work
- [ ] Direct assignment narrowing (`q = p` should narrow `q` if `p` is narrowed)
- [ ] Bounds safety integration (combine with `-fbounds-safety`)
- [ ] Interop mode for gradual adoption in existing codebases

## How It Works

cbang implements **flow-sensitive nullability analysis** directly in Clang's semantic analyzer. Unlike languages like Swift, TypeScript, or Kotlin that have flow analysis built into their type systems from the start, C!'s implementation brings this capability to C by layering it onto Clang's traditionally flow-insensitive type system. When you write `if (p)`, the compiler tracks that `p` is non-null within that branch and allows you to use it where a non-nullable pointer is expected—all without runtime overhead.

### Function Call Invalidation

By default, **cbang assumes all function calls have side effects** and conservatively invalidates narrowing:

```c
void some_function(void);

void example(int* p) {
    if (p) {
        *p = 1;           // OK - p is narrowed to non-null
        some_function();  // Invalidates narrowing
        *p = 2;           // ERROR - p is nullable again
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

**Good news**: Many standard library functions already have these attributes! The GNU C library's `ctype.h`, `string.h`, and other headers already mark functions like `tolower()`, `strlen()`, etc. as pure/const, so cbang automatically recognizes them without any special configuration.

## Null-Safe C Standard Library

The `clang/nullsafe-headers/` directory contains nullability-annotated versions of the C standard library headers (`string.h`, `stdlib.h`, `stdio.h`). These headers work with **any version of Clang**, not just cbang, but provide the best experience when combined with cbang's flow-sensitive analysis.

### Quick Start

```c
// Use null-safe headers
#include "string.h"
#include "stdlib.h"

void safe_code(const char* input) {
    if (!input) return;  // Early return for null check

    // After the check, cbang knows input is non-null
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
cbang -I/path/to/llvm-project/clang/nullsafe-headers/include mycode.c
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
