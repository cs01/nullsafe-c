# Nullsafe C: An experimental C/C++ compiler

[![Test Null-Safety](https://github.com/cs01/llvm-project/actions/workflows/test-null-safety.yml/badge.svg)](https://github.com/cs01/llvm-project/actions/workflows/test-null-safety.yml)

Nullsafe C adds NULL checks to catch errors at compile-time. It is 100% compatible with existing C codebases and can be used incrementally to identify safety issues at compile-time. 

This provides the following benefits:
* This makes the code safer by reducing the number of potential runtime null dereferences
* Improves developer experience by shifting errors left
* Makes the code more readable
* Adds type errors that other more modern languages have (Rust, TypeScript, Kotlin)

**Try it online:** [Interactive Playground](https://cs01.github.io/llvm-project/) - See null-safety warnings in real-time in your browser!

Nullsafe C treats all pointers as potentially null ('nullable') unless it is certain they are not. It does this in two ways. 

The first is by semantic analysis: if you test a pointer with `if(p)`, then it knows that branch contains a non-null pointer.

The second is by using Clang's [`Nullability`](https://clang.llvm.org/docs/AttributeReference.html#nullability-attributes) attributes, in particular `_Nonnull`. If a pointer is marked as `_Nonnull` the compiler will require a pointer it knows it not null is passed to it. This can be done either by passing a `_Nonnull`-annotated pointer, or by doing type narrowing. 

If using a compiler other than clang, you can add `#define _Nonnull` as a no-op. You will not get the same compile checks as with Nullsafe C (clang fork), but the compillation will still succeed without error.

## Examples

```c
void unsafe(int *data) {
  *data = 42; // warning: dereferencing nullable pointer of type 'int * _Nullable'
}
```
[Try it in the interactive playground](https://cs01.github.io/llvm-project/?code=dm9pZCB1bnNhZmUoaW50ICpkYXRhKSB7CiAgKmRhdGEgPSA0MjsgLy8gd2FybmluZzogZGVyZWZlcmVuY2luZyBudWxsYWJsZSBwb2ludGVyIG9mIHR5cGUgJ2ludCAqIF9OdWxsYWJsZScKfQ%3D%3D)

Type narrowing:
```c
void safe(int *data) {
  if (data) {
    *data = 42; // OK - data is non-null here
  }
}
```
[Try it in the interactive playground](https://cs01.github.io/llvm-project/?code=dm9pZCBzYWZlKGludCAqZGF0YSkgewogIGlmIChkYXRhKSB7CiAgICAqZGF0YSA9IDQyOyAvLyBPSyAtIGRhdGEgaXMgbm9uLW51bGwgaGVyZQogIH0KfQ%3D%3D)

Anontated with `_Nonnull`:
```c
void safe_typed(int *_Nonnull data) {
  *data = 42; // OK - we know data is not null so we can derefernce it
}
```
[Try it in the interactive playground](https://cs01.github.io/llvm-project/?code=dm9pZCBzYWZlX3R5cGVkKGludCAqX05vbm51bGwgZGF0YSkgewogICpkYXRhID0gNDI7IC8vIE9LIC0gd2Uga25vdyBkYXRhIGlzIG5vdCBudWxsIHNvIHdlIGNhbiBkZXJlZmVybmNlIGl0Cn0%3D)


## Installation

### Quick Install

```bash
curl -fsSL https://raw.githubusercontent.com/cs01/llvm-project/null-safe-c-dev/install.sh | bash
```

Or download manually from [releases](https://github.com/cs01/llvm-project/releases).

On mac you may need to do the following:
```bash
brew install zstd
xcode-select --install  # If not already installed
```

### Windows

Builds not available at this time, you must clone and build locally.

### What's Included

Each release includes:
- **`clang`** - The Null-Safe C compiler with flow-sensitive null checking
- **`clangd`** - Language server for IDE integration (VSCode, vim, Neovim, Emacs, etc.)

### IDE Integration

Once installed, configure your editor to use the null-safe `clangd`. Install the `clangd` extension from llvm and set the path to the clangd binary you just downloaded.

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
