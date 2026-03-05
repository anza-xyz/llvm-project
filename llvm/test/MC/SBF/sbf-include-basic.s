# RUN: llvm-mc %s -triple=sbf-solana-solana -filetype=obj -I %p -o %t.o
# RUN: llvm-objdump -d %t.o | FileCheck %s --check-prefix=CHECK-OBJ

# Test basic .include directive functionality for SBF

.include "sbf-include-helper.s"

.text
.globl entrypoint
entrypoint:
  call custom_log
  exit

# CHECK-OBJ: lddw
# CHECK-OBJ: r1
# CHECK-OBJ: lddw
# CHECK-OBJ: r2, 0xe
# CHECK-OBJ: call
# CHECK-OBJ: exit
# CHECK-OBJ: <entrypoint>:
# CHECK-OBJ: call
# CHECK-OBJ: exit
