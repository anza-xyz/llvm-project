//===-- SBPFCallStack.h ----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_GDB_REMOTE_SBPFCALLSTACK_H
#define LLDB_SOURCE_PLUGINS_PROCESS_GDB_REMOTE_SBPFCALLSTACK_H

#include "lldb/lldb-types.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include <vector>

namespace lldb_private::process_gdb_remote {

struct SBPFCallFrame {
  lldb::addr_t pc;
  lldb::addr_t frame_pointer;
};

/// Parse the response to qSBPFCallStack. Each frame is a pair of little-endian
/// 64-bit PC and frame-pointer values, hex encoded without delimiters. Frames
/// are ordered from the active frame to the oldest caller. Caller PCs are
/// return PCs and use LLDB's normal non-zero-frame address adjustment.
llvm::Expected<std::vector<SBPFCallFrame>>
ParseSBPFCallStack(llvm::StringRef response);

} // namespace lldb_private::process_gdb_remote

#endif
