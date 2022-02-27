;--------------------------------------------------
; Maths routines. 3-byte operations on VALUE and VALUE2
;
;--------------------------------------------------
exp_add:
  clc
  lda VALUE
  adc VALUE2
  sta VALUE
  lda VALUE+1
  adc VALUE2+1
  sta VALUE+1
  lda VALUE+2
  adc VALUE2+2
  sta VALUE+2
  rts

;
;  subtract VALUE2 from VALUE, store in VALUE, also set the sign bit
;   and put the low order two bytes in X and Y
;
exp_subtract:
  sec
  lda VALUE
  sbc VALUE2
  sta VALUE
  tay
  lda VALUE+1
  sbc VALUE2+1
  tax
  sta VALUE+1
  lda VALUE+2
  sbc VALUE2+2
  sta VALUE+2
  rts

exp_true:
  lda #1
  sta VALUE
  stz VALUE+1
  stz VALUE+2
  rts

exp_false:
  stz VALUE
  stz VALUE+1
  stz VALUE+2
  rts

exp_eql:
  lda VALUE
  cmp VALUE2
  bne exp_false
  lda VALUE+1
  cmp VALUE2+1
  bne exp_false
  lda VALUE+2
  cmp VALUE2+2
  bne exp_false
  bra exp_true

exp_neq:
  lda VALUE
  cmp VALUE2
  bne exp_true
  lda VALUE+1
  cmp VALUE2+1
  bne exp_true
  lda VALUE+2
  cmp VALUE2+2
  bne exp_true
  bra exp_false

exp_less_than:
  jsr exp_subtract
  bmi exp_true
  bpl exp_false

exp_greater_than:
  jsr exp_subtract
  bmi exp_false
  bne exp_true
  tya
  bne exp_true
  txa
  bne exp_true
  bpl exp_false

exp_leq:
  jsr exp_subtract
  bmi exp_true
  bne exp_false
  tya
  bne exp_false
  txa
  bne exp_false
  bpl exp_true

exp_geq:
  jsr exp_subtract
  bmi exp_false
  bpl exp_true

exp_bitwise_or:
  lda VALUE
  ora VALUE2
  sta VALUE
  tay
  lda VALUE+1
  ora VALUE2+1
  sta VALUE+1
  tax
  lda VALUE+2
  ora VALUE2+2
  sta VALUE+2
  rts

exp_bitwise_and:
  lda VALUE
  and VALUE2
  sta VALUE
  tay
  lda VALUE+1
  and VALUE2+1
  sta VALUE+1
  tax
  lda VALUE+2
  and VALUE2+2
  sta VALUE+2
  rts

exp_bitwise_xor:
  lda VALUE
  eor VALUE2
  sta VALUE
  tay
  lda VALUE+1
  eor VALUE2+1
  sta VALUE+1
  tax
  lda VALUE+2
  eor VALUE2+2
  sta VALUE+2
  rts


exp_trueJ jmp exp_true
exp_falseJ jmp exp_false

exp_logical_or:
  jsr exp_bitwise_or
  bne exp_trueJ
  cpx #0
  bne exp_trueJ
  cpy #0
  bne exp_trueJ
  bra exp_falseJ

exp_logical_and:
  lda VALUE
  ora VALUE+1
  ora VALUE+2
  beq exp_falseJ  ; if VALUE has all bits zero the logical "and" must be false
;
;  VALUE is non-zero, how about VALUE2?
;
  lda VALUE2
  ora VALUE2+1
  ora VALUE2+2
  beq exp_falseJ  ; if VALUE2 has all bits zero the logical "and" must be false
  bra exp_trueJ   ; both are non-zero, therefore the result is true

exp_shift_left:
  lda VALUE2+1
  ora VALUE2+2
  bne exp_falseJ  ; big shifts will return zero
  lda VALUE2
  beq exp_shift_left_done ; shift by zero? we are done then
  tax
  and #$E0        ; maximum of 32 shifts
  bne exp_falseJ
exp_shift_left_loop:
  asl VALUE
  rol VALUE+1
  rol VALUE+2
  dex
  bne exp_shift_left_loop
exp_shift_left_done:
  rts

exp_shift_right:
  lda VALUE2+1
  ora VALUE2+2
  bne exp_falseJ  ; big shifts will return zero
  lda VALUE2
  beq exp_shift_right_done ; shift by zero? we are done then
  tax
  and #$E0        ; maximum of 32 shifts
  bne exp_falseJ
exp_shift_right_loop:
  lsr VALUE+2
  ror VALUE+1
  ror VALUE
  dex
  bne exp_shift_right_loop
exp_shift_right_done:
  rts

exp_negate:
  lda VALUE
  eor #$FF
  sta VALUE
  lda VALUE+1
  eor #$FF
  sta VALUE+1
  lda VALUE+2
  eor #$FF
  sta VALUE+2
  rts

exp_not:
  lda VALUE
  bne exp_falseJ
  lda VALUE+1
  bne exp_falseJ
  lda VALUE+2
  bne exp_falseJ
  bra exp_trueJ

exp_unary_minus:
  sec
  lda #0
  sbc VALUE
  sta VALUE
  lda #0
  sbc VALUE+1
  sta VALUE+1
  lda #0
  sbc VALUE+2
  sta VALUE+2
  rts


exp_low_byte:
  stz VALUE+1
  stz VALUE+2
  rts

exp_high_byte:
  lda VALUE+1
  sta VALUE
  stz VALUE+1
  stz VALUE+2
  rts

;
;  make VALUE positive
;
exp_abs_val:
  lda  VALUE+2  ; check high-order byte, see if negative
  bpl  exp_abs_val_done
exp_negate_val:
  sec
  lda  #0
  sbc  VALUE
  sta  VALUE
  lda  #0
  sbc  VALUE+1
  sta  VALUE+1
  lda  #0
  sbc  VALUE+2
  sta  VALUE+2
exp_abs_val_done:
  rts

;
;  make VALUE2 positive
;
exp_abs_val2:
  lda  VALUE2+2  ; check high-order byte, see if negative
  bpl  exp_abs_val2_done
exp_negate_val2:
  sec
  lda  #0
  sbc  VALUE2
  sta  VALUE2
  lda  #0
  sbc  VALUE2+1
  sta  VALUE2+1
  lda  #0
  sbc  VALUE2+2
  sta  VALUE2+2
exp_abs_val2_done:
  rts


ZERRES:
  lda  #0
  sta  RES
  sta  RES+1
  sta  RES+2
  rts

exp_find_sign:
   jsr  ZERRES     ; zero result
   lda  VALUE+2
   and  #$80
   sta  RMNDR
   lda  VALUE2+2
   and  #$80
   eor  RMNDR
   sta  RMNDR     ; RMNDR is 0x80 if the result needs to be negative
   jsr  exp_abs_val
   jsr  exp_abs_val2
   rts
;

exp_fix_sign:
  lda  RMNDR
  bpl  exp_fix_sign_done      ; nothing needs to be changed
  sec                         ; subtract result from zero
  lda  #0
  sbc  RES
  tax
  lda  #0
  sbc  RES+1
  tay
  lda  #0
  sbc  RES+2
exp_fix_sign_done:
  rts


;
;  multiply VALUE1 by VALUE2 and store in VALUE
;
;  see: https://llx.com/Neil/a2/mult.html for a discussion
;

exp_multiply:
  jsr exp_find_sign
  ldx  #24       ; for all 24 bits
exp_multiply_loop:
  asl  RES
  rol  RES+1
  rol  RES+2                    ; shift RES left one bit
  asl  VALUE
  rol  VALUE+1
  rol  VALUE+2                  ; shift one argument left one bit and get the low-order bit into carry
  bcc  exp_multiply_no_add      ; if no low-order bit, don't add anything
  clc                           ; add the other argument to the result
  lda  VALUE2
  adc  RES
  sta  RES
  lda  VALUE2+1
  adc  RES+1
  sta  RES+1
  lda  VALUE2+2
  adc  RES+2
  sta  RES+2
exp_multiply_no_add:
  dex                      ; count bits
  bne  exp_multiply_loop   ; go back if more to go
  jsr  exp_fix_sign
  lda  RES
  sta  VALUE
  lda  RES+1
  sta  VALUE+1
  lda  RES+2
  sta  VALUE+2
  rts

;
;  divide VALUE by VALUE2, store result in VALUE
;
exp_divide:
  jsr exp_find_sign

  lda  VALUE2
  ora  VALUE2+1
  ora  VALUE2+2
  bne  exp_divide_not_zero  ; check not dividing by zero
  ldx  #47                  ; ERROR: divide by zero
  jmp  ass_error
  ;
exp_divide_not_zero:
  jsr  ZERRES     ; zero result (RES) - leaves zero in A register
  sta  REMAIN
  sta  REMAIN+1
  sta  REMAIN+2
  lda  #24       ; for all 24 bits of the operands ...
  sta  maths_work
exp_divide_loop:
  asl  VALUE
  rol  VALUE+1
  rol  VALUE+2    ; shift the dividend left one bit
  rol  REMAIN    ; the low-order bit of the dividend is shifted into the remainder
  rol  REMAIN+1
  rol  REMAIN+2
  sec
  lda  REMAIN    ; do a trial subtraction of: remainder - divisor
  sbc  VALUE2
  tax
  lda  REMAIN+1
  sbc  VALUE2+1
  tay
  lda  REMAIN+2
  sbc  VALUE2+2
  bmi  exp_divide_subtraction_negative      ; if the subtraction fails (is negative) then discard the result
  sta  REMAIN+2  ; if it succeeds then the result of the subtraction is our new remainder
  tya
  sta  REMAIN+1
  txa
  sta  REMAIN
  sec            ; shift a one-bit into the quotient
  bcs  exp_divide_subtraction_positive
exp_divide_subtraction_negative:
  clc            ; here if subtraction failed, quotient (RES) gets a 0-bit shifted in
exp_divide_subtraction_positive:
  rol  RES       ; the carry bit (from above) is shifted into the quotient
  rol  RES+1
  rol  RES+2
  dec  maths_work      ; one less bit to go
  bne  exp_divide_loop      ; repeat until 24 bits done
  jsr  exp_fix_sign
  lda  RES
  sta  VALUE
  lda  RES+1
  sta  VALUE+1
  lda  RES+2
  sta  VALUE+2
  rts

exp_modulo:
  jsr exp_divide
  lda REMAIN
  sta VALUE
  lda REMAIN+1
  sta VALUE+1
  lda REMAIN+2
  sta VALUE+2
  rts
