//===-- UnwindSBPF.cpp ----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "UnwindSBPF.h"
#include "RegisterContextSBPF.h"
#include "ThreadGDBRemote.h"
#include "lldb/Target/Process.h"
#include "lldb/Target/StackFrame.h"
#include "lldb/Target/UnwindLLDB.h"
#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/Log.h"

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::process_gdb_remote;

void UnwindSBPF::DoClear() {
  m_frames.clear();
  m_unwind_complete = false;
  m_fallback.reset();
}

uint32_t UnwindSBPF::DoGetFrameCount() {
  if (m_fallback)
    return m_fallback->GetFrameCount();
  if (m_unwind_complete)
    return m_frames.size();

  m_unwind_complete = true;
  ThreadGDBRemote &thread = static_cast<ThreadGDBRemote &>(GetThread());
  llvm::Expected<std::vector<SBPFCallFrame>> frames = thread.GetSBPFCallStack();
  if (!frames) {
    LLDB_LOG_ERROR(GetLog(LLDBLog::Unwind), frames.takeError(),
                   "SBPF remote unwinding unavailable: {0}");
    m_fallback = std::make_unique<UnwindLLDB>(GetThread());
    return m_fallback->GetFrameCount();
  }
  m_frames = std::move(*frames);
  return m_frames.size();
}

bool UnwindSBPF::DoGetFrameInfoAtIndex(uint32_t frame_idx, lldb::addr_t &cfa,
                                       lldb::addr_t &pc,
                                       bool &behaves_like_zeroth_frame) {
  if (!m_unwind_complete)
    DoGetFrameCount();
  if (m_fallback)
    return m_fallback->GetFrameInfoAtIndex(frame_idx, cfa, pc,
                                           behaves_like_zeroth_frame);
  if (frame_idx >= m_frames.size())
    return false;

  behaves_like_zeroth_frame = frame_idx == 0;
  pc = m_frames[frame_idx].pc;
  // The remote packet contains r10, not DWARF call frame information. Do not
  // expose a guest frame pointer (or an ordering key) as a fabricated CFA.
  cfa = LLDB_INVALID_ADDRESS;
  return true;
}

std::optional<uint32_t> UnwindSBPF::DoGetCallFrameDepth(uint32_t frame_idx) {
  if (!m_unwind_complete)
    DoGetFrameCount();
  if (m_fallback || frame_idx >= m_frames.size())
    return std::nullopt;
  return static_cast<uint32_t>(m_frames.size() - frame_idx - 1);
}

lldb::RegisterContextSP
UnwindSBPF::DoCreateRegisterContextForFrame(StackFrame *frame) {
  uint32_t frame_idx = frame->GetConcreteFrameIndex();
  if (!m_unwind_complete)
    DoGetFrameCount();
  if (m_fallback)
    return m_fallback->CreateRegisterContextForFrame(frame);
  if (frame_idx >= m_frames.size())
    return {};

  ThreadGDBRemote &thread = static_cast<ThreadGDBRemote &>(GetThread());
  return std::make_shared<RegisterContextSBPF>(
      thread, frame_idx, thread.GetDynamicRegisterInfo(),
      m_frames[frame_idx].pc, m_frames[frame_idx].frame_pointer);
}
