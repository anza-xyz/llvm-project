add_llvm_component_group(SBF)

set(LLVM_TARGET_DEFINITIONS SBF.td)

tablegen(LLVM SBFGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM SBFGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM SBFGenCallingConv.inc -gen-callingconv)
tablegen(LLVM SBFGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM SBFGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM SBFGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM SBFGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM SBFGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM SBFGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM SBFGenGlobalISel.inc -gen-global-isel)
tablegen(LLVM SBFGenRegisterBank.inc -gen-register-bank)

add_public_tablegen_target(SBFCommonTableGen)

add_llvm_target(SBFCodeGen
  GISel/SBFCallLowering.cpp
  GISel/SBFInstructionSelector.cpp
  GISel/SBFRegisterBankInfo.cpp
  GISel/SBFLegalizerInfo.cpp
  SBFAbstractMemberAccess.cpp
  SBFAsmPrinter.cpp
  SBFCheckAndAdjustIR.cpp
  SBFFrameLowering.cpp
  SBFFunctionInfo.cpp
  SBFInstrInfo.cpp
  SBFIRPeephole.cpp
  SBFISelDAGToDAG.cpp
  SBFISelLowering.cpp
  SBFMCInstLower.cpp
  SBFPreserveDIType.cpp
  SBFRegisterInfo.cpp
  SBFSubtarget.cpp
  SBFTargetMachine.cpp
  SBFMIPeephole.cpp
  SBFMIChecking.cpp
  SBFMISimplifyPatchable.cpp
  BTFDebug.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  GlobalISel
  MC
  SBFDesc
  SBFInfo
  IPO
  Passes
  Scalar
  SelectionDAG
  Support
  Target
  TargetParser
  TransformUtils

  ADD_TO_COMPONENT
  SBF
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
