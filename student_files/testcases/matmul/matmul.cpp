#include "matmul.hpp"

void matmul(
  int8_t * pSrcA,
  int8_t * pSrcB,
  int32_t * pDst,
  uint32_t numColsA,
  uint32_t numRowsA,
  uint32_t numColsB)
  
{
        int32_t sum;                                     /* Accumulator */
        int8_t *pIn1 = pSrcA;                    /* Input data matrix pointer A */
        int8_t *pIn2 = pSrcB;                    /* Input data matrix pointer B */
        int8_t *pInA = pSrcA;                    /* Input data matrix pointer A of Q15 type */
        int8_t *pInB = pSrcB;                    /* Input data matrix pointer B of Q15 type */
        int32_t *pOut = pDst;                     /* Output data matrix pointer */
        int32_t *px;                                     /* Temporary output data matrix pointer */
        uint32_t col, i = 0U, row = numRowsA, colCnt;  /* Loop counters */

  {
    /* The following loop performs the dot-product of each row in pSrcA with each column in pSrcB */
    /* row loop */
    do
    {
      /* Output pointer is set to starting address of the row being processed */
      px = pOut + i;

      /* For every row wise process, column loop counter is to be initiated */
      col = numColsB;

      /* For every row wise process, pIn2 pointer is set to starting address of pSrcB data */
      pIn2 = pSrcB;

      /* column loop */
      do
      {
        /* Set the variable sum, that acts as accumulator, to zero */
        sum = 0;

        /* Initiate pointer pIn1 to point to starting address of pSrcA */
        pIn1 = pInA;

        /* Matrix A columns number of MAC operations are to be performed */
        colCnt = numColsA;

        /* matrix multiplication */
        while (colCnt > 0U)
        {
          /* c(m,n) = a(1,1) * b(1,1) + a(1,2) * b(2,1) + .... + a(m,p) * b(p,n) */

          /* Perform multiply-accumulates */
          sum += (int32_t) * pIn1++ * *pIn2;
          pIn2 += numColsB;

          /* Decrement loop counter */
          colCnt--;
        }
        /* Store result in destination buffer */
        *px++ = sum;

        /* Decrement column loop counter */
        col--;

        /* Update pointer pIn2 to point to starting address of next column */
        pIn2 = pInB + (numColsB - col);

      } while (col > 0U);

      /* Update pointer pSrcA to point to starting address of next row */
      i = i + numColsB;
      pInA = pInA + numColsA;

      /* Decrement row loop counter */
      row--;

    } while (row > 0U);
  }

  /* Return to application */
  return;
}

