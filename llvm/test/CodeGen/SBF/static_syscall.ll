; RUN: llc -march=sbf < %s | FileCheck --check-prefixes=CHECK %s
; RUN: llc -march=sbf -mattr=+static-syscalls -show-mc-encoding < %s | FileCheck --check-prefixes=CHECK %s


; Function Attrs: nounwind
define dso_local i32 @test(i32 noundef %a, i32 noundef %b) {
entry:
; CHECK-LABEL: test

; CHECK: call 2
  %syscall_1 = tail call i32 inttoptr (i64 2 to ptr)(i32 noundef %a, i32 noundef %b)

; CHECK: call 11
  %syscall_2 = tail call i32 inttoptr (i64 11 to ptr)(i32 noundef %a, i32 noundef %b)

; CHECK: call 112
  %syscall_3 = tail call i32 inttoptr (i64 112 to ptr)(i32 noundef %a, i32 noundef %b)

; CHECK: mov64 r1, 89
; CHECK: mov64 r2, 87
; CHECK: call 112
  %syscall_4 = tail call i32 inttoptr (i64 112 to ptr)(i32 89,  i32 87)

  %add_1 = add i32 %syscall_1, %syscall_2
  %add_2 = add i32 %add_1, %syscall_3
  %add_3 = add i32 %add_2, %syscall_4
  ret i32 %add_3
}
