;---------------------------
; standard library stuff
;---------------------------

;
;  character type masks
;
m_upper   = %00000001    ; $01
m_lower   = %00000010    ; $02
m_alpha   = %00000100    ; $04
m_digit   = %00001000    ; $08
m_xdigit  = %00010000    ; $10
m_space   = %00100000    ; $20
m_cntrl   = %01000000    ; $40
m_alnum   = %10000000    ; $80

;
;  for fast checks which corrupt registers
;
;    ldx THE_CHARACTER_TO_BE_TESTED
;    lda character_types_table,X
;    bne IS_IN_THAT_CLASS
;

character_types_table = *
  dfb $40,$40,$40,$40,$40,$40,$40,$40    ; 0x00
  dfb $40,$60,$60,$60,$60,$60,$40,$40    ; 0x08
  dfb $40,$40,$40,$40,$40,$40,$40,$40    ; 0x10
  dfb $40,$40,$40,$40,$40,$40,$40,$40    ; 0x18
  dfb $20,$00,$00,$00,$00,$00,$00,$00    ; 0x20   !"#$%&'
  dfb $00,$00,$00,$00,$00,$00,$00,$00    ; 0x28  ()*+,-./
  dfb $98,$98,$98,$98,$98,$98,$98,$98    ; 0x30  01234567
  dfb $98,$98,$00,$00,$00,$00,$00,$00    ; 0x38  89:;<=>?
  dfb $00,$95,$95,$95,$95,$95,$95,$85    ; 0x40  @ABCDEFG
  dfb $85,$85,$85,$85,$85,$85,$85,$85    ; 0x48  HIJKLMNO
  dfb $85,$85,$85,$85,$85,$85,$85,$85    ; 0x50  PQRSTUVW
  dfb $85,$85,$85,$00,$00,$00,$00,$00    ; 0x58  XYZ[\]^_
  dfb $00,$96,$96,$96,$96,$96,$96,$86    ; 0x60  `abcdefg
  dfb $86,$86,$86,$86,$86,$86,$86,$86    ; 0x68  hijklmno
  dfb $86,$86,$86,$86,$86,$86,$86,$86    ; 0x70  pqrstuvw
  dfb $86,$86,$86,$00,$00,$00,$00,$40    ; 0x78  xyz{|}~
  dfb $00,$00,$00,$00,$00,$00,$00,$00    ; 0x80
  dfb $00,$00,$00,$00,$00,$00,$00,$00    ; 0x88
  dfb $00,$00,$00,$00,$00,$00,$00,$00    ; 0x90
  dfb $00,$00,$00,$00,$00,$00,$00,$00    ; 0x98
  dfb $00,$00,$00,$00,$00,$00,$00,$00    ; 0xA0
  dfb $00,$00,$00,$00,$00,$00,$00,$00    ; 0xA8
  dfb $00,$00,$00,$00,$00,$00,$00,$00    ; 0xB0
  dfb $00,$00,$00,$00,$00,$00,$00,$00    ; 0xB8
  dfb $00,$00,$00,$00,$00,$00,$00,$00    ; 0xC0
  dfb $00,$00,$00,$00,$00,$00,$00,$00    ; 0xC8
  dfb $00,$00,$00,$00,$00,$00,$00,$00    ; 0xD0
  dfb $00,$00,$00,$00,$00,$00,$00,$00    ; 0xD8
  dfb $00,$00,$00,$00,$00,$00,$00,$00    ; 0xE0
  dfb $00,$00,$00,$00,$00,$00,$00,$00    ; 0xE8
  dfb $00,$00,$00,$00,$00,$00,$00,$00    ; 0xF0
  dfb $00,$00,$00,$00,$00,$00,$00,$00    ; 0xF8

;
;  character type tests: test character in A, Carry set if true, carry clear if false
;    -no registers changed
;
is_xxx_fail:
  plx
  pla
  clc
  rts

is_xxx_pass:
  plx
  pla
  sec
  rts

  .macro isXXX
\1 = *
  pha
  phx
  tax
  lda character_types_table,x
  and #\2
  beq is_xxx_fail
  bra is_xxx_pass
  .endmacro

;
;  generate the various character type tests - done by fast table lookup
;
  isXXX isupper,  m_upper
  isXXX islower,  m_lower
  isXXX isalpha,  m_alpha
  isXXX isdigit,  m_digit
  isXXX isxdigit, m_xdigit
  isXXX isspace,  m_space
  isXXX iscntrl,  m_cntrl
  isXXX isalnum,  m_alnum

;
;  ran out of bits so implemented this one manually
;
isbinary   =  *
         cmp  #'0'
         beq  isbin
         cmp  #'1'
         bne  notbin
isbin:
         sec
         rts
notbin:
         clc
         rts

;
; Move memory down  (mem_move_dest is < mem_move_src)
;
;  Copies first byte first, working upwards
;
; mem_move_src = source start address
; mem_move_dest = destination start address
; mem_move_len = number of bytes mem_move_dest move
;
movedown:
  lda mem_move_len
  ora mem_move_len+1
  beq movedown_done
  ldy #0
  sty  token_work      ; counter
  sty  token_work+1
;
;  copy one byte, increment counter
;
movedown_loop:
  lda (mem_move_src),Y
  sta (mem_move_dest),Y
  inc token_work
  bne movedown2
  inc token_work+1
;
;  see if length reached
;
movedown2:

  lda  token_work
  cmp  mem_move_len
  bne  movedown1
  lda  token_work+1
  cmp  mem_move_len+1
  bne  movedown1
movedown_done:
  rts

;
; increment Y, and increment high-order address bytes if necessary
;
movedown1:
  iny
  bne  movedown_loop
  inc  mem_move_src+1
  inc  mem_move_dest+1
  bra  movedown_loop

; Move memory up (mem_move_dest is > mem_move_src)
;
;   Copies last byte first, moving downwards
;
; mem_move_src = source start address
; mem_move_dest = destination start address
; mem_move_len = number of bytes mem_move_dest move
;
moveup:
  lda mem_move_len
  ora mem_move_len+1
  beq moveup_done
  ldy #0
  sty  token_work      ; counter
  sty  token_work+1
  ;
  ;  calculate final address of source
  ;
  clc
  lda mem_move_src
  adc mem_move_len
  sta mem_move_src
  lda mem_move_src+1
  adc mem_move_len+1
  sta mem_move_src+1
  ;
  ;  calculate final address of destination
  ;
  clc
  lda mem_move_dest
  adc mem_move_len
  sta mem_move_dest
  lda mem_move_dest+1
  adc mem_move_len+1
  sta mem_move_dest+1
  bra moveup_next   ; have to decrement first
;
; move one byte, count moves
;
moveup_loop:
  lda (mem_move_src),Y
  sta (mem_move_dest),Y
  inc token_work
  bne moveup3
  inc token_work+1
moveup3:
;
;  see if length reached
;
  lda token_work
  cmp mem_move_len
  bne moveup_next
  lda token_work+1
  cmp mem_move_len+1
  bne moveup_next
moveup_done:
  rts
;
;  decrement Y, and decrement high-order address bytes if necessary
;
moveup_next   =  *
  dey
  cpy #$FF
  bne moveup_loop
  dec mem_move_src+1
  dec mem_move_dest+1
  bra moveup_loop

;
;
; String compare - case significant (max 128 byte strings)
;
; SRCE = source start address
; DEST = destination start address
; Y    = length of both strings (range 1 to 128)
;
;  returns Z set if strings are equal
;

str_compare:
  dey             ; make zero-relative
  bmi str_compare_done
  lda (SRCE),y
  sta str_work
  lda (DEST),y
  cmp str_work
  beq str_compare
  rts   ; no match, zero flag not set
str_compare_done:
  lda #0    ; set Z flag
  rts

;
; String compare - case NOT significant (max 128 byte strings)
;
; SRCE = source start address
; DEST = destination start address
; Y    = length of both strings (range 1 to 128)
;
;  returns Z set if strings are equal
;

str_ic_compare:
  dey
  bmi str_ic_compare_done
  lda (SRCE),y
  cmp #'z'+1
  bcs str_ic_compare_fixed_srce  ; BGE
  cmp #'a'
  bcc str_ic_compare_fixed_srce  ; BLT
  sbc #$20    ; make upper-case
str_ic_compare_fixed_srce:
  sta str_work
  lda (DEST),y
  cmp #'z'+1
  bcs str_ic_compare_fixed_dest  ; BGE
  cmp #'a'
  bcc str_ic_compare_fixed_dest  ; BLT
  sbc #$20    ; make upper-case
str_ic_compare_fixed_dest:
  cmp str_work
  beq str_ic_compare
  rts       ; no match, zero flag not set
str_ic_compare_done:
  lda #0    ; set Z flag
  rts

