#include <iostream>
#include <omp.h>

static long num_steps = 100000000;
double step;

int main() {
    double start_time = omp_get_wtime();

    // shared variables
    int n_thr = 0;
    step = 1.0 / static_cast<double>(num_steps);
    double pi = 0.0;
    #pragma omp parallel
    {
        // private variables
        int id = omp_get_thread_num();
        n_thr = omp_get_num_threads();
        double sum = 0.0;
       
        for (long long i = id + 1; i <= num_steps; i += n_thr) {
            double x = (i - 0.5) * step;
            sum += 4.0 / (1.0 + x * x);
        }
        // shared variable is only allowed to be accessed by one thread at a certain time
        #pragma omp atomic
        pi += step * sum;
    }

    double run_time = omp_get_wtime() - start_time;
    std::cout << "\n pi with # of steps " << num_steps
                << " (# of threads: " << n_thr
                <<") steps is " << pi
                << " in " << run_time << " seconds\n";
    return 0;
}

