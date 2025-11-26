#ifndef MATMUL_HPP
#define MATMUL_HPP
#include <cstdint>

#include "vicuna_crt.hpp"
#include "terminate_benchmark.hpp"
#include "uart.hpp"

void mul( const int16_t * pSrcA, const int16_t * pSrcB, int32_t * pDst, uint32_t vecLen);

#endif
