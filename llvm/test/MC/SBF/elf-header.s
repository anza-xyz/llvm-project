# RUN: llvm-mc %s -filetype=obj -triple=sbf-solana-solana --mcpu=v1 | llvm-readobj -h - \
# RUN:     | FileCheck --check-prefixes=CHECK,CHECK-V1 %s
# RUN: llvm-mc %s -filetype=obj -triple=sbf-solana-solana --mcpu=v2 | llvm-readobj -h - \
# RUN:     | FileCheck --check-prefixes=CHECK,CHECK-V1 %s
# RUN: llvm-mc %s -filetype=obj -triple=sbf-solana-solana --mcpu=v3 | llvm-readobj -h - \
# RUN:     | FileCheck --check-prefixes=CHECK,CHECK-V3 %s

# CHECK-V1:     Format: elf64-sbf
# CHECK-V3:     Format: elf64-bpf
# CHECK-V1:     Arch: sbf
# CHECK-V3:     Arch: bpf
# CHECK:     AddressSize: 64bit
# CHECK:     ElfHeader {
# CHECK:       Ident {
# CHECK:         Magic: (7F 45 4C 46)
# CHECK:         Class: 64-bit (0x2)
# CHECK:         DataEncoding: LittleEndian (0x1)
# CHECK:         FileVersion: 1
# CHECK:         OS/ABI: SystemV (0x0)
# CHECK:         ABIVersion: 0
# CHECK:       }
# CHECK:       Type: Relocatable (0x1)
# CHECK-V1:    Machine: EM_SBF (0x107)
# CHECK-V3:    Machine: EM_BPF (0xF7)
# CHECK:       Version: 1
