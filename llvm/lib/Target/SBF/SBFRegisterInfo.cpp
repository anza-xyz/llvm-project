//===-- SBFRegisterInfo.cpp - SBF Register Information ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the SBF implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "SBFFunctionInfo.h"
#include "SBFRegisterInfo.h"
#include "SBFSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_REGINFO_TARGET_DESC
#include "SBFGenRegisterInfo.inc"
using namespace llvm;

unsigned SBFRegisterInfo::FrameLength = 4096;

SBFRegisterInfo::SBFRegisterInfo()
    : SBFGenRegisterInfo(SBF::R0) {}

const MCPhysReg *
SBFRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_SaveList;
}

BitVector SBFRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  markSuperRegs(Reserved, SBF::W10); // [W|R]10 is read only frame pointer
  return Reserved;
}

static void warnSize(const int Offset, MachineFunction &MF,
                     const DebugLoc & DL, const bool StackGrowsUp)
{
  static Function *OldMF = nullptr;
  const int MaxOffset = -1 * SBFRegisterInfo::FrameLength;
  bool ShouldWarn = false;

  if (StackGrowsUp && Offset > 0) {
    ShouldWarn = true;
  } else if (!StackGrowsUp && Offset < MaxOffset) {
    ShouldWarn = true;
  }

  if (ShouldWarn) {
    if (&MF.getFunction() == OldMF) {
      return;
    }
    OldMF = &MF.getFunction();

    dbgs() << "Error:";
    if (DL) {
      dbgs() << " ";
      DL.print(dbgs());
    }
    const uint64_t StackSize = MF.getFrameInfo().getStackSize();
    const uint64_t Overflow =
        StackSize - static_cast<uint64_t>(SBFRegisterInfo::FrameLength);
    dbgs() << " Function " << MF.getFunction().getName()
           << " overflows the maximum allowed frame space by accessing "
           << "an offset " << Overflow << " bytes greater than the "
           << "maximum of " << SBFRegisterInfo::FrameLength
           << ". Please, minimize large stack variables. "
           << "Estimated function frame size: " << StackSize << " bytes."
           << " Exceeding the maximum stack offset may cause "
              "undefined behavior during execution.\n\n";
  }
}

bool SBFRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                          int SPAdj, unsigned FIOperandNum,
                                          RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected");

  unsigned i = 0;
  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  DebugLoc DL = MI.getDebugLoc();

  if (!DL)
    /* try harder to get some debug loc */
    for (auto &I : MBB)
      if (I.getDebugLoc()) {
        DL = I.getDebugLoc().getFnDebugLoc();
        break;
      }

  while (!MI.getOperand(i).isFI()) {
    ++i;
    assert(i < MI.getNumOperands() && "Instr doesn't have FrameIndex operand!");
  }

  Register FrameReg = getFrameRegister(MF);
  int FrameIndex = MI.getOperand(i).getIndex();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

  if (MI.getOpcode() == SBF::MOV_rr) {
    int Offset = resolveInternalFrameIndex(MF, FrameIndex, std::nullopt, DL);

    MI.getOperand(i).ChangeToRegister(FrameReg, false);
    Register reg = MI.getOperand(i - 1).getReg();
    BuildMI(MBB, ++II, DL, TII.get(SBF::ADD_ri), reg)
        .addReg(reg)
        .addImm(Offset);
    return false;
  }

  int Offset =
      resolveInternalFrameIndex(MF, FrameIndex,
                                MI.getOperand(i + 1).getImm(), DL);

  if (!isInt<32>(Offset))
    llvm_unreachable("bug in frame offset");

  if (MI.getOpcode() == SBF::FI_ri) {
    // architecture does not really support FI_ri, replace it with
    //    MOV_rr <target_reg>, frame_reg
    //    ADD_ri <target_reg>, imm
    Register reg = MI.getOperand(i - 1).getReg();

    BuildMI(MBB, ++II, DL, TII.get(SBF::MOV_rr), reg)
        .addReg(FrameReg);
    BuildMI(MBB, II, DL, TII.get(SBF::ADD_ri), reg)
        .addReg(reg)
        .addImm(Offset);

    // Remove FI_ri instruction
    MI.eraseFromParent();
  } else {
    MI.getOperand(i).ChangeToRegister(FrameReg, false);
    MI.getOperand(i + 1).ChangeToImmediate(Offset);
  }
  return false;
}

int SBFRegisterInfo::resolveInternalFrameIndex(llvm::MachineFunction &MF,
                                               int FI,
                                               std::optional<int64_t> Imm,
                                               const DebugLoc &DL) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const SBFFunctionInfo *SBFFuncInfo = MF.getInfo<SBFFunctionInfo>();
  int Offset = MFI.getObjectOffset(FI);
  const SBFSubtarget & SubTarget = MF.getSubtarget<SBFSubtarget>();
  const uint64_t StackSize = MFI.getStackSize();

  if (!SubTarget.getHasNoStackGaps() && SBFFuncInfo->containsFrameIndex(FI)) {
    Offset = SBFRegisterInfo::FrameLength - Offset;
    if (static_cast<uint64_t>(Offset) < StackSize) {
      dbgs() << "Error: A function call in method "
             << MF.getFunction().getName()
             << " overwrites values in the frame. Please, decrease stack usage "
             << "or remove parameters from the call. "
             << "The function call may cause undefined behavior "
                "during execution.\n\n";
    }
    return -Offset;
  }

  if (SubTarget.getHasNoStackGaps() && SBFFuncInfo->containsFrameIndex(FI)) {
    // When the stack grows up, the argument offset is off by the size of the
    // object because LLVM interprets that offset zero belongs to the caller,
    // not the callee.
    // PS: We have incremented it in fn LowerCall at SBFISelLowering.
    if (SubTarget.stackGrowsUp())
      return -(static_cast<int>(MFI.getObjectSize(FI)) + Offset);

    return -Offset;
  }

  Offset += Imm.value_or(0);

  if (SubTarget.getHasNoStackGaps()) {
    if (SubTarget.getHasDynamicFrames())
      return Offset + static_cast<int>(StackSize);

    const int V3Offset = Offset - static_cast<int>(FrameLength);
    warnSize(V3Offset, MF, DL, SubTarget.stackGrowsUp());
    return V3Offset;
  }

  // Only sBPFv0 will reach this stage, because it has stack gaps.
  warnSize(Offset, MF, DL, SubTarget.stackGrowsUp());
  return Offset;
}

Register SBFRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return SBF::R10;
}
