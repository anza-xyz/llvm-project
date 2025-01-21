; RUN: ld.lld -z notext -shared -Bdynamic -entry entrypoint %S/Inputs/sbf-dynsyms-dup.o -o %t1.so
; RUN: llvm-readelf %t1.so --dyn-syms | FileCheck %s

; Ensure no duplicates in case of function aliasing.
; sbf-dynsyms-dup.o contains the following rust code
;
; #[inline(never)]
; #[no_mangle]
; pub fn func_1(c: u64) -> u64 {
;     c % 67
; }
;
; #[inline(never)]
; #[no_mangle]
; pub fn func_2(a: u64) -> u64 {
;     return a % 67;
; }
;
; #[no_mangle]
; pub unsafe extern "C" fn entrypoint(a: u64, b: u64) -> u64 {
;     let r1 = func_1(a);
;     let r2 = func_2(b);
;     r1 + r2
; }
;
; Compiled with the command:
; rustc --target sbf-solana-solana --crate-type lib -C panic=abort -C opt-level=2 -C target_cpu=v3 a1.rs -o a1.o

; CHECK:      0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT   UND
; CHECK-NEXT: 1: 00000000000002a0    24 FUNC    GLOBAL DEFAULT     6 func_1
; CHECK-NEXT: 2: 00000000000002b8    56 FUNC    GLOBAL DEFAULT     6 entrypoint
; CHECK-NOT:  3:
