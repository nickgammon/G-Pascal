
  G-PASCAL assembler on-board support functions


  More documentation in the EEPROM source code.


  start                 ; where RESET takes us - a cold start (does not return)
  restart               ; where NMI takes us - a warm start  (does not return)
  hardware_init         ; initialises the VIA (65C22) and the LCD (returns)
;
;  Serial port functions
;
  write_char            ; Write the character in A to the serial port.
  getin                 ; Gets a character from the serial port into A. Blocks.
  serial_available      ; Sees if a character has been received from the serial port. Does not block.
  serial_print_message  ; Sends a null-terminated message to the serial port, message address in A (lo) and X (hi)
;
;  LCD functions
;
  lcd_print_char        ; Writes the character in A to the LCD. No register affected.
  lcd_clear_display     ; Clears the LCD display
  lcd_home              ; Move the LCD cursor to 0,0
  lcd_second_line       ; Moves the cursor to the second line of the LCD display (position 0x40)
  lcd_print_message     ; Print a null-terminated message on the LCD, message address in A (lo) and X (hi).
  lcd_instruction       ; Sends an "instruction" to the LCD. Changes no registers.
  lcd_get_address       ; Returns the LCD "cursor" address in A (so you know how far along the display you are)
  lcd_wait              ; Waits for the LCD to be ready to accept an instruction or data. No register affected.
;
;  String functions
;
  comstl         ; Case-insensitive string compare SRCE to DEST, assumes DEST is upper-case. Length in Y. Zero flag set if match.

  isupper        ; Is A an upper-case letter? (that is: A-Z) returns carry set if so, carry clear if not
  islower        ; Is A a lower-case letter? (that is: a-z) returns carry set if so, carry clear if not
  isalpha        ; Is A alpha? (that is: A-Z or a-z) returns carry set if so, carry clear if not
  isbinary       ; Is A binary? (that is: 0-1) returns carry set if so, carry clear if not
  isdigit        ; Is A numeric? (that is: 0-9) returns carry set if so, carry clear if not
  isxdigit       ; Is A hex? returns carry set if so, carry clear if not.
  isspace        ; Is A whitespace? returns carry set if so, carry clear if not. (0x0a, 0x09,  0x0C, 0x0D, 0x11, 0x20)
  iscntrl        ; Is A a control character? returns carry set if so, carry clear if not. (0x00 to 0x1f, plus 0x7F)
  isalnum        ; Is A alphanumeric? (that is: A-Z, a-z, 0-9) returns carry set if so, carry clear if not


; Arithmetic functions - all these operate on VALUE (3 bytes) and VALUE2 (3 bytes), effectively treating
; them as 3-byte signed numbers in the range -8388608 to +8388607
; These function are used in the assembler expression evaluator.
; They will change the A register and in some cases all registers.
;
; The result is always in VALUE.
; In the case of division the remainder is in REMAIN (always positive)
  exp_add            ; VALUE := VALUE + VALUE2
  exp_subtract       ; VALUE := VALUE - VALUE2
  exp_true           ; VALUE := 0x000001   (true)
  exp_false          ; VALUE := 0x000000   (false)
  exp_eql            ; VALUE is true if VALUE == VALUE2 otherwise false
  exp_neq            ; VALUE is true if VALUE != VALUE2 otherwise false
  exp_less_than      ; VALUE is true if VALUE <  VALUE2 otherwise false
  exp_greater_than   ; VALUE is true if VALUE >  VALUE2 otherwise false
  exp_leq            ; VALUE is true if VALUE <= VALUE2 otherwise false
  exp_geq            ; VALUE is true if VALUE >= VALUE2 otherwise false
  exp_bitwise_or     ; VALUE := VALUE |  VALUE2
  exp_bitwise_and    ; VALUE := VALUE &  VALUE2
  exp_bitwise_xor    ; VALUE := VALUE ^  VALUE2  (exclusive or)
  exp_logical_or     ; VALUE := VALUE || VALUE2  (returns true or false)
  exp_logical_and    ; VALUE := VALUE && VALUE2  (returns true or false)
  exp_shift_left     ; VALUE := VALUE << VALUE2
  exp_shift_right    ; VALUE := VALUE >> VALUE2
  exp_negate         ; VALUE := VALUE ^ 0xFFFFFF  (negates the bits in VALUE)
  exp_not            ; if VALUE is non-zero it becomes false, otherwise true
  exp_unary_minus    ; VALUE := 0 - VALUE
  exp_low_byte       ; VALUE := VALUE & 0xFF
  exp_high_byte      ; VALUE := (VALUE >> 8) & 0xFF
  exp_abs_val        ; absolute value of VALUE
  exp_multiply       ; VALUE := VALUE * VALUE2 (may overflow and the overflow is lost)
  exp_divide         ; VALUE := VALUE / VALUE2 (remainder in REMAIN) *
  exp_modulo         ; VALUE := VALUE % VALUE2  (modulus) - always positive *
;
; * Raises an error if the divisor is zero
;
;  Output-switching functions
;
  write_to_serial   ; future writes are to the serial port
  write_to_lcd      ; future writes are to the LCD screen

;  Functions below write to either the serial port or the LCD depending on
;  whether write_to_serial or write_to_lcd was called prior to this call.
;  In particular, they write by calling the function whose address is in "write_function" below.
;
  cout        ; writes a character to the serial port or the LCD
  crout       ; writes a newline
  prbyte      ; displays A in hex (ie. two characters)
  putsp       ; displays a space
  dsp_bin     ; write the number in REG (3 bytes) to the serial port (in decimal)
  print       ; Sends a null-terminated message to output, the message address in A (lo) and X (hi)
              ; but stops if the "abort" key is pressed (Ctrl+C)

;
;  VIA functions (names have trailing underscore to avoid clashing with compiler reserved words)
;
  pinmode           ; sets the pin mode - pin:0 to 15; mode: 0 or 1 : pin in A, mode in X
  digitalread       ; reads a pin - pin:0 to 15; Pin to read is in in A. Returns zero or non-zero in A.
  digitalwrite      ; writes to a pin - pin:0 to 15; value: 0 or 1 : pin in A, mode in X
;
;  Utility functions
;
  tknjmp            ; looks up a "token" and jumps to the address if found.
  delay_1ms         ; delay 1ms when running with the 1 Mhz oscillator
  delay_            ; delay for YYXX ms (Y = high-order byte, X = lo-order byte)
  crc16             ; CRC-16; address in crc_addr, count in crc_num, result in crc_val
  crc_byte          ; do a CRC of the byte in A, updating crc_val
  gen_random        ; generate a pseudo-random number (seed is in random, new value in random)

; ----------------------------------------
;
;  "Well known" zero-page locations - used by some of the above functions
;
;
;  General work-areas for arithmetic routines (eg. add, subtract, divide, multiply)
;   3-byte signed numbers: -8388608 to +8388607
;
  value           ; (3 bytes) used by the assembler expression evaluator
  value2          ; (3 bytes) used by the assembler expression evaluator
  remain          ; (3 bytes) division remainder and used in a few other places
  random_         ; (4 bytes) 32-bit random number
  typing_latency  ; (3 bytes) incremented while waiting for input
  call_a          ; (1 bytes) used when doing a machine code call:  A register
  call_x          ; (1 bytes)  ditto:                               X register
  call_y          ; (1 bytes)  ditto:                               Y register
  call_p          ; (1 bytes)  ditto:                               status register
  write_function  ; (2 bytes) address of write function

;
;  alternative names for some of the above variables
;
; SRCE and DEST used in compiler and editor, particularly for string comparisons
;
  srce       ; alias of reg  (3 bytes) work "register" used in various places
  dest       ; alias of reg2 (3 bytes) work "register" used in various places
;
;  re-using above variables for CRC-16 calculations
;
  crc_addr   ; alias of value  - the address to take a CRC of (2 bytes)
  crc_num    ; alias of value2 - the number of bytes to take a CRC of (2 bytes)
  crc_val    ; alias of remain - the current CRC value (2 bytes)

;
; Note that at present addreses $00D0 to $00FF are available for your use.
;
