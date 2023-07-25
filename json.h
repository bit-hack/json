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

typedef struct jsonS     jsonT,     *jsonP;
typedef struct jsonNodeS jsonNodeT, *jsonNodeP;

struct jsonS {

  jsonNodeP   root;     // root node
  const char *src;      // json source
  jsonNodeP   gc;       // garbage collection chain
};

struct jsonNodeS {

  const char   *src;    // entry in json source
  jsonNodeP     next;   // next node in sibling chain
  jsonNodeP     child;  // child node
  enum jsonType type;   // node type
  jsonNodeP     gc;     // garbage collection chain
};

bool      jsonParse(jsonP json, const char *src);
void      jsonFree (jsonP json);

int64_t   jsonValueI    (jsonNodeP node);
double    jsonValueD    (jsonNodeP node);
bool      jsonStrcmp    (jsonNodeP node, const char *str);
uint32_t  jsonStrlen    (jsonNodeP node);
bool      jsonAsBool    (jsonNodeP node);
jsonNodeP jsonFindMember(jsonNodeP node, const char *name);
void      jsonPrint     (jsonNodeP node);
void      jsonValidate  (jsonNodeP node);
