; RUN: llc -march=sbf -mcpu=v2 < %s | FileCheck %s
; RUN: llc -mtriple=sbpfv2-solana-solana < %s | FileCheck %s

define i64 @test_func(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e) {
start:
; CHECK-LABEL: test_func:

; CHECK: stw [r10 - 20], 300
; CHECK: stdw [r10 - 32], 5400
; CHECK: stw [r10 - 12], 65516
; CHECK: stw [r10 - 4], 5

  %res = call i64 @func(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e, i8 5, i16 -20, i32 300, i64 5400)
  ret i64 %res
}

define i64 @func(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e, i8 %b8, i16 %b16, i32 %b32, i64 %b64) {
start:
; CHECK-LABEL: func:
; CHECK: add64 r10, -64
  %a1 = add i64 %a, %b
  %a2 = sub i64 %a1, %c
  %a3 = mul i64 %a2, %d
  %a4 = add i64 %a3, %e

; -64 + 32 = -32, so this is 5400 in %a5
; CHECK: ldxdw r4, [r10 + 32]

; -64 + 60 = -4, so this is 5 in %b8
; CHECK: ldxw w4, [r10 + 60]
  %c0 = trunc i64 %a to i8
  %b1 = add i8 %b8, %c0

; -64 + 52 = -12, so this is -20 in %b16
; ldxw w1, [r10 + 52]
  %c1 = trunc i64 %b to i16
  %b2 = add i16 %b16, %c1

; -64 + 44 = -20, so this is 300 in %b32
; CHECK: ldxw w1, [r10 + 44]
  %c2 = trunc i64 %c to i32
  %b3 = add i32 %b32, %c2

  %a5 = add i64 %a4, %b64
  %b4 = sext i8 %b1 to i64
  %a6 = sub i64 %a5, %b4

  %b5 = sext i16 %b2 to i64
  %a7 = mul i64 %a6, %b5

  %b6 = sext i32 %b3 to i64
  %a8 = add i64 %a7, %b6

  ret i64 %a8
}