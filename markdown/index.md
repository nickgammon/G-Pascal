% G-Pascal for Ben Eater's 6502 board

**Author**: Nick Gammon

**Date written**: February 2022

<div class='quick_link'> [G-Pascal info](pascal_compiler.htm)</div>
<div class='quick_link'> [Assembler info](assembler.htm)</div>
<div class='quick_link'> [Suggested hardware mods](hardware_mods.htm) </div>
<div class='quick_link'> [How to install](installation.htm)</div>
<div class='quick_link'> [Text editor](editor.htm) </div>
<div class='quick_link'> [File menu](file_menu.htm) </div>
<div class='quick_link'> [Adventure game](adventure.htm) </div>
<div class='quick_link'> [I2C support](i2c.htm) </div>
<div class='quick_link'> [More electronics](https://gammon.com.au/electronics) </div>

## On-board assembler, tiny Pascal, and text editor for [Ben Eater's board](https://eater.net/6502)

Demonstration video [here on Vimeo](https://vimeo.com/682663375).

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

    * Detailed notes about the assembler [here](assembler.htm).

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

    * Detailed notes about the compiler [here](pascal_compiler.htm).



* A **text editor** for keying in programs. It supports:
    * Loading and saving (via the RS232 interface)
    * Inserting and deleting lines
    * Find and replace
    * Detailed notes about the [editor](editor.htm) and [how to load and save source](file_menu.htm).

    ```
    Available actions:

    List/SAve   line_number_range
    Delete      line_number_range
    Insert/LOad after_line
    Find        line_number_range /target/flags
    Replace     line_number_range /target/replacement/flags

    Help
    INfo
    Memory      first_address last_address
    Compile/Syntax/Assemble
    RUn/DEBug/Trace
    RECover
    (Actions may be abbreviated)
    (Flags: 'I'gnore case, 'G'lobal, 'Q'uiet)
    ```

* **Support functions**, such as:
    * RS232 interface for connecting a "dumb" terminal, or a PC/Mac
    * Support for the LCD interface described in Ben's videos
    * Other support functions for use by your assembler code, such as multiplication and division, CRC calculations, binary-to-decimal conversion
    * Support for NMI (non-maskable interrupts) so that you can recover from runaway code
    * Support for I^2^C communication for connecting to real-time clocks, port expanders, etc.
    * There is over 11k of space available on the EEPROM to add your own functions

Both the Pascal compiler and assembler are quite fast. Any program that will fit into memory will compile in a few seconds. For example, using a 1 MHz clock:

* A 335-line assembler program testing all opcodes assembles in 9 seconds
* A 651-line G-Pascal program (the Adventure game) compiles in 6 seconds

---

## How much does it cost?

* The software is open source
* There is no fee
* You do not need to register
* You do not need to provide any personal details such as email address
* The software can be [downloaded from GitHub](https://github.com/nickgammon/G-Pascal)
* The license (MIT License) is [here](doc/license.txt)

---

## Hardware modifications suggested/required


![](images/Board photo.png)

* RS232 interface (required)

    To interface with the compiler, text editor and assembler you clearly need some way to "talk" to Ben's 6502 board. The on-board code supports interfacing with a serial interface running at 4800 baud. To do this you will need to use an FTDI cable, available on eBay for around $US 10. The wiring for it is [here](hardware_mods.htm#rs232_interface).

* An NMI interrupt push-button (suggested)

    It helps to recover from runaway or rogue code by pressing a switch button connected to the NMI pin of the 65C02. This does a "warm restart" which basically starts the code again, however it retains your existing source code. The wiring for it is [here](hardware_mods.htm#nmi_interrupt).

* An enhancement that stops you attempting to write to EEPROM (suggested)

    The way Ben described the address decoding for the EEPROM, it is possible for both the CPU and the EEPROM to be attempting to write to the bus at the same time (eg. STA $8000). The suggested wiring uses a spare NAND gate on the existing board to prevent that. This would prevent possible damage to the CPU and/or the EEPROM if such code was attempted. The wiring for it is [here](hardware_mods.htm#protect_EEPROM).

* An enhancement to give yourself access to more of the on-board RAM (suggested)

    The way Ben did his address decoding for the RAM meant that only 16k of the chip's 32k was available. You can make a simple change using a 74LS08 "AND" gate that extends the RAM address space to 0x6000 rather than 0x4000, thus giving you 24k of RAM (an extra 8k) for the cost of a one dollar chip and a few jumper cables. This change is required to compile the "Adventure" game because of its size. The wiring for it is [here](hardware_mods.htm#increased_ram).

* Put in a faster oscillator

    To get a nice speed boost, simply replace the 1 MHz oscillator with 2 MHz or 4 MHz. You will get a corresponding increase in speed. Since the serial communications assume a 1 MHz oscillator, simply increase the baud rate that you use to communicate with the board to match. In other words, with a 2 MHz oscillator use 9600 baud, and with a 4 MHz oscillator use 19200 baud. I haven't tried faster clock speeds yet.

* You can disconnect, if you wish, the four low-order data bits from the HD44780U LCD screen to the VIA. (That is, VIA pins 10, 11, 12 and 13). This is because the LCD code has been rewritten to use 4-bit mode, thus freeing up those pins.

---

## More VIA pins are available

I rewrote the LCD code to use 4-bit mode on the HD44780U LCD display. This frees up 4 fairly valuable pins on the VIA (versatile interface adapter) chip for other uses, such as developing an SPI interface.

Pins PA0 and PA1 on the VIA are used for the serial interface, as well as CB2 to detect the start bit for incoming serial data.

That leaves 7 pins for your own use (PA2, PA3, PA4, PB0, PB1, PB2, PB3).

---

## Example programs

As an example of what can be done, there is a (roughly) 700-line "Adventure" program available [here](examples/pas/adventure.pas) for compiling. That demonstrates a lot of the features of the compiler and is a fun game to try out. Note that running this requires a small hardware modification to Ben's board to make more of the RAM available, as described [here](hardware_mods.htm#increased_ram). This program was originally written in 1982 for the Apple II version of G-Pascal, and has been modified slightly to conform to the syntax of this compiler.

Other example programs are [prime number generation](examples/pas/prime_numbers.pas), [square roots calculation](examples/pas/sqrt.pas), and a [guessing game](examples/pas/guessing_game.pas).

Or, you can play with assembler coding and make LEDs blink. :)

Example code [here](examples/).

The code is released under an extremely liberal license (MIT License), which basically means you can do anything at all you like with it, except claiming you wrote it yourself. ;)

---

## Ben Eater's 6502 board

Ben does a fabulous series of YouTube videos about wiring together this 6502 breadboard computer. The first in the series is [here](https://www.youtube.com/watch?v=LnzuMJLZRdU).

It really is extremely interesting and educational. I strongly recommend you watch the video series and consider supporting [Ben's Patreon](https://www.patreon.com/beneater).

You can buy a [kit from Ben](https://eater.net/6502). Ben has put a lot of work into making these educational videos --- I suggest you support him and his future endeavours by buying his kits from him.

---

## Support for other 6502 boards

There is no particular reason this code wouldn't run on other 6502 set-ups. You would need to be aware of the hardware addresses of things like your RAM (which presumably starts at 0x0000 because you need access to the 6502 hardware stack at 0x0100) and your ROM (which presumably covers 0xF000 to 0xFFFF because of the need to have NMI/Reset/IRQ vectors).

You may need to modify the way the code interfaces with the hardware, depending on your personal setup.

---

## Other links

* [Suggested hardware modifications](hardware_mods.htm) to Ben's 6502 board including accessing more of the on-board RAM, and connecting up the board to your PC/Mac
* [G-Pascal compiler features](pascal_compiler.htm) and limitations
* [Assembler features](assembler.htm) and limitations
* [Example code](examples/)
* [Text editor](editor.htm)
* [File menu](file_menu.htm)
* [How to install](installation.htm)
* [History of G-Pascal](history.htm) and the assembler
* [Ben Eater's 6502 board](https://eater.net/6502)
* [Ben Eater's first 6502 board tutorial on YouTube](https://www.youtube.com/watch?v=LnzuMJLZRdU)

---

## License

Information and images on this site are licensed under the [Creative Commons Attribution 3.0 Australia License](https://creativecommons.org/licenses/by/3.0/au/) unless stated otherwise.

Source code licensed under the [MIT License](doc/license.txt).
