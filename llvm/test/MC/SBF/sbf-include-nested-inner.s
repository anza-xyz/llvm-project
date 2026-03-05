# Inner file for nested include test
# This file is included by sbf-include-nested-middle.s

custom_log:
    lddw r1, message
    lddw r2, 7
    call sol_log_
    exit

.rodata
    message: .ascii "Nested!"
