#include <cstdlib>
#include <iostream>
#include <fstream>
#include "chip8.h"
#include <cstdio>
#include <climits>
#include <chrono>
#include <ctime>
#include <vector>

void error(const std::string &msg) {
	std::cerr << msg << std::endl;
	std::exit(1);
}

// this is a representation of the way fonts will be stored in the system
// this data will be loaded into the system's memory at startup
// first nibble of byte represents 4 pixels that will be used to draw graphics
// graphics should be drawn for every bit set to 1
// 5 rows of data for each font sprite
unsigned char chip8::font_data[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0,		// 0
	0x20, 0x60, 0x20, 0x20, 0x70,		// 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0,		// 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0,		// 3
	0x90, 0x90, 0xF0, 0x10, 0x10,		// 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0,		// 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0,		// 6
	0xF0, 0x10, 0x20, 0x40, 0x40,		// 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0,		// 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0,		// 9
	0xF0, 0x90, 0xF0, 0x90, 0x90,		// A
	0xE0, 0x90, 0xE0, 0x90, 0xE0,		// B
	0xF0, 0x80, 0x80, 0x80, 0xF0,		// C
	0xE0, 0x90, 0x90, 0x90, 0xE0,		// D
	0xF0, 0x80, 0xF0, 0x80, 0xF0,		// E
	0xF0, 0x80, 0xF0, 0x80, 0x80		// F
};


// constructor
chip8::chip8::chip8() {
  // set program counter to address 0x200 where games will be loaded
	this->_cpu.pc = 0x200;
  // seed random number generator
	this->rnd.seed(std::time(nullptr));

  // load font data into system's memory
	for (unsigned char i = 0; i < 80; ++i) {
		this->font[i] = font_data[i];
	}

  // set waiting flag to off
	this->waiting = 0x0;
}


bool chip8::chip8::is_waiting() {
	return (this->waiting & 0x80);
}

// executes FXXX instructions based on arguments
void chip8::chip8::exec_opcodeFxxx(unsigned short &opcode, unsigned char &x, unsigned char &nn) {
	switch(nn) {
		// 0xFX07 -> Store current value of delay timer in VX
		case 0x07:
			this->_cpu.v[x] = this->_cpu.delay_timer;
			break;
		// 0xFX0A -> Wait for a keypress and store the result in register VX
		case 0x0A:
			this->waiting = (0x80 | x);
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
			this->_cpu.i_reg = (5 * this->_cpu.v[x]) & 0xFFF;
			break;
		// 0xFX33 -> Store the binary coded decimal equivalent of value stored in register VX at
		// addresses I, I+1, and I+2
		case 0x33:
			{
				unsigned char vx = this->_cpu.v[x];
				this->memory[this->_cpu.i_reg & 0xFFF] = (vx / 100) % 10;
				this->memory[(this->_cpu.i_reg+1) & 0xFFF] = (vx / 10) % 10;
				this->memory[(this->_cpu.i_reg+2) & 0xFFF] = vx % 10;
			}
			break;
		// 0xFX55 -> Store the values of registers V0 to VX inclusive in memory starting at address I
		// I is set to I + X + 1 after operation
		case 0x55:
			{
			for(unsigned char i = 0; i <= x; ++i) {
				this->memory[this->_cpu.i_reg++ & 0xFFF] = this->_cpu.v[i] & 0xFF;
			}
			}
			break;
		// 0xFX65 -> Fill registers V0 to VX inclusive with values stored in memory starting at address I
		// I is set to I + X + 1 after operation
		case 0x65:
			{
        for(unsigned char i = 0; i <= x; ++i) {
          this->_cpu.v[i] = this->memory[this->_cpu.i_reg++ & 0xFFF];
        }
			}
			break;
	}
}

// executes 8XXX instructions based on n value.
void chip8::chip8::exec_opcode8xxx(unsigned short &opcode, unsigned char &o, unsigned char &x,
				unsigned char &y, unsigned char &n, unsigned char &nn, unsigned short &nnn) {
	switch (n) {
		// 0x8XY0 -> Store number value of register VY in register VX
		case 0:
			// this->_cpu.v[x] = nn;
			this->_cpu.v[x] = this->_cpu.v[y];
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
		// 0x8XY4 -> Add value of reg VY to reg VX; Set VF to 01 if carry, otherwise 00
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
        // if subtrahend is greater than minend, a borrow occurs. Else none occurs. 
        unsigned char val = this->_cpu.v[x] - this->_cpu.v[y];
        this->_cpu.v[0xF] = (this->_cpu.v[y] <= this->_cpu.v[x]) ? 0x0 : 0x1;
        this->_cpu.v[x] = val;	
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
        // if subtrahend (second value) is greater than minuend, a borrow occurs. Else none occurs
        unsigned char val = this->_cpu.v[y] - this->_cpu.v[x];
        this->_cpu.v[0xF] = (this->_cpu.v[x] <= this->_cpu.v[y]) ? 0x0 : 0x1;
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

// executes instructions from memory
void chip8::chip8::exec_instruction() {
	std::cout << "Executing exec_instructions\n";
	// Get first byte and multiply by 0x100 to shift it 8 bits over.
	// Add next byte to piece together correct opcode regardless of endian on system
	// Chip 8 is big endian, so byte value at lowest address is actually most significant byte
	unsigned short opcode = 0;
	opcode = this->memory[this->_cpu.pc++]*0x100 + this->memory[this->_cpu.pc++];
	std::cout << "Opcode is " << +opcode << std::endl;

	// o, x, y, n, nn, nnn
	// Getting correct parts of opcode from little endian architecture to big endian
  // Opcode is made of OXYN. NN is equal to YN. NNN is equal to XYN.
	unsigned char o = (opcode >> 12);
	unsigned char x = (opcode >> 8) & 0xF;
	unsigned char y = (opcode >> 4) & 0xF;
	unsigned char n = (opcode & 0xF);
	unsigned char nn = (opcode & 0xFF);
	unsigned short nnn = (opcode & 0x0FFF);

	/*
	std::cout << "O is " << +o << std::endl;
	std::cout << "x is " << +x << std::endl;
	std::cout << "y is " << +y << std::endl;
	std::cout << "n is " << +n << std::endl;
	std::cout << "nn is " << +nn << std::endl;
	std::cout << "nnn is "<< +nnn << std::endl;
	*/

  // switch on the first bit
  // TODO: Put checks on the jump instructions to make sure program will execute valid instructions
	switch(o) {
		case 0x0:
			// O0E0 -> Return from a subroutine
			if (nn == 0xEE) {
				std::cout << "Return from a subroutine\n";
				this->_cpu.pc = this->stack[this->_cpu.sp--];
			} else if (nn == 0xE0) {
				// TODO: clear the screen
				std::cout << "Clear the screen\n";
			} else {
        // 0NNN -> execute machine language subroutine at address NNN
        std::cout <<"Run 0NNN command\n";
      }
			break;
		case 0x1:
			// 1NNN -> Jump to address NNN
			this->_cpu.pc = nnn;
			break;
		case 0x2:
			// 2NNN -> Execute subroutine starting at address NNN
			std::cout << "Execute subroutine starting at address NNN: " << nnn << "\n";
			this->stack[this->_cpu.sp++ % STACKSIZE] = this->_cpu.pc;
			this->_cpu.pc += 2;
			break;
		case 0x3:
			// 3XNN -> Skip the following instruction if the value of register VX equals NN
			if (this->_cpu.v[x] == nn) {
				this->_cpu.pc += 2;
			}
			break;
		case 0x4:
			// 4XNN -> Skip the following instruction if the value of register VX != NN
			if (this->_cpu.v[x] != nn) {
				this->_cpu.pc += 2;
			}
			break;
		case 0x5:
			// 5XY0 -> Skip following instruction if value of register VX = value of reg VY
			if (this->_cpu.v[x] == this->_cpu.v[y]) {
				this->_cpu.pc += 2;
			}
			break;
		case 0x6:
			// 6XNN -> Store number NN in register VX
			this->_cpu.v[x] = nn;
			break;
		case 0x7:
			// 7XNN -> Add number NN to register VX
			this->_cpu.v[x] += nn;
			break;
		case 0x8:
			// Do its own function with its own switch
			std::cout << "8 opcode needing to do one of many things. Math\n";
			exec_opcode8xxx(opcode, o, x, y, n, nn, nnn);
			break;
		case 0x9:
			// 9XY0 -> Skip following instruction if value of reg VX is !- to value of reg VY
			if (this->_cpu.v[x] != this->_cpu.v[y]) {
				this->_cpu.pc += 2;
			}
			break;
		case 0xA:
			// ANNN -> Store memory address NNN in register I
			this->_cpu.i_reg = nnn;
			break;
		case 0xB:
			// BNNN -> Jump to address NNN + V0
			this->_cpu.pc = nnn + this->_cpu.v[0x0];
			break;
		case 0xC:
			// CXNN -> Set VX to a random number with mask of NN
			{
        std::uniform_int_distribution<std::mt19937::result_type> dist255(0, 255);
        this->_cpu.v[x] = dist255(rnd) && nn;
			}
			break;
		case 0xD:
			// Draw a sprite at position VX, VY with N bytes of sprite data starting at address stored in I
			// Set VF to 01 if any set pixels are changed to unset, and 00 otherwise
			{
        std::cout << "Draw a sprite at position "
        << "VX: " << +this->_cpu.v[x] << " VY: " << +this->_cpu.v[y]
        << " with N: " << +n << " bytes of sprite data "
        << "starting at address I: " << this->_cpu.i_reg << std::endl;

        // VX must contain a value between 00 and 3F (0-63)
        // VY must contain a value between 00 and 1F (0-31)

        // get sprite row data
        // sprite is made of n bytes of data. So n rows.
        // each byte corresponds to a row of pixels. 8 columns.

        // display_mem is 256 bytes of display data where each bit corresponds to a pixel on the screen
        // 256 = (64 * 32) / 8
        // VX,VY is coordinate on screen
        // Will draw the rectangle consisting of (VX, VY), (VX+8, VY), (VX, VY+n), (VX+8, VY+n)
        // However, graphics use flat arrays so each row consists of every WIDTH elements
        // So if VX = 1, then data is at least in WIDTH index and at most in WIDTH + HEIGHT
        // To make sure, don't write in areas off screen, VX and VX+8 by WIDTH and VY and VY+n by HEIGHT
        // Since display memory encodes byte data as bits we can divide by 8 to get correct location in display memory
        // However, if VX is not divisible by 8, then we will be changing the inner bits of a byte
        // And will also need to change the first remaining bits of the next byte
        // If any set pixels are toggled, then set VF to 01 else set VF to 00
        //
        // display_mem[index] 				    = 00000011		|	11000011
        // byte  			  				          = 00111111		|	00111111

        // (display_mem[index] ^= byte)		= 00111100		|	11111100
        // ^ byte							            = 00000011		|	11000011
        // & byte							            = 00000011		|	00000011
        //
        // If not 0, then collision occurred.
        //
        auto put = [this](unsigned char index, unsigned char byte) {
          return (
                // first XOR display_memory[index] with byte and store value in display_mem
                (
                (display_mem[index] ^= byte)
                // then XOR with byte again to get bits that were previously set to 1
                ^ byte)
                // then AND with byte to get only bits that were changed from 1 to 0 byte first XOR
                // If this value is not 0, then a collision occurred
                & byte
              );
        };


        // set col to 0
        unsigned char col = 0x0;
        // get vx and vy values. byte data
        // i value contains memory address containing data for a given sprite
        unsigned char vx = this->_cpu.v[x], vy = this->_cpu.v[y];
        unsigned short i = this->_cpu.i_reg;
        while(n--) {
          col |= 	put(((vx+0)%WIDTH + (vy+n)%HEIGHT * WIDTH)/8, this->memory[(i+n) & 0xFFF] >> (vx % 8))
            |	put(((vx+7)%WIDTH + (vy+n)%HEIGHT * WIDTH)/8, this->memory[(i+n) & 0xFFF] << (8 - (vx % 8)));
        }

        this->_cpu.v[0xF] = (col != 0x0);
			}
			break;
		case 0xE:
			// Two possibilities
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
	unsigned int count = 0;
	while (ist.good()) {
		this->memory[pos++] = ist.get();
		count += ist.gcount();
	}

	pos = 0x200;
	std::cout << "Count is " << std::dec << count << std::endl;
	unsigned char first = 0;
	first = this->memory[pos];
	std::cout << "First byte is " << std::showbase << std::hex << +first << std::endl;

}


