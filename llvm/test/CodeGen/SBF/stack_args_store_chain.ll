; RUN: llc < %s -march=sbf -mcpu=v2 | FileCheck %s


declare void @do_something(ptr, ptr, ptr, ptr, ptr);
declare void @test(ptr  %_0, ptr %pool_account_key, ptr %token_program, ptr %mint, ptr %destination, ptr %authority, i8 %bump_seed, i64 %amount);

define void @func() {
; CHECK-LABEL: func
   %1 = alloca [24 x i8], align 8
   %2 = alloca [48 x i8], align 8
   %3 = alloca [48 x i8], align 8
   %4 = alloca [48 x i8], align 8
   %5 = alloca [48 x i8], align 8

   call void @do_something(ptr %1, ptr %2, ptr %3, ptr %4, ptr %5)
   %6 = load ptr, ptr %1, align 8
   %bump_seed = load i8, ptr %4, align 1
   %other = load i64, ptr %3, align 8

; CHECK: ldxdw r2, [r10 + 232]
; CHECK: ldxb w1, [r10 + 88]
; CHECK: ldxdw r3, [r10 + 136]

; CHECK: stxdw [r10 - 24], r3

; The stxw [r10 - 12] store disappears if we use chained stores.

; CHECK: stxw [r10 - 12], w1
; CHECK: stxdw [r10 - 8], r7

   call void @test(ptr %1, ptr %6, ptr %2, ptr %3, ptr %4, ptr %5, i8 noundef %bump_seed, i64 noundef %other)

   ret void
}