#include "matmul.hpp"
#include "riscv_vector.h"
#include <cstring>

void matmul(
  int8_t * pSrcA,
  int8_t * pSrcB,
  int32_t * pDst,
  uint32_t numColsA,
  uint32_t numRowsA,
  uint32_t numColsB)
  // basic

{
  // Targets ACHIEVED
  // LUT: 14180
  // MATMUL_4: 74,42% Cycle Improvement
  // MATMUL_8: 72,23%
  /* USE THIS vector_config.cmake
  # VREG_W = VLEN
  set(VREG_W 128)
  set(VMEM_W 32)
  set(VPROC_PIPELINES "${VMEM_W}:VLSU 32:VMUL 32:VALU,VSLD,VDIV,VELEM") # sequence is important
  */


  // THIS IS NOT THE BEST SOLUTION ALGORITHM (best is matmul_v1.cpp), but it fullfills the targets!
  // this is just a basic idea implementation of generating one element each double loop cycle using reduce and strided accesses
  // in order to practice with reduce_sum/strided accesses
  uint32_t numRowsB = numColsA;
  int8_t * currentElemA = pSrcA;
  int8_t * currentElemB = pSrcB; 
  int32_t * currentElemDest = pDst;
  uint32_t remRows = numRowsB;

  for (uint32_t i = 0; i < numColsB; i++)
  {
    remRows = numRowsB;
    currentElemB = pSrcB + i;
    currentElemA = pSrcA;
    currentElemDest = pDst + i;
    while(remRows > 0U)
    {
      size_t vl = __riscv_vsetvl_e8m2(remRows);
      vint8m2_t rowB = __riscv_vlse8_v_i8m2(currentElemB, numColsB, vl); // 2 reg, VLSU
      uint32_t j = 0;
      while (4*j < numRowsA)
      {
        // max reg: 17 regs
        vint8m2_t rowA1 = __riscv_vle8_v_i8m2(currentElemA, vl); // 2 physical regs, VLSU
        currentElemA += numColsA;
        vint8m2_t rowA2 = __riscv_vle8_v_i8m2(currentElemA, vl); // 2 physical regs, VLSU
        currentElemA += numColsA;
        vint8m2_t rowA3 = __riscv_vle8_v_i8m2(currentElemA, vl); // 2 physical regs, VLSU
        currentElemA += numColsA;
        vint8m2_t rowA4 = __riscv_vle8_v_i8m2(currentElemA, vl); // 2 physical regs, VLSU

        vint16m4_t mul1 = __riscv_vwmul_vv_i16m4(rowA1, rowB, vl); // 4 physical regs, VMUL
        vint16m4_t mul2 = __riscv_vwmul_vv_i16m4(rowA2, rowB, vl); // 4 physical regs, VMUL
        vint16m4_t mul3 = __riscv_vwmul_vv_i16m4(rowA3, rowB, vl); // 4 physical regs, VMUL
        vint16m4_t mul4 = __riscv_vwmul_vv_i16m4(rowA4, rowB, vl); // 4 physical regs, VMUL

        vint32m1_t init = __riscv_vmv_v_x_i32m1(0, vl); // 1 physical reg, VELEM

        vint32m1_t cji1 = __riscv_vwredsum_vs_i16m4_i32m1(mul1,init,vl); // 1 physical reg, VELEM
        *currentElemDest += __riscv_vmv_x_s_i32m1_i32(cji1); // VELEM
        currentElemDest += numColsB;

        vint32m1_t cji2 = __riscv_vwredsum_vs_i16m4_i32m1(mul2,init,vl); // 1 physical reg, VELEM
        *currentElemDest += __riscv_vmv_x_s_i32m1_i32(cji2); //  VELEM
        currentElemDest += numColsB;

        vint32m1_t cji3 = __riscv_vwredsum_vs_i16m4_i32m1(mul3,init,vl); // 1 physical reg, VELEM
        *currentElemDest += __riscv_vmv_x_s_i32m1_i32(cji3); //  VELEM
        currentElemDest += numColsB;

        vint32m1_t cji4 = __riscv_vwredsum_vs_i16m4_i32m1(mul4,init,vl); // 1 physical reg, VELEM
        *currentElemDest += __riscv_vmv_x_s_i32m1_i32(cji4); //  VELEM

        currentElemDest += numColsB;
        currentElemA += numColsA;
        j++;
      }
      j = 4 * j;
      while (j < numRowsA)
      {// max reg: 10 regs
        vint8m2_t rowA1 = __riscv_vle8_v_i8m2(currentElemA, vl); // 2 physical regs, VLSU
        vint16m4_t mul1 = __riscv_vwmul_vv_i16m4(rowA1, rowB, vl); // 4 physical regs, VMUL
        vint32m1_t init = __riscv_vmv_v_x_i32m1(0, vl); // 1 physical reg, VELEM
        vint32m1_t cji1 = __riscv_vwredsum_vs_i16m4_i32m1(mul1,init,vl); // 1 physical reg, VELEM
        *currentElemDest += __riscv_vmv_x_s_i32m1_i32(cji1); // VELEM
        
        currentElemDest += numColsB;
        currentElemA += numColsA;
        j++;
      }
      remRows -= vl;
      currentElemB += (vl * numColsB);
      currentElemA = currentElemA - (numRowsA * numColsA) + vl;
      currentElemDest = pDst + i;
    }
  }
  return;
}

// better algo: reuse strided load, and re load A matrices again and again

/*
  for (uint32_t i = 0; i < numRowsA; i++){
    remCols = numColsA;
    size_t vl = __riscv_vsetvl_e8m1(remCols);

    currentElemA = pSrcA + i * numColsA;
    currentElemB = pSrcB;
    currentElemDest = pDst + i * numColsB;
    while(remCols > 0U){
      vl = __riscv_vsetvl_e8m1(remCols);
      vint8m1_t row_a = __riscv_vle8_v_i8m1(currentElemA, vl);
      vint32m1_t init = __riscv_vmv_v_x_i32m1(0, vl);
      // strided load, calculate difference
      for(uint32_t j = 0; j < numColsB; j++){
            vint8m1_t row_b = __riscv_vlse8_v_i8m1(currentElemB, numColsB,vl);
            vint16m2_t mul = __riscv_vwmul_vv_i16m2(row_a, row_b, vl);
            vint32m1_t res = __riscv_vwredsum_vs_i16m2_i32m1(mul, init , vl);

            *currentElemDest += __riscv_vmv_x_s_i32m1_i32(res);
            currentElemDest++;
            currentElemB++;
      }
      remCols -= vl;
      currentElemA += vl;
      currentElemB += (vl-1)*numColsB;
      currentElemDest -= numColsB;
    }
  }
*/