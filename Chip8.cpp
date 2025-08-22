#include "Chip8.h"
#include <fstream>  // File operations
#include <chrono>   // Time functions

const unsigned int START_ADDR = 0x200;          // Set the start address for the PC, 0x000 to 0x1FF are reserved
const unsigned int FONTSET_START_ADDR = 0x50;   // Set the start address for where the font is stored

/* ------------------------- CONSTRUCTOR ------------------------- */
Chip8::Chip8()
// Initialisers
: randGen(std::chrono::system_clock::now().time_since_epoch().count()),  // Seed the RNG using the current time
  randByte(0, 255)  // Generate a random num one byte in size (0 to 255)
{
    pc = START_ADDR;  // Set the starting address of the Chip8 to 0x200

    // Load font set into the memory
    for (unsigned int i = 0; i < FONTSET_SIZE; ++i) {
        memory[FONTSET_START_ADDR + i] = fontset[i];
    }
}

/* ----------------- FUNCTION TO LOAD A ROM FILE ----------------- */
void Chip8::LoadROM(char const* filename) {
    // Open binary file and move pointer to end
    std::fstream file(filename, std::ios::binary | std::ios::ate);  // Obj called file of type std::fstream

    if (file.is_open()){
        // Find size of file and create buffer of this size
        std::streampos size = file.tellg(); // .tellg() returns the current position of the file pointer (in this case the end)
        char* buffer = new char[size];      // Buffer now points to the first addr of some contiguous memory the same size as the ROM
        
        // Read file to the buffer
        file.seekg(0, std::ios::beg);       // With no offset (go all the way), seek the pointer to the beginning of the file
        file.read(buffer, size);            // Read the file to the buffer array, for the size of the file
        file.close();                       // We are done with the file! Let's close it!

        // Load the buffer into the Chip8's memory
        for (long i=0; i < size; ++i) {memory[START_ADDR + i] = buffer[i];}

        delete[] buffer;                    // Free the buffer memory
    }
}

/* --------------------- SET UP FONT DETAILS --------------------- */
/*
The font is stored as a sprite. Each character consists of 5 rows of 8 numbers, so 1 byte (5 bytes size).
Take F for example:
11110000
10000000
11110000
10000000
10000000
The first row is the hex number 0xF0.
We will store the whole character set using 16 sets of 5 bytes.
*/
const unsigned int FONTSET_SIZE = 80;
uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
