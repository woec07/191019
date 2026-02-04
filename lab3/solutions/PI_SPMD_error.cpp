#include <iostream>
#include <omp.h>

static long num_steps = 100000000;
double step;
int main() {
    step = 1.0 / static_cast<double>(num_steps);
    double pi = 0.0;

    int nthreads = 0;
    double start_time = omp_get_wtime();

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int numthreads = omp_get_num_threads();
        double x;
        double sum=0.0;

        #pragma omp single //only one thread in the parallel region should execute the following block of code, while all other threads skip it. By default, when that block finishes, all threads synchronize before continuing.
        {
            nthreads = numthreads;
            std::cout << "Only one thread prints this. Thread id: " << nthreads << "\n";
        }
        for (long i = id; i < num_steps; i += numthreads) {
            x = (i + 0.5) * step;
            sum += 4.0 / (1.0 + x * x); //Each thread can safely write to its own sum[id]
        }

        #pragma omp atomic
        pi += step * sum;
    }

    double run_time = omp_get_wtime() - start_time;
    std::cout << "\n pi is " << pi
              << " in " << run_time
              << " seconds with " << nthreads << " threads\n";

    return 0;
}
