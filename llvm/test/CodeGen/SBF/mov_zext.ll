; RUN: llc -march=sbf -mcpu=v1 < %s | FileCheck --check-prefix=CHECK-V1 %s
; RUN: llc -march=sbf -mcpu=v2 < %s | FileCheck --check-prefix=CHECK-V2 %s

define dso_local i64 @mov_1() {
entry:
; CHECK-LABEL: mov_1
; CHECK-V1: lddw r0, 4294967294
; CHECK-V2: mov32 r0, -2
; CHECK-V2-NOT: hor64
  ret i64 4294967294
}

define dso_local i64 @mov_2() {
entry:
; CHECK-LABEL: mov_2
; CHECK-V1: lddw r0, -4294967294
; CHECK-V2: mov32 r0, 2
; CHECK-V2: hor64 r0, -1
  ret i64 -4294967294
}

define dso_local i64 @mov_3() {
entry:
; CHECK-LABEL: mov_3
; CHECK-V1: mov64 r0, 429496729
; CHECK-V2: mov64 r0, 429496729
  ret i64 429496729
}