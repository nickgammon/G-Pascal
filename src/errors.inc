
errors_table:
  asciiz "Memory full"                               ;  1
  asciiz "Constant expected"                         ;  2
  asciiz "= expected"                                ;  3
  asciiz "Identifier expected"                       ;  4
  asciiz ", or : expected"                           ;  5
  asciiz "Bug"                                       ;  6
  asciiz "*) or } expected"                          ;  7
  asciiz "Incorrect string"                          ;  8
  asciiz ". expected"                                ;  9
  asciiz "; expected"                                ; 10
  asciiz "Undeclared identifier"                     ; 11
  asciiz "Illegal identifier"                        ; 12
  asciiz ":= expected"                               ; 13
  asciiz "Literal string of zero length"             ; 14
  asciiz "Compiler limits exceeded"                  ; 15
  asciiz "THEN expected"                             ; 16
  asciiz "; or END expected"                         ; 17
  asciiz "DO expected"                               ; 18
  asciiz "Incorrect symbol"                          ; 19
  asciiz "Bug"                                       ; 20
  asciiz "Use of procedure identifier in expression" ; 21
  asciiz ") expected"                                ; 22
  asciiz "Illegal factor"                            ; 23
  asciiz "Type mismatch"                             ; 24
  asciiz "BEGIN expected"                            ; 25
  asciiz "OF expected"                               ; 26
  asciiz "Stack full"                                ; 27
  asciiz "TO or DOWNTO expected"                     ; 28
  asciiz "String literal too big"                    ; 29
  asciiz "Number out of range"                       ; 30
  asciiz "( expected"                                ; 31
  asciiz ", expected"                                ; 32
  asciiz "[ expected"                                ; 33
  asciiz "] expected"                                ; 34
  asciiz "Parameters mismatched"                     ; 35
  asciiz "Data type not recognised"                  ; 36
  asciiz "Symbol table full"                         ; 37
  asciiz "Duplicate identifier"                      ; 38
  asciiz "Value expected"                            ; 39
  asciiz "Illegal opcode"                            ; 40
  asciiz "Illegal addressing mode"                   ; 41
  asciiz "Branch out of range"                       ; 42
  asciiz "Need zero page address"                    ; 43
  asciiz "Operand too large"                         ; 44
  asciiz "Symbol address changed"                    ; 45
  asciiz "Expression too complex"                    ; 46
  asciiz "Divide by zero"                            ; 47
  asciiz "Label required"                            ; 48
  asciiz "Symbol table in use"                       ; 49
  asciiz "No LCD"                                    ; 50
  asciiz "Code already generated"                    ; 51
assertion_failed_message:   ; used by Pascal assert function
  asciiz "Assertion failed"                          ; 52
  asciiz "Use of function identifier in statement"   ; 53
  asciiz "Use of opcode as label"                    ; 54
  dfb 0

;***********************************************
; DISPLAY ERROR - this does not return to the caller
;  but to main_prompt
;  Error number in X
;***********************************************
ERRLIT   asciiz  '*** Error: '

ERROR    stx  ERRNO
         lda  RUNNING
         beq  ERR7
         jmp  ERR6
ERR7     =  *
         lda  LIST
         bne  ERR1
         jsr  CROUT
         jsr  show_current_line
ERR1     =  *
         lda  token_address
         sec
         sbc  token_line_start  ; TODO: what about high-order byte?
         pha             ; CHARS UP TO ERROR POINT
         lda  #<ERRLIT   ; *** Error
         ldx  #>ERRLIT
         jsr  print
         pla
         clc
         adc   #1
         sta  TEMP       ; BYTES TO ERROR POINT
         tax
ERR3     =  *
         jsr  PUTSP
         dex
         bne  ERR3
         lda  #'^'
         jsr  COUT
         jsr  CROUT
         ldx  #9
ERR5     =  *
         jsr  PUTSP
         dex
         bne  ERR5
ERR6     =  *
  lda #<errors_table
  sta REG
  lda #>errors_table
  sta REG+1
  ldx #0
  ldy #0
error_next:
  inx
error_loop:
  cpx ERRNO
  beq error_found
error_find_next:
  lda (REG),Y
  pha
  inc REG
  bne error1
  inc REG+1
error1:
  pla           ; get the previous byte
  bne error_find_next
  lda (REG),Y   ; a null after a null?
  beq error_not_found
  bra error_next  ; we got a null, so add 1 to our current error number (X)

error_not_found:
  lda #<error_not_found_message
  ldx #>error_not_found_message
  jsr print
  lda ERRNO
  sta REG
  stz REG+1
  stz REGB
  jsr display_in_decimal
  jsr CROUT
  jmp main_prompt

error_found:
  lda REG
  ldx REG+1
  jsr print
  jsr CROUT
  jmp  main_prompt

error_not_found_message asciiz "Unknown error: "

show_current_line:
  lda #'('
  jsr COUT
  lda PCODE+1
  jsr PRBYTE
  lda PCODE
  jsr PRBYTE
  lda #')'
  jsr COUT
  jsr PUTSP
  jsr show_current_line_number
  lda token_line_start
  sta mem_move_src
  lda token_line_start+1
  sta mem_move_src+1

show_current_line_loop:
  ldy #0
  lda (mem_move_src),Y
  beq show_current_line_done
  cmp #NL
  beq show_current_line_done
  cmp #$20
  bcs show_current_line_loop_print_one_character
  pha
  lda #'^'
  jsr COUT
  pla
  clc
  adc #$40  ; make 0x01 print as ^A
show_current_line_loop_print_one_character:
  jsr COUT    ; show that byte
  inc mem_move_src
  bne show_current_line_loop
  inc mem_move_src+1
  bra show_current_line_loop

show_current_line_done:
  jsr CROUT
  rts
