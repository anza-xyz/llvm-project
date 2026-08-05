; RUN: llc -march=sbf -mcpu=v3 < %s 2>&1 >/dev/null | FileCheck %s

; CHECK: Function my_func overflows the maximum allowed frame space by accessing an offset
define i64 @my_func(i64 %my_idx) nounwind {
  %a = alloca [5024 x i8], align 8
  %b = getelementptr inbounds [5024 x i8], ptr %a, i64 0, i64 %my_idx
  %c = load i64, ptr %b, align 8

  %d = alloca [2 x i8], align 8
  %e = getelementptr inbounds [2 x i8], ptr %d, i64 0, i64 %my_idx
  %f = load i64, ptr %e, align 8

  %g = add i64 %f, %c
  ret i64 %g
}

; CHECK: Function my_func_2 overflows the maximum allowed frame space by accessing an offset
define i64 @my_func_2(i64 %my_idx) nounwind {
  %a = alloca [5024 x i8], align 8
  %b = getelementptr inbounds [5024 x i8], ptr %a, i64 0, i64 %my_idx
  %c = load i64, ptr %b, align 8

  ret i64 %c
}

; CHECK: Function callee overflows the maximum allowed frame space by accessing an offset
define i64 @callee(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e, i64 %f, i64 %g) {
  %h = alloca [5080 x i8], align 8
  %i = getelementptr inbounds [5024 x i8], ptr %h, i64 0, i64 %a
  %j = load i64, ptr %i, align 8
  %k = add i64 %j, %b
  %l = add i64 %k, %c
  %m = add i64 %l, %d
  %n = mul i64 %e, %f
  %o = add i64 %m, %n
  %p = add i64 %o, %g

  ret i64 %p
}

define i64 @caller(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e) {
    %f = add i64 %a, %b
    %g = add i64 %c, %d

    %res = call i64 @callee(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e, i64 %f, i64 %g)
    ret i64 %res
}
