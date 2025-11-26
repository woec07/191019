#include "fir.hpp"
#include "riscv_vector.h"
void fir(
        int16_t * input,
        int16_t * filter,
        int32_t * output,
        uint32_t  input_len,
        uint32_t  filter_len,
        uint32_t  filter_channels)
{
  // Reference Solution Copy
  /* Calculate the length of the output per channel based on inputs */
  uint32_t output_len = input_len - (filter_len - 1);

  int16_t * currInput = input;
  volatile int16_t * currFilter = filter;
  int32_t * currOutput = output;

  uint32_t i = 0;
  while (i < filter_channels){
    currInput = input;
    currOutput = output + i * output_len; // i think i got this wrong in the exam (we need to use output_len to overwrite the incorrectly written!!!!)
    currFilter = filter + i * filter_len;
    uint32_t remInput = input_len;
    while (remInput > 0U)
      {
        size_t vl = __riscv_vsetvl_e32m8(remInput);
        vint32m8_t result = __riscv_vmv_v_x_i32m8(0,vl);
        for (int j = 0; j < filter_len; j++){
          vl = __riscv_vsetvl_e16m4(remInput);
          vint16m4_t rowA = __riscv_vle16_v_i16m4(currInput, vl);
          int16_t filElm = *currFilter;
          result = __riscv_vwmacc_vx_i32m8(result,filElm,rowA, vl);
          currInput++;
          currFilter++;
        }
        // during exam i tried with these pointers, so i got there some errors in the final version
        __riscv_vse32_v_i32m8(currOutput, result, vl); // crucial: this vl is okay (but in the first i thought this, but i kept searching for the error so it got wild)
        currFilter = filter + i * filter_len; // obvious
        currOutput += vl; //obvious
        currInput = currInput - filter_len + vl; // this is a bit tricky, but just basically increment currInput by vl in each iteration (compared to the beginning)
        remInput -= vl; // obvious
      }
    i++;
  }
  return;
}
