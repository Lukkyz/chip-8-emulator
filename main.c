#include "./stack.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define REGISTER_SIZE 16
typedef char *String;
typedef unsigned char Byte;

Stack stack = {.top = -1};

Byte registers[REGISTER_SIZE] = {1, 2,  3,  4,  5,  6,  7,  8,
                                 9, 10, 11, 12, 13, 14, 15, 16};

unsigned short addr_register = 0;
// Program counter
unsigned int pc = 0;

typedef struct Prog {
  unsigned short *data;
  int size;
} Prog;

/*
 * Function: File_Get_Size
 *
 * name : file name
 *
 * returns size in bytes of the filename
 */
long File_Get_Size(String name) {
  struct stat info;
  if (stat(name, &info) != 0) {
    return -1;
  }
  return info.st_size;
}

/*
 * Function : Prog_Parse
 * Read the program byte by byte, and pack by two bits in unsigned short (= 2
 * bits)
 *
 * name : File name
 *
 * returns: struct Prog
 */

Prog Prog_Parse(String name) {
  FILE *fp;
  fp = fopen(name, "rb");
  long file_size = File_Get_Size(name);

  Byte *content = calloc(file_size, sizeof(Byte));
  size_t block_read = fread(content, file_size, 1, fp);
  fclose(fp);

  unsigned short *data = calloc(file_size / 2, sizeof(unsigned short));

  int j = 0;
  for (int i = 0; i < file_size - 1; i += 2) {
    unsigned short combined = (content[i] << 8) | (content[i + 1]);
    data[j++] = combined;
  }
  free(content);
  Prog prog;
  prog.data = data;
  prog.size = file_size / 2;
  return prog;
}

/* Function: Print_Prog
 * Print the content of the data in a struct Prog
 *
 * prog : struct Prog
 */

void Print_Prog(Prog prog) {
  for (int i = 0; i < prog.size; i++) {
    printf("%04x  ", prog.data[i]);
    if ((i + 1) % 8 == 0) {
      printf("\n");
    }
  }
}

/* Function: Opcode_Read
 * Read, parse and run opcode
 *
 * unsigned short opcode
 */
void Opcode_Read(unsigned short opcode) {
  Byte instruction = (opcode >> 12) & 0xff;
  switch (instruction) {
  case 0x6: {
    Byte reg = (opcode >> 8) & 0x0f;
    Byte value = (opcode)&0xff;
    registers[reg] = value;
  } break;
  case 0x2: {
    unsigned short value = (opcode & 0x0fff);
    value = truncf((value - 0x200) / 2) - 1;
    Stack_Push(&stack, &value);
    pc = value;
  } break;
  case 0x7: {
    Byte reg = (opcode >> 8) & 0x0f;
    Byte value = (opcode)&0xff;
    registers[reg] += value;
  } break;
  case 0xa: {
    addr_register = truncf((opcode & 0x0fff - 0x200) / 2);
    printf("%d\n", addr_register);
  } break;
  }
}

/* Function: Prog_Run
 * Read instructions
 *
 * prog : struct Prog
 */
void Prog_Run(Prog prog) {
  while (1) {
    unsigned short instruction = prog.data[pc];
    printf("instr %d %02x\n", pc, instruction);
    Opcode_Read(instruction);
    if (pc == 5 || pc == prog.size - 1) {
      break;
    }
    pc += 1;
  }
}

int main() {
  Prog prog = Prog_Parse("airplane.ch8");
  // SDL_Init(SDL_INIT_VIDEO);

  // SDL_Window *window = SDL_CreateWindow("Chip-8", SDL_WINDOWPOS_UNDEFINED,
  //                                       SDL_WINDOWPOS_UNDEFINED, 1280, 640,
  //                                       0);

  // SDL_Renderer *renderer =
  //     SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  // SDL_SetRenderDrawColor(renderer, 255, 225, 125, 255);

  // SDL_RenderDrawPoint(renderer, 1200, 600);

  // SDL_RenderPresent(renderer);

  // SDL_Delay(5000);

  // SDL_DestroyRenderer(renderer);
  // SDL_DestroyWindow(window);
  // SDL_Quit();

  Prog_Run(prog);
}
