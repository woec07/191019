#include "matmul.hpp"
#include "riscv_vector.h"

void matmul(
  int8_t * pSrcA,
  int8_t * pSrcB,
  int32_t * pDst,
  uint32_t numColsA,
  uint32_t numRowsA,
  uint32_t numColsB)
  
{
// TARGETS ACHIEVED - CURRENT VERSION
// Optimization 4 - Loop Unrolling done: => TC_4 = [70.08, 84.52]% improvement| TC_8: [70.56, 84.53]%, LUT = 14132 -> TARGETS ACHIEVED
  uint32_t numRowsB = numColsA;
  
  // create two pointers to the arrays
  volatile int8_t * currentElemA = pSrcA;
  int8_t * currentElemB = pSrcB;
  
  uint32_t j = 0;
  // take row after row from A
  while (j < numRowsA){
    size_t rem_colsB = numColsB;
    while (rem_colsB > 0U) {
      size_t vl = __riscv_vsetvl_e32m8(rem_colsB);
      // calculate pointers to start from
      currentElemA = pSrcA + j * numColsA;
      currentElemB = pSrcB + numColsB - rem_colsB;

      uint32_t i = 0;

      // initialize result vector with only
      vint32m4_t result = __riscv_vmv_v_x_i32m4(0, vl); //ALU,  4 physical regs 
      while (2* i < numRowsB){
        // sum of used regs: 18 < 32
        vl = __riscv_vsetvl_e8m1(rem_colsB);
        vint8m1_t rowB_i1 = __riscv_vle8_v_i8m1(currentElemB, vl); // 1 physical regs (LSU)
        int32_t a_i1 = (int32_t) *(currentElemA);
        vint32m4_t rowBext_i1 = __riscv_vsext_vf4_i32m4(rowB_i1, vl); // 4 physical regs (ALU)
        vint32m4_t immres = __riscv_vmacc_vx_i32m4(result ,a_i1, rowBext_i1, vl); // 4 physical regs (MUL)
        
        vint8m1_t rowB_i2 = __riscv_vle8_v_i8m1(currentElemB+numColsB, vl); //VLSU, 1 physical regs (LSU)
        int32_t a_i2 = (int32_t) *(currentElemA+1);
        vint32m4_t rowBext_i2 = __riscv_vsext_vf4_i32m4(rowB_i2, vl); // 4 physical regs (ALU)   
        result = __riscv_vmacc_vx_i32m4(immres ,a_i2, rowBext_i2, vl); //(MUL)

        currentElemA = currentElemA + 2;
        currentElemB = currentElemB + 2* numColsB;
        i++;
      }
      i = 2*i;
      while (i < numRowsB){

        // sum of used regs: 9 < 32
        // takes the i-th row of B (1,2,3, ... numRowsB)
        vl = __riscv_vsetvl_e8m1(rem_colsB);
        vint8m1_t rowB_i1 = __riscv_vle8_v_i8m1(currentElemB, vl); //VLSU, 1 physical regs
        vint32m4_t rowBext_i1 = __riscv_vsext_vf4_i32m4(rowB_i1, vl); // 4 physical regs
        // takes the i_th element of row j of A      
        int32_t a_i1 = (int32_t) *(currentElemA);
        result = __riscv_vmacc_vx_i32m4(result, a_i1, rowBext_i1, vl);
        currentElemA++;
        currentElemB +=  numColsB;
        i++;
      }
      // store first result row to destination
      __riscv_vse32_v_i32m4(pDst, result, vl);
      pDst += vl;
      rem_colsB -= vl;
    }
    j++;
  }
  /* Return to application */
  return;
}