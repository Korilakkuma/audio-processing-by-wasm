#include <stdlib.h>
#include <math.h>
#include <emscripten/emscripten.h>

int pow2(int n);
void FFT(float *x_real, float *x_imag, int N);
void IFFT(float *x_real, float *x_imag, int N);

EMSCRIPTEN_KEEPALIVE
void noisesuppressor(double threshold, float *in, float *out, int N) {
  float *x_real = (float *)calloc(N, sizeof(float));
  float *x_imag = (float *)calloc(N, sizeof(float));
  float *A      = (float *)calloc(N, sizeof(float));
  float *T      = (float *)calloc(N, sizeof(float));
  float *y_real = (float *)calloc(N, sizeof(float));
  float *y_imag = (float *)calloc(N, sizeof(float));

  for (int n = 0; n < N; n++) {
    x_real[n] = in[n];
    x_imag[n] = 0.0;
  }

  FFT(x_real, x_imag, N);

  for (int k = 0; k < N; k++) {
    A[k] = sqrt((x_real[k] * x_real[k]) + (x_imag[k] * x_imag[k]));

    if ((x_imag[k] != 0.0) && (x_real[k] != 0.0)) {
      T[k] = atan2(x_imag[k], x_real[k]);
    }
  }

  for (int k = 0; k < N; k++) {
    A[k] -= threshold;

    if (A[k] < 0.0) {
      A[k] = 0.0;
    }
  }

  for (int k = 0; k < N; k++) {
    y_real[k] = A[k] * cos(T[k]);
    y_imag[k] = A[k] * sin(T[k]);
  }

  IFFT(y_real, y_imag, N);

  for (int n = 0; n < N; n++) {
    out[n] = y_real[n];
  }

  free(x_real);
  free(x_imag);
  free(A);
  free(T);
  free(y_real);
  free(y_imag);
}

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
