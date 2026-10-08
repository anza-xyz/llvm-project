//===-- SBPFCallStack.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SBPFCallStack.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Endian.h"

using namespace lldb_private::process_gdb_remote;

llvm::Expected<std::vector<SBPFCallFrame>>
lldb_private::process_gdb_remote::ParseSBPFCallStack(llvm::StringRef response) {
  constexpr size_t frame_size = 2 * sizeof(uint64_t);
  std::string bytes;
  if (response.size() % 2 != 0 || !llvm::tryGetFromHex(response, bytes) ||
      bytes.empty() ||
      bytes.size() % frame_size != 0)
    return llvm::createStringError("invalid qSBPFCallStack response");

  std::vector<SBPFCallFrame> frames;
  for (size_t offset = 0; offset < bytes.size(); offset += frame_size) {
    const char *frame = bytes.data() + offset;
    frames.push_back(
        {llvm::support::endian::read64le(frame),
         llvm::support::endian::read64le(frame + sizeof(uint64_t))});
  }
  return frames;
}
