  list 7

; interface routines in the EEPROM
serial_print_message = $8012

      jmp start   ; skip the message
hello asciiz "Hello, world!"
start = *
      lda #<hello
      ldx #>hello
      jsr serial_print_message
      rts

