//===-- SBFInstrInfo.cpp - SBF Instruction Information ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the SBF implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "SBFInstrInfo.h"
#include "SBF.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>

#define GET_INSTRINFO_CTOR_DTOR
#include "SBFGenInstrInfo.inc"

using namespace llvm;

static inline bool isUncondBranchOpcode(int Opc) { return Opc == SBF::JMP; }

static inline bool isCondBranchOpcode(int Opc) {
  switch (Opc) {
  case SBF::JEQ_ri:
  case SBF::JEQ_rr:
  case SBF::JUGT_ri:
  case SBF::JUGT_rr:
  case SBF::JUGE_ri:
  case SBF::JUGE_rr:
  case SBF::JNE_ri:
  case SBF::JNE_rr:
  case SBF::JSGT_ri:
  case SBF::JSGT_rr:
  case SBF::JSGE_ri:
  case SBF::JSGE_rr:
  case SBF::JULT_ri:
  case SBF::JULT_rr:
  case SBF::JULE_ri:
  case SBF::JULE_rr:
  case SBF::JSLT_ri:
  case SBF::JSLT_rr:
  case SBF::JSLE_ri:
  case SBF::JSLE_rr:
    return true;
  default:
    return false;
  }
}

static void parseCondBranch(MachineInstr *LastInst, MachineBasicBlock *&Target,
                            SmallVectorImpl<MachineOperand> &Cond) {
  Cond.push_back(MachineOperand::CreateImm(LastInst->getOpcode()));
  Cond.push_back(LastInst->getOperand(0));
  Cond.push_back(LastInst->getOperand(1));
  Target = LastInst->getOperand(2).getMBB();
}

SBFInstrInfo::SBFInstrInfo()
    : SBFGenInstrInfo(SBF::ADJCALLSTACKDOWN, SBF::ADJCALLSTACKUP) {}

void SBFInstrInfo::initializeTargetFeatures(bool HasExplicitSext, bool NewMemEncoding) {
  this->HasExplicitSignExt = HasExplicitSext;
  this->NewMemEncoding = NewMemEncoding;
}

void SBFInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator I,
                               const DebugLoc &DL, MCRegister DestReg,
                               MCRegister SrcReg, bool KillSrc) const {
  if (SBF::GPRRegClass.contains(DestReg, SrcReg))
    BuildMI(MBB, I, DL, get(SBF::MOV_rr), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  else if (SBF::GPR32RegClass.contains(DestReg, SrcReg)) {
    unsigned OpCode =
        HasExplicitSignExt ? SBF::MOV_rr_32_no_sext_v2
                           : SBF::MOV_rr_32_no_sext_v1;
    BuildMI(MBB, I, DL, get(OpCode), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  }
  else
    llvm_unreachable("Impossible reg-to-reg copy");
}

void SBFInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator I,
                                       Register SrcReg, bool IsKill, int FI,
                                       const TargetRegisterClass *RC,
                                       const TargetRegisterInfo *TRI,
                                       Register VReg) const {
  DebugLoc DL;
  if (I != MBB.end())
    DL = I->getDebugLoc();

  if (RC == &SBF::GPRRegClass)
    BuildMI(MBB, I, DL, get(NewMemEncoding ?
                                           SBF::STD_V2 : SBF::STD_V1))
        .addReg(SrcReg, getKillRegState(IsKill))
        .addFrameIndex(FI)
        .addImm(0);
  else if (RC == &SBF::GPR32RegClass)
    BuildMI(MBB, I, DL, get(NewMemEncoding ?
                                           SBF::STW32_V2 : SBF::STW32_V1))
        .addReg(SrcReg, getKillRegState(IsKill))
        .addFrameIndex(FI)
        .addImm(0);
  else
    llvm_unreachable("Can't store this register to stack slot");
}

void SBFInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator I,
                                        Register DestReg, int FI,
                                        const TargetRegisterClass *RC,
                                        const TargetRegisterInfo *TRI,
                                        Register VReg) const {
  DebugLoc DL;
  if (I != MBB.end())
    DL = I->getDebugLoc();

  if (RC == &SBF::GPRRegClass)
    BuildMI(MBB, I, DL, get(NewMemEncoding ?
                                           SBF::LDD_V2 : SBF::LDD_V1),
            DestReg).addFrameIndex(FI).addImm(0);
  else if (RC == &SBF::GPR32RegClass)
    BuildMI(MBB, I, DL, get(NewMemEncoding ?
                                           SBF::LDW32_V2 : SBF::LDW32_V1),
            DestReg).addFrameIndex(FI).addImm(0);
  else
    llvm_unreachable("Can't load this register from stack slot");
}

bool SBFInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                 MachineBasicBlock *&TBB,
                                 MachineBasicBlock *&FBB,
                                 SmallVectorImpl<MachineOperand> &Cond,
                                 bool AllowModify) const {
  // If the block has no terminators, it just falls into the block after it.
  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
  if (I == MBB.end())
    return false;

  if (!isUnpredicatedTerminator(*I))
    return false;

  // Get the last instruction in the block.
  MachineInstr *LastInst = &*I;

  // If there is only one terminator instruction, process it.
  unsigned LastOpc = LastInst->getOpcode();
  if (I == MBB.begin() || !isUnpredicatedTerminator(*--I)) {
    if (isUncondBranchOpcode(LastOpc)) {
      TBB = LastInst->getOperand(0).getMBB();
      return false;
    }
    if (isCondBranchOpcode(LastOpc)) {
      // Block ends with fall-through condbranch.
      parseCondBranch(LastInst, TBB, Cond);
      return false;
    }
    return true; // Unknown case
  }

  // Get the instruction before it if it is a terminator.
  MachineInstr *SecondLastInst = &*I;
  unsigned SecondLastOpc = SecondLastInst->getOpcode();

  // If AllowModify is true and the block ends with two or more unconditional
  // branches, delete all but the first unconditional branch.
  if (AllowModify && isUncondBranchOpcode(LastOpc)) {
    while (isUncondBranchOpcode(SecondLastOpc)) {
      LastInst->eraseFromParent();
      LastInst = SecondLastInst;
      LastOpc = LastInst->getOpcode();
      if (I == MBB.begin() || !isUnpredicatedTerminator(*--I)) {
        // Return now the only terminator is an unconditional branch.
        TBB = LastInst->getOperand(0).getMBB();
        return false;
      }
      SecondLastInst = &*I;
      SecondLastOpc = SecondLastInst->getOpcode();
    }
  }

  // If we're allowed to modify and the block ends in a unconditional branch
  // which could simply fallthrough, remove the branch.  (Note: This case only
  // matters when we can't understand the whole sequence, otherwise it's also
  // handled by BranchFolding.cpp.)
  if (AllowModify && isUncondBranchOpcode(LastOpc) &&
      MBB.isLayoutSuccessor(getBranchDestBlock(*LastInst))) {
    LastInst->eraseFromParent();
    LastInst = SecondLastInst;
    LastOpc = LastInst->getOpcode();
    if (I == MBB.begin() || !isUnpredicatedTerminator(*--I)) {
      assert(!isUncondBranchOpcode(LastOpc) &&
             "unreachable unconditional branches removed above");

      if (isCondBranchOpcode(LastOpc)) {
        // Block ends with fall-through condbranch.
        parseCondBranch(LastInst, TBB, Cond);
        return false;
      }
      return true; // Can't handle indirect branch.
    }
    SecondLastInst = &*I;
    SecondLastOpc = SecondLastInst->getOpcode();
  }

  // If there are three terminators, we don't know what sort of block this is.
  if (SecondLastInst && I != MBB.begin() && isUnpredicatedTerminator(*--I))
    return true;

  // If the block ends with a conditional jump and a JA, handle it.
  if (isCondBranchOpcode(SecondLastOpc) && isUncondBranchOpcode(LastOpc)) {
    parseCondBranch(SecondLastInst, TBB, Cond);
    FBB = LastInst->getOperand(0).getMBB();
    return false;
  }

  // If the block ends with two unconditional branches, handle it.  The second
  // one is not executed, so remove it.
  if (isUncondBranchOpcode(SecondLastOpc) && isUncondBranchOpcode(LastOpc)) {
    TBB = SecondLastInst->getOperand(0).getMBB();
    I = LastInst;
    if (AllowModify)
      I->eraseFromParent();
    return false;
  }

  // Otherwise, can't handle this.
  return true;
}

unsigned SBFInstrInfo::insertBranch(MachineBasicBlock &MBB,
                                    MachineBasicBlock *TBB,
                                    MachineBasicBlock *FBB,
                                    ArrayRef<MachineOperand> Cond,
                                    const DebugLoc &DL, int *BytesAdded) const {
  assert(!BytesAdded && "code size not handled");

  // Shouldn't be a fall through.
  assert(TBB && "insertBranch must not be told to insert a fallthrough");

  if (BytesAdded)
    *BytesAdded = 8;

  if (Cond.empty()) {
    // Unconditional branch
    assert(!FBB && "Unconditional branch with multiple successors!");
    BuildMI(&MBB, DL, get(SBF::JMP)).addMBB(TBB);
    return 1;
  }

  // See the order we parse the jump information in `parseCondBranch`
  BuildMI(&MBB, DL, get(Cond[0].getImm()))
      .add(Cond[1])
      .add(Cond[2])
      .addMBB(TBB);

  if (FBB) {
    BuildMI(&MBB, DL, get(SBF::JMP)).addMBB(FBB);
    if (BytesAdded)
      *BytesAdded += 8;
  }

  return 1;
}

unsigned SBFInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                    int *BytesRemoved) const {
  assert(!BytesRemoved && "code size not handled");

  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
  if (I == MBB.end())
    return 0;

  if (!isUncondBranchOpcode(I->getOpcode()) &&
      !isCondBranchOpcode(I->getOpcode()))
    return 0;

  // Remove the branch.
  I->eraseFromParent();

  I = MBB.end();

  if (I == MBB.begin()) {
    if (BytesRemoved)
      *BytesRemoved = 8;
    return 1;
  }

  --I;
  if (!isCondBranchOpcode(I->getOpcode())) {
    if (BytesRemoved)
      *BytesRemoved = 8;
    return 1;
  }

  // Remove the branch.
  I->eraseFromParent();
  if (BytesRemoved)
    *BytesRemoved = 16;

  return 2;
}

bool SBFInstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  switch (Cond[0].getImm()) {
  default:
    llvm_unreachable("Unknown conditional branch!");
  case SBF::JEQ_ri:
    Cond[0].setImm(SBF::JNE_ri);
    break;
  case SBF::JEQ_rr:
    Cond[0].setImm(SBF::JNE_rr);
    break;
  case SBF::JUGT_ri:
    Cond[0].setImm(SBF::JULE_ri);
    break;
  case SBF::JUGT_rr:
    Cond[0].setImm(SBF::JULE_rr);
    break;
  case SBF::JUGE_ri:
    Cond[0].setImm(SBF::JULT_ri);
    break;
  case SBF::JUGE_rr:
    Cond[0].setImm(SBF::JULT_rr);
    break;
  case SBF::JNE_ri:
    Cond[0].setImm(SBF::JEQ_ri);
    break;
  case SBF::JNE_rr:
    Cond[0].setImm(SBF::JEQ_rr);
    break;
  case SBF::JSGT_ri:
    Cond[0].setImm(SBF::JSLE_ri);
    break;
  case SBF::JSGT_rr:
    Cond[0].setImm(SBF::JSLE_rr);
    break;
  case SBF::JSGE_ri:
    Cond[0].setImm(SBF::JSLT_ri);
    break;
  case SBF::JSGE_rr:
    Cond[0].setImm(SBF::JSLT_rr);
    break;
  case SBF::JULT_ri:
    Cond[0].setImm(SBF::JUGE_ri);
    break;
  case SBF::JULT_rr:
    Cond[0].setImm(SBF::JUGE_rr);
    break;
  case SBF::JULE_ri:
    Cond[0].setImm(SBF::JUGT_ri);
    break;
  case SBF::JULE_rr:
    Cond[0].setImm(SBF::JUGT_rr);
    break;
  case SBF::JSLT_ri:
    Cond[0].setImm(SBF::JSGE_ri);
    break;
  case SBF::JSLT_rr:
    Cond[0].setImm(SBF::JSGE_rr);
    break;
  case SBF::JSLE_ri:
    Cond[0].setImm(SBF::JSGT_ri);
    break;
  case SBF::JSLE_rr:
    Cond[0].setImm(SBF::JSGT_rr);
    break;
  }

  return false;
}

MachineBasicBlock *
SBFInstrInfo::getBranchDestBlock(const MachineInstr &MI) const {
  unsigned Opcode = MI.getOpcode();
  if (Opcode == SBF::JMP) {
    return MI.getOperand(0).getMBB();
  }

  if (isCondBranchOpcode(Opcode)) {
    return MI.getOperand(2).getMBB();
  }

  llvm_unreachable("unexpected opcode!");
}

unsigned SBFInstrInfo::getInstSizeInBytes(const MachineInstr &MI) const {
  if (MI.getOpcode() == SBF::LD_imm64)
    return 16;

  return 8;
}
