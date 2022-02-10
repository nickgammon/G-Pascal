

;
;  digitalWrite example
;
  jmp start     ; skip the variable declarations

;
;  VIA functions
;
pinmode_           = $809f     ; sets the pin mode - pin:0 to 15; mode: 0 or 1 : pin in A, mode in X
digitalread_       = $80a2     ; reads a pin - pin:0 to 15; Pin to read is in in A. Returns zero or non-zero in A.
digitalwrite_      = $80a5     ; writes to a pin - pin:0 to 15; value: 0 or 1 : pin in A, mode in X

delay_             = $80ae     ; delay for YYXX ms (Y = high-order byte, X = lo-order byte)

counter   dfb 0     ; how many toggles we did
pin_state dfb 0     ; current pin state

ITERATIONS = 20     ; how many times to loop (this will be 10 flashes)

start:

  stz pin_state
  stz counter

;
;  set PA2 to output
;
  lda #2   ; Port PA2
  ldx #1   ; write mode
  jsr pinmode_


write_loop:
;
;  write to PA2
;
  lda pin_state
  eor #1   ; toggle state
  sta pin_state
  tax
  lda #2   ; Port PA2
  jsr digitalwrite_
;
;  delay 500 ms
;
  ldx #<500
  ldy #>500
  jsr delay_
;
;  do it ITERATIONS times
;
  inc counter
  lda counter
  cmp #ITERATIONS
  bcc write_loop
;
;  all done
;
  rts

