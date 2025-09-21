# Chip8-CPP
Chip8 emulator built in c++
Tutoial can be found [here](https://austinmorlan.com/posts/chip8_emulator)

## Chip8 Specification
- 16x 8-bit general purpose registers
- 4096B memory
- 16-bit index register
- 16-bit program counter (PC)
- 16 level stack + 8-bit stack pointer
- 8-bit delay timer
- 8-bit sound timer
- 16x inputs (corresponding with 0 to F)
- 64x32 monochrome display

## Opcodes
The full list of opcodes can be found [here](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM).
Opcodes are briefly described in the header file `Chip8.h`, to help understand their meaning.

## Function Pointer Table
Right lets figure this thing out...
We essentially make a big ol array, and use the provided opcode as an index. This means the array must be big enough for every possible opcode.
The 1st dimension of the array must be able to accomodate up to $F indexes, and then other dimesnions are used to accomodate the next characters of the opcode.
