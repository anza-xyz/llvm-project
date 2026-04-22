; RUN: llc -march=sbf < %s 2>&1 >/dev/null | FileCheck %s --check-prefix=DEFAULT
; RUN: llc -march=sbf -sbf-stack-size=8192 < %s 2>&1 >/dev/null | FileCheck %s --check-prefix=BUMPED --allow-empty
; RUN: llc -march=sbf -mcpu=v3 < %s 2>&1 >/dev/null | FileCheck %s --check-prefix=DEFAULT
; RUN: llc -march=sbf -mcpu=v3 -sbf-stack-size=8192 < %s 2>&1 >/dev/null | FileCheck %s --check-prefix=BUMPED --allow-empty
;
; Source:
;   extern void doit(void *);
;   void warn(void) {
;       char buf[5000];
;       long x;
;       doit(&x);
;       doit(buf);
;   }
; Compilation:
;   clang -target sbf -S -emit-llvm -g -O2 warn_stack.c

declare void @doit(ptr)

; DEFAULT: Error: warn_stack.c
; DEFAULT: Please, minimize large stack variables
; DEFAULT: Exceeding the maximum stack offset may cause undefined behavior during execution.
; BUMPED-NOT: Please, minimize large stack variables
define void @warn() !dbg !5 {
  %buf = alloca [5000 x i8], align 1
  %x = alloca i64, align 8
  call void @doit(ptr nonnull %x), !dbg !7
  call void @doit(ptr nonnull %buf), !dbg !7
  ret void, !dbg !7
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "warn_stack.c", directory: "/")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = distinct !DISubprogram(name: "warn", scope: !1, file: !1, line: 2, type: !6, isLocal: false, isDefinition: true, scopeLine: 2, flags: DIFlagPrototyped, isOptimized: true, unit: !0)
!6 = !DISubroutineType(types: !2)
!7 = !DILocation(line: 5, column: 5, scope: !5)
