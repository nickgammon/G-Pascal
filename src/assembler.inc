;***********************************************
;
; Assembler
;
;***********************************************

;***********************************************
; Assembler operand types
;***********************************************

ASS_OPERAND_ABSOLUTE                           =  1      ; a
ASS_OPERAND_ABSOLUTE_INDEXED_INDIRECT          =  2      ; (a,x)
ASS_OPERAND_ABSOLUTE_INDEXED_WITH_X            =  3      ; a,x
ASS_OPERAND_ABSOLUTE_INDEXED_WITH_Y            =  4      ; a,y
ASS_OPERAND_ABSOLUTE_INDIRECT                  =  5      ; (a)
ASS_OPERAND_ACCUMULATOR_A                      =  6      ; A
ASS_OPERAND_IMMEDIATE                          =  7      ; #
ASS_OPERAND_IMPLIED                            =  8      ; i
ASS_OPERAND_PROGRAM_COUNTER_RELATIVE           =  9      ; r
ASS_OPERAND_STACK                              = 10      ; s
ASS_OPERAND_ZERO_PAGE                          = 11      ; zp
ASS_OPERAND_ZERO_PAGE_INDEXED_INDIRECT         = 12      ; (zp,x)
ASS_OPERAND_ZERO_PAGE_INDEXED_WITH_X           = 13      ; zp,x
ASS_OPERAND_ZERO_PAGE_INDEXED_WITH_Y           = 14      ; zp,y
ASS_OPERAND_ZERO_PAGE_INDIRECT                 = 15      ; (zp)
ASS_OPERAND_ZERO_PAGE_INDIRECT_INDEXED_WITH_Y  = 16      ; (zp),y
ASS_OPERAND_STRING                             = 17      ; "But does it get goat's blood out?"
ASS_OPERAND_ZERO_PAGE_AND_ABSOLUTE             = 18      ; zp,r

;
;  Entry point for assembler
;
ASSEMBLE:

   ldx  #NEW_STK
   txs    ; set stack back to 0xFF
   stz  ASS_PASS  ; first pass
   jsr  INIT       ; sets up pointers, calls LINE
   jsr  add_assembler_library_functions  ; add our inbuilt functions to the symbol table
   bra  ASSEMBLE_PASS

ASSEMBLE_SECOND_PASS:
   inc  ASS_PASS
   jsr  INIT_SECOND_PASS    ; don't re-initialise the symbol table
ASSEMBLE_PASS:
   stz  current_line  ; we'll got for zero, not one

   lda  #FLAG_ASSEMBLING
   tsb  system_flags

;
;  no listing yet
;
   stz  DCODE
   stz  LIST
   stz  SYNTAX
   stz  show_symbols
;
; zero line count
;
   stz  LINE_CNT
   stz  LINE_CNT+1

;
;  and emitted bytes count
;
   stz ass_emit_bytes
   stz ass_emit_bytes+1

;
; no previous token
;
   stz  token_type
   bra ass_line

ass_finish_line:
   lda token_type
   cmp #NL
   beq ass_line
   ldx #19    ; Incorrect Symbol
   jmp ass_error

ass_done_jump:
   jmp ass_done
;
;  here for the start of a line
;
ass_line:
  lda ASS_PASS    ; only showing codes on 2nd pass
  beq ass_line1
  lda ASS_EMIT_COUNT  ; only need a newline if we actually output a code
  beq ass_line1
  lda DCODE           ; is showing codes on?
  beq ass_line1
  jsr CROUT
ass_line1:
  ldy #0
  lda (token_start),y ; quick test so we don't list a non-existent line
  beq ass_done_jump

  lda token_start
  sta token_line_start
  lda token_start+1
  sta token_line_start+1
  jsr token_line        ; list the next line

  stz ASS_OPERAND   ; no operand yet
  stz ASS_EMIT_COUNT
  stz ass_current_label
  stz ass_current_label+1

  jsr get_token

  cmp #TOKEN_IDENTIFIER  ; could be a label if in column 1
  beq ass_identifier
  cmp #'='               ; = directive is OK
  beq ass_identifier

  cmp #NL
  beq ass_line

  cmp #0
  beq ass_done_jump

ass_bad_identifier:
  ldx #4      ; Identifier expected
  JMP ass_error

ass_identifier:

;
;  if the identifier is at the start of the line, it is a label, not an opcode
;
  lda token_address
  cmp token_line_start
  bne ass_opcode
  lda token_address+1
  cmp token_line_start+1
  bne ass_opcode

  lda token_type         ; can't have = as an identifier
  cmp #'='               ; or "="
  beq ass_bad_identifier

;
;  here for label - on the first pass, add it to the symbol table, on the second pass look it up
;
  lda ASS_PASS
  bne ass_lookup_existing_label
;
;  here for first pass
;
  jsr CHKDUP
;
; not a duplicate - add it
;
  lda PCODE
  sta token_value
  lda PCODE+1
  sta token_value+1
  stz token_value+2
  lda #SYMBOL_CONSTANT
  jsr ADDSYM
  bra ass_added_symbol

;
;  here for second pass
;
ass_lookup_existing_label:
  jsr SEARCH  ; look it up
  bne ass_lookup_save_address  ; it should be!
  ldx #6  ; ERROR: bug
  jmp ass_error

ass_lookup_save_address:
  lda token_value
  sta ASS_VALUE
  lda token_value+1
  sta ASS_VALUE+1
  lda token_value+2
  sta ASS_VALUE+2

;
;  the symbol has either been added, or looked up to make sure it is still there
;
ass_added_symbol:

;
;  remember its address in case we have an EQU directive
;
  lda SYMITM
  sta ass_current_label
  lda SYMITM+1
  sta ass_current_label+1

;
;  get whatever is after the label
;
  jsr get_token
  cmp #':'
  bne ass_not_colon
  jsr get_token  ; skip the colon
ass_not_colon:
  cmp #NL     ; newline means this is just a label line
  bne ass_not_colon2
  jmp ass_finish_line

ass_not_colon2:
  cmp #TOKEN_IDENTIFIER  ; should be an opcode now
  beq ass_opcode
  cmp #'='               ; = directive is OK
  beq ass_opcode

ass_illegal_opcode:
  jsr PRBYTE
  ldx #40      ; ERROR: illegal opcode
  JMP ass_error

ass_opcode:

;
;  this will be our assembler mnemonic or directive so save its start point and length
;

  ldx token_address
  stx OPCODE
  ldx token_address+1
  stx OPCODE+1
  ldx token_length
  stx OPCODE_LEN

;
;  if we had a label on the line, check its address hasn't changed (on the second pass)
;   UNLESS it is an EQU directive in which case it probably did change
;
  lda ASS_PASS
  beq ass_opcode2
  lda ass_current_label
  ora ass_current_label+1   ; was there a label?
  beq ass_opcode2    ; nope
  lda token_type
  cmp #'='
  beq ass_opcode2   ; don't check on "=" directive
  lda token_length
  cmp #3
  bne ass_opcode2   ; can't be EQU
;
; this is truly crap, but I am feeling lazy
;
  ldy #0
  lda (token_address),y
  jsr MAKE_UPPER
  cmp #'E'
  bne ass_check_label_changed
  iny
  lda (token_address),y
  jsr MAKE_UPPER
  cmp #'Q'
  bne ass_check_label_changed
  iny
  lda (token_address),y
  jsr MAKE_UPPER
  cmp #'U'
  beq ass_opcode2

;
;  we looked up the label earlier, when it was the current token,
;   and we saved its value in ASS_VALUE
;

ass_check_label_changed:
  lda PCODE
  cmp ASS_VALUE
  bne ass_symbol_address_changed
  lda PCODE+1
  cmp ASS_VALUE+1
  bne ass_symbol_address_changed
  lda ASS_VALUE+2   ; 3rd byte should be zero
  beq ass_opcode2

ass_symbol_address_changed:
  ldx #45     ; ERROR: Symbol address changed
  jmp ass_error



ass_opcode2:
;
;  look up operand
;

  jsr get_token
  cmp #NL           ; or end of line?
  bne ass_operand

;
;  here when the opcode is followed by (spaces) or a newline
;
  lda #ASS_OPERAND_IMPLIED
  sta ASS_OPERAND
  jsr opcode_lookup
  jsr ass_check_no_more_tokens
ass_finish_lineJ:
  jmp ass_finish_line

ass_operand:
  cmp #'#'  ; immediate?
  beq ass_immediate
  cmp #'('  ; indirect?
  beq ass_indirect
  cmp #TOKEN_IDENTIFIER
  beq ass_identifier_operand
  cmp #TOKEN_STRING
  beq ass_string
;
;  here for some kind of numeric address
;
  bra ass_operand_value

;
;  Here for a string as an operand, should be a compiler directive, eg.  ASC "Hello, world"
;
ass_string:
  lda #ASS_OPERAND_STRING
  sta ASS_OPERAND
  jsr opcode_lookup
  jsr get_token  ; NOW get the next token
  jmp ass_finish_line

;
;  immediate, that is:  #<expression>
;
ass_immediate:
  lda #ASS_OPERAND_IMMEDIATE
  sta ASS_OPERAND
  jsr get_token
  jsr ass_get_value
  jsr ass_check_no_more_tokens
  jsr opcode_lookup
  jmp ass_finish_line

;
;  indirect, that is: (expression) OR (expression,X) OR (expression),Y
;
ass_indirect:
  lda #ASS_OPERAND_ABSOLUTE_INDIRECT  ; first guess
  sta ASS_OPERAND
  jsr get_token    ; skip the bracket
  bra ass_operand_value

;
;  some sort of identifier, possibly the start of an expression or maybe just "A"
;
ass_identifier_operand:

;
;  see if the operand is just the letter "A"
;
  lda token_length
  cmp #1
  bne ass_operand_value
  ldy #0
  lda (token_address),Y
  jsr MAKE_UPPER
  cmp #'A'
  bne ass_operand_value
  lda #ASS_OPERAND_ACCUMULATOR_A
  sta ASS_OPERAND
  jsr get_token  ; skip past A
  jsr ass_check_no_more_tokens
  jsr opcode_lookup
  jmp ass_finish_line

;
;  a number or identifier - evaluate as an expression
;
ass_operand_value:

  lda token_type ; get token back
  jsr ass_get_value  ; should have some sort of address now in VALUE
;
;  we come back with the first non-value token loaded in TOKEN
;  now see if we originally had a bracket, so this must be one of those indirect things
;
  lda ASS_OPERAND
  cmp #ASS_OPERAND_ABSOLUTE_INDIRECT
  bne ass_value_not_indirectJ
  lda token_type   ; now we should have a comma or a bracket
  cmp #')'
  beq ass_indirect_with_bracket
;
;  We had an opening bracket but not a closing bracket. Must be a comma then
;

  lda #','
  ldx #32  ; ERROR: , expected
  jsr CHKTKN
;
;  now we must have X
;
  jsr ass_check_we_have_x_or_y
  bcc ass_bad_indirect
  cmp #'X'
  bne ass_bad_indirect
  lda #ASS_OPERAND_ABSOLUTE_INDEXED_INDIRECT
  sta ASS_OPERAND
  lda #')'
  ldx #22  ; ERROR: ) expected
  jsr GETCHK
  jsr get_token
  jsr ass_check_no_more_tokens
  jsr opcode_lookup
  jmp ass_finish_line

ass_value_not_indirectJ jmp ass_value_not_indirect

ass_bad_indirect:
  ldx #41  ; ERROR: illegal addressing mode
  jmp ass_error


;
;  we have some sort of indirect expression followed by a bracket, so see if there is ",Y" after it
;
ass_indirect_with_bracket:
;
;  can only be (a), (zp) or (zp),y
;
  jsr get_token
  cmp #','
  beq ass_zp_indirect_indexed
  jsr ass_check_no_more_tokens
  lda #ASS_OPERAND_ABSOLUTE_INDIRECT
  sta ASS_OPERAND
  jsr opcode_lookup
  jmp ass_finish_line


;
;  For indirect followed by a comma, we must have Y and nothing else
;  Also, the operand must be a zero-page address
;
ass_zp_indirect_indexed:
  jsr ass_check_we_have_x_or_y
  bcc ass_bad_indirect
  cmp #'Y'
  bne ass_bad_indirect
  lda ASS_VALUE+1
  bne ass_bad_zp_indirect_indexed
  lda ASS_VALUE+2
  bne ass_bad_zp_indirect_indexed
  lda #ASS_OPERAND_ZERO_PAGE_INDIRECT_INDEXED_WITH_Y
  sta ASS_OPERAND
  jsr get_token  ; skip the Y
  jsr ass_check_no_more_tokens
  jsr opcode_lookup
  jmp ass_finish_line

ass_bad_zp_indirect_indexed:
  ldx #43
  jmp ass_error

ass_not_x_or_y:
  clc
  rts

;
;  we had (<expression>),<something> so check that <something> is X or Y
;   sets carry if it is
;
ass_check_we_have_x_or_y:
  jsr get_token
  cmp #TOKEN_IDENTIFIER
  bne ass_not_x_or_y
  lda token_length
  cmp #1
  bne ass_not_x_or_y
  ldy #0
  lda (token_address),y
  jsr MAKE_UPPER
  cmp #'Y'
  beq ass_check_we_have_x_or_y_ok
  cmp #'X'
  bne ass_not_x_or_y
ass_check_we_have_x_or_y_ok:
  sec
  rts

;
;  we had a non-indirect expression, so was it followed by a comma?
;

ass_value_not_indirect:
  lda token_type
  cmp #','
  beq ass_indexed
  lda #ASS_OPERAND_ABSOLUTE
  sta ASS_OPERAND
ass_value_done:
  jsr ass_check_no_more_tokens
  jsr opcode_lookup
  jmp ass_finish_line

;
;  <expression>,X or <expression>,Y is what we are expecting here
;
;  OR: in the case of BBR0 ... BBR7 and BBS0 ... BBS7 : another expression
;

ass_indexed:
;
;  save the value in case we find a second one
;
;  checking for another one may change VALUE so we do this now
;
  lda VALUE
  sta REMAIN
  lda VALUE+1
  sta REMAIN+1
  lda VALUE+2
  sta REMAIN+2
  jsr ass_check_we_have_x_or_y
  bcc ass_absolute_another_value
  cmp #'X'
  beq ass_indexed_x
  lda #ASS_OPERAND_ABSOLUTE_INDEXED_WITH_Y
  sta ASS_OPERAND
  jsr get_token ; skip the Y
  bra ass_value_done
ass_indexed_x:
  lda #ASS_OPERAND_ABSOLUTE_INDEXED_WITH_X
  sta ASS_OPERAND
  jsr get_token ; skip the X
  bra ass_value_done

;
;  this is hopefully something like BBR0 $42,foo
;   so we need to evaluate foo
;
ass_absolute_another_value:
  lda token_type ; get token back
  jsr ass_get_value  ; should have some sort of address now in VALUE
;  jsr ass_check_no_more_tokens
  lda #ASS_OPERAND_ZERO_PAGE_AND_ABSOLUTE
  sta ASS_OPERAND
  jsr opcode_lookup
  jmp ass_finish_line


;
;  look up opcode
;
opcode_lookup:
  ; the opcode we are looking up
  lda OPCODE
  sta SRCE
  lda OPCODE+1
  sta SRCE+1
  stz ASS_COUNT   ; count of matches
;
;  get the address of our table of tables
;
  lda #<OPCODE_LOOKUP_TABLE
  sta ASS_OPCODE_WORK
  lda #>OPCODE_LOOKUP_TABLE
  sta ASS_OPCODE_WORK+1
opcode_lookup_loop:
  ldy #OPCODE_LOOKUP_OPCODE_LENGTH
  lda (ASS_OPCODE_WORK),Y       ; get opcode length
  beq opcode_lookup_loop_done   ; end of table
  cmp OPCODE_LEN                ; right length?
  beq opcode_lookup_correct_length ; yes
;
; no, try the next table item
;
opcode_lookup_next_table:
  clc
  lda ASS_OPCODE_WORK
  adc #OPCODE_LOOKUP_TABLE_SIZE  ; onto next item
  sta ASS_OPCODE_WORK
  lda ASS_OPCODE_WORK+1
  adc #0
  sta ASS_OPCODE_WORK+1
  bra opcode_lookup_loop
opcode_lookup_correct_length:
  ldy #OPCODE_LOOKUP_OPCODE_TABLE
  lda (ASS_OPCODE_WORK),Y ; get table address low-order byte
  sta DEST
  iny
  lda (ASS_OPCODE_WORK),Y ; get table address high-order byte
  sta DEST+1
  jsr ass_lookup_loop
  bcc opcode_lookup_next_table  ; not found
  tax   ; save the machine code byte
  inc ASS_COUNT
  ldy #OPCODE_LOOKUP_OPERAND_TYPE
  lda ASS_OPERAND         ; get wanted type of operand?
  cmp (ASS_OPCODE_WORK),Y ; compare to operand type in table
  beq opcode_lookup_correct_operand  ; yes
  cmp #ASS_OPERAND_ACCUMULATOR_A  ; see if in range 1 to 5
  bcs opcode_lookup_next_table
;
;  so if we have a so-called ASS_OPERAND_ABSOLUTE (with various indexing modes)
;  see if we can match on ASS_OPERAND_ZERO_PAGE by adding 10 to the operand type
;  - also applies to all the absolute modes (1 to 5)
;
  clc
  adc #10
  cmp (ASS_OPCODE_WORK),Y
  bne opcode_lookup_next_table
;
;  plus, the operand has to be in the zero page
;
  lda ASS_VALUE+1
  bne opcode_lookup_next_table
  lda ASS_VALUE+2
  bne opcode_lookup_next_table
opcode_lookup_correct_operand:
  txa   ; get the byte to emit back

;
; we have a match! emit the opcode
;
  jsr ass_emit            ; emit it
  ldy #OPCODE_LOOKUP_HANDLER  ; offset of handler
  lda (ASS_OPCODE_WORK),Y ; get opcode handler address low-order byte
  tax
  iny
  lda (ASS_OPCODE_WORK),Y ; get opcode handler address high-order byte
  beq opcode_lookup_no_handler
  sta ASS_OPCODE_WORK+1
  txa
  sta ASS_OPCODE_WORK
  jmp (ASS_OPCODE_WORK)   ; do handler - it will RTS for us

opcode_lookup_no_handler:
; put nothing here as it won't be done if there is a handler
  rts    ; done!

opcode_lookup_loop_done:
;
;  look for assembler directives
;
  lda #<DIRECTIVES
  sta DEST
  lda #>DIRECTIVES
  sta DEST+1
  lda OPCODE_LEN

;  pha
;   lda #'c'
;   jsr ass_debug_point
;   pla

  jsr handlerLookup
  bcc opcode_lookup_ok

;   lda #'d'
;   jsr ass_debug_point

;
;  here for unknown opcode
;
  lda OPCODE
  sta token_address
  lda OPCODE+1
  sta token_address+1
  lda OPCODE_LEN
  sta token_length

;
;  if we NEVER found a match, this is an illegal opcode
;
  lda #'!'
  jsr COUT
  ldx OPCODE_LEN
  ldy #0
wtf_loop:
   lda (OPCODE),Y
   jsr COUT
   iny
   dex
   bne wtf_loop


  ldx  #40    ; illegal opcode
  lda ASS_COUNT
  beq opcode_lookup_loop_unknown
;
;  if we found a match, but not for this addressing mode, they used an illegal mode
;
  ldx  #41   ; illegal addressing mode
opcode_lookup_loop_unknown:
  JMP  ass_error

opcode_lookup_ok:

;   lda #'e'
; jsr ass_debug_point

  rts

ass_done:
  jsr  CROUT
  LDA  ASS_PASS
  bne  ass_done_second_pass
  jmp  ASSEMBLE_SECOND_PASS
ass_done_second_pass:
  LDA  #<ass_done_message  ; Assemble finished: No Errors
  LDX  #>ass_done_message
  JSR  print
  lda  #FLAG_VALID_ASSEMBLE
  tsb  system_flags
  jsr  show_symbol_table_end
  jsr  show_source_end
  lda  show_symbols
  beq  ass_done1
  jsr  list_symbols
ass_done1:
  JMP  main_prompt

ass_done_message asciiz "Assemble finished: No errors\n"

ass_check_no_more_tokens:
  lda token_type
  cmp #NL           ; or end of line?
  beq ass_check_no_more_tokens_ok
  ldx #19    ; ERROR: incorrect symbol
  jmp ass_error

ass_check_no_more_tokens_ok:
  rts

;
;  push a 3-byte value (in VALUE) onto the evaluation stack
;    changes A and Y
;
exp_push_value = *
  lda exp_value_stack_count
  cmp #$FF
  bne exp_push_value_ok
exp_push_value_memory_full:
  ldx #46   ; ERROR: Expression too complex
  jmp ass_error
exp_push_value_ok:
  inc exp_value_stack_count
  sec
  lda exp_value_stack
  sbc #3
  sta exp_value_stack
  lda exp_value_stack+1
  sbc #0
  sta exp_value_stack+1
;
;  has our expression value fallen down into our P-codes?
;
;  This doesn't work if the output (PCODE) has been relocated to above
;  the symbol table and I can't think of an easy solution, so let's ignore
;  it and hope it doesn't happen.
;
;
; lda  PCODE+1               ; see if P-codes full
; cmp  exp_value_stack+1     ; in other words, have we hit the symbol table?
; bcc  exp_push_value_not_full      ; less than
; bne  exp_push_value_memory_full   ; greater than
; lda  PCODE
; cmp  exp_value_stack
; bcs  exp_push_value_memory_full   ; BGE

exp_push_value_not_full:
  ldy #0
  lda VALUE
  sta (exp_value_stack),y
  iny
  lda VALUE+1
  sta (exp_value_stack),y
  iny
  lda VALUE+2
  sta (exp_value_stack),y
  rts

;
;  pop a 3-byte value (into VALUE) from the evaluation stack
;    changes A and Y
;
exp_pop_value = *
  lda exp_value_stack_count
  bne exp_pop_value_ok
  ldx #23    ; ERROR: illegal factor
  jmp ass_error
exp_pop_value_ok:
  dec exp_value_stack_count
  ldy #0
  lda (exp_value_stack),y
  sta VALUE
  iny
  lda (exp_value_stack),y
  sta VALUE+1
  iny
  lda (exp_value_stack),y
  sta VALUE+2
  clc
  lda exp_value_stack
  adc #3
  sta exp_value_stack
  lda exp_value_stack+1
  adc #0
  sta exp_value_stack+1
  rts

;
;  I needed a work area for operators, and initially used INBUF however that got
;  repurposed for managing strings (in particular handling \Xnn and double quotes)
;  so now I am using the memory at the current PCODE address as a work area.
;  It is only needed during expression evaluation, and should hopefully be available.
;
;
;  push an operator (A) onto the operator stack
;   changes X and Y
;
exp_push_operator:
  ldx exp_operator_stack_count
  cpx #250  ; make sure we don't overflow our operator stack
  bcc exp_push_operator_ok
  ldx #46   ; ERROR: Expression too complex
  jmp ass_error
exp_push_operator_ok:
  inc exp_operator_stack_count  ; add, then store
  ldy exp_operator_stack_count
  sta (PCODE),Y
  rts

;
;  pop an operator (into A) from the operator stack
;   changes X and Y
;
exp_pop_operator:
  ldx exp_operator_stack_count
  bne exp_pop_operator_ok
  ldx #23   ; ERROR: Illegal factor
  jmp ass_error
exp_pop_operator_ok:
  ldy exp_operator_stack_count  ; retrieve, then decrement
  lda (PCODE),Y
  dec exp_operator_stack_count
  rts

;
;  get an operator (into A) from the operator stack (without removing it)
;   changes X and Y
;
exp_get_operator:
  ldx exp_operator_stack_count
  bne exp_get_operator_ok
  ldx #23   ; ERROR: Illegal factor
  jmp ass_error
exp_get_operator_ok:
  ldy exp_operator_stack_count
  lda (PCODE),Y
  rts

 .macro makePrecedenceTable ; table of operators, precedence, handler
   dfb   \1
   dfb   \2
   word  \3
 .endmacro

; cannot have more than 64 entries because we index into this with X
exp_precedence_table:
  makePrecedenceTable '~', 1, exp_negate
  makePrecedenceTable '!', 1, exp_not
  makePrecedenceTable TOKEN_UNARY_MINUS, 1, exp_unary_minus
  makePrecedenceTable TOKEN_LOW_BYTE, 1, exp_low_byte
  makePrecedenceTable TOKEN_HIGH_BYTE, 1, exp_high_byte
  makePrecedenceTable TOKEN_SHIFT_LEFT, 2, exp_shift_left
  makePrecedenceTable TOKEN_SHIFT_RIGHT, 2, exp_shift_right
  makePrecedenceTable '&', 3, exp_bitwise_and
  makePrecedenceTable '^', 4, exp_bitwise_xor
  makePrecedenceTable '|', 5, exp_bitwise_or
  makePrecedenceTable '*', 6, exp_multiply
  makePrecedenceTable '/', 6, exp_divide
  makePrecedenceTable '%', 6, exp_modulo
  makePrecedenceTable '+', 7, exp_add
  makePrecedenceTable '-', 7, exp_subtract
  makePrecedenceTable '<', 8, exp_less_than
  makePrecedenceTable '>', 8, exp_greater_than
  makePrecedenceTable TOKEN_LEQ, 8, exp_leq
  makePrecedenceTable TOKEN_GEQ, 8, exp_geq
  makePrecedenceTable TOKEN_EQUALITY, 9, exp_eql
  makePrecedenceTable TOKEN_NEQ, 9, exp_neq     ; <>
  makePrecedenceTable TOKEN_INEQUALITY, 9, exp_neq  ; !=
  makePrecedenceTable TOKEN_LOGICAL_AND, 10, exp_logical_and
  makePrecedenceTable TOKEN_LOGICAL_OR, 11, exp_logical_or

exp_precedence_table_end = *

;
;  finds precedence of operator in A, returns it in Y (returns 0 if not found)
;    changes X, A
;  also stores the operator evaluation function (eg. exp_add) in exp_evaluation_function
;
exp_get_precedence:
  ldx #0
exp_get_precedence_loop:
  cmp exp_precedence_table,x
  beq exp_get_precedence_found
  inx   ; skip operator
  inx   ; skip precedence
  inx   ; skip evaluation function
  inx   ;  (2 bytes)
  cpx #exp_precedence_table_end - exp_precedence_table
  bcc exp_get_precedence_loop
  ldy #0
  stz exp_evaluation_function
  stz exp_evaluation_function+1
  rts
exp_get_precedence_found:
  inx
  ldy exp_precedence_table,x
  inx
  lda exp_precedence_table,x
  sta exp_evaluation_function
  inx
  lda exp_precedence_table,x
  sta exp_evaluation_function+1
  rts

;
;  Convert an expression into a value
;
;   Ideas from https://www.geeksforgeeks.org/expression-evaluation/
;
ass_get_value:
;
;  set up expression evaluation stack
;
  stz exp_value_stack_count
  stz exp_operator_stack_count
  lda ENDSYM
  sta exp_value_stack
  lda ENDSYM+1
  sta exp_value_stack+1

  stz ASS_VALUE
  stz ASS_VALUE+1
  stz ASS_VALUE+2
  lda #1
  sta exp_unary_ok  ; unary operators can appear at the start of the expression
  lda token_type
;  jsr START_TRACE
;
;  one token lookahead
;
  bra ass_expression_loop_initial

;
;  1.1 Get the next token
;
ass_expression_loop:
  jsr get_token
ass_expression_loop_initial:

;
;  1.2.1 If it's a number, push onto the value stack
;
  cmp #TOKEN_NUMBER
  bne ass_expression_loop1a
ass_expression_number:
  lda token_value
  sta VALUE
  lda token_value+1
  sta VALUE+1
  lda token_value+2
  sta VALUE+2
  jsr exp_push_value
  stz exp_unary_ok          ; don't have unary operators after numbers
  bra ass_expression_loop
;
;  if it's a string and only 1 or 2 bytes, consider as a number
;
ass_expression_loop1a:
  cmp #TOKEN_STRING
  bne ass_expression_loop1
  lda token_length
  cmp #3
  bcs ass_expression_not_small_string  ; too long?
;
;  one or two character string - is this an immediate operand?
;
  lda ASS_OPERAND
  cmp #ASS_OPERAND_IMMEDIATE
  bne ass_expression_not_small_string
  bra ass_expression_number   ; treat as a number (its value should be in VALUE)

ass_expression_not_small_string:
  ldx #29     ; string literal too big
  jmp ass_error

;
;  1.2.2 If it's an identifier, get its value and push onto the value stack
;
ass_expression_loop1:
  cmp #TOKEN_IDENTIFIER
  bne ass_expression_loop2
  stz exp_unary_ok          ; don't have unary operators after identifiers

  jsr SEARCH    ; should be in symbol table as a constant, and the value in VALUE
  bne ass_expression_found_value
  lda ASS_PASS
  beq ass_label_not_found_yet

;
; it's an error if it isn't there on the second pass
;
  ldx #11  ; ERROR: Undeclared Identifier
  jmp ass_error

;
;  label not found on first pass, make the value 0x6666 for now
;   - we don't want a zero-page address assumption because that
;     will throw out opcode lengths (it might assume a zero page opcode
;     which is shorter and then change it to a longer one)
;
ass_label_not_found_yet:
  lda #$66
  sta VALUE
  sta VALUE+1
  stz VALUE+2
ass_expression_found_value:
  lda token_value
  sta VALUE
  lda token_value+1
  sta VALUE+1
  lda token_value+2
  sta VALUE+2
  jsr exp_push_value
  bra ass_expression_loop

;
; 1.2.3 If it's a left parenthesis: push it onto the operator stack.
;
ass_expression_loop2:
  cmp #'('
  bne ass_expression_loop3
  pha
  lda  #1
  sta exp_unary_ok          ; unary operators can follow left parentheses
  pla

  jsr exp_push_operator
  bra ass_expression_loop

;
;  1.2.4 A right parenthesis:
;       1 While the thing on top of the operator stack is not a
;         left parenthesis,
;           1 Pop the operator from the operator stack.
;           2 Pop the value stack twice, getting two operands.
;           3 Apply the operator to the operands, in the correct order.
;           4 Push the result onto the value stack.
;        2 Pop the left parenthesis from the operator stack, and discard it.


ass_expression_loop3:
  cmp #')'
  bne ass_expression_loop4
ass_expression_loop3b:
;
;  a RH parenthesis with an empty operator stack is probably the end of the expression, eg. ($1234),x
;  where the ")" belongs to the indexed operand stuff and not the expression
;
  lda exp_operator_stack_count
  beq exp_no_operators_left
  jsr exp_get_operator
  cmp #'('
  beq ass_expression_loop3c
;
;  not a left parenthesis
;
ass_expression_loop3a:
  jsr ass_expression_apply_operator ; pop the operator and two values, apply the operator and push the result
  bra ass_expression_loop3b          ; keep going until we don't have a left parenthesis

ass_expression_loop3c:
  jsr exp_pop_operator              ;  2 Pop the left parenthesis from the operator stack, and discard it.
ass_expression_loopJ:
  jmp ass_expression_loop

;       1.2.5 An operator (call it thisOp):
;         1 While the operator stack is not empty, and the top thing on the
;           operator stack has the same or greater precedence as thisOp,
;           1 Pop the operator from the operator stack.
;           2 Pop the value stack twice, getting two operands.
;           3 Apply the operator to the operands, in the correct order.
;           4 Push the result onto the value stack.
;         2 Push thisOp onto the operator stack.

ass_expression_loop4:
;
;  look for unary operators
;
  ldx exp_unary_ok
  beq ass_expression_cannot_be_unary
  cmp #'-'
  bne ass_not_unary_minus
  lda #TOKEN_UNARY_MINUS
  sta token_type
  bra ass_expression_cannot_be_unary
ass_not_unary_minus:
  cmp #'<'
  bne ass_not_unary_less_than
  lda #TOKEN_LOW_BYTE
  sta token_type
  bra ass_expression_cannot_be_unary
ass_not_unary_less_than:
  cmp #'>'
  bne ass_not_unary_greater_than
  lda #TOKEN_HIGH_BYTE
  sta token_type
  bra ass_not_unary_greater_than
ass_not_unary_greater_than:
  cmp #'*'
  bne ass_expression_cannot_be_unary
;
;  A '*' where a unary expression is allowed will be the current emit address (eg. foo = *)
;
  lda PCODE
  sta token_value
  lda PCODE+1
  sta token_value+1
  stz token_value+2
  stz exp_unary_ok          ; don't have unary operators after numbers
  lda #TOKEN_NUMBER
  sta token_type            ; make get_token think a sign as a token on its own
  bra ass_expression_found_value

ass_expression_cannot_be_unary:
  pha
  lda #1
  sta exp_unary_ok          ; can have unary operators after operators
  pla

  jsr exp_get_precedence
  sty exp_operator_precedence ; thisOp's precedence
  cpy #0
  beq exp_no_operators_left

ass_expression_loop7:         ; 1 While the operator stack is not empty
  lda exp_operator_stack_count
  beq ass_expression_loop6    ; stack empty
  jsr exp_get_operator    ; find the top thing on the operator stack
  cmp #'('                ; if a bracket, consider that low precedence
  beq ass_expression_loop6
  jsr exp_get_precedence  ; and its precedence
  cpy exp_operator_precedence
  bcs ass_expression_loop6  ; thing on top has less precedence (a higher value) than thisOp
  jsr ass_expression_apply_operator  ; pop the operator and two values, apply the operator and push the result
  bra ass_expression_loop7

ass_expression_loop6:
  lda token_type       ; thisOp
  jsr exp_push_operator
  jmp ass_expression_loop


; 2. While the operator stack is not empty,
;     1 Pop the operator from the operator stack.
;     2 Pop the value stack twice, getting two operands.
;     3 Apply the operator to the operands, in the correct order.
;     4 Push the result onto the value stack.


exp_no_operators_left:
    lda exp_operator_stack_count
    beq exp_done
    jsr ass_expression_apply_operator ; pop the operator and two values, apply the operator and push the result
    bra exp_no_operators_left

exp_done:
      lda exp_value_stack_count
      cmp #1
      beq exp_done_ok
exp_done_error:
      ldx #23       ; ERROR: Illegal factor
      jmp ass_error

exp_done_ok:
      lda exp_operator_stack_count
      bne exp_done_error
      jsr exp_pop_value   ; pop the finished value
      lda VALUE
      sta ASS_VALUE
      lda VALUE+1
      sta ASS_VALUE+1
      stz ASS_VALUE+2   ; make high-order byte zero just in case
      rts

;
;
;     1 Pop the operator from the operator stack.
;     2 Pop the value stack twice, getting two operands.
;     3 Apply the operator to the operands, in the correct order.
;     4 Push the result onto the value stack.

;
ass_expression_apply_operator:
  jsr exp_pop_value         ; 2 Pop the value stack twice, getting two operands.
  jsr exp_get_operator
  cmp #'~'
  beq ass_expression_unary
  cmp #'!'
  beq ass_expression_unary
  cmp #TOKEN_UNARY_MINUS
  beq ass_expression_unary
  cmp #TOKEN_LOW_BYTE
  beq ass_expression_unary
  cmp #TOKEN_HIGH_BYTE
  beq ass_expression_unary

;
;  if a binary operator get the second value
;
  lda VALUE                 ; copy top value to VALUE2
  sta VALUE2
  lda VALUE+1
  sta VALUE2+1
  lda VALUE+2
  sta VALUE2+2
  jsr exp_pop_value         ; second pop
ass_expression_unary:

;
;  now we have the top two values in VALUE (and VALUE2 in the case of binary operators)
;
  jsr exp_pop_operator      ; 1 Pop the operator from the operator stack.
  sta exp_operator
  jsr exp_get_precedence    ; find the evaluation function
  cpy #0
  beq ass_expression_apply_operator ; should not be here if the operator isn't in the table
  jsr ass_evaluate_operator ; do the evaluation
  jsr exp_push_value        ; 4 Push the result onto the value stack.
  rts

ass_expression_apply_operator_bug:
  ldx #6
  jmp ass_error

ass_evaluate_operator:
  jmp (exp_evaluation_function)


;
;  look up opcodes in a table
;  returns carry set if found, carry clear if not found
;
ass_lookup_loop:
  ldy OPCODE_LEN
  jsr COMSTL
  beq ass_opcode_found
;
;  add the opcode length, plus 1 to DEST
;
  clc
  ldy OPCODE_LEN
  iny
  tya
  adc DEST
  sta DEST
  lda DEST+1
  adc #0
  sta DEST+1
;
;  end of table?
;
  ldy #0
  lda (DEST),Y
  bne ass_lookup_loop   ; not yet
  clc
  rts


ass_opcode_found:
  ldy OPCODE_LEN
  lda (DEST),Y
  sec   ; indicate found
  rts

ass_emit:
  ldy ASS_PASS
  beq ass_emit3  ; only emit codes on 2nd pass
  ;
  ;  first time for this line of code, indent by two spaces
  ;
  ldy ASS_EMIT_COUNT
  bne ass_emit2
  ldy DCODE
  beq ass_emit2
  pha
  jsr PUTSP
  jsr PUTSP
  pla
ass_emit2:
;
;  if this is the *first* byte we emitted, then make that the start address
;  for running the code. That is, if you relocate the code, the first byte
;  actually emitted will be considered the runtime address.
;
;  this lets you relocate code (eg. to $4500) and have it run from there if
;  you happen to hit Run.
;
  pha
  lda ass_emit_bytes
  ora ass_emit_bytes+1
  bne ass_already_emitted
  lda PCODE
  sta ACT_PCDA
  lda PCODE+1
  sta ACT_PCDA+1
ass_already_emitted:
  pla
;
;  write the byte to memory
;
  ldy #0
  sta (PCODE),Y
  inc ass_emit_bytes
  bne ass_emit4
  inc ass_emit_bytes+1
ass_emit4:

;
;  if displaying codes, show the emitted byte
;
  ldy DCODE
  beq ass_emit3
  jsr DISHX
;
;  increment the PCODE (output) address
;
ass_emit3:
  inc PCODE
  bne ass_emit1
  inc PCODE+1
ass_emit1:
  inc ASS_EMIT_COUNT
  rts

ass_error:
  lda ASS_EMIT_COUNT
  beq ass_error1
  jsr CROUT
ass_error1:
  jmp ERROR

ass_fixup_bbr_branch:
  lda REMAIN+1
  bne ass_emit_zero_page_too_high
  lda REMAIN+2
  bne ass_emit_zero_page_too_high
  lda REMAIN
  jsr ass_emit  ; output zero page address

;
;  now fall down and emit the branch
;

ass_fixup_branch:
  clc         ; I want to subtract one more, so I CLEAR carry <sigh>
  lda ASS_VALUE
  sbc PCODE
  sta ASS_VALUE

  lda ASS_VALUE+1
  sbc PCODE+1
  cmp #$FF  ; backwards branch
  beq ass_branch_backwards
  cmp #0
  bne ass_branch_out_of_range
  lda ASS_VALUE ; must be 0x00 to 0x7F
  bmi ass_branch_out_of_range
  bra ass_branch_ok

ass_branch_backwards:
  lda ASS_VALUE   ; must be 0x80 to 0xFF
  bpl ass_branch_out_of_range

ass_branch_ok:
  lda ASS_VALUE
  jsr ass_emit  ; output relative address
  rts

ass_branch_out_of_range:
  lda ASS_PASS
  beq ass_branch_ok   ; out of range is OK on first pass, we don't know how far the branch is
  ldx #42
  jmp ass_error  ; branch out of range

ass_emit_zero_page:
  lda ASS_VALUE+1
  bne ass_emit_zero_page_too_high
  lda ASS_VALUE+2
  bne ass_emit_zero_page_too_high
  lda ASS_VALUE
  jsr ass_emit  ; output zero page address
  rts


ass_emit_zero_page_too_high:
  ldx #43       ; Need zero page address
  jmp ass_error

ass_emit_immediate:
  lda ASS_VALUE+1
  bne ass_emit_immediate_too_high
  lda ASS_VALUE+2
  bne ass_emit_immediate_too_high
  lda ASS_VALUE
  jsr ass_emit  ; output immediate operand
  rts

ass_emit_immediate_too_high:
  ldx #44
  jmp ass_error


ass_emit_absolute:
  lda ASS_VALUE
  jsr ass_emit  ; output absolute address - low-order byte
  lda ASS_VALUE+1
  jsr ass_emit  ; output absolute address - high-order byte
  rts


;------------------------------------
;  Opcode master table
;------------------------------------

   .macro makeOpcodeLookup ; table of opcodes, length of opcode, wanted operand type, special handler
     dfb   \1
     dfb   \2
     word  \3
     word   \4
   .endmacro

;
;  So, for an opcode to match, it has to:
;     a) be the right length (usually 3 or 4 characters)
;     b) have the right type of operand (eg. ASS_OPERAND_IMMEDIATE)
;     c) the opcode must match the name in the list
;
;   If those conditions are met we output the corresponding opcode,
;   return with the operand type in A
;
;   Otherwise, we return 0 in A
;

OPCODE_LOOKUP_OPCODE_LENGTH = 0
OPCODE_LOOKUP_OPERAND_TYPE = 1
OPCODE_LOOKUP_OPCODE_TABLE = 2    ; 2 bytes
OPCODE_LOOKUP_HANDLER = 4         ; 2 bytes
OPCODE_LOOKUP_TABLE_SIZE = 6      ; all of the above

OPCODE_LOOKUP_TABLE = *
;
;  zero-page opcodes go first as they are preferred if we are accessing a zero-page address
;
    makeOpcodeLookup  3,  ASS_OPERAND_ZERO_PAGE, OPCODE_ASS_OPERAND_ZERO_PAGE, ass_emit_zero_page ; (11)
    makeOpcodeLookup  1,  ASS_OPERAND_ZERO_PAGE, OPCODE_ASS_OPERAND_ZERO_PAGE_TOKENISED, ass_emit_zero_page ; (11)
    makeOpcodeLookup  3,  ASS_OPERAND_ZERO_PAGE_INDEXED_INDIRECT, OPCODE_ASS_OPERAND_ZERO_PAGE_INDEXED_INDIRECT, ass_emit_zero_page ; (12)
    makeOpcodeLookup  1,  ASS_OPERAND_ZERO_PAGE_INDEXED_INDIRECT, OPCODE_ASS_OPERAND_ZERO_PAGE_INDEXED_INDIRECT_TOKENISED, ass_emit_zero_page ; (12)
    makeOpcodeLookup  3,  ASS_OPERAND_ZERO_PAGE_INDEXED_WITH_X, OPCODE_ASS_OPERAND_ZERO_PAGE_INDEXED_WITH_X, ass_emit_zero_page ; (13)
    makeOpcodeLookup  1,  ASS_OPERAND_ZERO_PAGE_INDEXED_WITH_X, OPCODE_ASS_OPERAND_ZERO_PAGE_INDEXED_WITH_X_TOKENISED, ass_emit_zero_page ; (13)
    makeOpcodeLookup  3,  ASS_OPERAND_ZERO_PAGE_INDEXED_WITH_Y, OPCODE_ASS_OPERAND_ZERO_PAGE_INDEXED_WITH_Y, ass_emit_zero_page ; (14)
    makeOpcodeLookup  3,  ASS_OPERAND_ZERO_PAGE_INDIRECT, OPCODE_ASS_OPERAND_ZERO_PAGE_INDIRECT, ass_emit_zero_page ; (15)
    makeOpcodeLookup  1,  ASS_OPERAND_ZERO_PAGE_INDIRECT, OPCODE_ASS_OPERAND_ZERO_PAGE_INDIRECT_TOKENISED, ass_emit_zero_page ; (15)
    makeOpcodeLookup  3,  ASS_OPERAND_ZERO_PAGE_INDIRECT_INDEXED_WITH_Y, OPCODE_ASS_OPERAND_ZERO_PAGE_INDIRECT_INDEXED_WITH_Y, ass_emit_zero_page ; (16)
    makeOpcodeLookup  1,  ASS_OPERAND_ZERO_PAGE_INDIRECT_INDEXED_WITH_Y, OPCODE_ASS_OPERAND_ZERO_PAGE_INDIRECT_INDEXED_WITH_Y_TOKENISED, ass_emit_zero_page ; (16)

;
;  non zero-page opcodes
;
    makeOpcodeLookup  4,  ASS_OPERAND_ABSOLUTE, OPCODE_4_CHAR_ZERO_PAGE,  ass_emit_zero_page  ; (11)
    makeOpcodeLookup  3,  ASS_OPERAND_ABSOLUTE, OPCODE_ASS_OPERAND_ABSOLUTE,  ass_emit_absolute  ; (1)
    makeOpcodeLookup  1,  ASS_OPERAND_ABSOLUTE, OPCODE_ASS_OPERAND_ABSOLUTE_TOKENISED,  ass_emit_absolute  ; (1)

    makeOpcodeLookup  3,  ASS_OPERAND_ABSOLUTE_INDEXED_INDIRECT, OPCODE_ASS_OPERAND_ABSOLUTE_INDEXED_INDIRECT, ass_emit_absolute ; (2)
    makeOpcodeLookup  3,  ASS_OPERAND_ABSOLUTE_INDEXED_WITH_X, OPCODE_ASS_OPERAND_ABSOLUTE_INDEXED_WITH_X, ass_emit_absolute ; (3)
    makeOpcodeLookup  1,  ASS_OPERAND_ABSOLUTE_INDEXED_WITH_X, OPCODE_ASS_OPERAND_ABSOLUTE_INDEXED_WITH_X_TOKENISED, ass_emit_absolute ; (3)
    makeOpcodeLookup  3,  ASS_OPERAND_ABSOLUTE_INDEXED_WITH_Y, OPCODE_ASS_OPERAND_ABSOLUTE_INDEXED_WITH_Y, ass_emit_absolute ; (4)
    makeOpcodeLookup  1,  ASS_OPERAND_ABSOLUTE_INDEXED_WITH_Y, OPCODE_ASS_OPERAND_ABSOLUTE_INDEXED_WITH_Y_TOKENISED, ass_emit_absolute ; (4)
    makeOpcodeLookup  3,  ASS_OPERAND_ABSOLUTE_INDIRECT, OPCODE_ASS_OPERAND_ABSOLUTE_INDIRECT, ass_emit_absolute ; (5)
    makeOpcodeLookup  1,  ASS_OPERAND_ABSOLUTE_INDIRECT, OPCODE_ASS_OPERAND_IMMEDIATE_TOKENISED, ass_emit_absolute ; (5)
    makeOpcodeLookup  3,  ASS_OPERAND_ACCUMULATOR_A, OPCODE_ASS_OPERAND_ACCUMULATOR_A, 0 ; (6)
    makeOpcodeLookup  3,  ASS_OPERAND_IMMEDIATE, OPCODE_ASS_OPERAND_IMMEDIATE, ass_emit_immediate ; (7)
    makeOpcodeLookup  1,  ASS_OPERAND_IMMEDIATE, OPCODE_ASS_OPERAND_IMMEDIATE_TOKENISED, ass_emit_immediate ; (7)
    makeOpcodeLookup  3,  ASS_OPERAND_IMPLIED,  OPCODE_3_CHAR_IMPLIED,    0  ; (8) and (10)
    makeOpcodeLookup  4,  ASS_OPERAND_ZERO_PAGE_AND_ABSOLUTE, OPCODE_4_CHAR_BRANCH,     ass_fixup_bbr_branch  ; (9)
    makeOpcodeLookup  3,  ASS_OPERAND_ABSOLUTE, OPCODE_3_CHAR_BRANCH,     ass_fixup_branch  ; (9)
    dfb 0


;------------------------------------
;  directives
;------------------------------------


DIRECTIVES = *
  makeHandler "ASSERT",   asm_do_assert
  makeHandler "LIST",     asm_do_list
  makeHandler "NOLIST",   asm_do_nolist
  makeHandler "ASC",      asm_do_asc
  makeHandler "ASCII",    asm_do_asc
  makeHandler "ASCIIZ",   asm_do_string
  makeHandler "STRING",   asm_do_string
  makeHandler "EQU",      asm_do_equ
  makeHandler "=",        asm_do_equ
  makeHandler "ORG",      asm_do_org
  makeHandler "DFB",      asm_do_dfb
  makeHandler "DFW",      asm_do_dfw
  makeHandler "WORD",     asm_do_dfw
  makeHandler "BLK",      asm_do_blk
  makeHandler "RESERVE",  asm_do_reserve
  makeHandler "SYM",      asm_do_sym

  dfb 0

;------------------------------------
;  4-character branch opcodes
;------------------------------------


OPCODE_4_CHAR_BRANCH = *
  asc "BBR0",$0F
  asc "BBR1",$1F
  asc "BBR2",$2F
  asc "BBR3",$3F
  asc "BBR4",$4F
  asc "BBR5",$5F
  asc "BBR6",$6F
  asc "BBR7",$7F
  asc "BBS0",$8F
  asc "BBS1",$9F
  asc "BBS2",$AF
  asc "BBS3",$BF
  asc "BBS4",$CF
  asc "BBS5",$DF
  asc "BBS6",$EF
  asc "BBS7",$FF
  dfb 0

;------------------------------------
;  3-character branch opcodes
;------------------------------------


OPCODE_3_CHAR_BRANCH = *
  asc "BCC",$90
  asc "BCS",$B0
  asc "BEQ",$F0
  asc "BMI",$30
  asc "BNE",$D0
  asc "BPL",$10
  asc "BRA",$80
  asc "BVC",$50
  asc "BVS",$70
  dfb 0


;------------------------------------
;  3-character implied opcodes (no operand)
;------------------------------------

OPCODE_3_CHAR_IMPLIED = *
  asc "BRK",$00
  asc "CLC",$18
  asc "CLD",$D8
  asc "CLI",$58
  asc "CLV",$B8
  asc "DEX",$CA
  asc "DEY",$88
  asc "INX",$E8
  asc "INY",$C8
  asc "NOP",$EA
  asc "PHA",$48
  asc "PHP",$08
  asc "PHX",$DA
  asc "PHY",$5A
  asc "PLA",$68
  asc "PLP",$28
  asc "PLX",$FA
  asc "PLY",$7A
  asc "RTI",$40
  asc "RTS",$60
  asc "SEC",$38
  asc "SED",$F8
  asc "SEI",$78
  asc "STP",$DB
  asc "TAX",$AA
  asc "TAY",$A8
  asc "TSX",$BA
  asc "TXA",$8A
  asc "TXS",$9A
  asc "TYA",$98
  asc "WAI",$CB
  dfb 0

;------------------------------------
;  4-character zero page opcodes (operand must be zero-page address)
;------------------------------------
OPCODE_4_CHAR_ZERO_PAGE = *
  asc "SMB0",$87
  asc "SMB1",$97
  asc "SMB2",$A7
  asc "SMB3",$B7
  asc "SMB4",$C7
  asc "SMB5",$D7
  asc "SMB6",$E7
  asc "SMB7",$F7
  asc "RMB0",$07
  asc "RMB1",$17
  asc "RMB2",$27
  asc "RMB3",$37
  asc "RMB4",$47
  asc "RMB5",$57
  asc "RMB6",$67
  asc "RMB7",$77
  dfb 0

;------------------------------------
;  ASS_OPERAND_ABSOLUTE
;------------------------------------
OPCODE_ASS_OPERAND_ABSOLUTE = *
  asc "ADC",$6D
  asc "AND",$2D
  asc "ASL",$0E
  asc "BIT",$2C
  asc "CMP",$CD
  asc "CPX",$EC
  asc "CPY",$CC
  asc "DEC",$CE
  asc "EOR",$4D
  asc "INC",$EE
  asc "JMP",$4C
  asc "JSR",$20
  asc "LDA",$AD
  asc "LDX",$AE
  asc "LDY",$AC
  asc "LSR",$4E
  asc "ORA",$0D
  asc "ROL",$2E
  asc "ROR",$6E
  asc "SBC",$ED
  asc "STA",$8D
  asc "STX",$8E
  asc "STY",$8C
  asc "STZ",$9C
  asc "TRB",$1C
  asc "TSB",$0C
  dfb 0

;------------------------------------
;  ASS_OPERAND_ABSOLUTE - for tokenised opcodes like AND
;------------------------------------
OPCODE_ASS_OPERAND_ABSOLUTE_TOKENISED = *
  dfb TOKEN_AND,$2D
  dfb 0


;------------------------------------
;  ASS_OPERAND_ABSOLUTE_INDEXED_INDIRECT
;------------------------------------
OPCODE_ASS_OPERAND_ABSOLUTE_INDEXED_INDIRECT = *
  asc "JMP",$7C
  dfb 0

;------------------------------------
;  ASS_OPERAND_ABSOLUTE_INDEXED_WITH_X
;------------------------------------
OPCODE_ASS_OPERAND_ABSOLUTE_INDEXED_WITH_X = *
  asc "ADC",$7D
  asc "AND",$3D
  asc "ASL",$1E
  asc "BIT",$3C
  asc "CMP",$DD
  asc "DEC",$DE
  asc "EOR",$5D
  asc "INC",$FE
  asc "LDA",$BD
  asc "LDY",$BC
  asc "LSR",$5E
  asc "ORA",$1D
  asc "ROL",$3E
  asc "ROR",$7E
  asc "SBC",$FD
  asc "STA",$9D
  asc "STZ",$9E
  dfb 0

;------------------------------------
;  ASS_OPERAND_ABSOLUTE_INDEXED_WITH_X - for tokenised opcodes like AND
;------------------------------------
OPCODE_ASS_OPERAND_ABSOLUTE_INDEXED_WITH_X_TOKENISED = *
  dfb TOKEN_AND,$3D
  dfb 0

;------------------------------------
;  ASS_OPERAND_ABSOLUTE_INDEXED_WITH_Y
;------------------------------------
OPCODE_ASS_OPERAND_ABSOLUTE_INDEXED_WITH_Y = *
  asc "ADC",$79
  asc "AND",$39
  asc "CMP",$D9
  asc "EOR",$59
  asc "LDA",$B9
  asc "LDX",$BE
  asc "ORA",$19
  asc "SBC",$F9
  asc "STA",$99
  dfb 0

;------------------------------------
;  ASS_OPERAND_ABSOLUTE_INDEXED_WITH_Y - for tokenised opcodes like AND
;------------------------------------
OPCODE_ASS_OPERAND_ABSOLUTE_INDEXED_WITH_Y_TOKENISED = *
  dfb TOKEN_AND,$39
  dfb 0

;------------------------------------
;  ASS_OPERAND_ACCUMULATOR_A
;------------------------------------
OPCODE_ASS_OPERAND_ACCUMULATOR_A = *
  asc "ASL",$0A
  asc "DEC",$3A
  asc "INC",$1A
  asc "LSR",$4A
  asc "ROL",$2A
  asc "ROR",$6A
  dfb 0

;------------------------------------
;  ASS_OPERAND_ABSOLUTE_INDIRECT
;------------------------------------
OPCODE_ASS_OPERAND_ABSOLUTE_INDIRECT = *
  asc "JMP",$6C
  dfb 0

;------------------------------------
;  ASS_OPERAND_IMMEDIATE
;------------------------------------
OPCODE_ASS_OPERAND_IMMEDIATE = *
  asc "ADC",$69
  asc "AND",$29
  asc "BIT",$89
  asc "CMP",$C9
  asc "CPX",$E0
  asc "CPY",$C0
  asc "EOR",$49
  asc "LDA",$A9
  asc "LDX",$A2
  asc "LDY",$A0
  asc "ORA",$09
  asc "SBC",$E9
  dfb 0

;------------------------------------
;  ASS_OPERAND_IMMEDIATE - for tokenised opcodes like AND
;------------------------------------
OPCODE_ASS_OPERAND_IMMEDIATE_TOKENISED = *
  dfb TOKEN_AND,$29
  dfb 0

;------------------------------------
;  ASS_OPERAND_ZERO_PAGE
;------------------------------------
OPCODE_ASS_OPERAND_ZERO_PAGE = *
  asc "ADC",$65
  asc "AND",$25
  asc "ASL",$06
  asc "BIT",$24
  asc "CMP",$C5
  asc "CPX",$E4
  asc "CPY",$C4
  asc "DEC",$C6
  asc "EOR",$45
  asc "INC",$E6
  asc "LDA",$A5
  asc "LDX",$A6
  asc "LDY",$A4
  asc "LSR",$46
  asc "ORA",$05
  asc "ROL",$26
  asc "ROR",$66
  asc "SBC",$E5
  asc "STA",$85
  asc "STX",$86
  asc "STY",$84
  asc "STZ",$64
  asc "TRB",$14
  asc "TSB",$04
  dfb 0

;------------------------------------
;  ASS_OPERAND_ZERO_PAGE  - for tokenised opcodes like AND
;------------------------------------
OPCODE_ASS_OPERAND_ZERO_PAGE_TOKENISED = *
  dfb TOKEN_AND,$25
  dfb 0


;------------------------------------
;  ASS_OPERAND_ZERO_PAGE_INDEXED_INDIRECT
;------------------------------------
OPCODE_ASS_OPERAND_ZERO_PAGE_INDEXED_INDIRECT = *
  asc "ADC",$61
  asc "AND",$21
  asc "CMP",$C1
  asc "EOR",$41
  asc "LDA",$A1
  asc "ORA",$01
  asc "SBC",$E1
  asc "STA",$81
  dfb 0

;------------------------------------
;  ASS_OPERAND_ZERO_PAGE_INDEXED_INDIRECT  - for tokenised opcodes like AND
;------------------------------------
OPCODE_ASS_OPERAND_ZERO_PAGE_INDEXED_INDIRECT_TOKENISED  = *
  dfb TOKEN_AND,$21
  dfb 0


;------------------------------------
;  ASS_OPERAND_ZERO_PAGE_INDEXED_WITH_X
;------------------------------------
OPCODE_ASS_OPERAND_ZERO_PAGE_INDEXED_WITH_X = *
  asc "ADC",$75
  asc "AND",$35
  asc "ASL",$16
  asc "BIT",$34
  asc "CMP",$D5
  asc "DEC",$D6
  asc "EOR",$55
  asc "INC",$F6
  asc "LDA",$B5
  asc "LDY",$B4
  asc "LSR",$56
  asc "ORA",$15
  asc "ROL",$36
  asc "ROR",$76
  asc "SBC",$F5
  asc "STA",$95
  asc "STY",$94
  asc "STZ",$74
  dfb 0

;------------------------------------
;  ASS_OPERAND_ZERO_PAGE_INDEXED_WITH_X  - for tokenised opcodes like AND
;------------------------------------
OPCODE_ASS_OPERAND_ZERO_PAGE_INDEXED_WITH_X_TOKENISED = *
  dfb TOKEN_AND,$35
  dfb 0

;------------------------------------
;  ASS_OPERAND_ZERO_PAGE_INDEXED_WITH_Y
;------------------------------------
OPCODE_ASS_OPERAND_ZERO_PAGE_INDEXED_WITH_Y = *
  asc "LDX",$B6
  asc "STX",$96
  dfb 0

;------------------------------------
;  ASS_OPERAND_ZERO_PAGE_INDIRECT
;------------------------------------
OPCODE_ASS_OPERAND_ZERO_PAGE_INDIRECT = *
  asc "ADC",$72
  asc "AND",$32
  asc "CMP",$D2
  asc "EOR",$52
  asc "LDA",$B2
  asc "ORA",$12
  asc "SBC",$F2
  asc "STA",$92
  dfb 0

;------------------------------------
;  ASS_OPERAND_ZERO_PAGE_INDIRECT  - for tokenised opcodes like AND
;------------------------------------
OPCODE_ASS_OPERAND_ZERO_PAGE_INDIRECT_TOKENISED = *
  dfb TOKEN_AND,$32
  dfb 0

;------------------------------------
;  ASS_OPERAND_ZERO_PAGE_INDIRECT_INDEXED_WITH_Y
;------------------------------------
OPCODE_ASS_OPERAND_ZERO_PAGE_INDIRECT_INDEXED_WITH_Y = *
  asc "ADC",$71
  asc "AND",$31
  asc "CMP",$D1
  asc "EOR",$51
  asc "LDA",$B1
  asc "ORA",$11
  asc "SBC",$F1
  asc "STA",$91
  dfb 0

;------------------------------------
;  ASS_OPERAND_ZERO_PAGE_INDIRECT_INDEXED_WITH_Y - for tokenised opcodes like AND
;------------------------------------
OPCODE_ASS_OPERAND_ZERO_PAGE_INDIRECT_INDEXED_WITH_Y_TOKENISED  = *
  dfb TOKEN_AND,$31
  dfb 0


; ---------------------------
; Compiler directives
; ---------------------------

;
;  ASSERT <expression>  - raises an error if <expression> is zero
;
asm_do_assert:
  lda ASS_OPERAND
  cmp #ASS_OPERAND_ABSOLUTE
  bne asm_do_constant_expectedJ
  lda VALUE
  ora VALUE+1
  ora VALUE+2
  beq asm_do_assert_failed
  rts

asm_do_assert_failed:
  ldx #52
  jmp ass_error

asm_do_constant_expectedJ jmp asm_do_constant_expected

;
; LIST (0= none, 1=source, 2=opcodes, 4=symbols, can OR these together)
; LIST  is the same as LIST 3

asm_do_list = *

;   lda #'f'
;   jsr ass_debug_point

  lda ASS_OPERAND
  cmp #ASS_OPERAND_IMPLIED      ; implied (ie. no operand) will resolve to be the same as LIST 3
  beq asm_do_list_no_operand
  cmp #ASS_OPERAND_ABSOLUTE
  beq asm_do_list_ok
  ldx #2    ; ERROR: constant expected
  jmp ass_error

;
;  NOLIST is the same as LIST 0
;
asm_do_nolist:
  lda ASS_OPERAND
  cmp #ASS_OPERAND_IMPLIED
  beq asm_do_nolist_ok              ; should have no operand
  ldx #23   ; ERROR: Illegal factor
  jmp ass_error
asm_do_nolist_ok:
  lda #0
  sta VALUE
  bra asm_do_list_ok

asm_do_list_no_operand:
  lda #3
  sta ASS_VALUE
asm_do_list_ok:
  lda ASS_PASS
  beq asm_do_list_first_pass  ; ignore on first pass

;  lda ASS_VALUE
;  clc
;  adc #'0'
;  jsr ass_debug_point

  lda ASS_VALUE
  and #$01
  beq asm_do_list_not    ; no listing wanted
  lda #FLAG_LIST_SOURCE ; already listing?
  and system_flags
  bne  asm_do_list1   ; yep
  lda #FLAG_LIST_SOURCE
  tsb system_flags
;  jsr token_line        ; list the current line  TODO: don't know, I think it bumps the line number wrongly
  bra asm_do_list1

;
;  turn off list flag
;
asm_do_list_not:
  lda #FLAG_LIST_SOURCE
  trb system_flags

asm_do_list1:


  lda ASS_VALUE
  and #$02
  sta DCODE


  lda ASS_VALUE
  and #$04
  sta show_symbols
;
;  output a newline, to get past the asterisks
;
  jsr CROUT
asm_do_list_first_pass:
  rts

;
;  ASC "String"
;
asm_do_asc = *
  lda ASS_OPERAND
  cmp #ASS_OPERAND_STRING
  beq asm_do_asc_ok
  ldx #8    ; ERROR: incorrect string
  jmp ass_error
;
;  now emit the string
;
asm_do_asc_ok:
  ldy  #0
asm_do_asc_loop:
  lda  INBUF,Y
  iny         ; onto next character in the string
  phy
  jsr ass_emit
  ply

asm_do_asc_next:
  dec token_length
  bne asm_do_asc_loop
  rts


;
;  ASCIIZ / STRING - output a string with a null terminator
;

asm_do_string:
  jsr asm_do_asc    ; do the string
  lda #0            ; and the terminator
  jsr ass_emit
  rts


;
;  EQU
;
;  The label, which must be present, takes on the value of the expression
;

asm_do_equ:
  lda ASS_OPERAND
  cmp #ASS_OPERAND_ABSOLUTE
  bne asm_do_constant_expected
  lda ass_current_label
  ora ass_current_label+1
  bne asm_do_equ_have_label
  ldx #48    ; ERROR: label required
  jmp ass_error

asm_do_equ_have_label:
  ldy #SYMDSP
  lda ASS_VALUE
  sta (ass_current_label),y
  iny
  lda ASS_VALUE+1
  sta (ass_current_label),y
  rts

;
;  ORG
;
;  The program counter becomes the value of the expression
;

asm_do_org:
  lda ASS_OPERAND
  cmp #ASS_OPERAND_ABSOLUTE
  beq asm_do_org_ok
asm_do_constant_expected:
  ldx #2    ; ERROR: constant expected
  jmp ass_error

asm_do_org_ok:
  lda ASS_VALUE
  sta PCODE
  lda ASS_VALUE+1
  sta PCODE+1
  rts

;
;  DFB - define one byte (value being the expression)
;

asm_do_dfb:
  lda ASS_OPERAND
  cmp #ASS_OPERAND_ZERO_PAGE_AND_ABSOLUTE  ; multiple values?
  beq asm_do_dfb_multiple
  cmp #ASS_OPERAND_ABSOLUTE
  bne asm_do_constant_expected
  jsr asm_emit_one_byte
  rts

;
;  so if we got here we thought we had two values (like a BBR0 $12,foo)
;  - the first is in REMAIN and the other in VALUE
;
;  let's emit REMAIN, then VALUE, then look for other values
;
asm_do_dfb_multiple:
  lda REMAIN+1
  bne asm_do_dfb_too_big
  lda REMAIN+2
  bne asm_do_dfb_too_big
  lda REMAIN
  jsr ass_emit  ; emit REMAIN
asm_do_dfb_multiple_loop:
  jsr asm_emit_one_byte  ; emit VALUE
  lda token_type
  cmp #','      ; another?
  bne asm_do_dfb_multiple_done
  jsr get_token
  jsr ass_get_value  ; should have some sort of address now in VALUE
  bra asm_do_dfb_multiple_loop

asm_do_dfb_multiple_done:
  rts


asm_do_dfb_too_big:
  ldx #44
  jmp ass_error

asm_emit_one_byte:
  lda ASS_VALUE+1
  bne asm_do_dfb_too_big
  lda ASS_VALUE+2
  bne asm_do_dfb_too_big
  lda ASS_VALUE
  jsr ass_emit
  rts


;
;  DFW - define two bytes (value being the expression)
;

asm_do_dfw:
  lda ASS_OPERAND
  cmp #ASS_OPERAND_ZERO_PAGE_AND_ABSOLUTE  ; multiple values?
  beq asm_do_dfw_multiple
  cmp #ASS_OPERAND_ABSOLUTE
  bne asm_do_constant_expected
  jsr asm_emit_two_bytes
  rts

;
;  so if we got here we thought we had two values (like a BBR0 $12,foo)
;  - the first is in REMAIN and the other in VALUE
;
;  let's emit REMAIN, then VALUE, then look for other values
;
asm_do_dfw_multiple:
  lda REMAIN+2
  bne asm_do_dfb_too_big
  lda REMAIN
  jsr ass_emit  ; emit REMAIN
  lda REMAIN+1
  jsr ass_emit  ; emit REMAIN+1
asm_do_dfw_multiple_loop:
  jsr asm_emit_two_bytes  ; emit VALUE
  lda token_type
  cmp #','      ; another?
  bne asm_do_dfw_multiple_done
  jsr get_token
  jsr ass_get_value  ; should have some sort of address now in VALUE
  bra asm_do_dfw_multiple_loop

asm_do_dfw_multiple_done:
  rts


asm_emit_two_bytes:
  lda ASS_VALUE+2
  bne asm_do_dfb_too_big
  lda ASS_VALUE
  jsr ass_emit
  lda ASS_VALUE+1
  jsr ass_emit
  rts



;
;  BLK - zero fill <expression> bytes (eg. BLK 5 output 5 zeroes)
;

asm_do_blk:
  lda ASS_OPERAND
  cmp #ASS_OPERAND_ABSOLUTE
  bne asm_do_constant_expectedJ2
  lda ASS_VALUE+2
  bne asm_do_dfb_too_big
;
; handle (obscure) case of them defining zero bytes
;
  lda ASS_VALUE
  ora ASS_VALUE+1
  beq asm_do_blk_done
asm_do_blk_loop:
;
;  emit one zero
;
  lda #0
  jsr ass_emit
  dec ASS_VALUE
  bne asm_do_blk_loop
  dec ASS_VALUE+1
  bpl asm_do_blk_loop
asm_do_blk_done:
  rts

;
;  RESERVE - reserve <expression> bytes without writing to them
;   useful for leaving a gap (eg. for zero-page variables)
;

asm_do_reserve:
  lda ASS_OPERAND
  cmp #ASS_OPERAND_ABSOLUTE
  bne asm_do_constant_expectedJ2
  lda ASS_VALUE+2
  bne asm_do_dfb_too_big
  clc
  lda PCODE
  adc ASS_VALUE
  sta PCODE
  lda PCODE+1
  adc ASS_VALUE+1
  sta PCODE+1
  rts

asm_do_constant_expectedJ2 jmp asm_do_constant_expected

;
;  SYM <address> symbol table relocation
;
asm_do_sym:
  lda ASS_OPERAND
  cmp #ASS_OPERAND_ABSOLUTE
  bne asm_do_constant_expectedJ2
;
;  only relocate symbol table on the first pass
;
  lda ASS_PASS
  bne asm_do_sym_done

  lda  ASS_VALUE
  sta  ENDSYM
  sta  STARTSYM
  lda  ASS_VALUE+1
  sta  ENDSYM+1        ; store symbol table address
  sta  STARTSYM+1
;
;  put our library functions back into the symbol table
;
  jsr  add_assembler_library_functions  ; add our inbuilt functions to the symbol table

asm_do_sym_done:
  rts
;


info_message asciiz "Addr Name\n"

list_symbols  = *
         lda #<info_message
         ldx #>info_message
         jsr print

         lda  ENDSYM+1    ; get the end of the symbol table
         sta  SYMITM+1    ; make it the current symbol
         lda  ENDSYM
         bra  list_symbols_check

list_symbols_next =  *
         ldy  #SYMPRV
         lda  (SYMITM),Y
         tax
         iny
         lda  (SYMITM),Y
         sta  SYMITM+1   ; previous link
         txa
;
;  go to the previous symbol table item
;
list_symbols_check     =  *
         sta  SYMITM
         cmp  STARTSYM
         bne  list_symbols_line
         lda  SYMITM+1
         cmp  STARTSYM+1
         bne  list_symbols_line       ; more to go
         jmp  list_symbols_done       ; finished if symitm and symitm+1 are both zero

list_symbols_line = *
         ldy  #SYMDSP
         iny
         lda  (SYMITM),Y  ; displacement
         jsr  PRBYTE
         dey
         lda  (SYMITM),Y  ; displacement
         jsr  PRBYTE
         lda  #' '
         jsr  PRCHAR
         ldy  #SYMLEN
         lda  (SYMITM),Y  ; length
         tax
         ldy  #SYMNAM
list_symbols_name = *
         lda  (SYMITM),Y  ; name
         phy
         jsr  PRCHAR
         ply
         iny
         dex
         bne list_symbols_name
         jsr CROUT
         jmp list_symbols_next

list_symbols_done = *
         rts

assembler_library_functions_table:
;
;  put them in reverse alphabetic order so they come out in alphabetic
;  order if you request a symbols list
;
  makeAsmLibraryFunction "write_to_serial",      write_to_serial
  makeAsmLibraryFunction "write_to_lcd",         write_to_lcd
  makeAsmLibraryFunction "write_function",       write_function
  makeAsmLibraryFunction "write_char",           write_char
  makeAsmLibraryFunction "value",                VALUE
  makeAsmLibraryFunction "value2",               VALUE2
  makeAsmLibraryFunction "typing_latency",       typing_latency
  makeAsmLibraryFunction "tknjmp",               TKNJMP
  makeAsmLibraryFunction "reset",                START
  makeAsmLibraryFunction "srce",                 SRCE
  makeAsmLibraryFunction "spi_transfer",         spi_transfer
  makeAsmLibraryFunction "spi_ss_low",           spi_ss_low
  makeAsmLibraryFunction "spi_ss_high",          spi_ss_high
  makeAsmLibraryFunction "spi_send_two_bytes",   spi_send_two_bytes
  makeAsmLibraryFunction "spi_init",             spi_init
  makeAsmLibraryFunction "serial_print_message", serial_print_message
  makeAsmLibraryFunction "serial_available",     serial_available
  makeAsmLibraryFunction "nmi",                  RESTART
  makeAsmLibraryFunction "remain",               REMAIN
  makeAsmLibraryFunction "random",               random
  makeAsmLibraryFunction "putsp",                PUTSP
  makeAsmLibraryFunction "print",                print
  makeAsmLibraryFunction "prchar",               PRCHAR
  makeAsmLibraryFunction "prbyte",               PRBYTE
  makeAsmLibraryFunction "pinmode",              pinmode
  makeAsmLibraryFunction "lcd_wait",             lcd_wait
  makeAsmLibraryFunction "lcd_second_line",      lcd_second_line
  makeAsmLibraryFunction "lcd_print_message",    lcd_print_message
  makeAsmLibraryFunction "lcd_print_char",       lcd_print_char
  makeAsmLibraryFunction "lcd_instruction",      lcd_instruction
  makeAsmLibraryFunction "lcd_home",             lcd_home
  makeAsmLibraryFunction "lcd_get_address",      lcd_get_address
  makeAsmLibraryFunction "lcd_clear_display",    lcd_clear_display
  makeAsmLibraryFunction "isxdigit",             isxdigit
  makeAsmLibraryFunction "isupper",              isupper
  makeAsmLibraryFunction "isspace",              isspace
  makeAsmLibraryFunction "islower",              islower
  makeAsmLibraryFunction "isdigit",              isdigit
  makeAsmLibraryFunction "iscntrl",              iscntrl
  makeAsmLibraryFunction "isalpha",              isalpha
  makeAsmLibraryFunction "isalnum",              isalnum
  makeAsmLibraryFunction "inbuf",                INBUF
  makeAsmLibraryFunction "i2c_send",             i2c_send
  makeAsmLibraryFunction "i2c_receive",          i2c_receive
  makeAsmLibraryFunction "i2c_init",             i2c_init
  makeAsmLibraryFunction "hardware_init",        hardware_init
  makeAsmLibraryFunction "getin",                GETIN
  makeAsmLibraryFunction "gen_random",           gen_random
  makeAsmLibraryFunction "exp_unary_minus",      exp_unary_minus
  makeAsmLibraryFunction "exp_true",             exp_true
  makeAsmLibraryFunction "exp_subtract",         exp_subtract
  makeAsmLibraryFunction "exp_shift_right",      exp_shift_right
  makeAsmLibraryFunction "exp_shift_left",       exp_shift_left
  makeAsmLibraryFunction "exp_not",              exp_not
  makeAsmLibraryFunction "exp_neq",              exp_neq
  makeAsmLibraryFunction "exp_negate",           exp_negate
  makeAsmLibraryFunction "exp_multiply",         exp_multiply
  makeAsmLibraryFunction "exp_modulo",           exp_modulo
  makeAsmLibraryFunction "exp_low_byte",         exp_low_byte
  makeAsmLibraryFunction "exp_logical_or",       exp_logical_or
  makeAsmLibraryFunction "exp_logical_and",      exp_logical_and
  makeAsmLibraryFunction "exp_less_than",        exp_less_than
  makeAsmLibraryFunction "exp_leq",              exp_leq
  makeAsmLibraryFunction "exp_high_byte",        exp_high_byte
  makeAsmLibraryFunction "exp_greater_than",     exp_greater_than
  makeAsmLibraryFunction "exp_geq",              exp_geq
  makeAsmLibraryFunction "exp_false",            exp_false
  makeAsmLibraryFunction "exp_eql",              exp_eql
  makeAsmLibraryFunction "exp_divide",           exp_divide
  makeAsmLibraryFunction "exp_bitwise_xor",      exp_bitwise_xor
  makeAsmLibraryFunction "exp_bitwise_or",       exp_bitwise_or
  makeAsmLibraryFunction "exp_bitwise_and",      exp_bitwise_and
  makeAsmLibraryFunction "exp_add",              exp_add
  makeAsmLibraryFunction "exp_abs_val",          exp_abs_val
  makeAsmLibraryFunction "display_in_decimal",   display_in_decimal
  makeAsmLibraryFunction "digitalwrite",         digitalwrite
  makeAsmLibraryFunction "digitalread",          digitalread
  makeAsmLibraryFunction "dest",                 DEST
  makeAsmLibraryFunction "delay",                delay
  makeAsmLibraryFunction "delay_1ms",            delay_1ms
  makeAsmLibraryFunction "crout",                CROUT
  makeAsmLibraryFunction "crc_val",              crc_val
  makeAsmLibraryFunction "crc_num",              crc_num
  makeAsmLibraryFunction "crc_byte",             crc_byte
  makeAsmLibraryFunction "crc_addr",             crc_addr
  makeAsmLibraryFunction "crc16",                crc16
  makeAsmLibraryFunction "cout",                 COUT
  makeAsmLibraryFunction "comstl",               COMSTL
  makeAsmLibraryFunction "call_y",               call_y
  makeAsmLibraryFunction "call_x",               call_x
  makeAsmLibraryFunction "call_s",               call_s
  makeAsmLibraryFunction "call_p",               call_p
  makeAsmLibraryFunction "call_a",               call_a
  makeAsmLibraryFunction "binary_to_decimal",    binary_to_decimal
  makeAsmLibraryFunction "bcd_result",           bcd_result

  dfb 0   ; end of table


add_assembler_library_functions:
  lda #<assembler_library_functions_table
  sta REG
  lda #>assembler_library_functions_table
  sta REG+1
add_assembler_library_functions_loop:
  ldy #0
  lda (REG),Y
  beq add_assembler_library_functions_done
  lda REG
  sta token_address
  lda REG+1
  sta token_address+1
  stz token_length
;
;  find name length
;
add_assembler_library_functions_name_loop
  iny
  inc token_length
  lda (REG),Y
  bne add_assembler_library_functions_name_loop
  phy
  jsr CHKDUP
  lda #SYMBOL_CONSTANT  ; add the symbol as a constant for now
  jsr ADDSYM
  ply

  iny             ; get next byte
  lda (REG),Y     ; get function address low-byte
  phy
  ldy #SYMDSP
  sta (SYMITM),Y  ; save function address low-byte
  ply

  iny             ; get next byte
  lda (REG),Y     ; get function address high-byte
  phy
  ldy #SYMDSP+1
  sta (SYMITM),Y  ; save function address high-byte
  ply

  iny         ; get past that last byte
  tya         ; move REG onto next entry
  clc
  adc REG
  sta REG
  lda #0
  adc REG+1
  sta REG+1
  bra add_assembler_library_functions_loop

add_assembler_library_functions_done:
  rts
