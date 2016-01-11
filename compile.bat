path=D:\devkitadv\bin

gcc -c -O3 -mthumb -mthumb-interwork BasicFrontenis.c
gcc -mthumb -mthumb-interwork -o BasicFrontenis.elf BasicFrontenis.o

objcopy -O binary BasicFrontenis.elf BasicFrontenis.gba

del *.elf
del *.o

pause
