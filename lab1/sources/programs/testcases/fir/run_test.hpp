#ifndef RUN_TEST_HPP
#define RUN_TEST_HPP 

#include <cstdint>
#include "vicuna_crt.hpp"
#include "terminate_benchmark.hpp"
#include "uart.hpp"

bool run_test(int test_num);

bool validate_output(int32_t* output, int32_t* reference, uint32_t veclen);

void report_diff(int32_t* output, int32_t* reference, uint32_t output_len, uint32_t num_channels);

void report_metadata(uint32_t output_len, uint32_t filter_len, uint32_t num_channels);
#endif
