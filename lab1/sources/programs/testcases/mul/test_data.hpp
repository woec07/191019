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
  uint32_t vec_len;
};       // Structure variable

#define NUM_TESTS 1 //Due to space concerns, each test is compiled separately

extern const void* vec_a_array[];
extern const void* vec_b_array[];
extern const void* vec_c_array[];
extern const void* meta_array[];

#endif
