;***********************************************
; G-PASCAL COMPILER
; for Ben Eater's breadboard computer
;
; Author: Nick Gammon
; Date: 20 January 2022
;
; To compile:
;
;  vasm6502_oldstyle gpascal.asm -wdc02 -esc -Fbin -o gpascal.bin -dotdir -L gpascal.list
;
; To program EEPROM:
;
;  minipro -p AT28C256 -w gpascal.bin
;
;
;
; To communicate, I suggest: miniterm /dev/ttyUSB0 4800 -e
;
;   Miniterm: Ctrl+]        : to exit.
;             Ctrl+T Ctrl+H : help
;             Ctrl+T Ctrl+E : toggle local echo
;
;
;  Copyright 2022 by Nick Gammon
;
;  Permission is hereby granted, free of charge, to any person obtaining a copy
;  of this software and associated documentation files (the "Software"), to deal
;  in the Software without restriction, including without limitation the rights
;  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
;  copies of the Software, and to permit persons to whom the Software is
;  furnished to do so, subject to the following conditions:
;
;  The above copyright notice and this permission notice shall be included in all
;  copies or substantial portions of the Software.
;
;  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;  SOFTWARE.
;
;***********************************************
;

; Note: BCC == BLT and BCS == BGE
;

;
;  CONDITIONAL COMPILES

EMULATOR = 0          ; for testing on a PC running an emulator
LCD_SUPPORT = 1       ; 1 = support LCD, 0 = not. Unset if you have removed the LCD.
USE_CP437_FONT = 1    ; 1 = include the symbols for the CP437 font for use with MAX7219 chip, 0 = omit them
USE_PASCAL = 1        ; 1 = include the G-Pascal compiler, 0 = omit it
USE_ASSEMBLER = 1     ; 1 = include the assembler, 0 = omit it

SERIAL_DEBUGGING = 0  ; if set, toggle VIA PA2 when reading a bit, and PA3 when writing a bit
                      ;  DO NOT USE I2C if this is on, as I2C functions use these two pins


;
;  CONFIGURATION
;
CLOCK_RATE   = 1000000   ; 1 Mhz
START_OF_ROM = $8000     ; where the ROM chip starts
HIGHEST_RAM  = $3FFF      ; original board hardware
;HIGHEST_RAM  = $5FFF    ; with suggested additional AND gate

RUNNING_STACK_TOP = $CF   ; top of stack when running assembler code

;
;  serial output
;
BAUD_RATE    = 4800         ; baud
BIT_INTERVAL = CLOCK_RATE / BAUD_RATE ; time between bits in µs

;
;  serial input - input commences on the interrupt generated by the falling edge
;    of the start bit. Bits are then clocked in with a timed loop.
;  - these delays can be tweaked if you believe the serial input is not being
;    sampled at the right moment, confirm by turning SERIAL_DEBUGGING on
;    and checking the debugging pulses compared to the middle of the bit times
;    with an oscilloscope or logic analyser
;
SERIAL_DELAY1 = 48    ; initial delay (count) - higher because it is 1.5 bit times
SERIAL_DELAY2 = 35    ; subsequent delays between bits (count)

SYMBOL_TABLE_START = HIGHEST_RAM  ; symbol table pointer (ENDSYM) is decremented before being used

STACK            =  $100  ; hardware stack address
SPACE            =  $20   ; uh-huh
SINGLE_QUOTE     =  $27
NL               =  $0A   ; newline
CR               =  $0D   ; carriage-return
BACKSPACE        =  $08   ; backspace

MAX_STK          =  32
NEW_STK          =  $FF

    .if EMULATOR
END_EDITOR_INPUT =  '`'   ; backtick terminates input
    .else
END_EDITOR_INPUT =  $1B   ; Esc terminates editor input
    .endif

KEY_DEBUG      = 'D'-$40   ; Ctrl+D start debugging
KEY_TRACE      = 'T'-$40   ; Ctrl+T start tracing
KEY_STOP_TRACE = 'N'-$40   ; Ctrl+N stop tracing
KEY_ABORT      = 'C'-$40   ; Ctrl+C abort execution


  .if LCD_SUPPORT
;
;  Pins connected from the VIA to the LCD
;
LCD_RS             = %00100000  ; PA 5 - 0 = instruction, 1 = data
LCD_RW             = %01000000  ; PA 6 - 0 = write, 1 = read
LCD_E              = %10000000  ; PA 7 - toggle (0 -> 1 -> 0) to clock out a command or data
  .endif

  .include "zp_variables.inc"

;
;  A general work-buffer is allocated from $200 to $2FF for use during keyboard input, and also
;  for converting strings during tokenisation (eg. two quotes in a row to one quote, and handling
;  string escape sequences). It is also used during assembly as a 256-byte operator stack.
;
  .org $200
INBUF_SIZE = 256          ; we use this for string storage during tokenisation
INBUF  reserve INBUF_SIZE ; and also for preprocessing string literals (eg. backslash expansion)
TEXT_START = *            ; where source goes in memory (currently $300)
  .dend



;------------------------------------------
;  Macros
;------------------------------------------

 .macro tknjmpItem ; tknjmp entry: token, handler
   dfb   \1
   word  \2
 .endmacro

 .macro makeHandler ; table of words and handlers: word, handler
   asciiz   \1
   word  \2
 .endmacro

;
; makePasLibraryFunction NAME (SYMNAM), TYPE (SYMTYP), ARG_COUNT (SYMARG), EXECUTION_ADDRESS (SYMDSP)
;
;   TYPE = SYMBOL_LIBRARY_PROCEDURE or SYMBOL_LIBRARY_FUNCTION
   .macro makePasLibraryFunction
     asciiz   \1
     dfb      \2
     dfb      \3
     word     \4
   .endmacro

;
; makeAsmLibraryFunction NAME (SYMNAM), EXECUTION_ADDRESS (SYMDSP)
;
   .macro makeAsmLibraryFunction
     asciiz   \1
     word     \2
   .endmacro

  ORG  START_OF_ROM   ; normally $8000 in the case of Ben Eater's board

  JMP  START   ; where RESET takes us - a cold start
  JMP  RESTART ; where NMI takes us - a warm start

;***********************************************
; INCLUDES
;***********************************************

  .include "memory.inc"
  .include "editor.inc"
  .include "utilities.inc"
  .include "errors.inc"
  .if USE_ASSEMBLER
  .include "assembler.inc"
  .endif    ; USE_ASSEMBLER

  .include "math.inc"

  .if USE_PASCAL
  .include "compiler.inc"
  .include "interpreter.inc"
  .endif  ; USE_PASCAL

  .include "interrupts.inc"
  .include "lcd.inc"
  .include "symbols.inc"
  .include "hardware.inc"
  .include "gtoken.inc"
  .include "i2c.inc"
  .include "spi.inc"

  .if USE_CP437_FONT
    .include "cp437_font.inc"
  .endif

introduction asc    "G-Pascal compiler, version 4.06.\n"
             asciiz "Written by Nick Gammon.\nType H for help.\n"

  .if LCD_SUPPORT
LCD_welcome asciiz "Nick's G-Pascal\nCompiler v4.06"
  .endif

running_message   asciiz  'Running\n'

;
; here for cold start - clear text file to null etc. etc.
;
START    =  *
  cld             ; cancel decimal mode
  sei             ; no interrupts yet
  lda #$FF
  sta random      ; initialise random numbers
  sta random+1
  sta random+2
  sta random+3
;
;  Put 0x00 at start of source
;
  lda  #<TEXT_START
  sta  REG
  lda  #>TEXT_START
  sta  REG+1
  lda  #0
  tay
  sta  (REG),Y     ; null edit file
  sty  system_flags
  tax
;
;  now do rest of initialization
;
RESTART  =  *
  ldx  #NEW_STK
  txs             ; reset stack
  cli             ; allow interrupts after a NMI
  cld             ; cancel decimal mode
  jsr hardware_init

;
REST1    =  *
  .if LCD_SUPPORT
    ;
    ;  put message on the LCD screen to prove it is working
    ;
    lda  #<LCD_welcome  ; G-pascal compiler
    ldx  #>LCD_welcome
    jsr lcd_print_message
  .endif

;
;  now direct output to the serial port
;
  jsr  write_to_serial   ; set up outputting function
  lda  #0
  sta  RUNNING
  jmp  main_start    ; go to "shell"

end_of_rom_routines dfb 0

;
;  processor hardware vectors
;
  .org $FFFA
  .word RESTART   ; non-maskable interrupt (NMI)
  .word START     ; reset vector
  .word irq       ; maskable interrupt (IRQ)
