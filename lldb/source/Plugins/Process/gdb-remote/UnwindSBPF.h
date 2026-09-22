//===-- UnwindSBPF.h ------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_GDB_REMOTE_UNWINDSBPF_H
#define LLDB_SOURCE_PLUGINS_PROCESS_GDB_REMOTE_UNWINDSBPF_H

#include "SBPFCallStack.h"
#include "lldb/Target/Unwind.h"
#include <memory>

namespace lldb_private::process_gdb_remote {

class UnwindSBPF : public Unwind {
public:
  UnwindSBPF(Thread &thread) : Unwind(thread) {}

protected:
  void DoClear() override;
  uint32_t DoGetFrameCount() override;
  bool DoGetFrameInfoAtIndex(uint32_t frame_idx, lldb::addr_t &cfa,
                             lldb::addr_t &pc,
                             bool &behaves_like_zeroth_frame) override;
  lldb::RegisterContextSP
  DoCreateRegisterContextForFrame(StackFrame *frame) override;

private:
  std::vector<SBPFCallFrame> m_frames;
  std::unique_ptr<Unwind> m_fallback;
  bool m_unwind_complete = false;
};

} // namespace lldb_private::process_gdb_remote

#endif
