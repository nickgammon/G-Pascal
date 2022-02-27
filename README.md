# G-Pascal and Assembler for Ben Eater's 6502 breadboard PC

## On-board assembler, tiny Pascal, and text editor for Ben Eater's board

Full documentation can be viewed at <https://gammon.com.au/G-Pascal>


### Includes:

* A **6502 assembler** which lets you try out your assembler programming without having to keep removing the EEPROM chip and programming it externally. The assembler supports:
    * The documented WD65C02 instruction set, with all operand modes
    * Full expression evaluation of operands, with operator precedence, parentheses, bitwise operations and so on.
    * Relocation of the output to any memory address (ORG directive)
    * Here is "hello world" in assembler:


        ```asm
              jmp begin   ; skip the message
        hello asciiz "Hello, world!"
        begin = *
              lda #<hello
              ldx #>hello
              jsr print
              rts
        ```

* A **"tiny" Pascal compiler** (G-Pascal) which allows you to program the board in a high-level language. Whilst there are limitations in what can be done in a few KB of memory, the compiler supports:
    * CONST, VAR, FUNCTION and PROCEDURE declarations
    * Local declarations (functions and variables within functions, etc.)
    * Recursive function and procedure calls
    * Arithmetic: multiply, divide, add, subtract, modulus
    * Logical operations: and, or, shift left, shift right, exclusive or
    * INTEGER and CHAR data types. (Integer are 3 bytes and thus range from 8388607 to -8388608)
    * Arrays
    * Interface with any memory address by using the MEM and MEMC constructs (to peek and poke memory locations)
    * Built-in functions to write to the LCD display.
    * Built-in functions to do pinMode, digitalRead and digitalWrite, similar to the Arduino. These interface with any available ports on the VIA chip.

    * Here is "hello world" for the LCD display:


        ```pas
        begin
          lcdclear;
          lcdwrite ("Hello, world!")
        end.
        ```

        And for the serial monitor:

        ```pas
        begin
          writeln ("Hello, world!")
        end.
        ```

* A **text editor** for keying in programs. It supports:
    * Loading and saving (via the RS232 interface)
    * Inserting and deleting lines
    * Find and replace

* **Support functions**, such as:
    * RS232 interface for connecting a "dumb" terminal, or a PC/Mac
    * Support for the LCD interface described in Ben's videos
    * Other support functions for use by your assembler code, such as multiplication and division, CRC calculations, binary-to-decimal conversion
    * Support for NMI (non-maskable interrupts) so that you can recover from runaway code
    * There is approximately 12k of space available on the EEPROM to add your own functions

Both the Pascal compiler and assembler are quite fast. Any program that will fit into memory will compile in a few seconds. For example, using a 1 MHz clock:

* A 335-line assembler program testing all opcodes assembles in 9 seconds
* A 651-line G-Pascal program (the Adventure game) compiles in 6 seconds

---

## How much does it cost?

* The software is open source
* There is no fee
* You do not need to register
* You do not need to provide any personal details such as email address
* The software can be downloaded from GitHub
* The source is licensed under the MIT License.


