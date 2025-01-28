#pragma once

#include <cstdint>
#include <random>

const unsigned int REGISTER_COUNT = 16;
const unsigned int MEMORY_SIZE = 4096;
const unsigned int STACK_SIZE = 16;
const unsigned int KEY_COUNT = 16;
const unsigned int VIDEO_WIDTH = 64;
const unsigned int VIDEO_HEIGHT = 32;

// Chip-8 emulator class
class Chip8
{
public:
	Chip8();
	void LoadROM(const char* filename);

	uint8_t keypad[KEY_COUNT]{}; // Hex keypad
	uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT]{}; // 64x32 monochrome display

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

	std::default_random_engine randGen; // Random number generator
	std::uniform_int_distribution<uint8_t> randByte; // Random byte

	// Opcode functions
	void OP_00E0(); // CLS
	void OP_00EE(); // RET
	void OP_1nnn(); // JP addr
	void OP_2nnn(); // CALL addr
	void OP_3xkk(); // SE Vx, byte
	void OP_4xkk(); // SNE Vx, byte
	void OP_5xy0(); // SE Vx, Vy
	void OP_6xkk(); // LD Vx, byte
	void OP_7xkk(); // ADD Vx, byte
	void OP_8xy0(); // LD Vx, Vy
	void OP_8xy1(); // OR Vx, Vy
	void OP_8xy2(); // AND Vx, Vy
	void OP_8xy3(); // XOR Vx, Vy
	void OP_8xy4(); // ADD Vx, Vy
};