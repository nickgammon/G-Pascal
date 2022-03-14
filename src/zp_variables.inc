;***********************************************
;
; Zero-page variables (currently 64 (decimal) bytes are free for your use).
;  I suggest using the high ones and working down in case there is a change to the code.
;  For example, start with 0xFF, then 0xFE and so on.
;
;***********************************************

  .dsect


;
;  General work-areas for arithmetic routines (eg. add, subtract, divide, multiply)
;   3-byte signed numbers: -8388608 to +8388607
;
VALUE    reserve 3    ; the current value of the token (ie. if a number)
VALUE2   reserve 3    ; used by the assembler expression evaluator

; SRCE and DEST used in compiler and editor, particularly for string comparisons
;   (re-using VALUE and VALUE2)

SRCE     =  VALUE
DEST     =  VALUE2

REMAIN   reserve 3    ; division remainder

;
;  work "register" - re-using VALUE and VALUE2 again
;
REG   = VALUE
REGB  = VALUE+2
REG2  = VALUE2
REG2B = VALUE2+2

;
;  re-using above for CRC-16 calculations
;
crc_addr = VALUE      ; the address to take a CRC of (2 bytes)
crc_num  = VALUE2     ; the number of bytes to take a CRC of (2 bytes)
crc_val  = REMAIN     ; the current CRC value (2 bytes)

;
;  current random number - change this to reseed it
;
random         reserve 4 ; 32-bit random number
typing_latency reserve 3 ; incremented while waiting for input

;
;  Used for seeding registers when calling machine code from Pascal
;
call_a reserve 1      ; used when doing a machine code call:  A register
call_x reserve 1      ;  ditto: X register
call_y reserve 1      ;  ditto: Y register
call_p reserve 1      ;  ditto: status register
call_s reserve 1      ; stack register, used by BRK
brk_address reserve 2 ; BRK address

;
;  address of write function (2 bytes)
;
;  This is called by "character out" function COUT. It writes the character in "A" to
;   serial port or LCD. You could conceivably put another function address here if
;   you wanted to capture output (eg. from calling DISP_BIN) and putting it somewhere else
;   like into a series of memory locations, or to output to SPI.
;  The functions write_to_serial and write_to_lcd
;   put the appropriate outputting functions into this location.
;
write_function reserve 2  ; address of function to write to serial or LCD


LINE_CNT  reserve 2   ; line counter during compile
LINE_NO  =  LINE_CNT

WX       reserve 2    ; 2-byte address used during compilation (reserved word lookup)
LIST     reserve 1    ; 0 = no listing, 1 = list source during compile
DIGIT    reserve 1    ; used only in GET_NUM to process a decimal number
SIGN     reserve 1    ; used in compiling and inputting a number - is there a minus sign?

FRAME    reserve 2    ; stack frame number - not totally certain but it used for procedure calls at runtime
LEVEL    reserve 1    ; current nested procedure/function level (how far we are nested)
PCODE    reserve 2    ; current P-code address (increments during compile and running)
PNTR     =  PCODE     ; 2-byte work pointer used in editor (eg. in find/replace etc.)
ACT_PCDA reserve 2    ; start of P-codes (after source) - does not increment
DISPL    reserve 2    ; level? ahhh ... I don't know
OFFSET   reserve 2    ; variable offset? maybe just a work area
OPND     reserve 3    ; jump operand ... not sure
DCODE    reserve 1     ; 0 = no display, 1 = display P-codes during compile
STARTSYM reserve 2    ; start of symbol table
ENDSYM   reserve 2    ; end of symbol table - used for reverse searching symbols
T        =  ENDSYM     ; stack pointer 2 bytes
WORKD    reserve 2    ; work area for holding P-code address
ERRNO    reserve 1    ; current error number, if ERROR called
BSAVE    reserve 1    ; seems to be used to save "A" register, lol
WORK     reserve 2    ; work area, used for pushing 2-bytes onto the (processor) stack
PRCITM   reserve 2    ; seems to hold the current procedure/function symbol
BASE     reserve 2    ; the base interpreter stack frame (for this procedure)
TO       =  BASE      ; Used in editor (from/to list ranges, and copying text)
DATA     reserve 2    ; 2-byte address work-area for calculating the address of a variable at runtime
RUNNING  reserve 1    ; 0x40 - in editor, 0x80 - executing - affects stuff like numeric conversion
SYMITM   reserve 2    ; current symbol table item (found by searching)
FROM     =  SYMITM    ; Used in editor (from/to list ranges, and copying text)
SYNTAX   reserve 1    ; 0 = full compile, 1 = syntax-only compile (no writing to memory)
END_PCD  reserve 2    ; where the P-codes ended after a compile
TEMP     reserve 2    ; temporary work area?
CALL     reserve 2    ; the procedure address we are calling
DBGFLG   reserve 1    ; 0 = not debugging, 1 = debugging, $80 = tracing
DATTYP   reserve 1    ; 0 for integers and 1 for characters
COUNT1   reserve 1    ; work counter
COUNT2   reserve 1    ; and another
LASTP    reserve 2    ; 2-byte last P-code address executed at runtime (start of this P-code)
IO_A     reserve 1    ; dunno
IO_Y     reserve 1    ; seems to be used to save and restore Y
IO_X     reserve 1    ;  ditto for X
RES      reserve 3    ; multiplication result
DIVISOR  reserve 3      ; divisor for division
DVDN     reserve 3    ; dividend
RMNDR    reserve 1    ; for division, 0 if positive, 0x80 if negative
bcd_work    reserve 4    ; work area for converting binary to decimal (packed)
bcd_result  reserve 8    ; work area for turning numbers into printable (unpacked)
spi_mode   reserve 1   ; SPI mode: 0 to 3
QUOT_SYM reserve 1     ; current quote symbol
QT_SIZE  reserve 1    ; number of characters in reserved words
ASS_OPERAND reserve 1 ; assembler operand type = see defines below
OPCODE   reserve 2    ; address of opcode in assembler
OPCODE_LEN reserve 1  ; length of opcode
ASS_EMIT_COUNT reserve 1 ; number of bytes emitted on this line
ASS_OPCODE_WORK reserve 2; for opcode lookups
ASS_COUNT reserve 1 ; for counting opcode matches
ASS_VALUE reserve 3 ; the assembler operand (address or immediate) will be here
ASS_PASS  reserve 1 ; which assembler pass we are up to
ass_emit_bytes reserve 2 ; how many bytes the assembler emitted altogether
exp_value_stack reserve 2  ; address of assembler value stack
exp_value_stack_count reserve 1 ; how far through value stack we are (0 = empty)
exp_operator_stack_count reserve 1 ;  how far through operator stack we are (0 = empty)
exp_operator reserve 1  ; current operator for expression evaluation
exp_operator_precedence reserve 1 ; current operator precedence
exp_evaluation_function reserve 2 ; function to evaluate an expression
exp_unary_ok reserve 1  ; true if the next token can be a unary operator (like: - < >)
show_symbols reserve 1  ; 4 = list the symbol table after an assemble, 8 = include library symbols
ass_current_label reserve 2 ; current symbol table address of the label on this line
str_work reserve 1      ; work byte for string compares
  .if LCD_SUPPORT
lcd_work  reserve 1 ; used by the LCD routines
  .endif

;
;  serial work
;
serial_out_byte reserve 2 ; current byte we are sending
serial_out_bit  reserve 1 ; current bit count
serial_in_byte  reserve 1 ; current byte we are receiving
serial_in_byte_received  reserve 1 ; last received byte

;
; hardware
;
hardware_work  reserve 1  ; work for use during interpreting

;
; maths
;
maths_work  reserve 1 ; work for multiply/divide
;
;  for get_token
;
token_start      reserve 2  ; where to start looking for a token
token_address    reserve 2  ; where the token actually started
token_length     reserve 1  ; length of the token
token_type       reserve 1  ; what type of token it is
token_value      reserve 3  ; the value of numeric tokens
token_sign       reserve 1  ; non-zero if negative value
token_work       reserve 3  ; temporary work area - also used by handlerLookup
token_digit      reserve 1  ; used by get_token
token_line_start reserve 2  ; where the current line starts

;
;  for editor
;
from_line       reserve 2     ; list, delete, modify: FROM line number
to_line         reserve 2     ; list, delete, modify: TO line number
current_line    reserve 2     ; current editor line number
insert_limit    reserve 2     ; maximum we can input to
mem_move_src    reserve 2     ; for copying memory - source of move
mem_move_dest   reserve 2     ; for copying memory - destination of move
mem_move_len    reserve 2     ; for copying memory - length of move
this_line       reserve 2     ; which line we are searching for
this_line_addr  reserve 2     ; address of the "this" line
find_from       reserve 1     ; editor find: from offset in INBUF (start of target string)
find_to         reserve 1     ; editor find: to offset in INBUF (end of target string)
find_pos        reserve 1     ; which offset in the current line we are currently searching from
find_len        reserve 1     ; the length of the find text
find_delimiter  reserve 1     ; the find/replace delimiter
rep_from        reserve 1     ; editor replace: start of replacement string
rep_to          reserve 1     ; editor replace: end of replacement string
rep_diff        reserve 2     ; difference in find/replace string lengths (signed 2 bytes)
find_count      reserve 2     ; count of find/replace
found_this_line reserve 1     ; did we find something on this line?
insert_last_char = find_from  ; the last character we inserted in INSERT/LOAD mode

editor_flags    reserve 1     ; various flags, see below

EFLAG_SHOW_LINE        = %00000001    ; set after receiving a newline, so we display the next line
EFLAG_ALLOW_ZERO_FROM  = %00000010    ; for use with MEMORY, allow them to start from $0000
EFLAG_LOAD             = %00000100    ; doing a LOAD, do not show line numbers when inserting
EFLAG_SAVE             = %00001000    ; doing a SAVE, do not show line numbers
EFLAG_FIND             = %00010000    ; doing a FIND
EFLAG_REPLACE          = %00100000    ; doing a REPLACE
EFLAG_SUPPRESS_NEWLINE = %01000000    ; don't show newlines
EFLAG_NO_LINE_NUMBERS  = %10000000    ; don't show line numbers

find_flags     reserve 1      ; find/replace flags, see below

FFLAG_IGNORE_CASE = %00000001
FFLAG_GLOBAL      = %00000010
FFLAG_QUIET       = %00000100

;
;  system flags
;

system_flags    reserve 1    ; general flags

FLAG_COMPILING      = %00000001
FLAG_ASSEMBLING     = %00000010
FLAG_LIST_SOURCE    = %00000100
FLAG_VALID_COMPILE  = %00001000
FLAG_VALID_ASSEMBLE = %00010000
FLAG_RUNNING        = %00100000
FLAG_ONLY_ALPHA     = %01000000   ; for editor, so you can L5 to do LIST 5
FLAG_BRK_REACHED    = %10000000   ; Breakpoint encountered during run
