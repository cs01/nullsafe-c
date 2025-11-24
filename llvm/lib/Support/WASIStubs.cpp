//===-- WASIStubs.cpp - WASI Platform Stub Implementations ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides stub implementations for platform-specific functions
// that are not available in WASI.
//
//===----------------------------------------------------------------------===//

#ifdef BINJI_HACK

#include "llvm/Support/Signals.h"
#include "llvm/Support/Watchdog.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include <system_error>

namespace llvm {
namespace sys {

// Signal handling stubs
void RunInterruptHandlers() {
  // WASI stub: No-op
}

void AddSignalHandler(void (*FnPtr)(void *), void *Cookie) {
  // WASI stub: No-op
}

void DefaultOneShotPipeSignalHandler() {
  // WASI stub: No-op
}

void SetOneShotPipeSignalFunction(void (*Handler)()) {
  // WASI stub: No-op
}

void PrintStackTraceOnErrorSignal(StringRef Argv0, bool DisableCrashReporting) {
  // WASI stub: No-op
}

bool RemoveFileOnSignal(StringRef Filename,
                         std::string *ErrMsg) {
  // WASI stub: No-op
  return true;
}

void DontRemoveFileOnSignal(StringRef Filename) {
  // WASI stub: No-op
}

namespace path {
void home_directory(SmallVectorImpl<char> &Result) {
  // WASI stub: Return empty path
  Result.clear();
  Result.push_back('/');
}
} // namespace path

// Watchdog stubs
Watchdog::Watchdog(unsigned int Seconds) {
  // WASI stub: No-op
}

Watchdog::~Watchdog() {
  // WASI stub: No-op
}

// Process stubs
std::optional<std::string> Process::GetEnv(StringRef Name) {
  // WASI stub: Environment variables not supported
  return std::nullopt;
}

Expected<unsigned> Process::getPageSize() {
  // WASI stub: Return standard 64KB page size
  return 65536;
}

size_t Process::GetMallocUsage() {
  // WASI stub: Malloc tracking not supported
  return 0;
}

void Process::GetTimeUsage(TimePoint<> &elapsed,
                           std::chrono::nanoseconds &user_time,
                           std::chrono::nanoseconds &sys_time) {
  // WASI stub: Time usage tracking not supported
  elapsed = std::chrono::system_clock::now();
  user_time = std::chrono::nanoseconds::zero();
  sys_time = std::chrono::nanoseconds::zero();
}

bool Process::ColorNeedsFlush() {
  // WASI stub: No, we use ANSI escape sequences
  return false;
}

const char *Process::OutputBold(bool bg) {
  // WASI stub: Return ANSI bold code
  return "\033[1m";
}

const char *Process::OutputColor(char code, bool bold, bool bg) {
  // WASI stub: Return empty string (no colors)
  return "";
}

const char *Process::ResetColor() {
  // WASI stub: Return ANSI reset code
  return "\033[0m";
}

const char *Process::OutputReverse() {
  // WASI stub: Return ANSI reverse code
  return "\033[7m";
}

std::error_code Process::SafelyCloseFileDescriptor(int FD) {
  // WASI stub: Return success
  return std::error_code();
}

bool Process::FileDescriptorIsDisplayed(int fd) {
  // WASI stub: Return false
  return false;
}

bool Process::FileDescriptorHasColors(int fd) {
  // WASI stub: Return false
  return false;
}

// Process/Program stubs
std::error_code ChangeStdinMode(fs::OpenFlags Flags) {
  // WASI stub: Return success
  return std::error_code();
}

std::error_code ChangeStdoutMode(fs::OpenFlags Flags) {
  // WASI stub: Return success
  return std::error_code();
}

// File system stubs
namespace fs {

int getStdinHandle() {
  // WASI stub: Return standard input file descriptor
  return 0;
}

std::error_code remove(const Twine &Path, bool IgnoreNonExisting) {
  // WASI stub: Return success
  return std::error_code();
}

Expected<file_t> openNativeFileForRead(const Twine &Name, OpenFlags Flags,
                                        SmallVectorImpl<char> *RealPath) {
  // WASI stub: Not supported
  return errorCodeToError(std::make_error_code(std::errc::function_not_supported));
}

Expected<size_t> readNativeFile(int FD, MutableArrayRef<char> Buf) {
  // WASI stub: Not supported
  return errorCodeToError(std::make_error_code(std::errc::function_not_supported));
}

std::error_code closeFile(int &FD) {
  // WASI stub: Minimal implementation
  if (FD >= 0) {
    FD = -1;
    return std::error_code();
  }
  return std::make_error_code(std::errc::bad_file_descriptor);
}

std::error_code status(int FD, file_status &Result) {
  // WASI stub: Return error
  return std::make_error_code(std::errc::function_not_supported);
}

std::error_code status(const Twine &Path, file_status &Result, bool Follow) {
  // WASI stub: Return error
  return std::make_error_code(std::errc::function_not_supported);
}

std::error_code current_path(SmallVectorImpl<char> &Result) {
  // WASI stub: Return empty path
  Result.clear();
  Result.push_back('/');
  return std::error_code();
}

std::error_code set_current_path(const Twine &Path) {
  // WASI stub: No-op
  return std::error_code();
}

std::error_code real_path(const Twine &Path, SmallVectorImpl<char> &Dest,
                           bool ExpandTilde) {
  // WASI stub: Just copy the path
  Path.toVector(Dest);
  return std::error_code();
}

std::error_code is_local(const Twine &Path, bool &Result) {
  // WASI stub: Assume all paths are local
  Result = true;
  return std::error_code();
}

const file_t kInvalidFile = -1;

Expected<size_t> readNativeFileSlice(file_t FileHandle,
                                      MutableArrayRef<char> Buf,
                                      uint64_t Offset) {
  // WASI stub: Not supported
  return errorCodeToError(std::make_error_code(std::errc::function_not_supported));
}

// mapped_file_region stubs
int mapped_file_region::alignment() {
  // WASI stub: Return page size
  return 65536;
}

mapped_file_region::mapped_file_region(int fd, mapmode mode, size_t length,
                                        uint64_t offset, std::error_code &ec) {
  // WASI stub: Set error
  ec = std::make_error_code(std::errc::function_not_supported);
}

void mapped_file_region::dontNeedImpl() {
  // WASI stub: No-op
}

void mapped_file_region::unmapImpl() {
  // WASI stub: No-op
}

namespace detail {

std::error_code directory_iterator_construct(DirIterState &State,
                                               StringRef Path,
                                               bool FollowSymlinks) {
  // WASI stub: Not supported
  return std::make_error_code(std::errc::function_not_supported);
}

std::error_code directory_iterator_destruct(DirIterState &State) {
  // WASI stub: No-op
  return std::error_code();
}

std::error_code directory_iterator_increment(DirIterState &State) {
  // WASI stub: Not supported
  return std::make_error_code(std::errc::function_not_supported);
}

} // namespace detail

// fs::openFile stub
std::error_code openFile(const Twine &Name, int &ResultFD,
                          CreationDisposition Disp, FileAccess Access,
                          OpenFlags Flags, unsigned Mode) {
  // WASI stub: Not supported
  ResultFD = -1;
  return std::make_error_code(std::errc::function_not_supported);
}

} // namespace fs

// file_status member functions - these need to be outside the fs namespace
// because they're member functions of file_status class
fs::UniqueID fs::file_status::getUniqueID() const {
  // WASI stub: Return a dummy unique ID
  return fs::UniqueID(0, 0);
}

sys::TimePoint<> fs::basic_file_status::getLastModificationTime() const {
  // WASI stub: Return epoch time
  return sys::TimePoint<>();
}

ErrorOr<fs::basic_file_status> fs::directory_entry::status() const {
  // WASI stub: Return error
  return std::make_error_code(std::errc::function_not_supported);
}

} // namespace sys
} // namespace llvm

#endif // BINJI_HACK
