#include "matmul.hpp"
#include "riscv_vector.h"

void matmul(
  int8_t * pSrcA,
  int8_t * pSrcB,
  int32_t * pDst,
  uint32_t numColsA,
  uint32_t numRowsA,
  uint32_t numColsB)
  // basic

{
  // THIS IS NOT THE BEST SOLUTION ALGORITHM
  // this is just a basic idea implementation of generating one element each double loop cycle
  // in order to practice with reduce_sum/strided accesses
  uint32_t numRowsB = numColsA;

  int8_t * currentElemA = pSrcA;
  int8_t * currentElemB = pSrcB; 
  int32_t * currentElemDest = pDst;
  uint32_t remCols = numColsA;
  size_t vl = __riscv_vsetvl_e8m1(remCols);
  vint32m1_t init = __riscv_vmv_v_x_i32m1(0, vl);

  for (uint32_t i = 0; i < numRowsA; i++){
    remCols = numColsA;
    currentElemA = pSrcA + i * numColsA;
    currentElemB = pSrcB;
    currentElemDest = pDst + i * numColsB;
    while(remCols > 0U){
      vl = __riscv_vsetvl_e8m1(remCols);
      vint8m1_t row_a = __riscv_vle8_v_i8m1(currentElemA, vl);
      // strided load, calculate difference
      for(uint32_t j = 0; j < numColsB; j++){
            vint8m1_t row_b = __riscv_vlse8_v_i8m1(currentElemB, numColsB,vl);
            vint16m2_t mul = __riscv_vwmul_vv_i16m2(row_a, row_b, vl);
            vint32m1_t res = __riscv_vwredsum_vs_i16m2_i32m1(mul, init, vl);

            *currentElemDest += __riscv_vmv_x_s_i32m1_i32(res);
            currentElemDest++;
            currentElemB++;
      }
      remCols -= vl;
      currentElemA += vl;
      currentElemB = pSrcB + vl*numColsB;
      currentElemDest -= numColsB;
    }
  }

 
  return;
}

// better algo: reuse strided load, and re load A matrices again and again