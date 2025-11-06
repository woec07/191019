#include "add.hpp"
#include "riscv_vector.h"
void add(
  const int8_t * pSrcA,
  const int8_t * pSrcB,
        int16_t * pDst,
        uint32_t vecLen)
{
                             /* Loop counter */
  uint32_t len = vecLen;

  while (len > 0U)
  {
    /* C = A + B */
    // vl = min(len, VLEN)
    // vl defines the amount of 8bit elements (so one integer) to process at each loop cycle
    // so we try to do all operations at once (to acquire len), but if VLENMAX < len, then VLENMAX is chosen
    size_t vl = __riscv_vsetvl_e8m4(len);

    // so for VLEN = 256, LMUL = 1, SEW 8 -> this load operation loads the first 256*1/8 = VLENMAX = 32 elements into vec1 
    vint8m4_t vec1 = __riscv_vle8_v_i8m4(pSrcA, vl);
    vint8m4_t vec2 = __riscv_vle8_v_i8m4(pSrcB, vl);

    // so this __riscv_vwadd_vv_i16m1 takes two i8m1 inputs and sign extends it to 16 bit integers, as we have to take care of overflow when adding two 8bit integers
    // then it also adds the two inputs (it does it again for 32 elements at once)
    // also lmul has to be doubled as we need double space for one element
    vint16m8_t res_vec = __riscv_vwadd_vv_i16m8(vec1, vec2, vl);
    __riscv_vse16_v_i16m8(pDst, res_vec, vl); 

    pSrcA += vl;
    pSrcB += vl;
    pDst += vl;
    len -= vl;
    
  }
  return;
}
