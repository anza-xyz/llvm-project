; RUN: opt -O2 -S < %s | FileCheck %s

; ModuleID = 'rust_test.ae01a02caa2566bf-cgu.0'
source_filename = "rust_test.ae01a02caa2566bf-cgu.0"
target datalayout = "e-m:e-p:64:64-i64:64-n32:64-S128"
target triple = "sbf"

@alloc_e813fda33c33e38665803c55f01a1a57 = private unnamed_addr constant <{ [3 x i8] }> <{ [3 x i8] c"One" }>, align 1
@alloc_da1d77dfe6c47b0702ad778dd22ebff8 = private unnamed_addr constant <{ [3 x i8] }> <{ [3 x i8] c"Two" }>, align 1
@alloc_460442126579dd15a4cc4f66a722a171 = private unnamed_addr constant <{ [5 x i8] }> <{ [5 x i8] c"Three" }>, align 1

; We do not want the relative lookup table to be here
; CHECK-NOT: private unnamed_addr constant [3 x i32] [i32 trunc (i64 sub (i64 ptrtoint
; CHECK: private unnamed_addr constant [3 x ptr]

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define { ptr, i64 } @do_this(i8 noundef range(i8 0, 3) %0) unnamed_addr #0 {
start:
  switch i8 %0, label %default.unreachable [
    i8 0, label %bb1
    i8 1, label %bb4
    i8 2, label %bb3
  ]

default.unreachable:                              ; preds = %start
  unreachable

bb4:                                              ; preds = %start
  br label %bb1

bb3:                                              ; preds = %start
  br label %bb1

bb1:                                              ; preds = %start, %bb3, %bb4
  %_0.sroa.0.0 = phi ptr [ @alloc_460442126579dd15a4cc4f66a722a171, %bb3 ], [ @alloc_da1d77dfe6c47b0702ad778dd22ebff8, %bb4 ], [ @alloc_e813fda33c33e38665803c55f01a1a57, %start ]
  %_0.sroa.4.0 = phi i64 [ 5, %bb3 ], [ 3, %bb4 ], [ 3, %start ]
  %1 = insertvalue { ptr, i64 } poison, ptr %_0.sroa.0.0, 0
  %2 = insertvalue { ptr, i64 } %1, i64 %_0.sroa.4.0, 1
  ret { ptr, i64 } %2
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) "target-cpu"="generic" "target-features"="+store-imm,+jmp-ext" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 8, !"PIC Level", i32 2}
!1 = !{!"rustc version 1.84.1-dev"}
