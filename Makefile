all:
	cc -Wall -Wextra -g3 -fsanitize=undefined,address main.c stack.c -o main
clean:
	rm main
