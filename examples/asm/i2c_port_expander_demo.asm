; I2C testing of MCP23017 port-expander
;

  jmp begin      ; skip the variables

; MCP23017 registers

IODIRA   = $00   ; IO direction  (0 = output, 1 = input (Default))
IOCON    = $0A   ; IO Configuration: bank/mirror/seqop/disslw/haen/odr/intpol/notimp
GPIOA    = $12   ; Port value. Write to change, read to obtain value

;
;  Change this depending on how you set up the address jumpers. Value $20 to $27
;    - I had all mine set to high (+5V)
;
PORT     = $27  ; MCP23017 is on I2C port 0x27

;                            register value A    value B
io_configuration_message  dfb IOCON,  %00100000, %00100000  ; byte mode (not sequential)
io_direction_message      dfb IODIRA, $0,        $0         ; 1 = input, 0 = output
io_output_message         dfb GPIOA,  $22,       $00        ; a pattern to show on the LEDs

failure_message   string "Got NAK\n"
starting_message  string "Starting I2C port-expander test\n"

;---------------------------------------------
;  BEGIN code execution
;---------------------------------------------

begin:
  lda #<starting_message
  ldx #>starting_message
  jsr print

  jsr i2c_init

;
;  configuration
;
  lda #<io_configuration_message
  sta VALUE
  lda #>io_configuration_message
  sta VALUE+1
  ldy #3
  lda #PORT
  jsr i2c_send
  bcc i2c_failure


;
;  Set ports to output
;

  lda #<io_direction_message
  sta VALUE
  lda #>io_direction_message
  sta VALUE+1
  ldy #3
  lda #PORT
  jsr i2c_send
  bcc i2c_failure

loop:

  jsr serial_available
  cmp #'C'-$40  ; ctrl+C?
  bne loop_no_abort
  rts   ; we are done!

loop_no_abort:

  lda #<io_output_message
  sta VALUE
  lda #>io_output_message
  sta VALUE+1
  ldy #3
  lda #PORT
  jsr i2c_send
  bcc i2c_failure

;
;  ripple pattern
;
  lda io_output_message+2
  ror a
  rol io_output_message+1
  rol io_output_message+2

loop_delay:
  ldx #<200
  ldy #>200
  jsr delay
  bra loop

i2c_failure:
  lda #<failure_message
  ldx #>failure_message
  jsr print
  bra loop_delay

