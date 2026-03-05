# Helper file for sbf-include-basic.s
# This file defines a simple function that can be included

custom_log:
    lddw r1, message
    lddw r2, 14
    call sol_log_
    exit

.rodata
    message: .ascii "Hello, Solana!"
