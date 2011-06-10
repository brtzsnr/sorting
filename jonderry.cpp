#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <ctime>

using std::fill;

const double q[] = {
  0.0,
  9.865E-10,
  2.8665150000000003E-7,
  3.167E-5,
  0.001349898,
  0.022750132,
  0.158655254,
  0.5,
  0.8413447460000001,
  0.9772498679999999,
  0.998650102,
  0.99996833,
  0.9999997133485,
  0.9999999990134999,
  1.0,
};
int main(int argc, char** argv) {
  if (argc <= 1)
    return puts("No argument!");
  unsigned count = atoi(argv[1]);
  unsigned count2 = 3 * count;

  bool *ba = new bool[count2 + 1000];
  fill(ba, ba + count2 + 1000, false);
  double *a = new double[count];
  double *c = new double[count2 + 1000];

  FILE *f = fopen("gaussian.dat", "rb");
  if (fread(a, 8, count, f) != count)
    return puts("fread failed!");
  fclose(f);

  int i;
  int j;
  bool s;
  int t;
  double z;
  double p;
  double d1;
  double d2;
  for (i = 0; i < count; i++) {
    s = a[i] < 0;
    t = a[i];
    if (s) t--;
    z = a[i] - t;
    t += 7;
    if (t < 0) {
      t = 0;
      z = 0;
    } else if (t >= 14) {
      t = 13;
      z = 1;
    }
    p = q[t] * (1 - z) + q[t + 1] * z;
    j = count2 * p;
    while (ba[j] && c[j] < a[i]) {
      j++;
    }
    if (!ba[j]) {
      ba[j] = true;
      c[j] = a[i];
    } else {
      d1 = c[j];
      c[j] = a[i];
      j++;
      while (ba[j]) {
        d2 = c[j];
        c[j] = d1;
        d1 = d2;
        j++;
      }
      c[j] = d1;
      ba[j] = true;
    }
  }
  i = 0;
  int max = count2 + 1000;
  for (j = 0; j < max; j++) {
    if (ba[j]) {
      a[i++] = c[j];
    }
  }
  // for (i = 0; i < count; i += 1) {
  //   printf("here %f\n", a[i]);
  // }
  return 0;
}
