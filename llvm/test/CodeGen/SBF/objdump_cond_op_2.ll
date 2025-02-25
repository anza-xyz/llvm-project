; RUN: llc -march=sbf -mcpu=v1 -filetype=obj -o - %s | llvm-objdump -d - | FileCheck %s

; Source Code:
; int test(int a, int b) {
;   int s = 0;
;   while (a < b) { s++; a += s; b -= s; }
;   return s;
; }

define i32 @test(i32, i32) local_unnamed_addr #0 {
; CHECK-LABEL: <test>:
  %3 = icmp slt i32 %0, %1
  br i1 %3, label %4, label %13

; <label>:4:                                      ; preds = %2
  br label %5
; CHECK: jsge r4, r3, +0xa <LBB0_2>
; CHECK-LABEL: <LBB0_1>:

; <label>:5:                                      ; preds = %4, %5
  %6 = phi i32 [ %9, %5 ], [ 0, %4 ]
  %7 = phi i32 [ %11, %5 ], [ %1, %4 ]
  %8 = phi i32 [ %10, %5 ], [ %0, %4 ]
  %9 = add nuw nsw i32 %6, 1
  %10 = add nsw i32 %9, %8
  %11 = sub nsw i32 %7, %9
  %12 = icmp slt i32 %10, %11
  br i1 %12, label %5, label %13
; CHECK: mov64 r1, r3
; CHECK: jslt r3, r2, -0xa <LBB0_1>

; <label>:13:                                     ; preds = %5, %2
  %14 = phi i32 [ 0, %2 ], [ %9, %5 ]
  ret i32 %14
; CHECK-LABEL: <LBB0_2>:
; CHECK: exit
}
attributes #0 = { norecurse nounwind readnone }
