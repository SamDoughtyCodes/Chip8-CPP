#include "Chip8.h"
#include <fstream>  // File operations
#include <chrono>   // Time functions
#include <string.h> // To use memset

const unsigned int START_ADDR = 0x200;          // Set the start address for the PC, 0x000 to 0x1FF are reserved
const unsigned int FONTSET_START_ADDR = 0x50;   // Set the start address for where the font is stored
const unsigned int VIDEO_HEIGHT = 32;           // Stores height of the display
const unsigned int VIDEO_WIDTH = 64;            // Stores width of the display

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

// Dxyn -> DRW Vx Vy nibble: Draw sprite in index reg, at (Vx,Vy). (Collision? Stored in VF)
void Chip8::OP_Dxyn() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Extract Vx from opcode
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;  // Extract Vy from opcode
    uint8_t rows = opcode & 0x00Fu;         // Extract n from opcode, this stores the number of rows in the sprite

    // Use mod to wrap the sprite around the page, or just calculate the coordinate positions
    uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
    uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

    registers[0xF] = 0;                     // Set VF to 0, as currently there are no collisions

    /*
    NOTE: How this all crazy shit works...
    1 - Use the rows variable to iterate over the rows of the sprite
    2 - Use spriteByte to store the byte value of the current row (see fontset)
    3 - Iterate over the collumns of that row, storing the value of each pixel in the spritePixel var. This works by extracting the bit to display from the row's byte
    4 - Store memory address of display pixel in screenPixel pointer
    */
    for (unsigned int row = 0; row < rows; ++row) {  // Iterate over the rows of the sprite
        uint8_t spriteByte = memory[index + row];    // Get the byte of the current row to display from the index register's current value
        for (unsigned int col = 0; col < 8; ++col) { // Iterate over the rows of the sprite (all sprites are 8 pixels wide)
            uint8_t spritePixel = spriteByte & (0x80u >> col);  // Fancy bit-shifing/masking to get the right bit to display
            uint32_t* screenPixel = &video[((yPos + row) * VIDEO_WIDTH) + (xPos + col)];  // Fancy maths to get to the display pixel of the current bit. Pointer explained in above doc-comment (BP4)
            if (spritePixel) {                       // If value of pixel needs to be 1 (on)
                if (*screenPixel == 0xFFFFFFFF) {    // If the value of the pixel is already 1 (condition)
                    registers[0xF] = 1;              // Set VF to 1 to indicate a collision
                }
                *screenPixel ^= 0xFFFFFFFF;          // XOR the display's pixel
            }
        }
    }
}

// Ex9E -> SKP Vx: Skip next if key of value Vx is pressed
void Chip8::OP_Ex9E() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Extract Vx from opcode
    uint8_t key = registers[Vx];            // Get the value of the key to check
    if (keypad[key]) {                      // If the keypad key is pressed
        pc += 2;                            // Increment PC by 2 to skip next instruction
    }
}

// ExA1 -> SKNP Vx: Skip next if key of value Vx is NOT pressed
void Chip8::OP_ExA1() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Extract Vx from opcode
    uint8_t key = registers[Vx];            // Get the value of the key to check
    if (!keypad[key]) {                     // If the keypad key is NOT pressed
        pc += 2;                            // Increment PC by 2 to skip next instruction
    }
}

// Fx07 -> LD Vx DT: Set Vx = delayTimer
void Chip8::OP_Fx07() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;  // Extract Vx from opcode
    registers[Vx] = delayTimer;             // Set Vx = delayTimer
}

// Fx0A -> LD Vx: Set Vx to the value of the keypress, wait for the keypress
void Chip8::OP_Fx0A() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	if (keypad[0])
	{
		registers[Vx] = 0;
	}
	else if (keypad[1])
	{
		registers[Vx] = 1;
	}
	else if (keypad[2])
	{
		registers[Vx] = 2;
	}
	else if (keypad[3])
	{
		registers[Vx] = 3;
	}
	else if (keypad[4])
	{
		registers[Vx] = 4;
	}
	else if (keypad[5])
	{
		registers[Vx] = 5;
	}
	else if (keypad[6])
	{
		registers[Vx] = 6;
	}
	else if (keypad[7])
	{
		registers[Vx] = 7;
	}
	else if (keypad[8])
	{
		registers[Vx] = 8;
	}
	else if (keypad[9])
	{
		registers[Vx] = 9;
	}
	else if (keypad[10])
	{
		registers[Vx] = 10;
	}
	else if (keypad[11])
	{
		registers[Vx] = 11;
	}
	else if (keypad[12])
	{
		registers[Vx] = 12;
	}
	else if (keypad[13])
	{
		registers[Vx] = 13;
	}
	else if (keypad[14])
	{
		registers[Vx] = 14;
	}
	else if (keypad[15])
	{
		registers[Vx] = 15;
	}
	else
	{
		pc -= 2;                                // Decreent the PC to stop the next instruction from execiting (emulate a "wait" for a keypress)
	}
}

// Fx15 -> LD DT Vx: Set delay timer = Vx
void Chip8::OP_Fx15() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;      // Extract Vx from opcode
    delayTimer = registers[Vx];                 // Set delay timer to Vx contents
}

// Fx18 -> LD ST Vx: Set sound timer = Vx
void Chip8::OP_Fx18() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;      // Extract Vx from opcode
    soundTimer = registers[Vx];                 // Set sound timer to Vx contents
}

// Fx1E -> ADD I Vx: Add Vx to index reg.
void Chip8::OP_Fx1E() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;      // Extract Vx from opcode
    index += registers[Vx];                     // Add Vx to index register
}

// Fx29 -> LD F Vx: Load starting address of sprite in Vx to index reg.
void Chip8::OP_Fx29() {
    /* NOTE:
    This is a bit of a weird opcode, so I shall try to explain it:
    1 - The fontset is stored starting from memory address 0x50, and each sprite is 5 bytes long
    2 - We find the digit stored in register Vx
    3 - We use the FONTSET_START_ADDRESS const to find the starting address of the sprite
    4 - We load this address into th eindex register
    */
   uint8_t Vx = (opcode & 0x0F00u) >> 8u;       // Extract Vx from opcode
   uint8_t digit = registers[Vx];               // Store value of reg Vx
   index = FONTSET_START_ADDR + (5 * digit);    // Find starting address of digit and store in index
}

// Fx33 -> LD B Vx: Load BCD value of Vx into index (I), I+1 and I+2 respectively
void Chip8::OP_Fx33() {
    /* NOTE:
    If you are at all like me when I first saw this, you might be asking, what the hell is a BCD value?????
    BCD (Binary Coded Decimal) is a hybrid format. For our 8 bit number, we can store 0-255, so all values will have a hundreds, tens, and units column.
    Within BCD, our 3 memory locations each store one of these columns, with I storing hundreds, I+1 tens, and I+2 units.

    The maths to do this isn't as bad as you'd think:
    1 - Take MOD 10 of value to get final digit
    2 - Divide by 10 to remove final digit (integer types cause integer division)
    3 - Repeat for next place
    */
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;      // Extract Vx from opcode
    uint8_t value = registers[Vx];              // Store value of reg Vx
    for (int place = 2; place >= 0; --place) {  // Iterate 2 to 0 (2, 1, 0)
        memory[index + place] = value % 10;     // Store the final digit of the number in memory
        value /= 10;                            // Divide the value by 10 to remove the final digit
    }
}

// Fx55 -> LD I Vx: Load registers V0 to Vx into memory starting at index location

