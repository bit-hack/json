#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "json.h"


#define DO(EXPR) \
  if (!(EXPR)) {  \
    return false; \
  }

typedef struct jsonParseS {

  jsonNodeP   top;  // work space
  const char* o;    // old parse pointer
  const char* p;    // parse pointer
  jsonNodeP   gc;   // garbage collection chain

} jsonParseT, *jsonParseP;

static bool parseElement (jsonParseP j);
static bool parseElements(jsonParseP j);
static bool parseValue   (jsonParseP j);
static bool parseString  (jsonParseP j);

static jsonNodeP newNode(jsonParseP j, const char* src, enum jsonType type) {
  jsonNodeP obj = calloc(1, sizeof(jsonNodeT));
  assert(obj);
  obj->type = type;
  obj->src  = src;
  // insert into garbage collection chain
  obj->gc = j->gc;
  j->gc   = obj;
  return obj;
}

static bool found(jsonParseP j, char ch) {
  if (*j->p == ch) {
    j->o = j->p++;
    return true;
  }
  return false;
}

static bool foundStr(jsonParseP j, const char* str) {
  const char* o = j->p;
  const char* p = j->p;
  for (;; ++p, ++str) {
    if (*str == '\0') {
      j->o = o;
      j->p = p;
      return true;
    }
    if (*str != *p) {
      break;
    }
  }
  return false;
}

static bool foundAlpha(jsonParseP j) {
  const char ch = *j->p;
  if ((ch >= 'a' && ch <= 'z') ||
      (ch >= 'A' && ch <= 'Z') ||
      (ch == '_')) {
    j->o = (j->p)++;
    return true;
  }
  return false;
}

static bool foundNumeric(jsonParseP j, bool peek) {
  const char ch = *j->p;
  if (ch >= '0' && ch <= '9') {
    if (!peek) {
      j->o = j->p++;
    }
    return true;
  }
  return false;
}

static bool parseJson(jsonParseP j) {
  return parseElement(j);
}

static bool parseWs(jsonParseP j) {
  for (;;) {
    if (found(j, ' ')  ||
        found(j, '\t') ||
        found(j, '\r') ||
        found(j, '\n')) {
      continue;
    }
    return true;
  }
}

static bool parseElement(jsonParseP j) {
  DO(parseWs(j));
  DO(parseValue(j));
  DO(parseWs(j));
  return true;
}

static bool parseMember(jsonParseP j) {
  DO(parseWs(j));
  DO(found(j, '"'));

  DO(parseString(j));       // key
  assert(j->top);
  jsonNodeP node = j->top;
  node->type = jsonMember;

  DO(parseWs(j));
  DO(found(j, ':'));

  DO(parseElement(j));      // value
  node->child = j->top;
  j->top = node;
  return true;
}

static bool parseMembers(jsonParseP j) {

  jsonNodeP first = NULL;
  jsonNodeP node  = NULL;

  do {
    DO(parseMember(j));

    // track the first member
    if (!first) { first = j->top; }

    // insert into chain
    if (node) { node->next = j->top; }
    node = j->top;

  } while (found(j, ','));

  j->top = first;
  return true;
}

static bool parseElements(jsonParseP j) {

  jsonNodeP prev  = NULL;
  jsonNodeP first = NULL;

  do {
    DO(parseElement(j));

    // track the first element
    if (!first) { first = j->top; };

    // insert into chain
    if (prev) { prev->next = j->top; }
    prev = j->top;

  } while (found(j, ','));

  j->top = first;
  return true;
}

static bool parseTrue(jsonParseP j) {
  j->top = newNode(j, j->o, jsonTrue);
  return true;
}

static bool parseFalse(jsonParseP j) {
  j->top = newNode(j, j->o, jsonFalse);
  return true;
}

static bool parseNull(jsonParseP j) {
  j->top = newNode(j, j->o, jsonNull);
  return true;
}

static bool parseObject(jsonParseP j) {

  jsonNodeP node = newNode(j, j->o, jsonObject);

  assert(*(j->o) == '{');
  DO(parseWs(j));

  if (!found(j, '}')) {

    DO(parseMembers(j));
    node->child = j->top;
    j->top = node;

    DO(found(j, '}'));
    return true;
  }

  j->top = node;
  return true;
}

static bool parseArray(jsonParseP j) {
  assert(*(j->o) == '[');

  jsonNodeP node = newNode(j, j->o, jsonArray);

  DO(parseWs(j));
  if (!found(j, ']')) {
    DO(parseElements(j));

    node->child = j->top;
    j->top = node;

    DO(found(j, ']'));
  }

  j->top = node;
  return true;
}

static bool parseNumber(jsonParseP j) {
  const char* p = j->p;
  const char* o = j->o;
  j->top = newNode(j, j->p, jsonNumber);

  while (foundNumeric(j, /*peek*/false));
  if (j->p != p) {
    return true;
  }
  j->o = o;
  return false;
}

static bool parseString(jsonParseP j) {
  assert(*(j->o) == '"');

  j->top = newNode(j, j->p, jsonString);

  while (foundAlpha(j));
  DO(found(j, '"'));
  return true;
}

static bool parseValue(jsonParseP j) {
  if (found(j, '{')) {
    return parseObject(j);
  }
  if (found(j, '[')) {
    return parseArray(j);
  }
  if (found(j, '"')) {
    return parseString(j);
  }
  if (foundNumeric(j, /*peek*/true)) {
    return parseNumber(j);
  }
  if (foundStr(j, "true")) {
    return parseTrue(j);
  }
  if (foundStr(j, "false")) {
    return parseFalse(j);
  }
  if (foundStr(j, "null")) {
    return parseNull(j);
  }
  return true;
}

void jsonDiscard(jsonNodeP node) {
  assert(node && "invalid json node");

  while (node) {
    jsonNodeP next = node->gc;
    free(node);
    node = next;
  }
}

bool jsonParse(jsonP j, const char* src) {
  assert(j   && "invalid json object");
  assert(src && "invalid json source");

  memset(j, 0, sizeof(jsonT));

  jsonParseT parse = {
    .p   = src,
    .o   = src,
    .top = NULL,
    .gc  = NULL,
  };
  if (!parseJson(&parse)) {
    jsonDiscard(parse.gc);
    return false;
  }

  parseWs(&parse);
  if (!found(&parse, '\0')) {
    jsonDiscard(parse.gc);
    return false;
  }

  j->src = src;
  j->root = parse.top;
  j->gc   = parse.gc;
  return true;
}

void jsonFree(jsonP json) {
  assert(json && "invalid json object");

  jsonDiscard(json->gc);
}

int64_t jsonValue(jsonNodeP node) {
  assert(node->type == jsonNumber);

  int64_t val = 0;
  const char* c = node->src;
  for (; *c >= '0' && *c <= '9'; ++c) {
    val *= 10;
    val += (int64_t)*c - '0';
  }
  return val;
}

bool jsonStrcmp(jsonNodeP node, const char* str) {
  assert(node->type == jsonString || node->type == jsonMember);

  const char* c = node->src;
  for (;; ++str, ++c) {
    if (*str == '\0') {
      return true;
    }
    if (*c == '"') {
      return false;
    }
    if (*str != *c) {
      return false;
    }
  }
}

int32_t jsonStrlen(jsonNodeP node) {
  assert(node && "invalid json node");
  assert(node->type == jsonString || node->type == jsonMember);

  const char* c = node->src;
  for (; *c; ++c) {
    if (*c == '"') {
      return (int32_t)(c - node->src);
    }
  }

  return /*error*/0;
}

bool jsonAsBool(jsonNodeP node) {
  assert(node && "invalid json node");

  if (node->type == jsonTrue) {
    return true;
  }
  if (node->type == jsonFalse) {
    return false;
  }

  assert(!"incompatible json type");
  return false;
}

jsonNodeP jsonFindMember(jsonNodeP node, const char* name) {
  assert(node->type == jsonObject);

  jsonNodeP c = node->child;
  for (; c; c = c->next) {
    assert(c->type == jsonMember);
    if (jsonStrcmp(c, name)) {
      return c;
    }
  }

  return NULL;
}

static void jsonPrintImpl(int i, jsonNodeP n) {
  while (n) {
    // indent
    for (int j = 0; j < i; ++j) { printf("  "); }
    // node name
    switch (n->type) {
    case jsonNumber: {
      const int64_t value = jsonValue(n);
      printf("number (%llu)", value);
      break;
    }
    case jsonString: {
      uint32_t len = jsonStrlen(n);
      printf("string ('%.*s', len=%u)", len, n->src, len);
      break;
    }
    case jsonMember: {
      uint32_t len = jsonStrlen(n);
      printf("member ('%.*s', len=%u)", len, n->src, len);
      break;
    }
    case jsonTrue:   printf("true");   break;
    case jsonFalse:  printf("false");  break;
    case jsonNull:   printf("null");   break;
    case jsonObject: printf("object"); break;
    case jsonArray:  printf("array");  break;
    }
    // new line
    printf("\n");
    // discover children
    if (n->child) {
      jsonPrintImpl(i + 1, n->child);
    }
    // discover sibblings
    n = n->next;
  }
}

void jsonPrint(jsonNodeP node) {
  assert(node && "invalid json node");
  jsonPrintImpl(0, node);
}
