; RUN: llc -march=sbf < %s | FileCheck %s -check-prefixes=CHECK-SBF
; RUN: llc -march=sbf -mcpu=v2 -mattr=+alu32 < %s | FileCheck %s -check-prefixes=CHECK-SBFV2

; Function Attrs: norecurse nounwind readnone
define i32 @test(i32 %len) #0 {
  %1 = sdiv i32 %len, 15
; CHECK-SBF: call __divdi3
; CHECK-SBFV2: sdiv32 w0, 15
  ret i32 %1
}
