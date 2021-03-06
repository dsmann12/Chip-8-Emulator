#ifndef CHIP8CPU_H
#define CHIP8CPU_H

#include <string>
#include <random>

#define MEMSIZE 0x1000
#define STARTADDR 0x200
#define REGISTERCOUNT 16
#define STACKSIZE 24
#define HEIGHT 32
#define WIDTH 64

// Interpreter Namespace
namespace chip8 {
	// CPU
	struct cpu {
		// 16 8bit registers
		unsigned char v[REGISTERCOUNT];
		// 16bit address register I
		unsigned short i_reg;
		// 16bit program counter
		unsigned short pc;
		// 8bit stack pointer
		unsigned char sp;
		unsigned char delay_timer, sound_timer;
	};

	// Font pixel data
	extern unsigned char font_data[80];
	// Display

	// Keyboard
	// Chip8 System
	// Systems connects components and runs system functions
	class chip8 {
		public:
			chip8();
			void exec_instruction();
			void load_rom(const std::string &rom);
			bool is_waiting();
		private:
			union {
				unsigned char memory[MEMSIZE] { 0x12, 0x00 };
				struct {
					// Everything else in interface
					// Font
					unsigned char font[80];
					cpu _cpu;
					// waiting wire that tells system if waiting for a key
					// also holds value of key it is possibly waiting for
					unsigned char waiting;
					// Keyboard - Store keys as a bitmap?
					// Display
					// Stack
					// 48 bytes for stack. 24 memory locations
					unsigned short stack[STACKSIZE];
					// 256 bytes allocated for display
          // (64 * 32 = 2048) / 8 = 256
					unsigned char display_mem[(HEIGHT*WIDTH) / 8];
				};
			};

			std::mt19937 rnd;

			void exec_opcode8xxx(unsigned short &opcode, unsigned char &o, unsigned char &x, 
							unsigned char &y, unsigned char &n, unsigned char &nn, 
							unsigned short &nnn);	
			void exec_opcodeFxxx(unsigned short &opcode, unsigned char &x, unsigned char &nn);
	};
};



#endif
