#include "Chip8.h"
#include <fstream>  // File operations

// Set the start address for the PC, 0x000 to 0x1FF are reserved
const unsigned int START_ADDR = 0x200;

/* ------------------------- CONSTRUCTOR ------------------------- */
Chip8::Chip8() {
    pc = START_ADDR;  // Set the starting address of the Chip8 to 0x200
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