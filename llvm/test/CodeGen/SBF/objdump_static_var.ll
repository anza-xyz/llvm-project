; RUN: llc -march=sbf -mcpu=v2 -filetype=obj -o - %s | llvm-objdump -dr - | FileCheck --check-prefix=CHECK %s

; src:
;   static volatile long a = 2;
;   static volatile int b = 3;
;   int test() { return a + b; }
@a = internal global i64 2, align 8
@b = internal global i32 3, align 4

; Function Attrs: norecurse nounwind
define dso_local i32 @test() local_unnamed_addr #0 {
  %1 = load volatile i64, i64* @a, align 8, !tbaa !2
; CHECK: mov32 w1, 0x0
; CHECK: R_SBF_64_64	a
; CHECK: hor64 r1, 0x0
; CHECK: ldxdw r1, [r1 + 0x0]
  %2 = load volatile i32, i32* @b, align 4, !tbaa !6
; CHECK: mov32 w2, 0x0
; CHECK: R_SBF_64_64	b
; CHECK: hor64 r2, 0x0
; CHECK: ldxw w0, [r2 + 0x0]
  %3 = trunc i64 %1 to i32
  %4 = add i32 %2, %3
; CHECK: add32 w0, w1
  ret i32 %4
; CHECK: exit
}

attributes #0 = { norecurse nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.20181009 "}
!2 = !{!3, !3, i64 0}
!3 = !{!"long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
