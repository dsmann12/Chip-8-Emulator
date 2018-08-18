all:
		g++ chip8.cpp main.cpp -std=c++14 -lSDL2 -lGL -lGLEW -o chip8
