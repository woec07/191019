/*
*  Header file containing test data.
*/ 
#ifndef TEST_DATA_HPP
#define TEST_DATA_HPP

#include <cstdint>

/*
* struct containing metadata for the test.
*/ 
struct test_metadata{             // Structure declaration 
  uint32_t input_len;
  uint32_t filter_len;
  uint32_t filter_channels;
};       // Structure variable

#define NUM_TESTS 1 //Due to space concerns, each test is compiled separately

extern const void* input_array[];
extern const void* filter_array[];
extern const void* ref_output_array[];
extern const void* meta_array[];

#endif
