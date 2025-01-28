#include "Chip8.h"
#include <fstream>

const unsigned int START_ADDRESS = 0x200; // 0x000 to 0x1FF are reserved

Chip8::Chip8()
{
	pc = START_ADDRESS; 
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