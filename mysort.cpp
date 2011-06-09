#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <ctime>
#include <sys/mman.h>

using std::fill;
using std::reverse;

typedef unsigned long long ull;

int size;
double *dbuf, *copy;
int cnt[1 << 16];

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

void sort(int s, int e)
{
  ull *ibuf = (ull *) dbuf;
  ull mask = (1 << (e - s)) - 1;

  clock_t c1;
  
  c1 = clock();
  fill(cnt, cnt + (1 << (e - s)), 0);
  for (int i = 0; i < size; i++) {
    int p = (ibuf[i] >> s) & mask;
    cnt[p]++;
  }
  printf("1 %.3f\n", (double) ((clock() - c1)) / CLOCKS_PER_SEC);

  c1 = clock();
  int sum = 0;
  for (int i = 0; i <= mask; i++) {
    int tmp = sum + cnt[i];
    cnt[i] = sum;
    sum = tmp;
  }
  printf("2 %.3f\n", (double) ((clock() - c1)) / CLOCKS_PER_SEC);

  c1 = clock();
  for (int i = 0; i < size; i++) {
    int p = (ibuf[i] >> s) & mask;
    copy[cnt[p]++] = dbuf[i];
  }
  printf("3 %.3f\n", (double) ((clock() - c1)) / CLOCKS_PER_SEC);

  double *tmp = copy;
  copy = dbuf;
  dbuf = tmp;
}

int main(int argc, char **argv) {
  size = atoi(argv[1]);
  dbuf = new double[size];
  copy = new double[size];

  FILE *f = fopen("gaussian.dat", "r");
  fread(dbuf, size, sizeof(double), f);
  fclose(f);

  clock_t c0 = clock();

  int step = 11;
  for (int i = 0; i < 64; i += step) {
    sort(i, i + step);
  }

  int p;
  for (p = 0; p < size; p++)
    if (dbuf[p] < 0) break;
  reverse(dbuf, dbuf + p);
  reverse(dbuf, dbuf + size);
  print("sorted");

  printf("Finished after %.3f\n", (double) ((clock() - c0)) / CLOCKS_PER_SEC);
 
  // delete [] dbuf;
  // delete [] copy;
  return 0;
}
