//===- BPFInstructionSelector.cpp --------------------------------*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements the targeting of the InstructionSelector class for BPF.
//===----------------------------------------------------------------------===//

#include "SBFInstrInfo.h"
#include "SBFSubtarget.h"
#include "SBFTargetMachine.h"
#include "SBFRegisterBankInfo.h"
#include "llvm/CodeGen/GlobalISel/GIMatchTableExecutorImpl.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelector.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/IR/IntrinsicsBPF.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "SBF-gisel"

using namespace llvm;

namespace {

#define GET_GLOBALISEL_PREDICATE_BITSET
#include "SBFGenGlobalISel.inc"
#undef GET_GLOBALISEL_PREDICATE_BITSET

class SBFInstructionSelector : public InstructionSelector {
public:
  SBFInstructionSelector(const SBFTargetMachine &TM, const SBFSubtarget &STI,
                         const SBFRegisterBankInfo &RBI);

  bool select(MachineInstr &I) override;
  static const char *getName() { return DEBUG_TYPE; }

private:
  /// tblgen generated 'select' implementation that is used as the initial
  /// selector for the patterns that do not require complex C++.
  bool selectImpl(MachineInstr &I, CodeGenCoverage &CoverageInfo) const;

  const SBFInstrInfo &TII;
  const SBFRegisterInfo &TRI;
  const SBFRegisterBankInfo &RBI;

#define GET_GLOBALISEL_PREDICATES_DECL
#include "SBFGenGlobalISel.inc"
#undef GET_GLOBALISEL_PREDICATES_DECL

#define GET_GLOBALISEL_TEMPORARIES_DECL
#include "SBFGenGlobalISel.inc"
#undef GET_GLOBALISEL_TEMPORARIES_DECL
};

} // namespace

#define GET_GLOBALISEL_IMPL
#include "SBFGenGlobalISel.inc"
#undef GET_GLOBALISEL_IMPL

SBFInstructionSelector::SBFInstructionSelector(const SBFTargetMachine &TM,
                                               const SBFSubtarget &STI,
                                               const SBFRegisterBankInfo &RBI)
    : TII(*STI.getInstrInfo()), TRI(*STI.getRegisterInfo()), RBI(RBI),
#define GET_GLOBALISEL_PREDICATES_INIT
#include "SBFGenGlobalISel.inc"
#undef GET_GLOBALISEL_PREDICATES_INIT
#define GET_GLOBALISEL_TEMPORARIES_INIT
#include "SBFGenGlobalISel.inc"
#undef GET_GLOBALISEL_TEMPORARIES_INIT
{
}

bool SBFInstructionSelector::select(MachineInstr &I) {
  if (!isPreISelGenericOpcode(I.getOpcode()))
    return true;
  if (selectImpl(I, *CoverageInfo))
    return true;
  return false;
}

namespace llvm {
InstructionSelector *
createSBFInstructionSelector(const SBFTargetMachine &TM,
                             const SBFSubtarget &Subtarget,
                             const SBFRegisterBankInfo &RBI) {
  return new SBFInstructionSelector(TM, Subtarget, RBI);
}
} // namespace llvm
