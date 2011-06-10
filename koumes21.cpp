#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <algorithm>

#include <tbb/parallel_for.h>

using namespace std;

typedef unsigned long long ull;

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
        double* buckets, int* offsets) {
    for (int i = 0; i < N; ++i) {
        ++offsets[partitions[i]];
    }

    int offset = 0;
    for (int i = 0; i < bucketCount; ++i) {
        int t = offsets[i];
        offsets[i] = offset;
        offset += t;
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

    static void radixSort(double* arr, int len) {
        ull* encoded = (ull*)arr;
        for (int i = 0; i < len; ++i) {
            ull n = encoded[i];
            if (n & signBit) {
                n ^= allBits;
            } else {
                n ^= signBit;
            }
            encoded[i] = n;
        }

        const int step = 11;
        const ull mask = (1ull << step) - 1;
        int offsets[8][1ull << step];
        memset(offsets, 0, sizeof(offsets));

        for (int i = 0; i < len; ++i) {
            for (int b = 0, j = 0; b < 64; b += step, ++j) {
                int p = (encoded[i] >> b) & mask;
                ++offsets[j][p];
            }
        }

        int sum[8] = {0};
        for (int i = 0; i <= mask; i++) {
            for (int b = 0, j = 0; b < 64; b += step, ++j) {
                int t = sum[j] + offsets[j][i];
                offsets[j][i] = sum[j];
                sum[j] = t;
            }
        }

        ull* copy = new ull[len];
        ull* current = encoded;
        for (int b = 0, j = 0; b < 64; b += step, ++j) {
            for (int i = 0; i < len; ++i) {
                int p = (current[i] >> b) & mask;
                copy[offsets[j][p]] = current[i];
                ++offsets[j][p];
            }

            ull* t = copy;
            copy = current;
            current = t;
        }

        if (current != encoded) {
            for (int i = 0; i < len; ++i) {
                encoded[i] = current[i];
            }
        }

        for (int i = 0; i < len; ++i) {
            ull n = encoded[i];
            if (n & signBit) {
                n ^= signBit;
            } else {
                n ^= allBits;
            }
            encoded[i] = n;
        }
    }

    void operator() (tbb::blocked_range<int>& range) const {
        for (int i = range.begin(); i < range.end(); ++i) {
            double* begin = &data[offsets[i]];
            double* end = &data[offsets[i+1]];
            //std::sort(begin, end);
            radixSort(begin, end-begin);
        }
    }

private:
    double* data;
    int* offsets;
    static const ull signBit = 1ull << 63;
    static const ull allBits = ~0ull;
};

void sortBuckets(int bucketCount, double* data, int* offsets) {
    Sorter sorter(data, offsets);
    tbb::blocked_range<int> range(0, bucketCount);
    tbb::parallel_for(range, sorter);
    //sorter(range);
}

class Partitioner {
public:
    Partitioner(int bucketCount, double* data, int* partitions) {
        this->data = data;
        this->partitions = partitions;
        this->bucketCount = bucketCount;
    }

    void operator() (tbb::blocked_range<int>& range) const {
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

const int bucketCount = 512;
int offsets[bucketCount + 1];

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s N\n N = the size of the input\n", argv[0]);
        return 1;
    }

    puts("initializing...");
    int N = atoi(argv[1]);
    double* data = new double[N];
    double* buckets = new double[N];
    memset(offsets, 0, sizeof(offsets));
    int* partitions = new int[N];

    puts("loading data...");
    FILE* fp = fopen("gaussian.dat", "rb");
    if (fp == 0 || fread(data, sizeof(*data), N, fp) != N) {
        puts("Error reading data");
        return 1;
    }
    //print(data, N);

    puts("assigning partitions...");
    tbb::parallel_for(tbb::blocked_range<int>(0, N),
            Partitioner(bucketCount, data, partitions));

    puts("filling buckets...");
    fillBuckets(N, bucketCount, data, partitions, buckets, offsets);
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
