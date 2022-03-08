; I2C DS1307 read clock test
;

    jmp begin      ; skip the variables

CLOCK_PORT     = $68  ; DS1307 is on I2C port 0x68


failure_message   string "Got NAK\n"
starting_message  string "Starting I2C clock test (date format: DD/MM/YY)\n"

select_register_message dfb $00    ; set to register $00

read_time_work BLK 7            ; 7-byte workarea for reading time

;---------------------------------------------
;
;  BEGIN of code
;
;---------------------------------------------

begin:
  lda #<starting_message
  ldx #>starting_message
  jsr print

  jsr i2c_init

loop:

  jsr serial_available
  cmp #'C'-$40  ; ctrl+C?
  bne loop_no_abort
  rts   ; we are done!

loop_no_abort:

  lda #<select_register_message    ; set to register zero
  sta VALUE
  lda #>select_register_message
  sta VALUE+1
  ldy #1
  lda #CLOCK_PORT               ; DS1307 port
  jsr i2c_send
  bcc i2c_failure

;
;  now read 7 registers
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
;  display time
;
loop_confirm:
  lda read_time_work+2  ; hour
  jsr PRBYTE
  lda #':'
  jsr COUT
  lda read_time_work+1  ; minute
  jsr PRBYTE
  lda #':'
  jsr COUT
  lda read_time_work+0  ; second
  jsr PRBYTE
  jsr PUTSP

  lda read_time_work+4  ; day
  jsr PRBYTE
  lda #'/'
  jsr COUT
  lda read_time_work+5  ; month
  jsr PRBYTE
  lda #'/'
  jsr COUT
  lda read_time_work+6  ; year
  jsr PRBYTE
  jsr PUTSP
  jsr CROUT

;
;  delay 2 seconds with 1 Mhz clock
;
loop_delay:
  ldx #<2000
  ldy #>2000
  jsr delay
  bra loop

i2c_failure:
  lda #<failure_message
  ldx #>failure_message
  jsr print
  bra loop_delay
