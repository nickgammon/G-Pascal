; I2C set DS1307 clock test
;

    jmp begin      ; skip the variables

CLOCK_PORT     = $68  ; DS1307 is on I2C port 0x68

;
;  Setting time - numbers in in BCD (binary coded decimal) so they must be entered in hex
;
;  Example shows 07:49:45 on 05/03/22
;

SET_HOUR         = $07   ; $00 to $23. For 12-hour clock see datasheet
SET_MINUTE       = $49   ; $00 to $59
SET_SECOND       = $45   ; $00 to $59
SET_DAY_OF_WEEK  = $06   ; $01 to $07. Saturday in this case if Monday is $01
SET_DAY          = $05   ; $01 to $31.
SET_MONTH        = $03   ; $01 to $12
SET_YEAR         = $22   ; $00 to $99



failure_message    string "Got NAK\n"
starting_message   string "Attempting to set clock ...\n"
done_message       string "Done\n"

set_time_message:
  dfb $00                             ; start with register zero
  dfb SET_SECOND,SET_MINUTE,SET_HOUR  ; second, minute, hour
  dfb SET_DAY_OF_WEEK                 ; day of week: 1 = Sunday or Monday,  your choice
  dfb SET_DAY,SET_MONTH,SET_YEAR      ; day / month / year

;---------------------------------------------
;  BEGIN of code
;---------------------------------------------

begin:
  lda #<starting_message
  ldx #>starting_message
  jsr print
  jsr i2c_init

;
;  set clock
;
  lda #<set_time_message
  sta VALUE
  lda #>set_time_message
  sta VALUE+1
  ldy #8                ; length: register number + 7 data bytes
  lda #CLOCK_PORT       ; DS1307 port
  jsr i2c_send_message
  bcc i2c_failure

  lda #<done_message
  ldx #>done_message
  jsr print
  rts

i2c_failure:
  lda #<failure_message
  ldx #>failure_message
  jsr print
  rts
