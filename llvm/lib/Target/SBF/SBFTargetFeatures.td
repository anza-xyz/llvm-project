//===-- SBFTargetFeatures.td - Target Features for SBF Target ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the SBF target features in TableGen format.
//
//===----------------------------------------------------------------------===//

def ALU32 : SubtargetFeature<"alu32", "HasAlu32", "true",
                             "Enable ALU32 instructions">;

def DwarfRIS: SubtargetFeature<"dwarfris", "UseDwarfRIS", "true",
                               "Disable MCAsmInfo DwarfUsesRelocationsAcrossSections">;

def FeatureDynamicFrames : SubtargetFeature<"dynamic-frames", "HasDynamicFrames", "true",
                                            "Enable dynamic frames">;

def FeatureRelocAbs64 : SubtargetFeature<"reloc-abs64", "UseRelocAbs64", "true",
                                   "Fix 64bit data relocations">;

def FeatureStaticSyscalls : SubtargetFeature<"static-syscalls", "HasStaticSyscalls", "true",
                                   "Enable static syscalls">;

def FeatureDisableNeg : SubtargetFeature<"no-neg", "DisableNeg", "true",
                                    "Disable the neg instruction">;

def FeatureReverseSubImm : SubtargetFeature<"reverse-sub", "ReverseSubImm", "true",
                                    "Reverse the operands in the 'sub reg, imm' instruction">;

def FeatureDisableLddw : SubtargetFeature<"no-lddw", "NoLddw", "true",
                                    "Disable the lddw instruction">;

def FeatureCallxRegSrc : SubtargetFeature<"callx-reg-src", "CallxRegSrc", "true",
                                    "Encode Callx destination register in the src field">;

def FeatureCallxRegDst : SubtargetFeature<"callx-reg-dst", "CallxRegDst", "true",
                                    "Encode Callx destination register in the dst field">;

def FeaturePqrInstr : SubtargetFeature<"pqr-instr", "HasPqrClass", "true",
                                    "Enable the PQR instruction class">;

def FeatureExplicitSext : SubtargetFeature<"explicit-sext", "HasExplicitSignExt",
                    "true", "Enable the explicit sign extension instruction mov32">;

def FeatureNewMemEncoding : SubtargetFeature<"mem-encoding", "NewMemEncoding",
                                       "true", "Enable the new encoding for memory instructions">;

def FeatureAbiV2 : SubtargetFeature<"abi-v2", "IsAbiV2",
                                       "true", "Enable AbiV2 in SBF (no-op in LLVM)">;

def FeatureJmp32 : SubtargetFeature<"jmp-32", "HasJmp32", "true",
                                                "Enable the JMP32 instruction class">;

def FeatureDynamicFramesV3 : SubtargetFeature<"dynamic-frames-v3", "HasDynamicFramesV3", "true",
                                            "Enable dynamic frames in SBPFv3">;

def OptimizeStackSpace : SubtargetFeature<"optimize-stack-space", "OptimizeStackSpace", "true",
                                                                       "Optimize stack space usage in SBPFv3">;

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;

def : Proc<"v1", [FeatureDynamicFrames]>;

def : Proc<"v2", [FeatureDynamicFrames, FeatureDisableLddw,
                  FeatureNewMemEncoding, FeatureCallxRegSrc, FeaturePqrInstr, FeatureExplicitSext,
                  FeatureDisableNeg, FeatureReverseSubImm, ALU32]>;

def : Proc<"v3", [FeatureStaticSyscalls, FeatureRelocAbs64, FeatureJmp32,
                  FeatureDynamicFramesV3, FeatureCallxRegDst]>;

def : Proc<"v4", [FeatureStaticSyscalls, FeatureRelocAbs64, FeatureJmp32,
                  FeatureDynamicFramesV3, FeatureCallxRegDst]>;
