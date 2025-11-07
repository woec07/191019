#include "mul.hpp"
#include "riscv_vector.h"
void mul(
  const int16_t * pSrcA,
  const int16_t * pSrcB,
        int32_t * pDst,
        uint32_t vecLen)
{
                             /* Loop counter */
  uint32_t len = vecLen;

  while (len > 0U)
  {
    // [Cycles, Inst] done for REG_W = 128
    
    // Optimization 1 -- BASIC VECTORCODE(done in 3.1): write Vectorized Code => TC4: [34.28%, 83.06%] improvement
    
    // Optimization 2 -- LMUL MAXIMIZATION (done in 3.1): optimizing VLMAX -> set LMUL=4 (MAX value, as sign extension requires LMUL 8) instead of 1, as this works as 4+4+8 < 32 
    // => TC4: [74.01%, 95.52%] improvement
    
    // Optimization 3 -- PIPELINE WIDTH + SEPARATION(done in 3.2): changing vector_config.cmake
    // 3a: separate VALU and VMUL to separate pipelines => does not change anything, as we use vwmul which is better then manually sign extending
    // and then multiplyiing (as signextending is 2 writing instructions more to the reg file which is our bottleneck) => see option B
    // 3b: use wider pipelines within the pipelines (64 instead of 32)  => TC4: [81.03%, 95.52%] improvement
    
    // Optimization 4 (done in 3.3): mul4.cpp!

    //try to process the maximum amount of vectors per cycle
    size_t vl = __riscv_vsetvl_e16m4(len);

    // load vl elements in vec1 and vec2
    vint16m4_t vec1 = __riscv_vle16_v_i16m4(pSrcA, vl);  // takes 4 physical regs
    vint16m4_t vec2 = __riscv_vle16_v_i16m4(pSrcB, vl);  // takes 4 physical regs

    // option A, no split of pipelines BUT only one write to the vec reg file
    vint32m8_t mul_res = __riscv_vwmul_vv_i32m8(vec1, vec2, vl);

    /* option B
    vint32m8_t vec1s = __riscv_vsext_vf2_i32m8(vec1, vl); // takes 8 physical regs
    vint32m8_t vec2s = __riscv_vsext_vf2_i32m8(vec2, vl); // takes 8 physical
    vl = __riscv_vsetvl_e32m8(vl);
    vint32m8_t mul_res = __riscv_vmul_vv_i32m8(vec1s, vec2s, vl); // 8 physical regs
    */

    __riscv_vse32_v_i32m8(pDst, mul_res,vl);

    pSrcA += vl;
    pSrcB += vl;
    pDst += vl;
    len -= vl;

  }
  return;
}
