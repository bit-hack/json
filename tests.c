#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "json.h"


static void error(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

static const char* xfails[] = {
  "[", "]", "[[ ]",
  "\"",
  ".0", "0..0", "--1", "0.0.", ".0.0", "- 1", "0. 0", "0 .0",
  // extended ascii characters
  "\"£\"", "\"¬\"",
  // random things
  "FOoBar",
  // sentinel
  NULL
};

static bool expectNumber(const char* str, double value) {
  jsonT j = { 0 };
  if (!jsonParse(&j, str)) {
    error("failed to parse %s\n", str);
    return false;
  }
  jsonNodeP n = j.root;
  if (jsonValueD(n) != value) {
    error("value error for %s expecting %f\n", str, value);
    return false;
  }
  return true;
}

static void checkNumbers(void) {
  printf("---- %s ----\n", __func__);

  struct pair {
    const char* str;
    double num;
  };

  static const struct pair pairs[] = {
    { "0",   0.0 },
    { "0.0", 0.0 },
    { "0.",  0.  },

    { "1.1", 1.1 },
    { "1.1", 1.1 },
    { "1.1", 1.1 },

    { "123.456",   123.456 },
    { "-123.456", -123.456 },

    { "-0",   0.0 },
    { "-0.0", 0.0 },
    { "-0.",  0.  },

    { "-1",   -1.0 },
    { "-1.0", -1.0 },
    { "-1.",  -1.  },

    { "-1.1", -1.1 },

    { "-.1", -0.1 },

    { NULL,  0.0 }
  };

  const struct pair* p = pairs;
  for (; p->str; ++p) {
    expectNumber(p->str, p->num);
  }
}

static void checkString(void) {
  printf("---- %s ----\n", __func__);

  const char* tests[] = {
    "\"Hello World!\"",
    "\"0123456789\"",
    "\"abcdefghijklmnopqrstuvwxyz\"",
    "\"ABCDEFGHIJKLMNOPQRSTUVWXYZ\"",
    "\"!$%^&*()_+-=<>,./?'@#~[]{}`\"",
    NULL
  };

  for (uint32_t i = 0; ; ++i) {

    const char* t = tests[i];
    if (!t) {
      break;
    }

    jsonT j = { 0 };
    if (!jsonParse(&j, t)) {
      assert(!"jsonParse failed");
      continue;
    }

    jsonNodeP n = jsonRoot(&j);
    if (jsonStrcmp(n, t) != 0) {
      error("value error expecting %s\n", t);
    }

    jsonFree(&j);
  }
}

static bool checkArray(void) {
  printf("---- %s ----\n", __func__);

  const char* t = "[ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ]";

  jsonT json = { 0 };
  if (!jsonParse(&json, t)) {
    assert(!"jsonParse failed");
  }

  jsonNodeP n = jsonRoot(&json);
  assert(n);
  assert(!n->next);
  assert(n->type == jsonArray);

  jsonNodeP a = jsonChild(n);
  assert(a);
  assert(a->type == jsonNumber);
  assert(!a->child);
  assert(a->next);

  for (uint32_t i = 0; i < 10; ++i) {

    assert(a->type == jsonNumber);
    const int64_t v = jsonValueI(a);
    assert(v == i);

    a = jsonNext(a);
  }

  assert(a == NULL);
  jsonFree(&json);
  return true;
}

static bool checkObject(void) {
  printf("---- %s ----\n", __func__);

  const char* t = "{ \"a\" : 0, \"b\" : 1, \"c\" : 2 }";

  jsonT json = { 0 };
  if (!jsonParse(&json, t)) {
    assert(!"jsonParse failed");
  }

  jsonNodeP n = jsonRoot(&json);
  assert(n);
  assert(!n->next);
  assert(n->type == jsonObject);

  jsonNodeP m = jsonChild(n);
  assert(m->type == jsonMember);
  assert(jsonStrcmp(m, "a"));
  assert(jsonChild(m));
  assert(jsonChild(m)->type == jsonNumber);
  assert(jsonValueI(jsonChild(m)) == 0);

  jsonNodeP a = jsonFindMember(n, "b");
  assert(a);
  assert(a->type == jsonMember);
  assert(jsonChild(a));
  assert(jsonChild(a)->type == jsonNumber);
  assert(jsonValueI(jsonChild(a)) == 1);

  jsonNodeP d = jsonFindMember(n, "d");
  assert(!d);

  jsonFree(&json);
  return true;
}

bool checkKeywords(void) {
  printf("---- %s ----\n", __func__);

  const char* t = "[ true, false, null ]";

  jsonT json = { 0 };
  if (!jsonParse(&json, t)) {
    assert(!"jsonParse failed");
  }

  jsonNodeP n = jsonRoot(&json);
  assert(n);
  assert(!n->next);
  assert(n->type == jsonArray);

  jsonNodeP a = jsonChild(n);
  assert(a);
  assert(a->type == jsonTrue);

  jsonNodeP b = jsonNext(a);
  assert(b);
  assert(b->type == jsonFalse);

  jsonNodeP c = jsonNext(b);
  assert(c);
  assert(c->type == jsonNull);

  assert(jsonNext(c) == NULL);

  jsonFree(&json);
  return true;
}

static bool checkXFails(void) {
  printf("---- %s ----\n", __func__);

  const char** xf = xfails;
  for (; *xf; ++xf) {
    jsonT j = { 0 };
    if (jsonParse(&j, *xf)) {
      error("xfail: '%s'\n", *xf);
    }
  }

  return true;
}

int main(int argc, char** args) {

  checkXFails();
  checkKeywords();
  checkNumbers();
  checkString();
  checkArray();
  checkObject();

  return 0;
}
