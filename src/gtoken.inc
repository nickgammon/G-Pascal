;
;  get_token
;
;  Gets a lexical token from the specified address.
;
;  - Start parsing at token_start (this should be the first character of the token)
;  - The first found character (after spaces, comments, newlines) is in token_address
;      - this is mainly used for knowing what the identifier is so we can put it in the symbol table
;      - strings are put into INBUF after processing, so token_address is not as useful in that case
;  - The length is in token_length (in the case of strings, this is the processed length)
;  - The type is in token_type - zero would mean end of file
;
;  The types are one of the token codes below
;
;  Tokens which start with a letter and are possibly followed by letters, numbers, and
;    the underscore character become TOKEN_IDENTIFIER.
;
;  Tokens starting with a number (TOKEN_NUMBER) are parsed for the value which is placed into token_value.
;
;  Tokens starting with a single or double quote are parsed for the internal string (TOKEN_STRING)
;  which is placed in INBUF. The internal string has internal double-quotes turned
;  into single quotes, and sequences like \n or \x42 turned into their appropriate characters.
;
;  String tokens which are up to 3 bytes long also have the 3 bytes placed into token_value.
;
;  Hex and binary tokens are parsed for their value which is placed into token_value.
;
;  Various 2-character tokens are given their own code (eg. <> == != := << >> and so on).
;
;  Spaces and comments are skipped, however Pascal directives inside comments are processed
;
;  Newlines are skipped in Pascal but returned as a token in assembler (ASSEMBLING == true)
;  If newlines are skipped then token_line is called to handle listing the line (the line
;   we are about to process, not the previous line)
;
; Rewritten in February 2022 to be easier to read and have well-defined work variables (zero-page variables)

; multi-byte token codes (single byte tokens are themselves)

TOKEN_ASSIGN     = 'A'   ; :=
TOKEN_IDENTIFIER = 'I'   ; alpha-numeric identifiers
TOKEN_NUMBER     = 'N'   ; numbers
TOKEN_STRING     = '"'   ; string literal
TOKEN_LEQ        = $80   ; <=
TOKEN_GEQ        = $81   ; >=
TOKEN_NEQ        = 'U'   ; <>

;
;  the ones below are used in the assembler
;
TOKEN_EQUALITY    = 'E'     ; ==
TOKEN_SHIFT_LEFT  = 'L'     ; <<
TOKEN_SHIFT_RIGHT = 'R'     ; >>
TOKEN_UNARY_MINUS = 'M'     ; -
TOKEN_LOW_BYTE    = 'V'     ; eg. lda #<foo
TOKEN_HIGH_BYTE   = 'W'     ; eg. lda #>foo
TOKEN_LOGICAL_AND = 'X'     ; &&
TOKEN_LOGICAL_OR  = 'Y'     ; ||
TOKEN_INEQUALITY  = 'Z'     ; !=


get_token:
  lda token_type      ; remember previous token for checking if + or - stand on their own
  sta token_work
get_token_loop:
  ldy #0              ; offset into the token address
  sty token_sign      ; Y is zero at this point
  sty token_value     ; no value yet
  sty token_value+1
  sty token_value+2

gc_skip_spaces:
  lda (token_start),y
  beq gc_done         ; 0x00 means end, so return it WITHOUT incrementing the pointer
  cmp #NL             ; newline need special handling
  beq gc_newline
  jsr isspace
  bcc gc_not_space
  inc token_start     ; increment pointer because spaces can be any length
  bne gc_skip_spaces
  inc token_start+1
  bra gc_skip_spaces

;
;  got a newline - if assembling, return that so we know where lines end
;  otherwise just call token_line to display the next line and press on looking
;  for tokens.
;
gc_newline:
  lda #FLAG_ASSEMBLING
  and system_flags
  bne gc_newline_assembling    ; return the newline as a token
  iny              ; length in Y (ie. one)
  inc token_start
  bne gc_newline1
  inc token_start+1
gc_newline1:
  lda token_start
  sta token_line_start
  lda token_start+1
  sta token_line_start+1
  jsr token_line        ; list the next line
  bra get_token_loop    ; look for another token

gc_newline_assembling:
  lda #NL       ; get the newline back
  jmp gc_single_byte_token

;
;  wrap up getting a token:
;
;   - save the type
;   - save the current address as token_address
;   - add the length to token_start ready for next time
;
gc_single_byte_already_known:
  lda token_type        ; get back saved token type
gc_single_byte_token:
  ldy #1
gc_done:
  sty token_length      ; Y has our length
  sta token_type        ; A has our type
  clc
  lda token_start
  sta token_address     ; save the starting address of the token
  adc token_length      ; now add its length
  sta token_start       ; that gives the ending address

  lda token_start+1     ; repeat for high-order byte
  sta token_address+1
  adc #0
  sta token_start+1

  lda token_type        ; get token type back into A
  rts

;
;  not a space (or newline) so this must be the actual start of it
;

gc_not_space:

;
;  first look for classes of characters (ie. alpha, numeric)
;
  jsr isalpha
  bcc gc_not_alpha

;
;  find end of identifier
;
gc_alpha_loop:
  iny
  bne gc_alpha_ok   ; if it becomes zero now, we have 256+ long identifier
  ldx #12       ; ERROR: Illegal Identifier (too long)
  jmp ERROR

gc_alpha_ok:
  lda #FLAG_ONLY_ALPHA
  bit system_flags
  bne gc_alpha_alpha_only
  lda (token_start),y
  jsr isalnum
  bcs gc_alpha_loop
  cmp #'_'
  beq gc_alpha_loop

;
; end of identifier
;
gc_alpha_done:
  lda #TOKEN_IDENTIFIER
  bra gc_done


gc_alpha_alpha_only:
  lda (token_start),y
  jsr isalpha
  bcs gc_alpha_loop
  bra gc_alpha_done

;
; now see if it is a digit
;
gc_not_alpha:
  jsr isdigit
  bcc gc_not_digit

  jsr atoi
  lda  #TOKEN_NUMBER
  bra  gc_done

;
;  not alpha and not digit - so let's consider:
;   * prefixes: $ for hex and % for binary
;   *  + or - in front of numbers or on their own
;   * single character tokens, eg. =, (, ) and so on
;   * double-character tokens, eg. ==, >= << and so on
;   * strings, eg. "foo"
;
;  we'll do a jump on the first character
;
gc_not_digit:
  sta token_type            ; remember it in case of possible two-byte sequences
  ldx #<gtoken_table
  ldy #>gtoken_table
  jsr TKNJMP
  bra gc_single_byte_already_known  ; it is what it is

;
;  here when get_token discovers a newline - print the next line if wanted
;

token_line:
;
;  count lines
;
  inc current_line
  bne token_line1
  inc current_line+1
token_line1:
;
;  compiling or assembling?
;
  lda #FLAG_COMPILING
  ora #FLAG_ASSEMBLING
  and system_flags
  beq token_line_done ; no - no listing
;
;  listing wanted?
;
  lda #FLAG_LIST_SOURCE
  and system_flags
  bne token_line_listing ; yes

;
;  no listing - show an asterisk every 15 lines
;
  lda current_line
  and #$0F
  bne token_line_done
  lda #'*'
  jsr COUT
  bra token_line_done

token_line_listing:
  lda DCODE
  pha
  lda #1
  sta DCODE
  jsr DISPAD
  pla
  sta DCODE
  jsr show_current_line_number

  lda token_line_start
  sta token_work
  lda token_line_start+1
  sta token_work+1

token_line_loop:
  ldy #0
  lda (token_work),Y
  beq token_line_done
  pha
  jsr COUT
  inc token_work
  bne token_line_loop1
  inc token_work+1
token_line_loop1:
  pla
  cmp #NL
  bne token_line_loop

token_line_done:
  rts


;
;  atoi
;
; get a decimal number - returns carry flag set if bad number and running
;   (if compiling and a bad number gives an error)
;
;  token_start : the start address of the number
;  Y : offset into token_start
;  token_sign : non-zero if negative
;
;  returns: token_value  and carry clear (if no error)
;           carry set if error (for use at runtime)
;           Y at offset of first non-numeric character

atoi:
  sec
  sbc  #'0'   ;  ; subtract out ASCII stuff, save first digit
  sta  token_value
  stz  token_value+1
  stz  token_value+2
atoi_loop    =  *
  iny
  lda  (token_start),y
  jsr  isdigit
  bcs  atoi_more      ; more digits
;
; end of number - make negative if we got a sign earlier
;
  lda  token_sign
  beq  atoi_positive    ; not signed
;
;  make negative
;
  jsr negate_token_value
;
;  done! clear carry and return
;
atoi_positive:
  clc                   ; indicate no error for numeric conversion
  rts
;
;  Multiply previous value by 10, add in new digit
;
atoi_more    =  *
  sec
  sbc  #'0'
  sta  token_digit
  asl  token_value         ; multiply token_value by 2
  rol  token_value+1
  rol  token_value+2
  bmi  atoi_error

  lda  token_value         ; save token_value*2 in token_work
  sta  token_work
  lda  token_value+1
  sta  token_work+1
  lda  token_value+2
  sta  token_work+2

  asl  token_value         ; now token_value is multiplied by 4
  rol  token_value+1
  rol  token_value+2
  bmi  atoi_error

  asl  token_value         ; now token_value is multiplied by 8
  rol  token_value+1
  rol  token_value+2
  bmi  atoi_error

  lda  token_work          ; now add back in token_value*2 (giving token_value*10)
  adc  token_value
  sta  token_value
  lda  token_work+1
  adc  token_value+1
  sta  token_value+1
  lda  token_work+2
  adc  token_value+2
  sta  token_value+2
  bmi  atoi_error      ; if negative, the number was too big
  clc
  lda  token_value     ; now take the value and add in our new digit
  adc  token_digit
  sta  token_value
  lda #0
  adc  token_value+1   ; increment next byte if necessary
  sta  token_value+1
  lda #0
  adc  token_value+2
  sta  token_value+2
  bmi  atoi_error
  jmp  atoi_loop     ; onwards for next digit

atoi_error    =  *
  lda  RUNNING
  bpl  atoi_not_running
  sec             ; otherwise set carry and return
  rts
atoi_not_running  =  *
  ldx  #30       ; ERROR: Number out of Range
  jmp  ERROR

;
;  for negative numbers, subtract token_value from zero
;
negate_token_value:
  sec
  lda  #0
  sbc  token_value
  sta  token_value
  lda  #0
  sbc  token_value+1
  sta  token_value+1
  lda  #0
  sbc  token_value+2
  sta  token_value+2
  rts


gtoken_table:
  tknjmpItem '(',gc_lh_paren
  tknjmpItem '{',gc_lh_brace
  tknjmpItem '"',gc_quote
  tknjmpItem SINGLE_QUOTE,gc_quote
  tknjmpItem '$',gc_dollar
  tknjmpItem '%',gc_percent
  tknjmpItem ':',gc_colon
  tknjmpItem '<',gc_less_than
  tknjmpItem '>',gc_greater_than
  tknjmpItem '-',gc_minus
  tknjmpItem '+',gc_plus
  tknjmpItem '&',gc_ampersand
  tknjmpItem '|',gc_bar
  tknjmpItem '=',gc_equals
  tknjmpItem '!',gc_bang
  tknjmpItem ';',gc_semicolon
  dfb        0   ; end of table

gtoken_directive_table:
  tknjmpItem 'S',gc_directive_symbols
  tknjmpItem 'L',gc_directive_list
  tknjmpItem 'P',gc_directive_pcodes
  tknjmpItem 'N',gc_directive_nolist
  dfb        0

; single byte token, in A
gc_single_byte_tokenJ jmp gc_single_byte_token
; single byte token in token_type
gc_single_byte_already_knownJ jmp gc_single_byte_already_known

gc_lh_brace:
  ldy #0
  bra gc_pascal_comment
gc_lh_paren:
  lda #FLAG_ASSEMBLING
  and system_flags
  bne gc_single_byte_already_knownJ
  ldy #1
  lda (token_start),Y
  cmp #'*'      ; was it: (* ?
  bne gc_single_byte_already_knownJ  ; nope

;
;  Y will be 0 or 1 depending on the sort of starting comment
;  first look for % which indicates a compiler directive
;
gc_pascal_comment:
  iny
  lda (token_start),Y
  cmp #'%'   ; directive?
  bne gc_pascal_comment_find_end
  iny
  lda (token_start),Y ; get the directive: S, L, P or N
  pha   ; save the directive for now
  tya
;
;  make token_start point to where we are up to, so we can skip the comment, even if it is long
;
  sec               ; I actually want to be one past where the directive letter is
  adc token_start
  sta token_start
  lda #0
  adc token_start+1
  sta token_start+1
  pla   ; get the directive back
  ldx #<gtoken_directive_table
  ldy #>gtoken_directive_table
  jsr TKNJMP  ; do the directive handler
;
;  fall through if not found and just find the end of the comment
;

gc_pascal_comment_find_end:
  ldy #0    ; we normalised token_start to be where we want to start
  lda (token_start),Y
  bne gc_comment_not_eof
  ldx  #7           ; NO } FOUND
  jmp  ERROR
gc_comment_not_eof:
  cmp #NL
  bne gc_comment_not_newline
  jsr token_line    ; handle the newline (we can't be assembling or we wouldn't be here)
  bra gc_comment_keep_looking
gc_comment_not_newline:
  cmp #'}'
  beq gc_comment_found_end
  cmp #'*'
  bne gc_comment_keep_looking
  iny
  lda (token_start),Y
  cmp #')'
  beq gc_comment_found_end

;
;  skip this byte and keep looking for the end of comment
;
gc_comment_keep_looking:
  inc token_start
  bne gc_pascal_comment_find_end
  inc token_start+1
  bra gc_pascal_comment_find_end

;
;  found end of comment, so move token_start to this character
;   (the one past the end of the comment) and try again for a token
;
gc_comment_found_end:
  iny   ; get past end of comment
  tya
  clc
  adc token_start
  sta token_start
  lda #0
  adc token_start+1
  sta token_start+1
  jmp get_token_loop

;
;  {%S $nnnn}
;  symbol table relocation
;
gc_directive_symbols:
   jsr  get_token_loop     ; re-call get_token to find the address
   cmp  #TOKEN_NUMBER ; number?
   beq  gc_directive_symbols_ok      ; yes
   ldx  #2
   jmp  ERROR      ; 'Constant expected'

gc_directive_symbols_ok:
  ;
  ;  check we haven't generated any code
  ;
  lda PCODE
  cmp ACT_PCDA
  bne gc_directive_symbols_too_late
  lda PCODE+1
  cmp ACT_PCDA+1
  bne gc_directive_symbols_too_late

  lda  token_value
  sta  ENDSYM
  sta  STARTSYM
  sta  OPND
  lda  token_value+1
  sta  ENDSYM+1        ; store symbol table address
  sta  STARTSYM+1
  sta  OPND+1
  lda  #PCODE_STACK    ; change runtime stack
  jsr  GENJMP
;
;  put the library functions back
;
  jsr  add_pascal_library_functions

  jmp  gc_pascal_comment_find_end     ; go back and find the end of the comment
;

gc_directive_symbols_too_late:
  ldx #51        ; code already generated
  JMP  ERROR


;
;  {%L} - list source
;
gc_directive_list:
  lda #FLAG_LIST_SOURCE
  tsb system_flags
  jmp gc_pascal_comment_find_end

;
;  {%P} - list source and P-codes
;
gc_directive_pcodes:
  lda #1
  sta DCODE
  bra gc_directive_list   ; also list source
;
;  {%N} - do not list source or P-codes
;
gc_directive_nolist:
  lda #FLAG_LIST_SOURCE
  trb system_flags    ; clear both of the above flags
  stz DCODE
  jmp gc_pascal_comment_find_end

;
;  semicolons start comments in the assembler
;
gc_semicolon:
  lda #FLAG_ASSEMBLING
  and system_flags
  bne gc_assembler_comment_find_end
  jmp gc_single_byte_already_known

gc_assembler_comment_find_end:
  ldy #0
  lda (token_start),Y
  bne gc_assembler_comment_not_eof
  lda #NL
  sta token_type
  rts     ; exit without incrementing token_start

gc_assembler_comment_not_eof:
  cmp #NL
  bne gc_assembler_comment_keep_looking
  jmp gc_single_byte_token
;
;  skip this byte and keep looking for the newline at the end of comment
;
gc_assembler_comment_keep_looking:
  inc token_start
  bne gc_assembler_comment_find_end
  inc token_start+1
  bra gc_assembler_comment_find_end


gc_backslash_tokens:
         DFB  'A',$07  ; bell ($07)
         DFB  'B',$08  ; backspace ($08)
         DFB  'E',$1B  ; escape  (0x1B)
         DFB  'F',$0C  ; formfeed ($0C)
         DFB  'N',$0A  ; newline  (0x0A)
         DFB  'R',$0D  ; carriage return (0x0D)
         DFB  'T',$09  ; horizontal tab (0x09)
         DFB  'V',$0B  ; vertical tab (0x0B)
         DFB  $5C,$5C  ; backslash
         DFB  '"','"'  ; double quote
         DFB  SINGLE_QUOTE,SINGLE_QUOTE  ; single quote
gc_backslash_tokens_end = *
gc_backslash_tokens_length = gc_backslash_tokens_end - gc_backslash_tokens

;
;  quoted string
;
;  resolve double quotes and backslash sequences and store it in INBUF
;
;
gc_quote:
  sta token_work    ; remember which sort
  ldy #0            ; back to the start
  stz token_length  ; zero-length string so far
  lda token_start
  sta token_address   ; save the starting address of the token (not that it matters too much)
  lda token_start+1   ; repeat for high-order byte
  sta token_address+1

gc_quote_loop:
  iny                 ; onto next character
  lda (token_start),Y ; get the next character in the string
  beq gc_quote_bad
  cmp  #NL
  bne  gc_quote_loop_not_end_of_line

gc_quote_bad:
  ldx  #8         ; MISSING QUOTE: Incorrect string
  jmp  ERROR

gc_quote_loop_not_end_of_line:
  cmp  token_work  ; same as start quote?
  bne  gc_quote_not_finished       ; no
  iny
  lda (token_start),Y ; see if another quote follows
  cmp  token_work
  beq  gc_quote_not_backslash      ; embedded quote (ie. two quotes in a row)? Store a single quote symbol
  ;
  ;  here at end of quoted string
  ;
  sty  token_work  ; remember how far through our input we are
  ldy  #3          ; OK, we have the final quote, so is the string <= 3 characters long?
  cpy  token_length
  bcc  gc_quote_long_string   ; BLT - no
  ldy  token_length           ; yes - load its actual length into Y (ie. 0, 1, 2, 3)
gc_quote_long_string:
  dey      ; zero-relative
  bmi  gc_quote_copied_value   ; keep going into the length done
  lda  INBUF,Y   ; store the (up to) 3 bytes into VALUE as the token value
  sta  token_value,Y
  bne  gc_quote_long_string   ; keep going until we hit the 0x00 at the end

gc_quote_copied_value:
  lda  token_length    ; now see how long it is?
  bne  gc_quote_done     ; don't allow zero length strings
  ldx  #14        ; BAD STRING: literal string of zero length
  jmp  ERROR

gc_quote_done:
  clc
  lda token_start       ; bump up our token_start address to past the quoted string
  adc token_work        ; now add its (actual) length, not the length afer mucking around
  sta token_start       ; that gives the ending address
  lda token_start+1   ; repeat for high-order byte
  adc #0
  sta token_start+1
  lda  #TOKEN_STRING   ; it's a string token
  sta  token_type
  rts


gc_quote_not_finished:
  cmp #$5C    ; backslash?
  bne  gc_quote_not_backslash
;
;  backslash - see what follows
;
  iny
  lda (token_start),Y ; the character after the backslash
  beq gc_quote_bad    ; nothing? bad
  jsr MAKE_UPPER
  cmp #'X'
  beq gc_quote_hex_character
  ldx #0
gc_quote_backslash_loop:
  cmp gc_backslash_tokens,X
  beq gc_quote_found_backslash_character   ; good match
  inx
  inx
  cpx #gc_backslash_tokens_length
  bcc gc_quote_backslash_loop
gc_quote_bad_string:
  ldx #8
  jmp ERROR   ; ERROR: Incorrect string

gc_quote_hex_character_done:
  lda token_value
  bra gc_quote_not_backslash

gc_quote_found_backslash_character:
  inx
  lda gc_backslash_tokens,X  ; get its replacement

gc_quote_not_backslash:
  ldx  token_length
  sta  INBUF,x           ; store it in INBUF
  inc  token_length
  beq  gc_quote_too_long
  jmp  gc_quote_loop     ; keep adding to string


gc_quote_too_long:
  ;            string over 255 characters long - error!
  ldx   #29  ; ERROR: string literal too big
  jmp  ERROR

;
;  "fix" a hex digit by making it upper-case, then subtracting 7 if it is A-F
;   then subtracting '0' so we get a number in the range 0x00 to 0x0F in A
;
gc_fix_hex:
  jsr  MAKE_UPPER
  cmp  #'A'
  bcc  gc_fix_hex_not_a_to_f
  sbc  #7
gc_fix_hex_not_a_to_f:
  sec
  sbc  #'0'
  rts

gc_quote_hex_character:
  iny
  beq  gc_quote_too_long
  lda  (token_start),Y ; the character after the backslash
  jsr  isxdigit
  bcc  gc_quote_bad_string  ; what? should be hex
  jsr  gc_fix_hex
  sta  token_value  ; first nibble
  iny
  beq  gc_quote_too_long
  lda  (token_start),Y ; a second hex character?
  beq  gc_quote_bad_string
  jsr  isxdigit
  bcc  gc_quote_hex_character_one_only
  jsr  gc_fix_hex
  asl  token_value    ; shift existing value 4 bits left
  asl  token_value
  asl  token_value
  asl  token_value
  ora  token_value
  sta  token_value
  bra  gc_quote_hex_character_done

gc_quote_hex_character_one_only:
  dey         ; undo add since we didn't find another hex character
  bra gc_quote_hex_character_done

;
;  $nnnnnn where nnnnnn is one or more hex digits
;    if no hex digits, then it is the token '$'
;
gc_dollar:
  ldy  #1    ; start with the character after the dollar
  lda  (token_start),Y ; followed by a hex character?
  jsr  isxdigit
  bcs  gc_hex_literal
  jmp  gc_single_byte_already_known

gc_hex_literal:
  jsr gc_fix_hex
  sta token_value       ; store the first digit
  stz token_value+1
  stz token_value+2
gc_hex_loop:
  iny
  lda (token_start),Y ; followed by another hex character?
  jsr isxdigit
  bcc gc_hex_done
  jsr gc_fix_hex
;
;  shift the existing value left 4 bits to make room for the new nibble
;
  asl  token_value        ; token_value shifted left 1 bit
  rol  token_value+1
  rol  token_value+2
  bcs  gc_hex_too_big
  asl  token_value        ; token_value shifted left 2 bits
  rol  token_value+1
  rol  token_value+2
  bcs  gc_hex_too_big
  asl  token_value        ; token_value shifted left 3 bits
  rol  token_value+1
  rol  token_value+2
  bcs  gc_hex_too_big
  asl  token_value        ; token_value shifted left 4 bits
  rol  token_value+1
  rol  token_value+2
  bcs  gc_hex_too_big
  adc  token_value        ; now add in this last digit (carry will be clear)
  sta  token_value        ; store the new result
  bcc  gc_hex_loop
  inc  token_value+1
  bcc  gc_hex_loop
  inc  token_value+1
  bcs  gc_hex_too_big
  bra  gc_hex_loop

gc_hex_done:
  lda #TOKEN_NUMBER
  jmp gc_done

gc_hex_too_big:
  ldx  #30       ; ERROR: Number out of Range
  jmp  ERROR

;
;  %nnnnnn where nnnnnn is one or more binary digits
;    if no binary digits, then it is the token '%'
;

gc_percent:
  ldy  #1    ; start with the character after the %
  lda  (token_start),Y ; followed by a binary character?
  jsr  isbinary
  bcs  gc_binary_literal
  jmp  gc_single_byte_already_known

gc_binary_literal:
  sbc #'0'              ; we know carry is set
  sta token_value       ; store the first digit
  stz token_value+1
  stz token_value+2
gc_binary_loop:
  iny
  lda (token_start),Y ; followed by another binary character?
  jsr isbinary
  bcc gc_hex_done
  sbc #'0'              ; we know carry is set

  ror A   ; get the new bit into Carry
  rol token_value
  rol token_value+1
  rol token_value+2
  bcc gc_binary_loop
  jmp gc_hex_too_big   ; number too large

;
;  token : might be :=
;
gc_colon:
  ldy  #1               ; get the next character
  lda  (token_start),Y  ; followed by = ?
  cmp  #'='
  beq  gc_assign
  jmp  gc_single_byte_already_known

gc_assign:
  lda #TOKEN_ASSIGN
  iny
  jmp gc_done

;
;  token: < might be <=, <>, <<
;
gc_less_than:
  ldy  #1               ; get the next character
  lda  (token_start),Y  ; followed by = ?
  iny                   ; make it a 2-byte token
  cmp  #'='
  beq  gc_leq
  cmp  #'>'
  beq  gc_neq
  cmp  #'<'
  beq  gc_shift_left
  jmp  gc_single_byte_already_known

gc_leq:
  lda #TOKEN_LEQ
  jmp gc_done

gc_neq:
  lda #TOKEN_NEQ
  jmp gc_done

gc_shift_left:
  lda #TOKEN_SHIFT_LEFT
  jmp gc_done

;
;  token: > might be <>=, >>
;
gc_greater_than:
  ldy  #1               ; get the next character
  lda  (token_start),Y  ; followed by = ?
  iny                   ; make it a 2-byte token
  cmp  #'='
  beq  gc_geq
  cmp  #'>'
  beq  gc_shift_right

gc_single_byte_already_knownJ2:
  jmp  gc_single_byte_already_known

gc_geq:
  lda #TOKEN_GEQ
  jmp gc_done

gc_shift_right:
  lda #TOKEN_SHIFT_RIGHT
  jmp gc_done


;
;  minus and plus sign
;

gc_minus:
  sta token_sign
gc_plus:

;
;  new stuff - treat a sign as a token on its own if the previous token was one of:
;    a) a number
;    b) an identifier
;    c) a RH bracket
;
;  So, for example: 2+3 is three tokens, but 2+-5 is also three, as the -5 becomes one token
;
  lda  token_work            ; saved previous token on entry to get_token
  cmp  #TOKEN_NUMBER
  beq  gc_single_byte_already_knownJ2
  cmp  #TOKEN_IDENTIFIER
  beq  gc_single_byte_already_knownJ2
  cmp  #')'
  beq  gc_single_byte_already_knownJ2
;
;  if not, see if this is a signed number (ie. it is directly followed by a digit)
;
  ldy  #1               ; get the next character
  lda  (token_start),Y  ; followed by a digit?
  jsr  isdigit
  bcc  gc_single_byte_already_knownJ2
  jsr atoi
  lda  #TOKEN_NUMBER
  jmp  gc_done


;
;  token: & might be &&
;
gc_ampersand:
  ldy  #1               ; get the next character
  lda  (token_start),Y  ; followed by & ?
  cmp  #'&'
  beq  gc_logical_and
  jmp  gc_single_byte_already_known

gc_logical_and:
  lda #TOKEN_LOGICAL_AND
  iny
  jmp gc_done

;
;  token: | might be ||
;
gc_bar:
  ldy  #1               ; get the next character
  lda  (token_start),Y  ; followed by | ?
  cmp  #'|'
  beq  gc_logical_or
  jmp  gc_single_byte_already_known

gc_logical_or:
  lda #TOKEN_LOGICAL_OR
  iny
  jmp gc_done

;
;  token: = might be ==
;
gc_equals:
  ldy  #1               ; get the next character
  lda  (token_start),Y  ; followed by = ?
  cmp  #'='
  beq  gc_equality
  jmp  gc_single_byte_already_known

gc_equality:
  lda #TOKEN_EQUALITY
  iny
  jmp gc_done

;
;  token: ! might be !=
;
gc_bang:
  ldy  #1               ; get the next character
  lda  (token_start),Y  ; followed by = ?
  cmp  #'='
  beq  gc_inequality
  jmp  gc_single_byte_already_known

gc_inequality:
  lda #TOKEN_INEQUALITY
  iny
  jmp gc_done



; Pascal tokens

TOKEN_CONST     = $82
TOKEN_VAR       = $83
TOKEN_ARRAY     = $84
TOKEN_OF        = $85
TOKEN_PROCEDURE = $86
TOKEN_FUNCTION  = $87
TOKEN_BEGIN     = $88
TOKEN_END       = $89
TOKEN_OR        = $8A
TOKEN_DIV       = $8B
TOKEN_MOD       = $8C
TOKEN_AND       = $8D
TOKEN_SHL       = $8E
TOKEN_SHR       = $8F
TOKEN_NOT       = $90
TOKEN_MEM       = $91
TOKEN_IF        = $92
TOKEN_THEN      = $93
TOKEN_ELSE      = $94
TOKEN_CASE      = $95
TOKEN_WHILE     = $96
TOKEN_DO        = $97
TOKEN_REPEAT    = $98
TOKEN_UNTIL     = $99
TOKEN_FOR       = $9A
TOKEN_TO        = $9B
TOKEN_DOWNTO    = $9C
TOKEN_WRITE     = $9D
TOKEN_READ      = $9E
TOKEN_CALL      = $9F
TOKEN_CHAR      = $A1
TOKEN_MEMC      = $A2
TOKEN_XOR       = $A4
TOKEN_ADDRESS   = $A9
TOKEN_CHR       = $AB
TOKEN_HEX       = $AC
; $B0 to $DE used by message tokens
TOKEN_LCDWRITE  = $F1
TOKEN_INTEGER   = $FE
TOKEN_WRITELN   = $FF



; RESERVED WORD TABLE
;
;***********************************************
         .macro makeToken  ; MACRO DEFINITION FOR TOKENS: Length, token number, token name
         DFB  \1
         DFB  \2
         text  \3
         .endmacro           ; END OF MACRO

;
; Reserved words are identified internally by their code (eg. $86 for PROCEDURE)
;
;  They are also stored "tokenised" and expanded out when you list the source
;
; Other tokens are:
;  * Punctuation (eg. "<", ">", "=", ".", "(", ")", "$", "+", "-"
;  * Identifiers: "I"        (eg. MY_VARIABLE)
;  * Constant numbers: "N"   (eg. -1234 and $1234)
;  * Literal strings:  '"' (quote symbol)  (eg. "Nick")
;  * Double-character punctuation:
;       <= : $80
;       >= : $81
;       Unequal (<>) : "U"
;       Assignment (:=) : "A"
;
; Comments are skipped: (* some comment *)
;
;
; However comments can have compiler directives embedded, eg. (*%A $4000) - put P-codes at $4000
;    (*%L*) - list during compile
;    (*%N*) - stop listing during compile (cancels %L and %P)
;    (*%P*) - show P-codes during compile
;    (*%A  $nnnn *) - put P-codes at address $nnnn - must be at START of program
;
;

;              Length   Code             Name
;                  --   --------------- ---------------
RSVWRD   =  *
   makeToken   5, TOKEN_CONST     ,"CONST"
   makeToken   3, TOKEN_VAR       ,"VAR"
   makeToken   5, TOKEN_ARRAY     ,"ARRAY"
   makeToken   2, TOKEN_OF        ,"OF"
   makeToken   9, TOKEN_PROCEDURE ,"PROCEDURE"
   makeToken   8, TOKEN_FUNCTION  ,"FUNCTION"
   makeToken   5, TOKEN_BEGIN     ,"BEGIN"
   makeToken   3, TOKEN_END       ,"END"
   makeToken   2, TOKEN_OR        ,"OR"
   makeToken   3, TOKEN_DIV       ,"DIV"
   makeToken   3, TOKEN_MOD       ,"MOD"
   makeToken   3, TOKEN_AND       ,"AND"
   makeToken   3, TOKEN_SHL       ,"SHL"
   makeToken   3, TOKEN_SHR       ,"SHR"
   makeToken   3, TOKEN_NOT       ,"NOT"
   makeToken   3, TOKEN_MEM       ,"MEM"
   makeToken   2, TOKEN_IF        ,"IF"
   makeToken   4, TOKEN_THEN      ,"THEN"
   makeToken   4, TOKEN_ELSE      ,"ELSE"
   makeToken   4, TOKEN_CASE      ,"CASE"
   makeToken   5, TOKEN_WHILE     ,"WHILE"
   makeToken   2, TOKEN_DO        ,"DO"
   makeToken   6, TOKEN_REPEAT    ,"REPEAT"
   makeToken   5, TOKEN_UNTIL     ,"UNTIL"
   makeToken   3, TOKEN_FOR       ,"FOR"
   makeToken   2, TOKEN_TO        ,"TO"
   makeToken   6, TOKEN_DOWNTO    ,"DOWNTO"
   makeToken   5, TOKEN_WRITE     ,"WRITE"
   makeToken   4, TOKEN_READ      ,"READ"
   makeToken   4, TOKEN_CALL      ,"CALL"
   makeToken   4, TOKEN_CHAR      ,"CHAR"
   makeToken   4, TOKEN_MEMC      ,"MEMC"
   makeToken   3, TOKEN_XOR       ,"XOR"
   makeToken   7, TOKEN_ADDRESS   ,"ADDRESS"
   makeToken   3, TOKEN_CHR       ,"CHR"
   makeToken   3, TOKEN_HEX       ,"HEX"
   makeToken   7, TOKEN_INTEGER   ,"INTEGER"
   makeToken   7, TOKEN_WRITELN   ,"WRITELN"
   makeToken   8, TOKEN_LCDWRITE  ,"LCDWRITE"


RSVEND   DFB  0,0        ; END OF TABLE
;

;
;  Pascal get_token
;
;  Used in the Pascal section. Does a get_token, and then if an identifier is found tries to look
;  it up.
;
pas_get_token:
  jsr get_token
  beq pas_get_token_eof   ; branch to make sure Z flag stays set
  cmp #TOKEN_IDENTIFIER
  beq pas_gc_identifier
pas_get_token_eof:
  rts
;
;  here if get_token got an identifier
;
pas_gc_identifier:
  lda  #<RSVWRD
  sta  token_work
  lda  #>RSVWRD
  sta  token_work+1
pas_gc_loop:
  ldy  #0
  lda  (token_work),Y
  bne  pas_gc_more        ; MORE TO GO
  lda  #TOKEN_IDENTIFIER
  rts
;
; SEARCH for RESERVED WORD
;
pas_gc_more:
  lda  (token_work),Y     ; length of word in table
  cmp  token_length       ; same?
  bne  pas_gc_no_match    ; no - can't be it then
  tay                     ; length
  lda  token_address      ; put token start address into SRCE
  sta  SRCE
  lda  token_address+1
  sta  SRCE+1
  lda  token_work         ; put current token_work+2 into DEST
  clc
  adc  #2
  sta  DEST
  lda  token_work+1
  adc  #0
  sta  DEST+1
  jsr  str_ic_compare
  bne  pas_gc_no_match      ; NOT FOUND
  ldy  #1
  lda  (token_work),Y
  sta  token_type
  rts
pas_gc_no_match    =  *
  ldy  #0
  lda  (token_work),Y     ; token_work points to reserved word entry
  clc
  adc  #2         ; add 2 to bypass length and code
  adc  token_work
  sta  token_work         ; put it back
  bcc  pas_gc_loop
  inc  token_work+1       ; handle overflow
  bra  pas_gc_loop     ; try for another



;***********************************************
; GET A TOKEN - CHECK THAT IT
; IS THE SAME AS IN "A", IF NOT
; CALL ERROR "X"
;***********************************************
GETCHK:
  phx   ; save error number
  pha   ; save wanted token type
  jsr pas_get_token
  pla   ; get it back
  cmp token_type
  beq CHKOK
  plx   ; get error number back
CHKNOK   =  *
  jmp ERROR
CHKOK    =  *
  plx   ; get error number back
  rts

;***********************************************
; CHECK TOKEN AGREES WITH "A",
; IF NOT, GIVE ERROR "X"
;***********************************************
CHKTKN:
  cmp  token_type
  bne  CHKNOK
  rts

;***********************************************
;
; GET NEXT TOKEN - MUST BE IDENTIFIER
; THEN LOOK IT UP IN SYMBOL TABLE
;***********************************************

GET_LOOK:
  lda  #TOKEN_IDENTIFIER
  ldx  #4      ; ERROR: Identifier expected
  jsr  GETCHK
  jmp  LOOKUP
