#include <stdio.h>
#include <assert.h>

#include "json.h"


void border() {
  printf("----------------------------------------------------\n");
}

bool test1() {

  static const char* tests[] = {
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
    "[]",
    "{}",
    "\"\"",
    NULL
  };

  for (uint32_t i = 0; tests[i]; ++i) {
    const char* test = tests[i];

    border();
    printf("// '%s'\n", test);

    jsonT j = { 0 };
    bool ok = jsonParse(&j, test);
    if (ok) {
      jsonPrint(j.root);
    }

    printf("%s\n", ok ? "PASS" : "FAIL");

    jsonFree(&j);

  }

  return true;
}

bool test2() {

  border();

  static const char* test = "{ \"Hi\" : 1234, \"Xero\" : 96, \"Park\" : 6274 }";

  jsonT j = { 0 };
  if (!jsonParse(&j, test)) {
    return;
  }
  jsonPrint(j.root);

  jsonNodeP a = jsonFindMember(j.root, "Hi");
  jsonNodeP b = jsonFindMember(j.root, "Xero");
  jsonNodeP c = jsonFindMember(j.root, "Park");

  assert(a->type == jsonMember);
  assert(jsonValue(a->child) == 1234);

  assert(b->type == jsonMember);
  assert(jsonValue(b->child) == 96);

  assert(c->type == jsonMember);
  assert(jsonValue(c->child) == 6274);
}

int main(int argc, char** args) {

  test1();
  test2();

  return 0;
}
