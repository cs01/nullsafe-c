# Nullsafe C: An experimental C/C++ compiler

[![Test Null-Safety](https://github.com/cs01/llvm-project/actions/workflows/test-null-safety.yml/badge.svg)](https://github.com/cs01/llvm-project/actions/workflows/test-null-safety.yml)

Nullsafe C adds NULL checks to catch errors at compile-time, not runtime. It is 100% compatible with existing C codebases and can be used incrementally to identify safety issues at compile-time.

You can annotate your code with `_Nonnull` to presere narrowing.

**Try it online:** [Interactive Playground](https://cs01.github.io/llvm-project/) - See null-safety warnings in real-time in your browser!

It does this by making two changes:
1. All pointers are nullable by default, unless explicitly marked `_Nonnull`. Clang already allows the code to be annotated with `_Nullable` and `_Nonnull`, but this compiler treats all unmarked pointers as nullable by default.
2. The compiler tracks when you've null-checked a pointer and knows it's safe to use. When you write `if (p)`, the type system understands `p` is non-null in that branch.

## Example

```c
void unsafe(int *data) {
  *data = 42; // warning - data might be null!
}

void safe(int *data) {
  if (data) {
    *data = 42; // OK - data is non-null here
  }
}

void safe_typed(int *_Nonnull data) {
  *data = 42; // OK - data is known to be non-null by the compiler
}

```
Try it out in the [Interactive Playground](https://cs01.github.io/llvm-project/).

## Installation

### Prerequisites

**macOS:**
```bash
brew install zstd
xcode-select --install  # If not already installed
```

### Quick Install

```bash
curl -fsSL https://raw.githubusercontent.com/cs01/llvm-project/null-safe-c-dev/install.sh | bash
```

Or download manually from [releases](https://github.com/cs01/llvm-project/releases).

### Windows

Builds not available at this time, you must clone and build locally.

### What's Included

Each release includes:
- **`clang`** - The Null-Safe C compiler with flow-sensitive null checking
- **`clangd`** - Language server for IDE integration (VSCode, vim, Neovim, Emacs, etc.)

### IDE Integration

Once installed, configure your editor to use the null-safe `clangd`:

**VSCode:**
```json
// settings.json
{
  "clangd.path": "/path/to/null-safe-clang/bin/clangd"
}
```

**Neovim/vim:**
```lua
require('lspconfig').clangd.setup({
  cmd = { '/path/to/null-safe-clang/bin/clangd' }
})
```

This gives you real-time null-safety warnings as you type!

## Memory Safety

Do not that this is not a comprehensive solution, since Null pointer dereferences are just one category of memory safety bugs.

| Safety Issue            | Standard C | **Null-Safe Clang** (null checking) |
|-------------------------|------------|:-----------------------------------:|
| Null pointer dereferences | ❌ Unsafe | **✅ Fixed**                        |
| Buffer overflows        | ❌ Unsafe  | ❌ Unsafe                            |
| Use-after-free          | ❌ Unsafe  | ❌ Unsafe                            |
| Double-free             | ❌ Unsafe  | ❌ Unsafe                            |
| Uninitialized memory    | ❌ Unsafe  | ❌ Unsafe                            |

Although this doesn't fix all memory safety issues, it catches Null pointer dereferences for free.

### Why you still might want to try this

While Null-Safe Clang doesn't solve all memory safety issues, null pointer dereferences are a significant problem:

- Many memory safety bugs involve null pointer dereferences
- Easier to adopt than rewriting in Rust (100% compatible with existing C code)
- Complements other efforts (combine with `-fbounds-safety` for buffer safety)
- Incremental deployment (warnings by default, can enable per-file)


## Usage

Basic usage (warnings enabled by default):
```bash
# Warnings for nullable dereferences
clang mycode.c
# Treat nullability issues as errors
clang -Werror=nullability mycode.c
# Turn off nullability checking
clang -fno-strict-nullability mycode.c
```


## Features

- Nullable-by-default: All pointers are `_Nullable` unless marked `_Nonnull`
- Flow-sensitive narrowing: `if (p)` proves `p` is non-null in that scope
- Early-exit patterns: Understands `return`, `goto`, `break`, `continue`
- Pointer arithmetic: `q = p + 1` preserves narrowing from `p`
- Type checking through function calls, returns, and assignments
- Works with Typedefs
- Assumes functions have side effects (use `__attribute__((pure))` or `__attribute__((const))` to preserve narrowing)
- Null-safe headers: Annotated C standard library in `clang/nullsafe-headers/`
- IDE integration: `clangd` built from this fork has the same logic and warnings as clang

## Null-Safe C Standard Library

Nullability-annotated headers for `string.h`, `stdlib.h`, and `stdio.h` are available in `clang/nullsafe-headers/`. See [`clang/nullsafe-headers/README.md`](clang/nullsafe-headers/README.md) for details.
