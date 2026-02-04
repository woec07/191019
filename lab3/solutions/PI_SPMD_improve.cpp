#include <iostream>
#include <omp.h>

static long num_steps = 100000000;
double step;

int main() {
    step = 1.0 / static_cast<double>(num_steps);
    double pi = 0.0;

    double start_time = omp_get_wtime();

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int numthreads = omp_get_num_threads();
        double x;
        double local_sum = 0.0;  // local variable, no false sharing
        #pragma omp single
            printf(" num_threads = %d",omp_get_num_threads());

        for (long i = id; i < num_steps; i += numthreads) {
            x = (i + 0.5) * step;
            local_sum += 4.0 / (1.0 + x * x);
        }
        #pragma omp critical
        // Safely accumulate into global pi
        pi += local_sum * step;
    }

    double run_time = omp_get_wtime() - start_time;
    std::cout << "\n pi is " << pi
              << " in " << run_time << " seconds\n";

    return 0;
}

