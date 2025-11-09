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
  // not working yet
  // TODO: how to integrate vl into the algorithm??????????
  // stopped @: how to implement third loop, especially remaining chunk case

  uint32_t numRowsB = numColsA;
  uint32_t j = 0;
  // take row after row from A
  while (j < numRowsA){
    uint32_t k = 0;
    size_t rem_colsB = numColsB;
    while (rem_colsB > 0U) {
      size_t vl = __riscv_vsetvl_e8m1(numColsB);
      uint32_t i = 0;
      vint32m4_t rowRes, helper;
      while (i < numRowsB){
        // takes the i-th row of B (1,2,3, ... numRowsB)
        vint8m1_t rowB_i = __riscv_vle8_v_i8m1(pSrcB, vl);
        // takes the i_th element of row j of A
        int8_t a_i =  *pSrcA;
        vint16m2_t mulScal = __riscv_vwmul_vx_i16m2(rowB_i, a_i, vl);
        vint32m4_t mulScal_ex = __riscv_vsext_vf2_i32m4(mulScal, vl);
        vl = __riscv_vsetvl_e32m4(vl);
        if (i == 0) {
          helper = mulScal_ex;
        } 
        else{
          rowRes = __riscv_vadd_vv_i32m4(helper, mulScal_ex, vl);
          helper = rowRes;
        }
        pSrcA++;
        pSrcB += numColsB;
        i++;
      }
      // store first result row to destination
    __riscv_vse32_v_i32m4(pDst, rowRes, vl);
      pDst += vl;
      rem_colsB -= vl;
    }
    j++;
  }
  }
  /* Return to application */
  return;
}