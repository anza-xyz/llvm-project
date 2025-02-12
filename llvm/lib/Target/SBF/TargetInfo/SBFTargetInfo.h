//===-- SBFTargetInfo.h - SBF Target Implementation -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "llvm/TargetParser/Triple.h"

#ifndef LLVM_LIB_TARGET_SBF_TARGETINFO_SBFTARGETINFO_H
#define LLVM_LIB_TARGET_SBF_TARGETINFO_SBFTARGETINFO_H

namespace llvm {

class Target;

Target &getTheSBFXTarget();
std::string cpuFromSubArch(const Triple &TT, const std::string &CPU);
} // namespace llvm

#endif // LLVM_LIB_TARGET_SBF_TARGETINFO_SBFTARGETINFO_H
