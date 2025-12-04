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
#include "SBFInstrInfo.h"
#include "SBFSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

bool SBFFrameLowering::hasFP(const MachineFunction &MF) const { return true; }

void SBFFrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  const SBFSubtarget& Subtarget = MF.getSubtarget<SBFSubtarget>();
  if (!Subtarget.getHasDynamicFrames()) {
    return;
  }

  MachineBasicBlock::iterator MBBI = MBB.begin();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  int NumBytes = (int)MFI.getStackSize();
  if (NumBytes && MBBI != MBB.end()) {
    DebugLoc Dl = MBBI->getDebugLoc();
    const SBFInstrInfo &TII =
        *static_cast<const SBFInstrInfo *>(MF.getSubtarget().getInstrInfo());

    if (Subtarget.isDynamicFramesV1())
      NumBytes = -NumBytes;
    else if (NumBytes <= FrameSize)
      // In V3, we don't bump if the number of bytes is less than the default
      // frame size.
      return;
    else
      NumBytes -= FrameSize;

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
