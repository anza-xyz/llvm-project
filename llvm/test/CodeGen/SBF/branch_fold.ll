; RUN: llc -march=sbf -O3 < %s | FileCheck %s

; ModuleID = 'pinocchio.2fd090a2518ac939-cgu.0'
source_filename = "pinocchio.2fd090a2518ac939-cgu.0"
target datalayout = "e-m:e-p:64:64-i64:64-n32:64-S128"
target triple = "sbf"


; Function Attrs: nofree noinline norecurse nosync nounwind memory(readwrite, inaccessiblemem: none)
define void @deserialize(ptr dead_on_unwind noalias nocapture noundef writable writeonly sret([24 x i8]) align 8 dereferenceable(24) %_0, ptr noundef %0, ptr noundef %1) unnamed_addr {
start:
  %_4 = load i64, ptr %0, align 8
  %_7 = getelementptr inbounds i8, ptr %0, i64 8
  %_9.not = icmp eq i64 %_4, 0
  br i1 %_9.not, label %bb8, label %bb1

bb1:                                              ; preds = %start
  store ptr %_7, ptr %1, align 8
  %_16 = getelementptr inbounds i8, ptr %0, i64 10344
  %2 = getelementptr inbounds i8, ptr %0, i64 88
  %_21 = load i64, ptr %2, align 8
  %_18 = getelementptr inbounds i8, ptr %_16, i64 %_21
  %_24 = ptrtoint ptr %_18 to i64
  %_23 = add i64 %_24, 7
  %_22 = and i64 %_23, -8
  %3 = inttoptr i64 %_22 to ptr
  %to_process.sroa.0.05 = add i64 %_4, -1
  %_26.not6 = icmp eq i64 %to_process.sroa.0.05, 0
  br i1 %_26.not6, label %bb8, label %bb3

bb8:                                              ; preds = %bb2.backedge, %bb1, %start
  %input.sroa.0.0 = phi ptr [ %_7, %start ], [ %3, %bb1 ], [ %input.sroa.0.1.be, %bb2.backedge ]
  %_55 = load i64, ptr %input.sroa.0.0, align 8
  %_58 = getelementptr inbounds i8, ptr %input.sroa.0.0, i64 8
  %_65 = getelementptr inbounds i8, ptr %_58, i64 %_55
  store ptr %_65, ptr %_0, align 8
  %4 = getelementptr inbounds i8, ptr %_0, i64 8
  store ptr %_58, ptr %4, align 8
  %5 = getelementptr inbounds i8, ptr %_0, i64 16
  store i64 %_55, ptr %5, align 8
  ret void

; CHECK: LBB0_4:
; CHECK-NOT: jeq r4, 0, LBB0_7
; CHECK-NOT: ja LBB0_3
; CHECK: jne r4, 0, LBB0_4

bb3:                                              ; preds = %bb1, %bb2.backedge
  %to_process.sroa.0.09 = phi i64 [ %to_process.sroa.0.0, %bb2.backedge ], [ %to_process.sroa.0.05, %bb1 ]
  %input.sroa.0.18 = phi ptr [ %input.sroa.0.1.be, %bb2.backedge ], [ %3, %bb1 ]
  %accounts.sroa.0.07 = phi ptr [ %_28, %bb2.backedge ], [ %1, %bb1 ]
  %_28 = getelementptr inbounds i8, ptr %accounts.sroa.0.07, i64 8
  %_35 = load i8, ptr %input.sroa.0.18, align 8
  %_34.not = icmp eq i8 %_35, -1
  br i1 %_34.not, label %bb5, label %bb4

bb5:                                              ; preds = %bb3
  store ptr %input.sroa.0.18, ptr %_28, align 8
  %_44 = getelementptr inbounds i8, ptr %input.sroa.0.18, i64 10336
  %6 = getelementptr inbounds i8, ptr %input.sroa.0.18, i64 80
  %_49 = load i64, ptr %6, align 8
  %_46 = getelementptr inbounds i8, ptr %_44, i64 %_49
  %_52 = ptrtoint ptr %_46 to i64
  %_51 = add i64 %_52, 7
  %_50 = and i64 %_51, -8
  %7 = inttoptr i64 %_50 to ptr
  br label %bb2.backedge

bb2.backedge:                                     ; preds = %bb5, %bb4
  %input.sroa.0.1.be = phi ptr [ %_32, %bb4 ], [ %7, %bb5 ]
  %to_process.sroa.0.0 = add i64 %to_process.sroa.0.09, -1
  %_26.not = icmp eq i64 %to_process.sroa.0.0, 0
  br i1 %_26.not, label %bb8, label %bb3

bb4:                                              ; preds = %bb3
  %_32 = getelementptr inbounds i8, ptr %input.sroa.0.18, i64 8
  %_40 = zext i8 %_35 to i64
  %_39 = getelementptr inbounds ptr, ptr %1, i64 %_40
  store ptr %_39, ptr %_28, align 8
  br label %bb2.backedge
}

