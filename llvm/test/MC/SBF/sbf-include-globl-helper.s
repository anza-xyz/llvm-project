# Helper file for sbf-include-no-globl.s
# This file contains a .globl directive which should produce an error
# when included

.text
.globl helper_fn
helper_fn:
  mov64 r0, 0
  exit
