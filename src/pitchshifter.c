#include <stdlib.h>
#include <math.h>
#include <emscripten/emscripten.h>

int pow2(int n) {
  if (n == 0) {
    return 1;
  }

  return 2 << (n - 1);
}

void FFT(float *x_real, float *x_imag, int N) {
  int number_of_stages = log2(N);

  for (int stage = 1; stage <= number_of_stages; stage++) {
    for (int i = 0; i < pow2(stage - 1); i++) {
      int rest = number_of_stages - stage;

      for (int j = 0; j < pow2(rest); j++) {
        int n = i * pow2(rest + 1) + j;
        int m = pow2(rest) + n;
        int r = j * pow2(stage - 1);

        float a_real = x_real[n];
        float a_imag = x_imag[n];
        float b_real = x_real[m];
        float b_imag = x_imag[m];
        float c_real = cos((2.0 * M_PI * r) / N);
        float c_imag = -sin((2.0 * M_PI * r) / N);

        if (stage < number_of_stages) {
          x_real[n] = a_real + b_real;
          x_imag[n] = a_imag + b_imag;
          x_real[m] = (c_real * (a_real - b_real)) - (c_imag * (a_imag - b_imag));
          x_imag[m] = (c_real * (a_imag - b_imag)) + (c_imag * (a_real - b_real));
        } else {
          x_real[n] = a_real + b_real;
          x_imag[n] = a_imag + b_imag;
          x_real[m] = a_real - b_real;
          x_imag[m] = a_imag - b_imag;
        }
      }
    }
  }

  int *index = (int *)calloc(N, sizeof(int));

  for (int stage = 1; stage <= number_of_stages; stage++) {
    int rest = number_of_stages - stage;

    for (int i = 0; i < pow2(stage - 1); i++) {
      index[pow2(stage - 1) + i] = index[i] + pow2(rest);
    }
  }

  for (int k = 0; k < N; k++) {
    if (index[k] <= k) {
      continue;
    }

    float tmp_real = x_real[index[k]];
    float tmp_imag = x_imag[index[k]];

    x_real[index[k]] = x_real[k];
    x_imag[index[k]] = x_imag[k];

    x_real[k] = tmp_real;
    x_imag[k] = tmp_imag;
  }

  free(index);
}

void IFFT(float *x_real, float *x_imag, int N) {
  int number_of_stages = log2(N);

  for (int stage = 1; stage <= number_of_stages; stage++) {
    for (int i = 0; i < pow2(stage - 1); i++) {
      int rest = number_of_stages - stage;

      for (int j = 0; j < pow2(rest); j++) {
        int n = i * pow2(rest + 1) + j;
        int m = pow2(rest) + n;
        int r = j * pow2(stage - 1);

        float a_real = x_real[n];
        float a_imag = x_imag[n];
        float b_real = x_real[m];
        float b_imag = x_imag[m];
        float c_real = cos((2.0 * M_PI * r) / N);
        float c_imag = sin((2.0 * M_PI * r) / N);

        if (stage < number_of_stages) {
          x_real[n] = a_real + b_real;
          x_imag[n] = a_imag + b_imag;
          x_real[m] = (c_real * (a_real - b_real)) - (c_imag * (a_imag - b_imag));
          x_imag[m] = (c_real * (a_imag - b_imag)) + (c_imag * (a_real - b_real));
        } else {
          x_real[n] = a_real + b_real;
          x_imag[n] = a_imag + b_imag;
          x_real[m] = a_real - b_real;
          x_imag[m] = a_imag - b_imag;
        }
      }
    }
  }

  int *index = (int *)calloc(N, sizeof(int));

  for (int stage = 1; stage <= number_of_stages; stage++) {
    int rest = number_of_stages - stage;

    for (int i = 0; i < pow2(stage - 1); i++) {
      index[pow2(stage - 1) + i] = index[i] + pow2(rest);
    }
  }

  for (int k = 0; k < N; k++) {
    if (index[k] <= k) {
      continue;
    }

    float tmp_real = x_real[index[k]];
    float tmp_imag = x_imag[index[k]];

    x_real[index[k]] = x_real[k];
    x_imag[index[k]] = x_imag[k];

    x_real[k] = tmp_real;
    x_imag[k] = tmp_imag;
  }

  for (int k = 0; k < N; k++) {
    x_real[k] /= N;
    x_imag[k] /= N;
  }

  free(index);
}

EMSCRIPTEN_KEEPALIVE
void pitchshifter(
  double pitch,
  float *reals,
  float *imags,
  float *a_reals,
  float *a_imags,
  int N
) {
  FFT(reals, imags, N);

  for (int i = 0; i < N; i++) {
    int offset = floor(pitch * i);

    int eq = 1;

    if (i > (N / 2)) {
      eq = 0;
    }

    if ((offset >= 0) && (offset < N)) {
      a_reals[offset] += eq * reals[i];
      a_imags[offset] += eq * imags[i];
    }
  }

  IFFT(a_reals, a_imags, N);
}
