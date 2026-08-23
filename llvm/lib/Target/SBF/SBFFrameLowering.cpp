//===-- SBFFrameLowering.cpp - SBF Frame Information ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the SBF implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "SBFFrameLowering.h"
#include "SBFFunctionInfo.h"
#include "SBFRegisterInfo.h"
#include "SBFSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"

using namespace llvm;

bool SBFFrameLowering::hasFPImpl(const MachineFunction &MF) const { return true; }

void SBFFrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  const SBFSubtarget& Subtarget = MF.getSubtarget<SBFSubtarget>();
  if (!Subtarget.getHasDynamicFrames()) {
    return;
  }

  MachineBasicBlock::iterator MBBI = MBB.begin();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const int NumBytes = -static_cast<int>(MFI.getStackSize());

  if (MBBI != MBB.end()) {
    const DebugLoc Dl = MBBI->getDebugLoc();
    const SBFInstrInfo &TII =
        *static_cast<const SBFInstrInfo *>(MF.getSubtarget().getInstrInfo());

    if (NumBytes)
      BuildMI(MBB, MBBI, Dl, TII.get(SBF::ADD_ri), SBF::R10)
          .addReg(SBF::R10)
          .addImm(NumBytes);
  }
}

void SBFFrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {}

void SBFFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                            BitVector &SavedRegs,
                                            RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  SavedRegs.reset(SBF::R6);
  SavedRegs.reset(SBF::R7);
  SavedRegs.reset(SBF::R8);
  SavedRegs.reset(SBF::R9);
}

StackOffset
SBFFrameLowering::getFrameIndexReference(const MachineFunction &MF, int FI,
                                         Register &FrameReg) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const SBFSubtarget &Subtarget = MF.getSubtarget<SBFSubtarget>();
  const SBFFunctionInfo *SBFFuncInfo = MF.getInfo<SBFFunctionInfo>();

  FrameReg = SBF::R10;

  // For SBPFv3+ the runtime auto-bumps R10 by FrameLength on each call so
  // that R10 ends up at the high end of the callee's frame slot.
  // SBFRegisterInfo::resolveInternalFrameIndex therefore emits stores as
  // `r10 + (Offset_FI - FrameLength)` (a negative displacement). Mirror that
  // adjustment in the DWARF location so DW_OP_fbreg + N names the same byte
  // the store wrote, instead of the default `Offset_FI + StackSize` which
  // resolves to an address in the next, uninitialized frame slot.
  //
  // The override fires only for v3+ (HasNoStackGaps && !HasDynamicFrames):
  //   - v1/v2 have HasDynamicFrames=true, so emitPrologue inserts an
  //     `add r10, -StackSize`, making R10 the low end of the frame; the
  //     default formula matches that.
  //   - v0 has HasNoStackGaps=false and short-circuits to the default branch
  //     below; its (gapped) layout is intentionally left untouched here.
  // Stack-passed argument frame indices (containsFrameIndex) also use the
  // default — their bytes live in the caller's frame, which needs a separate
  // adjustment.
  if (Subtarget.getHasNoStackGaps() && !Subtarget.getHasDynamicFrames() &&
      !SBFFuncInfo->containsFrameIndex(FI)) {
    return StackOffset::getFixed(MFI.getObjectOffset(FI) -
                                 static_cast<int>(SBFRegisterInfo::FrameLength));
  }

  return TargetFrameLowering::getFrameIndexReference(MF, FI, FrameReg);
}
