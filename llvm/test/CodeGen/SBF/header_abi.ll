; RUN: llc -march=sbf -filetype=obj -o %t.el < %s
; RUN: llvm-readelf --headers %t.el | FileCheck %s
; RUN: llc -march=sbf -mcpu=v1 -filetype=obj -o %t.el < %s
; RUN: llvm-readelf --headers %t.el | FileCheck %s
; RUN: llc -march=sbf -mcpu=v2 -filetype=obj -o %t.el < %s
; RUN: llvm-readelf --headers %t.el | FileCheck %s
; RUN: llc -march=sbf -mcpu=v3 -filetype=obj -o %t.el < %s
; RUN: llvm-readelf --headers %t.el | FileCheck %s

; CHECK: OS/ABI:                            UNIX - System V

source_filename = "repro.6f693e153dbaee6f-cgu.0"
target datalayout = "e-m:e-p:64:64-i64:64-n32:64-S128"
target triple = "sbf"

@_ZN5repro7_RODATA17h37262c379f67649dE = hidden constant [1 x i8] zeroinitializer, align 1
@llvm.used = appending global [1 x ptr] [ptr @_ZN5repro7_RODATA17h37262c379f67649dE], section "llvm.metadata"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define noundef i64 @entrypoint(ptr nocapture noundef readnone %_input) unnamed_addr #0 {
start:
  ret i64 0
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) "target-cpu"="generic" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 8, !"PIC Level", i32 2}
!1 = !{!"rustc version 1.89.0-dev"}
