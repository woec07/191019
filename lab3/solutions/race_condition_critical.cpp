#include <omp.h>
#include <iostream>


int main(){

    int y[2]={0};
    int x[100];
    int c[20];

    for (int i=0;i<100;i++) {
        x[i]=1;
    }
    for (int j=0;j<20;j++) {
        c[j]=j%2+1;
    }

    for (int run=0; run < 10; run++) {
        y[0]=0;
        y[1]=0;

        double  start_time = omp_get_wtime();
        for (int j=0;j<20;j++)
        {
            #pragma omp parallel for
            for (int i=0; i<81; i++)
            {
            #pragma omp critical
            {
                y[j%2] += c[j] * x[i+j];
            }
            }
        }
        double run_time = omp_get_wtime() - start_time;
        std::cout << "y[0]=" << y[0]  << ",y[1]=" << y[1] << " run="<< run<< " runtime="<< run_time<<" (with pragma omp critical)" << std::endl;
    }

} // end main

