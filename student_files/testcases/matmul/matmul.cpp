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
  // version with optimization 1-4, VREG_W =128
  // Optimization 1: Basic Vector Code -> DONE
  // Optimization 2: maximizin LMULs (usage of 44 physical regs)
  // Optimization 3: changing pipeline strucutre and widening 
  // -> DONE TC_4 = [85.36, 94.53]% improvement, LUT_4 = | TC_8: [42.03, 78.342]%, LUT_8 = 
  ////////////////////////////////////////////////////////////////////////////////////////
  // Optimization 4: loop unrolling -> DONE TC_4 = [85.30, 94.50]% improvement, LUT_4 = 192XX | TC_8: [41.79, 78.20]%, LUT_8 = 
  // just for simplicity
  // note: loop unrolling until now with not the expected performance gain
  uint32_t numRowsB = numColsA;
  
  // create two pointers to the arrays
  volatile int8_t * currentElemA = pSrcA;
  int8_t * currentElemB = pSrcB;
  
  uint32_t j = 0;
  // take row after row from A
  while (j < numRowsA){
    size_t rem_colsB = numColsB;
    size_t vl_max = __riscv_vsetvlmax_e8m2();

    while(rem_colsB >= 2 * vl_max){
       // calculate pointers to start from
      currentElemA = pSrcA + j * numColsA;
      currentElemB = pSrcB + numColsB - rem_colsB;

      uint32_t i = 0;

      // initialize result vector with only
      vint32m8_t result1 = __riscv_vmv_v_x_i32m8(0, vl_max); //ALU,  8 physical regs 
      vint32m8_t result2 = __riscv_vmv_v_x_i32m8(0, vl_max); //ALU,  8 physical regs 
      while (i < numRowsB){
        // takes the i-th row of B (1,2,3, ... numRowsB)
        vint8m2_t rowB_i1 = __riscv_vle8_v_i8m2(currentElemB, vl_max); //VLSU, 2 physical regs
        vint8m2_t rowB_i2 = __riscv_vle8_v_i8m2(currentElemB+vl_max, vl_max); //VLSU, 2 physical regs
        int8_t a_i =  *(currentElemA);
                // takes the i_th element of row j of A      

        vint16m4_t mulScal1 = __riscv_vwmul_vx_i16m4(rowB_i1, a_i, vl_max); //VMUL, 4 physical regs
        vint16m4_t mulScal2 = __riscv_vwmul_vx_i16m4(rowB_i2, a_i, vl_max); //VMUL, 4 physical regs

        vint32m8_t mulScal_sex1 = __riscv_vsext_vf2_i32m8(mulScal1, vl_max); //ALU 8 physical regs
        vint32m8_t mulScal_sex2 = __riscv_vsext_vf2_i32m8(mulScal2, vl_max); //ALU 8 physical regs
        
        result1 = __riscv_vadd_vv_i32m8(mulScal_sex1, result1, vl_max); // ALU
        result2 = __riscv_vadd_vv_i32m8(mulScal_sex2, result2, vl_max); // ALU

        currentElemA++;
        currentElemB += numColsB;
        i++;
      }
      // store first result row to destination
      __riscv_vse32_v_i32m8(pDst, result1, vl_max);
      __riscv_vse32_v_i32m8(pDst+vl_max, result2, vl_max);

      pDst = pDst + 2*vl_max;
      rem_colsB = rem_colsB - 2* vl_max;
    }
    while (rem_colsB > 0U) {
      size_t vl = __riscv_vsetvl_e32m8(rem_colsB);

      // calculate pointers to start from
      currentElemA = pSrcA + j * numColsA;
      currentElemB = pSrcB + numColsB - rem_colsB;

      uint32_t i = 0;

      // initialize result vector with only
      vint32m8_t result = __riscv_vmv_v_x_i32m8(0, vl); //ALU,  8 physical regs 
      while (i < numRowsB){
        vl = __riscv_vsetvl_e8m2(rem_colsB);
        // takes the i-th row of B (1,2,3, ... numRowsB)
        vint8m2_t rowB_i = __riscv_vle8_v_i8m2(currentElemB, vl); //VLSU, 2 physical regs
        // takes the i_th element of row j of A      
        int8_t a_i =  *(currentElemA);

        vint16m4_t mulScal = __riscv_vwmul_vx_i16m4(rowB_i, a_i, vl); //VMUL, 4 physical regs
        vl = __riscv_vsetvl_e32m8(vl);
        vint32m8_t mulScal_sex = __riscv_vsext_vf2_i32m8(mulScal, vl); //ALU 8 physical regs
        result = __riscv_vadd_vv_i32m8(mulScal_sex, result, vl); // ALU

        currentElemA++;
        currentElemB += numColsB;
        i++;
      }
      // store first result row to destination
      __riscv_vse32_v_i32m8(pDst, result, vl);
      pDst += vl;
      rem_colsB -= vl;
    }
    j++;
  }
  /* Return to application */
  return;
}