#pragma once

#include <cstdint>

const unsigned int REGISTER_COUNT = 16;
const unsigned int MEMORY_SIZE = 4096;
const unsigned int STACK_SIZE = 16;
const unsigned int KEY_COUNT = 16;
const unsigned int VIDEO_WIDTH = 64;
const unsigned int VIDEO_HEIGHT = 32;

// Chip-8 emulator class
class Chip8
{
private:
	uint8_t registers[REGISTER_COUNT]{}; // V0 to VF
	uint8_t memory[MEMORY_SIZE]{}; // 4KB memory
	uint16_t index{}; // Index register
	uint16_t pc{}; // Program counter
	uint16_t stack[STACK_SIZE]{}; // Stack for function calls
	uint8_t sp{}; // Stack pointer
	uint8_t delayTimer{}; // Delay timer
	uint8_t soundTimer{}; // Sound timer
	uint16_t opcode{}; // Current opcode

public:
	uint8_t keypad[KEY_COUNT]{}; // Hex keypad
	uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT]{}; // 64x32 monochrome display
};