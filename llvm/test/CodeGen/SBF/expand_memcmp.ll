; RUN: opt -O2 -S < %s | FileCheck %s

target datalayout = "e-m:e-p:64:64-i64:64-n32:64-S128"
target triple = "sbf"

; Function Attrs: mustprogress nofree nounwind willreturn memory(argmem: read)
declare i32 @memcmp(ptr nocapture, ptr nocapture, i64) local_unnamed_addr

; The memcmp is expanded with an overlaping load
define i1 @yes_expand(ptr %a, ptr %b) {
entry:
    %res = call i32 @memcmp(ptr %a, ptr %b, i64 15)
    %is_zero = icmp eq i32 %res, 0
    ret i1 %is_zero

; CHECK: entry:
; CHECK: %0 = load i64, ptr %a, align 1
; CHECK: %1 = load i64, ptr %b, align 1
; CHECK: %.not = icmp eq i64 %0, %1
; CHECK: br i1 %.not, label %loadbb1, label %res_block

; CHECK: res_block:
; CHECK: br label %endblock

; CHECK: loadbb1:
; CHECK: %2 = getelementptr i8, ptr %a, i64 7
; CHECK: %3 = getelementptr i8, ptr %b, i64 7
; CHECK: %4 = load i64, ptr %2, align 1
; CHECK: %5 = load i64, ptr %3, align 1
; CHECK: %.not2 = icmp eq i64 %4, %5
; CHECK: br i1 %.not2, label %endblock, label %res_block

; CHECK: endblock:
; CHECK: %is_zero = phi i1 [ true, %loadbb1 ], [ false, %res_block ]
; CHECK: ret i1 %is_zero
}