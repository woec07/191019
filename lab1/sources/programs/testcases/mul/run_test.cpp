#include "mul.hpp"
#include "run_test.hpp"
#include "test_data.hpp"
#include "uart.hpp"

bool run_test(int test_num)
{
    //Setup inputs and outputs
  

    test_metadata meta = *(test_metadata*)meta_array[test_num];
    uint32_t vecLen = meta.vec_len;

    int32_t *output = (int32_t*)vicuna_malloc(2 * vecLen);

    int16_t* vec_a = (int16_t*)vec_a_array[test_num];
    int16_t* vec_b = (int16_t*)vec_b_array[test_num];

    
    //Begin counting performance
    uint32_t start_cycles, start_instr;
    uint32_t end_cycles, end_instr;
    asm volatile ("csrr %0,mcycle"   : "=r" (start_cycles)  );
    asm volatile ("csrr %0,minstret"   : "=r" (start_instr)  );

    //Run matmul test
    mul(vec_a, vec_b, output, vecLen);

    //stop counting performance
    asm volatile ("csrr %0,mcycle"   : "=r" (end_cycles)  );
    asm volatile ("csrr %0,minstret"   : "=r" (end_instr)  );

    //Verfiy results
    int32_t* vec_c = (int32_t*)vec_c_array[test_num];
    bool valid_out = validate_output(output, vec_c, vecLen);

    report_metadata(vecLen);
    //Report results
    if (valid_out)
    {
        printf("PASS:\n");
    }
    else
    {
        printf("ERROR: OUTPUT MISMATCH\n");
        report_diff(output, vec_c, vecLen);
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

void report_diff(int32_t* output, int32_t* reference, uint32_t vecLen)
{
    printf("Your Result:\n\n");
    for(uint32_t i=0; i<vecLen; i++)
    {
        printf("%d ",output[i]);
    }
    printf("\n");
    printf("Reference Result:\n\n");
    for(uint32_t i=0; i<vecLen; i++)
    {
            printf("%d ",reference[i]);
    }
    printf("\n");
}

void report_metadata(uint32_t vecLen)
{
    printf("Testcase:\n\n");
    printf("Vector Multiplication - Vectors %d elements long:\n\n", vecLen);

}
