  list 7

      jmp begin   ; skip the message
hello asciiz "Hello, world!"
begin = *
      lda #<hello
      ldx #>hello
      jsr print
      rts

