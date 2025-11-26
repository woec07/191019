#include "fir.hpp"
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

  /* Channel Loop Counter */
  uint32_t ch = 0;
  while (ch < filter_channels)
  {
    /* Set input, filter and output pointers*/
    int32_t * out_ptr = output + ch*output_len;
    int16_t * in_ptr = input;
    int16_t * in_ptr_base = input;

    /* Output Loop Counter */
    uint32_t out = 0;
    while (out < output_len)
    {
      /* Result Accumulator */
      int32_t acc = 0;
      /* Set in_ptr to start of next iteration of convolution and filter_ptr to start of filter */
      in_ptr = in_ptr_base;
      int16_t * filter_ptr = filter + ch*filter_len;
      /* Filter Loop Counter */
      uint32_t fil = 0;
      while (fil < filter_len)
      {
        /* Multiply Element of the filter with the corresponding element of the input */
        acc += (int32_t)*(in_ptr++) * (int32_t)*(filter_ptr++);

        /* Increment filter loop counter*/
        fil++;
      }
      /* Store result and advance pointers */
      *out_ptr++ = acc;
      in_ptr_base++;
      /* Increment output loop counter*/
      out++;
    }
    /* Increment channel loop counter */
    ch++;
  }
  return;
}
