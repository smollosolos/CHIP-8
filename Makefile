compile: build run

build:
	gcc -Wall -Wextra ./main.c ./chip8.c -o ./out/chip8.exe -lSDL3

run:
	./out/chip8.exe '$(file)'