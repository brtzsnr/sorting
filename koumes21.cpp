#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <algorithm>

#include <tbb/parallel_for.h>

using namespace std;

double signum(double x) {
    return (x<0) ? -1 : (x>0) ? 1 : 0;
}

const double fourOverPI = 4 / M_PI;

double erf(double x) {
    double a = 0.147;
    double x2 = x*x;
    double ax2 = a*x2;
    double f1 = -x2 * (fourOverPI + ax2) / (1 + ax2);
    double s1 = sqrt(1 - exp(f1));
    return signum(x) * s1;
}

const double sqrt2 = sqrt(2);

double cdf(double x) {
    return 0.5 + erf(x / sqrt2) / 2;
}

const int cdfTableSize = 200;
const double cdfTableLimit = 5;
double* computeCdfTable(int size) {
    double* res = new double[size];
    for (int i = 0; i < size; ++i) {
        res[i] = cdf(cdfTableLimit * i / (size - 1));
    }
    return res;
}
const double* const cdfTable = computeCdfTable(cdfTableSize);

double cdfApprox(double x) {
    bool negative = (x < 0);
    if (negative) x = -x;
    if (x > cdfTableLimit) return negative ? cdf(-x) : cdf(x);
    double p = (cdfTableSize - 1) * x / cdfTableLimit;
    int below = (int) p;
    if (p == below) return negative ? -cdfTable[below] : cdfTable[below];
    int above = below + 1;
    double ret = cdfTable[below] +
            (cdfTable[above] - cdfTable[below])*(p - below);
    return negative ? 1 - ret : ret;
}

void print(const double* arr, int len) {
    for (int i = 0; i < len; ++i) {
        printf("%e; ", arr[i]);
    }
    puts("");
}

void print(const int* arr, int len) {
    for (int i = 0; i < len; ++i) {
        printf("%d; ", arr[i]);
    }
    puts("");
}

void fillBuckets(int N, int bucketCount,
        double* data, int* partitions,
        double* buckets, int* offsets, int* sizes) {
    for (int i = 0; i < N; ++i) {
        ++sizes[partitions[i]];
    }
    int offset = 0;
    for (int i = 0; i < bucketCount; ++i) {
        offsets[i] = offset;
        offset += sizes[i];
    }
    offsets[bucketCount] = N;
    int next[bucketCount];
    memset(next, 0, sizeof(next));
    for (int i = 0; i < N; ++i) {
        int p = partitions[i];
        int j = offsets[p] + next[p];
        ++next[p];
        buckets[j] = data[i];
    }
}

class Sorter {
public:
    Sorter(double* data, int* offsets) {
        this->data = data;
        this->offsets = offsets;
    }

    void operator() (tbb::blocked_range<size_t>& range) const {
        for (int i = range.begin(); i < range.end(); ++i) {
            std::sort(&data[offsets[i]], &data[offsets[i+1]]);
        }
    }

private:
    double* data;
    int* offsets;
};

void sortBuckets(int bucketCount, double* data, int* offsets) {
    tbb::parallel_for(tbb::blocked_range<size_t>(0, bucketCount),
            Sorter(data, offsets));
}

class Partitioner {
public:
    Partitioner(int bucketCount, double* data, int* partitions) {
        this->data = data;
        this->partitions = partitions;
        this->bucketCount = bucketCount;
    }

    void operator() (tbb::blocked_range<size_t>& range) const {
        for (int i = range.begin(); i < range.end(); ++i) {
            double d = data[i];
            int p = (int) (cdfApprox(d) * bucketCount);
            partitions[i] = p;
        }
    }

private:
    double* data;
    int* partitions;
    int bucketCount;
};

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s N\n N = the size of the input\n", argv[0]);
        return 1;
    }

    puts("initializing...");
    int N = atoi(argv[1]);
    double* data = new double[N];
    int bucketCount = 256;
    double* buckets = new double[N];
    int* sizes = new int[bucketCount];
    memset(sizes, 0, sizeof(sizes));
    int* offsets = new int[bucketCount + 1];
    int* partitions = new int[N];

    puts("loading data...");
    FILE* fp = fopen("gaussian.dat", "rb");
    if (fp == 0 || fread(data, sizeof(*data), N, fp) != N) {
        puts("Error reading data");
        return 1;
    }
    //print(data, N);

    puts("assigning partitions...");
    tbb::parallel_for(tbb::blocked_range<size_t>(0, N),
            Partitioner(bucketCount, data, partitions));

    puts("filling buckets...");
    fillBuckets(N, bucketCount, data, partitions, buckets, offsets, sizes);
    data = buckets;

    puts("sorting buckets...");
    sortBuckets(bucketCount, data, offsets);

    puts("done.");

    /*
    for (int i = 0; i < N-1; ++i) {
        if (data[i] > data[i+1]) {
            printf("error at %d: %e > %e\n", i, data[i], data[i+1]);
        }
    }
    */

    //print(data, N);

    return 0;
}
