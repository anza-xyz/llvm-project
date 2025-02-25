; RUN: llc < %s -march=sbf | FileCheck %s
; RUN: llc < %s -march=sbf -mattr=+mem-encoding | FileCheck %s

define i16 @am1(i16* %a) nounwind {
  %1 = load i16, i16* %a
  ret i16 %1
}
; CHECK-LABEL: am1:
; CHECK: ldxh r0, [r1 + 0]

@foo = external global i16

define i16 @am2() nounwind {
  %1 = load i16, i16* @foo
  ret i16 %1
}
; CHECK-LABEL: am2:
; CHECK: ldxh r0, [r1 + 0]

define i16 @am4() nounwind {
  %1 = load volatile i16, i16* inttoptr(i16 32 to i16*)
  ret i16 %1
}
; CHECK-LABEL: am4:
; CHECK: mov64 r1, 32
; CHECK: ldxh r0, [r1 + 0]

define i16 @am5(i16* %a) nounwind {
  %1 = getelementptr i16, i16* %a, i16 2
  %2 = load i16, i16* %1
  ret i16 %2
}
; CHECK-LABEL: am5:
; CHECK: ldxh r0, [r1 + 4]

%S = type { i16, i16 }
@baz = common global %S zeroinitializer, align 1

define i16 @am6() nounwind {
  %1 = load i16, i16* getelementptr (%S, %S* @baz, i32 0, i32 1)
  ret i16 %1
}
; CHECK-LABEL: am6:
; CHECK: ldxh r0, [r1 + 2]
