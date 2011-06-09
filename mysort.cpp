#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <ctime>

typedef unsigned long long ull;

int size;
double *dbuf, *copy;
int cnt[8][1 << 16];

void print(const char *message)
{
  printf("\n%s:\n", message);
  ull *ibuf = (ull *) dbuf;

  for (int i = 0; i < size; i++) {
    printf("%+10.9f ", dbuf[i]);
    for (int j = 63; j >= 0; j--) {
      printf("%d", (ibuf[i] & (1ULL << j)) != 0);
      if (j == 63 || j == 52) printf(" ");
    }
    printf("\n");
  }
}

void sort()
{
  const int step = 10;
  const int start = 24;
  ull mask = (1ULL << step) - 1;

  ull *ibuf = (ull *) dbuf;
  for (int i = 0; i < size; i++) {
    for (int w = start, v = 0; w < 64; w += step, v++) {
      int p = (ibuf[i] >> w) & mask;
      cnt[v][p]++;
    }
  }

  int sum[8] = { 0 };
  for (int i = 0; i <= mask; i++) {
    for (int w = start, v = 0; w < 64; w += step, v++) {
      int tmp = sum[v] + cnt[v][i];
      cnt[v][i] = sum[v];
      sum[v] = tmp;
    }
  }

  for (int w = start, v = 0; w < 64; w += step, v++) {
    ull *ibuf = (ull *) dbuf;
    for (int i = 0; i < size; i++) {
      int p = (ibuf[i] >> w) & mask;
      copy[cnt[v][p]++] = dbuf[i];
    }

    double *tmp = copy;
    copy = dbuf;
    dbuf = tmp;
  }

  std::reverse(dbuf, dbuf + size);
  for (int p = 0; p < size; p++)
    if (dbuf[p] >= 0.) {
      std::reverse(dbuf + p, dbuf + size);
      break;
    }

  // Insertion sort
  for (int i = 1; i < size; i++) {
    double value = dbuf[i];
    if (value < dbuf[i - 1]) {
      dbuf[i] = dbuf[i - 1];
      int p = i - 1;
      for (; p > 0 && value < dbuf[p - 1]; p--)
        dbuf[p] = dbuf[p - 1];
      dbuf[p] = value;
    }
  }
}

int main(int argc, char **argv) {
  size = atoi(argv[1]);
  dbuf = new double[size];
  copy = new double[size];

  FILE *f = fopen("gaussian.dat", "r");
  fread(dbuf, size, sizeof(double), f);
  fclose(f);

  clock_t c0 = clock();
  sort();
  printf("Finished after %.3f\n", (double) ((clock() - c0)) / CLOCKS_PER_SEC);

  // print("sorted");

#if 0
  int count = 0;
  for (int i = 1; i < size; i++) {
    if (dbuf[i - 1] > dbuf[i]) {
      count++;
    }
  }
  printf("wrong in %d\n", count);
 
  delete [] dbuf;
  delete [] copy;
#endif
  return 0;
}
