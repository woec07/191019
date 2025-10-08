#ifndef RUN_TEST_HPP
#define RUN_TEST_HPP 

#include <cstdint>
#include "vicuna_crt.hpp"
#include "terminate_benchmark.hpp"
#include "uart.hpp"

bool run_test(int test_num);


bool validate_output(int32_t* output, int32_t* reference, uint32_t rows, uint32_t cols);

void report_diff(int32_t* output, int32_t* reference, uint32_t rows, uint32_t cols);

#endif
