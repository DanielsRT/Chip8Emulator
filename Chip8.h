#pragma once

#include <cstdint>


class Chip8
{
public:
	uint8_t registers[16]{}; // V0 to VF
	uint8_t memory[4096]{}; // 4KB memory
	uint16_t index{}; // Index register
	uint16_t pc{}; // Program counter
	uint16_t stack[16]{}; // Stack for function calls
	uint8_t sp{}; // Stack pointer
	uint8_t delayTimer{}; // Delay timer
	uint8_t soundTimer{}; // Sound timer
	uint8_t keypad[16]{}; // Hex keypad
	uint32_t video[64 * 32]{}; // 64x32 monochrome display
};