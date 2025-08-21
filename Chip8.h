#ifndef CHIP8_H
#define CHIP8_H
#include <cstdint>

class Chip8 {
    public:
        // Attributes
        uint8_t registers[16]{};            // 16x 8-bit registers, all initialised as 0
        uint8_t memory[4096]{};             // 4KB of memory
        uint16_t index{};                   // Index register
        uint16_t pc{};                      // Program counter
        uint16_t stack[16]{};               // 16-layer stack
        uint8_t sp{};                       // Stack pointer
        uint8_t delayTimer{};
        uint8_t soundTimer{};
        uint8_t keypad[16]{};               // Keypad keys 0 to F
        uint32_t video[64 * 32]{};          // 64x32 monochrome display (32-bit to help with SDL)
        uint16_t opcode;                    // Opcode of instruction, not initialised

        // Methods
        Chip8();                            // Constructor
        void LoadROM(char const* filename); // Method to load a ROM file
};

#endif // CHIP8_H
