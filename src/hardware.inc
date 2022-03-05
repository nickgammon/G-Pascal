
; VIA - Versatile Interface Adapter (W65C22) hardware

VIA_PORTB   = $7FF0   ; input/output register "B"
VIA_PORTA   = $7FF1   ; input/output register "A"
VIA_DDRB    = $7FF2   ; data direction register "B"
VIA_DDRA    = $7FF3   ; data direction register "A"
VIA_T1C_L   = $7FF4   ; T1 write: latch (L), read: counter (L)
VIA_T1C_H   = $7FF5   ; T1 high_order counter
VIA_T1L_L   = $7FF6   ; T1 low_order latches
VIA_T1L_H   = $7FF7   ; T1 high_order latches
VIA_T2C_L   = $7FF8   ; T2 write: latch (L), read: counter (L)
VIA_T2C_H   = $7FF9   ; T2 high_order counter
VIA_SR      = $7FFA   ; shift register
VIA_ACR     = $7FFB   ; auxilliary control register
VIA_PCR     = $7FFC   ; peripheral control register
VIA_IFR     = $7FFD   ; interrupt flag register
VIA_IER     = $7FFE   ; interrupt enable register

VIA_FLAG_ENABLE   = %10000000 ; high-order bit set enables the flag
VIA_FLAG_DISABLE  = %00000000 ; high-order bit clear disables the flag
VIA_FLAG_TIMER1   = %01000000
VIA_FLAG_TIMER2   = %00100000
VIA_FLAG_CB1      = %00010000
VIA_FLAG_CB2      = %00001000
VIA_FLAG_SHIFT    = %00000100
VIA_FLAG_CA1      = %00000010
VIA_FLAG_CA2      = %00000001

;
; VIA Port A pin masks
;

SERIAL_IN_MASK     = %00000001   ; serial in on PA0 (also connect to CB2) (to FTDI TxD pin - pin 4)
SERIAL_OUT_MASK    = %00000010   ; serial out on PA1 (to FTDI RxD pin - pin 5)
SERIAL_DEBUG1_MASK = %00000100   ; serial debugging on PA2
SERIAL_DEBUG2_MASK = %00001000   ; serial debugging on PA3
SERIAL_SPARE2_MASK = %00010000   ; spare VIA pins (PA4)


;***********************************************
;  Functions added for Ben's Board
;***********************************************

;
;  Converts a pin number in the range 0 to 15 to a mask
;  eg. 00000001, 00000010, 00000100 and so on
;
;  Returns mask in Y
;
;  Returns with Z set if the range was 0 to 7 an
;  and Z clear if the range was 8 to 15
;
pin_number_to_mask = *
  pha         ; save for a moment
  and #$7     ; make in range 0 to 7
  tay
  lda #1      ; initial mask
pin_number_to_mask_loop:
  cpy #0
  beq pin_number_to_mask_done
  asl A
  dey
  bra pin_number_to_mask_loop
pin_number_to_mask_done:
  tay         ; save mask in Y
  pla
  and #$8     ; DDRA or DDRB ?
  rts

;
;  pin in A, mode in X
;
pinmode:
  stx hardware_work ; input or output
  jsr pin_number_to_mask  ; convert to appropriate mask bit
  bne pinmode_B        ; Z is clear if it is DDRB
  lda #<VIA_DDRA
  sta REG
  lda #>VIA_DDRA
  sta REG+1
  bra pinmode_C
pinmode_B:
  lda #<VIA_DDRB
  sta REG
  lda #>VIA_DDRB
  sta REG+1
;
;  The DDR register is now in REG
;
pinmode_C:
  lda hardware_work     ; input or output?
  bne pinmode_OUTPUT
;
;  here to set input mode
;
  tya       ; get the mask back
  ldy #0
  eor #$FF  ; negate the mask
  and (REG),y
  sta (REG),y
  rts       ; done
;
;  here to set output mode
;
pinmode_OUTPUT:
  tya          ; get the mask back
  ldy #0
  ora (REG),y  ; or in with the existing DDR register
  sta (REG),y  ; and write it back
  rts          ; done


;
;  DIGITALWRITE (pin, value) pin:0 to 15; value: 0 or 1 : pin in A, mode in X
;
digitalwrite = *
  stx hardware_work ; save what to write (0 or 1)
  jsr pin_number_to_mask  ; convert to appropriate mask bit
  bne digitalwrite_B        ; Z is clear if it is PORTB
  lda #<VIA_PORTA
  sta REG
  lda #>VIA_PORTA
  sta REG+1
  bra digitalwrite_C
digitalwrite_B:
  lda #<VIA_PORTB
  sta REG
  lda #>VIA_PORTB
  sta REG+1
;
;  The PORT register is now in REG
;
digitalwrite_C:
  lda hardware_work
  bne digitalwrite_ONE  ; any non-zero value is writing a 1
;
;  here to write a zero
;
  tya
  ldy #0
  eor #$FF  ; negate the mask
  and (REG),y
  sta (REG),y
  rts       ; done

;
;  here to write a one
;
digitalwrite_ONE:
  tya          ; get the mask back
  ldy #0
  ora (REG),y  ; or in with the existing DDR register
  sta (REG),y  ; and write it back
  rts          ; done


;
;  DIGITALREAD (pin) pin:0 to 15; Pin to read is in in A. returns or non-zero in A.
;
digitalread = *
  jsr pin_number_to_mask  ; convert to appropriate mask bit
  bne digitalread_B        ; Z is clear if it is PORTB
  lda #<VIA_PORTA
  sta REG
  lda #>VIA_PORTA
  sta REG+1
  bra digitalread_C
digitalread_B:
  lda #<VIA_PORTB
  sta REG
  lda #>VIA_PORTB
  sta REG+1
;
;  The PORT register is now in REG
;
digitalread_C:
  tya         ; get the mask back
  ldy #0
  and (REG),y ; get that bit
  rts



;
;  Functions to change where writing is done
;
write_to_serial:
  pha
  lda  #<write_char
  sta  write_function
  lda  #>write_char
  sta  write_function+1
  pla
  rts

write_to_lcd:
  pha
  lda  #<lcd_print_char
  sta  write_function
  lda  #>lcd_print_char
  sta  write_function+1
  pla
  rts



; ---------------------------
; Simulate kernel interfaces
; ---------------------------
; We interface by reading/writing to special
; RAM that is implemented in the emulator



    .if EMULATOR

;
;  Emulator testing
;
CHRIN    =  *
         lda  $7F22
         rts

write_char   =  *
         sta  $7F23
         rts


GETIN    =  *
         lda  $7F29
         rts

hardware_init rts

;
;  put debug number in A
;
EMULATOR_DEBUG = *
         sta $7F46
         rts

START_TRACE = *
         sta $7F47
         rts

STOP_TRACE = *
         sta $7F48
         rts
;
;  not applicable in emulator
;
lcd_print_message   rts
lcd_clear_display   rts
lcd_print_char      rts
lcd_second_line     rts
lcd_instruction     rts
lcd_get_address     rts
lcd_wait            rts
lcd_home            rts
lcd_initialise      rts
serial_available    rts
serial_print_message rts

  .else

;
;  Ben's Board
;

;
;  typing_latency could be used as a seed for the random number generator
;    - the exact time between keypresses would be somewhat unpredictable
;
CHRIN  = *
GETIN    =  *
         inc typing_latency
         bne GETIN1
         inc typing_latency+1
         bne GETIN1
         inc typing_latency+2
GETIN1:
         lda serial_in_byte_received
         beq GETIN    ; nothing yet
         stz serial_in_byte_received
         rts

serial_available = *
         lda serial_in_byte_received
         rts


EMULATOR_DEBUG rts
START_TRACE rts
STOP_TRACE rts


;
;  set up serial hardware
;

hardware_init:

  lda #SERIAL_OUT_MASK  ; set 1 bit (idle serial comms), and others to zero
  sta VIA_PORTA

  ; set output enable bits
  lda #(SERIAL_OUT_MASK | SERIAL_DEBUG1_MASK | SERIAL_DEBUG2_MASK | SERIAL_SPARE2_MASK)
  sta VIA_DDRA

  lda #%01000000      ; continuous interrupts on T1
  sta VIA_ACR

  lda #0              ; Input-negative active edge
  sta VIA_PCR

  lda #<BIT_INTERVAL  ; ie. baud rate interval
  sta VIA_T1C_L       ; for sending bits
  lda #>BIT_INTERVAL
  sta VIA_T1C_H       ; this starts the counter

  ; set up for serial input

  lda #VIA_FLAG_ENABLE | VIA_FLAG_CB2  ; interrupt on CB2 falling edge
  sta VIA_IER

  lda #$FF         ; clear all interrupt flags
  sta VIA_IFR

  .if LCD_SUPPORT
  jsr lcd_initialise
  .endif

  stz serial_in_byte    ; non-zero means an interrupt put in incoming byte there
  stz serial_out_bit

  cli         ; enable interrupts
  rts


;
;  Write a character (in A) to the serial port.
;   Interrupts MUST be enabled for this to work!
;
write_char:
  pha                   ; save original character
  asl a                 ; shift in a zero which will be the start bit
  sta serial_out_byte
  lda #0xff             ; will become the stop bit(s)
  rol a                 ; rotate in the high-order bit from the data byte
  sta serial_out_byte+1
  lda #11               ; 8 data bits, one start bit, two stop bits
  sta serial_out_bit    ; number of bits to shift out
  lda #<BIT_INTERVAL    ; ie. baud rate interval
  sta VIA_T1C_L         ; for sending bits
  lda #>BIT_INTERVAL
  sta VIA_T1C_H         ; this starts the counter
  lda #VIA_FLAG_ENABLE | VIA_FLAG_TIMER1  ; interrupt on T1 timeout
  sta VIA_IER

;
; wait for this byte to finish - otherwise it might interfere with many things
;

write_char_loop:
  lda VIA_IER
  and #VIA_FLAG_TIMER1  ; are T1 interrupts enabled?
  bne write_char_loop   ; loop until this character sent
  pla                   ; get character back
  rts


;
;  sends null-terminated message to serial port, message in A (lo) and X (hi)
;  returns the length of the message in Y
;
serial_print_message:
  sta REG2
  stx REG2+1
  ldy #0
serial_print:
  lda (REG2),y
  beq serial_print_done
  jsr write_char
  iny
  bra serial_print

serial_print_done rts

  .endif  ; not EMULATOR

