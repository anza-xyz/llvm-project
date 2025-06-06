; RUN: opt -march=sbf -O2 -S < %s | FileCheck %s

target datalayout = "e-m:e-p:64:64-i64:64-n32:64-S128"

@alloc_a095982df5ef6fa38482490385e9df9c = private unnamed_addr constant <{ [11 x i8] }> <{ [11 x i8] c"Invalid num" }>, align 1
@alloc_a85d045283da191190505c6c52c35e47 = private unnamed_addr constant <{ ptr, [8 x i8] }> <{ ptr @alloc_a095982df5ef6fa38482490385e9df9c, [8 x i8] c"\0B\00\00\00\00\00\00\00" }>, align 8
@alloc_f5ffd2fd1476bab43ad89fb40c72d0c5 = private unnamed_addr constant <{ [10 x i8] }> <{ [10 x i8] c"src/lib.rs" }>, align 1
@alloc_2c31b023ec05614e5709d58aa57c990b = private unnamed_addr constant <{ ptr, [16 x i8] }> <{ ptr @alloc_f5ffd2fd1476bab43ad89fb40c72d0c5, [16 x i8] c"\0A\00\00\00\00\00\00\00\0E\00\00\00\12\00\00\00" }>, align 8

@do_that = unnamed_addr alias i8 (i64), ptr @"_ZN68_$LT$rust_test..MyEnum$u20$as$u20$core..convert..From$LT$u64$GT$$GT$4from17hd15e70d09a9b53c5E"

; <rust_test::MyEnum as core::convert::From<u64>>::from
; Function Attrs: nounwind
define hidden noundef range(i8 0, 3) i8 @"_ZN68_$LT$rust_test..MyEnum$u20$as$u20$core..convert..From$LT$u64$GT$$GT$4from17hd15e70d09a9b53c5E"(i64 noundef %num) unnamed_addr {
start:
  %_3 = alloca [48 x i8], align 8
  switch i64 %num, label %bb1 [
    i64 1, label %bb5
    i64 2, label %bb3
    i64 3, label %bb2
  ]

; Verifies we are simplifying the switch. In this case, we should expect the index to be '%num - 1'
; CHECK-NOT: switch i64 %num
; CHECK: %switch.tableidx = add i64 %num, -1
; CHECK: %0 = icmp ult i64 %switch.tableidx, 3
; CHECK: br i1 %0, label %switch.lookup, label %bb1

bb1:                                              ; preds = %start
  call void @llvm.lifetime.start.p0(i64 48, ptr nonnull %_3)
  store ptr @alloc_a85d045283da191190505c6c52c35e47, ptr %_3, align 8
  %0 = getelementptr inbounds i8, ptr %_3, i64 8
  store i64 1, ptr %0, align 8
  %1 = getelementptr inbounds i8, ptr %_3, i64 32
  store ptr null, ptr %1, align 8
  %2 = getelementptr inbounds i8, ptr %_3, i64 16
  store ptr inttoptr (i64 8 to ptr), ptr %2, align 8
  %3 = getelementptr inbounds i8, ptr %_3, i64 24
  store i64 0, ptr %3, align 8
; call core::panicking::panic_fmt
  call void @_ZN4core9panicking9panic_fmt17ha4a6c4ba8141afc1E(ptr noalias nocapture noundef nonnull readonly align 8 dereferenceable(48) %_3, ptr noalias noundef nonnull readonly align 8 dereferenceable(24) @alloc_2c31b023ec05614e5709d58aa57c990b)
  unreachable

bb3:                                              ; preds = %start
  br label %bb5

bb2:                                              ; preds = %start
  br label %bb5

bb5:                                              ; preds = %start, %bb2, %bb3
  %_0.sroa.0.0 = phi i8 [ 2, %bb2 ], [ 1, %bb3 ], [ 0, %start ]
  ret i8 %_0.sroa.0.0
}

; core::panicking::panic_fmt
; Function Attrs: cold noinline noreturn nounwind
declare void @_ZN4core9panicking9panic_fmt17ha4a6c4ba8141afc1E(ptr noalias nocapture noundef readonly align 8 dereferenceable(48), ptr noalias noundef readonly align 8 dereferenceable(24)) unnamed_addr

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)