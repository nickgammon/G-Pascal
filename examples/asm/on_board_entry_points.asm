;
;  G-PASCAL assembler on-board support functions
;
;  Jump table for the exposed internal functions. (Apart from the first two, do a JSR to call them)
;
;  More documentation in the EEPROM source code.
;
;  Author: Nick Gammon
;  Date:   9 Feb 2022
;
; ----------------------------------------
  start         = $8000         ; where RESET takes us - a cold start (does not return)
  restart       = $8003         ; where NMI takes us - a warm start  (does not return)
  hardware_init = $8006         ; initialises the VIA (65C22) and the LCD (returns)
;
;  Serial port functions
;
  write_char            = $8009 ; Write the character in A to the serial port.
  getin                 = $800c ; Gets a character from the serial port into A. Blocks.
  serial_available      = $800f ; Sees if a character has been received from the serial port. Does not block.
  serial_print_message  = $8012 ; Sends a null-terminated message to the serial port, message address in A (lo) and X (hi)
;
;  LCD functions
;
  lcd_print_char    = $8015     ; Writes the character in A to the LCD. No register affected.
  lcd_clear_display = $8018     ; Clears the LCD display
  lcd_home          = $801b     ; Move the LCD cursor to 0,0
  lcd_second_line   = $801e     ; Moves the cursor to the second line of the LCD display (position 0x40)
  lcd_print_message = $8021     ; Print a null-terminated message on the LCD, message address in A (lo) and X (hi).
  lcd_instruction   = $8024     ; Sends an "instruction" to the LCD. Changes no registers.
  lcd_get_address   = $8027     ; Returns the LCD "cursor" address in A (so you know how far along the display you are)
  lcd_wait          = $802a     ; Waits for the LCD to be ready to accept an instruction or data. No register affected.
;
;  String functions
;
  comstl    = $802d             ; Case-insensitive string compare SRCE to DEST, assumes DEST is upper-case. Length in Y. Zero flag set if match.
  isithx    = $8030             ; Is A hex? returns carry clear if hex, carry set if not. Also converts to be 0x0 to 0xF.
  isital    = $8033             ; Is A alpha? (that is: A-Z or a-z) returns carry clear if alpha, carry set if not
  isitnm    = $8036             ; Is A numeric? (that is: 0-9) returns carry clear if numeric, carry set if not
  isitbin   = $8039             ; Is A binary? (that is: 0-1) returns carry clear if binary, carry set if not

; Arithmetic functions - all these operate on VALUE (3 bytes) and VALUE2 (3 bytes), effectively treating
; them as 3-byte signed numbers in the range -8388608 to +8388607
; These function are used in the assembler expression evaluator.
; They will change the A register and in some cases all registers.
;
; The result is always in VALUE.
; In the case of division the remainder is in REMAIN (always positive)
  exp_add           = $803c     ; VALUE := VALUE + VALUE2
  exp_subtract      = $803f     ; VALUE := VALUE - VALUE2
  exp_true          = $8042     ; VALUE := 0x000001   (true)
  exp_false         = $8045     ; VALUE := 0x000000   (false)
  exp_eql           = $8048     ; VALUE is true if VALUE == VALUE2 otherwise false
  exp_neq           = $804b     ; VALUE is true if VALUE != VALUE2 otherwise false
  exp_less_than     = $804e     ; VALUE is true if VALUE <  VALUE2 otherwise false
  exp_greater_than  = $8051     ; VALUE is true if VALUE >  VALUE2 otherwise false
  exp_leq           = $8054     ; VALUE is true if VALUE <= VALUE2 otherwise false
  exp_geq           = $8057     ; VALUE is true if VALUE >= VALUE2 otherwise false
  exp_bitwise_or    = $805a     ; VALUE := VALUE |  VALUE2
  exp_bitwise_and   = $805d     ; VALUE := VALUE &  VALUE2
  exp_bitwise_xor   = $8060     ; VALUE := VALUE ^  VALUE2  (exclusive or)
  exp_logical_or    = $8063     ; VALUE := VALUE || VALUE2  (returns true or false)
  exp_logical_and   = $8066     ; VALUE := VALUE && VALUE2  (returns true or false)
  exp_shift_left    = $8069     ; VALUE := VALUE << VALUE2
  exp_shift_right   = $806c     ; VALUE := VALUE >> VALUE2
  exp_negate        = $806f     ; VALUE := VALUE ^ 0xFFFFFF  (negates the bits in VALUE)
  exp_not           = $8072     ; if VALUE is non-zero it becomes false, otherwise true
  exp_unary_minus   = $8075     ; VALUE := 0 - VALUE
  exp_low_byte      = $8078     ; VALUE := VALUE & 0xFF
  exp_high_byte     = $807b     ; VALUE := (VALUE >> 8) & 0xFF
  exp_abs_val       = $807e     ; absolute value of VALUE
  exp_multiply      = $8081     ; VALUE := VALUE * VALUE2 (may overflow and the overflow is lost)
  exp_divide        = $8084     ; VALUE := VALUE / VALUE2 (remainder in REMAIN) *
  exp_modulo        = $8087     ; VALUE := VALUE % VALUE2  (modulus) - always positive *
;
; * Raises an error if the divisor is zero
;
;  Output-switching functions
;
  write_to_serial   = $808a     ; future writes are to the serial port
  write_to_lcd      = $808d     ; future writes are to the LCD screen

;  Functions below write to either the serial port or the LCD depending on
;  whether write_to_serial or write_to_lcd was called prior to this call.
;  In particular, they write by calling the function whose address is in "write_function" below.
;
  cout              = $8090     ; writes a character to the serial port or the LCD
  crout             = $8093     ; writes a newline
  prbyte            = $8096     ; displays A in hex (ie. two characters)
  putsp             = $8099     ; displays a space
  dsp_bin           = $809c     ; write the number in REG (3 bytes) to the serial port (in decimal)
;
;  VIA functions (names have trailing underscore to avoid clashing with compiler reserved words)
;
  pinmode_          = $809f     ; sets the pin mode - pin:0 to 15; mode: 0 or 1 : pin in A, mode in X
  digitalread_      = $80a2     ; reads a pin - pin:0 to 15; Pin to read is in in A. Returns zero or non-zero in A.
  digitalwrite_     = $80a5     ; writes to a pin - pin:0 to 15; value: 0 or 1 : pin in A, mode in X
;
;  Utility functions
;
  tknjmp            = $80a8     ; looks up a "token" and jumps to the address if found.
  delay_1ms         = $80ab     ; delay 1ms when running with the 1 Mhz oscillator
  delay_            = $80ae     ; delay for YYXX ms (Y = high-order byte, X = lo-order byte)
  crc16             = $80b1     ; CRC-16; address in crc_addr, count in crc_num, result in crc_val
  crc_byte          = $80b4     ; do a CRC of the byte in A, updating crc_val

; ----------------------------------------
;
;  "Well known" zero-page locations - used by some of the above functions
;
  reg            = $0000 ; (3 bytes) work "register" used in various places
  reg2           = $0003 ; (3 bytes) work "register" used in various places
;
;  General work-areas for arithmetic routines (eg. add, subtract, divide, multiply)
;   3-byte signed numbers: -8388608 to +8388607
;
  value          = $0006 ; (3 bytes) used by the assembler expression evaluator
  value2         = $0009 ; (3 bytes) used by the assembler expression evaluator
  remain         = $000C ; (3 bytes) division remainder and used in a few other places
  random_        = $000F ; (4 bytes) 32-bit random number
  typing_latency = $0013 ; (3 bytes) incremented while waiting for input
  call_a         = $0016 ; (1 bytes) used when doing a machine code call:  A register
  call_x         = $0017 ; (1 bytes)  ditto:                               X register
  call_y         = $0018 ; (1 bytes)  ditto:                               Y register
  call_p         = $0019 ; (1 bytes)  ditto:                               status register
  write_function = $001A ; (2 bytes) address of write function

;
;  alternative names for some of the above variables
;
; SRCE and DEST used in compiler and editor, particularly for string comparisons
;
  srce           = $0000   ; alias of reg
  dest           = $0003   ; alias of reg2
;
;  re-using above variables for CRC-16 calculations
;
  crc_addr       = $0006     ; alias of value  - the address to take a CRC of (2 bytes)
  crc_num        = $0009     ; alias of value2 - the number of bytes to take a CRC of (2 bytes)
  crc_val        = $000C     ; alias of remain - the current CRC value (2 bytes)

;
; Note that at present addreses $00C0 to $00FF are available for your use.
;
