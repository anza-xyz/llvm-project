; RUN: ld.lld -z notext -shared -Bdynamic -entry entrypoint %S/Inputs/sbf-dynsyms-local.o -o %t1.so
; RUN: llvm-readelf %t1.so --dyn-syms | FileCheck %s
; RUN: ld.lld -z notext -shared -Bdynamic -entry entrypoint %S/Inputs/sbf-dynsyms-local.o -strip-all -o %t1.so
; RUN: llvm-readelf %t1.so --dyn-syms | FileCheck --check-prefix=CHECK-STRIP %s

; Ensure local symbols appear in the dynamic symbol table
; sbf-dynsyms-local.o contains the following rust code
;
; #[inline(never)]
; fn func_1(c: u64) -> u64 {
;     c % 67
; }
;
; #[inline(never)]
; fn func_2(a: u64) -> u64 {
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
; #[inline(never)]
; fn func_3(c: u64) -> u64 {
;     c % 68
; }
;
; Compiled with the command:
; rustc --target sbf-solana-solana --crate-type lib -C panic=abort -C opt-level=2 -C target_cpu=v3 a1.rs -o a1.o

; CHECK:       0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT   UND
; CHECK-NEXT:  1: 00000000000002c8    24 FUNC    LOCAL  DEFAULT     5 _ZN2a16func_117he4a7267c4c063035E
; CHECK-NEXT:  2: 00000000000002e0   104 FUNC    GLOBAL DEFAULT     5 entrypoint
; CHECK-NEXT:  3: 0000000000000348    24 FUNC    LOCAL  DEFAULT     5 _ZN2a16func_317ha1589f0ded44505fE
; CHECK-NOT:   4:

; CHECK-STRIP:       0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT   UND
; CHECK-STRIP-NEXT:  1: 0000000000000290    24 FUNC    LOCAL  DEFAULT     5 hidden_func
; CHECK-STRIP-NEXT:  2: 00000000000002a8   104 FUNC    GLOBAL DEFAULT     5 entrypoint
; CHECK-STRIP-NEXT:  3: 0000000000000310    24 FUNC    LOCAL  DEFAULT     5 hidden_func
; CHECK-STRIP-NOT:   4: