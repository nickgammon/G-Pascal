;
;  SPI interface routines
;
; Author: Nick Gammon
; Date: 6 March 2022
;

; Configuration of which VIA ports are used by SS, MOSI, MISO, SCK

SPI_SS    = %00000001   ; SS   (slave select) - PB0
SPI_MOSI  = %00000010   ; MOSI (master out, slave in) - PB1
SPI_MISO  = %00000100   ; MISO (master in, slave out) - PB2
SPI_SCK   = %00001000   ; SCK  (serial clock) - PB3

SPI_PORT    = VIA_PORTB   ; PORT B
SPI_DDR     = VIA_DDRB    ; DDR  B

;---------------------------------------------
;  Initialise hardware for SPI comms.
;
;  SPI mode in A, as follows:

;   Mode 0 - clock is normally low,  the data is sampled on the transition from low to high (leading edge)
;   Mode 1 - clock is normally low,  the data is sampled on the transition from high to low (trailing edge)
;   Mode 2 - clock is normally high, the data is sampled on the transition from high to low (leading edge)
;   Mode 3 - clock is normally high, the data is sampled on the transition from low to high (trailing edge)
;
;  Set SS, MOSI and SCK to outputs. Set MISO to input.
;---------------------------------------------
spi_init:

  sta spi_mode

;
;  Set MISO to input
;

  lda #SPI_MISO
  trb SPI_DDR

;
;  Set SS to high (it is active low)
;
  lda #SPI_SS
  tsb SPI_PORT

;
; In modes 0 and 1, the clock is normally low
;
  lda spi_mode
  and #2
  bne spi_init_clock_high

  lda #SPI_SCK
  trb SPI_PORT
  bra spi_init2

;
; In modes 2 and 3, the clock is normally high
;
spi_init_clock_high:
  lda #SPI_SCK
  tsb SPI_PORT
;
;  Now set up MOSI
;

spi_init2:

  lda spi_mode
  and #1
  bne spi_init_mosi_low

;
;  In modes 0 and 2, MOSI idles high
;
  lda #SPI_MOSI
  tsb SPI_PORT
  bra spi_init3

;
;  In modes 1 and 3, MOSI idles low
;

spi_init_mosi_low:
  lda #SPI_MOSI
  trb SPI_PORT

;
;   Set SS, MOSI and SCK to outputs
;

spi_init3:
  lda #SPI_SS | SPI_MOSI | SPI_SCK
  tsb SPI_DDR

  rts

;---------------------------------------------
;  Transfer one byte.
;    Byte to be sent in A.
;    Returns received byte in A.
;    Preserves X and Y
;
;   Mode 0 - clock is normally low,  the data is sampled on the transition from low to high (leading edge)
;   Mode 1 - clock is normally low,  the data is sampled on the transition from high to low (trailing edge)
;   Mode 2 - clock is normally high, the data is sampled on the transition from high to low (leading edge)
;   Mode 3 - clock is normally high, the data is sampled on the transition from low to high (trailing edge)
;---------------------------------------------

spi_transfer:
  phx
  phy
  ldx  #8               ; count of bits
  tay                   ; save the byte we are sending
spi_transfer_loop:
  tya                   ; get the byte we are sending back
;
;  set up MOSI, ready to be sampled on the clock transition
;
  rol A                 ; get MSB into carry
  tay                   ; save the byte which we shifted left
  bcc spi_send_zero     ; if no carry, we are sending a zero

;
; here to send a one bit
;
  lda #SPI_MOSI
  tsb SPI_PORT    ; set MOSI to high
  bra spi_send_clock_pulse_1

;
; here to send a zero bit
;
spi_send_zero:
  lda #SPI_MOSI
  trb SPI_PORT    ; set MOSI to low

;
; transition clock
;
spi_send_clock_pulse_1:

;
; In modes 0 and 1, the clock is normally low, so we need to make it high
;
  lda spi_mode
  and #2
  bne spi_send_clock_high_low

;
; Make SCK high
;
  lda #SPI_SCK
  tsb SPI_PORT
  bra spi_send2

;
; Otherwise we need to make it low
;
spi_send_clock_high_low:
;
;  Make SCK low
;
  lda #SPI_SCK
  trb SPI_PORT

;
;  We have done the leading edge of SCK
;
spi_send2:

;
;  In modes 1 and 3 we sample on the trailing edge, so do nothing here
;
  lda spi_mode
  and #1
  bne spi_send3

;
;  sample incoming data on leading edge
;
  clc
  lda #SPI_MISO
  and SPI_PORT
  beq spi_read_02_done
  sec
spi_read_02_done:
  rol hardware_work   ; rotate in zero or one from carry

spi_send3:

;
;  transition clock again to give the trailing edge
;

;
; In modes 0 and 1, the clock is normally low, so we need to make it low again
;
  lda spi_mode
  and #2
  bne spi_send_clock_low_high

  lda #SPI_SCK
  trb SPI_PORT
  bra spi_send4

spi_send_clock_low_high:
  lda #SPI_SCK
  tsb SPI_PORT

spi_send4:

;
;  In modes 1 and 3 we sample on the trailing edge
;
  lda spi_mode
  and #1
  beq spi_send_bit_done

;
;  sample incoming data on trailing edge
;
  clc
  lda #SPI_MISO
  and SPI_PORT
  beq spi_read_13_done
  sec
spi_read_13_done:
  rol hardware_work   ; rotate in zero or one from carry

spi_send_bit_done:
  dex
  bne spi_transfer_loop

;
;  get incoming byte back into A
;
  ply
  plx
  lda hardware_work
  rts


;---------------------------------------------
; Select the slave (send SS low) - preserves all registers
;---------------------------------------------
spi_ss_low:
  pha
  lda #SPI_SS
  trb SPI_PORT
  pla
  rts

;---------------------------------------------
; Deselect the slave (send SS high) - preserves all registers
;---------------------------------------------
spi_ss_high:
  pha
  lda #SPI_SS
  tsb SPI_PORT
  pla
  rts

;---------------------------------------------
;  send two bytes: first in A, second in X
;   preserves Y
;---------------------------------------------
spi_send_two_bytes:
  jsr spi_ss_low    ; SS low
  jsr spi_transfer  ; send first byte
  txa               ; get second byte
  jsr spi_transfer  ; send second byte
  jsr spi_ss_high   ; SS high again
  rts

