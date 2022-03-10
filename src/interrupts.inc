
break_message     asciiz "\nBRK executed at address $"
a_equals_message  asciiz ", A = $"
x_equals_message  asciiz ", X = $"
y_equals_message  asciiz ", Y = $"
p_equals_message  asciiz ", P = $"
s_equals_message  asciiz ", S = $"
id_equals_message asciiz ", id = $"

;--------------------------------------------------
;  IRQ - here on BRK interrupt
;--------------------------------------------------
brk_executed:
  sty call_y    ; save Y - we didn't touch that
  stx call_s    ; X still has the stack pointer in it
  plx           ; get X back
  stx call_x
  pla           ; get A back
  sta call_a    ; save A
  pla           ; get the processor flags from the stack
  sta call_p
  pla           ; low-order address of BRK
  sta brk_address
  pla           ; high order address of BRK
  sta brk_address+1
  cli           ; allow interrupts now so we can print

;
;  display BRK info for the user
;
  ;
  ;  subtract 2 from brk_address to get where it actually was
  ;
  sec
  lda brk_address
  sbc #2
  sta brk_address
  lda brk_address+1
  sbc #0
  sta brk_address+1
  lda #<break_message
  ldx #>break_message
  jsr print
  lda brk_address+1
  jsr PRBYTE
  lda brk_address
  jsr PRBYTE
;
;  show A, X, Y, P, S
;
  lda #<a_equals_message
  ldx #>a_equals_message
  jsr print
  lda call_a
  jsr PRBYTE
  lda #<x_equals_message
  ldx #>x_equals_message
  jsr print
  lda call_x
  jsr PRBYTE
  lda #<y_equals_message
  ldx #>y_equals_message
  jsr print
  lda call_y
  jsr PRBYTE
  lda #<p_equals_message
  ldx #>p_equals_message
  jsr print
  lda call_p
  jsr PRBYTE
  lda #<s_equals_message
  ldx #>s_equals_message
  jsr print
  lda call_s
  jsr PRBYTE
  lda #<id_equals_message
  ldx #>id_equals_message
  jsr print
;
;  advance one byte to get the break ID (byte after the BRK)
;
  inc brk_address
  bne brk_executed1
  inc brk_address+1
brk_executed1:
  ldy #0
  lda (brk_address),Y
  jsr PRBYTE
;
;  put brk_address back to where we should resume from
;
  inc brk_address
  bne brk_executed2
  inc brk_address+1
brk_executed2:

  jsr CROUT
;
;  display stack
;
  lda #<stack_message
  ldx #>stack_message
  jsr print
  lda call_s
  clc
  adc #6  ; the interrupt pushed 3 we pushed 2, plus the stack points to the first UNUSED spot
  tax
brk_executed3:
  lda $100,x
  phx
  jsr PRBYTE
  jsr PUTSP
  plx
  inx
  bne brk_executed3

;
;  we are done now
;
  jsr CROUT
  jmp main_prompt

stack_message asciiz "Stack: "

brk_executedJ jmp brk_executed

;--------------------------------------------------
;  IRQ - here on maskable interrupt
;--------------------------------------------------

irq:
  pha       ; save A and X
  phx
  tsx       ; get stack pointer
  cpx #$FB  ; should be lesss than $FB (since the interrupt pushed 3 and we pushed 2)
  bcc irq1
  ldx #27   ; ERROR: stack full
  cli       ; allow interrupts or error won't print
  jmp ERROR

irq1:
  lda $103,X    ; this will be the processor flags on the stack (3 up from where we are now)
  and #$10      ; check BRK flag
  bne brk_executedJ
;
;  here for a hardware IRQ
;
  lda VIA_IFR
  and #VIA_FLAG_CB2        ; CB2 active edge
  bne cb2_interrupt

  lda VIA_IFR
  and #VIA_FLAG_TIMER1     ; Time out of T1
  bne timer1_interrupt


;
;  we shouldn't get here, and if we do we will probably just loop servicing the interrupt
;
  .if SERIAL_DEBUGGING

    lda #SERIAL_SPARE2_MASK
    tsb VIA_PORTA
    trb VIA_PORTA

  .endif  ; SERIAL_DEBUGGING

  jmp irq_done

;
;  here for incoming start bit
;
cb2_interrupt:

  sta VIA_IER           ; turn off that interrupt
  sta VIA_IFR           ; indicate we noticed it

  phx
  phy

  ldx #8              ; count of bits
  ldy #SERIAL_DELAY1  ; delay interval for first bit (allow for code above) TODO - probably needs to be less now
serial_in_loop:
;
; wait for bit time to be up
;
  dey
  bne serial_in_loop

  .if SERIAL_DEBUGGING
    lda #SERIAL_DEBUG1_MASK     ; toggle debug flag (2 cycles)
    tsb VIA_PORTA               ; paint the fence: up (6 cycles)
    trb VIA_PORTA               ; down (6 cycles)
                                ; look eye! always look eye
  .else
    NOP    ; take the same time so the timing isn't thrown out
    NOP    ; 2 cycles each
    NOP
    NOP
    NOP
    NOP
    NOP    ; total of 14 cycles
  .endif  ; SERIAL_DEBUGGING

  lda VIA_PORTA
  ror A                 ; put incoming bit into carry
  ror serial_in_byte    ; shift one bit in
  ldy #SERIAL_DELAY2    ; delay interval for remaining bits
  dex
  bne serial_in_loop

  ply
  plx
;
;  re-enable interrupts on the falling edge
;
  lda #VIA_FLAG_ENABLE | VIA_FLAG_CB2
  sta VIA_IER
  sta VIA_IFR   ; indicate we know about previous falling edges

;
;  move to place that we know it will be noticed
;
  lda serial_in_byte
  sta serial_in_byte_received

  bra irq_done

;
;  here for time to send another outgoing bit
;
timer1_interrupt :

  sta VIA_IFR           ; indicate we noticed it
  lda serial_out_bit    ; are we in fact sending a byte?
  beq irq_done          ; no, just exit

  .if SERIAL_DEBUGGING

  lda #SERIAL_DEBUG2_MASK     ; toggle debug flag (2 cycles)
  tsb VIA_PORTA               ; wash on   (6 cycles)
  trb VIA_PORTA               ; wash off  (6 cycles)
                              ; now, sand the floor
 .else
     NOP    ; take the same time so the timing isn't thrown out
     NOP    ; 2 cycles each
     NOP
     NOP
     NOP
     NOP
     NOP    ; total of 14 cycles
  .endif  ; SERIAL_DEBUGGING

  ror serial_out_byte+1  ; we don't care about the high-order bit
  ror serial_out_byte    ; get the low-order bit into Carry
  bcc write_zero
  lda #SERIAL_OUT_MASK   ; (send a 1 bit)
  ora VIA_PORTA
  bra serial_send_count_bits

write_zero:
  lda #~SERIAL_OUT_MASK  ; (send a 0 bit)
  and VIA_PORTA

serial_send_count_bits:
  sta VIA_PORTA          ; output this bit
  dec serial_out_bit     ; remember bit count for later
  bne irq_done

; here when all bits have been sent
; disable the timer as we don't need it any more
; and this will be a flag that we can send another byte

  lda #VIA_FLAG_DISABLE | VIA_FLAG_TIMER1  ; cancel interrupt on T1 timeout
  sta VIA_IER
  lda VIA_T1C_L     ; clear interrupt flag

irq_done:
  plx   ; restore X
  pla   ; restore A
  rti
