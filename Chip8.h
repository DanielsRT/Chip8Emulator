#ifndef CHIP_8_H
#define CHIP_8_H

#include <stdint.h>

const unsigned int KEY_COUNT = 16;
const unsigned int MEMORY_SIZE = 4096;
const unsigned int REGISTER_COUNT = 16;
const unsigned int STACK_LEVELS = 16;
const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;

class Chip8 {
private:
    uint16_t stack[STACK_LEVELS]; // Stack
    uint16_t sp; // Stack pointer
    uint8_t memory[MEMORY_SIZE]; // Memory (4k)
    uint8_t registers[KEY_COUNT]; // V registers (V0-VF)
    uint16_t pc; // Program counter
    uint16_t opcode; // Current op code
    uint16_t index; // Index register
    uint8_t delayTimer; // Delay timer
    uint8_t soundTimer; // Sound timer

public:
    Chip8();
    void Cycle();
    bool LoadROM(const char* file_path);

    uint8_t  video[VIDEO_WIDTH * VIDEO_HEIGHT]; // Graphics buffer
    uint8_t  keypad[KEY_COUNT]; // Keypad
    bool drawFlag; // Indicates a draw has occurred
};

#endif