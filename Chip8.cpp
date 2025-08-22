#include "Chip8.h"
#include <fstream>  // File operations
#include <chrono>   // Time functions
#include <string.h> // To use memset

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

/* --------------------------- OPCODES --------------------------- */
// 00E0 -> CLS: Clears the display
void Chip8::OP_00E0() {
    memset(video, 0, sizeof(video));    // Sets entire video buffer (display) to 0s
}

// 00EE -> RET: Returns from a subroutine
void Chip8::OP_00EE() {
    --sp;                               // Decrement stack pointer (preparing to pop)
    pc = stack[sp];                     // Pop contents of stack into program counter
}

// 1nnn -> JUMP addr: Jumps to addr nnn
void Chip8::OP_1nnn() {
    /*
    NOTE: & performs bitwise AND with mask 0x0FFFu, so for Chip8's 2-byte instructions,
    it removes the first nible (the instruction type) and keeps the address nibbles (nnn)
    */
    uint16_t address = opcode & 0x0FFFu;  // Set the address to jump to
    pc = address;                         // Replace the program counter value with the address
}

// 2nnn -> CALL addr: Call subroutine at addr nnn
void Chip8::OP_2nnn() {
    uint16_t address = opcode & 0x0FFFu;  // Set the address of the subroutine
    stack[sp] = pc;                       // Add the current contents of the program counter to the stack
    ++sp;                                 // Increment the stack pointer
    pc = address;                         // Set the program counter to the new address
}

// 3xkk -> SE Vx kk: Skip if Vx == kk
void Chip8::OP_3xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Find Vx by bitwise AND-ing x from the opcode and bit-shifting 8-bits to the right (to remove leftover 0s from AND)
    uint8_t byte = opcode & 0x00FFu;        // Extract the byte to compare to (kk)
    if (registers[Vx] == byte) {            // If Vx == kk
        pc += 2;                            // Increment PC by 2 (as memory is 1 byte but instructions are 2 bytes)
    }
}

// 4xkk -> SNE Vx kk: Skip if Vx != kk
void Chip8::OP_4xkk() {
    // See Chip8::OP_3xkk for explanation of code
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;
    if (registers[Vx] != byte) {
        pc += 2;
    }
}
