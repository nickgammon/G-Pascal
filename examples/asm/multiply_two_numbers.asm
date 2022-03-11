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
  jsr display_in_decimal
; done!
  rts
