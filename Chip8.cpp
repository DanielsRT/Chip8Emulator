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

	// Function pointer table
	table[0x0] = &Chip8::Table0;
	table[0x1] = &Chip8::OP_1nnn;
	table[0x2] = &Chip8::OP_2nnn;
	table[0x3] = &Chip8::OP_3xkk;
	table[0x4] = &Chip8::OP_4xkk;
	table[0x5] = &Chip8::OP_5xy0;
	table[0x6] = &Chip8::OP_6xkk;
	table[0x7] = &Chip8::OP_7xkk;
	table[0x8] = &Chip8::Table8;
	table[0x9] = &Chip8::OP_9xy0;
	table[0xA] = &Chip8::OP_Annn;
	table[0xB] = &Chip8::OP_Bnnn;
	table[0xC] = &Chip8::OP_Cxkk;
	table[0xD] = &Chip8::OP_Dxyn;
	table[0xE] = &Chip8::TableE;
	table[0xF] = &Chip8::TableF;

	//Table 0
	table0[0x0] = &Chip8::OP_00E0;
	table0[0xE] = &Chip8::OP_00EE;

	//Table 8
	table8[0x0] = &Chip8::OP_8xy0;
	table8[0x1] = &Chip8::OP_8xy1;
	table8[0x2] = &Chip8::OP_8xy2;
	table8[0x3] = &Chip8::OP_8xy3;
	table8[0x4] = &Chip8::OP_8xy4;
	table8[0x5] = &Chip8::OP_8xy5;
	table8[0x6] = &Chip8::OP_8xy6;
	table8[0x7] = &Chip8::OP_8xy7;
	table8[0xE] = &Chip8::OP_8xyE;

	//Table E
	tableE[0x1] = &Chip8::OP_ExA1;
	tableE[0xE] = &Chip8::OP_Ex9E;

	//Table F
	tableF[0x07] = &Chip8::OP_Fx07;
	tableF[0x0A] = &Chip8::OP_Fx0A;
	tableF[0x15] = &Chip8::OP_Fx15;
	tableF[0x18] = &Chip8::OP_Fx18;
	tableF[0x1E] = &Chip8::OP_Fx1E;
	tableF[0x29] = &Chip8::OP_Fx29;
	tableF[0x33] = &Chip8::OP_Fx33;
	tableF[0x55] = &Chip8::OP_Fx55;
	tableF[0x65] = &Chip8::OP_Fx65;
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

void Chip8::Table0()
{
	((*this).*(table0[opcode & 0x000Fu]))();
}

void Chip8::Table8()
{
	((*this).*(table8[opcode & 0x000Fu]))();
}

void Chip8::TableE()
{
	((*this).*(tableE[(opcode & 0x00FFu) >> 8u]))();
}

void Chip8::TableF()
{
	((*this).*(tableF[(opcode & 0x00FFu) >> 8u]))();
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

void Chip8::OP_8xy4()
{
	// Add Vy to Vx. VF is set to 1 when there's a carry, and to 0 when there isn't
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint16_t sum = registers[Vx] + registers[Vy];
	if (sum > 255U)
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}
	registers[Vx] = sum & 0xFFu;
}

void Chip8::OP_8xy5()
{
	// Subtract Vy from Vx. VF is set to 0 when there's a borrow, and 1 when there isn't
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	if (registers[Vx] > registers[Vy])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}
	registers[Vx] -= registers[Vy];
}

void Chip8::OP_8xy6()
{
	// Shift Vx right by one. VF is set to the least significant bit of Vx before the shift
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	registers[0xF] = registers[Vx] & 0x1u;
	registers[Vx] >>= 1;
}

void Chip8::OP_8xy7()
{
	// Set Vx to Vy minus Vx. VF is set to 0 when there's a borrow, and 1 when there isn't
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	if (registers[Vy] > registers[Vx])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}
	registers[Vx] = registers[Vy] - registers[Vx];
}

void Chip8::OP_8xyE()
{
	// Shift Vx left by one. VF is set to the most significant bit of Vx before the shift
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	registers[0xF] = (registers[Vx] & 0x80u) >> 7u;
	registers[Vx] <<= 1;
}

void Chip8::OP_9xy0()
{
	// Skip the next instruction if Vx doesn't equal Vy
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	if (registers[Vx] != registers[Vy])
	{
		pc += 2;
	}
}

void Chip8::OP_Annn()
{
	// Set the index register to nnn
	uint16_t address = opcode & 0x0FFFu;
	index = address;
}

void Chip8::OP_Bnnn()
{
	// Jump to the address nnn plus V0
	uint16_t address = opcode & 0x0FFFu;
	pc = registers[0] + address;
}

void Chip8::OP_Cxkk()
{
	// Set Vx to a random number with a mask of kk
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;
	registers[Vx] = randByte(randGen) & byte;
}

void Chip8::OP_Dxyn()
{
	// Draw a sprite at coordinate (Vx, Vy) that has a width of 8 pixels and a height of n pixels
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;
	// Wrap if going beyond the boundaries of the screen
	uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
	uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

	registers[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row)
	{
		uint8_t spriteByte = memory[index + row];
		for (unsigned int col = 0; col < 8; ++col)
		{
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];
			// Sprite pixel is on
			if (spritePixel)
			{
				// Screen pixel is also on - collision
				if (*screenPixel == 0xFFFFFFFF)
				{
					registers[0xF] = 1;
				}
				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

void Chip8::OP_Ex9E()
{
	// Skip the next instruction if the key stored in Vx is pressed
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t key = registers[Vx];
	if (keypad[key])
	{
		pc += 2;
	}
}

void Chip8::OP_ExA1()
{
	// Skip the next instruction if the key stored in Vx isn't pressed
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t key = registers[Vx];
	if (!keypad[key])
	{
		pc += 2;
	}
}

void Chip8::OP_Fx07()
{
	// Set Vx to the value of the delay timer
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	registers[Vx] = delayTimer;
}

void Chip8::OP_Fx0A()
{
	// Wait for a key press and store the result in Vx. If no key is pressed, repeat the instruction by decrementing the program counter by 2
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
		pc -= 2;
	}
}

void Chip8::OP_Fx15()
{
	// Set the delay timer to Vx
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	delayTimer = registers[Vx];
}

void Chip8::OP_Fx18()
{
	// Set the sound timer to Vx
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	soundTimer = registers[Vx];
}

void Chip8::OP_Fx1E()
{
	// Add Vx to I
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	index += registers[Vx];
}

void Chip8::OP_Fx29()
{
	// Set I to the location of the sprite for the character in Vx
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = registers[Vx];
	index = FONTSET_START_ADDRESS + (5 * digit);
}

void Chip8::OP_Fx33()
{
	// Store the binary-coded decimal representation of Vx at the addresses I, I + 1, and I + 2
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = registers[Vx];
	// Ones-place
	memory[index + 2] = value % 10;
	value /= 10;
	// Tens-place
	memory[index + 1] = value % 10;
	value /= 10;
	// Hundreds-place
	memory[index] = value % 10;
}

void Chip8::OP_Fx55()
{
	// Store V0 to Vx in memory starting at address I
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	for (uint8_t i = 0; i <= Vx; ++i)
	{
		memory[index + i] = registers[i];
	}
}

void Chip8::OP_Fx65()
{
	// Fill V0 to Vx with values from memory starting at address I
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	for (uint8_t i = 0; i <= Vx; ++i)
	{
		registers[i] = memory[index + i];
	}
}

void Chip8::OP_NULL()
{
	// Unknown opcode
}