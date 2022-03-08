;---------------------------------------------
;
; Demonstration of writing a message to 8 x 64 pixel
;  dot matrix display, using 8 x MAX7219 chips
;
;---------------------------------------------

  jmp begin

;---------------------------------------------
;  MAX7219 registers
;---------------------------------------------

MAX7219_REG_NOOP        = $00  ; No operation - used for cascading multiple chips
MAX7219_REG_DIGIT0      = $01  ; Write to digit 1
MAX7219_REG_DIGIT1      = $02  ; Write to digit 2
MAX7219_REG_DIGIT2      = $03  ; Write to digit 3
MAX7219_REG_DIGIT3      = $04  ; Write to digit 4
MAX7219_REG_DIGIT4      = $05  ; Write to digit 5
MAX7219_REG_DIGIT5      = $06  ; Write to digit 6
MAX7219_REG_DIGIT6      = $07  ; Write to digit 7
MAX7219_REG_DIGIT7      = $08  ; Write to digit 8
MAX7219_REG_DECODEMODE  = $09  ; Decode mode for each digit: 1 = decode, 0 = no decode (one bit per digit)
MAX7219_REG_INTENSITY   = $0A  ; Intensity: 0x00 to 0x0F
MAX7219_REG_SCANLIMIT   = $0B  ; Scan limit: 0x00 to 0x07 - how many digits to display (ie. 1 to 8)
MAX7219_REG_SHUTDOWN    = $0C  ; Shutdown: 0x00 = do not display, 0x01 = display
MAX7219_REG_DISPLAYTEST = $0F  ; Display test: 0x00 = normal, 0x01 = display all segments

NUMBER_OF_CHIPS = 8

;---------------------------------------------
;  send two bytes to Y chips:
;   first in A (register), second in X (value)
;   Y is count of chips - preserves Y
;---------------------------------------------
send_to_all:
  phy
  jsr spi_ss_low    ; SS low
send_to_all_loop:
  pha
  jsr spi_transfer  ; send first byte
  txa               ; get second byte
  jsr spi_transfer  ; send second byte
  pla
  dey
  bne send_to_all_loop
  jsr spi_ss_high   ; SS high again
  ply
  rts


;---------------------------------------------
;  send one letter to one chip.
;  Letter in A, chip in X - preserves X and Y
;---------------------------------------------
send_letter:
  phy
  stx VALUE+2   ; which chip

; get the start of the character code by multiplying the letter we are displaying by 8 and
;  adding to the start of the cp437_font address
  sta VALUE       ; the letter code
  stz VALUE+1
  asl VALUE       ; times 2
  rol VALUE+1
  asl VALUE       ; times 4
  rol VALUE+1
  asl VALUE       ; times 8
  rol VALUE+1
  clc
  lda #<cp437_font
  adc VALUE
  sta VALUE
  lda #>cp437_font
  adc VALUE+1
  sta VALUE+1
  ldy #0            ; which column
send_letter_loop:
  jsr spi_ss_low    ; SS low
  ldx #0            ; what chip we are currently at
;
;  skip unwanted chips
;
send_letter_initial_nops:
  cpx VALUE+2       ; wanted chip?
  beq send_letter_got_wanted_chip
  phx
  lda #MAX7219_REG_NOOP            ; send a NOP
  jsr spi_transfer
  lda #MAX7219_REG_NOOP
  jsr spi_transfer
  plx
  inx
  bra send_letter_initial_nops

send_letter_got_wanted_chip:
  inx
  phx
  lda (VALUE),Y
  tax
  iny   ; make column 1-relative
  phy
  tya   ; which column
  jsr spi_transfer
  txa   ; the character to send
  jsr spi_transfer
  ply
  plx
;
;  now send more NOPs
;
send_letter_final_nops:
  cpx #NUMBER_OF_CHIPS      ; done all chips?
  beq send_letter_done
  phx
  lda #MAX7219_REG_NOOP            ; send a NOP
  jsr spi_transfer
  lda #MAX7219_REG_NOOP
  jsr spi_transfer
  plx
  inx
  bra send_letter_final_nops

send_letter_done:
  jsr spi_ss_high   ; SS high again
  cpy #8
  bne send_letter_loop
  ply
  ldx VALUE+2   ; get X back
  rts

;
;  Message to display
;

message asciiz "This is Ben's breadboard computer "

;---------------------------------------------
;
;  Get next character from "message", wrap if at end and get the first one
;---------------------------------------------
get_next_char:
  lda message,Y
  bne get_next_char_ok
  ldy #0
  lda message,Y
get_next_char_ok:
  iny
  rts

;---------------------------------------------
; begin code
;---------------------------------------------
begin:
  lda #0          ; mode 0 SPI
  jsr spi_init
;
;  send some NOPs to get us started
;
  ldy #NUMBER_OF_CHIPS
  lda #MAX7219_REG_NOOP
  ldx #MAX7219_REG_NOOP
  jsr send_to_all
;
;  short delay
;
  ldx #<1000
  ldy #>1000
  jsr delay

;
;  initialise MAX7219 chip
;
  lda #MAX7219_REG_SCANLIMIT    ; show 8 digits
  ldx #7
  jsr send_to_all
  lda #MAX7219_REG_DECODEMODE   ; use bit patterns
  ldx #0
  jsr send_to_all
  lda #MAX7219_REG_DISPLAYTEST  ; no display test
  ldx #0
  jsr send_to_all
  lda #MAX7219_REG_INTENSITY    ; character intensity: range: 0 to 15
  ldx #1
  jsr send_to_all
  lda #MAX7219_REG_SHUTDOWN     ; not in shutdown mode (ie. start it up)
  ldx #1
  jsr send_to_all
  ldy #0                  ; which letter in the string we are up to

display_loop:
;
;  abort on Ctrl+C
;
  jsr serial_available
  cmp #'C'-$40  ; ctrl+C?
  bne loop_no_abort
  lda #MAX7219_REG_SHUTDOWN     ; shut it down
  ldx #0
  jsr send_to_all
  rts   ; we are done!

loop_no_abort:
  phy
  ldx #0      ; which character in the display we are sending to (0 to NUMBER_OF_CHIPS)

;
;  send each letter
;
letter_loop:
  jsr get_next_char
  jsr send_letter
  inx
  cpx #NUMBER_OF_CHIPS
  bne letter_loop

;
;  wait briefly
;
loop_delay:
  ldx #<1000
  ldy #>1000
  jsr delay

;
;  next letter, go back to start if at 0x00 byte
;
  ply
  iny
  lda message,Y
  bne display_loop
  ldy #0
  bra display_loop
