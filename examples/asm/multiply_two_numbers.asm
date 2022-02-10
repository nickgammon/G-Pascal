;
;  zero-page locations used by this code
;
reg            = $0000 ; (3 bytes) work "register" used in various places
value          = $0006 ; (3 bytes) used by the assembler expression evaluator
value2         = $0009 ; (3 bytes) used by the assembler expression evaluator
;
;  functions used by this code
;
exp_multiply   = $8081     ; VALUE := VALUE * VALUE2 (may overflow and the overflow is lost)
dsp_bin        = $809c     ; write the number in REG (3 bytes) to the serial port (in decimal)

;
;  put 47302 into value
;
  lda #<47302
  sta value
  lda #>47302
  sta value+1
  stz value+2
;
;  put 55 into value2
;
  lda #<55
  sta value2
  lda #>55
  sta value2+1
  stz value2+2
;
;  multiply them to get the result 2601610
;
  jsr exp_multiply
;
;  display the result
;
  lda value
  sta reg
  lda value+1
  sta reg+1
  lda value+2
  sta reg+2
  jsr dsp_bin
; done!
  rts

