#include <stdlib.h>
#include <time.h>
#include <emscripten/emscripten.h>

int is_init = 0;

double b0 = 0.0;
double b1 = 0.0;
double b2 = 0.0;
double b3 = 0.0;
double b4 = 0.0;
double b5 = 0.0;
double b6 = 0.0;

double last_out = 0.0;

double EMSCRIPTEN_KEEPALIVE whitenoise(void) {
  if (is_init == 0) {
    srand((unsigned)time(NULL));
    is_init = 1;
  }

  double o = 2 * (((double)rand() / ((double)RAND_MAX + 1.0)) - 0.5);

  return o;
}

double EMSCRIPTEN_KEEPALIVE pinknoise(void) {
  if (is_init == 0) {
    srand((unsigned)time(NULL));
    is_init = 1;
  }

  double n = 2 * (((double)rand() / ((double)RAND_MAX + 1.0)) - 0.5);

  b0 = (0.99886 * b0) + (n * 0.0555179);
  b1 = (0.99332 * b1) + (n * 0.0750759);
  b2 = (0.96900 * b2) + (n * 0.1538520);
  b3 = (0.86650 * b3) + (n * 0.3104856);
  b4 = (0.55000 * b4) + (n * 0.5329522);
  b5 = (-0.7616 * b5) - (n * 0.0168980);

  double o = 0.11 * (b0 + b1 + b2 + b3 + b4 + b5 + b6 + (n * 0.5362));

  b6 = n * 0.115926;

  return o;
}

double EMSCRIPTEN_KEEPALIVE browniannoise(void) {
  if (is_init == 0) {
    srand((unsigned)time(NULL));
    is_init = 1;
  }

  double n = 2 * (((double)rand() / ((double)RAND_MAX + 1.0)) - 0.5);
  double o = 3.5 * ((last_out + (0.02 * n)) / 1.02);

  last_out = (last_out + (0.02 * n)) / 1.02;

  return o;
}
