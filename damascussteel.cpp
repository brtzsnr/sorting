#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <thread>
#include <cassert>
#include <vector>

using std::fill;
using std::reverse;

typedef unsigned long long ull;

#ifndef STEP
#define STEP 8
#endif

const int step = STEP;
const int start_step=24;
const int num_steps=(64-start_step)/step;
int size;
double *dbuf, *copy;

const int n_partitions=16;
const double distrib[]={0,
                        -1.5341,
                        -1.1503,
                        -0.88715,
                        -0.67449,
                        -0.48878,
                        -0.31864,
                        -0.15731,
                        0,
                        0.15731,
                        0.31864,
                        0.48878,
                        0.67449,
                        0.88715,
                        1.1503,
                        1.5341};

void sort_subpart(const int start, const int end, double *dbuf_local,
                  double *copy_local)
{
  ull mask = (1 << step) - 1;
  int cnt[num_steps][mask+1];

  ull *ibuf = (ull *) (dbuf_local);

  for(int i=0;i<num_steps;++i)
    for(int j=0;j<mask+1;++j)
      cnt[i][j]=0;

  for (int i = start; i < end; i++)
    {
      for (int w = start_step, v = 0; w < 64; w += step, v++)
        {
          int p = (~ibuf[i] >> w) & mask;
          (cnt[v][p])++;
        }
    }

  std::vector<int> sum(num_steps,0);
  for (int i = 0; i <= mask; i++)
    {
      for (int w = start_step, v = 0; w < 64; w += step, v++)
        {
          int tmp = sum[v] + cnt[v][i];
          cnt[v][i] = sum[v];
          sum[v] = tmp;
        }
    }

  for (int w = start_step, v = 0; w < 64; w += step, v++)
    {
      ibuf = (ull *) dbuf_local;

      for (int i = start; i < end; i++)
        {
          int p = (~ibuf[i] >> w) & mask;
          copy_local[start+((cnt[v][p])++)] = dbuf_local[i];
        }
      std::swap(copy_local,dbuf_local);
    }

  // Do the last set of reversals
  for (int p = start; p < end; p++)
    if (dbuf_local[p] >= 0.)
      {
        reverse(dbuf_local+p, dbuf_local + end);
        break;
      }

  // Insertion sort
  for (int i = start+1; i < end; i++) {
    double value = dbuf_local[i];
    if (value < dbuf_local[i - 1]) {
      dbuf_local[i] = dbuf_local[i - 1];
      int p = i - 1;
      for (; p > 0 && value < dbuf_local[p - 1]; p--)
        dbuf_local[p] = dbuf_local[p - 1];
      dbuf_local[p] = value;
    }
  }
}

void pivot(double* start,double * end, double middle, size_t& koniec)
{
  double * beg=start;
  end--;
  while (start!=end)
    {
      if (*start>middle)
        std::swap (*start,*end--);
      else
        start++;
    }
  if (*end<middle)
    start+=1;
  koniec= start-beg;
}

void sort_part(const int start, const int end, double *dbuf_local,
               double *copy_local, const int index)
{
#ifndef SPLIT
  sort_subpart(start,end,dbuf_local,copy_local);
#else
  size_t end1,end2,end3,temp;

  pivot(dbuf_local+start, dbuf_local+end,distrib[index + n_partitions/8],end2);
#if SPLIT == 8
  sort_subpart(start,start+end2,dbuf_local,copy_local);
  sort_subpart(start+end2,end,dbuf_local,copy_local);
#else

  pivot(dbuf_local+start,
        dbuf_local+start+end2,distrib[index + n_partitions/16],end1);
  pivot(dbuf_local+start+end2,
        dbuf_local+end,distrib[index + 3*n_partitions/16],end3);
  end3+=end2;

  sort_subpart(start,start+end1,dbuf_local,copy_local);
  sort_subpart(start+end1,start+end2,dbuf_local,copy_local);
  sort_subpart(start+end2,start+end3,dbuf_local,copy_local);
  sort_subpart(start+end3,end,dbuf_local,copy_local);

#endif
#endif

}

int main(int argc, char **argv) {
  size = atoi(argv[1]);
  dbuf = new double[size];
  copy = new double[size];

  FILE *f = fopen("gaussian.dat", "r");
  fread(dbuf, size, sizeof(double), f);
  fclose(f);

  clock_t c0 = clock();

  size_t end1,end2,end3;

  pivot(dbuf, dbuf+size,distrib[n_partitions/2],end2);
  pivot(dbuf, dbuf+end2,distrib[n_partitions/4],end1);
  pivot(dbuf+end2,dbuf+size,distrib[3*n_partitions/4],end3);
  end3+=end2;

  std::thread ts1(sort_part,0,end1,dbuf,copy,0);
  std::thread ts2(sort_part,end1,end2,dbuf,copy,n_partitions/4);
  std::thread ts3(sort_part,end2,end3,dbuf,copy,n_partitions/2);
  std::thread ts4(sort_part,end3,size,dbuf,copy,3*n_partitions/4);
  ts1.join(),ts2.join(),ts3.join(),ts4.join();

  if(num_steps%2==1)
    std::swap(dbuf,copy);

  for (int i=0; i<size-1; i++){
    if (dbuf[i]>dbuf[i+1])
      std::cout << "BAD "
                << i << " "
                << dbuf[i] << " "
                << dbuf[i+1] << " "
                << "\n";
  }

  std::cout << "Finished after "
            << (double) ((clock() - c0)) / CLOCKS_PER_SEC
            << "\n";

  // delete [] dbuf;
  // delete [] copy;
  return 0;
}
