;
;  digitalWrite example
;
  jmp start     ; skip the variable declarations

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
  jsr pinmode


write_loop:
;
;  write to PA2
;
  lda pin_state
  eor #1   ; toggle state
  sta pin_state
  tax
  lda #2   ; Port PA2
  jsr digitalwrite
;
;  delay 500 ms
;
  ldx #<500
  ldy #>500
  jsr delay
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

