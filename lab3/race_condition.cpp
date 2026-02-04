#include <omp.h>
#include <iostream>


int main(){

    double start_time = omp_get_wtime();

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

        for (int j=0;j<20;j++)
        {
            #pragma omp parallel for
            for (int i=0; i<81; i++)
            {
                #pragma omp atomic
                y[j%2] += c[j] * x[i+j];

            }
        }

        std::cout << "y[0]=" << y[0]  << ",y[1]=" << y[1] <<" run="<< run<< " (without pragma omp critical)" << std::endl;
    }
    double total_time = omp_get_wtime() - start_time;

    std::cout << "\n Total time used: " << total_time << std::endl; 



} // end main

