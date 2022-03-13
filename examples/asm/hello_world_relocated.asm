  SYM $5800     ; symbols are to be placed at $5800 (growing downwards)
  ORG $5800     ; code is to be placed at $5800 (growing upwards)

  jmp start   ; skip the message
hello asciiz "Hello, world!\n"

start:
  lda #<hello
  ldx #>hello
  jsr serial_print_message
  rts
