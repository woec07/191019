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
  uint32_t numRowsA; 
  uint32_t numColsA;  
  uint32_t numColsB;
};       // Structure variable

#define NUM_TESTS 1 //Due to space concerns, each test is compiled separately

extern const void* mat_a_array[];
extern const void* mat_b_array[];
extern const void* mat_c_array[];
extern const void* meta_array[];

#endif
