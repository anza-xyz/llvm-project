; RUN: llc < %s -march=sbf -mcpu=v2 | FileCheck %s


declare void @do_something(ptr, ptr, ptr, ptr, ptr);
declare void @test(ptr dead_on_unwind noalias nocapture noundef writable align 8 dereferenceable(24) %_0, ptr noalias noundef readonly align 1 dereferenceable(32) %pool_account_key, ptr noalias nocapture noundef readonly align 8 dereferenceable(48) %token_program, ptr noalias nocapture noundef readonly align 8 dereferenceable(48) %mint, ptr noalias nocapture noundef readonly align 8 dereferenceable(48) %destination, ptr noalias nocapture noundef readonly align 8 dereferenceable(48) %authority, i8 noundef %bump_seed, i64 noundef %amount);

define void @func() {
; CHECK-LABEL: func
   %185 = alloca [24 x i8], align 8
   %186 = alloca [48 x i8], align 8
   %187 = alloca [48 x i8], align 8
   %188 = alloca [48 x i8], align 8
   %189 = alloca [48 x i8], align 8

   call void @do_something(ptr %185, ptr %186, ptr %187, ptr %188, ptr %189)
   %200 = load ptr, ptr %185, align 8
   %bump_seed = load i8, ptr %188, align 1
   %other = load i64, ptr %187, align 8

; CHECK: ldxdw r2, [r10 + 232]
; CHECK: ldxb w1, [r10 + 88]
; CHECK: ldxdw r3, [r10 + 136]

; CHECK: stxdw [r10 - 24], r3

; The stxw [r10 - 12] store disappears if we use chained stores.

; CHECK: stxw [r10 - 12], w1
; CHECK: stxdw [r10 - 8], r7

   call void @test(ptr noalias nocapture noundef nonnull align 8 dereferenceable(24) %185, ptr noalias noundef nonnull readonly align 1 dereferenceable(32) %200, ptr noalias nocapture noundef nonnull align 8 dereferenceable(48) %186, ptr noalias nocapture noundef nonnull align 8 dereferenceable(48) %187, ptr noalias nocapture noundef nonnull align 8 dereferenceable(48) %188, ptr noalias nocapture noundef nonnull align 8 dereferenceable(48) %189, i8 noundef %bump_seed, i64 noundef %other)

   ret void
}