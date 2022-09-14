#pragma once
#include <stdbool.h>
#include <stdint.h>

enum jsonType {
  jsonTrue,
  jsonFalse,
  jsonNull,
  jsonObject,
  jsonMember,
  jsonArray,
  jsonString,
  jsonNumber
};

typedef struct jsonS jsonT, *jsonP;

typedef struct jsonNodeS jsonNodeT, * jsonNodeP;

struct jsonS {
  jsonNodeP   root;
  const char* src;
  jsonNodeP   gc;
};

struct jsonNodeS {
  const char*   src;
  jsonNodeP     next;
  jsonNodeP     child;
  enum jsonType type;

  jsonNodeP     gc;
};

bool jsonParse(jsonP json, const char* src);
void jsonFree (jsonP json);
