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
  /* THIS IS AN OLD VERSION */
  // version with optimization 1-3, no loop unrolling done VREG_W =128
  // Optimization 1: Basic Vector Code -> DONE
  // Optimization 2: maximizin LMULs (usage of 22 physical regs)
  // Optimization 3: changing pipeline strucutre and widening

  // just for simplicity
  uint32_t numRowsB = numColsA;
  
  // create two pointers to the arrays
  volatile int8_t * currentElemA = pSrcA;
  int8_t * currentElemB = pSrcB;
  
  // consider matrices where the # of columns of A  (rows of B) is less or equal to # of rows of A,
  // so we have matrices of B which do not have many rows (less iterations of inner loop)
  // narrow 
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
}

  /* Return to application */
  return;
}