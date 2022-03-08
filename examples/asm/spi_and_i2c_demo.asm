;---------------------------------------------
;  Demonstration of doing three things in sequence:
;
;  1. Rotate the output of a MCP23017 chip (with LEDs on the 16 outputs)
;  2. Read the time on a DS1307 I2C clock chip
;  3. Show the time on a 8-digit 7-segment display which has a MAX7219 chip on it
;     using SPI
;---------------------------------------------


  jmp begin   ; skip past our variables

;---------------------------------------------
;  I2C stuff
;---------------------------------------------

;---------------------------------------------
;   DS1307 stuff
;---------------------------------------------

CLOCK_PORT     = $68  ; DS1307 is on I2C port 0x68

failure_message   string "Got NAK\n"
select_register_message dfb $00    ; set to register $00
read_time_work BLK 7               ; 7-byte workarea for reading time

;---------------------------------------------
; MCP23017 registers
;---------------------------------------------

IODIRA   = $00   ; IO direction  (0 = output, 1 = input (Default))
IOCON    = $0A   ; IO Configuration: bank/mirror/seqop/disslw/haen/odr/intpol/notimp
GPIOA    = $12   ; Port value. Write to change, read to obtain value

;
;  Change this depending on how you set up the address jumpers. Value $20 to $27
;    - I had all mine set to high (+5V)
;
MCP23017_PORT     = $27  ; MCP23017 is on I2C port 0x27

;                            register value A    value B
io_configuration_message  dfb IOCON,  %00100000, %00100000  ; byte mode (not sequential)
io_direction_message      dfb IODIRA, $0,        $0         ; 1 = input, 0 = output
io_output_message         dfb GPIOA,  $22,       $00        ; a pattern to show on the LEDs

;---------------------------------------------
;  SPI stuff: MAX7219 registers
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

;
;  jmp for out-of-range branches
;
i2c_failureJ jmp i2c_failure

;---------------------------------------------
; begin code
;---------------------------------------------
begin:

;
;  SPI initialization
;
  lda #0          ; mode 0 SPI
  jsr spi_init
  lda #MAX7219_REG_SCANLIMIT    ; show 6 digits
  ldx #5
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

;
;  I2C initialization
;
  jsr i2c_init

;
;  configuration of MCP23017
;
  lda #<io_configuration_message
  sta VALUE
  lda #>io_configuration_message
  sta VALUE+1
  ldy #3
  lda #MCP23017_PORT
  jsr i2c_send
  bcc i2c_failureJ

;
;  Set ports to output on MCP23017
;

  lda #<io_direction_message
  sta VALUE
  lda #>io_direction_message
  sta VALUE+1
  ldy #3
  lda #MCP23017_PORT
  jsr i2c_send
  bcc i2c_failureJ

display_loop:

  jsr serial_available
  cmp #'C'-$40  ; ctrl+C?
  bne loop_no_abort
  rts   ; we are done!

loop_no_abort:

;
;  show a pattern on the LEDs using the MCP23017
;
  lda #<io_output_message
  sta VALUE
  lda #>io_output_message
  sta VALUE+1
  ldy #3
  lda #MCP23017_PORT
  jsr i2c_send
  bcc i2c_failureJ

;
;  ripple pattern by rotating it
;
  lda io_output_message+2
  ror a
  rol io_output_message+1
  rol io_output_message+2

;
;  now tell the DS1307 clock to go to register zero
;

  lda #<select_register_message    ; set to register zero
  sta VALUE
  lda #>select_register_message
  sta VALUE+1
  ldy #1
  lda #CLOCK_PORT               ; DS1307 port
  jsr i2c_send
  bcc i2c_failure

;
;  now read 7 DS1307 registers
;

  lda #<read_time_work
  sta VALUE
  lda #>read_time_work
  sta VALUE+1
  ldy #7
  lda #CLOCK_PORT               ; DS1307 port
  jsr i2c_receive
  bcc i2c_failure

;
;  Now show the time using the MAX7219 via SPI
;
  lda read_time_work+2  ; hour
  ror A                 ; rotate right 4 times to get high-order digit
  ror A
  ror A
  ror A
  and #$0F
  tax
  lda #6
  jsr spi_send_two_bytes

  lda read_time_work+2  ; hour
  and #$0F
  tax
  lda #5
  jsr spi_send_two_bytes

  lda read_time_work+1  ; minute
  ror A                 ; rotate right 4 times to get high-order digit
  ror A
  ror A
  ror A
  and #$0F
  tax
  lda #4
  jsr spi_send_two_bytes

  lda read_time_work+1  ; minute
  and #$0F
  tax
  lda #3
  jsr spi_send_two_bytes

 lda read_time_work+0  ; second
  ror A                ; rotate right 4 times to get high-order digit
  ror A
  ror A
  ror A
  and #$0F
  tax
  lda #2
  jsr spi_send_two_bytes

  lda read_time_work+0  ; second
  and #$0F
  tax
  lda #1
  jsr spi_send_two_bytes

;
;  wait briefly
;
loop_delay:
  ldx #<400
  ldy #>400
  jsr delay
  jmp display_loop


i2c_failure:
  lda #<failure_message
  ldx #>failure_message
  jsr print
  jmp loop_delay
