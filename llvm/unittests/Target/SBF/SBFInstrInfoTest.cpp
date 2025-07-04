//===- SBFInstrInfoTest.cpp - SBFInstrInfo unit tests -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SBFInstrInfo.h"
#include "SBFSubtarget.h"
#include "SBFTargetMachine.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include "gtest/gtest.h"

#include <memory>

using namespace llvm;

namespace {

class SBFInstrInfoTest : public testing::TestWithParam<const char *> {
protected:
  std::unique_ptr<SBFTargetMachine> TM;
  std::unique_ptr<LLVMContext> Ctx;
  std::unique_ptr<SBFSubtarget> ST;
  std::unique_ptr<MachineModuleInfo> MMI;
  std::unique_ptr<MachineFunction> MF;
  std::unique_ptr<Module> M;

  static void SetUpTestSuite() {
    LLVMInitializeSBFTargetInfo();
    LLVMInitializeSBFTarget();
    LLVMInitializeSBFTargetMC();
  }

  SBFInstrInfoTest() {
    std::string Error;
    auto TT(Triple::normalize(GetParam()));
    const Target *TheTarget = TargetRegistry::lookupTarget(TT, Error);
    TargetOptions Options;

    TM.reset(static_cast<SBFTargetMachine *>(TheTarget->createTargetMachine(
        TT, "generic", "", Options, std::nullopt, std::nullopt,
        CodeGenOptLevel::Default)));

    Ctx = std::make_unique<LLVMContext>();
    M = std::make_unique<Module>("Module", *Ctx);
    M->setDataLayout(TM->createDataLayout());
    auto *FType = FunctionType::get(Type::getVoidTy(*Ctx), false);
    auto *F = Function::Create(FType, GlobalValue::ExternalLinkage, "Test", *M);
    MMI = std::make_unique<MachineModuleInfo>(TM.get());

    ST = std::make_unique<SBFSubtarget>(
        TM->getTargetTriple(), TM->getTargetCPU().str(),
        TM->getTargetFeatureString().str(), *TM);

    MF = std::make_unique<MachineFunction>(*F, *TM, *ST, 42, *MMI);
  }
};

TEST_P(SBFInstrInfoTest, IsAddImmediate) {
  const SBFInstrInfo *TII = ST->getInstrInfo();
  DebugLoc DL;

  MachineInstr *MI1 = BuildMI(*MF, DL, TII->get(SBF::ADD_ri_32), SBF::R1)
                          .addReg(SBF::R1)
                          .addImm(80)
                          .getInstr();
  auto MI1Res = TII->isAddImmediate(*MI1, SBF::R1);
  ASSERT_TRUE(MI1Res.has_value());
  EXPECT_EQ(MI1Res->Reg, SBF::R1);
  EXPECT_EQ(MI1Res->Imm, 80);

  MachineInstr *MI2 =
      BuildMI(*MF, DL, TII->get(SBF::ADD_ri_32), SBF::R1).addImm(80).getInstr();
  EXPECT_FALSE(TII->isAddImmediate(*MI2, SBF::R2));

  MachineInstr *MI3 = BuildMI(*MF, DL, TII->get(SBF::ADD_ri), SBF::R1)
                          .addReg(SBF::R1)
                          .addImm(80)
                          .getInstr();
  auto MI3Res = TII->isAddImmediate(*MI3, SBF::R1);
  ASSERT_TRUE(MI3Res.has_value());
  EXPECT_EQ(MI3Res->Reg, SBF::R1);
  EXPECT_EQ(MI3Res->Imm, 80);

  MachineInstr *MI4 = BuildMI(*MF, DL, TII->get(SBF::SUB_ri), SBF::R1)
                          .addReg(SBF::R1)
                          .addImm(80)
                          .getInstr();
  auto MI4Res = TII->isAddImmediate(*MI4, SBF::R1);
  ASSERT_TRUE(MI4Res.has_value());
  EXPECT_EQ(MI4Res->Reg, SBF::R1);
  EXPECT_EQ(MI4Res->Imm, -80);

  MachineInstr *MI5 = BuildMI(*MF, DL, TII->get(SBF::SUB_ri_32), SBF::R1)
                          .addReg(SBF::R1)
                          .addImm(80)
                          .getInstr();
  auto MI5Res = TII->isAddImmediate(*MI5, SBF::R1);
  ASSERT_TRUE(MI5Res.has_value());
  EXPECT_EQ(MI5Res->Reg, SBF::R1);
  EXPECT_EQ(MI5Res->Imm, -80);

  MachineInstr *MI6 = BuildMI(*MF, DL, TII->get(SBF::SUB_ri_32), SBF::R1)
                          .addImm(2)
                          .addImm(80)
                          .getInstr();
  auto MI6Res = TII->isAddImmediate(*MI6, SBF::R1);
  ASSERT_FALSE(MI6Res.has_value());

  MachineInstr *MI7 = BuildMI(*MF, DL, TII->get(SBF::SUB_ri_32), SBF::R1)
                          .addReg(SBF::R2)
                          .addReg(SBF::R3)
                          .getInstr();
  auto MI7Res = TII->isAddImmediate(*MI7, SBF::R1);
  ASSERT_FALSE(MI7Res.has_value());
}

} // namespace

INSTANTIATE_TEST_SUITE_P(SBFTest, SBFInstrInfoTest, testing::Values("sbf"));
