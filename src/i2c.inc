;
;  I2C interface routines
;
; Author: Nick Gammon
; Date: 2 March 2022

; PROTOCOL
;
;  BUS NOT BUSY: Both data and clock lines remain HIGH.
;
;  START DATA TRANSFER: A change in the state of the data line, from HIGH to LOW, while the clock is
;  HIGH, defines a START condition.
;
;  STOP DATA TRANSFER: A change in the state of the data line, from LOW to HIGH, while the clock line
;  is HIGH, defines the STOP condition.
;
;  DATA VALID: The state of the data line represents valid data when, after a START condition, the data
;  line is stable for the duration of the HIGH period of the clock signal. The data on the line must be
;  changed during the LOW period of the clock signal. There is one clock pulse per bit of data.
;  Each data transfer is initiated with a START co ndition and terminated with a STOP condition. The
;  number of data bytes transferred between START and STOP conditions is not limited, and is
;  determined by the master device. The information is transferred byte-wise and each receiver
;  acknowledges with a ninth bit.
;
;  CLOCK STRETCHING
;
;  If a slave wants us to wait, or another master is attempting to take over the bus, SCL will not go
;  high immediately after we set it to input (high impedance) so we therefore wait until SCL is in
;  fact high before proceeding.
;
;  ARBITRATION
;
;  If two masters are attempting to write to the bus we can detect that as soon as one of them writes
;  a one, but reads back a zero. This indicates that another master wrote the zero. The master that
;  read back the one then abandons its attempt to write (to try again later).
;
;  Since, up to this point, both have read back what they wrote, the bus is not corrupted.
;


; Configuration of which VIA ports are used by SCL and SDA

I2C_SCL     = %00000100   ; SCL (serial clock) - PA2
I2C_SDA     = %00001000   ; SDA (serial data)  - PA3

I2C_PORT    = VIA_PORTA   ; PORT A
I2C_DDR     = VIA_DDRA    ; DDR  A

;
;  Initialise hardware for I2C comms.
;
;  Set SCL and SDA to high, however make the ports input for now
;
i2c_init:

;
;  Set ports to low
;
  lda #I2C_SCL | I2C_SDA
  trb I2C_PORT

;
;  Set ports to input
;

  lda #I2C_SCL | I2C_SDA
  trb I2C_DDR


  rts

;---------------------------------------------
;  delay between clock bits - could make shorter if using a 1 MHz clock
;---------------------------------------------
i2c_delay:
    nop
    nop
    nop
    rts

;---------------------------------------------
;  toggle the SCL line from low to high and back again
;---------------------------------------------
i2c_toggle_clock:
  lda #I2C_SCL
  trb I2C_DDR      ; SCL high
  jsr i2c_delay
;
;  check if the slave wants us to wait by pulling the clock low
;
i2c_toggle_clock_wait:
  lda I2C_PORT
  and #I2C_SCL
  beq i2c_toggle_clock_wait   ; should be high, but is low
;
;  we let the clock go high, and it actually IS high, so we can proceed
;
  tsb I2C_DDR       ; SCL low
  lda #I2C_SDA      ; immediately set SDA to low
  trb I2C_DDR
  jsr i2c_delay
  rts

;---------------------------------------------
;
;  Begin transmission: send a start condition
;
;  Slave address in A, read/write bit in low-order bit
;    0 = write, 1 = read
;
;  Sets carry if OK
;
;  START CONDITION:
;
;  A change in the state of the data line, from HIGH to LOW, while the clock is HIGH
;---------------------------------------------

i2c_begin_transmission:

  pha             ; save address

  jsr i2c_init    ; set ports to low, and ports to input

;
;  Check if SDA or SCL are in use (low) - if so, wait until they are not
;
i2c_begin_transmission_loop:
  lda #I2C_SDA
  and I2C_PORT
  beq i2c_begin_transmission_loop
  lda #I2C_SCL
  and I2C_PORT
  beq i2c_begin_transmission_loop

;
;  The start condition pulls SDA low but SCL stays high
;

  lda #I2C_SDA
  tsb I2C_DDR
  jsr i2c_delay

;
;  But now we pull SCL low ready for clocking out bits
;

  lda #I2C_SCL
  tsb I2C_DDR
  jsr i2c_delay

  pla   ; get address back

  jsr i2c_write

  rts

;---------------------------------------------
;
;  I2C write: send 8 data bits, wait for acknowledge
;
;---------------------------------------------

i2c_write:
  ldx #8  ; count of bits
  tay

i2c_write_loop:
  tya
  rol a   ; get bit to write into carry
  tay

  bcs i2c_write_one
;
;  write a zero by setting SDA to output
;
  lda #I2C_SDA
  tsb I2C_DDR
  bra i2c_write_clock
;
;  write a one by setting SDA to input
;
i2c_write_one:
  lda #I2C_SDA
  trb I2C_DDR
;
;  if SDA is not high, another master has taken control of the bus, abandon this write
;
  nop
  and I2C_PORT
  beq i2c_write_lost_arbitration   ; error return, give up

;---------------------------------------------
;
;  toggle clock
;
;---------------------------------------------
i2c_write_clock:
  jsr i2c_toggle_clock
;
;  back for another bit?
;
  dex
  bne i2c_write_loop

  ;---------------------------------------------
  ;  wait for acknowledge bit
  ;---------------------------------------------

  ;
  ;  make SDA input and see if it is pulled low
  ;

  lda #I2C_SDA
  trb I2C_DDR         ; SDA to input

  lda #I2C_SCL        ; clock high
  trb I2C_DDR

;
;  check if the slave wants us to wait by pulling the clock low
;
i2c_write_clock_wait:
  lda I2C_PORT
  and #I2C_SCL
  beq i2c_write_clock_wait   ; should be high, but is low

  jsr i2c_delay       ; give it time to respond

  lda #I2C_SDA
  and I2C_PORT        ; read SDA

  tax                 ; save result for a moment
  lda #I2C_SCL        ; clock low
  tsb I2C_DDR

  txa             ; get ack/nak bit back

  beq i2c_write_got_ack

i2c_write_lost_arbitration:
  clc   ; error return
  rts

i2c_write_got_ack:
  sec   ; carry means OK
  rts


;---------------------------------------------
;
;  I2C read: receive 8 data bits, then acknowledge unless hardware_work is zero
;    hardware_work will be the count of outstanding bytes to be sent
;    on the last bit we NAK it to tell the sender not to send any more
;
;---------------------------------------------

i2c_read:
  ldx #8  ; count of bits
  ldy #0  ; current result

  lda #I2C_SDA
  trb I2C_DDR         ; SDA to input

i2c_read_loop:

  lda #I2C_SCL        ; clock high
  trb I2C_DDR
  jsr i2c_delay       ; give it time to respond

  lda #I2C_SDA
  and I2C_PORT        ; read SDA
  beq i2c_read_zero
;
;  read a 1
;
  tya       ; get previous result
  sec
  rol a     ; rotate in a one-bit
  tay       ; keep for later
  bra i2c_read_next

;
;  read a 0
;
i2c_read_zero:
  tya       ; get previous result
  clc
  rol a     ; rotate in a zero-bit
  tay       ; keep for later

i2c_read_next:

  lda #I2C_SCL        ; clock low
  tsb I2C_DDR
  jsr i2c_delay       ; give it time to respond

;
;  back for another bit?
;
  dex
  bne i2c_read_loop

  lda hardware_work   ; NAK on last read so they stop sending
  beq i2c_read_nak

  lda #I2C_SDA
  tsb I2C_DDR         ; pull SDA low to ACK the byte
  bra i2c_read_ack_or_nak

i2c_read_nak:
  lda #I2C_SDA
  trb I2C_DDR         ; pull SDA high to NAK the final byte

i2c_read_ack_or_nak:

  lda #I2C_SCL        ; clock high
  trb I2C_DDR

;
;  check if the slave wants us to wait by pulling the clock low
;   (clock stretching)
;

i2c_read_ack_or_nak_wait:
  lda I2C_PORT
  and #I2C_SCL
  beq i2c_read_ack_or_nak_wait   ; should be high, but is low

  jsr i2c_delay       ; give it time to respond

  lda #I2C_SCL        ; clock low
  tsb I2C_DDR
  jsr i2c_delay

i2c_read_done:

  tya                 ; result in A
  rts


;---------------------------------------------
;  end transmission - send stop condition
;
;  STOP CONDITION:
;
;  A change in the state of the data line, from LOW to HIGH, while the clock is HIGH
;---------------------------------------------

i2c_end_transmission:

;
;  make sure SDA is low, otherwise it can't transition low to high
;
  lda #I2C_SDA
  tsb I2C_DDR

  jsr i2c_delay

;
;  Set SCL to input so it floats high
;
  lda #I2C_SCL
  trb I2C_DDR

  jsr i2c_delay

;
;  Now with SCL high, set SDA to input so it also floats high
;
  lda #I2C_SDA
  trb I2C_DDR

  jsr i2c_delay

;
;  Both SDA and SCL are now high (by the pull-up resistors) so we are done
;
  rts

;---------------------------------------------
;
;  Send a message, address in A (excluding write bit)
;  Message in (VALUE)
;  Length in Y
;  Returns carry set on success, carry clear on failure
;
;---------------------------------------------
i2c_send:
  sty hardware_work
  asl A     ; move zero into write bit
  jsr i2c_begin_transmission
  bcc i2c_send_fail

;
;  send each byte
;
i2c_send_loop:
  lda hardware_work           ; number of bytes to go
  beq i2c_send_done
  dec hardware_work
  ldy #0
  lda (VALUE),Y
  jsr i2c_write
  bcc i2c_send_fail
  inc VALUE
  bne i2c_send_loop
  inc VALUE+1
  bra i2c_send_loop
;
;  sending done
;
i2c_send_done:
  jsr i2c_end_transmission
  sec
  rts
;
;  sending failure
;
i2c_send_fail:
  jsr i2c_end_transmission
  clc
  rts

;---------------------------------------------
;
;  Receive messave, slave address in A (excluding write bit)
;  Message in (VALUE)
;  Length in Y
;---------------------------------------------

i2c_receive:
  sty hardware_work
  sec
  rol A   ; move one into write bit (so it is read mode)
  jsr i2c_begin_transmission
  bcc i2c_receive_fail

;
;  put serial data (SDA) into input mode so we can read from it
;
  lda #I2C_SDA
  trb I2C_DDR

;
;  receive each byte
;
i2c_receive_loop:
  lda hardware_work           ; number of bytes to go
  beq i2c_receive_done
  dec hardware_work
  jsr i2c_read
  ldy #0
  sta (VALUE),Y
  inc VALUE
  bne i2c_receive_loop
  inc VALUE+1
  bra i2c_receive_loop
;
;  receiving done
;
i2c_receive_done:
  jsr i2c_end_transmission
  sec
  rts
;
;  receiving failure
;
i2c_receive_fail:
  jsr i2c_end_transmission
  clc
  rts

