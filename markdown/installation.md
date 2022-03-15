# Installation

<div class='quick_link'> [Back to main G-Pascal page](index.htm)</div>
<div class='quick_link'> [Suggested hardware mods](hardware_mods.htm) </div>

---

## Hardware modifications

You need to make the [suggested hardware modifications](hardware_mods.htm) --- or at least the one for the serial interface --- in order to communicate with the G-Pascal system.



---

## Assemble source


The source to the G-Pascal system is [here](/src).

If you have installed the hardware mod to give you more memory, change the line in the source:

```
HIGHEST_RAM = $3FFF    ; original board hardware
```

to be:

```
HIGHEST_RAM = $5FFF    ; with suggested additional AND gate
```


You can assemble it yourself by downloading **Vasm** from <http://sun.hasenbraten.de/vasm/>:

```
vasm6502_oldstyle gpascal.asm -wdc02 -esc -Fbin -o gpascal.bin -dotdir -L gpascal.list
```

Or, just use the gpascal.bin file already in that directory.

---

## Assemble options

There are some conditional assembly options near the start of gpascal.asm:

```
LCD_SUPPORT = 1       ; 1 = support LCD, 0 = not. Unset if you have removed the LCD.
USE_CP437_FONT = 1    ; 1 = include the symbols for the CP437 font for use with MAX7219 chip, 0 = omit them
USE_PASCAL = 1        ; 1 = include the G-Pascal compiler, 0 = omit it
USE_ASSEMBLER = 1     ; 1 = include the assembler, 0 = omit it
```

If you remove LCD support, CP437 support and choose either USE_PASCAL or USE_ASSEMBLER then the assembled code will fit into 16k, which would be useful if you are assembling it for a 6502 system with only 16k of EEPROM. So, you could have a system that either runs Pascal, or Assembler. Maybe burn two chips so you can insert the one for the job at hand today. :)

In that case you would also have to change:

```
START_OF_ROM = $8000     ; where the ROM chip starts
```

to:

```
START_OF_ROM = $C000     ; where the ROM chip starts
```

---

### Installing Vasm

(Instructions for Linux, should work the same on the Mac, perhaps install Cygwin or MS-VSC++ for Windows)

1. Download vasm from <http://sun.hasenbraten.de/vasm/release/vasm.tar.gz>

2. Unzip the archive, eg.

    ```
    tar xzf vasm.tar.gz
    ```

3. Change to the vasm directory:

    ```
    cd vasm
    ```

4. Build the source:

    ```
    make CPU=6502 SYNTAX=oldstyle
    ```

    If you are not using Linux see [Vasm Compilation Instructions](http://sun.hasenbraten.de/vasm/index.php?view=compile).
    You may need to replace the word "make" with "make -f Makefile.Cygwin" or "make -f Makefile.Win32".

5. Copy the executable somewhere handy if you wish. For example:

    ```
    sudo cp vasm6502_oldstyle /usr/local/bin/
    sudo chmod o+rx /usr/local/bin/vasm6502_oldstyle
    ```


---

## Burn EEPROM

Install gpascal.bin onto your EEPROM as follows:

```
minipro -p AT28C256 -w gpascal.bin
```

Minipro is available from [https://gitlab.com/DavidGriffith/minipro](https://gitlab.com/DavidGriffith/minipro).

Alternatively use your own programmer and programming utility and follow the instructions you used to put the code onto the EEPROM when building Ben's Board.

---

<div class='quick_link'> [Back to main G-Pascal page](index.htm)</div>
<div class='quick_link'> [Suggested hardware mods](hardware_mods.htm) </div>

---

## License

Information and images on this site are licensed under the [Creative Commons Attribution 3.0 Australia License](https://creativecommons.org/licenses/by/3.0/au/) unless stated otherwise.

Source code licensed under the [MIT License](doc/license.txt).
