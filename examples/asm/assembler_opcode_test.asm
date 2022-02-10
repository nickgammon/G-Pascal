; Assembler opcode test


    LIST

    rts      ; in case you run this file - just return

; OPCODE_4_CHAR_BRANCH: zp,r

    bbr1 $45,bar  ; 1F 45 2A
    bbr2 $45,bar  ; 2F 45 27
    bbr3 $45,bar  ; 3F 45 24
    bbr4 $45,bar  ; 4F 45 21
    bbr5 $45,bar  ; 5F 45 1E
    bbr6 $45,bar  ; 6F 45 1B
    bbr7 $45,bar  ; 7F 45 18
    bbs0 $45,bar  ; 8F 45 15
    bbs1 $45,bar  ; 9F 45 12
    bbs2 $45,bar  ; AF 45 0F
    bbs3 $45,bar  ; BF 45 0C
    bbs4 $45,bar  ; CF 45 09
    bbs5 $45,bar  ; DF 45 06
    bbs6 $45,bar  ; EF 45 03
    bbs7 $45,bar  ; FF 45 00
bar bbr0 $45,bar  ; 0F 45 FD


; OPCODE_3_CHAR_BRANCH: r


    bcc bar  ; 90 FB
    bcs bar  ; B0 F9
    beq bar  ; F0 F7
    bmi bar  ; 30 F5
    bne bar  ; D0 F3
    bpl bar  ; 10 F1
    bra bar  ; 80 EF
    bvc bar  ; 50 ED
    bvs bar  ; 70 EB


; OPCODE_3_CHAR_IMPLIED: (no argument)

  brk  ; 00
  clc  ; 18
  cld  ; D8
  cli  ; 58
  clv  ; B8
  dex  ; CA
  dey  ; 88
  inx  ; E8
  iny  ; C8
  nop  ; EA
  pha  ; 48
  php  ; 08
  phx  ; DA
  phy  ; 5A
  pla  ; 68
  plp  ; 28
  plx  ; FA
  ply  ; 7A
  rti  ; 40
  rts  ; 60
  sec  ; 38
  sed  ; 58
  sei  ; 78
  stp  ; DB
  tax  ; AA
  tay  ; A8
  tsx  ; BA
  txa  ; 8A
  txs  ; 9A
  tya  ; 98
  wai  ; CB

; OPCODE_4_CHAR_ZERO_PAGE: zp

  smb0    $42   ; 87 42
  smb1    $42   ; 97 42
  smb2    $42   ; A7 42
  smb3    $42   ; B7 42
  smb4    $42   ; C7 42
  smb5    $42   ; D7 42
  smb6    $42   ; E7 42
  smb7    $42   ; F7 42
  rmb0    $42   ; 07 42
  rmb1    $42   ; 17 42
  rmb2    $42   ; 27 42
  rmb3    $42   ; 37 42
  rmb4    $42   ; 47 42
  rmb5    $42   ; 57 42
  rmb6    $42   ; 67 42
  rmb7    $42   ; 77 42

; ASS_OPERAND_ABSOLUTE: a

  adc  $1234    ; 6D 34 12
  and  $1234    ; 2D 34 12
  asl  $1234    ; 0E 34 12
  bit  $1234    ; 2C 34 12
  cmp  $1234    ; CD 34 12
  cpx  $1234    ; EC 34 12
  cpy  $1234    ; CC 34 12
  dec  $1234    ; CE 34 12
  eor  $1234    ; 4D 34 12
  inc  $1234    ; EE 34 12
  jmp  $1234    ; 4C 34 12
  jsr  $1234    ; 20 34 12
  lda  $1234    ; AD 34 12
  ldx  $1234    ; AE 34 12
  ldy  $1234    ; AC 34 12
  lsr  $1234    ; 4E 34 12
  ora  $1234    ; 0D 34 12
  rol  $1234    ; 2E 34 12
  ror  $1234    ; 6E 34 12
  sbc  $1234    ; ED 34 12
  sta  $1234    ; 8D 34 12
  stx  $1234    ; 8E 34 12
  sty  $1234    ; 8C 34 12
  stz  $1234    ; 9C 34 12
  trb  $1234    ; 1C 34 12
  tsb  $1234    ; 0C 34 12

; ASS_OPERAND_ABSOLUTE_INDEXED_INDIRECT: (a,x)

  JMP ($5678,x)  ; 7C 78 56

; ASS_OPERAND_ABSOLUTE_INDEXED_WITH_X: a,x


  adc  $7654,x   ; 7D  54 76
  and  $7654,x   ; 3D  54 76
  asl  $7654,x   ; 1E  54 76
  bit  $7654,x   ; 3C  54 76
  cmp  $7654,x   ; DD  54 76
  dec  $7654,x   ; DE  54 76
  eor  $7654,x   ; 5D  54 76
  inc  $7654,x   ; FE  54 76
  lda  $7654,x   ; BD  54 76
  ldy  $7654,x   ; BC  54 76
  lsr  $7654,x   ; 5E  54 76
  ora  $7654,x   ; 1D  54 76
  rol  $7654,x   ; 3E  54 76
  ror  $7654,x   ; 7E  54 76
  sbc  $7654,x   ; FD  54 76
  sta  $7654,x   ; 9D  54 76
  stz  $7654,x   ; 9E  54 76

; ASS_OPERAND_ABSOLUTE_INDEXED_WITH_Y: a,y

  adc  $4321,y   ; 79 21 43
  and  $4321,y   ; 39 21 43
  cmp  $4321,y   ; D9 21 43
  eor  $4321,y   ; 59 21 43
  lda  $4321,y   ; B9 21 43
  ldx  $4321,y   ; BE 21 43
  ora  $4321,y   ; 19 21 43
  sbc  $4321,y   ; F9 21 43
  sta  $4321,y   ; 99 21 43

; ASS_OPERAND_ACCUMULATOR_A: A


  asl  A  ; 0A
  dec  A  ; 3A
  inc  A  ; 1A
  lsr  A  ; 4A
  rol  A  ; 2A
  ror  A  ; 6A

; ASS_OPERAND_ABSOLUTE_INDIRECT

  JMP   ($6543)  ; 6C 43 65

; ASS_OPERAND_IMMEDIATE

  adc   #$42     ; 69  42
  and   #$42     ; 29  42
  bit   #$42     ; 89  42
  cmp   #$42     ; C9  42
  cpx   #$42     ; E0  42
  cpy   #$42     ; C0  42
  eor   #$42     ; 49  42
  lda   #$42     ; A9  42
  ldx   #$42     ; A2  42
  ldy   #$42     ; A0  42
  ora   #$42     ; 09  42
  sbc   #$42     ; E9  42

; ASS_OPERAND_ZERO_PAGE: zp

  adc   $78    ; 65 78
  and   $78    ; 25 78
  asl   $78    ; 06 78
  bit   $78    ; 24 78
  cmp   $78    ; C5 78
  cpx   $78    ; E4 78
  cpy   $78    ; C4 78
  dec   $78    ; C6 78
  eor   $78    ; 45 78
  inc   $78    ; E6 78
  lda   $78    ; A5 78
  ldx   $78    ; A6 78
  ldy   $78    ; A4 78
  lsr   $78    ; 46 78
  ora   $78    ; 05 78
  rol   $78    ; 26 78
  ror   $78    ; 66 78
  sbc   $78    ; E5 78
  sta   $78    ; 85 78
  stx   $78    ; 86 78
  sty   $78    ; 84 78
  stz   $78    ; 64 78
  trb   $78    ; 14 78
  tsb   $78    ; 04 78

; ASS_OPERAND_ZERO_PAGE_INDEXED_INDIRECT: (zp,x)

  adc   ($55,x)  ; 61  55
  and   ($55,x)  ; 21  55
  cmp   ($55,x)  ; C1  55
  eor   ($55,x)  ; 41  55
  lda   ($55,x)  ; A1  55
  ora   ($55,x)  ; 01  55
  sbc   ($55,x)  ; E1  55
  sta   ($55,x)  ; 81  55

; ASS_OPERAND_ZERO_PAGE_INDEXED_WITH_X: zp,x

  adc   $66,x   ; 75  66
  and   $66,x   ; 35  66
  asl   $66,x   ; 16  66
  bit   $66,x   ; 34  66
  cmp   $66,x   ; D5  66
  dec   $66,x   ; D6  66
  eor   $66,x   ; 55  66
  inc   $66,x   ; F6  66
  lda   $66,x   ; B5  66
  ldy   $66,x   ; B4  66
  lsr   $66,x   ; 56  66
  ora   $66,x   ; 15  66
  rol   $66,x   ; 36  66
  ror   $66,x   ; 76  66
  sbc   $66,x   ; F5  66
  sta   $66,x   ; 95  66
  sty   $66,x   ; 94  66
  stz   $66,x   ; 74  66

; ASS_OPERAND_ZERO_PAGE_INDEXED_WITH_Y: zp,y

  ldx   $32,y   ; B6  32
  stx   $32,y   ; 96  32

; ASS_OPERAND_ZERO_PAGE_INDIRECT: (zp)

  adc   ($37)   ; 72  37
  and   ($37)   ; 32  37
  cmp   ($37)   ; D2  37
  eor   ($37)   ; 52  37
  lda   ($37)   ; B2  37
  ora   ($37)   ; 12  37
  sbc   ($37)   ; F2  37
  sta   ($37)   ; 92  37

; ASS_OPERAND_ZERO_PAGE_INDIRECT_INDEXED_WITH_Y: (zp),y

  adc   ($98),y   ; 71  98
  and   ($98),y   ; 31  98
  cmp   ($98),y   ; D1  98
  eor   ($98),y   ; 51  98
  lda   ($98),y   ; B1  98
  ora   ($98),y   ; 11  98
  sbc   ($98),y   ; F1  98
  sta   ($98),y   ; 91  98
