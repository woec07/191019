#include <iostream>
#include <omp.h>
static long num_steps = 100000000;
double step;
int main ()
{
    step = 1.0/(double) num_steps;
    int i;
    double x, pi, sum = 0.0;
    double  start_time = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp single
             printf(" num_threads = %d",omp_get_num_threads());

        #pragma omp for
        for (i=1;i<= num_steps; i++){
            x = (i-0.5)*step;
            #pragma omp atomic
            sum = sum + 4.0/(1.0+x*x);
            
        }
    }
    pi = step * sum;
    double run_time = omp_get_wtime() - start_time;
    std::cout << "\n pi is " << pi
              << " in " << run_time << " seconds\n";

}
