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

; our variables
counter dfw 0
display_work dfb 0

;---------------------------------------------
; begin test code
;---------------------------------------------
begin:
  lda #0          ; mode 0 SPI
  jsr spi_init
  lda #MAX7219_REG_SCANLIMIT    ; show 8 digits
  ldx #7
  jsr spi_send_two_bytes
  lda #MAX7219_REG_DECODEMODE   ; use digits (not bit patterns)
  ldx #$FF
  jsr spi_send_two_bytes
  lda #MAX7219_REG_DISPLAYTEST  ; no display test
  ldx #0
  jsr spi_send_two_bytes
  lda #MAX7219_REG_INTENSITY    ; character intensity: range: 0 to 15
  ldx #7
  jsr spi_send_two_bytes
  lda #MAX7219_REG_SHUTDOWN     ; not in shutdown mode (ie. start it up)
  ldx #1
  jsr spi_send_two_bytes

display_loop:

  jsr serial_available
  cmp #'C'-$40  ; ctrl+C?
  bne loop_no_abort
  rts   ; we are done!

loop_no_abort:
;
;  we will display an incrementing number
;
  inc counter
  bne display_loop1
  inc counter+1
;
;  turn the number into decimal digits
;
display_loop1:
  lda counter
  sta VALUE
  lda counter+1
  sta VALUE+1
  stz VALUE+2
  jsr binary_to_decimal

;
;  the number is in bcd_result, take each byte, remove the high-order bits
;  and send to the display
;
  ldy #0              ; start with the high-order digit
  lda #8
  sta display_work
display_loop2:
  lda bcd_result,y
  and #$0f
  tax
  phy
  lda display_work    ; will be sending to 7,6,5,4,3,2,1,0 in that order
  dec display_work
  jsr spi_send_two_bytes
  ply
  iny
  cpy #8              ; stop after 8 digits
  bne display_loop2

;
;  wait briefly
;
loop_delay:
  ldx #<400
  ldy #>400
  jsr delay
  bra display_loop

