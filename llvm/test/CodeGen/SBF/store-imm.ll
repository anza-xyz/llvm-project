; RUN: llc < %s -march=sbf -mattr=+store-imm -show-mc-encoding | FileCheck %s
; RUN: llc < %s -march=sbf -mattr=+store-imm,+alu32 -show-mc-encoding | FileCheck %s
; RUN: llc < %s -march=sbf -mattr=+store-imm,+mem-encoding -show-mc-encoding | FileCheck --check-prefix=CHECK-V2 %s
; RUN: llc -march=sbf -mcpu=v3 -filetype=obj -o - %s | llvm-objdump -d --no-print-imm-hex - | FileCheck --check-prefix=CHECK-DUMP %s

define void @byte(ptr %p0) {
; CHECK-LABEL: byte:
; CHECK:       # %bb.0:
; CHECK-NEXT:    stb [r1 + 0], 1       # encoding: [0x72,0x01,0x00,0x00,0x01,0x00,0x00,0x00]
; CHECK-NEXT:    stb [r1 + 1], 255     # encoding: [0x72,0x01,0x01,0x00,0xff,0x00,0x00,0x00]

; CHECK-V2:    stb [r1 + 0], 1       # encoding: [0x27,0x01,0x00,0x00,0x01,0x00,0x00,0x00]
; CHECK-V2:    stb [r1 + 1], 255     # encoding: [0x27,0x01,0x01,0x00,0xff,0x00,0x00,0x00]

; CHECK-DUMP: stb [r1 + 0], 1
; CHECK-DUMP: stb [r1 + 1], 255

  %p1 = getelementptr i8, ptr %p0, i32 1
  store volatile i8  1, ptr %p0, align 1
  store volatile i8 -1, ptr %p1, align 1
  unreachable
}
define void @half(ptr, ptr %p0) {
; CHECK-LABEL: half:
; CHECK:       # %bb.0:
; CHECK-NEXT:    sth [r2 + 0], 1          # encoding: [0x6a,0x02,0x00,0x00,0x01,0x00,0x00,0x00]
; CHECK-NEXT:    sth [r2 + 2], 65535      # encoding: [0x6a,0x02,0x02,0x00,0xff,0xff,0x00,0x00]

; CHECK-V2:    sth [r2 + 0], 1          # encoding: [0x37,0x02,0x00,0x00,0x01,0x00,0x00,0x00]
; CHECK-V2:    sth [r2 + 2], 65535      # encoding: [0x37,0x02,0x02,0x00,0xff,0xff,0x00,0x00]

; CHECK-DUMP:    sth [r2 + 0], 1
; CHECK-DUMP:    sth [r2 + 2], 65535

  %p1 = getelementptr i8, ptr %p0, i32 2
  store volatile i16  1, ptr %p0, align 2
  store volatile i16 -1, ptr %p1, align 2
  unreachable
}
define void @word(ptr, ptr, ptr %p0) {
; CHECK-LABEL: word:
; CHECK:       # %bb.0:
; CHECK-NEXT:    stw [r3 + 0], 1            # encoding: [0x62,0x03,0x00,0x00,0x01,0x00,0x00,0x00]
; CHECK-NEXT:    stw [r3 + 4], -1           # encoding: [0x62,0x03,0x04,0x00,0xff,0xff,0xff,0xff]
; CHECK-NEXT:    stw [r3 + 8], -2000000000  # encoding: [0x62,0x03,0x08,0x00,0x00,0x6c,0xca,0x88]
; CHECK-NEXT:    stw [r3 + 12], -1          # encoding: [0x62,0x03,0x0c,0x00,0xff,0xff,0xff,0xff]
; CHECK-NEXT:    stw [r3 + 12], 0           # encoding: [0x62,0x03,0x0c,0x00,0x00,0x00,0x00,0x00]

; CHECK-V2:    stw [r3 + 0], 1            # encoding: [0x87,0x03,0x00,0x00,0x01,0x00,0x00,0x00]
; CHECK-V2:    stw [r3 + 4], -1           # encoding: [0x87,0x03,0x04,0x00,0xff,0xff,0xff,0xff]
; CHECK-V2:    stw [r3 + 8], -2000000000  # encoding: [0x87,0x03,0x08,0x00,0x00,0x6c,0xca,0x88]
; CHECK-V2:    stw [r3 + 12], -1          # encoding: [0x87,0x03,0x0c,0x00,0xff,0xff,0xff,0xff]
; CHECK-V2:    stw [r3 + 12], 0           # encoding: [0x87,0x03,0x0c,0x00,0x00,0x00,0x00,0x00]

; CHECK-DUMP:    stw [r3 + 0], 1
; CHECK-DUMP:    stw [r3 + 4], -1
; CHECK-DUMP:    stw [r3 + 8], -2000000000
; CHECK-DUMP:    stw [r3 + 12], -1
; CHECK-DUMP:    stw [r3 + 12], 0

  %p1 = getelementptr i8, ptr %p0, i32 4
  %p2 = getelementptr i8, ptr %p0, i32 8
  %p3 = getelementptr i8, ptr %p0, i32 12
  store volatile i32           1, ptr %p0, align 4
  store volatile i32          -1, ptr %p1, align 4
  store volatile i32 -2000000000, ptr %p2, align 4
  store volatile i32  4294967295, ptr %p3, align 4
  store volatile i32  4294967296, ptr %p3, align 4
  unreachable
}
define void @dword(ptr, ptr, ptr, ptr %p0) {
; CHECK-LABEL: dword:
; CHECK:       # %bb.0:
; CHECK-NEXT:    stdw [r4 + 0], 1             # encoding: [0x7a,0x04,0x00,0x00,0x01,0x00,0x00,0x00]
; CHECK-NEXT:    stdw [r4 + 8], -1            # encoding: [0x7a,0x04,0x08,0x00,0xff,0xff,0xff,0xff]
; CHECK-NEXT:    stdw [r4 + 16], 2000000000   # encoding: [0x7a,0x04,0x10,0x00,0x00,0x94,0x35,0x77]
; CHECK-NEXT:    stdw [r4 + 16], -2000000000  # encoding: [0x7a,0x04,0x10,0x00,0x00,0x6c,0xca,0x88]
; CHECK-NEXT:    lddw r1, 4294967295          # encoding: [0x18,0x01,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00]
; CHECK-NEXT:    stxdw [r4 + 24], r1          # encoding: [0x7b,0x14,0x18,0x00,0x00,0x00,0x00,0x00]

; CHECK-V2:    stdw [r4 + 0], 1             # encoding: [0x97,0x04,0x00,0x00,0x01,0x00,0x00,0x00]
; CHECK-V2:    stdw [r4 + 8], -1            # encoding: [0x97,0x04,0x08,0x00,0xff,0xff,0xff,0xff]
; CHECK-V2:    stdw [r4 + 16], 2000000000   # encoding: [0x97,0x04,0x10,0x00,0x00,0x94,0x35,0x77]
; CHECK-V2:    stdw [r4 + 16], -2000000000  # encoding: [0x97,0x04,0x10,0x00,0x00,0x6c,0xca,0x88]
; CHECK-V2:    lddw r1, 4294967295          # encoding: [0x18,0x01,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00]
; CHECK-V2:    stxdw [r4 + 24], r1          # encoding: [0x9f,0x14,0x18,0x00,0x00,0x00,0x00,0x00]

; CHECK-DUMP:    stdw [r4 + 0], 1
; CHECK-DUMP:    stdw [r4 + 8], -1
; CHECK-DUMP:    stdw [r4 + 16], 2000000000
; CHECK-DUMP:    stdw [r4 + 16], -2000000000

  %p1 = getelementptr i8, ptr %p0, i32 8
  %p2 = getelementptr i8, ptr %p0, i32 16
  %p3 = getelementptr i8, ptr %p0, i32 24
  store volatile i64           1, ptr %p0, align 8
  store volatile i64          -1, ptr %p1, align 8
  store volatile i64  2000000000, ptr %p2, align 8
  store volatile i64 -2000000000, ptr %p2, align 8
  store volatile i64  4294967295, ptr %p3, align 8
  unreachable
}

define void @unaligned(ptr %p0) {
; SBF allows misaligned memory accesses
; CHECK-LABEL: unaligned:
; CHECK:       # %bb.0:
; CHECK-NEXT:  sth [r1 + 0], 65534          # encoding: [0x6a,0x01,0x00,0x00,0xfe,0xff,0x00,0x00]
; CHECK-NEXT:  stw [r1 + 8], -2             # encoding: [0x62,0x01,0x08,0x00,0xfe,0xff,0xff,0xff]
; CHECK-NEXT:  stdw [r1 + 16], -2           # encoding: [0x7a,0x01,0x10,0x00,0xfe,0xff,0xff,0xff]

; CHECK-V2:  sth [r1 + 0], 65534          # encoding: [0x37,0x01,0x00,0x00,0xfe,0xff,0x00,0x00]
; CHECK-V2:  stw [r1 + 8], -2             # encoding: [0x87,0x01,0x08,0x00,0xfe,0xff,0xff,0xff]
; CHECK-V2:  stdw [r1 + 16], -2           # encoding: [0x97,0x01,0x10,0x00,0xfe,0xff,0xff,0xff]
  %p1 = getelementptr i8, ptr %p0, i32 8
  %p2 = getelementptr i8, ptr %p0, i32 16
  store volatile i16 -2, ptr %p0, align 1
  store volatile i32 -2, ptr %p1, align 2
  store volatile i64 -2, ptr %p2, align 4
  unreachable
}