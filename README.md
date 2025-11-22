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

### Future Work
- [ ] Multi-level pointer support (`**!`, `***!` syntax)
- [ ] Standard library nullability annotations (libc headers)
- [ ] Bounds safety integration (combine with `-fbounds-safety`)
- [ ] Interop mode for gradual adoption in existing codebases

## How It Works

cbang implements **flow-sensitive nullability analysis** directly in Clang's semantic analyzer. Unlike languages like Swift, TypeScript, or Kotlin that have flow analysis built into their type systems from the start, C!'s implementation brings this capability to C by layering it onto Clang's traditionally flow-insensitive type system. When you write `if (p)`, the compiler tracks that `p` is non-null within that branch and allows you to use it where a non-nullable pointer is expected—all without runtime overhead.


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
