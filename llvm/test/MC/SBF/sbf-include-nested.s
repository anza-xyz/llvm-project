# RUN: llvm-mc %s -triple=sbf-solana-solana -filetype=obj -I %p -o %t.o
# RUN: llvm-objdump -d %t.o | FileCheck %s --check-prefix=CHECK-OBJ

# Test nested .include directive functionality for SBF

.include "sbf-include-nested-middle.s"

.text
.globl entrypoint
entrypoint:
  call custom_log
  exit

# CHECK-OBJ: lddw
# CHECK-OBJ: r1
# CHECK-OBJ: lddw
# CHECK-OBJ: r2, 0x7
# CHECK-OBJ: call
# CHECK-OBJ: exit
# CHECK-OBJ: <entrypoint>:
# CHECK-OBJ: call
# CHECK-OBJ: exit
