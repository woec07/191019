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
  
  volatile int8_t * currentElemA = pSrcA;
  int8_t * currentElemB = pSrcB;
  
  size_t vl_max = __riscv_vsetvlmax_e8m1();
  uint32_t j = 0;
  // take row after row from A
  while (j < numRowsA){
    size_t rem_colsB = numColsB;
    while (rem_colsB >= vl_max) {
      size_t vl = __riscv_vsetvl_e8m1(numColsB);
      currentElemA = pSrcA + j * numColsA;
      currentElemB = pSrcB + numColsB - rem_colsB;
      uint32_t i = 0;
      vint32m4_t helper;
      vint32m4_t result;
      while (i < numRowsB){
        vl = __riscv_vsetvl_e8m1(numColsB);
        // takes the i-th row of B (1,2,3, ... numRowsB)
        vint8m1_t rowB_i = __riscv_vle8_v_i8m1(currentElemB, vl);
        // takes the i_th element of row j of A
      
        int8_t a_i =  *(currentElemA);


        vint16m2_t mulScal = __riscv_vwmul_vx_i16m2(rowB_i, a_i, vl);
        vl = __riscv_vsetvl_e32m4(vl);
        vint32m4_t mulScal_sex = __riscv_vsext_vf2_i32m4(mulScal, vl);

        if (i == 0) {
          helper = mulScal_sex;
          result = mulScal_sex;
        } 
        else{
          result = __riscv_vadd_vv_i32m4(mulScal_sex, helper, vl);
          helper = result;
        }
        currentElemA++;
        currentElemB += numColsB;
        i++;
      }
      // store first result row to destination
      __riscv_vse32_v_i32m4(pDst, result, vl);
      pDst += vl;
      rem_colsB -= vl;
    }
    if (rem_colsB > 0U){
      currentElemA = pSrcA + j * numColsA;
      currentElemB = pSrcB + numColsB - rem_colsB;
      uint32_t i = 0;
      vint32m4_t helper;
      vint32m4_t result;
      while (i < numRowsB){
        // takes the i-th row of B (1,2,3, ... numRowsB)
        vint8m1_t rowB_i = __riscv_vle8_v_i8m1(currentElemB, rem_colsB);
        // takes the i_th element of row j of A
        
        // POTENTIAL BUG???? - scalar LOAD
        int8_t a_i =  *(currentElemA);

        vint16m2_t mulScal = __riscv_vwmul_vx_i16m2(rowB_i, a_i, rem_colsB);
        vint32m4_t mulScal_sex = __riscv_vsext_vf2_i32m4(mulScal, rem_colsB);
        if (i == 0) {
          helper = mulScal_sex;
          result = mulScal_sex;
        } 
        else{
          result = __riscv_vadd_vv_i32m4(mulScal_sex, helper, rem_colsB);
          helper = result;
        }
        currentElemA++;
        currentElemB += numColsB;
        i++;
      }
      // store first result row to destination
    __riscv_vse32_v_i32m4(pDst, result, rem_colsB);
    pDst += rem_colsB;
    }
    j++;
  }
  
  /* Return to application */
  return;
}