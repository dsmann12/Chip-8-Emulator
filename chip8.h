#ifndef CHIP8CPU_H
#define CHIP8CPU_H

#include <string>

#define MEMSIZE 0x1000
#define STARTADDR 0x200
#define REGISTERCOUNT 16
#define STACKSIZE 24


// Interpreter Namespace
namespace chip8 {
	// CPU
	struct cpu {	
		unsigned char v[REGISTERCOUNT];
		// 16bit address register I
		unsigned short i_reg;
		// 16bit program counter
		unsigned short pc;
		// 8bit stack pointer
		unsigned char sp;				
		unsigned char delay_timer, sound_timer;
	};

	// Display

	// Keyboard
		
	// Chip8 System
	// Systems connects components and runs system functions
	class chip8 {
		public:
			chip8();
			void exec_instruction();
			void load_rom(const std::string &rom);
		private:
			union {
				unsigned char memory[MEMSIZE] { 0x12, 0x00 };
				struct {
					// Everything else in interface
					cpu _cpu;
					// Keyboard - Store keys as a bitmap?
					// Display
					// Stack
					unsigned short stack[STACKSIZE];
				};
			};

			void exec_opcode8xxx(unsigned short &opcode, unsigned char &o, unsigned char &x, 
							unsigned char &y, unsigned char &n, unsigned char &nn, 
							unsigned short &nnn);	
			void exec_opcodeFxxx(unsigned short &opcode, unsigned char &x, unsigned char &nn);
	};
};



#endif
