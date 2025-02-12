//===-- SBFTargetInfo.cpp - SBF Target Implementation ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/SBFTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

Target &llvm::getTheSBFXTarget() {
  static Target TheSBFTarget;
  return TheSBFTarget;
}

std::string llvm::cpuFromSubArch(const Triple &TT, const std::string &CPU) {
  std::string CpuType;
  switch (TT.getSubArch()) {
  case Triple::SBFSubArch_v1:
    CpuType = "v1";
    break;
  case Triple::SBFSubArch_v2:
    CpuType = "v2";
    break;
  case Triple::SBFSubArch_v3:
    CpuType = "v3";
    break;
  default:
    break;
  }

  assert((CPU.empty() || CpuType.empty() || CPU == CpuType) &&
         "Subarch type must match CPU type");

  if (!CpuType.empty()) {
    return CpuType;
  }

  return CPU;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSBFTargetInfo() {
  TargetRegistry::RegisterTarget(
      getTheSBFXTarget(), "sbf", "SBF new (little endian)", "SBF",
      [](Triple::ArchType Aarch) { return Aarch == Triple::ArchType::sbf; },
      true);
  RegisterTarget<Triple::sbf, /*HasJIT=*/true> XX(
      getTheSBFXTarget(), "sbf", "SBF new (little endian)", "SBF");
}
