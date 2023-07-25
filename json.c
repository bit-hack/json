#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "json.h"


#define DO(EXPR, PARSER, MSG)  \
  if (!(EXPR)) {       \
    error(PARSER, MSG);        \
    return false;      \
  }

typedef struct jsonParseS {

  jsonNodeP   top;    // work space
  const char* start;  // start point of json
  const char* o;      // old parse pointer
  const char* p;      // parse pointer
  jsonNodeP   gc;     // garbage collection chain

} jsonParseT, *jsonParseP;

static bool parseElement (jsonParseP j);
static bool parseElements(jsonParseP j);
static bool parseValue   (jsonParseP j);
static bool parseString  (jsonParseP j);

static void error(jsonParseP j, const char* msg) {
  const char* x = j->start;

  uint32_t line = 0;
  uint32_t col  = 0;

  for (; x < j->o; ++x) {
    col   = (*x == '\n') ? 0 : col + 1;
    line += (*x == '\n') ? 1 : 0;
  }

  fprintf(stderr, "line:%u:%u %s\n", line, col, msg);
}

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

static void inline advance(jsonParseP j) {
  j->o = (j->p)++;
}

static bool found(jsonParseP j, char ch) {
  if (*j->p == ch) {
    advance(j);
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

static bool foundAlphaNum(jsonParseP j) {
  const char ch = *j->p;
  // all printable ascii chars except quote
  // todo: escape chars
  if ((ch >= ' ' && ch <= '~') && ch != '"') {
    advance(j);
    return true;
  }
  return false;
}

static bool foundNumeric(jsonParseP j, bool peek, bool minus) {
  const char ch = *j->p;
  if (ch >= '0' && ch <= '9' || (minus && ch == '-')) {
    if (!peek) {
      advance(j);
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
  DO(parseWs(j),    j, "Whitespace expected");
  DO(parseValue(j), j, "Value expected");
  DO(parseWs(j),    j, "Whitespace expected");
  return true;
}

static bool parseMember(jsonParseP j) {
  DO(parseWs(j),       j, "Whitespace expected");
  DO(found(j, '"'), j, "'\"' expected");

  DO(parseString(j), j, "String expected"); // key
  assert(j->top);
  jsonNodeP node = j->top;
  node->type = jsonMember;

  DO(parseWs(j),       j, "Whitespace expected");
  DO(found(j, ':'), j, "':' expected");

  DO(parseElement(j),  j, "Element expected"); // value
  node->child = j->top;
  j->top = node;
  return true;
}

static bool parseMembers(jsonParseP j) {

  jsonNodeP first = NULL;
  jsonNodeP node  = NULL;

  do {
    DO(parseMember(j), j, "Member expected");

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
    DO(parseElement(j), j, "Element expected");

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
  DO(parseWs(j), j, "Whitespace expected");

  if (!found(j, '}')) {

    DO(parseMembers(j), j, "Members expected");
    node->child = j->top;
    j->top = node;

    DO(found(j, '}'), j, "'}' expected");
    return true;
  }

  j->top = node;
  return true;
}

static bool parseArray(jsonParseP j) {
  assert(*(j->o) == '[');

  jsonNodeP node = newNode(j, j->o, jsonArray);

  DO(parseWs(j), j, "Whitespace expected");
  if (!found(j, ']')) {
    DO(parseElements(j), j, "Elements expected");

    node->child = j->top;
    j->top = node;

    DO(found(j, ']'), j, "']' expected");
  }

  j->top = node;
  return true;
}

static bool parseNumber(jsonParseP j) {
  const char* p = j->p;
  const char* o = j->o;
  j->top = newNode(j, j->p, jsonNumber);

  // consume any unary minus
  found(j, '-');

  while (foundNumeric(j, /*peek*/false, /*minus*/false));

  // parse optional fractional part
  if (found(j, '.')) {
    while (foundNumeric(j, /*peek*/false, /*minus*/false));
  }

  // ensure we parsed something
  if (j->p != p) {
    return true;
  }

  // failure
  j->o = o;
  error(j, "Error parsing number");
  return /*error*/false;
}

static bool parseString(jsonParseP j) {
  assert(*(j->o) == '"');

  j->top = newNode(j, j->p, jsonString);

  while (foundAlphaNum(j));
  DO(found(j, '"'), j, "'\"' expected");
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
  if (foundNumeric(j, /*peek*/true, /*minus*/true)) {
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
  // nothing valid found
  error(j, "unexpected token");
  return /*error*/false;
}

static void jsonDiscard(jsonNodeP node) {
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
    .start = src,
    .p     = src,
    .o     = src,
    .top   = NULL,
    .gc    = NULL,
  };
  if (!parseJson(&parse)) {
    jsonDiscard(parse.gc);
    return /*error*/false;
  }

  parseWs(&parse);
  if (!found(&parse, '\0')) {
    jsonDiscard(parse.gc);
    return /*error*/false;
  }

  j->src = src;
  j->root = parse.top;
  j->gc   = parse.gc;
  return true;
}

void jsonFree(jsonP json) {
  assert(json && "invalid json object");

  if (json->gc) {
    jsonDiscard(json->gc);
  }
}

int64_t jsonValueI(jsonNodeP node) {
  assert(node->type == jsonNumber);
  const char* c = node->src;

  const bool minus = (*c == '-');
  c += minus ? 1 : 0;

  int64_t val = 0;
  for (; *c >= '0' && *c <= '9'; ++c) {
    val *= 10;
    val += (int64_t)*c - '0';
  }

  return minus ? -val : val;
}

double jsonValueD(jsonNodeP node) {
  assert(node->type == jsonNumber);
  const char* c = node->src;

  const bool minus = (*c == '-');
  c += minus ? 1 : 0;

  int64_t val   = 0;
  int64_t fract = 0;

  for (; *c >= '0' && *c <= '9' || *c == '.'; ++c) {
    if (*c == '.') {
      fract = 1;
    }
    else {
      fract *= 10;
      val   *= 10;
      val   += (int64_t)*c - '0';
    }
  }

  const double denom = fract ? (double)fract : 1.0;
  return (double)(minus ? -val : val) / denom;
}

bool jsonStrcmp(jsonNodeP node, const char* str) {
  assert(node->type == jsonString || node->type == jsonMember);

  const char* c = node->src;
  for (;; ++str, ++c) {
    if (*str == '\0') { return true;  }
    if (*c == '"')    { return false; }
    if (*str != *c)   { return false; }
  }
}

uint32_t jsonStrlen(jsonNodeP node) {
  assert(node && "invalid json node");
  assert(node->type == jsonString || node->type == jsonMember);

  const char* c = node->src;
  for (; *c; ++c) {
    if (*c == '"') {
      return (uint32_t)(c - node->src);
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
  return /*error*/false;
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

void jsonValidate(jsonNodeP n) {
  while (n) {
    assert(n->src);

    switch (n->type) {
    case jsonMember:
    case jsonObject:
    case jsonArray:
      break;
    case jsonString:
    case jsonNumber:
    case jsonTrue:
    case jsonFalse:
    case jsonNull:
      assert(n->child == NULL);
    }

    if (n->child) {
      jsonValidate(n->child);
    }

    n = n->next;
  }
}

static void jsonPrintImpl(int i, jsonNodeP n) {
  while (n) {
    // indent
    for (int j = 0; j < i; ++j) { printf("  "); }
    // node name
    switch (n->type) {
    case jsonNumber: {
      const double value = jsonValueD(n);
      printf("number (%f)", value);
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
    jsonPrintImpl(i + 1, jsonChild(n));
    // discover sibblings
    n = jsonNext(n);
  }
}

jsonNodeP jsonRoot(jsonP json) {
  assert(json && "invalid json");
  return json->root;
}

void jsonPrint(jsonNodeP node) {
  assert(node && "invalid json node");
  jsonPrintImpl(0, node);
}

jsonNodeP jsonNext(jsonNodeP node) {
  assert(node && "invalid json node");
  return node->next;
}

jsonNodeP jsonChild(jsonNodeP node) {
  assert(node && "invalid json node");
  return node->child;
}
