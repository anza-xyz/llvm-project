//===- SBFLegalizerInfo.h ----------------------------------------*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file declares the targeting of the Machinelegalizer class for SBF
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SBF_GISEL_SBFMACHINELEGALIZER_H
#define LLVM_LIB_TARGET_SBF_GISEL_SBFMACHINELEGALIZER_H

#include "llvm/CodeGen/GlobalISel/LegalizerInfo.h"

namespace llvm {

class SBFSubtarget;

/// This class provides the information for the SBF target legalizer for
/// GlobalISel.
class SBFLegalizerInfo : public LegalizerInfo {
public:
  SBFLegalizerInfo(const SBFSubtarget &ST);
};
} // namespace llvm
#endif
