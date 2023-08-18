all:
	cc -Wall -Wextra -g3 -fsanitize=undefined,address main.c stack.c graphics.c -o main -I/usr/include/SDL2 -D_REENTRANT -lSDL2 -lm 
clean:
	rm main
