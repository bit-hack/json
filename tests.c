#include <stdio.h>
#include "json.h"

static const char* xfails[] = {
  "[", "]", "[[ ]",
  ".0", "0..0", "--1", "0.0.", ".0.0", "- 1", "0. 0", "0 .0",
  NULL
};

bool expectNumber(const char* str, double value) {
  jsonT j = { 0 };
  if (!jsonParse(&j, str)) {
    fprintf(stderr, "failed to parse %s\n", str);
    return false;
  }
  jsonNodeP n = j.root;
  if (jsonValueD(n) != value) {
    fprintf(stderr, "value error for %s expecting %f\n", str, value);
    return false;
  }
  return true;
}

void checkNumbers() {

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

int main(int argc, char** args) {

  const char** xf = xfails;
  for (; *xf; ++xf) {
    jsonT j = { 0 };
    if (jsonParse(&j, *xf)) {
      fprintf(stderr, "xfail: '%s'\n", *xf);
    }
  }

  checkNumbers();

  return 0;
}
