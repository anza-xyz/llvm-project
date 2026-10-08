//===-- RegisterContextSBPF.cpp ------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "RegisterContextSBPF.h"
#include "lldb/Utility/RegisterValue.h"
#include <utility>

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::process_gdb_remote;

RegisterContextSBPF::RegisterContextSBPF(
    Thread &thread, uint32_t concrete_frame_idx,
    GDBRemoteDynamicRegisterInfoSP register_info, lldb::addr_t pc,
    lldb::addr_t frame_pointer)
    : RegisterContext(thread, concrete_frame_idx),
      m_register_info(std::move(register_info)), m_pc(pc),
      m_frame_pointer(frame_pointer) {}

void RegisterContextSBPF::InvalidateAllRegisters() {}

size_t RegisterContextSBPF::GetRegisterCount() {
  return m_register_info->GetNumRegisters();
}

const RegisterInfo *RegisterContextSBPF::GetRegisterInfoAtIndex(size_t reg) {
  return m_register_info->GetRegisterInfoAtIndex(reg);
}

size_t RegisterContextSBPF::GetRegisterSetCount() {
  return m_register_info->GetNumRegisterSets();
}

const RegisterSet *RegisterContextSBPF::GetRegisterSet(size_t reg_set) {
  return m_register_info->GetRegisterSet(reg_set);
}

bool RegisterContextSBPF::ReadRegister(const RegisterInfo *reg_info,
                                       RegisterValue &value) {
  if (!reg_info)
    return false;

  switch (reg_info->kinds[eRegisterKindGeneric]) {
  case LLDB_REGNUM_GENERIC_PC:
    return value.SetUInt(m_pc, reg_info->byte_size);
  case LLDB_REGNUM_GENERIC_FP:
    return value.SetUInt(m_frame_pointer, reg_info->byte_size);
  default:
    return false;
  }
}

bool RegisterContextSBPF::WriteRegister(const RegisterInfo *,
                                        const RegisterValue &) {
  return false;
}

bool RegisterContextSBPF::ReadAllRegisterValues(WritableDataBufferSP &) {
  return false;
}

bool RegisterContextSBPF::WriteAllRegisterValues(const DataBufferSP &) {
  return false;
}

uint32_t RegisterContextSBPF::ConvertRegisterKindToRegisterNumber(
    lldb::RegisterKind kind, uint32_t num) {
  return m_register_info->ConvertRegisterKindToRegisterNumber(kind, num);
}
