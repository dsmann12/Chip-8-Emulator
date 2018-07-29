#include <cstdlib>
#include <iostream>
#include <fstream>
#include "chip8.h"
#include <cstdio>
#include <climits>

void error(const std::string &msg) {
	std::cout << msg << std::endl;
	std::exit(1);
}

chip8::chip8::chip8() { 
	this->_cpu.pc = 0x200;
}

void chip8::chip8::exec_opcodeFxxx(unsigned short &opcode, unsigned char &x, unsigned char &nn) {
	switch(nn) {
		// 0xFX07 -> Store current value of delay timer in VX
		case 0x07:
			this->_cpu.v[x] = this->_cpu.delay_timer;
			break;
		// 0xFX0A -> Wait for a keypress and store the result in register VX
		case 0x0A:
			std::cout << "Unimplemented opcode\n";
			break;
		// 0xFX15 -> Set delay timer to the value of register VX
		case 0x15:
			this->_cpu.delay_timer = this->_cpu.v[x];
			break;
		// 0xFX18 -> Set the sound timer to the value of register VX
		case 0x18:
			this->_cpu.sound_timer = this->_cpu.v[x];
			break;
		// 0xFX1E -> Add the value stored in register VX to register I
		case 0x1E:
			this->_cpu.i_reg += this->_cpu.v[x];
			break;
		// 0xFX29 -> Set I to memory address of the sprite data corresponding to the hex digit
		// stored in register VX
		case 0x29:
			std::cout << "Unimplemented opcode\n";
			break;
		// 0xFX33 -> Store the binary coded decimal equivalent of value stored in register VX at
		// addresses I, I+1, and I+2
		case 0x33:
			std::cout << "Unimplemented opcode\n";
			break;
		// 0xFX55 -> Store the values of registers V0 to VX inclusive in memory starting at address I
		// I is set to I + X + 1 after operation
		case 0x55:
			std::cout << "Unimplemented opcode\n";
			break;
		// 0xFX65 -> Fill registers V0 to VX inclusive with values stored in memory starting at address I
		// I is set to I + X + 1 after operation
		case 0x65:
			std::cout << "Unimplemented opcode\n";
			break;
	}
}

void chip8::chip8::exec_opcode8xxx(unsigned short &opcode, unsigned char &o, unsigned char &x,
				unsigned char &y, unsigned char &n, unsigned char &nn, unsigned short &nnn) {
	switch (n) {
		// 0x8XY0 -> Store number NN in register VX
		case 0:
			this->_cpu.v[x] = nn;
			break;
		// 0x8XY1 -> Set VX to VX | VY
		case 1:
			this->_cpu.v[x] |= this->_cpu.v[y];
			break;
		// 0x8XY2 -> Set VX to VX & VY
		case 2:
			this->_cpu.v[x] &= this->_cpu.v[y];
			break;
		// 0x8XY3 -> Set VX to VX ^ VY
		case 3:
			this->_cpu.v[x] ^= this->_cpu.v[y];
			break;
		// 0x8XY4 -> Add value of reg XY to reg VX; Set VF to 01 if carry, otherwise 00
		case 4:
			{
			unsigned short val = this->_cpu.v[x] + this->_cpu.v[y];
			this->_cpu.v[0xF] = (val > 0xFF) ? 0x1 : 0x0;
			this->_cpu.v[x] = (unsigned char) val;
			}
			break;
		// 0x8XY5 -> Sub value of VY from VX; Set VF to 00 if borrow, otherwise 01
		case 5:
			{
			unsigned short val = this->_cpu.v[x] - this->_cpu.v[y];
			this->_cpu.v[0xF] = (val > 0xFF) ? 0x0 : 0x1;
			this->_cpu.v[x] = (unsigned char) val;	
			}
			break;
		// 0x8XY6 -> Store value of VY >> 1 in VX; Set VF to lsb prior to shift
		case 6:
			{
			unsigned char vy = this->_cpu.v[y];
			unsigned char lsb = (vy & 0x1);
			this->_cpu.v[0xF] = lsb;
			this->_cpu.v[x] = (vy >> 1);
			}
			break;
		// 0x8XY7 -> Set VX to value of VY - YX; Set VF to 00 if borrow, otherwise 0x
		case 7:
			{
			unsigned short val = this->_cpu.v[y] - this->_cpu.v[x];
			this->_cpu.v[0xF] = (val > 0xFF) ? 0x0 : 0x1;
			this->_cpu.v[x] = (unsigned char) val;
			}
			break;
		// 0x8XYE -> Store value of VY << 1 in VX; Set VX to most significcant bit prior to shift
		case 0xE:
			{
			unsigned char vy =  this->_cpu.v[y];
			unsigned char msb = (vy >> 7);
			this->_cpu.v[0xF] = msb;
			this->_cpu.v[x] = (vy << 1);
			}
			break;
		default:
			std::cout << "Unimplemented opcode: " << +opcode << std::endl;
			break;
	}
}

void chip8::chip8::exec_instruction() {
	std::cout << "Executing exec_instructions\n";
	// Get first byte and multiply by 0x100 to shift it 8 bits over.
	// Add next byte to piece together correct opcode regardless of endian on system
	// Chip 8 is big endian
	unsigned short opcode = 0;
	opcode = this->memory[this->_cpu.pc++]*0x100 + this->memory[(this->_cpu.pc++)];
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
		case 0x0:
			std::cout << "Return from a subroutine\n";
			break;
		case 0x1:
			std::cout << "This is 1\n";
			this->_cpu.pc = nnn;
			break;
		case 0x2:
			std::cout << "Execute subroutine starting at address NNN: " << nnn << "\n";
			break;
		case 0x3:
			if (this->_cpu.v[x] == nn) {
				this->_cpu.pc += 2;
			}
			break;
		case 0x4:
			if (this->_cpu.v[x] != nn) {
				this->_cpu.pc += 2;
			}
			break;
		case 0x5:
			if (this->_cpu.v[x] == this->_cpu.v[y]) {
				this->_cpu.pc += 2;
			}
			break;
		case 0x6:
			this->_cpu.v[x] = nn;
			break;
		case 0x7:
			this->_cpu.v[x] += nn;
			break;
		case 0x8:
			// Do its own function with its own switch
			std::cout << "8 opcode needing to do one of many things. Math\n";
			exec_opcode8xxx(opcode, o, x, y, n, nn, nnn);
			break;
		case 0x9:
			if (this->_cpu.v[x] != this->_cpu.v[y]) {
				this->_cpu.pc += 2;
			}
			break;
		case 0xA:
			this->_cpu.i_reg = nnn;
			break;
		case 0xB:
			std::cout << "Unimplemented opcode\n";
			break;
		case 0xC:
			std::cout << "Unimplemented opcode\n";
			break;
		case 0xD:
			std::cout << "Draw a sprite at position "
			<< "VX: " << +this->_cpu.v[x] << " VY: " << +this->_cpu.v[y]
			<< " with N: " << +n << " bytes of sprite data "
			<< "starting at address I: " << this->_cpu.i_reg << std::endl;
			break;
		case 0xE:
			std::cout << "Unimplemented opcode\n";
			break;
		case 0xF:
			// Do its own function with its own switch for many other 
			exec_opcodeFxxx(opcode, x, nn);
			break;
		default:
			std::cout << "This is default\n";
			break;
	}
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

}


int main() {
	// system should initialize cpu and other variables in constructor
	chip8::chip8 chip8;
	// load rom
	chip8.load_rom("INVADERS");
	for(int i = 0; i < 30; i++) {
		chip8.exec_instruction();
	}
	// run
	return 0;
}
