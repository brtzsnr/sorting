#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>

typedef unsigned long long ull;

const double a = (8 * (M_PI - 2)) / (3 * M_PI * (4 - M_PI));
const double sqrt2 = sqrt(2.);

static int size, ezis;
static double *dbuf, *obuf, *copy;

static double phi1(double x)
{
  return 1 / (1 + exp(- x * (0.07056 * x * x + 1.5976)));
}

static double phi(double x)
{
  static const double a[8] = {
    -6.1454887894467542277e-05, +1.4974760603095513788e-03,
    -1.4568393485249561681e-02, +7.0124953874134157705e-02,
    -1.5791447118585119624e-01, +5.6604004874536970049e-02,
    +3.8487173300694088596e-01, +5.0081528927157936693e-01,
  };

  bool flip = x < 0;
  if (flip) x = - x;

  double s = 0., r = 1.;
  for (int i = 7; i >= 0; i--) {
    s += a[i] * r;
    r *= x;
  }

  if (s > 1.) s = 1.;
  return flip ? 1. - s : s;
}

int main(int argc, char **argv) {
  const double scale = 1.75;
  clock_t c0;

  size = atoi(argv[1]);
  ezis = size * scale + 65536;

  dbuf = new double[size];
  obuf = new double[size];
  copy = new double[ezis];

  FILE *f = fopen("gaussian.dat", "r");
  fread(dbuf, size, sizeof(double), f);
  fclose(f);

  c0 = clock();
  memset(copy, 0xff, sizeof(double) * ezis);
  printf("set %.3f\n", (double) ((clock() - c0)) / CLOCKS_PER_SEC);

  c0 = clock();
  for (int i = 0; i < size; i++) {
    obuf[i] = phi(dbuf[i]);
  }
  printf("phi %.3f\n", (double) ((clock() - c0)) / CLOCKS_PER_SEC);

  c0 = clock();
  for (int i = 0; i < size; i++) {
    int pos = (int) (size * scale * obuf[i]);
    while (* (long long *) (copy + pos) != -1)
      pos++;
    copy[pos] = dbuf[i];
  }
  printf("moving %.3f\n", (double) ((clock() - c0)) / CLOCKS_PER_SEC);

  c0 = clock();
  for (int i = 0, pos = 0; i < ezis; i++)
    if (* (long long *) (copy + i) != -1) {
      double value = copy[i];
      int p = pos++;
      for (; p > 0 && value < copy[p - 1]; p--)
        copy[p] = copy[p - 1];
      copy[p] = value;
    }
  printf("shifting %.3f\n", (double) ((clock() - c0)) / CLOCKS_PER_SEC);

#if 0
  // Sorted array
  printf("min %.5f\n", copy[0]);
  printf("max %.5f\n", copy[size - 1]);

  int count = 0;
  for (int i = 1; i < size; i++) {
    if (copy[i - 1] > copy[i]) {
      count++;
    }
  }
  printf("wrong in %d\n", count);
 
  delete [] dbuf;
  delete [] copy;
#endif
  return 0;
}
