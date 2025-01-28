#include "Chip8.h"
#include <fstream>
#include <chrono>
#include <random>

const unsigned int START_ADDRESS = 0x200; // 0x000 to 0x1FF are reserved
const unsigned int FONTSET_SIZE = 80; // 16 characters, 5 bytes each
const unsigned int FONTSET_START_ADDRESS = 0x50;

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

Chip8::Chip8()
	: randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
	pc = START_ADDRESS; 

	// Load fontset into memory
	for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
	{
		memory[FONTSET_START_ADDRESS + i] = fontset[i];
	}

	randByte = std::uniform_int_distribution<uint8_t>(0, 255U);
}

void Chip8::LoadROM(char const* filename)
{
	// Open the file in binary mode and move the file pointer to the end
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open())
	{
		// Get the size of the file and allocate a buffer to hold the contents
		std::streampos size = file.tellg();
		char* buffer = new char[size];

		// Go back to the beginning of the file and read its contents into the buffer
		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();

		// Load the ROM contents into the Chip-8's memory
		for (long i = 0; i < size; ++i)
		{
			memory[START_ADDRESS + i] = buffer[i];
		}
		delete[] buffer;
	}
}

void Chip8::OP_00E0()
{
	// Clear the display by overwriting video memory with zeros
	memset(video, 0, sizeof(video));
}

void Chip8::OP_00EE()
{
	// Return from a subroutine by setting the program counter to the address at the top of the stack
	--sp;
	pc = stack[sp];
}

void Chip8::OP_1nnn()
{
	// Jump to address nnn by setting the program counter to nnn
	uint16_t address = opcode & 0x0FFF;
	pc = address;
}

void Chip8::OP_2nnn()
{
	// Call a subroutine at address nnn by adding the current address to the stack and setting the program counter to nnn
	stack[sp] = pc;
	++sp;
	uint16_t address = opcode & 0x0FFF;
	pc = address;
}

void Chip8::OP_3xkk()
{
	// Skip the next instruction if register Vx equals kk
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;
	if (registers[Vx] == byte)
	{
		pc += 2;
	}
}

void Chip8::OP_4xkk()
{
	// Skip the next instruction if register Vx doesn't equal kk
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;
	if (registers[Vx] != byte)
	{
		pc += 2;
	}
}

void Chip8::OP_5xy0()
{
	// Skip the next instruction if register Vx equals register Vy
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	if (registers[Vx] == registers[Vy])
	{
		pc += 2;
	}
}

void Chip8::OP_6xkk()
{
	// Set register Vx to kk
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;
	registers[Vx] = byte;
}

void Chip8::OP_7xkk()
{
	// Add kk to register Vx
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;
	registers[Vx] += byte;
}

void Chip8::OP_8xy0()
{
	// Set register Vx to the value of register Vy
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	registers[Vx] = registers[Vy];
}

void Chip8::OP_8xy1()
{
	// Set Vx to Vx OR Vy
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	registers[Vx] |= registers[Vy];
}

void Chip8::OP_8xy2()
{
	// Set Vx to Vx AND Vy
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	registers[Vx] &= registers[Vy];
}

void Chip8::OP_8xy3()
{
	// Set Vx to Vx XOR Vy
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	registers[Vx] ^= registers[Vy];
}

