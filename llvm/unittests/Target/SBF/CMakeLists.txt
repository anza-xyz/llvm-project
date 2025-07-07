include_directories(
        ${LLVM_MAIN_SRC_DIR}/lib/Target/SBF
        ${LLVM_BINARY_DIR}/lib/Target/SBF
)

set(LLVM_LINK_COMPONENTS
        SBFCodeGen
        SBFDesc
        SBFInfo
        Analysis
        CodeGen
        Core
        MC
        SelectionDAG
        TargetParser
)

add_llvm_target_unittest(SBFTests
        SBFInstrInfoTest.cpp
)
