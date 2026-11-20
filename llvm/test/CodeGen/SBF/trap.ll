; RUN: llc < %s -march=sbf --show-mc-encoding | FileCheck %s -check-prefixes=CHECK,DYNAMIC
; RUN: llc < %s -march=sbf -mcpu=v2 --show-mc-encoding | FileCheck %s -check-prefixes=CHECK,DYNAMIC
; RUN: llc < %s -march=sbf -mcpu=v3 -mattr=+static-syscalls --show-mc-encoding \
; RUN:     | FileCheck %s -check-prefixes=CHECK,STATIC
; RUN: llc < %s -march=sbf -mcpu=v3 -mattr=+static-syscalls -trap-unreachable --show-mc-encoding \
; RUN:     | FileCheck %s -check-prefixes=CHECK,STATIC,UNREACHABLE

; A trap is the `abort` syscall. Under static syscalls it is called by its key,
; murmur3_32("abort") = 0xb6fc1a11, as an immediate call with no symbol and no relocation.
; Before static syscalls a syscall is a relocated call to the undefined symbol, which the loader
; resolves; an immediate call there would be an internal call.

define void @trap() {
; CHECK-LABEL: trap:
; STATIC: call -1224992239 # encoding: [0x85,0x00,0x00,0x00,0x11,0x1a,0xfc,0xb6]
; DYNAMIC: call abort # encoding: [0x85'A',A,A,A,0x00,0x00,0x00,0x00]
; DYNAMIC-NEXT: # fixup A - offset: 0, value: abort, kind: FK_PCRel_4
  call void @llvm.trap()
  unreachable
}

define i64 @checked(i64 %x) {
; CHECK-LABEL: checked:
; UNREACHABLE: call -1224992239 # encoding: [0x85,0x00,0x00,0x00,0x11,0x1a,0xfc,0xb6]
  %bad = icmp eq i64 %x, 0
  br i1 %bad, label %die, label %ok
die:
  unreachable
ok:
  ret i64 %x
}

declare void @llvm.trap()
