% G-Pascal compiler specs and notes


## Contents

* [Features](#Features)
* [Limitations](#Limitations)
* [How to enter and compile a program](#how_to_compile)
* [Extensions and syntax notes](#extensions)
* [Reading and writing to the terminal](#reading)
* [Calling machine code](#machine_code)
* [Differences to C](#differences)
* [Accessing the VIA (Versatile Interface Adapater) pins](#via_adapter)
* [Syntax diagram](#syntax_diagram)
* [Compiler directives](#directives)
* [Debugging](#debugging)
* [P-code meanings](#pcode_meanings)
* [Example code](#example_code)
* [Reserved words](#reserved_words)
* [Memory layout](#memory_layout)
* [Credits](#credits)

**Author**: Nick Gammon (written in 2022)

<div class='quick_link'> [Back to main G-Pascal page](index.htm)</div>
<div class='quick_link'> [Assembler info](assembler.htm)</div>
<div class='quick_link'> [Text editor](editor.htm) </div>
<div class='quick_link'> [File menu](file_menu.htm) </div>


## Introduction

The G-Pascal compiler is a "tiny" Pascal compiler, entirely resident in the EEPROM, and available immediately after resetting your board.

---

## Features {#Features}


* CONST, VAR, FUNCTION and PROCEDURE declarations
* Local declarations (functions and variables within functions, etc.)
* Recursive function and procedure calls
* Arithmetic: multiply, divide, add, subtract, modulus
* Logical operations: and, or, shift left, shift right, exclusive or
* INTEGER and CHAR data types. (Integer are 3 bytes and thus range from -8388608 to 8388607)
* Arrays
* Interface with any memory address by using the MEM and MEMC constructs (to peek and poke memory locations)
* Built-in functions to write to the LCD display.
* Built-in functions to do pinMode, digitalRead and digitalWrite, similar to the Arduino. These interface with any available ports on the VIA chip.

---

## Limitations {#Limitations}

* Integers are only three bytes and range in value from -8388608 to 8388607.
* A quirk of the numeric literal parser means you cannot directly use the numeric literal -8388608 in a program. A workaround it to use $800000 instead as that is the hex equivalent of that value, or to use (-8388607 - 1).
* Char variables are one byte long and range in value from 0 to 255. Char and integer variables can be used interchangeably, however data stored into a char variable will be truncated to a single byte.
* There is no type-checking.
* There are no type definitions.
* Arrays are single dimension only, and range from 0 to whatever final index you specify. Thus an array declared "var foo : array \[10\] of integer" in fact takes eleven positions in memory, from \[0\] to \[10\]. Arrays start at index 0.
* Array bounds are not checked at run-time.
* Arguments to procedures and functions are passed by value, not by reference.
* Programs do not start with the word "program" --- you start a program with the first (main) block.
* The compiler does not produce machine code, it produces "Pseudo code" (P-code). This is then interpreted by the runtime system.

---

## How to enter and compile a program {#how_to_compile}

1. Use the Editor to enter your program (type "I" to insert text). Alternatively load it from your PC by using "LOAD". The Editor is described [here](editor.htm).

2. Type "S" to syntax-check the code. This is slightly faster than attempting to compile it. This step is optional.

3. When it is error-free type "C" to compile it and produce P-codes.

4. If it compiles without errors type "R" to run the code. Alternatively type D to debug or T to trace it.

5. If the code does not appear to be doing anything try typing Ctrl+C to abort it, Ctrl+T to trace it, or Ctrl+D to enter debugging mode. Alternatively press the NMI switch (if installed) to do a warm start, recovering control of the processor and retaining your source code.


---

## Extensions and syntax notes {#extensions}

* String literals are supported for use in **write**, **writeln**, and **lcdwrite** function calls.
* String literals up to three bytes long can be used as constants. For example:


    ```pas
      var foo : integer ;
      begin
        foo := 'a';    { writes: 97 }
        writeln (foo);
      end .
    ```

    This will display 97 as that is the value of the letter 'a'.

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

* You may use an **else** clause with the **case** statement, to specify a statement to be executed if no case matches. Case selectors may be expressions, not just constants.
* Numeric constants may be one of:
    * Signed decimal numbers (eg. -10, 42, +666)
    * Hexadecimal constants (eg. $1234)
    * Binary constants (used in the assembler, but available in the Pascal compiler) (eg. %00111011001010)
    * String literals up to a maximum of three bytes long. This is little endian architecture, so that constant "abc" would be stored as ("a" + ("b" * 256) + ("c" * 65536)).
    * String literals may start with either single or double quotes, and end with the same quote symbol they started with. You may use the other quote symbol inside a quoted string. (eg. "Nick's cat"). Alternatively you can insert a quote symbol inside a string by putting it twice. (eg. 'Nick''s cat').
* Direct access to memory. You can use **mem** or **memc** to "peek" or "poke" either three bytes or a single byte into memory. For example:


    ```pas
    memc [$1234] := $42;
    foo := memc [$5678];
    ```


    This could be used to directly access the hardware registers.

* **write**, **writeln** may be used to write to the serial port. They behave the same except that **writeln** adds a newline at the end. **writeln** may be called without arguments to simply output a newline.
* **lcdwrite** may be used to write to the LCD screen.
* **lcdhome** homes the cursor on the LCD screen.
* **lcdclear** clears the LCD screen.
* **lcdpos** (line, column) moves the cursor on the LCD screen to the specified position (zero-relative).
* **write**, **writeln** and **lcdwrite** statements may take the following arguments:
    * String literals of any length: these will be written "as is".
    * An *expression*, which is written as a signed decimal number.
    * **chr** ( *expression* ) --- this converts the value of the expression to a character and writes that, for example:


        ```pas
        write ( chr (65) );
        ```

        That would write the letter "A".

    * **hex**  ( *expression* ) --- this writes the value of the expression as a 6-character hex number.
    * See below for more examples.

* Random numbers can be obtained by calling the **random** function. These are pseudo-random numbers generated with an initial seed internally of 0xFFFFFFFF. Thus they will be the same every time the processor is reset. You can change the seed by using **mem** to change the current value in memory. A possible way of seeding the numbers in an unpredictable way (eg. for games) is to use an internal value in memory which increments every time the code is waiting for a keypress from the user. Since the time between keypresses will be unpredictable (up to a point) the random numbers should look random enough for casual applications. The numbers returned will be in the range -8388608 to 8388607. You can reduce that to (say) a number from 1 to 10 by doing this:

    ```
    writeln (random mod 10 + 1);
    ```


* You can obtain the address of a variable by using **address** ( *variable* ). This could be used to pass the address of a variable to a function, which could then use **mem** to change that variable, thus effectively passing a pointer to a function rather than a value.

* Comments are between braces, or bracket-asterisk, see examples below. Comments may not be nested. The two different sorts of comment symbols are treated interchangeably, so you could conceivably start with one and end with another, see below.


    ```pas
     { my comment }
     (* my comment *)
     { my comment *)
    ```


* There is no **boolean** type, however any variable could be considered a boolean, and you could declare:


    ```pas
    const true = 1; false = 0;
    ```


    The results of conditional comparisons (eg. greater-than, less-than) will always be 0 (false) or 1 (true).

* Any statement (like **if**) which tests for true or false will consider 0 to be false and any other value to be true.

* The compiler is not case-sensitive.

* Identifier names can be any length (within reason), must start with a letter, and after that be letters, numbers or the underscore character. All characters of the name are significant.

* Arrays start at index 0.

* You can use **assert** to do a runtime assertion (unlike the assembler which does a compile-time assertion). This can be used to "bail out" if some variable has an unexpected value, eg.

    ```
    assert (2 + 2 = 4);
    ```
* You can reseed the random number generator by calling **randomseed** with some number as an argument.

* You can get the typing latency (a number which is incremented while waiting for keyboard input) by calling **latency**.

* Thus you could reseed the random number generator with a fairly unpredictable number like this:

    ```
    randomseed (latency);
    ```

---

### Reading and writing to the terminal {#reading}

#### Read

The built-in statement READ (which is a reserved word) can read from your terminal in three ways:

* Read into an INTEGER variable

    A line of input is accepted from the user, and the first integer found on that line (bypassing leading spaces) is placed into the variable. If the input on that line could not be parsed into a (signed) integer then hex $800000 (decimal -8388608) is placed in the variable as an "error" marker. The integer is obtained by calling the compiler's token parser, so any input that resolves to a number is acceptable (for example, a binary or hex integer). Any other text on that line is discarded without comment. eg.

    ```
    var i : integer;
    begin
      repeat
        write ("Enter a number: ");
        read (i);
        writeln ("You entered: ", i)
      until i = 0
    end.
    ```

    Note that because of the way the token parser works you will get an error trying to compare to -8388608. Instead, compare to $800000. For example:

    ```
    read (i);
    if i = $800000 then
      begin
      writeln ("Bad number entered")
      end;
    ```




* Read into a CHAR variable

    A single character is accepted from the user without waiting for a newline. This "blocks", that is control does not return to the program until a character has been typed. This is placed in the variable.

* Read into a CHAR array.

    A line of input is accepted from the user, which is placed into the array, up to the array maximum length. The line will be terminated by a newline (hex $0a, decimal 10).

Multiple variables can be read into (eg. READ (a, b, c)) however each one is treated exactly as if they appeared in separate READ statements, and thus to read three numbers (for example) you would have to enter them on three lines. An alternative would be to read into a CHAR array, and parse multiple numbers yourself.

To read without blocking use GETKEY which returns the current key as a number (eg. 65 for the letter 'A') or zero if no key is pressed. GETKEY clears the "read" key, so if you need to check if a key is pressed and also find out what key that is, you would need to save it, for example:

```
var myKey : char;
begin
  write ("Press a key ...");
  repeat
    myKey := getkey;
  until myKey;
  writeln ("You entered ", myKey)
end .
```


#### WRITE and WRITELN

The built-in statement WRITE (which is a reserved word) can be used to write numbers, string literals, hex numbers or characters.

WRITELN behaves the same as WRITE except that it appends a newline character after writing its parameters. WRITELN may be used without any arguments in order to simply output a newline.

* Write a number

    If you write a literal number, or a variable (which can be an array element), or an expression, that number is written in decimal. For example:

    ```
    var i : integer;
    begin
      i := 666;
      write (i, " ", 1234567, " ", 5 * 8)  { writes: 666 1234567 40 }
    end.
    ```

* Write a number in hex

    To write a number in hex, use HEX (expression), e.g.

    ```
    var i : integer;
    begin
      i := 666;
      writeln (hex (i))   { writes: 00029a }
    end.
    ```

* Write a number as an ASCII character

    To convert a number into its ASCII (printable) equivalent, use CHR (expression), e.g.

    ```
    var i : integer;
    begin
      i := 65;
      writeln (chr (i))  { writes: A }
    end.
    ```

* Write a literal string

    Literal strings (like "Hello, world") can be used in a WRITE statement, e.g.

    ```
    begin
      writeln ("Hello, world")
    end .
    ```

    As mentioned earlier literal strings may include control characters by using escape (backslash) codes, so you can embed newlines, e.g.

    ```
    begin
      writeln ("Hello, world.\nEnjoy learning Pascal.")
    end .
    ```

WRITE and WRITELN can take any number of parameters, these are just written one after the other without intervening spaces.

---

### Calling machine code {#machine_code}

You can call machine code (eg. that you produced with the assembler) by using **call**.

You can "seed" the A, X, Y and status registers prior to the call by using **memc**. For example:


```pas
memc [$10] := 22;  { A register }
memc [$11] := 33;  { X register }
memc [$12] := 44;  { Y register }
memc [$13] := %00000010;   { status register P: NV1BDIZC - see image below
                             - example sets Z flag }
call ($5000);      { call the machine-code function, which should end with an RTS }
                   { the values of A,X,Y,P after the call are now in the above addresses }
```


![](images/6502 status register.png)


These addresses ($10 to $13) are "well-known" addresses used by G-Pascal. They will always be used for this purpose.

---

### Accessing the VIA (Versatile Interface Adapater) pins {#via_adapter}

There are three inbuilt procedures and functions to control the 16 pins on the VIA chip, which operate similarly to the ones on the Arduino.

The VIA ports are PA0 to PA7 (pins 2 to 9 on the chip) and PB0 to PB7 (pins 10 to 17 on the chip). For the purposes of these functions they are numbered 0 to 15, where 0 is PA0, 1 is PA1, 8 is PB0 and 15 is PB7, and so on for the ones in-between.

Pins PA0 and PA1 on the VIA are used for the serial interface, as well as CB2 to detect the start bit for incoming serial data.

Pins PA5, PA6, PA7, PB4, PB5, PB6, PB7 are used for the LCD interface.

That leaves 7 pins for your own use (PA2, PA3, PA4, PB0, PB1, PB2, PB3).

* **pinMode** (port, mode). This sets the mode of the port, where 0 is input and 1 is output.

* **digitalWrite** (port, value). Writes to the specified port. The value can be 0 or 1.

* **digitalRead** (port). Reads a value from the specified port. Returns 0 or 1.

![](images/VIA pin mappings.png)

Example:


```pas
pinMode (2, 1);  { set VIA PA2 (pin 4 on the chip) to output }
digitalWrite (2, 1);  { set PA2 to logic level 1 }
foo := digitalRead (3);  { read PA3 }
```

---

## Syntax diagram {#syntax_diagram}

The compiler's syntax diagram is [here](G-Pascal syntax diagram.xhtml) --- check that if you find yourself getting error messages.

This was generated from the EBNF syntax [here](doc/ebnf_syntax.txt).

---

## Differences to C {#differences}

If you are used to C (or C++) you may find yourself getting a lot of error messages. Points to note are:

* Procedures and functions must be declared at the *start* of a block, not in the middle.

* The order is important. CONST first, then VAR, then procedure/function declarations, then BEGIN.  eg.



    ```pas
    const LIMIT = 10;                { <-- this is the start of a block: CONST before VAR }
    var k    : integer;              { we declare variables and constants here - BEFORE "begin" }
        line : array [100] of char;  { array of 101 items }

    function foo;       { and functions and procedures - this is the start of another block}
      { constants and variables would go here, if needed }
    begin
      foo := 42         { assign to function return value }
    end;                { end of the function block - semicolon is required }

    begin               { end of declarations - now we have the main block statements }
     for k := 1 to LIMIT do
       writeln ("square of ", k, " is ", k * k);
     write (foo)        { call function }
    end.
    ```


* If a procedure or function does not take arguments *omit the brackets*.

* Likewise, omit the brackets if you *call* the procedure or function if it takes no arguments (unlike C)

* Semicolons *separate* statements, they don't terminate them.

* BEGIN and END are similar to braces in C. If you want to do multiple things (eg. as part of an IF statement) then
put BEGIN ... END around them.

### Things to be careful of

* Functions which do not assign to the function return value return zero.
* Variables are not initialised and therefore their initial contents are undefined.
* Multiplication discards high-order bits if the result is too large.
* Divide remainder is always positive (ie. MOD always gives a positive result, regardless of operand signs)
* Function arguments are always integers and always return an integer.
* Divide-by-zero raises an error and stops execution. Check for a zero divisor if you don't want that to happen.
* One to three-byte strings can be used as integer constants

    eg. a := "abc"; write (a);   --> Result: 6513249 (ie. 0x636261 or "abc")

    The first character is the low-order byte in the number (little endian), explaining the above results.

* Maximum number of arguments to a procedure/function is 85 (255 / 3) due to internal coding
* Procedures or functions may be nested (declared within another procedure or function) up to 127 levels of depth.
* There is no **return** statement, unlike C. For a function to return a value you assign to the function name. See the example just above where inside the function foo an assignment is made to foo. That is setting the function return value.
* Check the [syntax diagram](G-Pascal syntax diagram.xhtml) if you are unsure.

---

## Compiler directives {#directives}

Compiler directives are placed inside comments, with a "%" in front of them.

* {%L} - list during compile (source code and current P-code address)
* {%P} - show generated P-codes during compile - only works during compile (not syntax check)
* {%N} - stop listing during compile (cancels %L and %P)
* {%S *address* } - relocate symbol table to *address* - must be done before defining symbols,
also generates an opcode to relocate the runtime stack. More information about the usefulness of this is on the [assembler page](assembler.htm).

The % must directly follow the start of the comment.

---

## Debugging {#debugging}

If your program seems to be producing strange results you can either Trace or Debug it.

### Trace mode

If you select Trace instead of Run when you go to start execution, you will see something like this:


```
(0590)  00 d0 07 00
(0594)  14 3d 1c 00
(0595)  3d 1c 00 2c
(0598)  2c 00 20 f8
(059c)  80 37 00 f7
(059d)  37 00 f7 ff
(05a1)  2c 00 20 f8
(05a5)  2c 00 1d f8
(05a9)  04 32 00 20
```


This lists the address of the current P-code (pseudo-code) and then the P-code (first byte) and the following three bytes. Some P-codes are only one byte long (in the example above, at $0594 you can see that the next line is at $0595, so that particular P-code was only one byte.

You can see what lines of code the P-codes relate to by putting "{%L}" at the start of your source code. Then, when compiling, you will see something like this:


```
(058c)   27           while K < size do
(0598)   28           begin
(0598)   29             flags [k] := false;
(05a1)   30             k := k + prime
(05a9)   31           end ;
```



You can see from the above that we are clearly in the loop from lines 27 to 31, because the P-code addresses agree with the trace information.

**Tip**: If you add {%L} to your source, and recompile it, the P-code addresses detected in the trace may have changed because adding to the source file makes it longer, and thus the generated P-codes (which follow the source) will be at different addresses. Thus you may need to do a Trace again to make the addressess in the listing agree with the addresses in the Trace.

You can also start tracing in the middle of execution by typing Ctrl+T.

### Debug mode

If you select Debug instead of Run when you go to start execution, you will see something like this:



```
(0537)  16 3d 18 00
 Stack: 5810 = 02 00 00 d0 07 00 d0 07 00
 Base:  5fff = 05 04 53 49 5a 45
(0538)  3d 18 00 2c
 Stack: 5813 = 01 00 00 d0 07 00 02 00 00
 Base:  5fff = 05 04 53 49 5a 45
(053b)  2c 00 1a f8
 Stack: 5816 = d0 07 00 02 00 00 07 00 00
 Base:  5fff = 05 04 53 49 5a 45
(053f)  81 37 00 f7
 Stack: 5813 = 02 00 00 d0 07 00 02 00 00
 Base:  5fff = 05 04 53 49 5a 45
(0540)  37 00 f7 ff
 Stack: 5810 = 01 00 00 02 00 00 d0 07 00
 Base:  5fff = 05 04 53 49 5a 45
(0544)  2c 00 1a f8
 Stack: 5816 = d0 07 00 02 00 00 07 00 00
 Base:  5fff = 05 04 53 49 5a 45
```



This gives you somewhat more verbose information. For each P-code address you see the P-code itself on the right. On the first line the P-code is $16 (Greater or Equal) followed by $3d on the next line (Jump if zero). See [here](doc/P-code-meanings.txt) for the meanings of the P-code numbers.

The Stack, underneath, shows the top three values on the runtime stack. In the case of the first line (Greater or Equal), the top value is 02 00 00 and since this is a little-endian machine, that is the number 2. The next value is  d0 07 00, which is therefore $0007d0, which is 2000 in decimal. This example is from the Eratosthenes Sieve prime number generator below, so clearly we are comparing to see if 2 < 2000 (or effectively, if 2000 >= 2). Clearly either of those comparisons is true (2 is less than 2000) so we see a $000001 (true) on the top of the stack underneath the P-code at $0538. Thus the "jump if zero" is not taken, and the loop continues.

The Base underneath that is rather complicated, and is explained in the compiler source code. Basically it is used to restore the stack when a function returns, and has provision for three two-byte addresses:

* The return address from this procedure/function call
* The dynamic stack frame address (what the stack was when this function was called)
* The static stack frame address (which function was lexically prior to this function) which is used for finding variables in earlier functions.

You can also start tracing in the middle of execution by typing Ctrl+D.


### Stopping tracing/debugging/execution

You can stop tracing/debugging in the middle of execution by typing Ctrl+N.

You can abort execution by typing Ctrl+C, or by pressing the NMI switch if you installed it.

---

## P-code meanings {#pcode_meanings}

A list of P-code meanings is [here](doc/P-code-meanings.txt).

Many of the P-codes have no operands. That is, they operate on the top contents of the runtime stack. For example, to add two numbers the compiler would generate:


```
LIT $012345     <--- push the constant $12345
LIT $056789     <--- push the constant $56789
ADD             <--- add top two items on the stack (leaving the result on top of the stack)
```

---

## Example code {#example_code}


```pas
{ Eratosthenes Sieve prime number generator }

{ Note: does not detect the number 2 }

const size = 2000;
      true = 1;
      false = 0;
      perline = 10;

var flags : array [size] of char;
    i, prime, k, count, online : integer;

begin
    count := 0;     { how many primes we found }
    writeln;

    online := 0;    { numbers we have shown on this line }

    for i := 0 to size do
      flags [i] := true;

    for i := 0 to size do
        if flags [i] then
        begin
          prime := i + i + 3;
          k := i + prime;
          while K < size do
          begin
            flags [k] := false;
            k := k + prime
          end;
          if online > perline then
          begin
            writeln;
            online := 0
          end;
          online := online + 1;
          count := count + 1;
          write (prime, " ")
        end;
      writeln;
      writeln (count, "  primes")
end.
```

Output:



```
Running

3 5 7 11 13 17 19 23 29 31 37
41 43 47 53 59 61 67 71 73 79 83
89 97 101 103 107 109 113 127 131 137 139
149 151 157 163 167 173 179 181 191 193 197
199 211 223 227 229 233 239 241 251 257 263
269 271 277 281 283 293 307 311 313 317 331
...
3089 3109 3119 3121 3137 3163 3167 3169 3181 3187 3191
3203 3209 3217 3221 3229 3251 3253 3257 3259 3271 3299
3301 3307 3313 3319 3323 3329 3331 3343 3347 3359 3361
3371 3373 3389 3391 3407 3413 3433 3449 3457 3461 3463
3467 3469 3491 3499 3511 3517 3527 3529 3533 3539 3541
3547 3557 3559 3571 3581 3583 3593 3607 3613 3617 3623
3631 3637 3643 3659 3671 3673 3677 3691 3697 3701 3709
3719 3727 3733 3739 3761 3767 3769 3779 3793 3797 3803
3821 3823 3833 3847 3851 3853 3863 3877 3881 3889 3907
3911 3917 3919 3923 3929 3931 3943 3947 3967 3989 4001
4003
551  primes
```


---

## Reserved words {#reserved_words}

The following words are reserved by the compiler. They may not be used as identifiers (variables, constants, function or procedure names) in the compiler.

```
address, and, array, begin, call, case, char, chr, const,
div, do, downto, else, end, for, function, hex, if, integer,
mem, memc, mod, not, of, or, procedure, read, repeat, shl,
shr, then, to, until, var, while, write, writeln, xor
```

---

## Memory layout {#memory_layout}

![](images/Compiler memory organisation.svg.png)

---

## Credits {#credits}

* The compiler was originally written in 1979 for the Motorola 6800 after reading a series of articles in Byte magazine (September, October, and November 1978) written by Kin-Man Chung and Herbert Yuen. That article described making a "Tiny" Pascal compiler written in Basic.

* Syntax diagrams generated from the "Bottlecaps"  Railroad Diagram Generator available [here](https://bottlecaps.de/rr/ui#expression).


---

<div class='quick_link'> [Back to main G-Pascal page](index.htm)</div>


---

## License

Information and images on this site are licensed under the [Creative Commons Attribution 3.0 Australia License](https://creativecommons.org/licenses/by/3.0/au/) unless stated otherwise.
