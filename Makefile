all:
	cc -Wall -Wextra -g3 -fsanitize=undefined,address main.c stack.c -o main -I/usr/include/SDL2 -D_REENTRANT -lSDL2 
clean:
	rm main
