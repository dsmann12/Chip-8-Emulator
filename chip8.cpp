#include <cstdlib>
#include <iostream>
#include <fstream>
#include "chip8.h"
#include <cstdio>

void error(const std::string &msg) {
	std::cout << msg << std::endl;
	std::exit(1);
}

void chip8::chip8::exec_instructions() {
	std::cout << "Executing exec_instructions\n";
}

void chip8::chip8::load_rom(const std::string &rom) {
	std::cout << "Executing load_rom\n";
	const std::string &folder = "ROM";
	const std::string &path = folder + "/" + rom;
	std::cout << "Path is " << path << std::endl;

	// open input file stream at file path
	std::ifstream ist {path, std::ios_base::binary};
	if (!ist) {
		error("ROM INPUT STREAM ERROR");
	}

	unsigned short pos = 0x200;
	unsigned char byte = 0;
	unsigned int count = 0;
	while(ist.good()) {
		this->memory[pos++] = ist.get();
		count += ist.gcount();
	}

	pos = 0x200;
	std::cout << "Count is " << std::dec << count << std::endl;
	unsigned char first = 0;
	first = this->memory[pos];
	std::cout << "First byte is " << std::showbase << std::hex << +first << std::endl;

	// Get first byte and multiply by 0x100 to shift it 8 bits over.
	// Add next byte to piece together correct opcode regardless of endian on system
	// Chip 8 is big endian
	unsigned short opcode = 0;
	opcode = this->memory[pos]*0x100 + this->memory[pos+1];
	std::cout << "Opcode is " << +opcode << std::endl;

	// o, x, y, n, nn, nnn
	// Getting correct parts of opcode from little endian architecture to big endian
	unsigned char o = (opcode >> 12);
	unsigned char x = (opcode >> 8) & 0xF;
	unsigned char y = (opcode >> 4) & 0xF;
	unsigned char n = (opcode & 0xF);
	unsigned char nn = (opcode & 0xFF);
	unsigned short nnn = (opcode & 0x0FFF);


	std::cout << "O is " << +o << std::endl;
	std::cout << "x is " << +x << std::endl;
	std::cout << "y is " << +y << std::endl;
	std::cout << "n is " << +n << std::endl;
	std::cout << "nn is " << +nn << std::endl;
	std::cout << "nnn is "<< +nnn << std::endl;

	switch(o) {
		case 0x1:
			std::cout << "This is 1\n";
			break;
		case 0x2:
			break;
		case 0x3:
			break;
		case 0x4:
			break;
		case 0x5:
			break;
		case 0x6:
			break;
		case 0x7:
			break;
		case 0x8:
			break;
		case 0x9:
			break;
		case 0xA:
			break;
		case 0xB:
			break;
		case 0xC:
			break;
		case 0xD:
			break;
		case 0xE:
			break;
		case 0xF:
			break;
		default:
			std::cout << "This is default\n";
			break;
	}
}


int main() {
	// system should initialize cpu and other variables in constructor
	chip8::chip8 chip8;
	// load rom
	chip8.load_rom("INVADERS");
	chip8.exec_instructions();
	// run
	return 0;
}
