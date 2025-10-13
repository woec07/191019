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

    /* Add and store result in destination buffer. */
    *pDst++ = (int16_t)*pSrcA++ + (int16_t)*pSrcB++;

    /* Decrement loop counter */
    len--;
  }
  return;
}
