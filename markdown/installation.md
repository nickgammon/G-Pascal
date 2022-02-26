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

## Burn EEPROM

Install gpascal.bin onto your EEPROM as follows:

```
minipro -p AT28C256 -w gpascal.bin
```

Minipro is available from [https://gitlab.com/DavidGriffith/minipro](https://gitlab.com/DavidGriffith/minipro).


---

<div class='quick_link'> [Back to main G-Pascal page](index.htm)</div>
<div class='quick_link'> [Suggested hardware mods](hardware_mods.htm) </div>

---

## License

Information and images on this site are licensed under the [Creative Commons Attribution 3.0 Australia License](https://creativecommons.org/licenses/by/3.0/au/) unless stated otherwise.

Source code licensed under the [MIT License](doc/license.txt).
