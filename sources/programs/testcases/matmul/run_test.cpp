#include "matmul.hpp"
#include "run_test.hpp"
#include "test_data.hpp"
#include "uart.hpp"

bool run_test(int test_num)
{
    //Setup inputs and outputs
  

    test_metadata meta = *(test_metadata*)meta_array[test_num];
    uint32_t numRowsA = meta.numRowsA;
    uint32_t numColsA = meta.numColsA;
    uint32_t numColsB = meta.numColsB;

    int32_t *output = (int32_t*)vicuna_malloc(4 * numRowsA * numColsB);

    int8_t* mat_a = (int8_t*)mat_a_array[test_num];
    int8_t* mat_b = (int8_t*)mat_b_array[test_num];

    
    //Begin counting performance
    uint32_t start_cycles, start_instr;
    uint32_t end_cycles, end_instr;
    asm volatile ("csrr %0,mcycle"   : "=r" (start_cycles)  );
    asm volatile ("csrr %0,minstret"   : "=r" (start_instr)  );

    //Run matmul test
    matmul(mat_a, mat_b, output, numColsA, numRowsA, numColsB);

    //stop counting performance
    asm volatile ("csrr %0,mcycle"   : "=r" (end_cycles)  );
    asm volatile ("csrr %0,minstret"   : "=r" (end_instr)  );

    //Verfiy results
    int32_t* mat_c = (int32_t*)mat_c_array[test_num];
    bool valid_out = validate_output(output, mat_c, numRowsA, numColsB);

    //Report results
    if (valid_out)
    {
        printf("PASS:\n");
    }
    else
    {
        printf("ERROR: OUTPUT MISMATCH\n");
        report_diff(output, mat_c, numRowsA, numColsB);
    }

    printf("Total Cycles:       %d\n",end_cycles-start_cycles);
    printf("Total Instructions: %d\n\n",end_instr-start_instr);

    //Cleanup
    vicuna_free(output);
    return valid_out;
}


bool validate_output(int32_t* output, int32_t* reference, uint32_t rows, uint32_t cols)
{
    for(uint32_t i=0; i<rows; i++)
    {
        for(uint32_t j=0; j<cols; j++)
        {
            if (output[i*cols + j] != reference[i*cols + j])
            {
                return false;
            }
        }
    }

    return true;
}

void report_diff(int32_t* output, int32_t* reference, uint32_t rows, uint32_t cols)
{
    printf("Your Result:\n\n");
    for(uint32_t i=0; i<rows; i++)
    {
        for(uint32_t j=0; j<cols; j++)
        {
            printf("%d ",output[i*cols + j]);
        }
        printf("\n");
    }
    printf("\n");
    printf("Reference Result:\n\n");
    for(uint32_t i=0; i<rows; i++)
    {
        for(uint32_t j=0; j<cols; j++)
        {
            printf("%d ",reference[i*cols + j]);
        }
        printf("\n");
    }
    printf("\n");
}
