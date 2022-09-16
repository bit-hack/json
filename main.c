#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

int main(int argc, char** args) {

  if (argc <= 1) {
    return 1;
  }

  // open the source file
  FILE* fd = fopen(args[1], "rb");
  if (!fd) {
    return 2;
  }

  // get file length
  fseek(fd, 0, SEEK_END);
  long size = ftell(fd);
  fseek(fd, 0, SEEK_SET);

  // allocate memory for file
  char* source = malloc(size + 1);
  if (!source) {
    return 3;
  }

  // read the file in
  memset(source, 0, size + 1);
  size_t read = fread(source, 1, size, fd);
  fclose(fd);

  // parse the json file
  jsonT json = { 0 };
  if (!jsonParse(&json, source)) {
    return 4;
  }

  // print the parse json
  if (json.root) {
    jsonPrint(json.root);
    jsonValidate(json.root);
  }

  // release the json file
  jsonFree(&json);
  return 0;
}
