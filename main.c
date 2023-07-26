#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef char *String;
typedef unsigned char Byte;

Byte registers[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
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
 *
 * return : void
 */

void Print_Prog(Prog prog) {
  for (int i = 0; i < prog.size; i++) {
    printf("%04x  ", prog.data[i]);
    if ((i + 1) % 8 == 0) {
      printf("\n");
    }
  }
}

/* Function: Prog_Run
 * Read instructions
 *
 * prog : struct Prog
 *
 *
 */
void Prog_Run(Prog prog) {
  while (1) {
    unsigned short instruction = prog.data[pc];
    Byte current_register = (instruction >> (8 * 1)) & 0xff;
    char value = (instruction)&0xff;
    current_register = current_register & 0x0f;
    registers[current_register] = value;

    if (pc == 4)
      break;
    pc += 1;
  }
  for (int i = 0; i < 16; i++) {
    printf("%d\n", registers[i]);
  }
}

int main() {
  Prog prog = Prog_Parse("airplane.ch8");
  Prog_Run(prog);
}
