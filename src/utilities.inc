;
;  function to look up a handler for a word (eg. assembler directive)
;
;  word in SRCE
;
;  table of words/handlers in DEST
;
;  length of word in A
;
;  see makeHandler for making the table
;
;  Basically:
;    asciiz "foo"
;    word   foo_handler
;
;  In other words:
;    makeHandler "foo",foo_handler
;
;  Returns with carry set if no match, otherwise calls the handler which should clear carry
;

handlerLookup:
  sta token_work    ; save length

;
;  find the length of the word
;
handler_loop:
  ldy #0
  lda (DEST),y
  beq handlers_done
handler_find_length:
  lda (DEST),y
  beq handler_found_length
  iny
  beq handlers_done   ; sanity clause
  bra handler_find_length
handler_found_length:

  cpy token_work
  beq handler_same_length
;
;  not the same length, or no match on the string
;
handler_next:
  tya
  clc
  adc #3  ; skip the 0x00 and the execution address
  adc DEST
  sta DEST
  lda DEST+1
  adc #0
  sta DEST+1
  bra handler_loop

handler_same_length:

  phy  ; save the length
  jsr COMSTL
  beq  handler_found_directive
  ply
  bra handler_next
handler_found_directive:
  ply  ; get the length back
  iny  ; skip past the 0x00 byte
  lda (DEST),y
  sta token_work+1
  iny
  lda (DEST),y
  sta token_work+2
  clc
  rts

handlers_done:
  sec     ; error flag
  rts

;
;  function to look up a handler for a word on a partial match (eg. editor command)
;
;  word in SRCE
;
;  table of words/handlers in DEST - in UPPER CASE
;
;  length of word in A
;
;  see makeHandler for making the table
;
;  Basically:
;    asciiz "FOO"
;    word   foo_handler
;
;  In other words:
;    makeHandler "FOO",foo_handler
;
;  Returns if no match, otherwise jumps to the handler and thus doesn't return
;

partial_handler_Lookup:
  sta token_work    ; save length
;
;  here for a new table item
;
partial_handler_next_item:
  ldy #0
  lda (DEST),y
  beq partial_handler_done
  ldx token_work        ; source length
partial_handler_next_loop:
;
;  ensure the nth character matches until the source runs out
;
  lda (SRCE),y
  jsr MAKE_UPPER
  cmp (DEST),y
  bne partial_handler_next
  iny
  dex     ; one less count
  beq partial_handler_found_directive ; source has finished and we found the directive
  bra partial_handler_next_loop       ; try matching the next one
;
;  no match on the string - advance Y to the end of the destination string
;
partial_handler_next:
  lda (DEST),y
  beq partial_handler_next_found_item_length
  iny
  bra partial_handler_next
partial_handler_next_found_item_length:
  tya     ; length of the word in the table
  clc
  adc #3  ; skip the 0x00 and the execution address
  adc DEST
  sta DEST
  lda DEST+1
  adc #0
  sta DEST+1
  bra partial_handler_next_item

partial_handler_found_directive:
  lda (DEST),y
  beq partial_handler_skipped_string
  iny
  bra partial_handler_found_directive
partial_handler_skipped_string:
  iny  ; skip past the 0x00 byte
  lda (DEST),y
  sta token_work+1
  iny
  lda (DEST),y
  sta token_work+2
  jmp (token_work+1)   ; do partial_partial_handler_ - it will RTS for us

partial_handler_done:
  rts

;***********************************************
; JUMP ON TOKEN
; X/Y = START OF TABLE
; END OF TABLE IS A NULL
; A = TOKEN
;***********************************************
TKNJMP   =  *
         stx  REG     ; save table address
         sty  REG+1

         jsr MAKE_UPPER
         tax          ; save the token
JMP1     =  *
         ldy  #0
         lda  (REG),Y
         bne  JMP2
; failed to find the token
         txa          ; retore the token
         rts
JMP2     =  *
         txa          ; get token back
         cmp  (REG),Y
         bne  JMP3    ; branch if not found
         pla
         pla          ; remove return address for TKNJMP call
         iny          ; move Y on to routine address
         lda  (REG),Y ; get low order byte of routine
         sta  REG2
         iny          ; next byte
         lda  (REG),Y ; get high order byte
         sta  REG2+1
         txa          ; get token back
         jmp  (REG2)  ; go to routine
JMP3     =  *
         lda  REG     ; skip this token sequence
         clc
         adc  #3
         sta  REG
         bcc  JMP1
         inc  REG+1
         bra  JMP1    ; try again

;


;***********************************************
;
; Case conversion
;
;***********************************************


MAKE_UPPER = *
         CMP  #('z'+1)
         BCS  MAKE_UPPER_DONE
         CMP  #'a'
         BCC  MAKE_UPPER_DONE
         SBC  #$20  ; make upper-case - we want carry set so there is no borrow
MAKE_UPPER_DONE = *
         RTS

MAKE_LOWER = *
         CMP  #'A'
         BCC  MAKE_LOWER_DONE
         CMP  #('Z'+1)
         BCS  MAKE_LOWER_DONE
         ADC  #$20  ; make upper-case - we want carry clear so there is no carry
MAKE_LOWER_DONE = *
         RTS


;
;  generate the next random number
;
gen_random:
    asl random
    rol random+1
    rol random+2
    rol random+3
    bcc gen_random1
    lda random
    eor #$B7
    sta random
    lda random+1
    eor #$1D
    sta random+1
    lda random+2
    eor #$C1
    sta random+2
    lda random+3
    eor #$04
    sta random+3
gen_random1:
    rts



;
;  From: http://www.6502.org/source/integers/crc-more.html
;
;   jacksum  -a crc:16,1021,FFFF,false,false,0 -X SOME_FILE
;
;   https://jacksum.net/en/index.html
;
;   With a starting CRC of $FFFF, the binary string $01 $02 $03 $04 should evaluate to $89C3.
;
;   Start at crc_addr, for crc_num bytes
;
crc16 = *
;
;  initialise CRC
;
  lda #$FF
  sta crc_val
  sta crc_val+1
  lda crc_num
  ora crc_num+1
  beq crc_done

;
;  here for each byte
;
crc16_loop  = *
  ldy #0
  lda (crc_addr),y  ; get next byte
  jsr crc_byte
  ;
  ;  onto next address
  ;
  inc crc_addr
  bne crc16_next
  inc crc_addr+1       ; Step to next byte
crc16_next:
  ;
  ; now do a 16-bit decrement
  ;
  LDA crc_num
  BNE crc16_skip    ; decrement the high-order byte if we are about to wrap
  DEC crc_num+1
crc16_skip:
  DEC crc_num       ; decrement the low-order byte
  BNE crc16_loop    ; not zero, keep going
  LDA crc_num+1     ; are we zero here too?
  BNE crc16_loop    ; not yet

crc_done:
  rts

;
;  do a CRC of one byte (in A) updating crc_val
;
crc_byte = *
  eor crc_val+1     ; A contained the data
  sta crc_val+1     ; XOR it into high byte
  lsr               ; right shift A 4 bits
  lsr               ; to make top of x^12 term
  lsr               ; ($1...)
  lsr
  tax               ; save it
  asl               ; then make top of x^5 term
  eor crc_val       ; and XOR that with low byte
  sta crc_val       ; and save
  txa               ; restore partial term
  eor crc_val+1     ; and update high byte
  sta crc_val+1     ; and save
  asl               ; left shift three
  asl               ; the rest of the terms
  asl               ; have feedback from x^12
  tax               ; save bottom of x^12
  asl               ; left shift two more
  asl               ; watch the carry flag
  eor crc_val+1     ; bottom of x^5 ($..2.)
  tay               ; save high byte
  txa               ; fetch temp value
  rol               ; bottom of x^12, middle of x^5!
  eor crc_val       ; finally update low byte
  sta crc_val+1     ; then swap high and low bytes
  sty crc_val
  rts


crc_helper = *
  phy
  phx
  jsr crc_byte
  inc crc_num
  bne crc_helper1
  inc crc_num+1
crc_helper1:
  plx
  ply
  rts

crc_message asciiz "Source CRC       $"               ; source CRC
source_length_message asciiz "Source length: "    ; source length
bytes_message asciiz " bytes\n"

crc_source = *
  jsr find_source_end

;
;  move existing source to end of RAM
;
  clc             ; don't count the final 0x00 byte
  lda PCODE
  sbc #<TEXT_START
  sta crc_num
  lda PCODE+1
  sbc #>TEXT_START
  sta crc_num+1
;
;  Show the length of the source
;
  lda #<source_length_message
  ldx #>source_length_message
  jsr print
  lda crc_num
  sta REG
  lda crc_num+1
  sta REG+1
  stz REGB
  jsr display_in_decimal
  lda #<bytes_message
  ldx #>bytes_message
  jsr print

;
;  now the CRC
;
  lda #<TEXT_START
  sta crc_addr
  lda #>TEXT_START
  sta crc_addr+1
  jsr crc16
  lda  #<crc_message            ;  source CRC
  ldx  #>crc_message            ;
  jsr  print
  lda crc_val+1
  jsr PRBYTE
  lda crc_val
  jsr PRBYTECR
  rts


;
;  delays for roughly 1 ms - changes no registers
;
delay_1ms = *
         phx
         ldx #108
delay_1ms_loop:
         nop
         nop
         dex
         bne delay_1ms_loop
         plx
         rts

;
; delay for YYXX ms (Y = high-order byte, X = lo-order byte)
;  maximum delay 0x7FFF, that is 32767 ms
;  Changes X and Y
;
delay = *
  jsr delay_1ms
  dex
  bne delay
  dey
  bpl delay
  rts


;--------------------------------------------------------------------------
;
;  BINARY TO DECIMAL CONVERSION
;
;  This works as follows:
;
;  1. First the number is checked if it is negative (sign bit set).
;  2. If so, a "-" is output and the number subtracted from zero to make it positive.
;  3. Our next step is to set a BCD (binary coded decimal) output area to all zeroes.
;      In BCD, the number 1234 is actually stored as 0x1234.
;      With BCD mode turned on in the processor, doing adds results in an early carry
;      so that numbers add "the decimal way" rather than the binary way.
;  4. Then we a loop of 24 iterations (one for each bit) which adds the appropriate number from bcd_table
;      if that bit was set in the original value.
;  5. The function bcd_unpack is then called to convert the BCD format into ASCII format. It takes each
;     nibble and adds 0x30 ('0') to it, and saves that in bcd_result.
;  6. At this stage bcd_result has a 8-digit ASCII number in it.
;  7. We then skip leading zeroes in bcd_result.
;  8. Finally the remaining digits in bcd_result are output.
;
;
;   I'm not sure where the idea for this came from. There was no Internet when this was written in 1979
;   so it is possibly (probably) my own.
;   My original implementation, although similar, was more complex than this one, which I found at:
;              http://www.6502.org/source/integers/hex2dec.htm
;
;   Other suggestions for binary to decimal conversion I found recently do not employ this method. One
;   technique (as described by Ben Eater here: https://www.youtube.com/watch?v=v3-a-zqKfgA ) is to
;   repeatedly divide by 10 and use the remainder as the digit. This has to be done until the result of
;   the division is zero. Dividing a 3-byte number by 10 on an 8-bit machine is not particularly fast,
;   as it involves an inner loop of 24 iterations (one for each bit) and this would have to be done for
;   each digit, so the number of loops of 24 iterations would increase for each digit. Therefore an
;   8-digit number (like 80000000) would take 144 iterations in the divide loop. Then the digits have to
;   be reversed because the remainder gives you the low-order decimal digit first.
;
;   Another technique is the "Double dabble" routine.
;
;     https://en.wikipedia.org/wiki/Double_dabble
;
;   The method below seems fairly efficient, as the inner loop only does anything substantial if a
;     1-bit is detected. Thus large numbers can be produced quite efficiently, however it would be a
;     bit slower if the input number had a lot of 1-bits set.
;
;   Considering that this is only used when outputting to serial, itself a slow process, speed is not
;   really the issue here.
;
;   - Nick Gammon
;
;
;--------------------------------------------------------------------------
;
; Converts BCD to ASCII
;
bcd_unpack:
  pha  ; save the number for later
;
;  shift right 4 bits
;
  lsr A
  lsr A
  lsr A
  lsr A
  ora #'0'          ; make printable
  sta bcd_result,X  ; save in bcd_result
  inx
  pla               ; get the original character back
  and #$0F          ; take low-order bits and make printable
  ora #'0'
  sta bcd_result,X  ; save in bcd_result
  inx
  rts               ; done!

;
;
; Table for adding into the BCD result.
;
bcd_table  =  *
  dfb $00,$00,$00,$01   ; 0000001
  dfb $00,$00,$00,$02   ; 0000002
  dfb $00,$00,$00,$04   ; 0000004
  dfb $00,$00,$00,$08   ; 0000008
  dfb $00,$00,$00,$16   ; 0000016
  dfb $00,$00,$00,$32   ; 0000032
  dfb $00,$00,$00,$64   ; 0000064
  dfb $00,$00,$01,$28   ; 0000128
  dfb $00,$00,$02,$56   ; 0000256
  dfb $00,$00,$05,$12   ; 0000512
  dfb $00,$00,$10,$24   ; 0001024
  dfb $00,$00,$20,$48   ; 0002048
  dfb $00,$00,$40,$96   ; 0004096
  dfb $00,$00,$81,$92   ; 0008192
  dfb $00,$01,$63,$84   ; 0016384
  dfb $00,$03,$27,$68   ; 0032768
  dfb $00,$06,$55,$36   ; 0065536
  dfb $00,$13,$10,$72   ; 0131072
  dfb $00,$26,$21,$44   ; 0262144
  dfb $00,$52,$42,$88   ; 0524288
  dfb $01,$04,$85,$76   ; 1048576
  dfb $02,$09,$71,$52   ; 2097152
  dfb $04,$19,$43,$04   ; 4194304
  dfb $08,$38,$86,$08   ; 8388608

;--------------------------------------------------------------------------
; display_in_decimal - start here - VALUE will be zero after this is executed
;--------------------------------------------------------------------------

display_in_decimal  =  *
  lda  VALUE+2
  bpl  bcd_positive
;
;  if the number is negative, output a minus sign and make it positive
;
  lda  #'-'
  jsr  COUT
  jsr  exp_unary_minus
;
;  number is now positive
;
bcd_positive:

  jsr binary_to_decimal   ; do the conversion

  ldx  #7         ; zero suppress count (always keep the last zero)
  ldy  #0
;
;  this is skipping the leading zeroes
;
bcd_skip_zeroes:
  lda  bcd_result,Y
  cmp  #'0'
  bne  bcd_output_digit
  iny
  dex
  bne  bcd_skip_zeroes
;
;  now we have a non-zero, start outputting them
;
;  Y points to how far we are through bcd_result
;  X is how many digits to go
;
bcd_output_digit:
  lda  bcd_result,Y
  jsr  COUT
  iny
  dex
  bpl  bcd_output_digit
;
;  done!
;
  rts

;--------------------------------------------------------------------------
;
;  Binary to decimal routine: Converts VALUE into bcd_result (8 digits)
;
;  See: http://www.6502.org/source/integers/hex2dec.htm
;
;  - VALUE will be zero after this is executed
;
;--------------------------------------------------------------------------
binary_to_decimal:
  sed                 ; decimal mode for adds below
  stz  bcd_work       ; zero our result area (4 bytes = 8 digits)
  stz  bcd_work+1
  stz  bcd_work+2
  stz  bcd_work+3

  ldx  #(4*23)        ; X points to the start of the highest entry

binary_to_decimal_loop:
  asl VALUE      ;  shift out high-order bit into Carry
  rol VALUE+1
  rol VALUE+2
  bcc binary_to_decimal_next  ; if carry clear, no add needs to be done
;
;  that bit was set, so add the appropriate BCD number from the table (eg. 1, 2, 4, 8, 16 and so on)
;
  clc
  lda bcd_work
  adc bcd_table+3,X
  sta bcd_work
  lda bcd_work+1
  adc bcd_table+2,X
  sta bcd_work+1
  lda bcd_work+2
  adc bcd_table+1,X
  sta bcd_work+2
  lda bcd_work+3
  adc bcd_table,X
  sta bcd_work+3

binary_to_decimal_next:
  dex      ; back to the previous table item
  dex
  dex
  dex
  bpl binary_to_decimal_loop
  cld     ; decimal mode off


;
;  convert BCD to ASCII - note that UNPACK increments x by two each time
;    - the results are in bcd_result - an 8-character field
;
;  We unpack the high-order bytes first, naturally
;
  ldx  #0
  lda  bcd_work+3
  jsr  bcd_unpack
  lda  bcd_work+2
  jsr  bcd_unpack
  lda  bcd_work+1
  jsr  bcd_unpack
  lda  bcd_work
  jsr  bcd_unpack
  rts

NOSCE    asciiz  'No source file.\n'   ; No source file

;***********************************************
;
; Initialize a compile or assembly
;
;***********************************************


INIT     =  *
;
;  set up symbol table pointer
;

  lda  #>SYMBOL_TABLE_START
  sta  ENDSYM+1
  sta  STARTSYM+1
  lda  #<SYMBOL_TABLE_START
  sta  ENDSYM
  sta  STARTSYM

INIT_SECOND_PASS:   ; for assembler

  lda  #0
  sta  LIST            ; not listing
  sta  LEVEL           ; variable level zero
  sta  DCODE
  sta  RUNNING         ; not running
  sta  PRCITM
  sta  PRCITM+1
  sta  REGB
  sta  token_type
  stz  system_flags
  lda #1
  sta  current_line        ; we are currently on line 1
  stz  current_line+1

;
;  set up for reading first byte of source
;
  lda  #<TEXT_START
  sta  token_start
  sta  token_line_start
  lda  #>TEXT_START
  sta  token_start+1
  sta  token_line_start+1
  jsr  find_source_end
  lda  PCODE
  sta  ACT_PCDA
  lda  PCODE+1
  sta  ACT_PCDA+1
  ldy  #0
  lda  (token_start),y
  bne  INIT9
  lda  #<NOSCE
  ldx  #>NOSCE
  jsr  print                      ; No source file
  jmp  main_prompt
INIT9    rts




;
CROUT    =  *             ; show a newline
  lda  #NL         ; was a carriage-return in legacy, now is newline
  jmp  COUT
;
;***********************************************
; COMPARE STRING - compare SRCE to DEST, assumes DEST is upper-case
;                - length in Y (therefore max 255 characters)
;                - zero flag set if match (therefore do a BEQ to see if strings match)
;***********************************************
COMSTL   =  *
         dey
         bmi  COMS8
         lda  (SRCE),Y
         jsr MAKE_UPPER
         cmp  (DEST),Y
         beq  COMSTL
COMS9    rts             ; NOT EQUAL
COMS8    lda  #0
         rts             ; EQUAL

;***********************************************
; DISPLAY A LINE
;

;
; DISPLAY IN HEX
;
DISHX    jsr  PRBYTE
         jmp  PUTSP
;

;***********************************************
; DISPLAY PCODE ADDRESS
;***********************************************
DISPAD   =  *
         lda  DCODE
         beq  DISPAD2
DISPAD1  =  *
         lda  #'('
         jsr  COUT
         lda  PCODE+1
         jsr  PRBYTE
         lda  PCODE
         jsr  PRBYTE
         lda  #')'
         jsr  COUT
         jmp  PUTSP
DISPAD2  rts
;

;***********************************************
; PUSH 'WORK' ONTO STACK
;***********************************************
PSHWRK   =  *
         sta  BSAVE
         pla
         tax
         pla
         tay
         lda  WORK+1
         pha
         lda  WORK
         pha
         tya
         pha
         txa
         pha
         lda  BSAVE
PSH9     rts
;
;***********************************************
; PULL 'WORK' FROM STACK
;***********************************************
PULWRK   =  *
         sta  BSAVE
         pla
         tax
         pla
         tay
         pla
         sta  WORK
         pla
         sta  WORK+1
         tya
         pha
         txa
         pha
         lda  BSAVE
         rts
;
;***********************************************
; PRINTING SUBROUTINES
; Control characters print with a ^ in front of them, eg. ^A
;***********************************************
PRCHAR   =  *
  pha
  cmp #NL
  beq prchar_not_control
  cmp #$20
  bcs prchar_not_control
  pha
  lda #'^'
  jsr COUT
  pla
  clc
  adc #$40  ; make 0x01 print as ^A
prchar_not_control:
  jsr  COUT
  pla
  rts
;
; Prints A in hex
;
PRBYTE:
  pha
  lsr
  lsr
  lsr
  lsr
  jsr  PRHEXZ    ; first nibble
  pla            ; now do other nibble
PRHEX:
  and  #$0F
PRHEXZ:
  ora  #'0'
  cmp  #'0' + $0A
  bcc  PRHEX1
  adc  #$26  ; (carry is set: adding 7 to make 0x0a become 'a')
PRHEX1:
  jmp  PRCHAR
;
PUTSP    =  *
  lda  #' '
  bra  PRHEX1
;
;
;  PT - put text
;   Low-order address in A, high-order address in X, count in Y
;
;
PT:
  sta  REG2
  stx  REG2+1
  tya    ; count of bytes to print
  tax    ; put that into X
  ldy  #0
PT6:
  lda  (REG2),Y    ; next character
  jsr COUT         ; show it
  iny
  dex
  bne  PT6
  rts
;
;
;
;---- token_address --> WORK
;
TKNWRK   =  *
         PHA
         LDA  token_address
         STA  WORK
         LDA  token_address+1
         STA  WORK+1
         PLA
         RTS
;
;---- WORK --> token_address
;
WRKTKN   =  *
         PHA
         LDA  WORK
         STA  token_address
         LDA  WORK+1
         STA  token_address+1
         PLA
         RTS
;


;***********************************************
; MENU, FILE HANDLING
;***********************************************
;
pcodes_ended_message        asciiz  "P-codes ended at $"
compile_finished_message    asciiz  "Compile finished: No errors.\n"
symbol_table_ended_message  asciiz "Symbol table ended at $"
no_valid_compile_message    asciiz  'No valid compile or assemble done before\n'
;
PRBYTECR =  *
         JSR  PRBYTE
         JMP  CROUT
;
;
;
CHK_VAL  =  *
         lda  #FLAG_VALID_COMPILE
         ora  #FLAG_VALID_ASSEMBLE
         and  system_flags
         bne  CHK_VAL9
         lda  #<no_valid_compile_message    ; No valid Compile or Assemble done before
         ldx  #>no_valid_compile_message
         jsr  print
         jmp  main_prompt
CHK_VAL9 =  *
BELL1X   =  *          ; no bell yet
         rts
;
CHK_RUN  jsr  CHK_VAL
         lda  #FLAG_VALID_ASSEMBLE
         and  system_flags
         bne  run_assembler
;
;  If we somehow get here without Pascal included, go back to editor
;
         .if  USE_PASCAL
         jmp  INTERP
         .else
         jmp main_prompt
         .endif ; USE_PASCAL

run_assembler:
         ldx #RUNNING_STACK_TOP           ; don't use all of stack
         txs
         lda #FLAG_BRK_REACHED    ; no BRK reached yet
         trb system_flags
         jsr call_assembler
         jmp  EX_FINISHD
;
;  let's assume they put a RTS at the end of their code
;
call_assembler:
         lda  #<running_message   ; Running
         ldx  #>running_message
         jsr  print
         jmp (ACT_PCDA)

;

;
; Get a line, read byte by byte until we get a newline, store in INBUF
;   returns length in Y - handles backspace, ignores carriage-return
;
GET_LINE =  *
GETLN1   =  *
  ldy  #0
GET1:
  jsr  CHRIN
  cmp  #CR       ; carriage-return?
  beq  GET1       ; ignore it
  sta  INBUF,Y    ; SAVE IN BUFFER
  iny
  beq  GETLN_OVERFLOW       ; overflow
  cmp  #NL        ; END OF LINE?
  beq  GET3       ; yes
  cmp  #BACKSPACE ; backspace?
  bne  GET1       ; no - keep adding to buffer
;
;  here for backspace
;
  dey  ; get rid of backspace
  dey  ; get rid of character we backspaced over
  cpy  #$FF
  beq  GETLN1   ; if Y underflowed, go back to zero
  bra GET1

GET3:
  lda  #0
  sta  INBUF,Y    ; turn newline into 0x00
  rts             ; RETURN
;
;  here on INBUF overflow
;
GETLN_OVERFLOW:
  dey           ; get back to 255 (last byte)
  bra  GET3     ; store a zero there


;
;  output a character - saves all registers
;
COUT:
  pha
  phx
  phy
  jsr COUT_CALL   ; call the current outputting routine
  ply
  plx
  pla
  rts


COUT_CALL:
  jmp (write_function)


DM7      asciiz  "Error occurred at P-code "

;
; Ding a bell - however we don't have it on this board
;
BELL1    =  *
         pha
         lda  #0
         sta  RUNNING
         jsr  CROUT
         pla
         rts
;
;
RUNERR   jsr  BELL1
         lda  #<DM7  ; Error occurred at P-code
         ldx  #>DM7
         jsr  print
         lda  LASTP+1
         jsr  PRBYTE
         lda  LASTP
         jsr  DISHX
EX_FINISHD  =  *
         lda  #0
         jsr  CROUT
         lda  #<FIN_MSG
         ldx  #>FIN_MSG
         jsr  print
         stz  serial_in_byte_received  ; get rid of read-ahead
         jsr  GETIN      ; wait till message seen
         jsr  CROUT      ; output a newline in case they press space or something
         jmp  RESTART
;
FIN_MSG  asciiz  'Run finished - press a key ...'

ass_error:
  lda ASS_EMIT_COUNT
  beq ass_error1
  jsr CROUT
ass_error1:
  jmp ERROR

