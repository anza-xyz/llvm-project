# RUN: not llvm-mc %s -triple=sbf-solana-solana -filetype=obj -I %p 2>&1 \
# RUN:     | FileCheck %s --check-prefix=CHECK

# Test that including a non-existent file produces an error

# CHECK: error:
# CHECK: Could not find include file
# CHECK: nonexistent_file.s

.include "nonexistent_file.s"

.text
.globl entrypoint
entrypoint:
  exit
