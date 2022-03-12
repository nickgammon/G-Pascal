;-------------------------------------------------
;
;  Shell, and text editor
;
;-------------------------------------------------


;-------------------------------------------------
;  print null-terminated message, message in A (lo) and X (hi)
;  returns the length of the message in Y
;-------------------------------------------------
print:
  tay   ; save A
  lda REG2 ; save REG2
  pha
  lda REG2+1  ; save REG2+1
  pha
  tya   ; get original A back
  sta REG2
  stx REG2+1
  ldy #0
print_loop:
  lda (REG2),y
  beq print_done
  jsr COUT
  iny
  bne print_loop  ; give up if we overflow
print_done:
;  stz serial_in_byte_received   ; TODO: Not sure about this
;
;  get REG2 back
;
  pla
  sta REG2+1
  pla
  sta REG2
  rts

prompt       asciiz ": "
bad_command  asciiz "Unknown command, type H for help.\n"

;-------------------------------------------------
; Start here after Reset or NMI - show introduction
;-------------------------------------------------

main_start:
  lda #<introduction
  ldx #>introduction
  jsr print

main_prompt:
  ldx  #NEW_STK
  txs
  cld

  lda #<prompt
  ldx #>prompt
  jsr print

  jsr  GET_LINE

  lda #<INBUF
  sta token_start
  lda #>INBUF
  sta token_start+1

  lda #1
  sta RUNNING

;-------------------------------------------------
; EDITOR (SHELL) MAIN LOOP
;-------------------------------------------------

main_loop:
  stz editor_flags  ; no flags yet
  stz find_flags    ;  ditto
  lda #FLAG_COMPILING
  ora #FLAG_ASSEMBLING
  trb system_flags  ; not compiling or assembling now

  lda #FLAG_ONLY_ALPHA
  tsb system_flags  ; we only want alpha commands, thus we can enter L1 and be the same as List 1

  jsr get_token
  beq main_prompt   ; no input

;
;  Shortcut: If the input is "R" or "r" followed directly by a newline, assume they want to Run
;
  cmp #TOKEN_IDENTIFIER
  bne main_loop_not_run

  ldy #0
  lda (token_address),y
  jsr MAKE_UPPER
  cmp #'R'
  bne main_loop_not_run
  iny
  lda (token_address),y
  cmp #NL
  bne main_loop_not_run
  jmp editor_run

;
;  Not a RUN shortcut, look up the command in our table
;
main_loop_not_run:
;
;  set up our input line as where to find tokens
;
  lda token_address
  sta SRCE
  lda token_address+1
  sta SRCE+1

;
;  get editor handlers
;
  lda #<editor_handlers
  sta DEST
  lda #>editor_handlers
  sta DEST+1

;
;  get the token length and look up the handler
;
  lda token_length
  jsr partial_handler_Lookup

;
;  if we returned, unknown command
;
  lda #<bad_command
  ldx #>bad_command
  jsr print
  bra  main_prompt


editor_handlers:
  makeHandler "LIST",    editor_list
  makeHandler "INSERT",  editor_insert
  makeHandler "LOAD",    editor_load
  makeHandler "DELETE",  editor_delete
  makeHandler "HELP",    editor_help
  makeHandler "COMPILE" ,editor_compile
  makeHandler "SYNTAX",  editor_syntax
  makeHandler "SAVE",    editor_save
;  makeHandler "TEST",    editor_test
  makeHandler "MEMORY",  editor_memory
 ; makeHandler "SYMTBL", editor_symbol_table_list
  makeHandler "INFO",    editor_info
  makeHandler "TRACE",   editor_trace
  makeHandler "DEBUG",   editor_debug
  makeHandler "ASSEMBLE",editor_assemble
  makeHandler "FIND",    editor_find
  makeHandler "REPLACE", editor_replace
  makeHandler "RUN",     editor_run
  makeHandler "RESUME",  editor_resume
  makeHandler "RECOVER", editor_recover
  makeHandler "POKE",    editor_poke
  makeHandler "JSR",     editor_jsr
  makeHandler "JMP",     editor_jmp
  dfb 0

editor_help_info:
  asc "Available actions:\n\n"
  asc "List/SAve   line_number_range\n"
  asc "Delete      line_number_range\n"
  asc "Insert/LOad after_line\n"
  asc "Find        line_number_range /target/flags\n"
  asc "Replace     line_number_range /target/replacement/flags\n"
  asc "\n"
  asc "Help\n"
  asc "INfo\n"
  asc "Memory      first_address last_address\n"
  asc "Compile/Syntax/Assemble\n"
  asc "RUn/DEBug/Trace/RESume\n"
  asc "Poke/Jsr/JMp\n"
  asc "RECover\n"
  asc "(Actions may be abbreviated)\n"
  asc "(Flags: 'I'gnore case, 'G'lobal, 'Q'uiet)\n"
  dfb 0


;--------------------------------------------
;  HELP
;--------------------------------------------
editor_help:
  jsr editor_check_no_more
  lda #<editor_help_info
  sta REG
  lda #>editor_help_info
  sta REG+1
editor_help_loop:
  ldy #0
  lda (REG),Y
  beq editor_help_done
  jsr COUT
  inc REG
  bne editor_help_loop
  inc REG+1
  bra editor_help_loop
editor_help_done:
  jmp main_prompt

;
;  Sets default range from 1 to 65535
;
editor_default_range:
;
;  default to all lines
;
  lda #1
  sta from_line
  stz from_line+1
  lda #$FF
  sta to_line
  sta to_line+1
  rts

number_expected_message:
  asciiz "Number expected\n"

editor_bad_number:
  lda #<number_expected_message
  ldx #>number_expected_message
  jsr print
  jmp main_prompt

;
;  get a number for the editor: accepts eof or newline as "no number"
;  otherwise must be a number and in the range 0x0001 to 0xFFFF
;  returns carry set if a number (in token_value) and carry clear if
;  no number. Raises an error otherwise
;
editor_get_number:
  jsr pas_get_token   ; use this to find END as a token
  beq editor_get_number_none ; no token? return with carry set
  cmp #NL
  beq editor_get_number_none ; newline counts as no token
  cmp #TOKEN_NUMBER
  beq editor_get_number_ok
  cmp #TOKEN_END
  beq editor_get_number_end
  cmp #TOKEN_IDENTIFIER
  bne editor_get_number_not_identifier
  lda token_length
  cmp #3
  bne editor_bad_number
  ldy #0
  lda (token_address),y
  jsr MAKE_UPPER
  cmp #'A'
  bne editor_bad_number
  iny
  lda (token_address),y
  jsr MAKE_UPPER
  cmp #'L'
  bne editor_bad_number
  iny
  lda (token_address),y
  jsr MAKE_UPPER
  cmp #'L'
  bne editor_bad_number
  jsr editor_default_range
  jsr editor_check_no_more ; can't have ALL - something
  bra editor_get_number_none
;
;  find / replace can go straight to the delimiter
;
editor_get_number_not_identifier:
  lda #EFLAG_FIND
  ora #EFLAG_REPLACE
  and editor_flags
  beq editor_bad_number
  lda token_length
  cmp #1
  bne editor_bad_number
;
;  go back one character, we need get_token to find this again later
;
  dec token_start
  bne editor_get_number_none
  dec token_start+1
  bra editor_get_number_none

editor_get_number_ok:
  lda token_value+2         ; must be a number
  bne editor_line_number_too_big
  lda editor_flags
  and #EFLAG_ALLOW_ZERO_FROM
  bne editor_get_number_done
  lda token_value
  ora token_value+1
  beq editor_line_number_too_big  ; too small actually
editor_get_number_done:
  sec
  rts

editor_get_number_none:
  clc
  rts

;
;  the word "END" will count as the highest possible line number
;
editor_get_number_end:
  lda #$FF
  sta token_value
  sta token_value+1
  stz token_value+2
  jsr editor_check_no_more ; can't have END - something
  sec
  rts

number_too_large_message:
  asciiz "Number too large or too small\n"

editor_line_number_too_big:
  lda #<number_too_large_message
  ldx #>number_too_large_message
  jsr print
  jmp main_prompt

number_bad_range_message:
  asciiz "Range invalid\n"

editor_bad_range:
  lda #<number_bad_range_message
  ldx #>number_bad_range_message
  jsr print
  jmp main_prompt

too_much_on_line:
  asciiz "Unexpected trailing input on line\n"

;
;  check that all is left on the line is spaces
;
editor_check_no_more:
  ldy #0
editor_check_no_more_loop:
  lda (token_start),y
  beq editor_check_no_more_ok   ; end of file
  cmp #NL
  beq editor_check_no_more_ok   ; newline is OK
  jsr isspace
  bcc editor_check_no_more_failed ; not spaces is bad
  iny
  bne editor_check_no_more_loop   ; skip spaces

editor_check_no_more_failed:
  lda #<too_much_on_line
  ldx #>too_much_on_line
  jsr print
  jmp main_prompt
editor_check_no_more_ok:
  rts



;--------------------------------------------
;  get a range of line numbers (eg. L 1,10; D 4; M 6,10 )
;--------------------------------------------

editor_get_range:


;
;  get the first line number (if any)
;
  jsr editor_get_number
  bcc editor_get_range_done ; no token? take defaults
;
;  store the from line
;
  lda token_value
  sta from_line
  sta to_line
  lda token_value+1
  sta from_line+1
  sta to_line+1

;
;  comma or hyphen?
;
  jsr get_token
  beq editor_get_range_done   ; line done, just the single line wanted

  cmp #'-'
  beq editor_get_range_hyphen
  cmp #TOKEN_NUMBER
  beq editor_get_range_number
  cmp #TOKEN_END
  beq editor_get_range_number
  cmp #','
  beq editor_get_range_hyphen
;
;  find and replace may have another character after the first line number
;  which isn't a second line number or a hyphen/comma
;
  lda #EFLAG_FIND
  ora #EFLAG_REPLACE
  and editor_flags
  beq editor_bad_range

;
;  go back one character, we need get_token to find this again later
;
  dec token_start
  bne editor_get_range_done
  dec token_start+1
  bra editor_get_range_done

editor_get_range_hyphen:
;
;  get the second line number
;
  jsr editor_get_number
  bcc editor_bad_rangeJ

editor_get_range_number:
  lda token_value
  sta to_line
  lda token_value+1
  sta to_line+1

;
;  make sure range ascends
;
  sec
  lda to_line
  sbc from_line
  lda to_line+1
  sbc from_line+1
  bcc editor_bad_rangeJ    ; <sigh> because of the way SBC works

editor_get_range_done:
  rts

editor_bad_rangeJ jmp editor_bad_range

;

found_message asciiz  ' found\n'



from_msg: asciiz "From: "
to_msg:   asciiz " to "

bad_delimiter_message asciiz "Bad delimiter\n"

editor_bad_delimiter:
  lda #<bad_delimiter_message
  ldx #>bad_delimiter_message
editor_find_problem:
  jsr print
  jmp main_prompt

find_string_too_long_message asciiz "Find/replace string too long\n"

find_string_too_long:
  lda #<find_string_too_long_message
  ldx #>find_string_too_long_message
  bra editor_find_problem

editor_get_delimiter:
  jsr get_token
  cmp #TOKEN_IDENTIFIER
  beq editor_bad_delimiter
  cmp #TOKEN_NUMBER
  beq editor_bad_delimiter
  cmp #TOKEN_STRING
  beq editor_bad_delimiter
  ldx token_length
  cpx #1
  bne editor_bad_delimiter
  sta find_delimiter
  rts

editor_no_closing_delimiter_message asciiz "No closing delimiter\n"

editor_no_closing_delimiter:
  lda #<editor_no_closing_delimiter_message
  ldx #>editor_no_closing_delimiter_message
  bra editor_find_problem

no_find_message asciiz "Target string empty\n"

editor_no_find_string:
  lda #<no_find_message
  ldx #>no_find_message
  bra editor_find_problem

;--------------------------------------------
;  LIST / SAVE / FIND / REPLACE / MODIFY
;
;  This rather complex routine lists the source between from_line and to_line
;   In the process it can show or omit line numbers  (omit for SAVE)
;  Also, it can find the target text, and optionally replace it with the replacement text
;  In find/replace mode the lines are not shown unless they had something found (unless
;   the Quiet flag is set)
;
;--------------------------------------------

editor_replace:
  lda #EFLAG_REPLACE
  ora #EFLAG_FIND   ; makes things simpler later on
  sta editor_flags
  bra editor_list

editor_find:

  lda #EFLAG_FIND
  sta editor_flags
  bra editor_list

editor_save

  lda #EFLAG_NO_LINE_NUMBERS
  ora #EFLAG_SAVE
  sta editor_flags

;
;  main entry point - get the from and to lines
;
editor_list:
  jsr editor_default_range
  jsr editor_get_range

;
;  turn the from_line into an address and put it into PCODE
;
  lda from_line
  sta this_line
  lda from_line+1
  sta this_line+1
  jsr find_line    ; find the "from" line address (this_line in fact)
  lda this_line_addr
  sta PCODE
  lda this_line_addr+1
  sta PCODE+1
  stz find_count
  stz find_count+1

;
;  for FIND and REPLACE, get the first delimiter
;
  lda editor_flags
  and #EFLAG_FIND
  bne editor_list_doing_find
  jmp editor_list_not_find

editor_list_doing_find:
  jsr editor_get_delimiter
;
;  token_next should now be the start of what we are searching for
;
  sec
  lda token_start
  sbc #<INBUF
  sta find_from
  bcs editor_list_got_from_point ; delimiter should be a single byte

editor_find_bug:
  ldx #20  ; ERROR: bug
  jmp ERROR

find_string_too_longJ         jmp find_string_too_long
editor_no_closing_delimiterJ  jmp editor_no_closing_delimiter

editor_list_got_from_point:

;
;  Now find closing delimiter by a simple scan as we will pass who-knows-what on the way
;
  ldx find_from
editor_find_second_delimiter_loop:
  inx
  lda INBUF,x
  beq editor_no_closing_delimiterJ
  cmp #NL
  beq editor_no_closing_delimiterJ
  cmp find_delimiter
  bne editor_find_second_delimiter_loop

;
;  found the closing delimiter
;
  stx find_to
  cpx find_from
  beq editor_no_find_string
;
;  maximum 127 byte find string
;
  sec
  lda find_to
  sbc find_from
  bmi find_string_too_longJ
  sta find_len    ; save for later memory move calculations

  lda #EFLAG_REPLACE
  and editor_flags
  beq editor_find_get_flags

;
;  doing a replace - find second delimiter
;

  stx rep_from
  inc rep_from    ; get past the delimiter

editor_find_third_delimiter_loop:
  inx
  lda INBUF,x
  beq editor_no_closing_delimiterJ
  cmp #NL
  beq editor_no_closing_delimiterJ
  cmp find_delimiter
  bne editor_find_third_delimiter_loop

;
;  found the replacement closing delimiter
  stx rep_to

;
;  maximum 127 byte replacement string
;
  sec
  lda rep_to
  sbc rep_from
  bmi find_string_too_longJ

;
;  now find the signed difference between the find and replace strings
;  namely: (rep_to - rep_from) - find_len
;
  stz rep_diff+1
  sec
  lda rep_to
  sbc rep_from
  sbc find_len
  sta rep_diff
  bpl editor_find_get_flags
  dec rep_diff+1   ; make that $ff

;
; get any find/replace flags
;
editor_find_get_flags:
  stz find_flags

editor_flags_loop:
  inx
  lda INBUF,X
  cmp #NL
  beq editor_list_fix_target_string
  jsr isspace
  bcs editor_flags_loop
  jsr MAKE_UPPER
  cmp #'Q'
  bne editor_flags1
  lda #FFLAG_QUIET
  tsb find_flags
  lda #EFLAG_NO_LINE_NUMBERS
  tsb editor_flags
  bra editor_flags_loop
editor_flags1:
  cmp #'G'
  bne editor_flags2
  lda #FFLAG_GLOBAL
  tsb find_flags
  bra editor_flags_loop
editor_flags2:
  cmp #'I'
  bne editor_flags3
  lda #FFLAG_IGNORE_CASE
  tsb find_flags
  bra editor_flags_loop
editor_flags3:
  lda #<bad_flag_message
  ldx #>bad_flag_message
  jsr print
  jmp main_prompt

bad_flag_message: asciiz "Unknown flag\n"

editor_list_fix_target_string:

  lda #FFLAG_IGNORE_CASE
  and find_flags
  beq editor_list_get_on_with_it

;
;  for case insensitive compare force target to be upper case
;
  ldx find_from
editor_list_fixup_loop:
  cpx find_to
  beq editor_list_get_on_with_it
  lda INBUF,X
  jsr MAKE_UPPER
  sta INBUF,X
  inx
  bra editor_list_fixup_loop


editor_list_not_find:
;
;  we should have no more tokens now
;
  jsr editor_check_no_more

editor_list_get_on_with_it:
;
;  make sure current_line is correct
;
  lda from_line
  sta current_line
  lda from_line+1
  sta current_line+1

;
;  exit immediately if no source from here on
;
  ldy #0
  lda (PCODE),Y
  bne editor_list_have_source
  jmp editor_list_done

editor_list_have_source:
  lda #EFLAG_SHOW_LINE
  tsb editor_flags
  bra editor_list_loop

editor_list_doneJ jmp editor_list_done
editor_list_no_line_numberJ jmp editor_list_no_line_number

editor_list_loop:
  lda serial_in_byte_received
  cmp  #KEY_ABORT
  beq  editor_list_aborted
  stz  found_this_line
  ldy #0
  lda (PCODE),Y
  beq editor_list_doneJ
  lda editor_flags
  and #EFLAG_SHOW_LINE
  beq  editor_list_no_line_numberJ
  lda editor_flags
  and #EFLAG_SAVE
  bne editor_list_no_line_numberJ
;
;  At this exact point we know we are at the start of a line
;  and we aren't doing a Save. Now, if we are doing a Find then
;  we need to know if this line (from here to the next newline)
;  matches our find string, and if not, to skip it

  lda #EFLAG_SUPPRESS_NEWLINE
  trb editor_flags              ; we can show newlines now

  lda editor_flags
  and #EFLAG_FIND
  bne editor_find_or_replacing

  jmp  editor_list_not_finding

;
;  Here when Ctrl+C pressed
;
editor_list_aborted:
  ldx #50
  ldy #0
  jsr delay   ; 50 ms delay to allow serial to settle
  jsr CROUT
  jmp editor_list_done

editor_find_or_replacing:

;
;  here if finding or replacing
;

  lda #0

;
;  we come back here for multiple replacements with a new starting pos
;
editor_replacement_loop:
  sta find_pos    ; searching line from beginning (or not, if doing a multiple replace)

editor_find_loop:
  ldx find_from   ; out target string position in INBUF
  ldy find_pos    ; where we are on the line
editor_find_inner_loop:
  cpx find_to     ; reached end of target, found the string!
  beq editor_find_found_it
  lda (PCODE),Y
  cmp #NL
  beq editor_find_no_match ; hit end of line without matching

;
;  force to upper-case if required
;
  pha
  lda #FFLAG_IGNORE_CASE
  and find_flags
  beq editor_find_case_sensitive
  pla
  jsr MAKE_UPPER
  bra editor_find_test_character
editor_find_case_sensitive:
  pla
editor_find_test_character:
  cmp INBUF,X     ; does source byte match target byte?
  bne editor_find_skip_this
  iny
  inx
  bra editor_find_inner_loop

editor_find_skip_this:
  inc find_pos
  bne editor_find_loop  ; if it wraps, give up

;
;  we are at the newline without a match, increment PCODE to match
;  where we are and go onto newline processing
;
editor_find_no_match:
   lda found_this_line
   bne editor_list_not_finding
   clc
   tya
   adc PCODE
   sta PCODE
   lda #0
   adc PCODE+1
   sta PCODE+1
   lda #EFLAG_SUPPRESS_NEWLINE
   tsb editor_flags
   bra editor_newline_processing

;
;  here when find succeeded
;
editor_find_found_it:
  inc find_count
  bne editor_find_found_it1
  inc find_count+1
editor_find_found_it1:
  lda #1
  sta found_this_line
  lda #EFLAG_REPLACE
  and editor_flags
  bne editor_do_the_replace

  lda #FFLAG_GLOBAL
  and find_flags
  beq editor_list_not_finding   ; one replace is enough

;
;  add the find length to Y (moving past what we just found)
;
;
  clc
  lda find_pos
  adc find_len
  jmp editor_replacement_loop

;
;  here when replacing
;
editor_do_the_replace:
  lda #FLAG_VALID_COMPILE   ; compile or assemble no longer valid
  ora #FLAG_VALID_ASSEMBLE
  trb system_flags

  lda rep_diff ; same length?
  beq editor_easy_replace
  jmp editor_do_replace

editor_easy_replace:
  ldy find_pos
  ldx rep_from
editor_easy_replace_loop:
  cpx rep_to
  beq editor_easy_replace_done
  lda INBUF,x
  sta (PCODE),y
  iny
  inx
  bne editor_easy_replace_loop  ; keep replacing

editor_easy_replace_done:
  lda #FFLAG_GLOBAL
  and find_flags
  beq editor_list_not_finding   ; one replace is enough

;
;  add the replacement length to Y (moving past what we just replaced)
;   the replacement length is rep_to - rep_from
;
  clc
  lda find_pos
  adc rep_to
  sec
  sbc rep_from
  jmp editor_replacement_loop

;
;
;  display the line number
;
editor_list_not_finding:
  lda #EFLAG_SHOW_LINE
  trb editor_flags      ; cancel flag until next newline
  lda #EFLAG_NO_LINE_NUMBERS
  and editor_flags
  bne editor_list_no_line_number
  jsr show_current_line_number

editor_list_no_line_number:
  ldy  #0
  lda  (PCODE),Y
  cmp  #NL
  bne  editor_list_not_newline

editor_newline_processing:
  lda  #EFLAG_SHOW_LINE
  tsb  editor_flags
  inc  current_line
  bne  editor_list_check_to_line
  inc  current_line+1

;
;  see if we hit the wanted "to" line
;
editor_list_check_to_line:
  lda  current_line+1
  cmp  to_line+1
  bcc  editor_list_check_to_line_ok
  bne  editor_list_done
  lda  current_line
  cmp  to_line
  bcc  editor_list_check_to_line_ok
  bne  editor_list_done

editor_list_check_to_line_ok:

  lda #EFLAG_SUPPRESS_NEWLINE
  and editor_flags
  bne editor_list_skip_newline
  lda #NL   ; put the newline back

;
;  just echo the character, and increment the memory address
;
editor_list_not_newline:
  tax
  lda #FFLAG_QUIET
  and find_flags
  bne editor_list_skip_newline
  txa
  jsr PRCHAR

editor_list_skip_newline:
  inc PCODE
  bne editor_list_loopJ
  inc PCODE+1

editor_list_loopJ:
  jmp editor_list_loop

editor_list_done:
  jsr CROUT

  lda find_count
  ora find_count+1
  beq  editor_list_completely_done
  lda find_count
  sta REG
  lda find_count+1
  sta REG+1
  stz REGB
  jsr display_in_decimal
  lda #<found_message
  ldx #>found_message
  jsr print

editor_list_completely_done:
  jmp main_prompt

;
;  This if for replacing when the lengths differ
;
editor_do_replace:
;
; add our current find_pos to PCODE to get where we are in memory
;
  clc
  lda  PCODE
  pha   ; save where we currently are
  adc  find_pos
  sta  mem_move_src
  lda  PCODE+1
  pha
  adc  #0
  sta  mem_move_src+1

;
;  add the source string length as we don't need to copy that
;
  clc
  lda mem_move_src
  adc find_len
  sta mem_move_src
  lda mem_move_src+1
  adc #0
  sta mem_move_src+1

;
;  the move destination is the source plus the replacement length
;   (ie. a gap of the difference in bytes)

  clc
  lda mem_move_src
  adc rep_diff
  sta mem_move_dest
  lda mem_move_src+1
  adc rep_diff+1
  sta mem_move_dest+1


  jsr find_source_end   ; goes into PCODE
;
;  the length to move will be (the end of the source) - (the start of the move)
;
  sec
  lda PCODE
  sbc mem_move_src
  sta mem_move_len
  lda PCODE+1
  sbc mem_move_src+1
  sta mem_move_len+1

  lda rep_diff+1
  bmi editor_do_replace_smaller
  jsr moveup      ; replacing with larger string
  bra editor_do_replace_copy_done

editor_do_replace_smaller:
  jsr movedown    ; replacing with smaller string

editor_do_replace_copy_done:
;
;  get the current source address back
;
  pla
  sta  PCODE+1
  pla
  sta  PCODE
  jmp  editor_easy_replace    ; now move in new string


load_message asciiz "Enter source, terminate with Esc\n"

;--------------------------------------------
;  INSERT / LOAD
;--------------------------------------------
editor_load:

  lda #EFLAG_LOAD
  sta editor_flags

editor_insert:
  jsr editor_get_number
  bcc editor_insert_at_start

  ;
  ; I really want to insert AFTER this line so I'll add 1 to whatever line number they gave
  ; To insert at the start use Insert without a line number
  ;
  clc
  lda token_value
  adc #1
  sta this_line
  lda token_value+1
  adc #0
  sta this_line+1

  jsr editor_check_no_more
  jsr find_line    ; find the "from" line address
  bra editor_insert_common

editor_insert_at_start:
  lda #<TEXT_START
  sta this_line_addr
  lda #>TEXT_START
  sta this_line_addr+1
  lda #1
  sta current_line    ; current line number (1)
  stz current_line+1

editor_insert_common:
  jsr find_source_end

;
;  move existing source to end of RAM
;
  sec
  lda PCODE
  sbc this_line_addr
  sta mem_move_len
  lda PCODE+1
  sbc this_line_addr+1
  sta mem_move_len+1

;
;  find where to move to
;
  sec
  lda #<HIGHEST_RAM
  sbc mem_move_len
  sta mem_move_dest
  sta insert_limit
  lda #>HIGHEST_RAM
  sbc mem_move_len+1
  sta mem_move_dest+1
  sta insert_limit+1

;
;  the source of the move
;
  lda this_line_addr
  sta mem_move_src
  lda this_line_addr+1
  sta mem_move_src+1

  jsr moveup

;
;  put input at from_line_addr
;

  lda this_line_addr
  sta PCODE
  lda this_line_addr+1
  sta PCODE+1

  lda  #<load_message
  ldx  #>load_message
  jsr  print

;
;  show the first line number
;

  lda #EFLAG_SHOW_LINE
  tsb editor_flags
  ldy #0    ; offset from PCODE for inserting (should always be zero)
  ldx #0    ; last character we inserted

editor_insert_loop:
  lda #EFLAG_LOAD         ; no line numbers on load
  and editor_flags
  bne editor_insert_no_line_number

  lda editor_flags
  and #EFLAG_SHOW_LINE
  beq  editor_insert_no_line_number
;
;  display the line number
;

  stz editor_flags      ; cancel flag until next newline
  jsr show_current_line_number

editor_insert_no_line_number:
  jsr  CHRIN
  cmp  #CR                          ; carriage-return?
  beq  editor_insert_no_line_number ; yes, ignore
  cmp  #END_EDITOR_INPUT            ; Esc?
  beq  editor_insert_done
  cmp  #BACKSPACE
  beq  editor_insert_backspace
  cmp  #NL
  bne  editor_insert_not_newline
  lda  #EFLAG_SHOW_LINE
  tsb  editor_flags
  lda  #NL            ; get the newline back
  inc  current_line
  bne  editor_insert_not_newline
  inc  current_line+1
editor_insert_not_newline:
  sta  (PCODE),y
  tax                ; for detecting final newline
  inc  PCODE
  bne  editor_insert_loop1
  inc  PCODE+1

editor_insert_loop1:
  lda  insert_limit+1
  cmp  PCODE+1
  bcs  editor_insert_loop   ; carry is SET if we didn't borrow
  bne  editor_too_full
  lda  insert_limit
  cmp  PCODE
  beq  editor_too_full
  bra  editor_insert_loop

;
;  backspace reduces our pointer by one, unless we are at the start
;
editor_insert_backspace:
  lda PCODE
  cmp this_line_addr
  bne editor_insert_backspace_not_at_start
  lda PCODE+1
  cmp this_line_addr+1
  beq editor_insert_loop    ; at start, ignore backspace

editor_insert_backspace_not_at_start:
  sec                       ; move source pointer back one
  lda PCODE
  sbc #1
  sta PCODE
  lda PCODE+1
  sbc #0
  sta PCODE+1
  lda (PCODE),y
  tax                      ; this is now the last character we inserted
;
;  however we don't want them backspacing over a previous line, so if we detect a newline
;  just go back up and put it back (and increment PCODE back to where it was)
;
  cmp #NL
  beq editor_insert_not_newline
  bra editor_insert_loop

editor_insert_done:
  txa
  beq editor_insert_done_with_newline   ; nothing got inserted at all
  cmp #NL
  beq editor_insert_done_with_newline  ; skip inserting a newline if we just got a newline
;
;  They finished inserting, but without a newline ... add one
;
  lda #NL
  sta (PCODE),y
  inc PCODE
  bne editor_insert_done_with_newline
  inc PCODE+1

;
;  they are done - copy the rest back to the end of where they just inserted
;
editor_insert_done_with_newline:

  stz system_flags     ; compile is not valid any more

  lda PCODE
  sta mem_move_dest
  lda PCODE+1
  sta mem_move_dest+1

  lda insert_limit
  sta mem_move_src
  lda insert_limit+1
  sta mem_move_src+1

  jsr movedown

  jsr CROUT   ; put a newline after the ESC character
  jmp main_prompt

editor_too_full:
  lda #<memory_full
  ldx #>memory_full
  jsr print
;
;  consume input so it isn't mistaken for a command
;
editor_too_full_loop:
  jsr CHRIN
  cmp #END_EDITOR_INPUT
  bne editor_too_full_loop
;
;  another message, they may miss the first one during a bulk load
;
  lda #<memory_filled_up
  ldx #>memory_filled_up
  jsr print
  jmp editor_insert_done

editor_no_lines  asciiz "No range supplied\n"
memory_full      asciiz "\n\nMemory full - press Esc\n"
memory_filled_up asciiz "\n\nNo room for source during load/input\n\n"

;
;  subroutine to check we got a range for certain actions
;
editor_argument_check:
  ldy #0
;
;  skip spaces to make sure we got a range
;
editor_argument_check_loop:
  lda (token_start),y
  beq editor_argument_check_failed
  cmp #NL
  beq editor_argument_check_failed
  jsr isspace
  bcc editor_argument_check_got_argument
  iny
  bne editor_argument_check_loop

;
;  only found spaces, not good enough
;
editor_argument_check_failed:
  lda #<editor_no_lines
  ldx #>editor_no_lines
  jsr print
  jmp  main_prompt

editor_argument_check_got_argument:
  rts


;--------------------------------------------
;  DELETE
;--------------------------------------------
editor_delete:
;
;  default to no lines
;
  stz from_line
  stz from_line+1
  stz to_line
  stz to_line+1

  stz find_count      ; delete count in this case
  stz find_count+1
  jsr editor_argument_check ; cannot get away with no arguments

  jsr editor_get_range
  jsr editor_check_no_more

  lda from_line
  sta this_line
  lda from_line+1
  sta this_line+1
  jsr find_line       ; find the "from" line address
  lda this_line_addr  ; that becomes the destination of the move
  sta mem_move_dest
  lda this_line_addr+1
  sta mem_move_dest+1
  stz mem_move_len
  stz mem_move_len+1
;
;  now we'll work our way up until we get to the to_line, however we'll
;  add one because we want the final address to actually be the start of
;  the NEXT line
;
  clc
  lda to_line
  adc #1
  sta this_line
  lda to_line+1
  adc #0
  sta this_line+1
  jsr find_line_loop    ; find the "to" line address
  lda this_line_addr  ; that becomes the destination of the move
  sta mem_move_src
  lda this_line_addr+1
  sta mem_move_src+1

;
;  count how many lines we delete by counting newlines from
;   mem_move_dest to mem_move_src
;

  lda mem_move_dest
  sta PCODE
  lda mem_move_dest+1
  sta PCODE+1
  ldy #0

editor_delete_line_count_loop:
  lda PCODE
  cmp mem_move_src
  bne editor_delete_line_count_not_done
  lda PCODE+1
  cmp mem_move_src+1
  beq editor_delete_line_count_done

editor_delete_line_count_not_done:
  lda (PCODE),y
  cmp #NL
  bne editor_delete_line_count_not_newline
  inc find_count
  bne editor_delete_line_count_not_newline
  inc find_count+1
editor_delete_line_count_not_newline:
  inc PCODE
  bne editor_delete_line_count_loop
  inc PCODE+1
  bra editor_delete_line_count_loop

;
;  now find where the source ends
;  PCODE will now be at the first line to not be deleted
;
editor_delete_line_count_done:

  jsr find_source_end_loop
  sec
  lda PCODE
  sbc this_line_addr
  sta mem_move_len
  lda PCODE+1
  sbc this_line_addr+1
  sta mem_move_len+1
  jsr movedown  ; that should do it

  lda find_count
  sta REG
  lda find_count+1
  sta REG+1
  stz REGB
  jsr display_in_decimal
  lda #<lines_deleted_message
  ldx #>lines_deleted_message
  jsr print
  jmp main_prompt

lines_deleted_message asciiz " deleted.\n"

;--------------------------------------------
;  show current line number with leading spaces
;--------------------------------------------
show_current_line_number:
  pha
  phx
  phy
;
;  get the line number into REG ready for printing
;
  lda current_line
  sta REG
  ldx current_line+1
  stx REG+1
  ldy #0        ; count of leading spaces
  sty REGB
  cpx #>1000    ; if the number is >= 1000 don't print any spaces
  bcc line_number_lt_1000
  bne line_number_output_spaces
  cmp #<1000
  bcs line_number_output_spaces
line_number_lt_1000:
  iny           ; output one space
  cpx  #0       ; if line number > 255 then one space is all we need
  bne  line_number_output_spaces
  cmp  #100     ; if line number >= 100 then one space is all we need
  bcs  line_number_output_spaces
  iny           ; output two spaces
  cmp  #10      ; if line number >= 10 then two spaces are all we need
  bcs  line_number_output_spaces
  iny           ; output three spaces
line_number_output_spaces:
  cpy #0            ; leading spaces in front of line number
  beq  show_current_line_number_done
  phy
  jsr  PUTSP      ; output the space
  ply
  dey             ; count them
  bne  line_number_output_spaces  ; more spaces to go
show_current_line_number_done:
  jsr  display_in_decimal    ; now the actual number
  jsr  PUTSP      ; and a space after it
  ply
  plx
  pla
  rts

;--------------------------------------------
;  find_line
;
;  finds address of this_line in memory - returns in this_line_addr
;--------------------------------------------
find_line:
  lda #1
  sta current_line    ; current line number (1)
  stz current_line+1
  lda #<TEXT_START   ; start at start of source
  sta this_line_addr
  lda #>TEXT_START
  sta this_line_addr+1
find_line_loop:
  ldy #0
  lda current_line
  cmp this_line
  bne find_line_next
  lda current_line+1
  cmp this_line+1
  bne find_line_next
  rts
find_line_next:
  lda (this_line_addr),Y
  beq find_line_eof
  cmp #NL
  bne find_line_same_line
  inc current_line       ; add 1 to our line number counter
  bne find_line_same_line
  inc current_line+1
find_line_same_line:
  inc this_line_addr     ; onto next address - which means that a newline
  bne find_line_loop     ; will actually end up being the start of the new
  inc this_line_addr+1   ; line, which is what we want
  bra find_line_loop
find_line_eof:           ; we are pointing to 0x00 - make that the address
  rts



;***********************************************
; Find text end
;***********************************************
find_source_end:
  lda  #<TEXT_START        ; start at start of source
  sta  PCODE
  lda  #>TEXT_START
  sta  PCODE+1
find_source_end_loop =  *
  ldy  #0
  lda  (PCODE),Y             ; keep going until 0x00 indicates end of source
  beq  find_source_end_wrap
  inc  PCODE
  bne  find_source_end_loop
  inc  PCODE+1
  bra  find_source_end_loop
;
;  include final 0x00
;
find_source_end_wrap =  *
  inc  PCODE
  bne  find_source_end_done
  inc  PCODE+1
find_source_end_done =  *
  rts

;
;  testing BRK handling
;
brk_resumed asciiz "\nResumed after BRK\n"

editor_test:
  lda #$12
  ldx #$23
  ldy #$42
  cmp #$12
  brk
  dfb $99   ; break number
  lda #<brk_resumed
  ldx #>brk_resumed
  jsr print
  rts

;
;  memory dump
;
editor_memory:
  jsr editor_argument_check  ; check they supplied some sort of range
  jsr editor_default_range
  lda #EFLAG_ALLOW_ZERO_FROM
  sta editor_flags
  jsr editor_get_range
  jsr editor_check_no_more
  stz current_line      ; count of bytes shown
  lda from_line
  sta this_line_addr
  lda from_line+1
  sta this_line_addr+1

editor_memory_loop:
  lda serial_in_byte_received
  cmp  #KEY_ABORT
  beq  editor_memory_aborted

  lda current_line
  bne editor_memory_loop1
;
;  show address
;
  lda #'$'
  jsr COUT
  lda from_line+1
  jsr PRBYTE
  lda from_line
  jsr PRBYTE
  lda #':'
  jsr COUT
  jsr PUTSP
editor_memory_loop1:
  ldy #0
  lda (from_line),Y
  jsr PRBYTE
  jsr PUTSP
  inc current_line
  inc from_line
  bne editor_memory_loop2
  inc from_line+1
editor_memory_loop2:
;
;  if we wrap around back to zero, stop
;
  lda from_line
  ora from_line+1
  beq editor_memory_done
  lda from_line+1
  cmp to_line+1
  bcc editor_memory_loop3   ; less than
  bne editor_memory_done    ; greater than
  lda from_line             ; equal, so try low-order byte
  cmp to_line
  bcc editor_memory_loop3   ; less than
  bne editor_memory_done    ; greater than
editor_memory_loop3:
  lda current_line
  cmp #16
  bcc editor_memory_loop
  jsr editor_memory_show_ascii
  stz current_line      ; count of bytes shown
  bra editor_memory_loop

editor_memory_done:
  jsr editor_memory_show_ascii
editor_memory_aborted:
  jsr CROUT
  jmp  main_prompt

editor_memory_show_ascii:
;
;  now the ASCII equivalents
;
  jsr PUTSP
  ldy #0
editor_memory_ascii:
  lda (this_line_addr),Y
  cmp #$20
  bcs editor_memory_not_control
  bra editor_memory_show_dot
editor_memory_not_control:
  cmp #$7F
  bcc editor_memory_not_high
editor_memory_show_dot
  lda #'.'
editor_memory_not_high:
  jsr COUT
  jsr PUTSP
  iny
  dec current_line
  bne editor_memory_ascii
;
;  adjust the ASCII printing address
;
  lda from_line
  sta this_line_addr
  lda from_line+1
  sta this_line_addr+1

  jsr CROUT
  rts




symbol_table_message asciiz "Addr Prev L  T Dsp  Ar Da Ln Name\n"

editor_symbol_table_list  = *
  lda #<symbol_table_message
  ldx #>symbol_table_message
  jsr print

  lda  ENDSYM+1    ; get the end of the symbol table
  sta  SYMITM+1    ; make it the current symbol
  lda  ENDSYM
  bra  ST_SYMTBL_LIST_CHECK

ST_SYMTBL_LIST_NEXT =  *
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
ST_SYMTBL_LIST_CHECK     =  *
  sta  SYMITM
  cmp  #<SYMBOL_TABLE_START
  bne  ST_SYMTBL_LIST_LINE
  lda  SYMITM+1
  cmp  #>SYMBOL_TABLE_START
  bne  ST_SYMTBL_LIST_LINE       ; more to go
  jmp  ST_SYMTBL_LIST_DONE       ; finished if symitm and symitm+1 are both zero

ST_SYMTBL_LIST_LINE = *
  lda  SYMITM+1
  jsr  PRBYTE
  lda  SYMITM
  jsr  PRBYTE
  lda  #' '
  jsr  PRCHAR
  ldy  #SYMPRV+1
  lda  (SYMITM),Y
  jsr  PRBYTE
  dey
  lda  (SYMITM),Y
  jsr  PRBYTE
  lda  #' '
  jsr  PRCHAR
  ldy  #SYMLVL
  lda  (SYMITM),Y  ; level
  jsr  PRBYTE
  lda  #' '
  jsr  PRCHAR
  ldy  #SYMTYP
  lda  (SYMITM),Y  ; type
  jsr  PRCHAR
  lda  #' '
  jsr  PRCHAR
  ldy  #SYMDSP
  iny
  lda  (SYMITM),Y  ; displacement
  jsr  PRBYTE
  dey
  lda  (SYMITM),Y  ; displacement
  jsr  PRBYTE
  lda  #' '
  jsr  PRCHAR
  ldy  #SYMARG
  lda  (SYMITM),Y  ; argument count
  jsr  PRBYTE
  lda  #' '
  jsr  PRCHAR
  ldy  #SYMDAT
  lda  (SYMITM),Y  ; data type
  jsr  PRBYTE
  lda  #' '
  jsr  PRCHAR
  ldy  #SYMLEN
  lda  (SYMITM),Y  ; length
  pha
  jsr  PRBYTE
  lda  #' '
  jsr  PRCHAR
  ldy  #SYMNAM
  pla
  tax
ST_SYMTBL_LIST_NAME = *
  lda  (SYMITM),Y  ; name
  phy
  jsr  PRCHAR
  ply
  iny
  dex
  bne ST_SYMTBL_LIST_NAME
  jsr CROUT
  jmp ST_SYMTBL_LIST_NEXT

ST_SYMTBL_LIST_DONE = *

  jmp main_prompt


recover_done asciiz "Done\n"
recover_not_needed asciiz "Not needed\n"

;
; Recover: if the first byte of the source is 0x00, change it to a space
;
editor_recover:
  jsr editor_check_no_more
  lda  #<TEXT_START
  sta  FROM
  lda  #>TEXT_START
  sta  FROM+1
  ldy  #0
  lda  (FROM),Y
  bne  editor_recover_not_needed
  lda  #' '
  sta  (FROM),Y
editor_done_message:
  lda #<recover_done
  ldx #>recover_done
  jsr print
  jmp main_prompt

editor_recover_not_needed:
  lda #<recover_not_needed
  ldx #>recover_not_needed
  jsr print
  jmp main_prompt

not_assembler_message asciiz "Cannot trace or debug assembler code\n"

editor_debug:
  jsr editor_check_no_more
  lda  #FLAG_VALID_ASSEMBLE
  and  system_flags
  beq editor_debug_ok

editor_not_assembler:
  lda #<not_assembler_message
  ldx #>not_assembler_message
  jsr print
  jmp main_prompt

editor_debug_ok:
  lda #1
  sta  DBGFLG
  sta  DCODE
  jmp  CHK_RUN

editor_trace:
  jsr editor_check_no_more
  lda  #FLAG_VALID_ASSEMBLE
  and  system_flags
  bne  editor_not_assembler
  lda  #$80
  sta  DBGFLG
  sta  DCODE
  jmp  CHK_RUN


editor_info  = *
  jsr editor_check_no_more
  jsr show_source_start
  jsr show_source_end
  jsr show_source_lines
  jsr crc_source
  jmp main_prompt

editor_compile:
  jsr editor_check_no_more
  lda  #0
  sta  SYNTAX
  jmp  COMPIL

editor_syntax:
  jsr editor_check_no_more
  lda #1
  sta  SYNTAX
  jmp  COMPIL

editor_run:
  jsr editor_check_no_more
  lda  #0
  sta  DBGFLG
  sta  DCODE
  jmp  CHK_RUN

  ;
show_symbol_table_end:
  lda  #<symbol_table_ended_message    ; Symbol table ended at
  ldx  #>symbol_table_ended_message
  jsr  print
  lda  ENDSYM+1
  jsr  PRBYTE
  lda  ENDSYM
  jsr  PRBYTECR
  rts


source_ended_message asciiz "Source ends   at $"
show_source_end:
  lda #<source_ended_message
  ldx #>source_ended_message
  jsr print
  jsr find_source_end
  lda PCODE+1
  jsr PRBYTE
  lda PCODE
  jsr PRBYTECR
  rts

source_starts_message asciiz "Source starts at $"    ; source starts at
show_source_start:
  lda #<source_starts_message
  ldx #>source_starts_message
  jsr print
  lda #>TEXT_START
  jsr PRBYTE
  lda #<TEXT_START
  jsr PRBYTECR
  rts

;--------------------------------------------
;  show_source_lines - show count of number of source lines
;--------------------------------------------
source_line_count asciiz "Source lines:  "    ; count of lines
show_source_lines:
  stz VALUE      ; line count in this case
  stz VALUE+1
  stz VALUE+2

  lda #<TEXT_START
  sta PCODE
  lda #>TEXT_START
  sta PCODE+1
  ldy #0

editor_line_count_loop:
  lda (PCODE),y
  beq editor_line_count_done    ; 0x00 means end of source
  cmp #NL                       ; newline?
  bne editor_line_count_not_newline
  inc VALUE
  bne editor_line_count_not_newline
  inc VALUE+1
;
;  increment pointer
;
editor_line_count_not_newline:
  inc PCODE
  bne editor_line_count_loop
  inc PCODE+1
  bra editor_line_count_loop

editor_line_count_done:
  lda #<source_line_count
  ldx #>source_line_count
  jsr print
  jsr display_in_decimal
  jsr CROUT
  rts

editor_assemble:
  jsr editor_check_no_more
  jmp ASSEMBLE

no_break_message asciiz "No BRK previously encountered\n"

;--------------------------------------------
;  editor_resume - resume after a breakpoint
;--------------------------------------------
editor_resume:
  jsr CHK_VAL     ; check we had a valid compile/assemble
  lda system_flags
  and #FLAG_BRK_REACHED    ; BRK reached yet?
  bne editor_resume_ok
  lda #<no_break_message
  ldx #>no_break_message
  jsr print
  jmp main_prompt
editor_resume_ok:
  lda #FLAG_BRK_REACHED    ; no BRK reached yet
  trb system_flags
;
;  put the stack back to what it was and push the
;   processor flags and breakpoint address
;
  clc
  lda call_s
  adc #5      ; undo effect of pushing X and Y, and the 3 pushes the processor did
  tax
  txs         ; switch back to what the stack pointer was
  lda brk_address+1   ; put stuff back ready for rti
  pha
  lda brk_address
  pha
  lda call_p
  pha         ; processor flags
  lda call_a  ; get registers back
  ldy call_y
  ldx call_x
  rti


editor_poke_message asciiz "Need an address and at least one value\n"

;--------------------------------------------
;  editor_poke - put stuff into memory
;--------------------------------------------
editor_poke:
;
;  first check how many numbers are on the input line
;
  stz find_count
  lda token_start
  sta from_line
  lda token_start+1
  sta from_line+1

editor_poke_loop:
  jsr get_token
  beq editor_poke_got_all_numbers
  cmp #TOKEN_NUMBER
  beq editor_poke_got_number
  cmp #TOKEN_STRING           ; a one-byte string is OK, eg. poke $1000 'a'
  bne editor_poke_bad_token
  lda find_count
  beq editor_poke_bad_token   ; however not for the address
  lda token_length
  cmp #1
  beq editor_poke_got_number
;
;  here when we got neither a number or a single-byte string
;
editor_poke_bad_token:
  jmp editor_bad_number
editor_poke_got_number:
  inc find_count
  lda find_count
  cmp #1
  bne editor_poke_not_first
;
;  here for first number - must be in range 0x0000 to 0xffff
;
  lda token_value+2
  beq editor_poke_loop
  jmp editor_line_number_too_big
editor_poke_not_first:
;
;  here for subsequent numbers - must be in range 0x00 to 0xff
;
  lda token_value+1
  ora token_value+2
  beq editor_poke_loop
  jmp editor_line_number_too_big

;
;  must have at least two numbers
;
editor_poke_got_all_numbers:
  lda find_count
  cmp #2
  bcs editor_poke_got_enough
  lda #<editor_poke_message
  ldx #>editor_poke_message
  jsr print
  jmp main_prompt

editor_poke_got_enough:
;
;  reset for get_token to do its stuff again
;
  lda from_line
  sta token_start
  lda from_line+1
  sta token_start+1
;
;  get address again - we know it is valid since we checked above
;
  jsr get_token
;
;  where to put stuff
;
  lda token_value
  sta from_line
  lda token_value+1
  sta from_line+1

editor_poking_loop:
  jsr get_token
  bne editor_poke_another
  jmp editor_done_message

editor_poke_another:
  ldy #0
  lda token_value
  sta (from_line),y
;
;  check change was successful - if not, probably trying to change ROM
;
  cmp (from_line),y
  bne editor_poke_failed
  inc from_line
  bne editor_poking_loop
  inc from_line+1
  bra editor_poking_loop

editor_poke_failed_message asciiz "Failed to change address $"
editor_poke_to_message     asciiz " to $"

editor_poke_failed:
  lda #<editor_poke_failed_message
  ldx #>editor_poke_failed_message
  jsr print
  lda from_line+1
  jsr PRBYTE
  lda from_line
  jsr PRBYTE
  lda #<editor_poke_to_message
  ldx #>editor_poke_to_message
  jsr print
  lda token_value
  jsr PRBYTE
  jsr CROUT
  jmp main_prompt

;--------------------------------------------
;  editor_jsr - jump to subroutine
;--------------------------------------------
editor_jsr:
  jsr get_token
  cmp #TOKEN_NUMBER
  beq editor_jsr_got_number
  jmp editor_bad_number
;
;  check in range 0x0000 to 0xFFFF
;
editor_jsr_got_number:
  lda token_value+2
  beq editor_jsr_number_in_range
  jmp editor_line_number_too_big

editor_jsr_number_in_range:
  jsr editor_check_no_more
  lda  call_p      ; status register
  pha
  lda  call_a      ; A register
  ldx  call_x      ; X register
  ldy  call_y      ; Y register
  plp              ; get status register back
  jsr  editor_jsr_got_number_execute  ; call the subroutine
  php              ; save the status register
  sta  call_a      ; store the A/X/Y register
  stx  call_x
  sty  call_y
  pla              ; get status register back
  sta  call_p      ; save it
  jmp main_prompt

editor_jsr_got_number_execute  jmp  (token_value)


;--------------------------------------------
;  editor_jmp - jump to address
;--------------------------------------------
editor_jmp:
  jsr get_token
  cmp #TOKEN_NUMBER
  beq editor_jmp_got_number
  jmp editor_bad_number
;
;  check in range 0x0000 to 0xFFFF
;
editor_jmp_got_number:
  lda token_value+2
  beq editor_jmp_number_in_range
  jmp editor_line_number_too_big

editor_jmp_number_in_range:
  jsr editor_check_no_more
  lda  call_p      ; status register
  pha
  lda  call_a      ; A register
  ldx  call_x      ; X register
  ldy  call_y      ; Y register
  plp              ; get status register back
  jmp  (token_value)


;16-bit number comparison...
;
;         lda #>x               ; MSB of 1st number
;         cmp #>y               ; MSB of 2nd number
;         bcc islower           ; X < Y
;
;         bne ishigher          ; X > Y
;
;         lda #<x               ; LSB of 1st number
;         cmp #<y               ; LSB of 2nd number
;         bcc islower           ; X < Y
;
;         beq issame            ; X = Y
;
;         bne ishigher          ; X > Y
;
;
