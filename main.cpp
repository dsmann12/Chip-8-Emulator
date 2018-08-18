#include <iostream>
#include <chrono>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "chip8.h"

int main() {
	const std::string emulator_title = "Chip 8 Emulator";
	bool isClosed = false;
	// system should initialize cpu and other variables in constructor
	chip8::chip8 chip8;
	// load rom
	chip8.load_rom("INVADERS");
	
	/*
	for(int i = 0; i < 1283; i++) {
		chip8.exec_instruction();
	}
	*/

	// set up SDL
	SDL_Window* window = nullptr;
	SDL_GLContext gl_context;
	SDL_Init(SDL_INIT_EVERYTHING);

	// set SDL attrbutes
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// create window
	window = SDL_CreateWindow(emulator_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH*16, HEIGHT*16, SDL_WINDOW_OPENGL);
	// create context
	gl_context = SDL_GL_CreateContext(window);

	GLenum status = glewInit();
	if (status != GLEW_OK) {
		std::cerr << "GLEW failed to initialize\n";
	}	

	
	// Emulator should run at 60 hz
	// Drawing should be done at 60hz
	// timers count down at 60 hz
	// Means must draw a frame every 1/60 second
	// 1000ms / 60 = 16.6667ms
	// So if time since last frame drawn is >= 16.6667ms, draw
	
	// typedef to simplify time_point declarations (I don't like auto)
	typedef std::chrono::system_clock::time_point time_pt;
	// std::chono::duration type for millisecond durations
	// store time since last frame in milliseconds
	std::chrono::milliseconds time_since_last_frame;
	// Get current time and set it to time_since_last_frame
	time_pt time_at_last_frame = std::chrono::system_clock::now();
	// frame interval is every 16ms
	// probably should use microseconds to get more accurate interval
	std::chrono::milliseconds frame_interval{16};
	// Emulation loop
	while (!isClosed) {
		// if not waiting, run an instruction
		if (!chip8.is_waiting()) {
			chip8.exec_instruction();
		}

		// calculate time since last frame
		time_since_last_frame = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - time_at_last_frame);	
		// draw check
		if (time_since_last_frame >= frame_interval) {
			// draw
			// set clear color to black
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	
			// clear color buffer. Fill all colors with clear color
			glClear(GL_COLOR_BUFFER_BIT);

			// update display
			// swap window for double buffering
			SDL_GL_SwapWindow(window);

			// get time when this 
			time_at_last_frame = std::chrono::system_clock::now();
		}
		
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
				case SDL_QUIT:
					isClosed = true;
					break;
				case SDL_KEYDOWN:
					isClosed = true;
					break;
			}
		}
	}

	// clean up SDL
	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	// run
	return 0;
}
