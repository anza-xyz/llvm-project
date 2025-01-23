; RUN: ld.lld -z notext -entry entrypoint --export-dynamic-symbol=entrypoint --pic-executable %S/Inputs/sbf-dynsyms-noexport.o -o %t1.so
; RUN: llvm-readelf %t1.so --dyn-syms | FileCheck %s

; Ensure global symbols not exported are also in the dynamic symbol table.
; sbf-dynsyms-noexport.o contains the following rust code
;
; #[inline(never)]
; fn func_1(c: u64) -> u64 {
;     c % 67
; }
;
; #[inline(never)]
; fn func_2(c: u64) -> u64 {
;     c % 67
; }
;
; #[no_mangle]
; pub unsafe extern "C" fn entrypoint(a: u64, b: u64, c: u64) -> u64 {
;     let r1 = func_1(a);
;     let r2 = func_2(b);
;     let r3 = func_3(c);
;     r1 + r2 + r3
; }
;
; #[inline(never)]
; #[no_mangle]
; pub extern "C" fn func_3(a: u64) -> u64 {
;     return a % 65;
; }
;
; Compiled with the command:
; rustc --target sbf-solana-solana --crate-type lib -C panic=abort -C opt-level=2 -C target_cpu=v3 a1.rs -o a1.o

; CHECK:      0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT   UND
; CHECK-NEXT: 1: 00000000000002b8    24 FUNC    LOCAL  DEFAULT     5 _ZN2a16func_117he4a7267c4c063035E
; CHECK-NEXT: 2: 00000000000002d0   104 FUNC    GLOBAL DEFAULT     5 entrypoint
; CHECK-NEXT: 3: 0000000000000338    24 FUNC    GLOBAL DEFAULT     5 func_3
; CHECK-NOT:  4:
