# GBA-Basic-Frontenis
A repository to storage the source code of the homebrew game "Basic Frontenis" programmed for the Game Boy Advance.
The Frontenis Game Engine has been developed in C programming language and DEVKITADV develpment kit.

Basic-Frontenis for GBA.

Author: Ryoga a.k.a. JDURANMASTER

First step, on GBA programming. This module is a sample tetris-clone.

- Testing GBA mode 4, mode 3, double buffering, keyboard controlling.
- No sound implemented yet

Change the following compilation script if you need to do it in order to compile the source
code in your local system.

path=D:\devkitadv\bin

gcc -c -O3 -mthumb -mthumb-interwork BasicFrontenis.c
gcc -mthumb -mthumb-interwork -o BasicFrontenis.elf BasicFrontenis.o

objcopy -O binary BasicFrontenis.elf BasicFrontenis.gba

del *.elf
del *.o

pause


The project also includes pcx2sprite, pcx2gba and gbafix tools.