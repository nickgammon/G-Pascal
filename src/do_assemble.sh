#
#  Assemble the assembler
#
vasm6502_oldstyle gpascal.asm  -wdc02 -esc -dotdir -L ../list/gpascal.list -Fbin -o ../bin/gpascal.bin
#
#  Get rid of any emulator assembler output
#
rm -f gpascal.bin
rm -f gpascal.list
