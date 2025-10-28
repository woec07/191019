#include "mul.hpp"
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
    /* C = A * B */

    /* Add and store result in destination buffer. */
    *pDst++ = (int32_t)*pSrcA++ * (int32_t)*pSrcB++;

    /* Decrement loop counter */
    len--;
  }
  return;
}
