#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"


#define ERROR(...)                \
  {                               \
    fprintf(stderr, __VA_ARGS__); \
  }


int main(int argc, char** args) {

  if (argc <= 1) {
    ERROR("Argument required\n");
    return 1;
  }

  // open the source file
  FILE* fd = fopen(args[1], "rb");
  if (!fd) {
    ERROR("Unable to open %s\n", args[1]);
    return 1;
  }

  // get file length
  fseek(fd, 0, SEEK_END);
  const long size = ftell(fd);
  fseek(fd, 0, SEEK_SET);

  if (size < 0) {
    ERROR("ftell() failed\n");
    return 1;
  }

  // allocate memory for file
  char* source = malloc((size_t)size + 1);
  if (!source) {
    ERROR("Failed to allocate %u bytes\n", size);
    return 1;
  }

  // read the file in
  memset(source, 0, (size_t)size + 1);
  const size_t read = fread(source, 1, size, fd);
  fclose(fd);

  // parse the json file
  jsonT json = { 0 };
  if (!jsonParse(&json, source)) {
    ERROR("jsonParse failed\n");
    return 1;
  }

  // print the parse json
  if (json.root) {
    jsonValidate(json.root);
    jsonPrint(json.root);
  }

  // release the json file
  jsonFree(&json);
  return 0;
}
