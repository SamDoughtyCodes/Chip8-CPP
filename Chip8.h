#ifndef CHIP8_H
#define CHIP8_H
#include <cstdint>
#include <random>

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

    private:
        // Attributes
        std::default_random_engine randGen;                 // Engine to generate a random number
        std::uniform_int_distribution<uint8_t> randByte;    // Used to store a byte of random data

        // Opcodes
        void OP_00E0();                     // OPCODE 00E0 -> CLS: Clears the display
        void OP_00EE();                     // OPCODE 00EE -> RET: Return from a subroutine
        void OP_1nnn();                     // OPCODE 1nnn -> JUMP addr: Jumps to addr nnn
        void OP_2nnn();                     // OPCODE 2nnn -> CALL addr: Call subroutine at addr nnn
        void OP_3xkk();                     // OPCODE 3xkk -> SE Vx kk: Skip next if Vx (register x) and kk (1 byte of data) are equal
        void OP_4xkk();                     // OPCODE 4xkk -> SNE Vx kk: Skip next if Vx != kk
        void OP_5xy0();                     // OPCODE 5xy0 -> SE Vx Vy: Skip next if Vx == Vy
        void OP_6xkk();                     // OPCODE 6xkk -> LD Vx kk: Load Vx with byte kk (set Vx == kk)
        void OP_7xkk();                     // OPCODE 7xkk -> ADD Vx kk: Add kk to the contents of Vx (set Vx += kk)
        void OP_8xy0();                     // OPCODE 8xy0 -> LD Vx Vy: Load Vy into Vx (set Vx = Vy)
        void OP_8xy1();                     // OPCODE 8xy1 -> OR Vx Vy: Set Vx equal to result of Vx OR Vy (Vx = Vx | Vy)
        void OP_8xy2();                     // OPCODE 8xy2 -> AND Vx Vy: Set Vx = Vx & Vy
        void OP_8xy3();                     // OPCODE 8xy3 -> XOR Vx Vy: Set Vx = Vx ^ Vy
        void OP_8xy4();                     // OPCODE 8xy4 -> ADD Vx Vy: Set Vx += Vy, with VF holding any overflow (set to 1).
        void OP_8xy5();                     // OPCODE 8xy5 -> SUB Vx Vy: Set Vx -= Vy. If Vx > Vy, VF is set to 1
        void OP_8xy6();                     // OPCODE 8xy6 -> SHR Vx: Shift Vx value right 1 bit, with LSB stored in VF
};

#endif
