#include "Chip8.h"

const unsigned int START_ADDRESS = 0x200; // 0x000 to 0x1FF are reserved

Chip8::Chip8()
{
	pc = START_ADDRESS; 
}