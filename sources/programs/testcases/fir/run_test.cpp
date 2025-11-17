#include "fir.hpp"
#include "run_test.hpp"
#include "test_data.hpp"
#include "uart.hpp"

bool run_test(int test_num)
{
    //Setup inputs and outputs
  

    test_metadata meta = *(test_metadata*)meta_array[test_num];
    uint32_t input_len = meta.input_len;
    uint32_t filter_len = meta.filter_len;
    uint32_t filter_channels = meta.filter_channels;

    int32_t *output = (int32_t*)vicuna_malloc((input_len - (filter_len - 1))*4*filter_channels);

    int16_t* input = (int16_t*)input_array[test_num];
    int16_t* filter = (int16_t*)filter_array[test_num];

    
    //Begin counting performance
    uint32_t start_cycles, start_instr;
    uint32_t end_cycles, end_instr;
    asm volatile ("csrr %0,mcycle"   : "=r" (start_cycles)  );
    asm volatile ("csrr %0,minstret"   : "=r" (start_instr)  );

    //Run fir test
    fir(input, filter, output, input_len, filter_len, filter_channels);

    //stop counting performance
    asm volatile ("csrr %0,mcycle"   : "=r" (end_cycles)  );
    asm volatile ("csrr %0,minstret"   : "=r" (end_instr)  );

    //Verfiy results
    int32_t* ref = (int32_t*)ref_output_array[test_num];
    bool valid_out = validate_output(output, ref, (input_len - (filter_len - 1))*filter_channels);

    report_metadata(input_len, filter_len, filter_channels);
    //Report results
    if (valid_out)
    {
        printf("PASS:\n");
    }
    else
    {
        printf("ERROR: OUTPUT MISMATCH\n");
        report_diff(output, ref, (input_len - (filter_len - 1)), filter_channels);
    }

    printf("Total Cycles:       %d\n",end_cycles-start_cycles);
    printf("Total Instructions: %d\n\n",end_instr-start_instr);

    //Cleanup
    vicuna_free(output);
    return valid_out;
}


bool validate_output(int32_t* output, int32_t* reference, uint32_t vecLen)
{
    for(uint32_t i=0; i<vecLen; i++)
    {
            if (output[i] != reference[i])
            {
                return false;
            }
    }

    return true;
}

void report_diff(int32_t* output, int32_t* reference, uint32_t output_len, uint32_t num_channels)
{
    printf("Your Result:\n\n");
    for(uint32_t j=0; j<num_channels; j++)
    {
        for(uint32_t i=0; i<output_len; i++)
        {
            printf("%d ",output[j*output_len + i]);
        }
        printf("\n");
    }
    printf("\n");
    printf("Reference Result:\n\n");
    for(uint32_t j=0; j<num_channels; j++)
    {
        for(uint32_t i=0; i<output_len; i++)
        {
                printf("%d ",reference[j*output_len + i]);
        }
        printf("\n");
    }
    printf("\n");
}

void report_metadata(uint32_t output_len, uint32_t filter_len, uint32_t num_channels)
{
    printf("Testcase:\n\n");
    printf("Multi-Channel FIR Filter\n");
    printf("Input %d elements long\n", output_len);
    printf("Filter %d elements long\n", filter_len);
    printf("%d Channels\n\n", num_channels);

}
