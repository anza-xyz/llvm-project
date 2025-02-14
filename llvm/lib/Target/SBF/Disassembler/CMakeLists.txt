add_llvm_component_library(LLVMSBFDisassembler
  SBFDisassembler.cpp

  LINK_COMPONENTS
  MCDisassembler
  MC
  SBFInfo
  Support

  ADD_TO_COMPONENT
  SBF
)

