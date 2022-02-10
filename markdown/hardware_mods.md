# Ben Eater's board suggested hardware modifications


## Contents

* [RS232 Interface](#rs232_interface)
* [Protect from writing to ROM](#protect_EEPROM)
* [NMI interrupt](#nmi_interrupt)
* [Increased RAM](#increased_ram)

**Author**: Nick Gammon (written in 2022)

[Back to main G-Pascal page](index.htm)

---

## RS232 Interface (required) {#rs232_interface}

In order to "talk" to Ben's 6502 Board you need some way of interfacing a keyboard and monitor. You could conceivably use a "dumb terminal" but they may be hard to find these days. The simplest thing is to obtain a FTDI cable, available from eBay for around $US 10.

![](images/FTDI cable.jpg)

One end plugs into a USB port on your PC/Mac/Linux box, and the other end breaks out the six RS232 signals:

1. Ground --- usually a **black** cable
2. CTS (clear to send)
3. Vcc (+5V)
4. TxD (transmit data - PC to board)
5. RxD (receive data - board to PC)
6. RTS (request to send) --- usually a **green** cable

![](images/FTDI connection.png)

The order shown in the photos is from "above" the connector, in other words you cannot see the metal pins inside the connector (they are visible on the "bottom" side).

![](images/FTDI schematic.png)

* Connect FTDI pin 1 (Ground - black) to Ground
* Connect FTDI pin 2 (CTS) to ground
* Connect FTDI pin 3 (VCC) to the +5V rail if you want to power the board from it
* Connect FTDI pin 4 (FTDI -> VIA) to 65C22 (U5) PA0 (pin 2) via a 1k resistor (receiving data)
* Connect FTDI pin 5 (VIA -> FTDI) to 65C22 (U5) PA1 (pin 3) (sending data)
* Connect FTDI pin 6 (RTS - green) via a 0.1µF capacitor for automatic reset, if you are powering from the FTDI

### Interrupt detection

In order for the code to detect the start of a byte, we need to generate an interrupt on the falling edge of the received data. Thus also:

* Connect 65C22 (U5) pin 2 to CB2 (pin 19)

![](images/Serial connections.png)


### Caveats regarding the RS232 interface

* The baud rate is fixed at 4800 baud (assuming your 6502 board is clocked at 1 MHz as it was in Ben's kit).
* If you choose a faster clock adjust the baud rate on your Mac/PC program accordingly. For example:
    * If the board is clocked at 2 MHz the baud rate will be 9600
    * If the board is clocked at 4 MHz the baud rate will be 19200
* The serial read/write software is somewhat timing and interrupt dependent. If you type while it is sending then data **will** be corrupted. I advise you to not type anything while the code is sending data to your PC.


### Program to "talk" to the serial interface.

I found **miniterm** very good on my Linux box. It appears to be available from <https://pyserial.readthedocs.io/en/latest/index.html>.

Miniterm is available on Mac, Windows and Linux. See <https://pypi.org/project/pyserial/#files>

It requires Python 3.x to run. You may need to install that as well.

When I was testing G-Pascal I used this command:

```bash
miniterm /dev/ttyUSB0 4800 -e
```

The port (in my case "/dev/ttyUSB0") may be different. If you aren't sure what it is, try opening the Arduino IDE and looking at the Tools Menu -> Port, like this:

![](images/Serial device name.png)


The "-e" command-line option on Miniterm activates "local echo". You need this because the board will not echo back what you type (and if it did, it would be corrupted as described above).

Type Ctrl+] to exit from Miniterm.

### Warning about dumb terminals

Whilst you can use a dumb terminal that has a RS232 interface, you need to be aware the the electrical signals are not compatible with the 6502 processor. The original RS232 specs had a "1" bit as -12V and a "0" bit as +12V. You would need to use a conversion cable, or make up something, for example with the MAX232 chip, which will do the conversion.

---

## Protect from writing to ROM (suggested) {#protect_EEPROM}

The way Ben described the address decoding for the EEPROM, it is possible for both the CPU and the EEPROM to be attempting to write to the bus at the same time (eg. STA $8000). The suggested wiring uses a spare NAND gate on the existing board to prevent that. This would prevent possible damage to the CPU and/or the EEPROM if such code was attempted.

I suggest using the spare NAND gate on U4 and run R/W from the CPU through the AND gate to invert it, and run that to /OE on the EEPROM.

* Disconnect EEPROM (U2) pin 22 from Gnd
* CPU (U1) R/W pin 34 to NAND (U4A) pin 1
* NAND (U4A) pin 2 to pin 1
* NAND (U4A) output (pin 3) to /OE on EEPROM (U2) pin 22

![](images/Protect EEPROM.png)

I am using the 74LS00 as an inverter here, and you can see that when the CPU is writing, then the output of the EEPROM is disabled.

---


## NMI interrupt (suggested) {#nmi_interrupt}


It helps to recover from runaway or rogue code by pressing a switch button connected to the NMI pin of the 65C02. This does a “warm restart” which basically starts the code again, however it retains your existing source code.

* Connect NMI on CPU (U1) pin 6 via 10k resistor to +5V (pull-up).
* Connect same pin via a push-button to ground.

![](images/NMI interrupt.png)

---


## Increased RAM (recommended) {#increased_ram}

The way Ben did his address decoding for the RAM meant that only 16k of the chip’s 32k was available. You can make a simple change using a 74LS08 “AND” gate that extends the RAM address space to 0x6000 rather than 0x4000, thus giving you 24k of RAM (an extra 8k) for the cost of a one dollar chip and a few jumper cables. This change is required to compile the “Adventure” game because of its size.


* Insert a 74LS08 (AND gate) onto a spare part of the breadboard. Connect power to pin 14 and ground to pin 7.
* Connect A13 to pin 2 (from the CPU (U1) pin 23)
* Connect A14 to pin 1 (from the CPU (U1) pin 24)
* Disconnect the previous connection from pin 1 to pin 22 on the RAM (U6).
* Connect output of 74LS08 (pin 3) to RAM (U6) /OE (pin 22).

![](images/More RAM.png)

This increases the top of RAM from $3FFF to $5FFF. Basically if both A13 and A14 are high then the RAM output enable will be off. In other words, the RAM is disabled when **both** the 0x4000 and 0x2000 address bits are set (because we are "and"ing them together), which means that the RAM works up to 0x6000 rather than 0x4000 as it originally did.

---

[Back to main G-Pascal page](index.htm)
