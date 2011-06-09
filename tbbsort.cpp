#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <ctime>
#include <tbb/parallel_sort.h>

int main(void)
{
    std::ifstream ifs("gaussian.dat", std::ios::binary | std::ios::in);
    std::vector<double> values;
    values.reserve(50000000);
    double d;
    while (ifs.read(reinterpret_cast<char*>(&d), sizeof(double)))
    values.push_back(d);
    clock_t c0 = clock();
    tbb::parallel_sort(values.begin(), values.end());
    std::cout << "Finished after "
              << static_cast<double>((clock() - c0)) / CLOCKS_PER_SEC
              << std::endl;
}
