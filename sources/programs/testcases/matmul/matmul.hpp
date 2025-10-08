#ifndef MATMUL_HPP
#define MATMUL_HPP
#include <cstdint>

#include "vicuna_crt.hpp"
#include "terminate_benchmark.hpp"
#include "uart.hpp"

void matmul(int8_t * pSrcA, int8_t * pSrcB, int32_t * pDst, uint32_t numColsA, uint32_t numRowsA, uint32_t numColsB);

#endif
