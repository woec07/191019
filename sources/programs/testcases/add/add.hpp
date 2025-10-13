#ifndef MATMUL_HPP
#define MATMUL_HPP
#include <cstdint>

#include "vicuna_crt.hpp"
#include "terminate_benchmark.hpp"
#include "uart.hpp"

void add( const int8_t * pSrcA, const int8_t * pSrcB, int16_t * pDst, uint32_t vecLen);

#endif
