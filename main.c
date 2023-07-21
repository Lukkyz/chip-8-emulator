#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef char *String;
typedef char Byte;

// One instruction = UNSIGNED SHORT data type = 2 bytes = 16 bits
long File_Get_Size(String filename) {
  struct stat info;
  if (stat(filename, &info) != 0) {
    return -1;
  }
  return info.st_size;
}

void File_Read(String name) {
  FILE *fp;
  fp = fopen(name, "rb");
  long file_size = File_Get_Size(name);

  Byte *content = calloc(file_size, sizeof(Byte));
  size_t block_read = fread(content, file_size, 1, fp);
  fclose(fp);

  unsigned short *data = calloc(file_size / 2, sizeof(unsigned short));

  int j = 0;
  for (int i = 0; i < file_size - 1; i += 2) {
    unsigned short combined = (content[i] << 8) + (content[i + 1]);
    data[j++] = combined;
    printf(" %02x ", data[j - 1]);
    if (j % 8 == 0) {
      printf("\n");
    }
  }
}

int main() { File_Read("airplane.ch8"); }
