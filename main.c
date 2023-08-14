#include "./stack.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define MEMORY_SIZE 4096
#define STARTING_POINT 0x200

#define REGISTER_SIZE 16

#define DISPLAY_HEIGHT 32
#define DISPLAY_WIDTH 64

typedef char *String;
typedef unsigned char Byte;

Stack stack = {.top = -1};

SDL_Window *window;
SDL_Renderer *renderer;

Byte registers[REGISTER_SIZE] = {1, 2,  3,  4,  5,  6,  7,  8,
                                 9, 10, 11, 12, 13, 14, 15, 16};

bool display[32][64];

int delay_timer = 0;
int sound_timer = 0;

unsigned short addr_register = 0;

typedef struct {
  char key[16];
} Input;

Input in;
// Program counter
unsigned int pc = STARTING_POINT;

unsigned char sprites[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

typedef struct Prog {
  Byte *data;
  int size;
} Prog;

unsigned char keymap[16] = {SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_q, SDLK_w,
                            SDLK_e, SDLK_r, SDLK_a, SDLK_s, SDLK_d, SDLK_f,
                            SDLK_y, SDLK_x, SDLK_c, SDLK_v};

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

  Byte *content = calloc(MEMORY_SIZE, sizeof(Byte));

  memcpy(content, sprites, 80);

  size_t block_read = fread(content + STARTING_POINT, file_size, 1, fp);

  Prog prog;
  prog.data = content;
  prog.size = file_size;

  return prog;
}

/* Function: Print_Prog
 * Print the content of the data in a struct Prog
 *
 * prog : struct Prog
 */

void Print_Prog(Prog prog) {
  for (int i = 0; i < prog.size; i++) {
    if ((i + 1) % 8 == 0) {
      printf("\n");
    }
  }
}

void Draw_Display() {
  for (int i = 0; i < DISPLAY_WIDTH; i++) {
    for (int j = 0; j < DISPLAY_HEIGHT; j++) {
      bool is_set = display[j][i];
      if (is_set) {
        SDL_Rect rect = {i * 10, j * 10, 9, 9};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &rect);
      }
    }
  }
  SDL_RenderPresent(renderer);
}

void Update_Display(int x, int y, int n, Prog prog) {
  Byte bytes[n];

  memcpy(bytes, prog.data + addr_register, n);

  int b = 0;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < 8; j++) {
      bool current = display[(i + y) % 32][(j + x) % 64];
      bool v = (bytes[b] >> (8 - 1 - j)) & 1;
      if (current == 1 && v == 1) {
        registers[0xF] = 1;
      } else {
        registers[0xF] = 0;
      }
      display[(i + y) % 32][(j + x) % 64] = v;
    }
    b++;
  }

  Draw_Display();
}

/* Function: Opcode_Read
 * Read, parse and run opcode
 *
 * unsigned short opcode
 */
void Opcode_Read(Byte opcode, Byte arg, Prog prog) {
  Byte instruction = (opcode >> 4) & 0xff;
  printf("%02X %02X\n", opcode, arg);
  switch (instruction) {

  case 0x0: {
    if ((opcode & 0x0f) == 0) {
      if (arg == 0xEE) {
        unsigned short addr = Stack_Pop(&stack);
        pc = addr;
      } else if (arg == 0xE0) {
        memset(display, 0, 32 * 64);
        Draw_Display();
      }
    } else {
      unsigned short value = ((opcode & 0x0f) << 8) | arg;
      pc = value - 2;
    }
  } break;

  case 0x1: {
    unsigned short addr = ((opcode & 0x0f) << 8) | arg;
    pc = addr - 2;
  } break;

  case 0x2: {
    unsigned short value = ((opcode & 0x0f) << 8) | arg;
    Stack_Push(&stack, &pc);
    pc = value - 2;
  } break;

  case 0x3: {
    Byte reg = opcode & 0x0F;
    if (registers[reg] == arg) {
      pc += 2;
    }
  } break;

  case 0x4: {
    Byte reg = (opcode & 0x0F);
    if (registers[reg] != arg) {
      pc += 2;
    }
  } break;

  case 0x5: {
    Byte reg = (opcode & 0x0F);
    if (registers[reg] == registers[arg]) {
      pc += 2;
    }
  } break;

  case 0x6: {
    Byte reg = opcode & 0x0f;
    registers[reg] = arg;
  } break;

  case 0x7: {
    Byte reg = opcode & 0x0f;
    registers[reg] += arg;
  } break;

  case 0x8: {
    Byte mode = (arg)&0x0f;
    Byte dest = opcode & 0x0f;
    Byte src = arg & 0xf;
    switch (mode) {
    case 0x0: {
      registers[dest] = registers[src];
    } break;
    case 0x1: {
      registers[dest] |= registers[src];
    } break;
    case 0x2: {
      registers[dest] &= registers[src];
    } break;
    case 0x3: {
      registers[dest] ^= registers[src];
    } break;
    case 0x4: {
      // Checkk bits overflo<
      if (registers[dest] > 0 && registers[src] > INT_MAX - registers[dest]) {
        registers[0xf] = 1;
      } else {
        registers[0xf] = 0;
      }
      registers[dest] += registers[src];
    } break;
    case 0x5: {
      registers[dest] -= registers[src];
      if (registers[dest] > registers[src]) {
        registers[0xf] = 1;
      } else {
        registers[0xf] = 0;
      }
    } break;
    case 0x6: {
      if (registers[dest] & 1) {
        registers[0xf] = 1;
      } else {
        registers[0xf] = 0;
      }
      registers[dest] /= 2;
    } break;
    case 0x7: {
      if (registers[src] > registers[dest]) {
        registers[0xf] = 1;
      } else {
        registers[0xf] = 0;
      }
      registers[dest] = registers[src] - registers[dest];
    } break;
    case 0x8: {
      if (registers[dest] & 1) {
        registers[0xf] = 1;
      } else {
        registers[0xf] = 0;
      }
      registers[dest] *= 2;
    } break;
    }
  } break;

  case 0x9: {
    Byte x = (opcode & 0x0f);
    Byte y = (opcode & 0xf0);
    if (x != y) {
      pc += 2;
    }
  } break;

  case 0xA: {
    unsigned short value = ((opcode & 0x0f) << 8) | arg;
    addr_register = value;
    // printf("%02X\n", prog.data[addr_register]);
  } break;

  case 0xB: {
    unsigned short value = ((opcode & 0x0f) << 8) | arg;
    pc = value + registers[0] - 2;
  } break;

  case 0xC: {
    Byte dest = opcode & 0xf;
    srand(time(0));
    int random = rand() % 255;
    registers[dest] = random & arg;
  } break;

  case 0xD: {
    Byte x = (opcode)&0x0f;
    Byte y = (arg >> 4) & 0xf;
    Byte n = (arg)&0x0f;
    x = registers[x];
    y = registers[y];
    Update_Display(x, y, n, prog);
  } break;

  case 0xE: {
    Byte key = opcode & 0x0f;
    switch (arg) {
    case 0x9E: {
      if (in.key[key] == 1) {
        printf("PRESSED\n");
        pc += 2;
      }
    } break;
    case 0xA1: {
      if (in.key[key] == 0) {
        pc += 2;
      }
    } break;
    }
  } break;

  case 0xF: {
    Byte reg = opcode & 0x0f;
    switch (arg) {
    case 0x07: {
      registers[reg] = delay_timer;
    } break;
    case 0x0A: {
      bool pressed = false;
      int k = 0;
      while (!pressed) {
        if (in.key[k]) {
          pressed = true;
          break;
        }
        k++;
        if (k == 16) {
          k = 0;
        }
      }
      registers[reg] = keymap[k];
    } break;
    case 0x15: {
      delay_timer = registers[reg];
    } break;
    case 0x18: {
      sound_timer = registers[reg];
    } break;
    case 0x1E: {
      addr_register += registers[reg];
    } break;
    case 0x29: {
      Byte reg = (opcode & 0x0f);
      addr_register = registers[reg] * 5;
    } break;

    case 0x33: {
      Byte reg = (opcode & 0x0f);
      int value = registers[reg];
      prog.data[addr_register] = registers[reg] / 100;
      prog.data[addr_register + 1] = (registers[reg] / 10) % 10;
      prog.data[addr_register + 2] = (registers[reg] % 100) % 10;
    } break;
    case 0x55: {
      Byte max = opcode & 0x0f;
      for (int i = 0; i < max; i++) {
        prog.data[addr_register + i] = registers[i];
      }
    } break;
    case 0x65: {
      Byte max = opcode & 0x0f;
      for (int i = 0; i < max; i++) {
        registers[i] = prog.data[addr_register + i];
      }
    } break;
    }
  } break;

  default: {
    printf("%02x\n", instruction);
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
  }
}

void Init_Display() {
  for (int i = 0; i < DISPLAY_HEIGHT; i++) {
    for (int j = 0; j < DISPLAY_WIDTH; j++) {
      display[i][j] = 0;
    }
  }
}

int main() {
  memset(&in, 0, sizeof(in));
  Init_Display();

  Prog prog = Prog_Parse("airplane.ch8");

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
    return 1;
  }

  window = SDL_CreateWindow("SDL2 Rectangle Drawing", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, DISPLAY_WIDTH * 10,
                            DISPLAY_HEIGHT * 10, SDL_WINDOW_SHOWN);

  if (!window) {
    fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (!renderer) {
    fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  SDL_Event event;
  bool running = true;
  while (running) {
    if (delay_timer > 0) {
      delay_timer--;
    }
    if (sound_timer > 0) {
      sound_timer--;
    }
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
        break;
      }
      if (event.type == SDL_KEYDOWN) {
        for (int i = 0; i < 16; i++) {
          if (event.key.keysym.sym == keymap[i]) {
            in.key[i] = 1;
          }
        }
      }
      if (event.type == SDL_KEYUP) {
        for (int i = 0; i < 16; i++) {
          if (event.key.keysym.sym == keymap[i]) {
            in.key[i] = 1;
          }
        }
      }
    }
    Byte instruction = prog.data[pc];
    Byte arg = prog.data[pc + 1];
    Opcode_Read(instruction, arg, prog);
    if (pc == prog.size - 1) {
      break;
    }
    SDL_Delay(100);
    pc += 2;
  }

  free(prog.data);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
