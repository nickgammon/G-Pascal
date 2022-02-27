
;***********************************************
; COMPILER
;***********************************************


;
;
;  Compiler directives:
;
;    {%L} - list during compile (source code and current P-code address)
;    {%P} - show generated P-codes during compile - only works during compile (not syntax check)
;    {%N} - stop listing during compile (cancels %L and %P)
;    {%S <address>} - relocate symbol table to <address> - must be done before defining symbols
;                     also generates an opcode to relocate the runtime stack
;
;   The % must directly follow the start of the comment.
;
;
;  Comments:  (* some comment *) or { some comment }
;             "(*" / "{", and "*)" / "}" are treated interchangeably so (* some comment } would work
;  Comments cannot be nested.
;
;  Tips:
;
;   Procedures and functions must be declared at the START of a block, not in the middle.
;   The order is important. CONST first, then VAR, then procedure/function declarations, then BEGIN

;    eg.
;
;    const limit = 10;   { <-- this is the start of a block: CONST before VAR }
;    var k: integer;     { we declare variables and constants here - BEFORE "begin" }
;    line : array [100] of char;  { array }
;    function foo;       { and functions and procedures - this is the start of another block}
;    begin
;      foo := 42         { assign to function return value }
;    end;                { end of the function block - note semicolon }
;    begin               { end of declarations - now we have the block statements }
;     for k := 1 to limit do
;       writeln ("square of ", k, " is ", k * k);
;     write (foo)        { call function }
;    end.
;
;    ARGUMENTS: If the procedure or function does not take arguments OMIT THE BRACKETS.
;               Likewise, omit the brackets if you call the procedure or function (unlike C)
;
;    Semicolons SEPARATE statements, they don't terminate them.
;
; Warnings: * Functions which do not assign to the function return value give undefined results.
;           * Multiplication discards high-order bits if result is too large.
;           * Divide remainder is always positive (ie. MOD always gives a positive result, regardless of operand signs)
;           * Function arguments are always integers and always return an integer.
;           * One to three-byte strings can be used as integer constants
;               eg. a := "abc"; write (a);   --> Result: 6513249 (ie. 0x636261 or "abc")
;               The first character is the low-order byte in the number, explaining the above results.
;           * Strings can start with single or double quotes, whatever starts the string must terminate it.
;           * String literals can have the same character that started the string embedded, to do that put it twice.
;             eg. 'Nick''s cat'
;           * Array access it not range-checked.
;           * Arrays start at zero. The array size declared is the maximum index (unlike C) so foo: array [10] of integer
;               would declare an array of 11 elements, numbered from foo [0] to foo [10]
;           * Maximum arguments to a procedure/function is 85 (255 / 3) due to internal coding
;


;
COMPIL   LDX  #NEW_STK
  txs    ; set stack back to 0xFF

  jsr  INIT
  lda  #FLAG_COMPILING
  tsb  system_flags
  jsr  add_pascal_library_functions
  jsr  pas_get_token
  jsr  BLOCK
  lda  #'.'
  ldx  #9        ; ERROR: . expected
  jsr  CHKTKN
  lda  #0
  ldx  #19       ; ERROR: Incorrect Symbol
  jsr  GETCHK
  jsr  CROUT
  lda  #<pcodes_ended_message  ; P-codes ended at
  ldx  #>pcodes_ended_message
  jsr  print
  lda  PCODE+1
  sta  END_PCD+1
  jsr  PRBYTE
  lda  PCODE
  sta  END_PCD
  jsr  PRBYTECR
  jsr  show_symbol_table_end
  jsr  show_source_end

  lda  #<compile_finished_message  ; <C>ompile finished: No Errors
  ldx  #>compile_finished_message
  jsr  print
  ldx  SYNTAX
  bne  END_CMP
  lda  #FLAG_VALID_COMPILE
  tsb  system_flags
END_CMP  =  *
  jmp  main_prompt
;

;
CHKLHP   =  *
         LDA  #'('
         LDX  #31     ; ERROR: ( expected
         JMP  GETCHK
;
CHKRHP   =  *
         LDA  #')'
         LDX  #22     ; ERROR: ) expected
         JSR  CHKTKN
         JMP  pas_get_token
;
GETSUB   =  *
         JSR  CHKLHB
         JSR  EXPRES
         JMP  CHKRHB
;
CHKLHB   =  *
         LDA  #'['
         LDX  #33     ; ERROR: [ expected
         JSR  GETCHK
         JMP  pas_get_token
;
CHKRHB   =  *
         LDA  #']'
         LDX  #34     ; ERROR: ] expected
         JSR  CHKTKN
         JMP  pas_get_token
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



; CONSTANT DEC
;
CONDEC   =  *
         LDA  #TOKEN_IDENTIFIER
         LDX  #4        ; ERROR: Identifier expected
         JSR  CHKTKN
         JSR  TKNWRK
         LDA  token_length
         PHA
         LDA  #'='
         LDX  #3        ; ERROR: = expected
         JSR  GETCHK
         JSR  pas_get_token
         JSR  CONST
         JSR  WRKTKN
         PLA
         STA  token_length
         JSR  CHKDUP
         LDA  #SYMBOL_CONSTANT
         JSR  ADDSYM
         JMP  pas_get_token
;
;
;--- SYMITM --> WORK
;
SYMWRK   =  *
         PHA
         LDA  SYMITM
         STA  WORK
         LDA  SYMITM+1
         STA  WORK+1
         PLA
         RTS
;
;--- WORK --> SYMITM
;
WRKSYM   =  *
         PHA
         LDA  WORK
         STA  SYMITM
         LDA  WORK+1
         STA  SYMITM+1
         PLA
         RTS
;
; PUSH PCODE ONTO STACK
;
PSHPCODE =  *
         STA  BSAVE
         PLA
         TAX
         PLA
         TAY
         LDA  PCODE+1
         PHA
         LDA  PCODE
         PHA
         TYA
         PHA
         TXA
         PHA
         LDA  BSAVE
         RTS
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
GETEXPR  =  *
         JSR  pas_get_token
         JMP  EXPRES
;
;
PCD_WRKD =  *
         PHA
         LDA  PCODE
         STA  WORKD
         LDA  PCODE+1
         STA  WORKD+1
         PLA
         RTS
;
WRK_OPND =  *
         PHA
         LDA  WORK
         STA  OPND
         LDA  WORK+1
         STA  OPND+1
         PLA
         RTS
;
WRKD_WRK =  *
         PHA
         LDA  WORKD
         STA  WORK
         LDA  WORKD+1
         STA  WORK+1
         PLA
         RTS
;
WRK_WRKD =  *
         PHA
         LDA  WORK
         STA  WORKD
         LDA  WORK+1
         STA  WORKD+1
         PLA
         RTS
;
GET_COMM =  *
         LDA  #','
         LDX  #32     ; ERROR: , expected
         JMP  CHKTKN
;
GET_ITEM =  *
         JSR  GET_COMM   ; check for comma
         JMP  GETEXPR
;
;  Load (push) a constant (ie. a literal number)
;
;  Numbers in the range 0 to 0x7F are turned into a "low literal" which is a P-code
;  with the 0x80 bit set. These then become a single-byte P-code which simply pushes
;  that number onto the stack, thus saving having to have a 4-byte P-code being PCODE_LIT
;  followed by a 3-byte number, 2 bytes of which would be zero.
;
VAL_MOVE =  *
         PHA
         CLC
         LDA  token_value
         STA  DISPL   ; why?
         BPL  VAL_1   ; is it 0x00 to 0x7F?
         SEC          ; no, set carry bit
VAL_1    =  *
         LDA  token_value+1
         BEQ  VAL_2   ; is second byte 0x00?
         SEC          ; no, set carry bit
VAL_2    =  *
         STA  OFFSET
         LDA  token_value+2
         STA  OFFSET+1
         BEQ  VAL_3   ; is third byte 0x00?
         SEC          ; no, set carry bit
VAL_3    =  *
         BCC  VAL_5   ; did we ever set the carry bit?
         LDA  #PCODE_LIT  ; yes, so just generate a "load literal"
         JSR  GENADR
         PLA
         RTS
VAL_5    =  *
         LDA  token_value   ; get the low-order byte back
         ORA  #$80    ; set the 0x80 bit as a flag
         JSR  GENNOP  ; output one P-code
         PLA
         RTS
;
;
CHK_STAK =  *
         TSX
         TXA
         CMP  #MAX_STK
         BCC  STK_FULL
         RTS
STK_FULL =  *
STK_ERR  LDX  #27     ; ERROR: Stack full
         jmp  ERROR      ; FULL
;
;
; CONST get the value into token_value
;
CONST    =  *
         LDA  token_type
         CMP  #TOKEN_NUMBER
         BEQ  CONST9
         CMP  #TOKEN_IDENTIFIER
         BEQ  CONST1
         CMP  #TOKEN_STRING
         BNE  CONST3
         LDX  token_length
         CPX  #4
         BCC  CONST9
         JMP  FACERR1    ; STRING TOO BIG
CONST1   JSR  SEARCH
         BNE  CONST2
CONST3   =  *
         LDX  #2        ; ERROR: Constant expected
         jmp  ERROR
CONST2   CMP  #SYMBOL_CONSTANT
         BNE  CONST3
CONST9   RTS
;
; VARIABLE DEC
;
VARDEC   LDA  #TOKEN_IDENTIFIER
         LDX  #4      ; ERROR: Identifier expected
         JSR  CHKTKN
         JSR  CHKDUP
         LDA  #SYMBOL_VARIABLE
         JSR  ADDSYM
         JMP  pas_get_token
;
; SIMPLE EXPRESSION
;
SIMEXP   =  *
         LDA  token_type
         CMP  #'+'
         BEQ  SIM1
         CMP  #'-'
         BNE  SIM2
SIM1     PHA
         JSR  pas_get_token
         JSR  TERM
         PLA
         CMP  #'-'
         BNE  SIM3
         LDA  #PCODE_NEG      ; NEG       Negate (sp)
         JSR  GENNOP     ; NEGATE
SIM3     LDA  token_type
         CMP  #'+'
         BEQ  SIM4
         CMP  #'-'
         BEQ  SIM4
         CMP  #TOKEN_OR       ; OR
         BEQ  SIM4
         CMP  #TOKEN_XOR      ; XOR
         BEQ  SIM4
         RTS
SIM4     PHA
         JSR  pas_get_token
         JSR  TERM
         PLA
         CMP  #'-'
         BEQ  SIM5
         CMP  #'+'
         BEQ  SIM6
         CMP  #TOKEN_XOR          ; XOR
         BEQ  SIM8
         LDA  #PCODE_ORR          ; ORR       OR  (sp - 1) | (sp)
SIM7     JSR  GENNOP
         JMP  SIM3
SIM5     LDA  #PCODE_SUB          ; MINUS
         BNE  SIM7
SIM6     LDA  #PCODE_ADD          ; PLUS
         BNE  SIM7
SIM2     JSR  TERM
         JMP  SIM3
SIM8     LDA  #PCODE_XOR         ; XOR
         BNE  SIM7
;
; TERM
;
TERMT1   ASC    '*'
         word   TERM1
         DFB    TOKEN_DIV    ; div
         word   TERM1
         ASC    '/'
         word   TERM1
         DFB    TOKEN_AND    ; and
         word   TERM1
         DFB    TOKEN_MOD    ; mod
         word   TERM1
         DFB    TOKEN_SHL    ; shl
         word   TERM1
         DFB    TOKEN_SHR    ; shr
         word   TERM1
         DFB    0
;
TERM     JSR  FACTOR
TERM2    LDX  #<TERMT1
         LDY  #>TERMT1
         LDA  token_type
         JSR  TKNJMP
         RTS
;
TERM1    PHA
         JSR  pas_get_token
         JSR  FACTOR
         PLA
         LDX  #<TERMT3
         LDY  #>TERMT3
         JSR  TKNJMP
;
TERM4    LDA  #PCODE_DIV       ; Divide (sp - 1) / (sp)
TERM3    JSR  GENNOP
         JMP  TERM2
TERM5    LDA  #PCODE_AND        ; AND
         BNE  TERM3
TERM6    LDA  #PCODE_MOD        ; MOD
         BNE  TERM3
TERM7    LDA  #PCODE_SHL       ; Shift left (sp) bits
         BNE  TERM3
TERM8    LDA  #PCODE_SHR       ; Shift right (sp) bits
         BNE  TERM3
TERM9    LDA  #PCODE_MUL        ; Multiply (sp) * (sp - 1)
         BNE  TERM3
;
TERMT3   DFB    TOKEN_DIV     ; div
         word   TERM4
         ASC    '/'
         word   TERM4
         DFB    TOKEN_AND     ; and
         word   TERM5
         DFB    TOKEN_MOD     ; mod
         word   TERM6
         DFB    TOKEN_SHL     ; shl
         word   TERM7
         DFB    TOKEN_SHR     ; shr
         word   TERM8
         ASC    '*'
         word   TERM9
         DFB  0
;
; FACTOR
;
FACTOR   JSR  CHK_STAK
         LDA  token_type
         LDX  #<FACTB1
         LDY  #>FACTB1
         JSR  TKNJMP
         LDX  #23     ; ERROR: Illegal factor
         jmp  ERROR
;
IDENT    JSR  LOOKUP
IDENT1   CMP  #SYMBOL_PROCEDURE
         beq  IDENT1A
         CMP  #SYMBOL_LIBRARY_PROCEDURE
         BNE  IDENT2
IDENT1A:
         LDX  #21     ; ERROR: Use of procedure Identifier in expression
         jmp  ERROR
IDENT2   CMP  #SYMBOL_FUNCTION_RETURN
         BNE  IDENT2A
         LDY  #SYMPRV      ; this symbol is actually the function return value symbol
         LDA  (SYMITM),Y   ; so go to the previous symbol which will be the function
         TAX               ; declaration symbol with the address of the function to be called
         INY
         LDA  (SYMITM),Y
         STA  SYMITM+1
         TXA
         STA  SYMITM
         lda  #$80       ; push 0 onto the stack which will be the default function return value
         jsr  GENNOP     ; when the function returns this will be left on the top of the stack
         JMP  FNCPRC     ; with the function's assigned value in it (or zero if nothing assigned);

IDENT2A  CMP #SYMBOL_LIBRARY_FUNCTION
         bne IDENT3
         jmp  library_function_call

IDENT3   CMP  #SYMBOL_ARRAY
         BEQ  IDENT4
         CMP  #SYMBOL_CONSTANT
         BNE  IDENT5
         JSR  VAL_MOVE
         JMP  IDENT7
;
;   Here for address of integer variable
;
FACAD1   LDA  #PCODE_ADRNN
         JSR  IDENT5_A
         JMP  CHKRHP
;
IDENT5   LDA  #PCODE_LOD   ; load integer onto stack
IDENT5_A PHA
;
         STX  BSAVE
         LDA  LEVEL
         SEC
         SBC  BSAVE
         STA  DISPL
         PLA
IDENT6   CLC
         ADC  DATTYP
         JSR  GENADR
IDENT7   JMP  pas_get_token
;
FACAD2   LDA  #PCODE_ADRAN  ; address of integer array
         JSR  IDENT4_A
         JMP  CHKRHP
;
;    Here for load indexed integer
;
IDENT4   LDA  #PCODE_LDI         ; Load integer indexed
IDENT4_A PHA
;
         JSR  SYMWRK
         JSR  PSHWRK
         JSR  GETSUB
         JSR  PULWRK
         JSR  WRKSYM
         JSR  GET_DAT
         JSR  GET_LEV
         JSR  GET_OFF
         PLA
         CLC
         ADC  DATTYP
         JMP  GENADR
;
; ADDRESS (IDENTIFIER)
;
;
FACADR   =  *
         JSR  CHKLHP
         JSR  GET_LOOK
         CMP  #SYMBOL_VARIABLE
         BEQ  FACAD1
         CMP  #SYMBOL_ARRAY
         BEQ  FACAD2
         LDX  #23     ; ERROR: Illegal factor
         jmp  ERROR
;
;
FACSTR   LDA  token_length
         CMP  #4
         BCC  FACNUM    ; BLT
FACERR1  LDX  #29        ; ERROR: string literal too big
         jmp  ERROR
FACNUM   =  *
         JSR  VAL_MOVE
         JMP  IDENT7
;
PAREN    JSR  GETEXPR
         JMP  CHKRHP
;
FACMEM   LDA  #0          ; add zero below to get PCODE_LDA
         STA  DATTYP
         BEQ  FACM2
FACMMC   LDA  #1          ; add one below to get PCODE_LDAC
         STA  DATTYP
FACM2    LDA  DATTYP
         PHA
         JSR  GETSUB
         PLA
         CLC
         ADC  #PCODE_LDA    ; Load absolute address integer or PCODE_LDAC if we added 1
         BNE  GENNOP1
;
FACNOT   JSR  pas_get_token
         JSR  FACTOR
         LDA  #PCODE_EOR     ; Not (sp) (logical negate)
GENNOP1  JMP  GENNOP
;
FACRND1  JSR  GENNOP
         JMP  pas_get_token
;
;
;
FACTB1   DFB    TOKEN_IDENTIFIER
         word   IDENT
         ASC    TOKEN_NUMBER
         word   FACNUM
FACTQT1  DFB    TOKEN_STRING        ; QUOTE SYMBOL
         word   FACSTR
         ASC    '('
         word   PAREN
         DFB    TOKEN_MEM
         word   FACMEM     ; MEM
         DFB    TOKEN_NOT
         word   FACNOT
         DFB    TOKEN_MEMC
         word   FACMMC     ; MEMC
         DFB    TOKEN_ADDRESS
         word   FACADR
         DFB  0
;
; EXPRESSION
;
EXPRES   JSR  CHK_STAK
         JSR  SIMEXP
         LDA  token_type
         LDX  #<EXPTB1
         LDY  #>EXPTB1
         JSR  TKNJMP
         RTS
;
EXPTB1   ASC    '='
         word   EXPR1
         DFB    TOKEN_NEQ
         word   EXPR1
         ASC    '<'
         word   EXPR1
         DFB    TOKEN_LEQ
         word   EXPR1
         DFB    TOKEN_GEQ
         word   EXPR1
         ASC    '>'
         word   EXPR1
         DFB  0
;
EXPR1    PHA
         JSR  pas_get_token
         JSR  SIMEXP
         PLA
         LDX  #<EXPTB3
         LDY  #>EXPTB3
         JSR  TKNJMP
;
EXPTB3   ASC    '='
         word   EXPR2
         ASC    TOKEN_NEQ
         word   EXPR3
         ASC    '<'
         word   EXPR4
         DFB    TOKEN_GEQ
         word   EXPR5
         ASC    '>'
         word   EXPR6
         DFB    TOKEN_LEQ
         word   EXPR7
         DFB  0
;
EXPR2    LDA  #PCODE_EQL       ; EQL      Test (sp - 1) == (sp)
EXPR8    JSR  GENNOP
         RTS
EXPR3    LDA  #PCODE_NEQ       ; NEQ      Test (sp - 1) != (sp)
         BNE  EXPR8
EXPR4    LDA  #PCODE_LSS       ; LSS      Test (sp - 1) < (sp)
         BNE  EXPR8
EXPR5    LDA  #PCODE_GEQ       ; GEQ      Test (sp - 1) >= (sp)
         BNE  EXPR8
EXPR6    LDA  #PCODE_GTR       ; GTR      Test (sp - 1) > (sp)
         BNE  EXPR8
EXPR7    LDA  #PCODE_LEQ       ; LEQ      Test (sp - 1) <= (sp)
         BNE  EXPR8
;
; STATEMENT
;
STMNT    JSR  CHK_STAK
         LDA  token_type
         LDX  #<STMNT1
         LDY  #>STMNT1
         JSR  TKNJMP
         RTS
;
STMNT1   DFB    TOKEN_IDENTIFIER
         word   ASSIGN
         DFB    TOKEN_IF
         word   IF
         DFB    TOKEN_FOR
         word   FOR
         DFB    TOKEN_WHILE
         word   WHILE
         DFB    TOKEN_CASE
         word   CASE
         DFB    TOKEN_REPEAT
         word   REPEAT
         DFB    TOKEN_BEGIN
         word   BEG
         DFB    TOKEN_READ
         word   READ
         DFB    TOKEN_WRITE
         word   WRITE
         DFB    TOKEN_MEM
         word   MEM
         DFB    TOKEN_CALL
         word   CALLSB
         DFB    TOKEN_MEMC
         word   MEMC
         DFB    TOKEN_WRITELN
         word   WRITELN
         DB     TOKEN_LCDWRITE
         word   STMNT_WRITE_LCD
         DFB  0
;
; ASSIGNMENT - this is where we store an integer/char into a variable or array index
;
ASSIGN   JSR  LOOKUP
ASS1     LDX  #<ASSTB1
         LDY  #>ASSTB1
         JSR  TKNJMP
         LDX  #24     ; ERROR: Type mismatch
         jmp  ERROR
;
ASSTB1   DFB   SYMBOL_ARRAY
         word  ASSARR
         DFB   SYMBOL_VARIABLE
         word  ASSVAR
         DFB   SYMBOL_FUNCTION_RETURN
         word  ASSVAR
         DFB   SYMBOL_PROCEDURE
         word  FNCPRC
         DFB   SYMBOL_LIBRARY_PROCEDURE
         word  library_function_call
         DFB   SYMBOL_FUNCTION
         word  assign_bad_identifier
         DFB   SYMBOL_LIBRARY_FUNCTION
         word  assign_bad_identifier
         DFB  0

assign_bad_identifier:
          ldx #53   ; ERROR: Use of function identifier in statement
          jmp ERROR
;
ASSARR   JSR  SYMWRK
         JSR  PSHWRK
         LDA  #PCODE_STI    ; Store integer indexed
         CLC
         ADC  DATTYP        ; A becomes PCODE_STIC (Store character indexed) if DATTYP == 1
         PHA
         JSR  GETSUB
         JMP  ASS2
;
ASSVAR   JSR  SYMWRK
         JSR  PSHWRK
         LDA  #PCODE_STO    ;  Store integer
         CLC
         ADC  DATTYP        ; A becomes PCODE_STOC (Store character indexed) if DATTYP == 1
         PHA
         JSR  pas_get_token
ASS2     LDA  #TOKEN_ASSIGN
         LDX  #13       ; ERROR: := expected
         JSR  CHKTKN
         JSR  GETEXPR
         PLA
         JSR  PULWRK
         JSR  WRKSYM
         PHA
         JSR  GET_LEV
         JSR  GET_OFF
         PLA
         JMP  GENADR
;

;
;
; WRITELN
;
WRITELN  JSR  pas_get_token     ; SEE IF ( PRESENT
         CMP  #'('
         BNE  WRITELN9   ; NOPE
         JSR  WRIT9
WRITELN9 =  *
         LDA  #PCODE_OUTCR   ; OUTPUT C/R
         JMP  GENNOP
;
;
; WRITE
;
WRITE    JSR  CHKLHP
WRIT9    JSR  pas_get_token
         CMP  #TOKEN_STRING
         BNE  WRIT1
         LDA  #PCODE_OUS    ; Output string
         JSR  W_STRING
         JMP  WRIT5
;
W_STRING =  *
         JSR  GENNOP
         LDA  token_length
         JSR  GENNOP
         LDY  #0
WRIT2    LDA  INBUF,Y
WRIT10   INY
         phy
         JSR  GENNOP
         ply
         DEC  token_length
         BNE  WRIT2
         JMP  pas_get_token
;
WRIT1    =  *          ; here if not string
         CMP  #TOKEN_CHR       ; CHR?
         BEQ  W_CHR      ; yes
         CMP  #TOKEN_HEX       ; HEX?
         BEQ  W_HEX      ; yes
         JSR  EXPRES     ; just ordinary number - get it
         LDA  #PCODE_OUT        ; OUT       Output number
         JSR  GENNOP
WRIT5    LDA  token_type
         CMP  #','
         BEQ  WRIT9
         JMP  CHKRHP
;
; here for write (chr(x))
;
W_CHR    =  *
         LDA  #PCODE_OUTC   ; OUTC      Output character
W_CHR1   =  *
         JSR  WAIT_1     ; process expression in parentheses
         JMP  WRIT5      ; back for next item
;
; here for write (hex(x))
;
W_HEX    =  *
         LDA  #PCODE_OUH        ; OUH       Output hex number
         BNE  W_CHR1

;
;
; WRITELCD: can write number, string, hex or chr
;
STMNT_WRITE_LCD    JSR  CHKLHP
WRITE_LCD9    JSR  pas_get_token
         CMP  #TOKEN_STRING
         BNE  WRITE_LCD1
         LDA  #PCODE_LCD_WRITE_STR    ; Output string
         JSR  WRITE_LCD_STRING
         JMP  WRITE_LCD5
;
WRITE_LCD_STRING =  *
         JSR  GENNOP
         LDA  token_length
         JSR  GENNOP
         LDY  #0
WRITE_LCD2    LDA  INBUF,Y    ; strings are placed in INBUF during parsing
         INY
         phy
         JSR  GENNOP
         ply
         DEC  token_length
         BNE  WRITE_LCD2
         JMP  pas_get_token
;
WRITE_LCD1    =  *            ; here if not string
         CMP  #TOKEN_CHR      ; CHR?
         BEQ  WRITE_LDC_CHR   ; yes
         CMP  #TOKEN_HEX      ; HEX?
         BEQ  WRITE_LDC_HEX   ; yes
         JSR  EXPRES          ; just ordinary number - get it
         LDA  #PCODE_LCD_WRITE_NUM        ; OUT       Output number
         JSR  GENNOP
WRITE_LCD5    LDA  token_type
         CMP  #','
         BEQ  WRITE_LCD9
         JMP  CHKRHP
;
; here for lcdwrite (chr(x))
;
WRITE_LDC_CHR    =  *
         LDA  #PCODE_LCD_WRITE_CHR   ; OUTC     Output character
WRITE_LDC_CHR1   =  *
         JSR  WAIT_1          ; process expression in parentheses
         JMP  WRITE_LCD5      ; back for next item
;
; here for lcdwrite (hex(x))
;
WRITE_LDC_HEX    =  *
         LDA  #PCODE_LCD_WRITE_HEX        ; OUH       Output hex number
         BNE  WRITE_LDC_CHR1

;
;
; READ
;
READ     jsr  CHKLHP    ; check for "("
READ8    jsr  GET_LOOK
READ2    jsr  SYMWRK      ; SYMITM --> WORK
         jsr  PSHWRK
         ldx  #0
         stx  COUNT1      ; this will have 1 in it if we are reading into an array element
         cmp  #SYMBOL_ARRAY
         beq  READ3
         cmp  #SYMBOL_VARIABLE
         beq  READ9
         ldx  #12     ; ERROR: Illegal Identifier
         jmp  ERROR

READ9    jsr get_token
READ11   lda  #PCODE_INP  ; Input number
         clc
         adc  DATTYP      ; becomes PCODE_INPC (input character) if DATTYP == 1
         jsr  GENNOP
         jsr  PULWRK
         jsr  WRKSYM      ; WORK --> SYMITM
         jsr  GET_DAT
         jsr  GET_LEV
         jsr  GET_OFF
         lda  #PCODE_STO    ; Store integer
         ldx  COUNT1
         beq  READ7
         lda  #PCODE_STI    ; Store integer indexed
READ7    clc
         adc  DATTYP        ; add 1 if a char to become PCODE_STOC / PCODE_STIC
         jsr  GENADR
READ7_A  lda  token_type
         cmp  #','
         beq  READ8         ; get another variable to read into
;
;  done with READ statement
;
         jsr  CHKRHP        ; check for ")"
         rts
;
;  here for reading into an array
;
READ3    lda  DATTYP
         pha              ; push data type of array
         jsr  pas_get_token
         cmp  #'['
         beq  READ3_A
         pla            ; get back data type of array
         sta  DATTYP    ; 0 = integer, 1 = char
         bne  READ3_B
         ldx  #24       ; ERROR: Type mismatch (cannot read into array of integer)
         jmp  ERROR
READ3_B  jsr  PULWRK
         jsr  WRKSYM     ; WORK --> SYMITM
         lda  #PCODE_INS        ; READ STRING - Input string into array
         jsr  GENNOP
         jsr  GET_LEV
         jsr  GET_OFF
         ldy  #SYMSUB
         lda  (SYMITM),Y  ; maximum array size
         jsr  GENADR     ; A = length, not the P-code which we just output above
         jmp  READ7_A
;
;  read into subscripted item
;
READ3_A  jsr  GETEXPR   ; subscript
         jsr  CHKRHB    ; check for "]"
         inc  COUNT1    ; flag for reading into array element
         pla            ; get back data type of array
         sta  DATTYP    ; save
         jmp  READ11
;

;
;
TWO_OP   JSR  CHKLHP
         JSR  GETEXPR
ONE_OP2  JSR  GET_ITEM
ONE_OP   JSR  CHKRHP
         PLA
         JMP  GENNOP



GENNOP2  JMP  GENNOP
;

WAIT_1   PHA
         JSR  CHKLHP
         JSR  GETEXPR
         JMP  ONE_OP
;
; MEM
;
MEM      LDA  #0        ; integer
         PHA
         BEQ  MEM2
MEMC     LDA  #1        ; character
         PHA
MEM2     JSR  GETSUB
         LDA  #TOKEN_ASSIGN
         LDX  #13     ; ERROR: := expected
         JSR  CHKTKN
         JSR  GETEXPR
         PLA
         CLC
         ADC  #PCODE_STA     ; Store integer absolute address (or PCODE_STAC if A == 1)
         BNE  GENNOP2
;
; CALL ABSOLUTE ADDRESS
;
CALLSB   JSR  CHKLHP
         JSR  GETEXPR
         JSR  CHKRHP
         LDA  #PCODE_CLA    ; Call absolute address
         BNE  GENNOP2
;
; FUNCTION OR PROCEDURE CALL
;
FNCPRC   LDA  #0
         STA  COUNT1      ; number of arguments
         LDY  #SYMARG
         LDA  (SYMITM),Y
         BEQ  FNC1
         JSR  CHKLHP
FNC2     LDA  COUNT1    ; save on stack in case the expression calls functions
         PHA
         JSR  SYMWRK
         JSR  PSHWRK
         JSR  GETEXPR
         JSR  PULWRK
         JSR  WRKSYM
         PLA            ; get number of arguments back
         STA  COUNT1
         INC  COUNT1    ; add 1 for the one we just processed
         LDA  token_type
         CMP  #','
         BEQ  FNC2
         LDA  COUNT1
         LDY  #SYMARG
         CMP  (SYMITM),Y   ; check we got the right number of arguments
         BEQ  FNC3
         LDX  #35     ; ERROR: Parameters mismatched
         jmp  ERROR
FNC3     JSR  CHKRHP
         JMP  FNC5
FNC1     JSR  pas_get_token
FNC5     JSR  GET_LEV
         JSR  GET_OFF
         LDY  #SYMDAT
         LDA  (SYMITM),Y
         BNE  FNC5A
         LDA  OFFSET
         SEC
         SBC  PCODE         ; calculate relative procedure address
         STA  OFFSET
         LDA  OFFSET+1
         SBC  PCODE+1
         STA  OFFSET+1
         LDA  #PCODE_CLL         ; Relative procedure/function call
         BNE  FNC5B
FNC5A    LDA  #PCODE_ABSCLL      ; Absolute procedure/function call
FNC5B    JSR  GENADR
         LDA  COUNT1    ; number of arguments
         BEQ  FNC4
         LDA  COUNT1     ; TIMES 3
         ASL             ; multiply by two then add one more COUNT1
         BCS  FNC6
         ADC  COUNT1
         STA  COUNT1
         BCS  FNC6
         LDA  #0
         SEC
         SBC  COUNT1
         STA  OPND
         LDA  #$FF
         STA  OPND+1      ;  calculate: 0 - (number_of_arguments * 3)
         LDA  #PCODE_INT     ; Increment stack pointer (allow for arguments)
         JSR  GENJMP
FNC4     RTS
FNC6     LDX  #15       ; ERROR: compiler limits exceeded
         jmp  ERROR
;
;
; IF
;
IF       JSR  GETEXPR
         LDA  #TOKEN_THEN
         LDX  #16     ; ERROR: THEN expected
         JSR  CHKTKN
         JSR  pas_get_token
         JSR  PSHPCODE
         LDA  #PCODE_JMZ     ; JMZ      Jump if (sp) zero
         JSR  GENNJM
         JSR  STMNT
         LDA  token_type
         CMP  #TOKEN_ELSE       ; ELSE
         BEQ  IF1
IF2      JSR  PULWRK
         JSR  FIXAD
         RTS
IF1      JSR  PULWRK     ; HERE FOR ELSE
         JSR  WRK_WRKD
         JSR  PSHPCODE
         JSR  GENNJP
         JSR  WRKD_WRK
         JSR  FIXAD
         JSR  pas_get_token
         JSR  STMNT
         JMP  IF2
;
; BEGIN
;
BEG      JSR  pas_get_token
         JSR  STMNT
         LDA  token_type
         CMP  #';'
         BEQ  BEG
         LDA  #TOKEN_END       ; END
         LDX  #17         ; ERROR: ; or END expected
         JSR  CHKTKN
         JMP  pas_get_token
;
; REPEAT
;
REPEAT   JSR  PSHPCODE
REP1     JSR  pas_get_token
         JSR  STMNT
         LDA  token_type
         CMP  #';'
         BEQ  REP1
         LDA  #TOKEN_UNTIL
         LDX  #10           ; ERROR: ; expected
         JSR  CHKTKN
         JSR  GETEXPR
         JSR  PULWRK
         JSR  WRK_OPND
         LDA  #PCODE_JMZ         ; JMZ      Jump if (sp) zero
         JMP  GENRJMP
;
; WHILE
;
WHILE    JSR  PSHPCODE
         JSR  GETEXPR
         JSR  PSHPCODE
         LDA  #PCODE_JMZ       ; JMZ      Jump if (sp) zero
         JSR  GENNJM
         LDA  #TOKEN_DO
         LDX  #18             ; ERROR: DO expected
         JSR  CHKTKN
         JSR  pas_get_token
         JSR  STMNT
         JSR  PULWRK
         JSR  WRK_WRKD
         JSR  PULWRK
         JSR  WRK_OPND
         LDA  #PCODE_JMP     ; JMP      Jump unconditionally
         JSR  GENRJMP
         JSR  WRKD_WRK
         JMP  FIXAD
;
; CASE
;
CASE     JSR  GETEXPR
         LDA  #TOKEN_OF       ; OF
         LDX  #26         ; ERROR: "of " expected
         JSR  CHKTKN
         LDA  #1
         STA  COUNT1
CASE7    LDA  #0
         STA  COUNT2
CASE2    =  *
         LDA  #PCODE_MOV  ; make copy of selector:  MOV       Copy (sp) to (sp + 1)
         JSR  GENNOP
         JSR  GETEXPR    ; next expression to compare
         LDA  #PCODE_EQL       ;  EQL       Test (sp - 1) == (sp)
         JSR  GENNOP
         LDA  token_type
         CMP  #':'
         BEQ  CASE1
         LDA  #','
         LDX  #5      ; ERROR: , or : expected
         JSR  CHKTKN
         JSR  PSHPCODE
         LDA  #PCODE_JM1     ; JM1      Jump if (sp) not zero
         JSR  GENNJM
         INC  COUNT2
         JMP  CASE2
CASE1    JSR  PCD_WRKD
         LDA  #PCODE_JMZ     ; JMZ      Jump if (sp) zero
         JSR  GENNJM
         LDA  COUNT2
         BEQ  CASE3
CASE4    JSR  PULWRK
         JSR  FIXAD
         DEC  COUNT2
         BNE  CASE4
CASE3    JSR  WRKD_WRK
         JSR  PSHWRK
         JSR  pas_get_token
         LDA  COUNT1
         PHA
         JSR  STMNT
         PLA
         STA  COUNT1
         LDA  token_type
         CMP  #TOKEN_ELSE       ; ELSE
         BEQ  CASE5
         CMP  #';'
         BNE  CASE6
         JSR  PCD_WRKD
         JSR  GENNJP
         JSR  PULWRK
         JSR  FIXAD
         JSR  WRKD_WRK
         JSR  PSHWRK
         INC  COUNT1
         JMP  CASE7
CASE5    JSR  PCD_WRKD
         JSR  GENNJP
         JSR  PULWRK
         JSR  FIXAD
         JSR  WRKD_WRK
         JSR  PSHWRK
         JSR  pas_get_token
         LDA  COUNT1
         PHA
         JSR  STMNT
         PLA
         STA  COUNT1
CASE6    LDA  #TOKEN_END       ; END
         LDX  #17         ; ERROR: ; or END expected
         JSR  CHKTKN
         LDA  COUNT1
         BEQ  CASE8
CASE9    JSR  PULWRK
         JSR  FIXAD
         DEC  COUNT1
         BNE  CASE9
CASE8    JSR  FOR6
         JMP  pas_get_token
;
; FOR
;
FOR      LDA  #TOKEN_IDENTIFIER
         LDX  #4      ; ERROR:  Identifier expected
         JSR  GETCHK
         JSR  LOOKUP
FOR1     CMP  #SYMBOL_VARIABLE
         BEQ  FOR2
         CMP  #SYMBOL_FUNCTION_RETURN
         BEQ  FOR2
         LDX  #12   ; ERROR: Illegal Identifier
         jmp  ERROR
FOR2     JSR  ASSVAR
         JSR  SYMWRK
         LDA  #0
         STA  COUNT1
         LDA  token_type
         CMP  #TOKEN_TO       ; TO
         BEQ  FOR3
         LDA  #TOKEN_DOWNTO       ; DOWNTO
         LDX  #28       ; ERROR: TO or DOWNTO expected
         JSR  CHKTKN
         DEC  COUNT1
FOR3     LDA  COUNT1
         PHA
         JSR  PSHWRK
         JSR  GETEXPR
         JSR  PULWRK
         JSR  WRKSYM
         PLA
         STA  COUNT1
         JSR  PSHPCODE
         LDA  #PCODE_MOV     ; MOV      Copy (sp) to (sp + 1)
         JSR  GENNOP
         JSR  GET_LEV
         JSR  GET_OFF
         JSR  GET_DAT
         CLC
         ADC  #PCODE_LOD  ; Load integer onto stack or PCODE_LODC if a char
         JSR  GENADR
         LDA  #PCODE_GEQ        ; UP (GEQ): GEQ       Test (sp - 1) >= (sp)
         LDX  COUNT1
         BEQ  FOR4
         LDA  #PCODE_LEQ        ; DOWN (LEQ): LEQ       Test (sp - 1) <= (sp)
FOR4     JSR  GENNOP
         JSR  PSHPCODE
         LDA  #PCODE_JMZ       ;  JMZ       Jump if (sp) zero
         JSR  GENNJM
         LDA  COUNT1
         PHA
         JSR  SYMWRK
         JSR  PSHWRK
         LDA  #TOKEN_DO
         LDX  #18       ; ERROR: DO expected
         JSR  CHKTKN
         JSR  pas_get_token
         JSR  STMNT
         JSR  PULWRK
         JSR  WRKSYM
         JSR  GET_LEV
         JSR  GET_DAT
         JSR  GET_OFF
         LDA  DATTYP
         CLC
         ADC  #PCODE_LOD       ; LOD      Load integer onto stack
         JSR  GENADR
         PLA
         STA  COUNT1
         LDA  #PCODE_INC     ; INC      Increment (sp) by 1
         LDX  COUNT1
         BEQ  FOR5
         LDA  #PCODE_DEC        ; DEC       Decrement (sp) by 1
FOR5     JSR  GENNOP
         LDA  #PCODE_STO       ; STO      Store integer
         CLC
         ADC  DATTYP          ; may become PCODE_STOC if a char
         JSR  GENADR
         JSR  PULWRK
         JSR  WRK_WRKD
         JSR  PULWRK
         JSR  WRK_OPND
         LDA  #PCODE_JMP       ; JMP      Jump unconditionally
         JSR  GENRJMP
         JSR  WRKD_WRK
         JSR  FIXAD
FOR6     LDA  #$FF
         STA  OPND+1
         LDA  #$FD
         STA  OPND
         LDA  #PCODE_INT       ;  INT       Increment stack pointer
         JMP  GENJMP


;***********************************************
; MORE OF THE COMPILER (PROCESSING A 'BLOCK')
;***********************************************

CHKGET   =  *
         JSR  CHKTKN
         JMP  pas_get_token
;
;  copy WORK to token_value
;
WRK_VAL  =  *
         PHA
         LDA  WORK
         STA  token_value
         LDA  WORK+1
         STA  token_value+1
         PLA
         RTS
;
;  copy VALUE to WORK
;
VAL_WRK  =  *
         PHA
         LDA  token_value
         STA  WORK
         LDA  token_value+1
         STA  WORK+1
         PLA
         RTS
;
;  copy ENDSYM to WORK
;
END_WRK  =  *
         PHA
         LDA  ENDSYM
         STA  WORK
         LDA  ENDSYM+1
         STA  WORK+1
         PLA
         RTS
;
;***********************************************
;
;
; BLOCK
;
BLCKT1   DFB  TOKEN_CONST
         word BLKCNS
BLCKT2   DFB  TOKEN_VAR
         word BLKVAR
BLCKT3   DFB  TOKEN_PROCEDURE
         word BLKPRC
         DFB  TOKEN_FUNCTION
         word BLKFNC
         DFB  TOKEN_BEGIN
         word BLKBEG
         DFB  0
;
;
BLOCK    JSR  CHK_STAK
         LDA  #0
         STA  FRAME+1
         LDA  #6
         STA  FRAME
         LDA  PRCITM
         STA  WORK
         LDX  PRCITM+1
         STX  WORK+1
         ORA  PRCITM+1
         BEQ  BLK1

;
BLK1A    LDY  #SYMDSP
         LDA  PCODE
         STA  (WORK),Y
         INY
         LDA  PCODE+1
         STA  (WORK),Y
         LDA  #0
         LDY  #SYMDAT
         STA  (WORK),Y   ; FLAG RELATIVE PROCEDURE
         JMP  BLK2
BLK1     LDA  PCODE
         STA  WORK
         LDA  PCODE+1
         STA  WORK+1
BLK2     JSR  PSHWRK
         JSR  GENNJP
         LDX  #<BLCKT1
         LDY  #>BLCKT1
BLK4     LDA  token_type
         JSR  TKNJMP
         LDX  #25       ; ERROR: BEGIN expected
         jmp  ERROR
;
;
; CONSTANT
;
BLKCNS   JSR  pas_get_token
BLKCN1   JSR  CONDEC
         LDA  #';'
         LDX  #10       ; ERROR: ; expected
         JSR  CHKGET
         LDX  #<BLCKT2
         LDY  #>BLCKT2
         JSR  TKNJMP
         JMP  BLKCN1
;
; VARIABLE
;
BLKVAR   LDA  #0
         STA  COUNT1
BLKVR1   JSR  pas_get_token
BLKVR6   JSR  VARDEC    ; check we have an identifier, add it to the symbol table, get another token
         INC  COUNT1    ; how many variables in this declaration
         BPL  BLKVR7
         JMP  BLKV13     ; ERROR  - compiler limits exceeded (more than 127 variables?)
BLKVR7   LDA  token_type
         CMP  #','
         BEQ  BLKVR1    ; back for another
         LDA  #':'
         LDX  #5        ; ERROR: , or : expected
         JSR  CHKGET
         CMP  #TOKEN_ARRAY   ; ARRAY
         BEQ  BLKVR2
         CMP  #TOKEN_INTEGER ; INTEGER
         BEQ  BLKVR8
         LDA  #TOKEN_CHAR    ; CHAR
         LDX  #36            ; ERROR: Data Type not recognised
         JSR  CHKTKN
         JMP  BLKVR3

;
;  integer variable
;
BLKVR8   JSR  BLKVR9  ; subtract COUNT1 from FRAME, put WORK into VALUE, put ENDSYM into WORK
BLKV10   = *
         LDY  #SYMDAT
         LDA  #0         ; INTEGER TYPE
         STA  (WORK),Y
         LDA  FRAME
         LDY  #SYMDSP
         STA  (WORK),Y
         INY
         LDA  FRAME+1
         STA  (WORK),Y
         CLC
         LDA  FRAME
         ADC  #3
         STA  FRAME
         BCC  BLKV10_A
         INC  FRAME+1
BLKV10_A =  *
         DEC  COUNT1
         BNE  BLKV10_B
         JMP  BLKVR3

BLKV10_B = *
;
;  more in the list, go to the previous symbol
;
         LDY  #SYMPRV
         LDA  (WORK),Y
         TAX
         INY
         LDA  (WORK),Y
         STA  WORK+1
         TXA
         STA  WORK       ; PREVIOUS ITEM
         BRA  BLKV10

;
; ARRAY [ N ] OF ...
;
BLKVR2   JSR  CHKLHB
         JSR  CONST     ; get how many are in the array
         LDA  token_value+2   ; can't be more than 65536
         BNE  BLKV13
         LDA  token_value     ; add 1 to the count since array [3] of char means 4 elements
         CLC
         ADC  #1
         STA  token_value
         LDA  token_value+1
         BMI  BLKV13
         ADC  #0
         BPL  BLKVR4
BLKV13   LDX  #15       ; ERROR: compiler limits exceeded
         jmp  ERROR
BLKVR4   STA  token_value+1
         JSR  VAL_WRK   ; put the value (array count) in WORK
         JSR  pas_get_token
         JSR  CHKRHB
         LDA  #1        ; char
         STA  DATTYP
         LDA  #TOKEN_OF  ; OF
         LDX  #26        ; ERROR: "of " expected
         JSR  CHKGET
         CMP  #TOKEN_INTEGER  ; INTEGER
         BNE  BLKV11
         DEC  DATTYP    ; DATTYP of 0 is integer, and 1 is char
         JSR  WRK_VAL   ; get the array count back into token_value
;
; MULTIPLY token_value BY 3
;
         LDA  token_value
         LDX  token_value+1
         ASL  token_value
         ROL  token_value+1
         BCS  BLKV13
         ADC  token_value
         STA  token_value
         TXA
         ADC  token_value+1
         BCS  BLKV13
         STA  token_value+1
         JSR  VAL_WRK     ; put the array count (times 3) back into WORK
         JMP  BLKV12
 ;
 ;  here for array of char
 ;
BLKV11   LDA  #TOKEN_CHAR       ; CHAR
         LDX  #36           ; ERROR: Data Type not recognised
         JSR  CHKTKN
BLKV12   JSR  BLKVR9        ; we are COUNT1 frames back
         JMP  BLKVR5

BLKVR9   =  *
         LDA  FRAME         ; subtract COUNT1 (the variable position) from the frame number
         SEC
         SBC  COUNT1
         STA  FRAME
         LDA  FRAME+1
         SBC  #0
         STA  FRAME+1
         JSR  WRK_VAL     ; copy WORK to VALUE (array count, possibly multiplied by 3)
         LDA  ENDSYM      ; end of symbol table into WORK
         STA  WORK
         LDA  ENDSYM+1
         STA  WORK+1
         RTS

;
;  we should be here with ENDSYM in WORK (for walking back through the symbol table for all the variables we just declared)
;  COUNT1 will be the number of symbols we have to fix up
;
BLKVR5   = *
         ldy  #SYMTYP
         lda  #SYMBOL_ARRAY
         sta  (WORK),Y    ; make symbol type array
         ldy  #SYMDSP
         lda  FRAME       ; FRAME is the displacement
         sta  (WORK),Y
         iny
         lda  FRAME+1
         sta  (WORK),Y
         lda  token_value       ; array count (multiplied by 3 in the case of integers)
         clc
         adc  FRAME       ; add to the stack frame address
         sta  FRAME
         lda  token_value+1
         adc  FRAME+1
         sta  FRAME+1
         ldy  #SYMDAT
         lda  DATTYP       ; save the data type in the symbol
         sta  (WORK),Y
         ldy  #SYMSUB
         lda  token_value        ; max subscript + 1
         sta  (WORK),Y
         lda  token_value+1
         iny
         sta  (WORK),Y
         dec  COUNT1
         beq  BLKVR3
;
;  onto the previous symbol
;
         ldy  #SYMPRV
         lda  (WORK),Y
         tax
         iny
         lda  (WORK),Y
         sta  WORK+1
         txa
         sta  WORK       ; PREVIOUS ITEM
         bra  BLKVR5
;
;  here at end of variable declaration (after INTEGER / CHAR)
;
BLKVR3   LDA  #';'
         LDX  #10       ; ERROR: ; expected
         JSR  GETCHK
         JSR  pas_get_token
         LDX  #<BLCKT3  ; now look for procedure / function / begin
         LDY  #>BLCKT3
         JSR  TKNJMP    ; if not there, look for another variable declaration
         LDA  #0        ; zero variables in this list so far
         STA  COUNT1
         JMP  BLKVR6
;
; PROCEDURE DECLARATION
;
BLKPRC   LDA  #'I'
         LDX  #4         ; ERROR: Identifier expected
         JSR  GETCHK
         LDA  #0
         STA  COUNT1    ; no function return value
         JSR  CHKDUP
         LDA  #SYMBOL_PROCEDURE
         JSR  ADDSYM
         INC  LEVEL
         LDA  SYMITM
         STA  PRCITM    ; PRCITM is the symbol of the procedure/function
         LDA  SYMITM+1
         STA  PRCITM+1
         JMP  BLKPR1
;
; FUNCTION DECLARATION
;
BLKFNC   LDA  #TOKEN_IDENTIFIER
         LDX  #4      ; ERROR: Identifier expected
         JSR  GETCHK
         JSR  CHKDUP
         LDA  #SYMBOL_FUNCTION
         JSR  ADDSYM
         INC  LEVEL
         LDA  #1
         STA  COUNT1    ; we have the function return as a symbol
         LDA  SYMITM
         STA  PRCITM   ; PRCITM is the symbol of the procedure/function
         LDA  SYMITM+1
         STA  PRCITM+1
         LDA  #SYMBOL_FUNCTION_RETURN ; for assigning the results to
         JSR  ADDSYM
;
; PROCEDURE AND FUNCTION COMMON CODE
;
BLKPR1   LDA  COUNT1
         STA  COUNT2    ; remember if we had 0 or 1 extra symbols (function return)
         JSR  END_WRK
         JSR  PSHWRK
         LDA  FRAME
         STA  WORK
         LDA  FRAME+1
         STA  WORK+1
         JSR  PSHWRK
         JSR  pas_get_token
         CMP  #'('      ; opening bracket after procedure/function name?
         BNE  BLKPR2
BLKPR3   JSR  pas_get_token
         JSR  VARDEC
         INC  COUNT1    ; count arguments
         BPL  BLKPR6
         JMP  BLKV13
BLKPR6   LDA  token_type
         CMP  #','
         BEQ  BLKPR3
         JSR  CHKRHP
BLKPR2   LDA  PRCITM      ; here after argument list, if any
         STA  WORK
         LDA  PRCITM+1
         STA  WORK+1
         LDY  #SYMARG
         LDA  COUNT1
         SEC
         SBC  COUNT2      ; subtract 0 or 1 depending on if this is a function or not
         STA  (WORK),Y    ; store the number of arguments to this procedure/function
         LDA  #';'
         LDX  #10         ; ERROR: ; expected
         JSR  CHKTKN
         LDA  COUNT1
         BEQ  BLKPR4
         JSR  END_WRK     ; ENDSYM -> WORK
         LDX  #$FD        ; X = -3
; WORK starts off pointing to the procedure/function symbol
BLKPR5   = *
         LDY  #SYMDAT
         LDA  #0
         STA  (WORK),Y    ; data type = integer
         LDY  #SYMDSP
         TXA
         STA  (WORK),Y
         SEC
         SBC  #3
         TAX
         LDA  #$FF
         INY
         STA  (WORK),Y
         DEC  COUNT1
         beq  BLKPR4
;
;  back to previous symbol
;
         LDY  #SYMPRV
         LDA  (WORK),Y
         PHA
         INY
         LDA  (WORK),Y
         STA  WORK+1
         PLA
         STA  WORK
         bra  BLKPR5
BLKPR4   JSR  pas_get_token
         JSR  BLOCK
         DEC  LEVEL
         JSR  PULWRK
         LDA  WORK
         STA  FRAME
         LDA  WORK+1
         STA  FRAME+1
         JSR  PULWRK
         LDA  WORK
         STA  ENDSYM
         LDA  WORK+1
         STA  ENDSYM+1
         LDA  #';'
         LDX  #10       ; ERROR: ; expected
         JSR  CHKGET
         LDX  #<BLCKT3
         LDY  #>BLCKT3
         JMP  BLK4
;
; BEGIN (COMPOUND STATEMENT)
;
BLKBEG   JSR  pas_get_token
         JSR  PULWRK
         LDA  LEVEL
         BNE  BLKB1
BLKB3    JSR  FIXAD
         JMP  BLKB2
BLKB1    JSR  WRKSYM
         LDY  #SYMDSP
         LDA  (SYMITM),Y
         STA  WORK
         INY
         LDA  (SYMITM),Y
         STA  WORK+1
         LDY  #SYMDSP
         LDA  PCODE
         STA  (SYMITM),Y
         LDA  PCODE+1
         INY
         STA  (SYMITM),Y
         JMP  BLKB3
BLKB2    LDA  FRAME
         STA  OPND
         LDA  FRAME+1
         STA  OPND+1
         LDA  #PCODE_INT         ; INT      Increment stack pointer
         JSR  GENJMP
BLKB5    JSR  STMNT
         LDA  token_type
         CMP  #';'
         BNE  BLKB4
         JSR  pas_get_token
         JMP  BLKB5
BLKB4    LDA  #TOKEN_END       ; END
         LDX  #17         ; ERROR: ; or END expected
         JSR  CHKGET
         LDA  #PCODE_RTN  ; OK - we are at the end of a block - this is a procedure/function return
         LDX  LEVEL       ; OR - if we are at level 0, then that is the end of the program
         BNE  BLKB6
         LDA  #PCODE_FINISHD        ; Opcode: FINISHED  Stop run (end program)
TEST1    =  *
BLKB6    JMP  GENNOP
;


;
;  Writing library functions is quite easy:
;
;   1. For each of the n arguments that they have (could be zero) do a PULTOP to get
;      that value from the runtime stack into REG. Or, PULTOP2 to get the value into REG2.
;      For two arguments you can do PULBOTH to get them into REG and REG2.
;   2. Do whatever the function/procedure is supposed to do with those arguments
;   3. If a procedure, you are finished: JMP MAIN
;      For a function, put the result into REG, and then JMP MAINP
;      - or - (equivalently): JSR PSHTOP, JMP MAIN
;
;  Note: PULTOP puts the value into REG and also: A, X and Y registers where A is the low-order
;        byte, X the middle byte, and Y the high-order byte.
;

;                         Name           function or procedure   Args Execution address
pascal_library_functions_table:
;
;    functions
;
  makePasLibraryFunction "ABS",          SYMBOL_LIBRARY_FUNCTION,  1, EX_ABS
  makePasLibraryFunction "DIGITALREAD",  SYMBOL_LIBRARY_FUNCTION,  1, EX_DIGITALREAD
  makePasLibraryFunction "GETKEY",       SYMBOL_LIBRARY_FUNCTION,  0, EX_GETKEY
  makePasLibraryFunction "RANDOM",       SYMBOL_LIBRARY_FUNCTION,  0, EX_RANDOM
  makePasLibraryFunction "LATENCY",      SYMBOL_LIBRARY_FUNCTION,  0, EX_LATENCY
;
;      procedures
;
  makePasLibraryFunction "ASSERT",       SYMBOL_LIBRARY_PROCEDURE, 1, EX_ASSERT
  makePasLibraryFunction "DELAY",        SYMBOL_LIBRARY_PROCEDURE, 1, EX_DELAY
  makePasLibraryFunction "DIGITALWRITE", SYMBOL_LIBRARY_PROCEDURE, 2, EX_DIGITALWRITE
  makePasLibraryFunction "LCDCLEAR",     SYMBOL_LIBRARY_PROCEDURE, 0, EX_LCDCLEAR
  makePasLibraryFunction "LCDHOME",      SYMBOL_LIBRARY_PROCEDURE, 0, EX_LCDHOME
  makePasLibraryFunction "LCDPOS",       SYMBOL_LIBRARY_PROCEDURE, 2, EX_LCDPOS
  makePasLibraryFunction "PINMODE",      SYMBOL_LIBRARY_PROCEDURE, 2, EX_PINMODE
  makePasLibraryFunction "RANDOMSEED",   SYMBOL_LIBRARY_PROCEDURE, 1, EX_RANDOMSEED
  dfb 0   ; end of table

add_pascal_library_functions:
  lda #<pascal_library_functions_table
  sta REG
  lda #>pascal_library_functions_table
  sta REG+1
add_pascal_library_functions_loop:
  ldy #0
  lda (REG),Y
  beq add_pascal_library_functions_done
  lda REG
  sta token_address
  lda REG+1
  sta token_address+1
  stz token_length
;
;  find name length
;
add_pascal_library_functions_name_loop
  iny
  inc token_length
  lda (REG),Y
  bne add_pascal_library_functions_name_loop
  phy
  lda #SYMBOL_CONSTANT  ; add the symbol as a constant for now
  jsr ADDSYM
  ply

  iny             ; get past the 0x00
  lda (REG),Y     ; get symbol type
  phy
  ldy #SYMTYP
  sta (SYMITM),Y  ; change symbol type to be correct
  ply

  iny             ; get next byte
  lda (REG),Y     ; get number of arguments
  phy
  ldy #SYMARG
  sta (SYMITM),Y  ; save number of arguments
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
  bra add_pascal_library_functions_loop

add_pascal_library_functions_done:
  rts


library_function_call:

  lda  #0
  sta  COUNT1      ; number of arguments
  ldy  #SYMARG
  lda  (SYMITM),Y
  beq  library_function_call_1

;
;  here if it has arguments - push them on the stack
;

  jsr  CHKLHP
library_function_call_2
  lda  COUNT1    ; save on stack in case the expression calls functions
  pha
  jsr  SYMWRK
  jsr  PSHWRK
  jsr  GETEXPR
  jsr  PULWRK
  jsr  WRKSYM
  pla            ; get number of arguments back
  sta  COUNT1
  inc  COUNT1    ; add 1 for the one we just processed
  lda  token_type
  cmp  #','
  beq  library_function_call_2

  lda  COUNT1
  ldy  #SYMARG
  cmp  (SYMITM),Y   ; check we got the right number of arguments
  beq  library_function_call_3

  ldx  #35     ; ERROR: Parameters mismatched
  jmp  ERROR
library_function_call_3:
  jsr  CHKRHP
  bra  library_function_call_5

;
;  here for no arguments
;

library_function_call_1:
  jsr  pas_get_token      ; one token look-ahead

library_function_call_5:
  ldy  #SYMDSP
  lda  (SYMITM),Y
  sta  OPND
  iny
  lda  (SYMITM),Y
  sta  OPND+1
  lda  #PCODE_LIB_CALL
  jsr  GENJMP
  rts


;***********************************************
; GENERATE P-CODES - NO OPERANDS
;***********************************************
GENNOP   =  *
         LDY  SYNTAX
         BNE  GEN1      ; no storing/displaying if syntax-only compile
         STA  (PCODE),Y
         PHA
         JSR  DISPAD
         PLA
         LDX  DCODE
         BEQ  GEN1
         JSR  DISHX
         JSR  CROUT
GEN1     =  *
         LDA  #1      ; 1-byte P-code
         BNE  GEN2_B
;***********************************************
; GENERATE P-CODES - WITH ADDRESS (3-bytes stored in DISPL (1)/OFFSET(2))
;***********************************************
GENADR   =  *
         LDY  SYNTAX
         BNE  GEN2    ; no storing/displaying if syntax-only compile
         STA  (PCODE),Y
         PHA
         LDA  DISPL
         INY
         STA  (PCODE),Y
         LDA  OFFSET
         INY
         STA  (PCODE),Y
         LDA  OFFSET+1
         INY
         STA  (PCODE),Y
         JSR  DISPAD
         PLA
         LDX  DCODE
         BEQ  GEN2
         JSR  DISHX
         LDA  DISPL
         JSR  DISHX
         LDA  OFFSET
         JSR  DISHX
         LDA  OFFSET+1
         JSR  DISHX
         JSR  CROUT
GEN2     =  *
         LDA  #4      ; 4-byte P-code
GEN2_B   =  *
         CLC
         ADC  PCODE
         STA  PCODE
         BCC  GEN2_A
         INC  PCODE+1
GEN2_A   =  *
         LDA  SYNTAX
         BNE  GEN2_C      ; no storing/displaying if syntax-only compile
         LDA  PCODE+1     ; see if P-codes full
         CMP  ENDSYM+1   ; in other words, have we hit the symbol table?
         BCC  GEN2_C      ; less than
         BNE  GEN_FULL
         LDA  PCODE
         CMP  ENDSYM   ; TODO - what?
         BCC  GEN2_C
GEN_FULL LDX  #1         ; MEM FULL
         jmp  ERROR
GEN2_C   =  *
DISP9    RTS
;***********************************************
; GENERATE P-CODES - JUMP ADDRESS (relative to current P-code address in OPND)
;***********************************************
GENRJMP  =  *
         PHA
         LDA  OPND
         SEC
         SBC  PCODE
         STA  OPND      ; subtract P-code address to find difference
         LDA  OPND+1
         SBC  PCODE+1
         STA  OPND+1
         PLA
         JMP  GENJMP
;
GENNJP   =  *
         LDA  #PCODE_JMP       ; JMP

; Generate jump, P-code is in "A" register (eg. PCODE_JMZ, PCODE_JM1)
;
;  Note: the destination here is 0000, however FIXAD will be called later to fix up
;        the jump, once we know what the destination will be (ie. further down the code)
;
GENNJM   LDX  #0
         STX  OPND
         STX  OPND+1
;
;***********************************************
; GENERATE P-CODES - JUMP ADDRESS (absolute address in OPND)
;   also used in other places, like increment stack address
;***********************************************
GENJMP   =  *
         LDY  SYNTAX
         BNE  GEN3     ; no storing/displaying if syntax-only compile
         STA  (PCODE),Y
         PHA
         LDA  OPND
         INY
         STA  (PCODE),Y
         LDA  OPND+1
         INY
         STA  (PCODE),Y
         JSR  DISPAD
         PLA
         LDX  DCODE
         BEQ  GEN3
         JSR  DISHX
         LDA  OPND
         JSR  DISHX
         LDA  OPND+1
         JSR  DISHX
         JSR  CROUT
GEN3     =  *
         LDA  #3        ; 3-byte P-code
         JMP  GEN2_B

;***********************************************
; FIXUP ADDRESSES
;***********************************************
FIXAD    =  *
         LDY  SYNTAX
         BNE  FIXAD1
         LDY  #1
         LDA  PCODE
         SEC
         SBC  WORK
         STA  (WORK),Y
         INY
         LDA  PCODE+1
         SBC  WORK+1
         STA  (WORK),Y
         LDA  DCODE
         BEQ  FIXAD1
         LDA  #<FIXM1
         LDX  #>FIXM1
         JSR  print
         LDA  WORK+1
         JSR  PRBYTE
         LDA  WORK
         JSR  DISHX
         LDA  #<FIXM2
         LDX  #>FIXM2
         JSR  print
         LDA  PCODE+1
         JSR  PRBYTE
         LDA  PCODE
         JSR  DISHX
         JMP  CROUT
FIXAD1    rts

FIXM1    asciiz  'Jump at '
FIXM2    asciiz  'changed to '   ; changed to

