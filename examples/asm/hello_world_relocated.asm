  SYM $5800     ; symbols are to be placed at $5800 (growing downwards)
  ORG $5800     ; code is to be placed at $5800 (growing upwards)

; interface routines in the EEPROM
serial_print_message = $8012

      jmp start   ; skip the message
hello ascii "Hello, world!"
      dfb   $0A
      dfb   0
start = *
      lda #<hello
      ldx #>hello
      jsr serial_print_message
      rts

