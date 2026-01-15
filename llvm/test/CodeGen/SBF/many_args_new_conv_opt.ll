; RUN: llc -march=sbf -mattr=+dynamic-frames-v3,+optimize-stack-space < %s | FileCheck --check-prefix=CHECK %s

; Function Attrs: nounwind uwtable
define i32 @caller_no_alloca(i32 %a, i32 %b, i32 %c) #0 {
entry:
; CHECK-LABEL: caller_no_alloca

; No changes to the stack pointer
; CHECK: add64 r10, -4096

; Saving arguments on the stack
; CHECK: stdw [r10 + 40], 60
; CHECK: stdw [r10 + 32], 55
; CHECK: stdw [r10 + 24], 50
; CHECK: stdw [r10 + 16], 4
; CHECK: stdw [r10 + 8], 3

; CHECK: mov64 r4, 1
; CHECK: mov64 r5, 2
; CHECK: call callee_alloca

  %call = tail call i32 @callee_alloca(i32 %a, i32 %b, i32 %c, i32 1, i32 2, i32 3, i32 4, i32 50, i32 55, i32 60) #3
  ret i32 %call
}

; Function Attrs: nounwind uwtable
define i32 @caller_alloca(i32 %a, i32 %b, i32 %c) #0 {
; CHECK-LABEL: caller_alloca
; CHECK: add64 r10, -896
; CHECK: ldxw r1, [r10 - 3144]
; 4096 - 896 + 56 = 3144

; Saving arguments in the callee's frame

; Offset in the callee: frame_size - 40
; CHECK-V3: stdw [r10 + 40], 60
; Offset in the callee: frame_size - 32
; CHECK-V3: stdw [r10 + 32], 55
; Offset in the callee: frame_size - 24
; CHECK-V3: stdw [r10 + 24], 50
; Offset in the callee: frame_size - 16
; CHECK-V3: stdw [r10 + 16], 4
; Offset in the callee: frame_size - 8
; CHECK-V3: stdw [r10 + 8], 3

; CHECK: mov64 r4, 1
; CHECK: mov64 r5, 2
; CHECK: call callee_no_alloca
; CHECK: ldxw r1, [r10 - 72]

entry:
  %g = alloca [3128 x i8], align 8
  %off = getelementptr i64, ptr %g, i64 7
  %g1 = load i32, ptr %off
  %call = tail call i32 @callee_no_alloca(i32 %g1, i32 %b, i32 %c, i32 1, i32 2, i32 3, i32 4, i32 50, i32 55, i32 60) #3
  %h = alloca i128
  %h1 = load i32, ptr %h
  %res = sub i32 %call, %h1
  ret i32 %res
}

; Function Attrs: nounwind uwtable
define i32 @callee_alloca(i32 %a, i32 %b, i32 %c, i32 %d, i32 %e, i32 %f, i32 %p, i32 %y, i32 %a1, i32 %a2) #1 {
; CHECK-LABEL: callee_alloca
; CHECK: add64 r10, -1024

; Loading arguments
; CHECK: ldxw r2, [r10 - 3064]
; CHECK: ldxw r2, [r10 - 3056]
; CHECK: ldxw r2, [r10 - 3048]
; CHECK: ldxw r2, [r10 - 3040]
; CHECK: ldxw r2, [r10 - 3032]
; Loading allocated i32
; CHECK-V3: ldxw r0, [r10 - 3024]


; CHECK-NOT: add64 r10, 128

entry:
  %o = alloca [3000 x i8], align 8
  %g = add i32 %a, %b
  %h = sub i32 %g, %c
  %i = add i32 %h, %d
  %j = sub i32 %i, %e
  %k = add i32 %j, %f
  %l = add i32 %k, %p
  %m = add i32 %l, %y
  %n = add i32 %m, %a1
  %q = add i32 %n, %a2
  %r = load i32, ptr %o
  %s = add i32 %r, %q
  ret i32 %s
}

; Function Attrs: nounwind uwtable
define i32 @callee_no_alloca(i32 %a, i32 %b, i32 %c, i32 %d, i32 %e, i32 %f, i32 %p, i32 %y, i32 %a1, i32 %a2) #1 {
; CHECK-LABEL: callee_no_alloca
; CHECK: add64 r10, -4032

; Loading arguments
; CHECK: ldxw r1, [r10 - 56]
; CHECK: ldxw r1, [r10 - 48]
; CHECK: ldxw r1, [r10 - 40]
; CHECK: ldxw r1, [r10 - 32]
; CHECK: ldxw r1, [r10 - 24]

; CHECK-NOT: add64 r10, 64
entry:
  %g = add i32 %a, %b
  %h = sub i32 %g, %c
  %i = add i32 %h, %d
  %j = sub i32 %i, %e
  %k = add i32 %j, %f
  %l = add i32 %k, %p
  %m = add i32 %l, %y
  %n = add i32 %m, %a1
  %q = add i32 %n, %a2
  ret i32 %q
}