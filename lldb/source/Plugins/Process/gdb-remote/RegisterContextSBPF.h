//===-- RegisterContextSBPF.h ----------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_GDB_REMOTE_REGISTERCONTEXTSBPF_H
#define LLDB_SOURCE_PLUGINS_PROCESS_GDB_REMOTE_REGISTERCONTEXTSBPF_H

#include "GDBRemoteRegisterContext.h"

namespace lldb_private::process_gdb_remote {

/// Register context for an interpreter-owned SBPF caller frame. The runtime
/// supplies the frame's PC and r10 value; other historical register values are
/// unavailable.
class RegisterContextSBPF : public RegisterContext {
public:
  RegisterContextSBPF(Thread &thread, uint32_t concrete_frame_idx,
                      GDBRemoteDynamicRegisterInfoSP register_info,
                      lldb::addr_t pc, lldb::addr_t frame_pointer);

  void InvalidateAllRegisters() override;
  size_t GetRegisterCount() override;
  const RegisterInfo *GetRegisterInfoAtIndex(size_t reg) override;
  size_t GetRegisterSetCount() override;
  const RegisterSet *GetRegisterSet(size_t reg_set) override;
  bool ReadRegister(const RegisterInfo *reg_info,
                    RegisterValue &value) override;
  bool WriteRegister(const RegisterInfo *reg_info,
                     const RegisterValue &value) override;
  bool ReadAllRegisterValues(lldb::WritableDataBufferSP &data_sp) override;
  bool WriteAllRegisterValues(const lldb::DataBufferSP &data_sp) override;
  uint32_t ConvertRegisterKindToRegisterNumber(lldb::RegisterKind kind,
                                               uint32_t num) override;

private:
  GDBRemoteDynamicRegisterInfoSP m_register_info;
  lldb::addr_t m_pc;
  lldb::addr_t m_frame_pointer;
};

} // namespace lldb_private::process_gdb_remote

#endif
