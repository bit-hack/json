#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

int main(int argc, char** args) {

  if (argc <= 1) {
    return 1;
  }


  FILE* fd = fopen(args[1], "rb");
  if (!fd) {
    return 2;
  }
  fseek(fd, 0, SEEK_END);
  long size = ftell(fd);
  fseek(fd, 0, SEEK_SET);
  char* source = malloc(size + 1);
  if (!source) {
    return 3;
  }

  memset(source, 0, size + 1);
  size_t read = fread(source, 1, size, fd);
  fclose(fd);

  if (read != size) {
    return 4;
  }

  source[size] = '\0';


  jsonT json = { 0 };
  if (!jsonParse(&json, source)) {
    return 5;
  }

  if (json.root) {
    jsonPrint(json.root);
  }

  jsonFree(&json);
  return 0;
}
