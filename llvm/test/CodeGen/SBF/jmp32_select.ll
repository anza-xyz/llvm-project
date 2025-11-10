; RUN: llc -march=sbf -mattr=+alu32,+jmp-32 < %s | FileCheck %s

define i32 @test_jne_reg(i32 %a, i32 %b, i32 %c, i32 %d) {
; CHECK-LABEL: test_jne_reg
    %cmp1 = icmp ne i32 %a, %b
    %res = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jne32 w1, w2
    ret i32 %res
}

define i32 @test_jne_imm(i32 %a, i32 %b, i32 %c, i32 %d) {
; CHECK-LABEL: test_jne_imm
    %cmp1 = icmp ne i32 %a, 78
    %res = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jne32 w1, 78
    ret i32 %res
}

define i32 @test_jeq_reg(i32 %a, i32 %b, i32 %c, i32 %d) {
; CHECK-LABEL: test_jeq_reg
    %cmp1 = icmp eq i32 %a, %b
    %res = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jeq32 w1, w2
    ret i32 %res
}

define i32 @test_jeq_imm(i32 %a, i32 %b, i32 %c, i32 %d) {
; CHECK-LABEL: test_jeq_imm
    %cmp1 = icmp eq i32 %a, 78
    %res = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jeq32 w1, 78
    ret i32 %res
}

define i32 @test_jugt_reg(i32 %a, i32 %b, i32 %c, i32 %d) {
; CHECK-LABEL: test_jugt_reg
    %cmp1 = icmp ugt i32 %a, %b
    %res = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jgt32 w1, w2
    ret i32 %res
}

define i32 @test_jugt_imm(i32 %a, i32 %b, i32 %c, i32 %d) {
; CHECK-LABRL: test_jugt_imm
    %cmp1 = icmp ugt i32 %a, 78
    %res = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jgt32 w1, 78
    ret i32 %res
}

define i32 @test_juge_reg(i32 %a, i32 %b, i32 %c, i32 %d) {
; CHECK-LABEL: test_juge_reg
    %cmp1 = icmp uge i32 %a, %b
    %res = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jge32 w1, w2
    ret i32 %res
}

define i32 @test_juge_imm(i32 %a, i32 %b, i32 %c, i32 %d) {
; uge is always converted to jgt
; CHECK: test_juge_imm
    %cmp1 = icmp uge i32 %a, 2
    %res = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jgt32 w1, 1
    ret i32 %res
}

define i32 @test_jsgt_reg(i32 %a, i32 %b, i32 %c, i32 %d) {
; CHECK-LABRL: test_jsgt_reg
    %cmp1 = icmp sgt i32 %a, %b
    %res = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jsgt32 w1, w2
    ret i32 %res
}

define i32 @test_jsgt_imm(i32 %a, i32 %b, i32 %c, i32 %d) {
; CHECK-LABEL: test_jsgt_imm
    %cmp1 = icmp sgt i32 %a, 78
    %res = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jsgt32 w1, 78
    ret i32 %res
}

define i32 @test_jsge_reg(i32 %a, i32 %b, i32 %c, i32 %d) {
; CHECK-LABEL: test_jsge_reg
    %cmp1 = icmp sge i32 %a, %b
    %res = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jsge32 w1, w2
    ret i32 %res
}

define i32 @test_jsge_imm(i32 %a, i32 %b, i32 %c, i32 %d) {
; sge is always converted to jsgt
; CHECK-LABEL: test_jsge_imm
    %cmp1 = icmp sge i32 %a, 5
    %res = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jsgt32 w1, 4
    ret i32 %res
}

define i32 @test_jult_reg(i32 %a, i32 %b, i32 %c, i32 %d) {
; CHECK-LABEL: test_jult_reg
    %cmp1 = icmp ult i32 %a, %b
    %res = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jlt32 w1, w2
    ret i32 %res
}

define i32 @test_jult_imm(i32 %a, i32 %b, i32 %c, i32 %d) {
; CHECK-LABEL: test_jult_imm
    %cmp1 = icmp ult i32 %a, 78
    %res = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jlt32 w1, 78
    ret i32 %res
}

define i32 @test_jule_reg(i32 %a, i32 %b, i32 %c, i32 %d) {
; CHECK-LABEL: test_jule_reg
    %cmp1 = icmp ule i32 %a, %b
    %res = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jle32 w1, w2
    ret i32 %res
}

define i32 @test_jule_imm(i32 %a, i32 %b, i32 %c, i32 %d) {
; ule is always converted to jlt
; CHECK-LABEL: test_jule_imm
    %cmp1 = icmp ule i32 %a, 20
    %res1 = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jlt32 w1, 21
    ret i32 %res1
}

define i32 @test_jslt_reg(i32 %a, i32 %b, i32 %c, i32 %d) {
; CHECK-LABEL: test_jslt_reg
    %cmp1 = icmp slt i32 %a, %b
    %res = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jslt32 w1, w2
    ret i32 %res
}

define i32 @test_jslt_imm(i32 %a, i32 %b, i32 %c, i32 %d) {
; CHECK-LABEL: test_jslt_imm
    %cmp1 = icmp slt i32 %a, 20
    %res1 = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jslt32 w1, 20
    ret i32 %res1
}

define i32 @test_jsle_reg(i32 %a, i32 %b, i32 %c, i32 %d) {
; CHECK-LABEL: test_jsle_reg
    %cmp1 = icmp sle i32 %a, %b
    %res = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jsle32 w1, w2
    ret i32 %res
}

define i32 @test_jsle_imm(i32 %a, i32 %b, i32 %c, i32 %d) {
; sle is always converted to jslt
; CHECK-LABEL: test_jsle_imm
    %cmp1 = icmp sle i32 %a, 20
    %res1 = select i1 %cmp1, i32 %c, i32 %d
; CHECK: jslt32 w1, 21
    ret i32 %res1
}