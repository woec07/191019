#include <iostream>
#include <omp.h>

static long num_steps = 100000000;
double step;

int main() {
    step = 1.0 / static_cast<double>(num_steps);
    double pi = 0.0;

    double start_time = omp_get_wtime();
    double sum = 0.0;
    #pragma omp parallel
    {
        #pragma omp for reduction (+:sum)
        for (long long i = 1; i <= num_steps; ++i) {
            double x = (i - 0.5) * step;
            sum += 4.0 / (1.0 + x * x);
        }

    }
    pi = step * sum;
    double run_time = omp_get_wtime() - start_time;

    std::cout << "\n pi with " << num_steps
              << " steps is " << pi
              << " in " << run_time << " seconds\n";

    return 0;
}

