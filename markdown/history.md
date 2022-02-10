# What is G-Pascal?

In 1979 I bought a Motorola 6800 evaluation kit, which involved a couple of printed circuit boards and a heap of components. I soldered it together and got it working. The "PC" had 512 bytes of RAM, and programs had to be keyed in through a small keyboard in hex (that is, raw machine code).

Interesting though this was, it was very tedious hand-assembling programs and keying them in, byte by byte. After I expanded the computer with a RAM board (from Pennywise Peripherals) I had the idea of writing an assembler, to simplify writing code. My first version of this assembler had to be hand-coded in hex machine-code. I found that the 6800 instruction set had a lot of consistency about it. For example, the top 4 bits of an instruction would be the addressing mode, and the bottom 4 bits the opcode. For example with LDA (load accumulator):

*  LDA (A immediate): $86
*  LDA (A direct):    $96
*  LDA (A extended):  $B6
*  LDA (A indirect):  $A6

... and so on. So you can see that LDA always was in the format $x6 where x was the addressing mode and the 6 was for LDA.

Meanwhile SBC (subtract with carry) looked like this:

*  SBC (A immediate): $82
*  SBC (A direct):    $92
*  SBC (A extended):  $B2
*  SBC (A indirect):  $A2

... and so on. So again, the "2" at the end was the operation (subtract with carry) and the first four bits were the addressing mode. From that you could deduce that any of the instructions (which used this format) had addressing modes like this:

*  A immediate: $8x
*  A direct:    $9x
*  A extended:  $Bx
*  A indirect:  $Ax

This made writing the assembler comparatively easy, because you basically worked out the addressing mode, which gave you 4 bits of the opcode, and the instruction name, which gave you the other 4 bits. Combining those gave you a complete opcode. There were some exceptions, of course, such as the "push" and "pull" instructions which didn't have an addressing mode.

Once this was finished I bought an EEPROM expansion board (also from Pennywise Peripherals) and burnt the assembler onto the EEPROM chips, giving me an on-board assembler that was available as soon as you powered up the board.

Once I had an assembler I could keep enhancing it by writing new versions of the assembler in the earlier version. The EEPROM board had enough capacity that there was room for the old version (on one half) and the new, improved, version (on the other half). Thus I was able to keep "bootstrapping" the assembler into a newer and better version.

## Tiny Pascal

The compiler was originally written in 1979 for the Motorola 6800 after reading a series of articles in Byte magazine (September, October, and November in 1978) written by Kin-Man Chung and Herbert Yuen. That article described making a "Tiny" Pascal compiler written in Basic. That particular version was burnt onto UV-eraseable PROM, and kept on-board to my "personal" computer, along with a text editor and code to load/save programs.

After I purchased an Apple II, I ported the compiler to the 6502 processor, and released a version in April 1982.



[Back to main G-Pascal page](index.htm)


