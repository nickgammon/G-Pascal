;
; expression evaluator test
;

; PRECEDENCE 1

 assert $FFFFFF == ~0   ; negation
 assert %0101010000110010 == ((~%1010101111001101) & $FFFF)  ; negation

 assert 0 == !1    ; not
 assert 1 == !0    ; not
 assert 0 == !666  ; not

 assert -1 == - (1) ; unary minus

 assert <$1234 == $34  ; low-order byte
 assert >$1234 == $12  ; high-order byte


; PRECEDENCE 2

  assert $1234 << 8 == $123400  ; shift left
  assert $1234 >> 4 == $123     ; shift right

; PRECEDENCE 3


 assert $ABCDEF & $00FF00 == $00CD00  ; bitwise AND

; PRECEDENCE 4

 assert $FFFFFF ^ $101010 == $EFEFEF  ; bitwise xor

; PRECEDENCE 5

 assert $000000 | $101010 == $101010  ; bitwise or

; PRECEDENCE 6

 assert 524288 * 2 == 1048576  ; multiplication
 assert 1048576 / 2 == 524288  ; division
 assert 1235684 % 3 == 2       ; modulus (remainder)

; PRECEDENCE 7

 assert 2 + 2 == 4                  ; addition lol
 assert 1235684 + 524288 == 1759972 ; addition
 assert 1235684 - 524288 == 711396  ; subtraction

; PRECEDENCE 8

 assert 42 < 666               ; less than
 assert 42 <= 42               ; less than or equal
 assert 12345678 > 12345677    ; greater than
 assert 12345678 >= 12345678   ; greater than or equal

; PRECEDENCE 9

 assert 2 <> 3  ; unequal

;
;  put the equality test first because of precedence
;

; PRECEDENCE 10

 assert 1 == (456 && 789)  ; logical and
 assert 0 == (0 && 1)      ; logical and

; PRECEDENCE 11

 assert 1 == (456 || 0)    ; logical or
 assert 1 == (0 || 666)    ; logical or
 assert 0 == (0 || 0)      ; logical or

; ----------------------
;  Precedence tests

 assert 2+3*4 == 14 ; not 20
 assert - 5 + 6 == 1 ; not -11
 assert - (5 + 6) == -11

