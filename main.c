#include <stdio.h>

#include "json.h"

const char* tests[] = {
  "true",
  "false",
  "null",
  "123456",
  "\"HelloWorld\"",
  "[ 1 ]",
  "[ 1, 2 ]",
  "[ 1, 2, 3 ]",
  "[ 1, 2, 3, 4 ]",
  "{ \"test\" : 1 }",
  "[ [ 1 ], [ 2 ] ]",
  "{ \"x\" : { \"y\" : 2 } }",
  "[ true, false, null ]",
  "[ \"Hi\" ]",
  NULL
};

void walk(int i, jsonNodeP n) {

  while (n) {

    // indent
    for (int j = 0; j < i; ++j) { printf("  "); }

    // name
    switch (n->type) {
    case jsonTrue:   printf("true");   break;
    case jsonFalse:  printf("false");  break;
    case jsonNull:   printf("null");   break;
    case jsonString: printf("string"); break;
    case jsonNumber: printf("number"); break;
    case jsonObject: printf("object"); break;
    case jsonMember: printf("member"); break;
    case jsonArray:  printf("array");  break;
    }

    printf("\n");

    // discover children
    if (n->child) {
      walk(i + 1, n->child);
    }

    // discover sibblings
    n = n->next;
  }
}

int main(int argc, char** args) {

  for (uint32_t i = 0; tests[i]; ++i) {
    const char* test = tests[i];

    jsonT j = { 0 };
    bool ok = jsonParse(&j, test);

    if (ok) {
      walk(0, j.root);
    }

    jsonFree(&j);

    printf("%c - %s\n", ok ? 'P' : 'F', test);
  }

  return 0;
}
