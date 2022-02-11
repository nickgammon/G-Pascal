% G-Pascal Assembler

**Author**: Nick Gammon

**Date written**: February 2022

<div class='quick_link'> [Back to main G-Pascal page](index.htm)</div>

## Contents

* [Features](#Features)
* [How to assemble a program](#how_to_assemble)
* [Syntax and notes](#syntax)
    * [Labels](#labels)
    * [Whitespace](#whitespace)
    * [Opcode / directive](#opcode)
    * [Operand and expression evaluation](#operand)
    * [Directives](#directives)
* [Assembler directives](#directives)
* [Memory layout](#memory_layout)
* [How to co-exist with Pascal code?](#co_exist)
* [Access to EEPROM functions and variables](#eeprom_functions)
* [Accessing the VIA (Versatile Interface Adapater) pins](#via_adapter)
* [Credits](#credits)

**Author**: Nick Gammon (written in 2022)



## Introduction

The G-Pascal assembler is a 65C02 assembler, entirely resident in the EEPROM, and available immediately after resetting your board. This is intended to let you try small to medium-scale assembler projects, without having to keep removing the EEPROM chip and programming it externally.

---

## Features {#Features}

* The documented WD65C02 instruction set, with all operand modes
* Full expression evaluation of operands, with operator precedence, parentheses, bitwise operations and so on.
* Relocation of the output to any memory address (ORG directive)
* Here is "hello world" in assembler:


    ```asm
    ; interface routines in the EEPROM
    serial_print_message = $8012

          jmp start   ; skip the message
    hello asciiz "Hello, world!"
    start = *
          lda #<hello
          ldx #>hello
          jsr serial_print_message
          rts
    ```


---

## How to enter and assemble a program {#how_to_assemble}

1. Enter the Editor from the main menu (type E) and use that to enter your program. Alternatively load it from your PC using the File menu. The Editor is described in a separate page.

2. Type "A" from the main menu to assemble it and produce machine code.

3. If it assembles without errors type "R" from the main menu to run the code. It will automatically start running at the location of the first byte of emitted machine code. It is probably useful to put a JMP there to go to where you want the code to start executing. Put a RTS at the end of your code to return to the G-Pascal system.

---

## Syntax and notes {#syntax}

The assembler is intended to be compatible --- to a point --- with the vasm6502 assembler, in particular when run on your PC like this:

```sh
vasm6502_oldstyle MYFILE.asm  -wdc02 -esc -Fbin -o MYFILE.bin -L MYFILE.list
```

Note that there is a limit to how much can be implemented in a few kB of EEPROM. Thus source files written in this assembler should compile in vasm, however if you use extra features of vasm (like conditional compiles) then these are not supported. In other words, file written here should be compatible with vasm, not necessarily the other way around.

Vasm is available from <http://sun.hasenbraten.de/vasm/>

The assembler is line-based, that is each line is treated separately, and is terminated by the newline character (0x0A).

---

## Labels {#labels}

A line may optionally contain of a label, which must be located in column 1 (the first column).

  * Labels must start with a letter, and then consist of letters, numbers and/or the underscore character.
  * Labels are not case-sensitive.
  * There are no local labels.
  * It is an error to re-use a label.
  * Labels may optionally be followed by a colon (":").
  * Except in the case of the EQU (or "=") directive the value of the label is the value of the current output code address.
  * A line may consist of a label only.

---

## Whitespace {#whitespace}

Whitespace (spaces) are ignored, except that a label must start in the first column.

## Comments

A comment starts with semicolon (";") and continues to the end of the line. Lines may consist entirely of comments, or be entirely blank.

---

## Opcode / directive {#opcode}

Opcodes (eg. LDA) or directives (eq. ORG) are separated from the label, if present, by one or more spaces. If there is no label then the opcode must be at least on column 2 of the line. Opcodes are not case-sensitive.

A test file is [here](examples/asm/assembler_opcode_test.asm) which assembles every opcode/addressing-mode combination. The expected code to be generated is shown in the comments. This should assemble the same under this assembler and vasm.

---

## Operand  {#operand}

Operands are separated from the opcode by one or more spaces.

* The operand "*" must appear on its own, and evaluates to the current output code address, eg.


    ```asm
    foo = *  ; foo is assigned to the current address
    ```

* In the case of opcodes which may or may not use the A register as an operand, the "A" must be explicitly provided, eg.


    ```asm
      LSR A
    ```


* String operands only apply to the ASC, ASCII, ASCIIZ and STRING directives. In those cases a quoted string of any length (up to 255 characters) may be the operand. In a similar way to the Pascal compiler, strings may start with either single of double quotes, and contain the other type of quote inside. Alternatively you may double the starting quote to include it in the string. eg. 'Nick''s cat'. In other places strings of one to three characters long may be used, and will be treated as numbers. Eg.



    ```asm
      lda #'A'   ; load A with $41
    ```


    Strings may also contain "escape" sequences of a backslash followed by a letter, as follows:

        \A    bell ($07)
        \B    backspace ($08)
        \E    escape  (0x1B)
        \F    formfeed ($0C)
        \N    newline  (0x0A)
        \R    carriage return (0x0D)
        \T    horizontal tab (0x09)
        \V    vertical tab (0x0B)
        \'    single quote
        \"    double quote
        \\    backslash
        \Xnn  where nn is one or two hex digits (eg. \x0A \X42 \x9 )

    The letter following the backslash is not case-dependent. For the \\Xnn form, if there is potential confusion if the character following the escape sequence happens to be a hex digit then you should use two digits. Only the first two digits are considered when parsing.


* Other operands go through the "expression evaluator" after taking into account indirect addressing which is noticed by the presence of a leading "(" on the line.

    The expression evaluator combines values using operators following the following precedence list, as obtained from the vasm documentation:

        1. + - ! ~ (unary +/- sign, not, complement),  also < (low-byte) > (high byte)
        2. << >> (shift left, shift right)
        3. & (bitwise and)
        4. ^ (bitwise exclusive-or)
        5. | (bitwise inclusive-or)
        6. * / % (multiply, divide, modulo)
        7. + - (plus, minus)
        8. < > <= >= (less, greater, less or equal, greater or equal)
        9. == != <> (equality, inequality)
        10. && (logical and)
        11. || (logical or)

    The high-byte and low-byte operators are unary operators with a high precedence. Depending on their location in the expression they are not considered to be comparison operators. For example
  :



    ```asm
      lda #>foo           ; load A with the high-order byte of the address of foo
      lda #>(foo + $100)  ; load A with the high-order byte of the address of (foo + $100)
      lda #<foo           ; load A with the low-order byte of the address of foo
    ```

    There is a [operator evaluation](examples/asm/expression_evaluator_test.asm) test file you can try.

* Parentheses may be used to force the order of evaluation of operators.

* Labels, if used in expressions, take on the value of the address of that label.

* Expressions are evaluated internally to 24 bits of precision, and then truncated to 16 bits at the end of the expression evaluation. This means that operations which might overflow 16 bits may still evaluate correctly. eg. lda # ($8000 << 4) >> 12 would generate a9 80 (that is, lda #$80) because the high-order bit was not lost during the initial shift left (it was still in the range of a 24-bit value).


---

## Directives {#directives}

The following assembler directives are supported:

* LIST *number*

    This controls the listing of your code during assembly. It consists of three bits which may be OR'ed together as follows:

    * 1 : Show the source during assembly (with the generated address on the left)
    * 2 : Show the generated opcodes during assembly
    * 4 : Show the symbol table at the completion of the assembly

    For example:


    ```asm
      LIST 1     ; show source
      LIST 1|2   ; show source and generated code (alternatively: LIST 3)
      LIST 1|2|4 ; show source, generated code, and symbols (alternatively: LIST 7)
      LIST 0     ; stop listing anything
    ```


    The assembler does two passes, the first to find the address of forward-declared symbols, and the second to output the machine code.
    The listing is shown during the second pass, unless there is an error, in which case the line in error is shown.

* LIST

    Same as "LIST 3".

* NOLIST

    Same as "LIST 0".

* ASSERT *expression*

    Raises an error "Assertion failed" if *expression* is zero. This can be used to do compile-time checking. For example, you could check if a block of memory exceeded a certain size. You can also use it to check that the assembler is evaluating expressions correctly, for example:

    ```
      assert 2 + 2 == 4
    ```


* ASC *string*

    Insert the string into the code, with no terminator.


    ```asm
      asc "Hello, world!"
    ```


* ASCII *string*

    Same as ASC.

* ASCIIZ *string*

    Insert the string into the code, with a 0x00 (null) terminator.

* STRING *string*

    Same as ASCIIZ.

* *label* EQU *address*

    The specified label takes on the address of the evaluated expression. Alternatively, the address may be the symbol "*" which means "the current address".

    ```
    EEPROM         =      $8000
    start_message  ASC    "Hi there"
    end_message    =      *
    message_length =      end_message - start_message   ; calculate message length
    ```


* *label* = *address*

    Same as EQU.

* ORG *expression*

    The output generation is relocated to whatever *expression* evaluates to, from this point on. If this line had a label the label evaluates to the address *before* the relocation.

    ```
       org $5000    ; output code at $5000 onwards now
    ```

* DFB *expression* \[, *expression* \]

     A single byte is emitted to the output, which is the value of the expression. It must evaluate to 0x00 to 0xFF. Multiple bytes may be emitted separated by commas.

     ```
       dfb $01,$02,$03,$04
     ```

* DFW *expression* \[, *expression* \]

    Two bytes (one word) are emitted to the output, which is the value of the expression. It must evaluate to 0x0000 to 0xFFFF. The bytes are emitted in little-endian order, that is the low-order byte first, followed by the high-order byte. Multiple words may be emitted separated by commas.

    ```
      dfw $1234,$5678,$ABCD
    ```

* WORD *expression* \[, *expression* \]

    Same as DFW.

* BLK *expression*

    The assembler emits *expression* zeroes. For example:


    ```asm
      BLK 10    ; emit 10 zeroes, advancing the output address by 10
    ```


* RESERVE *expression*

    The assembler advances the output address by *expression* bytes *without* emitting anything. This could be handy for reserving blocks of memory without overwriting them during the assembly process.

* SYM *expression*

    Relocate the symbol table, used during assembly, to the address that *expression* evaluates to. This must be done before any symbols are created (that is, before any labels). The intention of this is to allow you to make a "safe place" for code which could be shared by the Pascal compiler and the assembler, as described below.

    For example:


    ```asm
      SYM $4800     ; symbols are to be placed at $4800 (growing downwards)
      ORG $4800     ; code is to be placed at $4800 (growing updwards)
    ```



---

## Memory layout {#memory_layout}

### Where is the generated machine-code placed?

In the absence of an ORG directive, the generated code is placed directly after the end of the source code. This is fine for testing out assembly-code ideas.

During assembly the symbol table is placed at the top of RAM and grows downwards, see graphic:

![](images/Assembler memory organisation.svg.png)

---

### How to co-exist with Pascal code? {#co_exist}

It is possible to generate machine code and call that from a Pascal program (using **call** ( *address* )  ). However to do this we need to move the assembler code somewhere where it won't be overwritten by the Pascal source or the Pascal P-codes. One possible spot would be somewhere between the P-codes and the symbol table, however the exact safe location for such code might be hard to determine.

A safer approach is to move the symbol table (with a compiler and assembler directive) and free up a block of memory, like this:

![](images/Assembler memory organisation with symbol table relocation.svg.png)

To do this, in the assembler we need two extra lines, right at the start of the source file:


```asm
  SYM $5800     ; symbols are to be placed at $5800 (growing downwards)
  ORG $5800     ; code is to be placed at $5800 (growing upwards)
```


If you haven't installed my RAM extending mod, change the address from $5800 to $3800, as RAM ends at $4000 in that case.

Now when you assemble your code the output goes to $5800 and is not affected by the symbol table growing, or the later loading of Pascal source.

Meanwhile, in your Pascal code you need to do a similar thing:



```pas
{%S $5800 }
```


That ensures that the Pascal compiler symbol table also starts lower in memory, thus not clobbering the generated machine code. That directive also relocates the run-time stack to the same address.

---

### Example of co-existence



```asm
  SYM $5800     ; symbols are to be placed at $5800 (growing downwards)
  ORG $5800     ; code is to be placed at $5800 (growing upwards)

; interface routines in the EEPROM
serial_print_message = $8012

      jmp start   ; skip the message
hello asciiz "Hello, world!\n"
start = *
      lda #<hello
      ldx #>hello
      jsr serial_print_message
      rts
```


Load and compile the above assembler code, which relocates itself to $5800 and with the symbol table moved to $5800 to avoid a clash with the symbols.

You can test that on its own now by going to the main menu and selecting \<R\>un. The runtime system automatically starts running at the first *emitted* object code. In this case, that is the "jmp start" instruction.


```pas
  { Relocate symbol table and runtime stack: } {%s $5800}
  { Note: hello_world_relocated.asm should be loaded and compiled before running this }

begin
  writeln ("About to call machine code ...");
  call ($5800);
  writeln ("Machine code done.")
end .
```


Now load, compile and run the above Pascal code. It will display a message, then call the assembler code at $5800, and then return to display its final message.

If you don't have the extended RAM mod then change $5800 in the above examples to $3800.


---

## Access to EEPROM functions and variables {#eeprom_functions}


There are various useful functions exposed in the EEPROM code by way of a "jump table" at the very start of the code. This consists of a series of 3-byte JMP instructions which jump to the start of the functions. The intention here is that, even if the functions move around in subsequent releases of the EEPROM code, the jump tables will always be in the same place, and thus code that relies on them will still work.

A list of the jump tables is [here](examples/asm/on_board_entry_points.asm). You could copy/paste part or all of that file into your project. For space reasons you may choose to only use the parts relevant to a particular project, as each line stands on its own.

There are also the addresses of "well known" zero-page locations which are used by some of the functions.

Example of using the inbuilt functions to multiply two numbers and display the result:



```asm
;
;  zero-page locations used by this code
;
reg            = $0000 ; (3 bytes) work "register" used in various places
value          = $0006 ; (3 bytes) used by the assembler expression evaluator
value2         = $0009 ; (3 bytes) used by the assembler expression evaluator
;
;  functions used by this code
;
exp_multiply   = $8081     ; VALUE := VALUE * VALUE2 (may overflow and the overflow is lost)
dsp_bin        = $809c     ; write the number in REG (3 bytes) to the serial port (in decimal)

;
;  put 47302 into value
;
  lda #<47302
  sta value
  lda #>47302
  sta value+1
  stz value+2
;
;  put 55 into value2
;
  lda #<55
  sta value2
  lda #>55
  sta value2+1
  stz value2+2
;
;  multiply them to get the result 2601610
;
  jsr exp_multiply
;
;  display the result
;
  lda value
  sta reg
  lda value+1
  sta reg+1
  lda value+2
  sta reg+2
  jsr dsp_bin
; done!
  rts
```


---

### Accessing the VIA (Versatile Interface Adapater) pins {#via_adapter}

There are three utility functions in the EEPROM code to control the 16 pins on the VIA chip, which operate similarly to the ones on the Arduino.

The functions are in the [exposed functions jump table](examples/asm/on_board_entry_points.asm).

The VIA ports are PA0 to PA7 (pins 2 to 9 on the chip) and PB0 to PB7 (pins 10 to 17 on the chip). For the purposes of these functions they are numbered 0 to 15, where 0 is PA0, 1 is PA1, 8 is PB0 and 15 is PB7, and so on for the ones in-between.

Pins PA0 and PA1 on the VIA are used for the serial interface, as well as CB2 to detect the start bit for incoming serial data.

Pins PA5, PA6, PA7, PB4, PB5, PB6, PB7 are used for the LCD interface.

That leaves 7 pins for your own use (PA2, PA3, PA4, PB0, PB1, PB2, PB3).

* **pinmode** (port, mode). This sets the mode of the port, where 0 is input and 1 is output.

    Port in A, mode in X, then **jsr pinmode**

* **digitalWrite** (port, value). Writes to the specified port. The value can be 0 or 1.

    Port in A, value in X, then **jsr digitalwrite**

* **digitalRead** (port). Reads a value from the specified port. Returns 0 or 1.

    Port in A, then **jsr digitalread**. Returns the value in A.

![](images/VIA pin mappings.png)

Example:


```asm

;
;  digitalWrite example
;
  jmp start     ; skip the variable declarations

;
;  VIA functions
;
pinmode_           = $809f     ; sets the pin mode - pin:0 to 15; mode: 0 or 1 : pin in A, mode in X
digitalread_       = $80a2     ; reads a pin - pin:0 to 15; Pin to read is in in A. Returns zero or non-zero in A.
digitalwrite_      = $80a5     ; writes to a pin - pin:0 to 15; value: 0 or 1 : pin in A, mode in X

delay_             = $80ae     ; delay for YYXX ms (Y = high-order byte, X = lo-order byte)

counter   dfb 0     ; how many toggles we did
pin_state dfb 0     ; current pin state

ITERATIONS = 20     ; how many times to loop (this will be 10 flashes)

start:

  stz pin_state
  stz counter

;
;  set PA2 to output
;
  lda #2   ; Port PA2
  ldx #1   ; write mode
  jsr pinmode_


write_loop:
;
;  write to PA2
;
  lda pin_state
  eor #1   ; toggle state
  sta pin_state
  tax
  lda #2   ; Port PA2
  jsr digitalwrite_
;
;  delay 500 ms
;
  ldx #<500
  ldy #>500
  jsr delay_
;
;  do it ITERATIONS times
;
  inc counter
  lda counter
  cmp #ITERATIONS
  bcc write_loop
;
;  all done
;
  rts
```


---


## Credits {#credits}

* **Vasm** portable and retargetable assembler was used to assemble the G-Pascal EEPROM binary file. It is available from <http://sun.hasenbraten.de/vasm/>



---

<div class='quick_link'> [Back to main G-Pascal page](index.htm)</div>


---

## License

Information and images on this site are licensed under the [Creative Commons Attribution 3.0 Australia License](https://creativecommons.org/licenses/by/3.0/au/) unless stated otherwise.
