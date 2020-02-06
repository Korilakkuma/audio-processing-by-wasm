#include <stdlib.h>
#include <math.h>
#include <emscripten/emscripten.h>

/*
void timestretch(double rate, float *in, float *out, int fs, int length) {
  if ((rate == 1.0) || (rate <= 0.0)) {
    for (int n = 0; n < length; n++) {
      out[n] = in[n];
    }

    return;
  }

  int template_size = length;
  int p_min = (int)(length * 0.005);
  int p_max = (int)(length * 0.02);

  float *x = (float *)calloc(template_size, sizeof(float));
  float *y = (float *)calloc(template_size, sizeof(float));
  float *r = (float *)calloc((p_max + 1), sizeof(float));

  int offset0 = 0;
  int offset1 = 0;

    for (int n = 0; n < template_size; n++) {
      x[n] = in[offset0 + n];
    }

    float max_of_r = 0.0;
    int p = p_min;

    for (int m = p_min; m <= p_max; m++) {
      for (int n = 0; n < template_size; n++) {
        y[n] = in[offset0 + m + n];
      }

      r[m] = 0.0;

      for (int n = 0; n < template_size; n++) {
        r[m] += x[n] * y[n];
      }

      if (r[m] > max_of_r) {
        max_of_r = r[m];
        p = m;
      }
    }

    if (rate < 1.0) {
      for (int n = 0; n < p; n++) {
        out[offset1 + n] = in[offset0 + n];
      }
    }

    for (int n = 0; n < p; n++) {
      if (rate > 1.0) {
        out[offset1 + n] = (in[offset0 + n] * (p - n)) / p;
        out[offset1 + n] += (in[offset0 + p + n] * n) / p;
      } else if (rate < 1.0) {
        out[offset1 + p + n] = (in[offset0 + p + n] * (p - n)) / p;
        out[offset1 + p + n] += (in[offset0 + n] * n) / p;
      }
    }

    int q = 0;

    if (rate > 1.0) {
      q = (int)((p / (rate - 1.0)) + 0.5);
    } else if (rate < 1.0) {
      q = (int)(((p * rate) / (1.0 - rate)) + 0.5);
    }

    if (rate > 1.0) {
      for (int n = p; n < q; n++) {
        if ((offset0 + p + n) >= length) {
          break;
        }

        out[offset1 + n] = in[offset0 + p + n];
      }

      offset0 += p + q;
      offset1 += q;
    } else if (rate < 1.0) {
      for (int n = p; n < q; n++) {
        if ((offset0 + n) >= length) {
          break;
        }

        out[offset1 + p + n] = in[offset0 + n];
      }

      offset0 += q;
      offset1 += p + q;
    }

  free(x);
  free(y);
  free(r);
}
*/

int pow2(int n);
void FFT(float *x_real, float *x_imag, int N);
void IFFT(float *x_real, float *x_imag, int N);
void interpolate(float *in, float *out, double rate, int fs, int length);

EMSCRIPTEN_KEEPALIVE
void timestretch(
  double rate,
  float *in,
  float *out,
  int fs,
  int length
) {
  if (rate <= 0.0) {
    for (int n = 0; n < length; n++) {
      out[n] = in[n];
    }

    return;
  }

  int fit_count = length / rate;

  float *reals   = (float *)calloc(length, sizeof(float));
  float *imags   = (float *)calloc(length, sizeof(float));
  float *a_reals = (float *)calloc(length, sizeof(float));
  float *a_imags = (float *)calloc(length, sizeof(float));

  for (int n = 0; n < length; n++) {
    int m = (int)((double)n / rate);

    // Interpolate
    int l = (int)((double)(n + 1) / rate);
    int d = l - m;

    for (int o = 0; o < d; o++) {
      if ((m + o) < length) {
        if (rate >= 1.0) {
          out[n] = in[m + o];
        } else {
          out[m + o] = in[n];
        }
      }
    }
  }

  /*
  for (int n = 0; n < length; n++) {
    reals[n] = out[n];
  }

  FFT(reals, imags, length);

  for (int k = 0; k < length; k++) {
    int offset = floor(k / rate);

    int eq = 1;

    if (k > (length / 2)) {
      eq = 0;
    }

    if ((offset >= 0) && (offset < length)) {
      a_reals[offset] += eq * reals[k];
      a_imags[offset] += eq * imags[k];
    }
  }

  IFFT(a_reals, a_imags, length);

  for (int n = 0; n < length; n++) {
    out[n] = a_reals[n];
  }
  */

  free(reals);
  free(imags);
  free(a_reals);
  free(a_imags);
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

void interplate(float *in, float *out, double fit_count, int length) {
  double factor = (length - 1.0) / (fit_count - 1.0);

  out[0] = in[0];

  for (int n = 1; n < ((int)fit_count - 1); n++) {
    double tmp = factor * n;

    int b = (int)floor(tmp);
    int a = (int)ceil(tmp);

    double at = tmp - b;

    out[n] = (float)(in[b] + (at * (in[a] - in[b])));
  }

  out[(int)fit_count - 1] = in[length - 1];
}
