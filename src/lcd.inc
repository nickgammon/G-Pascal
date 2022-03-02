  .if !EMULATOR
  .if LCD_SUPPORT

;
;  LCD stuff
;

lcd_initialise:
;
;  LCD initialise
;

  lda VIA_DDRA
  ora #(LCD_RW|LCD_RS|LCD_E)    ; set appropriate pins on Port A to output
  sta VIA_DDRA

  ; see HD44780U documentation: "Initializing by Instruction" (page 46)
  jsr lcd_prepare_to_write_instruction
  lda #$30
  jsr lcd_write_instruction_nibble
  ; wait > 4.1 ms
  ldx #20
  ldy #0
  jsr delay      ; 20 ms delay (in case of faster board clock speeds)
  jsr lcd_write_instruction_nibble
  jsr delay_1ms  ; do it 4 times in case of faster board clock speeds
  jsr delay_1ms
  jsr delay_1ms
  jsr delay_1ms
  jsr lcd_write_instruction_nibble
  jsr delay_1ms  ; do it 4 times in case of faster board clock speeds
  jsr delay_1ms
  jsr delay_1ms
  jsr delay_1ms
  lda #$20    ; switch to 4-bit mode
  jsr lcd_write_instruction_nibble
  jsr delay_1ms  ; do it 4 times in case of faster board clock speeds
  jsr delay_1ms
  jsr delay_1ms
  jsr delay_1ms
;
;  now we can switch to sending 8-bits in two nibbles
;
  lda #%00101000 ; Set 4-bit mode; 2-line display; 5x8 font
  jsr lcd_instruction
  lda #%00001110 ; Display on; cursor on; blink off
  jsr lcd_instruction
  lda #%00000110 ; Increment and shift cursor; don't shift display
  jsr lcd_instruction
  jsr lcd_clear_display
  rts

lcd_prepare_to_read_register:
  lda VIA_PORTA
  ora #LCD_RW             ; set RW bit  (ie. enable reading)
  and #<(~(LCD_RS|LCD_E)) ; Clear RS/E bits
  sta VIA_PORTA
  lda VIA_DDRB
  and #$0F
  sta VIA_DDRB            ; we are reading from DDRB
  rts

lcd_prepare_to_write_data:
  pha
  lda VIA_PORTA
  and #<(~(LCD_RW|LCD_E))  ; clear RW and E (enable writing)
  ora #LCD_RS              ; setting this means we write a character
  sta VIA_PORTA
  lda VIA_DDRB
  ora #$F0      ; Set high-order pins on port B to output
  sta VIA_DDRB
  pla
  rts

lcd_prepare_to_write_instruction:
  pha
  lda VIA_PORTA
  and #<(~(LCD_RW|LCD_RS|LCD_E))  ; clear RW, RS and E (enable writing)
  sta VIA_PORTA
  lda VIA_DDRB
  ora #$F0      ; Set high-order pins on port B to output
  sta VIA_DDRB
  pla
  rts

lcd_set_enable:
  lda VIA_PORTA
  ora #LCD_E
  sta VIA_PORTA   ; Set enable (E)
  rts

lcd_clear_enable:
  lda VIA_PORTA
  and #<(~LCD_E)  ; Clear enable (E)
  sta VIA_PORTA
  rts

lcd_read_instruction_nibble:
  jsr lcd_set_enable
  lda VIA_PORTB   ; read the data on port B
  pha             ; save the data
  jsr lcd_clear_enable
  pla             ; get the data back
  rts

;
; wait until LCD is not busy
;
lcd_wait:
  phx
  phy
  pha
  jsr lcd_prepare_to_read_register
  ldx #0
lcdbusy:
;
;  timeout after 256 iterations of this loop so we don't hang
;  if there is no LCD or it doesn't respond
;
  dex
  beq lcd_wait_timeout
  jsr lcd_read_instruction_nibble
  and #$80        ; get busy flag
  tay             ; save busy status
  jsr lcd_read_instruction_nibble   ; do second nibble
  tya             ; get busy status back
  bne lcdbusy     ; if busy go back
;
;  here when not busy any more
;
lcd_wait_timeout:
  pla
  ply
  plx
  rts

;
;  Find the current LCD (cursor) address - useful for knowing
;  if we exceed the limit of line 1
;
lcd_get_address:
  jsr lcd_prepare_to_read_register
  jsr lcd_read_instruction_nibble
  and #$70        ; ignore busy bit, mask out other junk
  sta lcd_work
  jsr lcd_read_instruction_nibble
  lsr A   ; shift this lot into low-order bits
  lsr A
  lsr A
  lsr A
  ora lcd_work    ; get the high-order bits back
  rts

lcd_write_instruction_nibble:
  pha
  sta VIA_PORTB         ; set up the data
  jsr lcd_set_enable    ; toggle enable
  jsr lcd_clear_enable
  pla
  rts

;
;  send an instruction to the LCD
;
lcd_instruction:
  jsr lcd_wait
  jsr lcd_prepare_to_write_instruction
  jsr lcd_write_instruction_nibble
  pha
  asl a   ; move the low-order bits into the high-order bits
  asl a   ; because the high-order 4 bits are the active ones
  asl a
  asl a
  jsr lcd_write_instruction_nibble
  pla
  rts

lcd_data_nibble:
  pha
  jsr lcd_prepare_to_write_data
  sta VIA_PORTB
  jsr lcd_set_enable
  jsr lcd_clear_enable
  pla
  rts

;
;  print a character to the LCD
;
lcd_print_char:
  jsr lcd_wait  ; wait for previous write to finish
  cmp #NL
  bne lcd_print_not_newline ; newline jumps to 2nd line
  jsr lcd_second_line       ; go to second line
  rts                       ; and don't print the newline :)

lcd_print_not_newline:
  pha                   ; save the character we want to print
  jsr lcd_get_address
  cmp #16         ; 16 characters on the first line
  bcc lcd_print_char_ok ; not there yet
  bne lcd_print_char_on_2nd_line  ; not < 16 and not == 16, must be > 16
  ; we have exactly 16, so drop down to address 64
  jsr lcd_second_line  ; at char 16 go to 2nd line (address 64)
  bra lcd_print_char_ok

lcd_print_char_on_2nd_line:
  cmp #80                  ; more than 16 characters on in 2nd line? (64 + 16)
  bcc lcd_print_char_ok    ; nah
  jsr lcd_clear_display    ; yes, clear display then and start again
lcd_print_char_ok:
lcd_print_char_newline:
  jsr lcd_prepare_to_write_data
  pla                      ; get the character back
  jsr lcd_data_nibble
  pha
  asl a     ; now send the low-order bits
  asl a
  asl a
  asl a
  jsr lcd_data_nibble
  pla
  rts

lcd_clear_display:
  pha
  lda #%00000001 ; Clear display
  jsr lcd_instruction
  pla
  rts

lcd_second_line:
  pha
  lda #$C0 ; set output address to 0x40 (set DDRAM address)
  jsr lcd_instruction
  pla
  rts

lcd_home:
  pha
  lda #$80 ; set output address to 0 (set DDRAM address)
  jsr lcd_instruction
  pla
  rts


;
;  print null-terminated message on LCD, message in A (lo) and X (hi)
;  returns the length of the message in Y
;
lcd_print_message:
  sta REG2
  stx REG2+1
  ldy #0
  jsr lcd_clear_display
lcd_print:
  lda (REG2),y
  beq lcd_print_done
  jsr lcd_print_char
  iny
  bra lcd_print

lcd_print_done rts

  .else ; LCD_SUPPORT

lcd_print_char :
lcd_clear_display :
lcd_second_line:
lcd_print_message:
lcd_instruction:
lcd_get_address:
lcd_home:
lcd_wait:
    ldx #50     ; No LCD
    jmp ERROR


  .endif  ; (no) LCD_SUPPORT

  .endif  ; not emulator
