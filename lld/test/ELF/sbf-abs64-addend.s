# REQUIRES: sbf
## Check that R_SBF_64_ABS64 relocations honor the implicit addend stored in
## the relocated data. SBF uses REL relocations, so the addend for a `.quad
## sym + N` lives in the 8 bytes at the relocation offset and must be read back
## by SBF::getImplicitAddend(); otherwise it would be silently dropped and the
## linked value would be just `sym`.

# RUN: llvm-mc -filetype=obj -triple=sbf -mcpu=v3 %s -o %t.o
# RUN: llvm-readobj -r %t.o | FileCheck --check-prefix=RELOC %s
# RUN: ld.lld %t.o -o %t.out --section-start .data=0x1000 -e target
# RUN: llvm-readelf -x .data %t.out | FileCheck --check-prefix=DATA %s

## The input carries a single R_SBF_64_ABS64 relocation against `target`, with
## the addend 0x42 encoded in the data at the relocation offset (0x8).
# RELOC:      Relocations [
# RELOC-NEXT:   Section ({{.*}}) .rel.data {
# RELOC-NEXT:     0x8 R_SBF_64_ABS64 target
# RELOC-NEXT:   }
# RELOC-NEXT: ]

## .data is pinned at 0x1000, so `target` == 0x1000 and `ptr` must resolve to
## target + 0x42 == 0x1042 (little-endian: 42 10 ..). Without reading the
## implicit addend `ptr` would instead be just 0x1000 (00 10 ..).
# DATA:      Hex dump of section '.data':
# DATA-NEXT: 0x00001000 00000000 00000000 42100000 00000000

        .section .data,"aw"
        .globl target
target:
        .quad 0

        .globl ptr
ptr:
        .quad target + 0x42
