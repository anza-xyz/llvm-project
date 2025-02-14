//===-- SBFRegisterBankInfo.h -----------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file declares the targeting of the RegisterBankInfo class for SBF.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SBF_GISEL_SBFREGISTERBANKINFO_H
#define LLVM_LIB_TARGET_SBF_GISEL_SBFREGISTERBANKINFO_H

#include "MCTargetDesc/SBFMCTargetDesc.h"
#include "llvm/CodeGen/RegisterBankInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGBANK_DECLARATIONS
#include "SBFGenRegisterBank.inc"

namespace llvm {
class TargetRegisterInfo;

class SBFGenRegisterBankInfo : public RegisterBankInfo {
protected:
#define GET_TARGET_REGBANK_CLASS
#include "SBFGenRegisterBank.inc"
};

class SBFRegisterBankInfo final : public SBFGenRegisterBankInfo {
public:
  SBFRegisterBankInfo(const TargetRegisterInfo &TRI);
};
} // namespace llvm

#endif
