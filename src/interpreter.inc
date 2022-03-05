;--------------------------------------------------
; Runtime P-code interpreter
;
;--------------------------------------------------


;--------------------------------
; P-codes - these are generated in the compilation phase and looked up
;           in execution_address_table at runtime
;
; Note: some P-codes must appear in sequence, eg. PCODE_STO and PCODE_STOC
;       That is because one refers to integers (STO) and one refers to characters (STOC)
;       The code in various places is virtually identical except it adds 1 to the P-code
;       for characters. These generally are the Load/Store operations.
;--------------------------------

; Not implemented yet: $05, $13, $15, $39

;                 Code            Description                         Arguments
;                 ----            -----------------------------       -----------------------
PCODE_LIT     =   $00 ; Dec:  0 - Load constant                     - 3-byte literal (number)
PCODE_STACK   =   $01 ; Dec:  1 - Alter runtime stack to address    - 2-byte stack address
PCODE_NEG     =   $02 ; Dec:  2 - Unary minus 0 - (sp)              - none
PCODE_LIB_CALL =  $03 ; Dec:  3 - Call library function/procedure   - absolute address
PCODE_ADD     =   $04 ; Dec:  4 - Add (sp) to (sp - 1)              - none
PCODE_SUB     =   $06 ; Dec:  6 - Subtract (sp) from (sp - 1)       - none
PCODE_MUL     =   $08 ; Dec:  8 - Multiply (sp) * (sp - 1)          - none
PCODE_DIV     =   $0A ; Dec: 10 - Divide (sp - 1) / (sp)            - none
PCODE_MOD     =   $0B ; Dec: 11 - Modulus (sp - 1) MOD (sp)         - none
PCODE_ADRNN   =   $0C ; Dec: 12 - Address of integer                - level, displ
PCODE_ADRNC   =   $0D ; Dec: 13 - Address of character              - level, displ
PCODE_ADRAN   =   $0E ; Dec: 14 - Address of integer array          - level, displ
PCODE_ADRAC   =   $0F ; Dec: 15 - Address of character array        - level, displ
PCODE_EQL     =   $10 ; Dec: 16 - Test (sp - 1) == (sp)             - none
PCODE_FINISHD =   $11 ; Dec: 17 - Stop run (end program)            - none
PCODE_NEQ     =   $12 ; Dec: 18 - Test (sp - 1) != (sp)             - none
PCODE_LSS     =   $14 ; Dec: 20 - Test (sp - 1) < (sp)              - none
PCODE_GEQ     =   $16 ; Dec: 22 - Test (sp - 1) >= (sp)             - none
PCODE_GTR     =   $18 ; Dec: 24 - Test (sp - 1) > (sp)              - none
PCODE_LEQ     =   $19 ; Dec: 25 - Test (sp - 1) <= (sp)             - none
PCODE_ORR     =   $1A ; Dec: 26 - OR  (sp - 1) | (sp)               - none
PCODE_AND     =   $1B ; Dec: 27 - AND (sp - 1) & (sp)               - none
PCODE_INP     =   $1C ; Dec: 28 - Input number                      - none
PCODE_INPC    =   $1D ; Dec: 29 - Input character                   - none
PCODE_OUT     =   $1E ; Dec: 30 - Output number                     - none
PCODE_OUTC    =   $1F ; Dec: 31 - Output character                  - none
PCODE_EOR     =   $20 ; Dec: 32 - Not (sp) (logical negate)         - none
PCODE_OUH     =   $21 ; Dec: 33 - Output hex number                 - none
PCODE_SHL     =   $22 ; Dec: 34 - Shift left (sp) bits              - none
PCODE_OUS     =   $23 ; Dec: 35 - Output string                     - length, string
PCODE_SHR     =   $24 ; Dec: 36 - Shift right (sp) bits             - none
PCODE_INS     =   $25 ; Dec: 37 - Input string into array           - max, level, displ
PCODE_INC     =   $26 ; Dec: 38 - Increment (sp) by 1               - none
PCODE_CLL     =   $27 ; Dec: 39 - Relative procedure/function call  - level, relative address
PCODE_DEC     =   $28 ; Dec: 40 - Decrement (sp) by 1               - none
PCODE_RTN     =   $29 ; Dec: 41 - Procedure/function return         - none
PCODE_MOV     =   $2A ; Dec: 42 - Copy (sp) to (sp + 1)             - none
PCODE_CLA     =   $2B ; Dec: 43 - Call absolute address             - none
PCODE_LOD     =   $2C ; Dec: 44 - Load integer onto stack           - level, displ
PCODE_LODC    =   $2D ; Dec: 45 - Load character onto stack         - level, displ
PCODE_LDA     =   $2E ; Dec: 46 - Load absolute address integer     - none
PCODE_LDAC    =   $2F ; Dec: 47 - Load absolute address character   - none
PCODE_LDI     =   $30 ; Dec: 48 - Load integer indexed              - level, displ
PCODE_LDIC    =   $31 ; Dec: 49 - Load character indexed            - level, displ
PCODE_STO     =   $32 ; Dec: 50 - Store integer                     - level, displ
PCODE_STOC    =   $33 ; Dec: 51 - Store character                   - level, displ
PCODE_STA     =   $34 ; Dec: 52 - Store integer absolute address    - none
PCODE_STAC    =   $35 ; Dec: 53 - Store character absolute address  - none
PCODE_STI     =   $36 ; Dec: 54 - Store integer indexed             - level, displ
PCODE_STIC    =   $37 ; Dec: 55 - Store character indexed           - level, displ
PCODE_ABSCLL  =   $38 ; Dec: 56 - Absolute procedure/function call  - level, absolute address
PCODE_XOR     =   $3A ; Dec: 58 - XOR (sp - 1) ^ (sp)               - none
PCODE_INT     =   $3B ; Dec: 59 - Increment stack pointer           - increment
PCODE_JMP     =   $3C ; Dec: 60 - Jump unconditionally              - relative address
PCODE_JMZ     =   $3D ; Dec: 61 - Jump if (sp) zero                 - relative address
PCODE_JM1     =   $3E ; Dec: 62 - Jump if (sp) not zero             - relative address
PCODE_OUTCR   =   $40 ; Dec: 64 - Output a carriage-return          - none
PCODE_LCD_WRITE_NUM = $42 ; Dec: 66 - Write number to LCD           - none
PCODE_LCD_WRITE_STR = $43 ; Dec: 67 - Write string to LCD           - none
PCODE_LCD_WRITE_HEX = $44 ; Dec: 68 - Write hex number to LCD       - none
PCODE_LCD_WRITE_CHR = $45 ; Dec: 69 - Write character to LCD        - none

PCODE_LAST    =   PCODE_LCD_WRITE_CHR  ; PUT THIS LAST! (error check)                  -

; 0x80 to 0xFF - Load low literal (onto stack), where literal is P-code with 8-bit clear (ie. P-code & 0x7F)


;
;  P-code execution routine lookup.
;  For symbolic version see list above
;
;  Y is zero when starting to execute these routines
;
execution_address_table  =  *
  word  EX_LIT             ; $00 = LIT      Load constant
  word  EX_NEW_STACK       ; $01 =          Change stack to addr
  word  EX_NEG             ; $02 = NEG      Unary minus: 0 - (sp)
  word  EX_LIB_CALL        ; $03 = LIB_CALL Library function call
  word  EX_ADD             ; $04 = ADD      Add (sp) to (sp - 1)
  word  EX_INVINS          ; $05 =          NOT IMPLEMENTED
  word  EX_SUB             ; $06 = SUB      Subtract (sp) from (sp - 1)
  word  EX_INVINS          ; $07 =          NOT IMPLEMENTED
  word  EX_MUL             ; $08 = MUL      Multiply (sp) * (sp - 1)
  word  EX_INVINS          ; $09 =          NOT IMPLEMENTED
  word  EX_DIV             ; $0A = DIV      Divide (sp - 1) / (sp)
  word  EX_MOD             ; $0B = MOD      Modulus (sp - 1) MOD (sp)
  word  EX_ADRNN           ; $0C = ADRNN    Address of integer
  word  EX_ADRNC           ; $0D = ADRNC    Address of character
  word  EX_ADRAN           ; $0E = ADRAN    Address of integer array
  word  EX_ADRAC           ; $0F = ADRAC    Address of character array
  word  EX_EQL             ; $10 = EQL      Test (sp - 1) == (sp)
  word  EX_FINISHD         ; $11 = FINISHD  Stop run (end program)
  word  EX_NEQ             ; $12 = NEQ      Test (sp - 1) != (sp)
  word  EX_INVINS          ; $13 =          NOT IMPLEMENTED
  word  EX_LSS             ; $14 = LSS      Test (sp - 1) < (sp)
  word  EX_INVINS          ; $15 =          NOT IMPLEMENTED
  word  EX_GEQ             ; $16 = GEQ      Test (sp - 1) >= (sp)
  word  EX_INVINS          ; $17 =          NOT IMPLEMENTED
  word  EX_GTR             ; $18 = GTR      Test (sp - 1) > (sp)
  word  EX_LEQ             ; $19 = LEQ      Test (sp - 1) <= (sp)
  word  EX_ORR             ; $1A = ORR      OR  (sp - 1) | (sp)
  word  EX_AND             ; $1B = AND      AND (sp - 1) & (sp)
  word  EX_INP             ; $1C = INP      Input number
  word  EX_INPC            ; $1D = INPC     Input character
  word  EX_OUT             ; $1E = OUT      Output number
  word  EX_OUTC            ; $1F = OUTC     Output character
  word  EX_EOR             ; $20 = EOR      Not (sp) (logical negate)
  word  EX_OUH             ; $21 = OUH      Output hex number
  word  EX_SHL             ; $22 = SHL      Shift left (sp) bits
  word  EX_OUS             ; $23 = OUS      Output string
  word  EX_SHR             ; $24 = SHR      Shift right (sp) bits
  word  EX_INS             ; $25 = INS      Input string into array
  word  EX_INC             ; $26 = INC      Increment (sp) by 1
  word  EX_CLL             ; $27 = CLL      Relative procedure/function call
  word  EX_DEC             ; $28 = DEC      Decrement (sp) by 1
  word  EX_RTN             ; $29 = RTN      Procedure/function return
  word  EX_MOV             ; $2A = MOV      Copy (sp) to (sp + 1)
  word  EX_CLA             ; $2B = CLA      Call absolute address
  word  EX_LOD             ; $2C = LOD      Load integer onto stack
  word  EX_LODC            ; $2D = LODC     Load character onto stack
  word  EX_LDA             ; $2E = LDA      Load absolute address integer
  word  EX_LDAC            ; $2F = LDAC     Load absolute address character
  word  EX_LDI             ; $30 = LDI      Load integer indexed
  word  EX_LDIC            ; $31 = LDIC     Load character indexed
  word  EX_STO             ; $32 = STO      Store integer
  word  EX_STOC            ; $33 = STOC     Store character
  word  EX_STA             ; $34 = STA      Store integer absolute address
  word  EX_STAC            ; $35 = STAC     Store character absolute address
  word  EX_STI             ; $36 = STI      Store integer indexed
  word  EX_STIC            ; $37 = STIC     Store character indexed
  word  EX_ABSCLL          ; $38 = ABSCLL   Absolute procedure/function call
  word  EX_INVINS          ; $39 =          NOT IMPLEMENTED
  word  EX_XOR             ; $3A = XOR      XOR (sp - 1) ^ (sp)
  word  EX_INT             ; $3B = INT      Increment stack pointer
  word  EX_JMP             ; $3C = JMP      Jump unconditionally
  word  EX_JMZ             ; $3D = JMZ      Jump if (sp) zero
  word  EX_JM1             ; $3E = JM1      Jump if (sp) not zero
  word  EX_INVINS          ; $3F =          NOT IMPLEMENTED
  word  EX_OUTCR           ; $40 = OUTCR    Output a carriage-return
  word  EX_INVINS          ; $41 =          NOT IMPLEMENTED
  word  EX_LCD_WRITE_NUM   ; $42 = LCD_WRITE_NUM  Write number to LCD
  word  EX_LCD_WRITE_STR   ; $43 = LCD_WRITE_STR  Write string to LCD
  word  EX_LCD_WRITE_HEX   ; $44 = LCD_WRITE_HEX  Write hex to LCD
  word  EX_LCD_WRITE_CHR   ; $45 = LCD_WRITE_CHR  Write character to LCD

;--------------------------------------------------------------------------
;
;  STACK FRAMES
;
;  The compiler can't know where variables are actually going to be stored because
;  they will be relative to the (interpreter) stack (not the processor stack) which
;  is an unknown address because procedures/functions can be called directly or from
;  other procedures/functions.
;
;  Hence variables are referenced to BASE which is the base of the current stack frame,
;  thus a variable will be a certain offset from that. However in the case of nested
;  calls we need to go back to a previous stack frame. That is what the 6-byte "stack
;  frame linkage data" is about.
;
;  Variables are referred to by a "level difference" and a displacement. For the current
;  procedure the level difference will be zero, and thus the variable will be BASE +
;  the offset for this particular variable.
;
;  For variables in earlier procedures/functions (ones which called this one) there will
;  be a level difference, being the depth (lexically) between the called function and the
;  one with the variable. For example, if foo calls bar, and bar recurses five times, foo
;  and bar are still only one level apart, from the point of view of accessing foo's
;  variables.
;
;  Thus, to find an address we need to "walk" back through the STATIC stack frame links
;  (bytes 5 and 6 of the stack frame data) to find the appropriate stack frame, and then
;  take the offset from that.
;
;  However for recursive calls, once a recursed call ends, we need to get back the stack
;  frame of the caller, which will be different from the static frame. So, to restore the
;  stack frame after a procedure/function ends, we must take the DYNAMIC stack frame address
;  (bytes 3 and 4 of the stack frame data) to get back the previous stack frame. In fact,
;  all procedures/functions do this, it's just that the static and dynamic stack frames will
;  be the same if the procedure/function does not recurse.
;
;  Finally, the return address (in the P-codes) for the procedure/function call is also
;  stored in the stack frame data (bytes 1 and 2 of the stack frame data) so that, after
;  doing a return, we can restore the P-code address to the PCODE variable after the call.
;
;  When a procedure/function is called these 6 bytes are pushed onto the stack. Then, for
;  some reason, the code moves the stack pointer back 6 (adds to it). Then as part of the
;  procedure/function initialisation the stack pointer has 6 or 9 added to it, to allow
;  for not overwriting this stack frame data, and in the case of functions, the extra 3
;  bytes are for the returned value.
;
;  Note that the stack grows downwards, as it starts at "high memory" (whatever that is
;  defined as, and grows downwards, taking the gap between the end of the P-codes and the
;  end of memory.
;
;  Then the function will add additional spaces to the stack (ie. by subtracting from the
;  stack pointer) to make room for variables declared in that procedure/function. Now we
;  are ready to push data onto the stack as part of normal expression evalution. It follows
;  from this that variables initially have undefined values, as they will contain whatever
;  happened to be in the stack when this space was allocated.
;
;  The function GETADR resolves these addresses by starting with BASE, the stack frame base
;  for the current procedure/function, and then using the static stack frame link to load a
;  (temporary) new base, and repeating that until the level difference is zero. Then it adds
;  the offset of the wanted variable to the stack base to derive the actual variable address.
;
;--------------------------------------------------------------------------


;
DM1               asciiz  ' Stack: '
DM2               asciiz  ' Base:  '
running_message   asciiz  'Running\n'

;
DEBUG    =  *
DB11     jsr  DISPAD    ; display P-code address
         lda  PCODE
         sta  WORK
         lda  PCODE+1
         sta  WORK+1
         ldx  #4
         jsr  display_x_characters      ; display 4 bytes from WORK (ie. the P-codes)
         jsr  CROUT
         ldx  DBGFLG
         bmi  DEBUG_DONE        ; trace only
         lda  #<DM1             ; ' Stack: '
         ldx  #>DM1
         jsr  print
         lda  T+1
         jsr  PRBYTE        ; display the stack pointer address
         lda  T
         jsr  DISHX         ; display hex and a space after
         lda  #'='
         jsr  COUT
         lda  T
         sta  WORK
         lda  T+1
         sta  WORK+1
         ldx  #9         ; show 9 bytes on stack, namely 3 variables
         jsr  display_x_characters
         jsr  CROUT
         lda  #<DM2       ; ' Base:  '
         ldx  #>DM2
         jsr  print
         lda  BASE+1
         jsr  PRBYTE      ; display the base pointer address
         lda  BASE
         jsr  DISHX
         lda  #'='
         jsr  COUT
         lda  BASE
         sec
         sbc  #6    ; the linkage data is 6 bytes below the base
         sta  WORK
         lda  BASE+1
         sbc  #0
         sta  WORK+1
         ldx  #6    ; show 6 bytes of base linkage data (return, dynamic link, static link)
         jsr  display_x_characters
         jmp  CROUT

DEBUG_DONE rts


;***********************************************
; Interpreter initialization
;***********************************************

INTERP   =  *
  php
  pla
  sta  call_p   ; make sure call_p has some reasonable value in it
;
;  Move the start of where the P-codes start into PCODE (our first P-code)
;
  lda  ACT_PCDA
  sta  PCODE
  lda  ACT_PCDA+1
  sta  PCODE+1
;
;  Running message
;
  lda  #<running_message   ; Running
  ldx  #>running_message
  jsr  print
;
;  Running flag
;
  ldy  #$0C
  sty  RUNNING
;
;  Set up the runtime stack pointer which is also our first stack frame base
;
  lda  #<HIGHEST_RAM
  sta  T
  sta  BASE
  lda  #>HIGHEST_RAM
  sta  T+1
  sta  BASE+1

  jmp  MAIN ; start interpreting
;
;
;


;***********************************************
;  INTERPRETER
;***********************************************

;
;
DM5      asciiz  "Illegal instruction\n"
DM6      asciiz  'Break ...\n'
DM7      asciiz  "Error occurred at P-code "

;
; DISPLAY (X) CHARACTERS FROM (WORK)
;
display_x_characters:
         txa
         pha
         jsr  PUTSP
         pla
         tax
DIS5     ldy  #0
         lda  (WORK),Y
         inc  WORK
         bne  DIS5_A
         inc  WORK+1
DIS5_A   =  *
         tay
         txa
         pha
         tya
         jsr  DISHX
         pla
         tax
         dex
         bne  DIS5
         rts

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
;
;
CHK_KBD  =  *
         cmp  #KEY_STOP_TRACE       ; Ctrl+N - stop tracing
         bne  CHK_NOTN
         stz  serial_in_byte_received
         lda  #0
         sta  DBGFLG
         sec
         rts
CHK_NOTN cmp  #KEY_TRACE      ; Ctrl+T - start tracing
         bne  CHK_NOTT
         stz  serial_in_byte_received
         lda  #$80
         sta  DBGFLG
         sta  DCODE
         sec
         rts
CHK_NOTT cmp  #KEY_DEBUG      ; Ctrl+D - start debugging
         bne  CHK_NOTD
         stz  serial_in_byte_received
         lda  #1
         sta  DBGFLG
         sta  DCODE
         sec
         rts
CHK_NOTD clc
         rts
;
EX_OUTCR    =  *
  jsr  CROUT      ; OUTPUT C/R
  jmp  MAIN
;
;  Here for a "low literal" - a P-code with the 0x80 bit set. Clear that bit and push
;   the resulting number onto the stack
;
LOWLIT:
  inc  PCODE     ; increment P-code past the low literal
  bne  LOWLIT1
  inc  PCODE+1
LOWLIT1:
  sty  REG+1     ; Y and REGB were cleared below
  and  #$7F
  sta  REG
;
;  fall down to push this value
;


;
; Push previous result (REG) and then drop down to do the next instruction
;
MAINP:
 jsr  PSHTOP    ; push REG onto stack and go back to MAIN

;
;  Main interpreter loop - each instruction jumps back here (or to MAINP above if it
;              needs to push a previous calculation result)
;
MAIN:
  lda  DBGFLG   ; debugging?
  beq  MAIN_2   ; no
  jsr  DEBUG    ; yes - show debug info
MAIN_2   =  *
; check if key pressed here, to abort, or turn on/off tracing or debugging
  lda  serial_in_byte_received
  beq  MAIN_OK
  cmp  #KEY_ABORT
  bne  MAIN_NOT_ABORT
  jmp  EX_FINISHD
MAIN_NOT_ABORT = *
  jsr  CHK_KBD          ; check for turning on trace, debug, etc.
MAIN_OK  =  *
  lda  PCODE             ; save P-code address as LASTP (for errors I presume)
  sta  LASTP
  lda  PCODE+1
  sta  LASTP+1
;
;  now grab the P-code
;
  ldy  #0
  sty  REGB
  lda  (PCODE),Y         ; get the P-code from the P-codes
  bmi  LOWLIT            ; low literal - just handle it
  cmp  #PCODE_LAST+1
  bcs  EX_INVINS         ; error if off end of table
;
;  increment past this P-code
;
MAIN_5   =  *
  inc  PCODE
  bne  MAIN_1
  inc  PCODE+1
;
;  grab the execution handler from the execution_address_table
;
MAIN_1   =  *
  asl  A             ; double the P-code because each address takes 2 bytes
  tax
  lda  execution_address_table,X      ; otherwise get the execution address
  sta  WORK
  lda  execution_address_table+1,X    ; and the high-order byte
  sta  WORK+1
  jmp  (WORK)        ; execute the appropriate P-code implementation routine

;
NOTIMP   =  *
EX_INVINS   =  *
         LDA  #<DM5   ; Illegal Instruction
         LDX  #>DM5
NOTIM1   =  *
         JSR  print
         JMP  RUNERR
;
BREAK    =  *
         LDA  #<DM6  ; Break ...
         LDX  #>DM6
         JMP  NOTIM1


GETADR   LDY  #0
         LDA  (PCODE),Y     ; level
         STA  COUNT1        ; count of levels to work through
         LDA  BASE+1        ; start with our stack base address
         LDX  BASE
GET2     =  *
         STA  DATA+1        ; our data is relative to the stack base
         STX  DATA
         TAY
         LDA  COUNT1        ; any levels left?
         BEQ  GETADR1       ; no, we can continue
         SEC                ; yes
         TXA                ; subtract 2 from the base to get the static link
         SBC  #2
         STA  WORK
         TYA
         SBC  #0
         STA  WORK+1        ; BASE - 2 -> WORK
         LDY  #0
         LDA  (WORK),Y      ; grab contents of BASE-2
         INY
         TAX
         LDA  (WORK),Y
         DEC  COUNT1        ; one less level
         JMP  GET2          ; keep going until we used all levels

GETADR1     =  *
         LDY  #1            ; now get the offset
         CLC
         LDA  (PCODE),Y     ; offset low-order byte
         ADC  DATA          ; add base
         STA  DATA
         INY
         LDA  (PCODE),Y     ; offset high-order byte
         ADC  DATA+1        ; add base
         STA  DATA+1        ; DATA now holds the data address
         LDA  PCODE         ; add 3 to PCODE to skip level and offset
         CLC
         ADC  #3
         STA  PCODE
         BCC  GET1_A
         INC  PCODE+1
GET1_A   =  *
         RTS                ; done - we can use DATA to access whatever-it-is

;
;  Pull the top item on the runtime stack into REG (aka VALUE)
;
PULTOP   =  *
         LDY  #0
         LDA  (T),Y
         STA  REG
         INY
         LDA  (T),Y
         STA  REG+1
         INY
         LDA  (T),Y
         STA  REGB
         LDA  T
         CLC      ; add 3 to the stack because we removed one integer from it
         ADC  #3
         STA  T
         BCC  PUL_END
         INC  T+1
PUL_END  =  *
         LDA  REG
         LDX  REG+1
         LDY  REGB
         RTS

PULBOTH  JSR  PULTOP     ; Pulls both of them - do one and then fall through to do the other

;
;  Pull the top item on the runtime stack into REG2 (aka VALUE2)
;
PULTOP2  =  *
         LDY  #0
         LDA  (T),Y
         STA  REG2
         INY
         LDA  (T),Y
         STA  REG2+1
         INY
         LDA  (T),Y
         STA  REG2B
         LDA  T
         CLC         ; add 3 to the stack because we removed one integer from it
         ADC  #3
         STA  T
         BCC  PUL2_END
         INC  T+1
PUL2_END =  *
         LDA  REG2
         LDX  REG2+1
         LDY  REG2B
         RTS

;
;  Push REG (aka VALUE) onto the runtime stack
;
PSHTOP   =  *
         SEC        ;  subtract 3 from the stack because we are about to add one integer to it
         LDA  T
         SBC  #3
         STA  T
         BCS  PSH1
         DEC  T+1
PSH1     =  *
         LDY  #0
         LDA  REG
         STA  (T),Y
         INY
         LDA  REG+1
         STA  (T),Y
         INY
         LDA  REGB
         STA  (T),Y
         RTS
;
;  get a literal (2 bytes) from the P-codes and put it into REG
;
GETLIT   =  *
  ldy  #0
  lda  (PCODE),Y
  sta  REG
  iny
  lda  (PCODE),Y
  sta  REG+1
  lda  PCODE
  clc         ; P-code address goes up by 2
  adc  #2
  sta  PCODE
  bcc  GET_END
  inc  PCODE+1
GET_END:
  rts
;
; load literal onto the stack (the literal is the next 3 bytes after the P-code)
;
EX_LIT:
  jsr  GETLIT   ; get the first 2 bytes
  ldy  #0
  lda  (PCODE),Y    ; now the third byte
  sta  REGB
  inc  PCODE
  bne  LIT1
  inc  PCODE+1
LIT1:
  jmp  MAINP    ; now we can push it
;
;  Negate (0 - (sp) )
;
EX_NEG:
  jsr PULTOP
  jsr exp_unary_minus
  jmp MAINP
;
;  Add (sp) to (sp -1)
;
EX_ADD:
  JSR  PULBOTH
  jsr  exp_add
  JMP  MAINP
;
;
;  Add (sp) from (sp - 1)
;
EX_SUB:
  JSR  PULTOP2
  jsr  PULTOP
  jsr  exp_subtract
  JMP  MAINP
;
;  Multiply (sp) by (sp - 1)
;
EX_MUL:
  jsr PULBOTH
  jsr exp_multiply
  jmp MAINP

;
;  we will take the current key (and return to the user)
;  and zero it out anyway, so that consumes the key
;
EX_GETKEY   =  *
  lda  serial_in_byte_received
  stz  serial_in_byte_received
  jmp  TRUE2
;
;  Modulus (remainder)
;
EX_MOD:
  jsr PULTOP2
;
;  check for divide by zero because I want a run-time error message
;
  lda VALUE2
  ora VALUE2+1
  ora VALUE2+2
  beq EX_DIVIDE_BY_ZERO
  jsr PULTOP
  jsr exp_modulo
  jmp MAINP
;

EX_DIVIDE_BY_ZERO:
  lda  #<DIVBY0
  ldx  #>DIVBY0
  jmp  NOTIM1

DIVBY0   asciiz  'Divide by zero\n'
;
;  Divide (sp - 1) by (sp)
;
EX_DIV:
  jsr PULTOP2
  lda VALUE2
  ora VALUE2+1
  ora VALUE2+2
  beq EX_DIVIDE_BY_ZERO
  jsr PULTOP
  jsr exp_divide
  jmp MAINP

;
;  Absolute value of (sp)
;
EX_ABS:
  jsr  PULTOP
  jsr exp_abs_val
  jmp  MAINP

; Equal
;
EX_EQL:
  jsr PULBOTH
  jsr exp_eql
  jmp MAINP

;
;  Push 1
;
TRUE:
  lda  #1
;
;  Push A into low-order byte, zero other two bytes
;
TRUE2:
  sta  REG
  lda  #0
  sta  REG+1
  sta  REGB
  jmp  MAINP

;
;  Push 0
;
FALSE:
  lda  #0
  bra TRUE2
;
;  Not equal
;
EX_NEQ:
    jsr PULBOTH
    jsr exp_neq
    jmp MAINP

;
;  Less than
;
EX_LSS:
  jsr PULTOP2
  jsr PULTOP
  jsr exp_less_than
  jmp MAINP

;
;  Greater than
;
EX_GTR:
  jsr PULTOP2
  jsr PULTOP
  jsr exp_greater_than
  jmp MAINP

;
;  Greater than or equal
;
EX_GEQ:
  jsr PULTOP2
  jsr PULTOP
  jsr exp_geq
  jmp MAINP

;
;  Less than or equal
;
EX_LEQ:
  jsr PULTOP2
  jsr PULTOP
  jsr exp_leq
  jmp MAINP

;
;  Exclusive OR
;
EX_XOR:
  jsr PULBOTH
  jsr exp_bitwise_xor
  jmp MAINP
;
;  Or
;
EX_ORR:
  jsr PULBOTH
  jsr exp_bitwise_or
  jmp MAINP

;
;  And
;
EX_AND:
  jsr  PULBOTH
  jsr exp_bitwise_and
  jmp MAINP

;
;  Not
;
EX_EOR:
  jsr PULTOP
  jsr exp_not
  jmp MAINP
;
; Shift left
;
EX_SHL:
  jsr PULTOP2
  jsr PULTOP
  jsr exp_shift_left
  jmp MAINP

;
;  Shift right
;
EX_SHR:
  jsr PULTOP2
  jsr PULTOP
  jsr exp_shift_right
  jmp MAINP

;
;  increment number on top of stack by one
;
EX_INC:
  clc
  lda  (T),Y
  adc  #1
  sta  (T),Y
  iny
  lda  (T),Y
  adc  #0
  sta  (T),Y
  iny
  lda  (T),Y
  adc  #0
  sta  (T),Y
  jmp  MAIN
;
;  decrement number on top of stack by one
;

EX_DEC:
  sec
  lda  (T),Y
  sbc  #1
  sta  (T),Y
  iny
  lda  (T),Y
  sbc  #0
  sta  (T),Y
  iny
  lda  (T),Y
  sbc  #0
  sta  (T),Y
  jmp  MAIN

;
;  Copy (sp) to (sp + 1)
;
EX_MOV:
  lda  (T),Y  ; get item at top of stack
  sta REG
  iny
  lda  (T),Y
  sta REG+1
  iny
  lda  (T),Y
  sta REGB
  jmp MAINP   ; push it

;
;  Load a single-character value (variable) onto the stack
;

EX_LODC:
  jsr  GETADR
EX_LOD3:
  ldy  #2
EX_LOD3_A:
  lda  #0      ; since this is a character zero out the high-order bytes
  sta  REG+1
  sta  REGB
  lda  (DATA),Y  ; (DATA) contains the lower-order byte
  sta  REG       ; REG will be pushed
  jmp  MAINP     ; push REG and go to MAIN
;
;  Load a 3-byte value (variable) onto the stack
;
EX_LOD:
  jsr  GETADR
EX_LOD2:
  ldy  #0
  lda  (DATA),Y    ; (DATA) contains the variable value
  sta  REG
  iny
  lda  (DATA),Y
  sta  REG+1
  iny
  lda  (DATA),Y
  sta REGB
  jmp MAINP   ; push it

;
;  Load absolute address character
;
EX_LDAC:
  jsr  PULTOP
  sta  DATA
  stx  DATA+1
  ldy  #0
  beq  EX_LOD3_A
;
;  Load absolute address integer
;
EX_LDA:
  jsr  PULTOP
  sta  DATA
  stx  DATA+1
  jmp  EX_LOD2
;
;  Get address of indexed character
;
GETIDC:
  jsr  PULTOP2    ; pull index into REG2
  jsr  GETADR
  jmp  GETID2
;
;  Get address of indexed integer
;
GETIDX   =  *
  jsr  PULTOP2  ; pull index into REG2
  asl  REG2     ; now multiply by 3
  rol  REG2+1
  clc
  adc  REG2
  sta  REG2
  txa
  adc  REG2+1
  sta  REG2+1     ; TIMES 3
  jsr  GETADR
;
;  Get address of indexed integer/character
;
;  It looks like arrays expand downwards
;
GETID2:
  lda  DATA     ; initial address
  sec           ; subtract subscript
  sbc  REG2
  sta  DATA
  lda  DATA+1
  sbc  REG2+1
  sta  DATA+1
  rts
;
;  Load indexed character
;
EX_LDIC:
  jsr  GETIDC
  jmp  EX_LOD3
;
;  Load indexed integer
;
EX_LDI:
  jsr  GETIDX
  jmp  EX_LOD2
;
;  Store character
;
EX_STOC:
  jsr  GETADR
  jsr  PULTOP
  ldy  #2   ; why?
EX_STO5:
  sta  (DATA),Y
  jmp  MAIN
;
;  Store integer
;
EX_STO:
  jsr  GETADR   ; get its address
  jsr  PULTOP   ; get the value to store
EX_STO2:
  ldy  #0
  sta  (DATA),Y
  iny
  txa
  sta  (DATA),Y
  lda  REGB
  iny
  bne  EX_STO5
;
;  Store integer at absolute address
;
EX_STA:
  jsr  PULBOTH  ; get address into REG2 and value into REG
  ldy  #0
  lda  REG
  sta  (REG2),Y
  iny
  lda  REG+1
  sta  (REG2),Y
  iny
  lda  REGB
EX_STA5:
  sta  (REG2),Y
  jmp  MAIN
;
;  Store character at absolute address
;
EX_STAC:
  jsr  PULBOTH  ; get address into REG2 and value into REG
  lda  REG
  ldy  #0
  beq  EX_STA5
;
;     Store character indexed
;
EX_STIC:
  jsr  PULTOP
  sta  TEMP
  jsr  GETIDC
  lda  TEMP
  ldy  #2
  bne  EX_STO5
;
;     Store integer indexed
;
EX_STI :
  jsr  PULTOP
  sta  TEMP
  stx  TEMP+1
  tya
  pha
  jsr  GETIDX
  ldy  #0
  lda  TEMP
  sta  (DATA),Y
  lda  TEMP+1
  iny
  sta  (DATA),Y
  pla
  iny
  bne  EX_STO5
;
;  Procedure/function return
;
;  Stack frame linkage data:
;
;    (from low to high address)
;
;    Return address <-- P-code to return to when function/procedure exits
;    Dynamic link   <-- The previous stack frame from the earlier function call
;    Static link    <-- The stack frame of the previous static function
;
;  Static and dynamic links will be the same except for recursive calls.
;
;  To unwind the stack we have to go back to the previous dynamic link (on a return)
;
;  However to find variables in previous lexical declarations we have to use the static link.
;
;
EX_RTN:
;
;  Subtract 6 from the base address, put result into WORK
;
  sec
  lda  BASE
  sbc  #6
  sta  WORK
  lda  BASE+1
  sbc  #0
  sta  WORK+1
;
;  The return address was at the bottom of BASE, so put that into PCODE
;
  ldy  #0
  lda  (WORK),Y
  sta  PCODE
  iny
  lda  (WORK),Y
  sta  PCODE+1

;
;  The base address becomes our new top of runtime stack
;
  lda  BASE+1
  sta  T+1
  lda  BASE
  sta  T
;
;  The dynamic stack frame address is 4 bytes down from the BASE - put that into WORK
;
  sec
  sbc  #4
  sta  WORK
  lda  BASE+1
  sbc  #0
  sta  WORK+1
;
;  Get the previous dynamic stack frame address and put it into BASE (in case of
;   recursive function calls)
;
  ldy  #0
  lda  (WORK),Y
  sta  BASE
  iny
  lda  (WORK),Y
  sta  BASE+1
;
;  Stack, base and Pcode all changed, ready for next instruction
;
  jmp  MAIN

;
; Input a number into a variable
;
EX_INP      =  *
  lda #$80
  sta  RUNNING
  JSR  GET_LINE
  lda  INBUF
  cmp  #KEY_ABORT   ; check for aborting with Ctrl+C
  bne  EX_INP_OK
  jmp  ex_input_aborted

EX_INP_OK:
  lda #<INBUF
  sta token_start
  lda #>INBUF
  sta token_start+1
  stz token_type
  jsr get_token
  cmp #TOKEN_NUMBER
  bne BAD_INP
  LDX  token_value
  LDY  token_value+1
  LDA  token_value+2
INP3:
  sty  REG+1
  stx  REG
  sta  REGB
  jsr  PSHTOP
  ldx  #12
  stx  RUNNING
  jmp  MAIN
;
;  bad number: set it to $800000
;
BAD_INP:
  lda #$80
  ldx #0
  ldy #0
  beq INP3
;
; write a number to output
;
EX_OUT:
  jsr write_to_serial
  bra OUT_COMMON

EX_LCD_WRITE_NUM:
  jsr write_to_lcd

OUT_COMMON:
  jsr  PULTOP
  jsr  display_in_decimal
  jsr  write_to_serial
  jmp  MAIN
;
;
; Output 6 hex characters (3 bytes)
;
EX_OUH:
  jsr write_to_serial
  bra EX_OUH_COMMON

EX_LCD_WRITE_HEX:
  jsr write_to_lcd

EX_OUH_COMMON:
  jsr  PULTOP
  lda  REGB
  jsr  PRBYTE
  lda  REG+1
  jsr  PRBYTE
  lda  REG
  jsr  PRBYTE
  jsr  write_to_serial
  jmp  MAIN
;
;  write a string to output
;
EX_OUS      =  *
  jsr write_to_serial
  bra EX_OUS_COMMON

EX_LCD_WRITE_STR:
  jsr write_to_lcd

EX_OUS_COMMON:
  lda  PCODE
  clc
  adc  #1
  sta  WORK
  lda  PCODE+1
  adc  #0
  sta  WORK+1
  lda  (PCODE),Y
  sta  COUNT1     ; NO. OF CHARS
  clc
  adc  #1
  adc  PCODE
  sta  PCODE
  bcc  EX_OUS1
  inc  PCODE+1
EX_OUS1:
  lda  WORK
  ldx  WORK+1
  ldy  COUNT1
  jsr  PT
  jsr write_to_serial
  jmp  MAIN



;
;  Stack frame linkage data:
;
;    (from low to high address)
;
;    Return address <-- P-code to return to when function/procedure exits
;    Dynamic link   <-- The previous stack frame from the earlier function call
;    Static link    <-- The stack frame of the previous static function
;
;  Static and dynamic links will be the same except for recursive calls.
;
;  To unwind the stack we have to go back to the previous dynamic link (on a return)
;
;  However to find variables in previous lexical declarations we have to use the static link.
;
;
; Call absolute procedure - I don't think this is used in this version TBH
;
; Note: Y is zero when entering
;
EX_ABSCLL   =  *
  sty  CALL      ; zero out address to be called
  sty  CALL+1    ; (offset therefore becomes absolute address)
  jmp  EX_CLL_A
;
; Call relative procedure
;
; Note: Y is zero when entering
;
EX_CLL      =  *
  lda  LASTP
  sta  CALL
  lda  LASTP+1
  sta  CALL+1
EX_CLL_A    =  *
  lda  (PCODE),Y     ; level
  sta  COUNT1
  iny
  clc
  lda  (PCODE),Y     ; relative address
  adc  CALL          ; add to current P-code address
  sta  CALL
  iny
  lda  (PCODE),Y
  adc  CALL+1
  sta  CALL+1
  lda  PCODE         ; add 3 to P-code to bypass level / address
  clc                ; this will become our return address
  adc  #3
  sta  PCODE
  bcc  EX_CLL4
  inc  PCODE+1
EX_CLL4     =  *
  lda  BASE+1
  ldx  BASE
EX_CLL2     =  *
  sta  DATA+1      ; this was BASE
  stx  DATA
  tay
  lda  COUNT1      ; this was the level
  beq  EX_CLL3
  sec
  txa
  sbc  #2          ; subtract 2 from BASE, store in WORK
  sta  WORK
  tya
  sbc  #0
  sta  WORK+1
  ldy  #0
  lda  (WORK),Y
  iny
  tax
  lda  (WORK),Y
  dec  COUNT1
  jmp  EX_CLL2
EX_CLL3     =  *
  lda  T         ; our stack pointer
  sta  TEMP      ; save it
  lda  T+1
  sta  TEMP+1
;
; this is as confusing as all-get-out but it looks like we
; are pushing onto the stack: DATA / BASE / PCODE (return address)
;  (which is why we saved T into TEMP so it didn't change during the pushes)
;

  lda  DATA      ; put DATA into REG
  sta  REG+1
  lda  DATA+1
  sta  REGB
  lda  BASE+1
  sta  REG
  jsr  PSHTOP      ; push REG (DATA + BASE[1])
  lda  BASE
  sta  REGB
  lda  TEMP        ; old stack pointer
  sta  BASE        ; becomes BASE
  lda  TEMP+1      ; ditto for other byte
  sta  BASE+1
  lda  PCODE       ; get ready to push current P-Code
  sta  REG
  lda  PCODE+1
  sta  REG+1
  jsr  PSHTOP     ; push REG (BASE[0] + PCODE)
  lda  CALL       ; now get the address of the place we want to call
  sta  PCODE      ; and put it into PCODE
  lda  CALL+1
  sta  PCODE+1
  clc             ; add 6 to the stack pointer because we later emit the code to INT 6 (leave room for base stuff)
  lda  T
  adc  #6
  sta  T
  bcc  EX_CLL5
  inc  T+1
EX_CLL5     =  *
  jmp  MAIN        ; we are done, MAIN will now go to the called subroutine
;
;  Here for calling machine code from Pascal
;
EX_CLA:
  jsr  PULTOP      ; pull the address to be called from the stack
  lda  call_p      ; status register
  pha
  lda  call_a      ; A register
  ldx  call_x      ; X register
  ldy  call_y      ; Y register
  plp              ; get status register back
  jsr  EX_CLL_JMP  ; call the subroutine
  php              ; save the status register
  sta  call_a      ; store the A/X/Y register
  stx  call_x
  sty  call_y
  pla              ; get status register back
  sta  call_p      ; save it
  jmp  MAIN        ; done!

EX_CLL_JMP  jmp  (REG)
;
;
; increment stack pointer (that is, subtract the literal from it, as it grows downwards)
;
EX_INT:
  jsr  GETLIT
  sec
  lda  T
  sbc  REG
  sta  T
  lda  T+1
  sbc  REG+1
  sta  T+1
  cmp  END_PCD+1
  bcc  INT_ERR
  jmp  MAIN
;
INT_ERR  =  *
  lda  #<INT_ERRM    ; stack full
  ldx  #>INT_ERRM
  jmp  NOTIM1
;
INT_ERRM asciiz "Stack full\n"    ; stack full
;

;
;  Change the runtime stack to the address given (assuming stack is empty)
;   Done by the {%S nnnn} compiler directive.
;
EX_NEW_STACK:
  jsr GETLIT
  lda REG
  sta T
  sta BASE
  lda REG+1
  sta T+1
  sta BASE+1
  jmp MAIN

;
;  Jump to the address following the P-code
;
EX_JMP:
  jsr  GETLIT   ; get the address
  clc
  lda  REG
  adc  LASTP
  sta  PCODE
  lda  REG+1
  adc  LASTP+1
  sta  PCODE+1
  jmp  MAIN
;
;  Jump if the top of the stack is zero
;
EX_JMZ:
  jsr  PULTOP   ; puts REG into A
  ora  REG+1
  ora  REGB
  bne  EX_NOJUMP
  beq  EX_JMP
;
;  Don't jump, just move past the address (TODO: just add 2?)
;
EX_NOJUMP:
  jsr  GETLIT
  jmp  MAIN
;
;  Jump if the top of the stack is non-zero
;
EX_JM1:
  jsr  PULTOP    ; puts REG into A
  ora  REG+1
  ora  REGB
  bne  EX_JMP
  beq  EX_NOJUMP
;
;
EX_INPC:
  jsr GETIN
  cmp  #KEY_ABORT   ; check for aborting with Ctrl+C
  bne  EX_INPC_OK
  jmp  ex_input_aborted

EX_INPC_OK:
  jsr CHK_KBD
  bcs EX_INPC
  sta REG
  lda #0
  sta REG+1
  sta REGB
  jmp  MAINP
;
EX_OUTC:
  jsr write_to_serial
  bra EX_OUTC_COMMON

EX_LCD_WRITE_CHR:
  jsr write_to_lcd

EX_OUTC_COMMON:
  jsr  PULTOP
  lda  REG
  jsr  COUT
  jsr write_to_serial
  jmp  MAIN
  ;

EX_LCDHOME:
  jsr lcd_home
  JMP MAIN


EX_LCDCLEAR:
  jsr lcd_clear_display
  JMP MAIN

EX_INS      =  *
  lda  (PCODE),Y   ; wanted length
  sta  TEMP
  inc  PCODE
  bne  EX_INS3
  inc  PCODE+1
EX_INS3     =  *
  jsr  GET_LINE
  lda  INBUF
  cmp  #KEY_ABORT     ; check for them aborting the run on input
  beq  ex_input_aborted
  jsr  CHK_KBD
  bcs  EX_INS3      ; we turned on/off tracing etc, ignore this line
  tya               ; length of line
  clc
  adc  #1        ; why?
  cmp  TEMP
  bcc  EX_INS1
  lda  TEMP      ; use max allowed length rather than received length
EX_INS1     =  *
  sta  TEMP+1    ; this is the length we are copying
  jsr  GETADR    ; get address to put the line
  ldy  #3        ; why?
  ldx  #0
;
;  copying loop - arrays are stored downwards, so we subtract from DATA each time
;
EX_INS2     =  *
  dec  DATA
  lda  DATA
  cmp  #$FF
  bne  EX_INS4
  dec  DATA+1
EX_INS4     =  *
  lda  INBUF,X
  sta  (DATA),Y
  inx
  dec  TEMP+1      ; count of bytes to copy
  bne  EX_INS2
  jmp  MAIN        ; stop now
  ;

execution_aborted_message asciiz "\nExecution aborted\n"

ex_input_aborted:
  lda #<execution_aborted_message
  ldx #>execution_aborted_message
  jmp NOTIM1
;
;
EX_ADRNC    =  *
  jsr  GETADR
EX_ADRNC2   =  *
  lda  DATA
  clc
  adc  #2
  sta  DATA
  bcc  EX_ADRN2
  inc  DATA+1
  bcs  EX_ADRN2
EX_ADRNN    =  *
  jsr  GETADR
EX_ADRN2    =  *
  lda  DATA
  sta  REG
  lda  DATA+1
  sta  REG+1
  jmp  MAINP
;
EX_ADRAN    =  *
  jsr  GETIDX
  jmp  EX_ADRN2
;
EX_ADRAC    =  *
   jsr  GETIDC
   jmp  EX_ADRNC2
;
;
;


;
;  DIGITALREAD (pin) pin:0 to 15; pushes 0 or 1
;
EX_DIGITALREAD = *
  jsr PULTOP  ; which pin
  jsr digitalread
  bne EX_DIGITALREAD_ONE
  jmp FALSE
EX_DIGITALREAD_ONE:
  jmp TRUE    ; done

;
;  LCDPOS (line, column)
;

EX_LCDPOS = *
  jsr PULTOP   ; x
  and #$0F     ; max 15
  sta hardware_work  ; save it
  jsr PULTOP   ; y
  and #1       ; can be 0 or 1
  beq EX_LCDPOS_1
  lda #$40
EX_LCDPOS_1:
  ora hardware_work  ; or in the column
  ora #$80     ; command to set the address
  jsr lcd_instruction
  jmp MAIN

;
;  RANDOM
;
EX_RANDOM:
  jsr gen_random
  lda random
  sta REG
  lda random+1
  sta REG+1
  lda random+2
  sta REGB
  jmp MAINP


;
;  LATENCY
;
EX_LATENCY:
  lda typing_latency
  sta REG
  lda typing_latency+1
  sta REG+1
  lda typing_latency+2
  sta REGB
  jmp MAINP

; PULTOP puts stuff here thus:
;
;        LDA  REG
;        LDX  REG+1
;        LDY  REGB

EX_DELAY = *
  JSR  PULTOP  ; get the time interval in ms
  ; (Y = high-order byte, X = lo-order byte)
  ldx REG
  lda REG+1
  and #$7F
  tay
  jsr delay
  jmp MAIN

;
;  RANDOMSEED
;
EX_RANDOMSEED = *
  JSR  PULTOP  ; get the seed
  sta random+1
  stx random+2
  sty random+3
  lda #$FF     ; ensure seed has some 1 bits
  sta random
  jmp MAIN

;
;  PINMODE (pin, mode) pin:0 to 15; mode: 0=read, 1=write
;
EX_PINMODE = *
  jsr PULTOP  ; the mode
  tax         ; input or output?
  phx         ; save the mode
  jsr PULTOP  ; which pin
  plx         ; get mode back
  jsr pinmode ; pin in A, mode in X
  jmp MAIN    ; done

;
;  DIGITALWRITE (pin, value) pin:0 to 15; value: 0 or 1
;
EX_DIGITALWRITE = *
  jsr PULTOP  ; the value
  tax         ;
  phx         ; save the value
  jsr PULTOP  ; which pin
  plx         ; get mode back
  jsr digitalwrite ; pin in A, value in X
  jmp MAIN    ; done

EX_LIB_CALL:
  JSR  GETLIT
  jmp  (REG)

;
;  Assert (expression)
;

EX_ASSERT:
  jsr PULTOP  ; the value
  lda REG
  ora REG+1
  ora REGB
  bne EX_ASSERT_OK
  lda #<assertion_failed_message  ; "Assertion failed"
  ldx #>assertion_failed_message
  jsr print
  jmp RUNERR

EX_ASSERT_OK:
  jmp MAIN
