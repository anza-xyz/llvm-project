//===----- SBFMISimplifyPatchable.cpp - MI Simplify Patchable Insts -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass targets a subset of instructions like below
//    ld_imm64 r1, @global
//    ldd r2, r1, 0
//    add r3, struct_base_reg, r2
//
// Here @global should represent an AMA (abstruct member access).
// Such an access is subject to bpf load time patching. After this pass, the
// code becomes
//    ld_imm64 r1, @global
//    add r3, struct_base_reg, r1
//
// Eventually, at BTF output stage, a relocation record will be generated
// for ld_imm64 which should be replaced later by bpf loader:
//    r1 = <calculated field_info>
//    add r3, struct_base_reg, r1
//
// This pass also removes the intermediate load generated in IR pass for
// __builtin_btf_type_id() intrinsic.
//
//===----------------------------------------------------------------------===//

#include "SBF.h"
#include "SBFCORE.h"
#include "SBFInstrInfo.h"
#include "SBFTargetMachine.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/Debug.h"
#include <set>

using namespace llvm;

#define DEBUG_TYPE "sbf-mi-simplify-patchable"

namespace {

struct SBFMISimplifyPatchable : public MachineFunctionPass {

  static char ID;
  const SBFInstrInfo *TII;
  MachineFunction *MF;

  SBFMISimplifyPatchable() : MachineFunctionPass(ID) {
    initializeSBFMISimplifyPatchablePass(*PassRegistry::getPassRegistry());
  }

private:
  std::set<MachineInstr *> SkipInsts;

  // Initialize class variables.
  void initialize(MachineFunction &MFParm);

  bool isLoadInst(unsigned Opcode);
  bool removeLD();
  void processCandidate(MachineRegisterInfo *MRI, MachineBasicBlock &MBB,
                        MachineInstr &MI, Register &SrcReg, Register &DstReg,
                        const GlobalValue *GVal, bool IsAma);
  void processDstReg(MachineRegisterInfo *MRI, Register &DstReg,
                     Register &SrcReg, const GlobalValue *GVal,
                     bool doSrcRegProp, bool IsAma);
  void processInst(MachineRegisterInfo *MRI, MachineInstr *Inst,
                   MachineOperand *RelocOp, const GlobalValue *GVal);
  void checkADDrr(MachineRegisterInfo *MRI, MachineOperand *RelocOp,
                  const GlobalValue *GVal);
  void checkShift(MachineRegisterInfo *MRI, MachineBasicBlock &MBB,
                  MachineOperand *RelocOp, const GlobalValue *GVal,
                  unsigned Opcode);

public:
  // Main entry point for this pass.
  bool runOnMachineFunction(MachineFunction &MF) override {
    if (skipFunction(MF.getFunction()))
      return false;

    initialize(MF);
    return removeLD();
  }
};

// Initialize class variables.
void SBFMISimplifyPatchable::initialize(MachineFunction &MFParm) {
  MF = &MFParm;
  TII = MF->getSubtarget<SBFSubtarget>().getInstrInfo();
  LLVM_DEBUG(dbgs() << "*** SBF simplify patchable insts pass ***\n\n");
}

static bool isST(unsigned Opcode, bool NewEncoding) {
  if (NewEncoding) {
    return Opcode == SBF::STB_imm_V2 || Opcode == SBF::STH_imm_V2 ||
           Opcode == SBF::STW_imm_V2 || Opcode == SBF::STD_imm_V2;
  }

  return Opcode == SBF::STB_imm_V1 || Opcode == SBF::STH_imm_V1 ||
         Opcode == SBF::STW_imm_V1 || Opcode == SBF::STD_imm_V1;
}

static bool isSTX32(unsigned Opcode, bool NewEncoding) {
  if (NewEncoding) {
    return Opcode == SBF::STB32_V2 || Opcode == SBF::STH32_V2
           || Opcode == SBF::STW32_V2;
  }
  return Opcode == SBF::STB32_V1 || Opcode == SBF::STH32_V1
         || Opcode == SBF::STW32_V1;
}

static bool isSTX64(unsigned Opcode, bool NewEncoding) {
  if (NewEncoding) {
    return Opcode == SBF::STB_V2 || Opcode == SBF::STH_V2 ||
           Opcode == SBF::STW_V2 ||Opcode == SBF::STD_V2;
  }

  return Opcode == SBF::STB_V1 || Opcode == SBF::STH_V1 ||
         Opcode == SBF::STW_V1 ||Opcode == SBF::STD_V1;
}

static bool isLDX32(unsigned Opcode, bool NewEncoding) {
  if (NewEncoding)
    return Opcode == SBF::LDB32_V2 || Opcode == SBF::LDH32_V2 ||
           Opcode == SBF::LDW32_V2;

  return Opcode == SBF::LDB32_V1 || Opcode == SBF::LDH32_V1 ||
         Opcode == SBF::LDW32_V1;
}

static bool isLDX64(unsigned Opcode, bool NewEncoding) {
  if (NewEncoding)
    return Opcode == SBF::LDB_V2 || Opcode == SBF::LDH_V2 ||
           Opcode == SBF::LDW_V2 || Opcode == SBF::LDD_V2;

  return Opcode == SBF::LDB_V1 || Opcode == SBF::LDH_V1 ||
         Opcode == SBF::LDW_V1 || Opcode == SBF::LDD_V1;
}

bool SBFMISimplifyPatchable::isLoadInst(unsigned Opcode) {
  bool NewMemEncoding = MF->getSubtarget<SBFSubtarget>().getNewMemEncoding();
  return isLDX32(Opcode, NewMemEncoding) ||
         isLDX64(Opcode, NewMemEncoding);
}

void SBFMISimplifyPatchable::checkADDrr(MachineRegisterInfo *MRI,
    MachineOperand *RelocOp, const GlobalValue *GVal) {
  const MachineInstr *Inst = RelocOp->getParent();
  const MachineOperand *Op1 = &Inst->getOperand(1);
  const MachineOperand *Op2 = &Inst->getOperand(2);
  const MachineOperand *BaseOp = (RelocOp == Op1) ? Op2 : Op1;

  // Go through all uses of %1 as in %1 = ADD_rr %2, %3
  const SBFSubtarget& SubTarget = MF->getSubtarget<SBFSubtarget>();
  bool NewMemEncoding = SubTarget.getNewMemEncoding();
  const MachineOperand Op0 = Inst->getOperand(0);
  for (MachineOperand &MO :
       llvm::make_early_inc_range(MRI->use_operands(Op0.getReg()))) {
    // The candidate needs to have a unique definition.
    if (!MRI->getUniqueVRegDef(MO.getReg()))
      continue;

    MachineInstr *DefInst = MO.getParent();
    unsigned Opcode = DefInst->getOpcode();
    unsigned COREOp;
    if (isLDX64(Opcode, NewMemEncoding))
      COREOp = NewMemEncoding ? SBF::CORE_LD64_V2 : SBF::CORE_LD64_V1;
    else if (isLDX32(Opcode, NewMemEncoding))
      COREOp = NewMemEncoding ? SBF::CORE_LD32_V2 : SBF::CORE_LD32_V1;
    else if (isSTX64(Opcode, NewMemEncoding) ||
             isSTX32(Opcode, NewMemEncoding) ||
             isST(Opcode, NewMemEncoding))
      COREOp = NewMemEncoding ? SBF::CORE_ST_V2 : SBF::CORE_ST_V1;
    else
      continue;

    // It must be a form of %2 = *(type *)(%1 + 0) or *(type *)(%1 + 0) = %2.
    const MachineOperand &ImmOp = DefInst->getOperand(2);
    if (!ImmOp.isImm() || ImmOp.getImm() != 0)
      continue;

    // Reject the form:
    //   %1 = ADD_rr %2, %3
    //   *(type *)(%2 + 0) = %1
    if (isSTX64(Opcode, NewMemEncoding) ||
        isSTX32(Opcode, NewMemEncoding)) {
      const MachineOperand &Opnd = DefInst->getOperand(0);
      if (Opnd.isReg() && Opnd.getReg() == MO.getReg())
        continue;
    }

    BuildMI(*DefInst->getParent(), *DefInst, DefInst->getDebugLoc(), TII->get(COREOp))
        .add(DefInst->getOperand(0)).addImm(Opcode).add(*BaseOp)
        .addGlobalAddress(GVal);
    DefInst->eraseFromParent();
  }
}

void SBFMISimplifyPatchable::checkShift(MachineRegisterInfo *MRI,
    MachineBasicBlock &MBB, MachineOperand *RelocOp, const GlobalValue *GVal,
    unsigned Opcode) {
  // Relocation operand should be the operand #2.
  MachineInstr *Inst = RelocOp->getParent();
  if (RelocOp != &Inst->getOperand(2))
    return;

  BuildMI(MBB, *Inst, Inst->getDebugLoc(), TII->get(SBF::CORE_SHIFT))
      .add(Inst->getOperand(0)).addImm(Opcode)
      .add(Inst->getOperand(1)).addGlobalAddress(GVal);
  Inst->eraseFromParent();
}

void SBFMISimplifyPatchable::processCandidate(MachineRegisterInfo *MRI,
    MachineBasicBlock &MBB, MachineInstr &MI, Register &SrcReg,
    Register &DstReg, const GlobalValue *GVal, bool IsAma) {
  if (MRI->getRegClass(DstReg) == &SBF::GPR32RegClass) {
    if (IsAma) {
      // We can optimize such a pattern:
      //  %1:gpr = LD_imm64 @"llvm.s:0:4$0:2"
      //  %2:gpr32 = LDW32 %1:gpr, 0
      //  %3:gpr = SUBREG_TO_REG 0, %2:gpr32, %subreg.sub_32
      //  %4:gpr = ADD_rr %0:gpr, %3:gpr
      //  or similar patterns below for non-alu32 case.
      auto Begin = MRI->use_begin(DstReg), End = MRI->use_end();
      decltype(End) NextI;
      for (auto I = Begin; I != End; I = NextI) {
        NextI = std::next(I);
        if (!MRI->getUniqueVRegDef(I->getReg()))
          continue;

        unsigned Opcode = I->getParent()->getOpcode();
        if (Opcode == SBF::SUBREG_TO_REG) {
          Register TmpReg = I->getParent()->getOperand(0).getReg();
          processDstReg(MRI, TmpReg, DstReg, GVal, false, IsAma);
        }
      }
    }

    BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(SBF::COPY), DstReg)
        .addReg(SrcReg, 0, SBF::sub_32);
    return;
  }

  // All uses of DstReg replaced by SrcReg
  processDstReg(MRI, DstReg, SrcReg, GVal, true, IsAma);
}

void SBFMISimplifyPatchable::processDstReg(MachineRegisterInfo *MRI,
    Register &DstReg, Register &SrcReg, const GlobalValue *GVal,
    bool doSrcRegProp, bool IsAma) {
  auto Begin = MRI->use_begin(DstReg), End = MRI->use_end();
  decltype(End) NextI;
  for (auto I = Begin; I != End; I = NextI) {
    NextI = std::next(I);
    if (doSrcRegProp) {
      // In situations like below it is not known if usage is a kill
      // after setReg():
      //
      // .-> %2:gpr = LD_imm64 @"llvm.t:0:0$0:0"
      // |
      // |`----------------.
      // |   %3:gpr = LDD %2:gpr, 0
      // |   %4:gpr = ADD_rr %0:gpr(tied-def 0), killed %3:gpr <--- (1)
      // |   %5:gpr = LDD killed %4:gpr, 0       ^^^^^^^^^^^^^
      // |   STD killed %5:gpr, %1:gpr, 0         this is I
      //  `----------------.
      //     %6:gpr = LDD %2:gpr, 0
      //     %7:gpr = ADD_rr %0:gpr(tied-def 0), killed %6:gpr <--- (2)
      //     %8:gpr = LDD killed %7:gpr, 0       ^^^^^^^^^^^^^
      //     STD killed %8:gpr, %1:gpr, 0         this is I
      //
      // Instructions (1) and (2) would be updated by setReg() to:
      //
      //     ADD_rr %0:gpr(tied-def 0), %2:gpr
      //
      // %2:gpr is not killed at (1), so it is necessary to remove kill flag
      // from I.
      I->setReg(SrcReg);
      I->setIsKill(false);
    }

    // The candidate needs to have a unique definition.
    if (IsAma && MRI->getUniqueVRegDef(I->getReg()))
      processInst(MRI, I->getParent(), &*I, GVal);
  }
}

// Check to see whether we could do some optimization
// to attach relocation to downstream dependent instructions.
// Two kinds of patterns are recognized below:
// Pattern 1:
//   %1 = LD_imm64 @"llvm.b:0:4$0:1"  <== patch_imm = 4
//   %2 = LDD %1, 0  <== this insn will be removed
//   %3 = ADD_rr %0, %2
//   %4 = LDW[32] %3, 0 OR STW[32] %4, %3, 0
//   The `%4 = ...` will be transformed to
//      CORE_[ALU32_]MEM(%4, mem_opcode, %0, @"llvm.b:0:4$0:1")
//   and later on, BTF emit phase will translate to
//      %4 = LDW[32] %0, 4 STW[32] %4, %0, 4
//   and attach a relocation to it.
// Pattern 2:
//    %15 = LD_imm64 @"llvm.t:5:63$0:2" <== relocation type 5
//    %16 = LDD %15, 0   <== this insn will be removed
//    %17 = SRA_rr %14, %16
//    The `%17 = ...` will be transformed to
//       %17 = CORE_SHIFT(SRA_ri, %14, @"llvm.t:5:63$0:2")
//    and later on, BTF emit phase will translate to
//       %r4 = SRA_ri %r4, 63
void SBFMISimplifyPatchable::processInst(MachineRegisterInfo *MRI,
    MachineInstr *Inst, MachineOperand *RelocOp, const GlobalValue *GVal) {
  unsigned Opcode = Inst->getOpcode();
  if (isLoadInst(Opcode)) {
    SkipInsts.insert(Inst);
    return;
  }

  if (Opcode == SBF::ADD_rr)
    checkADDrr(MRI, RelocOp, GVal);
  else if (Opcode == SBF::SLL_rr)
    checkShift(MRI, *Inst->getParent(), RelocOp, GVal, SBF::SLL_ri);
  else if (Opcode == SBF::SRA_rr)
    checkShift(MRI, *Inst->getParent(), RelocOp, GVal, SBF::SRA_ri);
  else if (Opcode == SBF::SRL_rr)
    checkShift(MRI, *Inst->getParent(), RelocOp, GVal, SBF::SRL_ri);
}

/// Remove unneeded Load instructions.
bool SBFMISimplifyPatchable::removeLD() {
  MachineRegisterInfo *MRI = &MF->getRegInfo();
  MachineInstr *ToErase = nullptr;
  bool Changed = false;

  for (MachineBasicBlock &MBB : *MF) {
    for (MachineInstr &MI : MBB) {
      if (ToErase) {
        ToErase->eraseFromParent();
        ToErase = nullptr;
      }

      // Ensure the register format is LOAD <reg>, <reg>, 0
      if (!isLoadInst(MI.getOpcode()))
        continue;

      if (SkipInsts.find(&MI) != SkipInsts.end())
        continue;

      if (!MI.getOperand(0).isReg() || !MI.getOperand(1).isReg())
        continue;

      if (!MI.getOperand(2).isImm() || MI.getOperand(2).getImm())
        continue;

      Register DstReg = MI.getOperand(0).getReg();
      Register SrcReg = MI.getOperand(1).getReg();

      MachineInstr *DefInst = MRI->getUniqueVRegDef(SrcReg);
      if (!DefInst)
        continue;

      if (DefInst->getOpcode() != SBF::LD_imm64)
        continue;

      const MachineOperand &MO = DefInst->getOperand(1);
      if (!MO.isGlobal())
        continue;

      const GlobalValue *GVal = MO.getGlobal();
      auto *GVar = dyn_cast<GlobalVariable>(GVal);
      if (!GVar)
        continue;

      // Global variables representing structure offset or type id.
      bool IsAma = false;
      if (GVar->hasAttribute(SBFCoreSharedInfo::AmaAttr))
        IsAma = true;
      else if (!GVar->hasAttribute(SBFCoreSharedInfo::TypeIdAttr))
        continue;

      processCandidate(MRI, MBB, MI, SrcReg, DstReg, GVal, IsAma);

      ToErase = &MI;
      Changed = true;
    }
  }

  return Changed;
}

} // namespace

INITIALIZE_PASS(SBFMISimplifyPatchable, DEBUG_TYPE,
                "SBF PreEmit SimplifyPatchable", false, false)

char SBFMISimplifyPatchable::ID = 0;
FunctionPass *llvm::createSBFMISimplifyPatchablePass() {
  return new SBFMISimplifyPatchable();
}
