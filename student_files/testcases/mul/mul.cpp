#include "mul.hpp"
#include "riscv_vector.h"

// mul.cpp with optimization technique 4, VREG_W = 128 - LOOP UNROLLING => TC4: [85.57%, 96.8%] improvement
// LUT Usage: 17XXX

void mul(
  const int16_t * pSrcA,
  const int16_t * pSrcB,
        int32_t * pDst,
        uint32_t vecLen)
{
                             /* Loop counter */
  uint32_t len = vecLen;
  size_t vl_max = __riscv_vsetvlmax_e16m4();
  while (len >= 2* vl_max)
  { // [Cycles, Inst] done for REG_W = 128
    // Optimization 4 -- LOOP UNROLLING (done in 3.3): loop unrolling
    // WITH LMUL = 4/8 -> MAX 32 registers in usage
    const int16_t * pSrcA2 = pSrcA + vl_max;
    const int16_t * pSrcB2 = pSrcB + vl_max;
    int32_t * pDst2 = pDst + vl_max;


    // load vl elements in vec1 and vec2
    vint16m4_t vec1 = __riscv_vle16_v_i16m4(pSrcA, vl_max);  // takes 4 physical regs
    vint16m4_t vec2 = __riscv_vle16_v_i16m4(pSrcB, vl_max);  // takes 4 physical regs

    // option A, no split of pipelines BUT only one write to the vec reg file
    vint32m8_t mul_res = __riscv_vwmul_vv_i32m8(vec1, vec2, vl_max); // 8 physical reg

    vint16m4_t vec1_2 = __riscv_vle16_v_i16m4(pSrcA2, vl_max);  // takes 4 physical regs
    vint16m4_t vec2_2 = __riscv_vle16_v_i16m4(pSrcB2, vl_max);  // takes 4 physical regs
    vint32m8_t mul_res2 = __riscv_vwmul_vv_i32m8(vec1_2, vec2_2, vl_max); // 8 regs talem

    __riscv_vse32_v_i32m8(pDst, mul_res,vl_max);
    __riscv_vse32_v_i32m8(pDst2, mul_res2,vl_max);

    pSrcA += 2*vl_max;
    pSrcB += 2*vl_max;
    pDst += 2*vl_max;
    len -= 2*vl_max;

  }

  while (len > 0U){
    size_t vl = __riscv_vsetvl_e16m4(len);

    vint16m4_t vec1 = __riscv_vle16_v_i16m4(pSrcA, vl);
    vint16m4_t vec2 = __riscv_vle16_v_i16m4(pSrcB, vl);  

    vint32m8_t mul_res = __riscv_vwmul_vv_i32m8(vec1, vec2, vl);

    __riscv_vse32_v_i32m8(pDst, mul_res,vl);

    pSrcA += vl;
    pSrcB += vl;
    pDst += vl;
    len -= vl;
  }

  return;
}
