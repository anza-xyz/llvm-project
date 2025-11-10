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
#define BRANCH_INSTR(X)  \
  case SBF::X##_ri:      \
  case SBF::X##_rr:      \
  case SBF::X##_ri_32:   \
  case SBF::X##_rr_32:   \

  BRANCH_INSTR(JEQ)
  BRANCH_INSTR(JNE)
  BRANCH_INSTR(JUGT)
  BRANCH_INSTR(JUGE)
  BRANCH_INSTR(JSGT)
  BRANCH_INSTR(JSGE)
  BRANCH_INSTR(JULT)
  BRANCH_INSTR(JULE)
  BRANCH_INSTR(JSLT)
  BRANCH_INSTR(JSLE)
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
                               MCRegister SrcReg, bool KillSrc,
                               bool RenamableDest, bool RenamableSrc) const {
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
                                       Register VReg,
                                       MachineInstr::MIFlag Flags) const {
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

Register SBFInstrInfo::isStoreToStackSlot(const MachineInstr &MI,
                                          int &FrameIndex,
                                          unsigned &MemBytes) const {
  switch (MI.getOpcode()) {
  default:
    break;
  case SBF::STD_V2:
  case SBF::STD_V1:
    MemBytes = 8;
    if (MI.getOperand(0).isReg() && MI.getOperand(1).isFI() &&
        MI.getOperand(2).isImm() && MI.getOperand(2).getImm() == 0) {
      FrameIndex = MI.getOperand(1).getIndex();
      return MI.getOperand(0).getReg();
    }
    break;
  case SBF::STW32_V2:
  case SBF::STW32_V1:
    MemBytes = 4;
    if (MI.getOperand(0).isReg() && MI.getOperand(1).isFI() &&
        MI.getOperand(2).isImm() && MI.getOperand(2).getImm() == 0) {
      FrameIndex = MI.getOperand(1).getIndex();
      return MI.getOperand(0).getReg();
    }
    break;
  }

  return 0;
}

Register SBFInstrInfo::isStoreToStackSlot(const MachineInstr &MI,
                                          int &FrameIndex) const {
  unsigned MemBytes = 0;
  return isStoreToStackSlot(MI, FrameIndex, MemBytes);
}

void SBFInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator I,
                                        Register DestReg, int FI,
                                        const TargetRegisterClass *RC,
                                        const TargetRegisterInfo *TRI,
                                        Register VReg,
                                        MachineInstr::MIFlag Flags) const {
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

Register SBFInstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                           int &FrameIndex,
                                           unsigned &MemBytes) const {
  switch (MI.getOpcode()) {
  default:
    break;
  case SBF::LDD_V2:
  case SBF::LDD_V1:
    MemBytes = 8;
    if (MI.getOperand(0).isReg() && MI.getOperand(1).isFI() &&
        MI.getOperand(2).isImm() && MI.getOperand(2).getImm() == 0) {
      FrameIndex = MI.getOperand(1).getIndex();
      return MI.getOperand(0).getReg();
    }
    break;
  case SBF::LDW32_V2:
  case SBF::LDW32_V1:
    MemBytes = 4;
    if (MI.getOperand(0).isReg() && MI.getOperand(1).isFI() &&
        MI.getOperand(2).isImm() && MI.getOperand(2).getImm() == 0) {
      FrameIndex = MI.getOperand(1).getIndex();
      return MI.getOperand(0).getReg();
    }
    break;
  }

  return 0;
}

Register SBFInstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                           int &FrameIndex) const {
  unsigned MemBytes = 0;
  return isLoadFromStackSlot(MI, FrameIndex, MemBytes);
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

#define REVERSE_X_FOR_Y(X, Y)       \
  case SBF::X##_ri:                 \
    Cond[0].setImm(SBF::Y##_ri);    \
    break;                          \
  case SBF::X##_rr:                 \
    Cond[0].setImm(SBF::Y##_rr);    \
    break;                          \
  case SBF::X##_ri_32:              \
    Cond[0].setImm(SBF::Y##_ri_32); \
    break;                          \
  case SBF::X##_rr_32:              \
   Cond[0].setImm(SBF::Y##_rr_32);  \
   break;                           \

  REVERSE_X_FOR_Y(JEQ, JNE)
  REVERSE_X_FOR_Y(JNE, JEQ)
  REVERSE_X_FOR_Y(JUGT, JULE)
  REVERSE_X_FOR_Y(JUGE, JULT)
  REVERSE_X_FOR_Y(JSGT, JSLE)
  REVERSE_X_FOR_Y(JSGE, JSLT)
  REVERSE_X_FOR_Y(JULT, JUGE)
  REVERSE_X_FOR_Y(JULE, JUGT)
  REVERSE_X_FOR_Y(JSLT, JSGE)
  REVERSE_X_FOR_Y(JSLE, JSGT)
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

std::optional<RegImmPair> SBFInstrInfo::isAddImmediate(const MachineInstr &MI,
                                                       Register Reg) const {
  const MachineOperand &Op0 = MI.getOperand(0);
  if (!Op0.isReg() || Reg != Op0.getReg())
    return std::nullopt;

  if (!MI.getOperand(1).isReg() || !MI.getOperand(2).isImm())
    return std::nullopt;

  int Sign = 1;
  int64_t Offset = 0;
  unsigned Opcode = MI.getOpcode();
  switch (Opcode) {
  default:
    return std::nullopt;
  case SBF::SUB_ri:
  case SBF::SUB_ri_32:
    Sign *= -1;
    [[fallthrough]];
  case SBF::ADD_ri:
  case SBF::ADD_ri_32: {
    Offset = MI.getOperand(2).getImm() * Sign;
    return RegImmPair{MI.getOperand(1).getReg(), Offset};
  }
  }
}
