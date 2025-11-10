; RUN: llc -march=sbf -mattr=+alu32,+jmp-32 < %s | FileCheck %s

define i32 @test_jne(i32 %a, i32 %b) {
; CHECK-LABEL: test_jne
 entry:
    %cmp1 = icmp eq i32 %a, %b
    br i1 %cmp1, label %ret_now, label %cmp_imm
; CHECK: jne32 w0, w2,
 ret_now:
    %res1 = add i32 %a, %b
    ret i32 %res1
 cmp_imm:
    %cmp2 = icmp eq i32 %a, 98
    br i1 %cmp2, label %do_sub, label %do_mul
; CHECK: jne32 w0, 98
 do_sub:
    %res2 = sub i32 %a, %b
    ret i32 %res2
 do_mul:
    %res3 = mul i32 %a, %b
    ret i32 %res3
}

define i32 @test_jeq(i32 %a, i32 %b) {
; CHECK-LABEL: test_jeq
 entry:
    %cmp1 = icmp ne i32 %a, %b
    br i1 %cmp1, label %ret_now, label %cmp_imm
; CHECK: jeq32 w0, w2
 ret_now:
    %res1 = add i32 %a, %b
    ret i32 %res1
 cmp_imm:
    %cmp2 = icmp ne i32 %a, 98
    br i1 %cmp2, label %do_sub, label %do_mul
; CHECK: jeq32 w0, 98
 do_sub:
    %res2 = sub i32 %a, %b
    ret i32 %res2
 do_mul:
    %res3 = mul i32 %a, %b
    ret i32 %res3
}

define i32 @test_jugt(i32 %a, i32 %b) {
; CHECK-LABEL: test_jugt
 entry:
    %cmp1 = icmp ule i32 %a, %b
    br i1 %cmp1, label %ret_now, label %cmp_imm
; CHECK: jgt32 w0, w2
 ret_now:
    %res1 = add i32 %a, %b
    ret i32 %res1
 cmp_imm:
    %cmp2 = icmp ult i32 %a, 98
    br i1 %cmp2, label %do_sub, label %do_mul
; CHECK: jgt32 w0, 97
 do_sub:
    %res2 = sub i32 %a, %b
    ret i32 %res2
 do_mul:
    %res3 = mul i32 %a, %b
    ret i32 %res3
}

define i32 @test_jult(i32 %a, i32 %b) {
; CHECK-LABEL: test_jult
 entry:
    %cmp1 = icmp uge i32 %a, %b
    br i1 %cmp1, label %ret_now, label %cmp_imm
; CHECK: jlt32 w0, w2
 ret_now:
    %res1 = add i32 %a, %b
    ret i32 %res1
 cmp_imm:
    %cmp2 = icmp ugt i32 %a, 98
    br i1 %cmp2, label %do_sub, label %do_mul
; CHECK: jlt32 w0, 99
 do_sub:
    %res2 = sub i32 %a, %b
    ret i32 %res2
 do_mul:
    %res3 = mul i32 %a, %b
    ret i32 %res3
}

define i32 @test_jsgt(i32 %a, i32 %b) {
; CHECK-LABEL: test_jsgt
 entry:
    %cmp1 = icmp sle i32 %a, %b
    br i1 %cmp1, label %ret_now, label %cmp_imm
; CHECK: jsgt32 w0, w2
 ret_now:
    %res1 = add i32 %a, %b
    ret i32 %res1
 cmp_imm:
    %cmp2 = icmp slt i32 %a, 98
    br i1 %cmp2, label %do_sub, label %do_mul
; CHECK: jsgt32 w0, 97
 do_sub:
    %res2 = sub i32 %a, %b
    ret i32 %res2
 do_mul:
    %res3 = mul i32 %a, %b
    ret i32 %res3
}

define i32 @test_jslt(i32 %a, i32 %b) {
; CHECK-LABEL: test_jslt
 entry:
    %cmp1 = icmp sge i32 %a, %b
    br i1 %cmp1, label %ret_now, label %cmp_imm
; CHECK: jslt32 w0, w2
 ret_now:
    %res1 = add i32 %a, %b
    ret i32 %res1
 cmp_imm:
    %cmp2 = icmp sgt i32 %a, 98
    br i1 %cmp2, label %do_sub, label %do_mul
; CHECK: jslt32 w0, 99
 do_sub:
    %res2 = sub i32 %a, %b
    ret i32 %res2
 do_mul:
    %res3 = mul i32 %a, %b
    ret i32 %res3
}

define i32 @test_juge(i32 %a, i32 %b) {
; CHECK-LABEL: test_juge
 entry:
    %cmp1 = icmp ult i32 %a, %b
    br i1 %cmp1, label %ret_now, label %cmp_imm
; CHECK: jge32 w0, w2
 ret_now:
    %res1 = add i32 %a, %b
    ret i32 %res1
 cmp_imm:
    %cmp2 = icmp ult i32 %a, 0
    br i1 %cmp2, label %do_sub, label %do_mul
; CHECK: jge32 w0, 0
 do_sub:
    %res2 = sub i32 %a, %b
    ret i32 %res2
 do_mul:
    %res3 = mul i32 %a, %b
    ret i32 %res3
}

define i32 @test_jule(i32 %a, i32 %b) {
; CHECK-LABEL: test_jule
 entry:
    %cmp1 = icmp ugt i32 %a, %b
    br i1 %cmp1, label %ret_now, label %cmp_imm
; CHECK: jle32 w0, w2
 ret_now:
    %res1 = add i32 %a, %b
    ret i32 %res1
 cmp_imm:
    %cmp2 = icmp ugt i32 %a, -1
    br i1 %cmp2, label %do_sub, label %do_mul
; CHECK: jle32 w0, -1
 do_sub:
    %res2 = sub i32 %a, %b
    ret i32 %res2
 do_mul:
    %res3 = mul i32 %a, %b
    ret i32 %res3
}

define i32 @test_jsge(i32 %a, i32 %b) {
; CHECK-LABEL: test_jsge
 entry:
    %cmp1 = icmp slt i32 %a, %b
    br i1 %cmp1, label %ret_now, label %cmp_imm
; CHECK: jsge32 w0, w2
 ret_now:
    %res1 = add i32 %a, %b
    ret i32 %res1
 cmp_imm:
    %cmp2 = icmp slt i32 %a, -2147483648
    br i1 %cmp2, label %do_sub, label %do_mul
; CHECK: jsge32 w0, -2147483648
 do_sub:
    %res2 = sub i32 %a, %b
    ret i32 %res2
 do_mul:
    %res3 = mul i32 %a, %b
    ret i32 %res3
}

define i32 @test_jsle(i32 %a, i32 %b) {
; CHECK-LABEL: test_jsle
 entry:
    %cmp1 = icmp sgt i32 %a, %b
    br i1 %cmp1, label %ret_now, label %cmp_imm
; CHECK: jsle32 w0, w2
 ret_now:
    %res1 = add i32 %a, %b
    ret i32 %res1
 cmp_imm:
    %cmp2 = icmp slt i32 %a, 0
    br i1 %cmp2, label %do_sub, label %do_mul
; CHECK: jsle32 w0, -1
 do_sub:
    %res2 = sub i32 %a, %b
    ret i32 %res2
 do_mul:
    %res3 = mul i32 %a, %b
    ret i32 %res3
}

