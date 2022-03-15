;***********************************************
; SYMBOL TABLE STUFF: Offsets from the start of a particular symbol
; Stored as a linked list, where you search backwards so the more local one is found first
;***********************************************

SYMPRV   =  0         ; address of previous symbol (for working backwards through symbols)
SYMLVL   =  2         ; nested level (eg. entering a procedure/function increments the level,
                      ;                   and leaving it decrements the level)
                      ; - used to detect duplicate symbols at the same level
SYMLIB   =  2         ; (re-used above for assembler symbols: 0 = user-declared, 1 = library)
SYMTYP   =  3         ; Symbol type: C = constant, V = variable, Y = function return value
                      ;              P = procedure, F = function, A = array
                      ;              p = library procedure, f = library function
                      ;  See defines below.
SYMDSP   =  4         ; if the symbol is a constant, what the value is (3 bytes)
                      ; if the symbol is a variable, array or function return then this is the offset (2 bytes)
SYMARG   =  6         ; if the symbol is a procedure/function, count of arguments (can be zero)
SYMSUB   =  6         ; MAX SUBSCRIPT+1 - for arrays, so this is the actual array length
SYMDAT   =  8         ; data type: 0 = integer, 1 = char
SYMLEN   =  9         ; length of the name, der
SYMNAM   =  10        ; name of symbol for SYMLEN characters, stored in upper-case

;
; Symbol types
;

SYMBOL_CONSTANT          = 'C'
SYMBOL_VARIABLE          = 'V'
SYMBOL_FUNCTION_RETURN   = 'Y'
SYMBOL_PROCEDURE         = 'P'
SYMBOL_FUNCTION          = 'F'
SYMBOL_LIBRARY_PROCEDURE = 'Q'  ; library procedures and functions are implemented in assembler
SYMBOL_LIBRARY_FUNCTION  = 'R'
SYMBOL_ARRAY             = 'A'



;
;***********************************************
;SEARCH SYMBOL TABLE - returns Z if not found (BEQ <not found>)
;***********************************************
SEARCH   =  *
         lda  ENDSYM+1    ; get the end of the symbol table
         sta  SYMITM+1    ; make it the current symbol
         lda  ENDSYM
         bra  SEA1

SEA_NEXT =  *
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
SEA1     =  *
         sta  SYMITM
         cmp  STARTSYM
         bne  SEA2
         lda  SYMITM+1
         cmp  STARTSYM+1
         bne  SEA2       ; more to go
         rts             ; finished if we are back at the start of the symbol table

SEA2     =  *
         ldy  #SYMLEN
         lda  (SYMITM),Y  ; get this item's length
         cmp  token_length      ; compare to the token legnth we got
         bne  SEA_NEXT    ; wrong length - try another
         lda  SYMITM      ; back to this symbol's start address
         clc
         adc  #SYMNAM     ; add the offset of the name to it
         sta  DEST
         lda  SYMITM+1
         adc  #0
         sta  DEST+1      ; the memory location of the symbol name is in DEST
         lda  token_address      ; now put the token's address into SRCE
         sta  SRCE
         lda  token_address+1
         sta  SRCE+1
         ldy  token_length
         jsr  COMSTL      ; compare SRCE to DEST
         bne  SEA_NEXT    ; not that one - try another
         jsr  GET_DAT     ; get the symbol's data type into DATTYP
         ldy  #SYMLVL     ; and level - why isn't this a function, huh?
         lda  (SYMITM),Y
         tax              ; level into X
         ldy  #SYMTYP
         lda  (SYMITM),Y  ; get type of symbol
         sta  BSAVE       ; symbol type -> BSAVE
         cmp  #SYMBOL_CONSTANT       ; constant?
         bne  SEA4        ; nah
;
;  symbol is a constant
;
         ldy  #SYMDSP     ; if a constant, this is the value
         lda  (SYMITM),Y
         sta  token_value
         iny
         lda  (SYMITM),Y
         sta  token_value+1
         iny
         lda  (SYMITM),Y
         sta  token_value+2     ; token_value (3 bytes) now has the value in it
         jmp  SEA3
;
;  symbol is not a constant
;
SEA4     =  *                           ; not constant
         cmp  #SYMBOL_VARIABLE          ; variable?
         beq  SEA5                      ; yes
         cmp  #SYMBOL_FUNCTION_RETURN   ; argument?
         bne  SEA3                      ; no - must be SYMBOL_ARRAY, SYMBOL_PROCEDURE or SYMBOL_FUNCTION
;
;  here for variables and function return values
;
SEA5     =  *
         jsr  GET_OFF    ; get the variable's offset
SEA3     =  *
         lda  BSAVE      ; A now has the symbol type in it

; Types are: SYMBOL_CONSTANT, SYMBOL_VARIABLE, SYMBOL_FUNCTION_RETURN,
;            SYMBOL_PROCEDURE, SYMBOL_FUNCTION, SYMBOL_ARRAY

         rts             ; SHOULD SET 'NEQ' FLAG


SYM_FULL ldx  #37     ; ERROR: Symbol table full
         jmp  ERROR

;***********************************************
; ADD SYMBOL TO SYMBOL TABLE - going downwards until we hit the P-codes
;  A = the symbol type
;***********************************************
ADDSYM   =  *
         pha            ; save the symbol type

         ldx  ENDSYM    ; get the end of the symbol table
         stx  SYMITM    ; make it the current symbol
         ldx  ENDSYM+1
         stx  SYMITM+1
;
;  since we are going downwards, we need to subtract the fixed size of a symbol table item,
;  plus the symbol length, from SYMITM
;
         sec            ; first subtract the token length
         lda  SYMITM
         sbc  token_length
         sta  SYMITM
         sta  DEST      ; put this spot into DEST for copying the name
         lda  SYMITM+1
         sbc  #0
         sta  SYMITM+1
         sta  DEST+1
         sec            ; and now the fixed size
         lda  SYMITM
         sbc  #SYMNAM
         sta  SYMITM
         lda  SYMITM+1
         sbc  #0
         sta  SYMITM+1
;
;  check we haven't hit the P-codes
;
         lda  #FLAG_ASSEMBLING
         and  system_flags
         bne  ADDSYM_NOT_FULL
         lda  PCODE+1
         cmp  SYMITM+1
         bcc  ADDSYM_NOT_FULL   ; less than - ok
         bra  SYM_FULL          ; greater than or equal (high-order byte) - uh oh
;
;  ah, for assembling it is a pain to know whether the symbol table has overflowed, so I'm going
;  to ignore that as a problem for now. Checking against PCODE will not work if the assembler output
;  has been relocated to above the symbol table, and the only other thing we can compare against is
;  the end of the source which we don't necessarily know.
;
;
;  now put the previous start in as the link for the previous one for this item
;
ADDSYM_NOT_FULL:
         ldy  #SYMPRV
         lda  ENDSYM
         sta  (SYMITM),Y
         lda  ENDSYM+1
         iny
         sta  (SYMITM),Y
;
;  SYMITM now becomes the start of the symbol table
;
         lda  SYMITM
         sta  ENDSYM
         lda  SYMITM+1
         sta  ENDSYM+1
;
;  OK, we should be done linking it in.
;  SYMITM now points to this (as yet unfilled-in) item and the first two bytes now point to
;   where the symbols previously ended
;
         pla                ; get symbol type back
         ldy  #SYMTYP
         sta  (SYMITM),Y    ; store the type which was in A
         ldy  #SYMLVL
         pha                ; save symbol type again
         lda  LEVEL
         sta  (SYMITM),Y    ; store the level
         ldy  #SYMLEN
         lda  token_length
         sta  (SYMITM),Y    ; store the token length
         tay
         dey                ; make zero-relative
ADD1     =  *               ; copy the name across, in upper case
         lda  (token_address),Y
         jsr  MAKE_UPPER
         sta  (DEST),Y
         dey
         bpl  ADD1      ; keep copying

;
;  we have added name to the table, now do the rest
;
         pla                     ; get symbol type back again
         tax                     ; symbol type
         cmp  #SYMBOL_CONSTANT   ; CONSTANT??
         bne  ADD4
;
;  constant
;
         ldy  #SYMDSP            ; put this token's value into SYMDSP offset from SYMITM (for 3 bytes)
         lda  token_value
         sta  (SYMITM),Y
         iny
         lda  token_value+1
         sta  (SYMITM),Y
         iny
         lda  token_value+2
         sta  (SYMITM),Y
         jmp  ADD9
;
;  not a constant
;
ADD4     =  *
         ldy  #SYMDAT
         lda  #1            ; char type (?)
         sta  (SYMITM),Y
         txa                ; get symbol type back
         cmp  #SYMBOL_VARIABLE
         bne  ADD9
;
;  variable type
;
         ldy  #SYMDSP+1
         lda  FRAME+1       ; save frame
         sta  (SYMITM),Y
         dey
         lda  FRAME
         sta  (SYMITM),Y
         inc  FRAME         ; add 1 to frame number
         bne  ADD9
         inc  FRAME+1
ADD9     =  *
         rts
;

;
LOOKUP   =  *
         JSR  SEARCH
         BNE  LOOK1
         LDX  #11     ; ERROR: Undeclared Identifier
         jmp  ERROR
LOOK1    RTS
;
CHKDUP   JSR  SEARCH
         BEQ  DUP9
         TXA
         CMP  LEVEL
         BNE  DUP9
         LDX  #38   ; ERROR: Duplicate Identifier
         jmp  ERROR
DUP9     RTS

;
;  Get the offset of this current symbol into OFFSET
;
GET_OFF  =  *
         PHA
         LDY  #SYMDSP
         LDA  (SYMITM),Y
         STA  OFFSET
         INY
         LDA  (SYMITM),Y
         STA  OFFSET+1
         LDY  #SYMTYP
         LDA  (SYMITM),Y
         CMP  #SYMBOL_VARIABLE
         BEQ  GETO_1
         CMP  #SYMBOL_ARRAY
         BEQ  GETO_1
         CMP  #SYMBOL_FUNCTION_RETURN
         BNE  GETO_2
;
;  here if the symbol is a variable, array or function return value
;   subtract the offset from -3
;
GETO_1   =  *
         SEC
         LDA  #$FD
         SBC  OFFSET
         STA  OFFSET
         LDA  #$FF
         SBC  OFFSET+1
         STA  OFFSET+1
GETO_2   =  *
         PLA
         RTS

;
GET_LEV  =  *
         LDA  LEVEL
         LDY  #SYMLVL
         SEC
         SBC  (SYMITM),Y
         STA  DISPL
         RTS
;
GET_DAT  =  *
         LDY  #SYMDAT
         LDA  (SYMITM),Y
         STA  DATTYP
         RTS
;

