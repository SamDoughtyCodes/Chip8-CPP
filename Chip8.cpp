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

// 5xy0 -> SE Vx Vy: Skip if Vx == Vy
void Chip8::OP_5xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Extract Vx from opcode
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;  // Extract Vy from opcode
    if (registers[Vx] == registers[Vy]) {   // If Vx == Vy
        pc += 2;                            // Increment the program counter to skip the next step
    }
}

// 6xkk -> LD Vx kk: Set Vx == kk
void Chip8::OP_6xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Extract Vx from opcode
    uint8_t byte = opcode & 0x00FFu;        // Extract kk from opcode
    registers[Vx] = byte;                   // Load byte into register Vx
}

// 7xkk -> ADD Vx kk: Set Vx += kk
void Chip8::OP_7xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Extract Vx from opcode
    uint8_t byte = opcode & 0x00FFu;        // Extract kk from opcode
    registers[Vx] += byte;                  // Add kk to the contents of Vx
}

// 8xy0 -> LD Vx Vy: Set Vx = Vy
void Chip8::OP_8xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Extract Vx from opcode
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;  // Extract Vy from opcode
    registers[Vx] = registers[Vy];          // Set Vx = Vy
}

// 8xy1 -> OR Vx Vy: Set Vx = Vx | Vy
void Chip8::OP_8xy1() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Extract Vx from opcode
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;  // Extract Vy from opcode
    registers[Vx] |= registers[Vy];         // |= is shorthand for Vx = Vx | Vy
}

// 8xy2 -> AND Vx Vy: Set Vx = Vx & Vy
void Chip8::OP_8xy2() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Extract Vx from opcode
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;  // Extract Vy from opcode
    registers[Vx] &= registers[Vy];
}

// 8xy3 -> XOR Vx Vy: Set Vx = Vx ^ Vy
void Chip8::OP_8xy3() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Extract Vx from opcode
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;  // Extract Vy from opcode
    registers[Vx] ^= registers[Vy];
}

// 8xy4 -> ADD Vx Vy; Set Vx += Vy (VF stores overflow)
void Chip8::OP_8xy4() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Extract Vx from opcode
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;  // Extract Vy from opcode
    uint16_t sum = registers[Vx] + registers[Vy];  // Sum Vx and Vy
    if (sum > 255U) {                       // If the result is greater than 1 byte
        registers[0xF] = 1;                 // Set VF to 1 to represent overflow
    } else {                                // If the result is less than 1 byte
        registers[0xF] = 0;                 // Set VF to 0
    }
    registers[Vx] = sum & 0xFFu;            // Set Vx to lowest 8 bits of sum
}

// 8xy5 -> SUB Vx Vy: Set Vx -= Vy (VF stores 1 if Vx > VY)
void Chip8::OP_8xy5() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Extract Vx from opcode
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;  // Extract Vy from opcode
    if (registers[Vx] > registers[Vy]) {    // If Vx > Vy
        registers[0xF] = 1;                 // Set VF to 1
    } else {
        registers[0xF] = 0;                 // Set VF to 0
    }
}

// 8xy6 -> SHR Vx: Shift right Vx 1 time (LSB to VF)
void Chip8::OP_8xy6() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Extract Vx from opcode
    registers[0xF] = registers[Vx] & 0x1u;  // Extract LSB and store in VF
    registers[Vx] >>= 1;                    // Shorthand for Vx = Vx >> 1
}

// 8xy7 -> SUBN Vx Vy: Vx = Vy-Vx (VF = 1 if Vx < Vy)
void Chip8::OP_8xy7() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Extract Vx from opcode
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;  // Extract Vy from opcode
    if (registers[Vx] < registers[Vy]) {    // If Vx < Vy
        registers[0xF] = 1;                 // Set VF = 1
    } else {
        registers[0xF] = 0;
    }
    registers[Vx] = registers[Vy] - registers[Vx];
}

// 8xyE -> SHL Vx: Shift left Vx by 1 bit
void Chip8::OP_8xyE() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Extract Vx from opcode
    registers[0xF] = (registers[Vx] & 0x80u) >> 7u;  // Extract MSB from Vx value and store to VF
    registers[Vx] <<= 1;                    // Shorthand for Vx = Vx << 1
}

// 9xy0 -> SNE Vx Vy: Skip next if Vx != Vy
void Chip8::OP_9xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Extract Vx from opcode
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;  // Extract Vy from opcode
    if (registers[Vx] != registers[Vy]) {   // If Vx != Vy
        pc += 2;                            // Increment PC by 2, to skip next instruction
    }
}

// Annn -> LD I addr: Load I with value nnn
void Chip8::OP_Annn() {
    uint16_t address = opcode & 0x0FFFu;    // Extract nnn from opcode
    index = address;                        // Set the index register with value nnn
}

// Bnnn -> JP V0 nnn: Jump to location V0 + nnn
void Chip8::OP_Bnnn() {
    uint16_t address = opcode & 0x0FFFu;    // Extract nnn from opcode
    pc = registers[0] + address;            // PC = V0 + nnn
}

// Cxkk -> RND Vx kk: Set Vx to randomByte & kk
void Chip8::OP_Cxkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Extract Vx from opcode
    uint8_t byte = opcode & 0x00FFu;        // Extract kk from opcode
    registers[Vx] = randByte(randGen) & byte;
}
