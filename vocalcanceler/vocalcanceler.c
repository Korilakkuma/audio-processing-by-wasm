#include <emscripten/emscripten.h>

EMSCRIPTEN_KEEPALIVE
double vocalcanceler(double l, double r, double d) {
  return l - (d * r);
}
