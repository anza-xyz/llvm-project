# RUN: not llvm-mc %s -triple=sbf-solana-solana -filetype=obj -I %p 2>&1 \
# RUN:     | FileCheck %s --check-prefix=CHECK

# Test that .globl in an included file produces an error for SBF

# CHECK: error:
# CHECK: .globl 'helper_fn' is not allowed in included files
# CHECK: Only the main entrypoint file should declare .globl symbols

.include "sbf-include-globl-helper.s"

.text
.globl entrypoint
entrypoint:
  call helper_fn
  exit
