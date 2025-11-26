#include "test_data.hpp"
#include "run_test.hpp"
#include "terminate_benchmark.hpp"

//Generic main function for testing.  run_test() is unique for each test case.


int main() 
{
    bool all_pass = true;
    //run all available test cases
    for (uint32_t i=0; i < NUM_TESTS; i++)
    {
        bool cur_test = run_test(i);
        all_pass = all_pass & cur_test;
    }

    if(all_pass)
    {
        benchmark_success();
    }
    else
    {
        benchmark_failure();
    }

} 
