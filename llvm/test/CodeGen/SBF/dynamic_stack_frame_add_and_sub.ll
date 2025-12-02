; RUN: llc < %s -march=sbf -mattr=+dynamic-frames | FileCheck %s
; RUN: llc -march=sbf -mattr=+dynamic-frames-v3,+alu32 < %s | FileCheck --check-prefix=CHECK-V3 %s
;
; Source:
; int test_func(int * vec, int idx) {
;      vec[idx] = idx-1;
;      return idx;
;  }
; Compilation flag:
; clang -S -emit-llvm test.c


; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define i32 @test_func(ptr noundef %vec, i32 noundef %idx) #0 {
; CHECK-LABEL: test_func:
; CHECK: add64 r10, -128
; CHECK-V3-NOT: add64 r10, 128
entry:
  %vec.addr = alloca ptr, align 8
  %idx.addr = alloca i512, align 4
  store ptr %vec, ptr %vec.addr, align 8
  store i32 %idx, ptr %idx.addr, align 4
  %0 = load i32, ptr %idx.addr, align 4
  %sub = sub nsw i32 %0, 1
  %1 = load ptr, ptr %vec.addr, align 8
  %2 = load i32, ptr %idx.addr, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds i32, ptr %1, i64 %idxprom
  store i32 %sub, ptr %arrayidx, align 4
  %3 = load i32, ptr %idx.addr, align 4
  ret i32 %3
}

declare i64 @read_ptr(ptr %a);

define i64 @test_func_4096(i64 %idx) {
; CHECK-LABEL: test_func_4096
; CHECK-V3-NOT: add64 r10, 4096
entry:
    %large_var = alloca [4096 x i8], align 8
    %val = call i64 @read_ptr(ptr %large_var)
    ret i64 %val
}

define i64 @test_func_4128(i64 %idx) {
; CHECK-LABEL: test_func_4128
; CHECK-V3: add64 r10, 64
; The stack is aligned at 64, so we bump 64 to have a stack size of 4096+64=4160,
; so we can fit the 4128 bytes of the array.
entry:
    %large_var = alloca [4128 x i8], align 8
    %val = call i64 @read_ptr(ptr %large_var)
    ret i64 %val
}