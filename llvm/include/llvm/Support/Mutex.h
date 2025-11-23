//===- llvm/Support/Mutex.h - Mutex Operating System Concept -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the llvm::sys::Mutex class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_MUTEX_H
#define LLVM_SUPPORT_MUTEX_H

#include "llvm/Support/Threading.h"
#include <cassert>
#ifndef BINJI_HACK
#include <mutex>
#endif

namespace llvm
{
  namespace sys
  {
#ifdef BINJI_HACK
    // BINJI_HACK: WASI doesn't support mutexes, provide stub implementation
    namespace {
      struct stub_recursive_mutex {
        void lock() {}
        void unlock() {}
        bool try_lock() { return true; }
      };
      template<typename T>
      struct stub_lock_guard {
        stub_lock_guard(T&) {}
      };
    }
#endif

    /// SmartMutex - A mutex with a compile time constant parameter that
    /// indicates whether this mutex should become a no-op when we're not
    /// running in multithreaded mode.
    template<bool mt_only>
    class SmartMutex {
#ifndef BINJI_HACK
      std::recursive_mutex impl;
#else
      stub_recursive_mutex impl;
#endif
      unsigned acquired = 0;

    public:
      bool lock() {
        if (!mt_only || llvm_is_multithreaded()) {
          impl.lock();
          return true;
        }
        // Single-threaded debugging code.  This would be racy in
        // multithreaded mode, but provides not basic checks in single
        // threaded mode.
        ++acquired;
        return true;
      }

      bool unlock() {
        if (!mt_only || llvm_is_multithreaded()) {
          impl.unlock();
          return true;
        }
        // Single-threaded debugging code.  This would be racy in
        // multithreaded mode, but provides not basic checks in single
        // threaded mode.
        assert(acquired && "Lock not acquired before release!");
        --acquired;
        return true;
      }

      bool try_lock() {
        if (!mt_only || llvm_is_multithreaded())
          return impl.try_lock();
        return true;
      }
    };

    /// Mutex - A standard, always enforced mutex.
    typedef SmartMutex<false> Mutex;

    template <bool mt_only>
#ifndef BINJI_HACK
    using SmartScopedLock = std::lock_guard<SmartMutex<mt_only>>;
#else
    using SmartScopedLock = stub_lock_guard<SmartMutex<mt_only>>;
#endif

    typedef SmartScopedLock<false> ScopedLock;
  }
}

#endif
