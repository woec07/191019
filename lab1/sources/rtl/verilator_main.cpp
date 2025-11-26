// Copyright TU Wien
// Licensed under the Solderpad Hardware License v2.1, see LICENSE.txt for details
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1


#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include "Vvproc_top.h"

#include "Vvproc_top_vproc_top.h"
#include "Vvproc_top_cv32e40x_core__pi1.h"
//#include "Vvproc_top_vproc_core__pi2.h"
//#include "Vvproc_top_vproc_decoder__pi9.h"
//#include "Vvproc_top_vproc_decoder__V800_Cb_X20_Dz3.h"
//#include "Vvproc_top_vproc_decoder__V800_Cb_X40_Dz3.h"

#include "verilator_support.h"


#include "verilated.h"


static void log_cycle(Vvproc_top *top, VerilatedTrace_t *tfp, FILE *fcsv);

int main(int argc, char **argv) {
    fprintf(stderr, "Starting Verilator Main()\n");
    
    //////////////////////////
    //Check validity and parse input arguments
    //////////////////////////
    int exit_code = 0;
    
    if (argc != 7 && argc != 8) {
        fprintf(stderr, "Usage: %s PROG_PATHS_LIST MEM_W MEM_SZ MEM_LATENCY EXTRA_CYCLES TEST_OUTPUT_PATH [VCD WAVEFORM_FILE]\n", argv[0]);
        return 1;
    }

    int mem_w, mem_sz, mem_latency, extra_cycles;
    {
        char *endptr;
        mem_w = strtol(argv[2], &endptr, 10);
        if (mem_w == 0 || *endptr != 0) {
            fprintf(stderr, "ERROR: invalid MEM_W argument\n");
            return 1;
        }
        mem_sz = strtol(argv[3], &endptr, 10);
        if (mem_sz == 0 || *endptr != 0) {
            fprintf(stderr, "ERROR: invalid MEM_SZ argument\n");
            return 1;
        }
        mem_latency = strtol(argv[4], &endptr, 10);
        if (*endptr != 0) {
            fprintf(stderr, "ERROR: invalid MEM_LATENCY argument\n");
            return 1;
        }
        extra_cycles = strtol(argv[5], &endptr, 10);
        if (*endptr != 0) {
            fprintf(stderr, "ERROR: invalid EXTRA_CYCLES argument\n");
            return 1;
        }
    }

    Verilated::traceEverOn(true);
    //Verilated::commandArgs(argc, argv);

    FILE *fprogs = fopen(argv[1], "r");
    if (fprogs == NULL) {
        fprintf(stderr, "ERROR: opening `%s': %s\n", argv[1], strerror(errno));
        return 2;
    }

    //Program UART Outputs
    FILE *flog = fopen(argv[6], "w");


    //////////////////////////
    //Allocate memory latency buffers
    //////////////////////////
    bool *mem_rvalid_queue = (bool *)malloc(sizeof(bool) * mem_latency);
    unsigned char **mem_rdata_queue  = (unsigned char **)malloc(sizeof(unsigned char *) * mem_latency); //memory data port
    bool *mem_err_queue    = (bool *)malloc(sizeof(bool) * mem_latency);

    for(int queue_pos = 0; queue_pos < mem_latency; queue_pos++)
    {
        mem_rdata_queue[queue_pos] = (unsigned char *)malloc(sizeof(unsigned char) * mem_w/8);
    }

    bool *mem_ivalid_queue = (bool *)malloc(sizeof(bool) * mem_latency);
    unsigned char **mem_idata_queue    = (unsigned char **)malloc(sizeof(unsigned char *) * mem_latency); //memory instruction port
    bool *mem_ierr_queue    = (bool *)malloc(sizeof(bool) * mem_latency);
    //even though known instruction interface width of 32 bits, malloc like this for compatability with memory management helper functions
    for(int queue_pos = 0; queue_pos < mem_latency; queue_pos++)
    {
        mem_idata_queue[queue_pos] = (unsigned char *)malloc(sizeof(unsigned char) * 32/8);
    }

    Vvproc_top *top = new Vvproc_top;
    
    //////////////////////////
    //Setup vcd trace file
    //////////////////////////
    VerilatedTrace_t *tfp = NULL;
    if (argc == 8) {
        #ifdef TRACE_VCD
        tfp = new VerilatedTrace_t;
        top->trace(tfp, 99);  // Trace 99 levels of hierarchy
        tfp->open(argv[7]);
        #endif
    }

    //////////////////////////
    //Read file containing program paths : TODO - deprecate this, just pass path directly
    //////////////////////////
    char *line = NULL, *prog_path = NULL;
    size_t line_sz = 0;
    getline(&line, &line_sz, fprogs);
    // allocate sufficient storage space for the four paths (length of the
    // line, or at least 32 bytes)
    if (line_sz < 32) {
        line_sz = 32;
    }
    prog_path = (char *)realloc(prog_path, line_sz);

    int items;
    items = sscanf(line, "%s", prog_path);
    if (items == 0 || items == EOF) {
        return -1;
    }

    unsigned char *mem = load_program(mem_sz, prog_path);

    //////////////////////////
    //Begin Program execution
    //////////////////////////
    
    for (int i = 0; i < mem_latency; i++) {
        mem_rvalid_queue[i] = 0;
    }
    top->mem_rvalid_i = 0;
    top->clk_i        = 0;
    top->rst_ni       = 0;
    for (int i = 0; i < 10; i++) {
        advance_cycle(top);
    }
    top->rst_ni = 1;
    top->eval();
        
    bool main_reached = true; //detect if main has been reached to begin collecting statistics
    
    //////////////////////////
    //Program Execution - Infinite loop with defined exit conditions
    //////////////////////////
    fprintf(stderr, "Beginning Simulation\n");
    while (true) {
        
        
        //////////////////////////
        //Update Memory interfaces
        //////////////////////////
        //Update write interface
        update_mem_write(top->mem_addr_o, (top->mem_req_o && top->mem_we_o), mem_w, mem_sz, (unsigned char*)&(top->mem_wdata_o), (unsigned char*)&(top->mem_be_o), mem);
        //Update read interface TODO - STALL IF (top->mem_req_o && !top->mem_we_o).  Original Vicuna also did not contain this condition
        update_mem_load(top->mem_addr_o, (top->mem_req_o), mem_w, mem_latency, mem_sz, (unsigned char*)&(top->mem_rdata_i), (bool*)&(top->mem_rvalid_i), (bool*)&(top->mem_err_i), mem_rdata_queue, mem_rvalid_queue, mem_err_queue, mem);

        //Update instruction memory interface
        update_mem_load(top->mem_iaddr_o, top->mem_ireq_o, 32, mem_latency, mem_sz, (unsigned char*)&(top->mem_irdata_i), (bool*)&(top->mem_irvalid_i), (bool*)&(top->mem_ierr_i), mem_idata_queue, mem_ivalid_queue, mem_ierr_queue, mem);

        //////////////////////////
        // Check Memory Mapped IO
        //////////////////////////
        //UART Data Write.  Only takes lowest byte.
        char w_port;
        if (check_memmapio(top->mem_addr_o, (top->mem_req_o && top->mem_we_o), 8, (unsigned char*)&(top->mem_wdata_o), 0x00000400u, &w_port)){
            putc(w_port, stdout);
            putc(w_port, flog);
        }

        //////////////////////////
        // Advance to next clock cycle
        //////////////////////////
        //advance_cycle(top);
        advance_half_cycle(top, 1);
        update_vcd(tfp, 0, 0);
        update_stats(top);
        advance_half_cycle(top, 0);
        update_vcd(tfp, 0, 0);
            
        //////////////////////////
        // Check Exit Conditions
        //////////////////////////
        
        //A jump to address 0x78 is a failed test caused by mismatched output
        if (check_PC(top, 0x00000078u) ) {
            exit_code = 1;
            break;
        }
        //A jump to address 0x74 is a failed test caused by an interrupt being called (all other interrupts also funnel here)
        if (check_PC(top, 0x000000074u) ) {
            fprintf(stderr, "ERROR: TEST FAILURE - Interrupt Called\n");
            exit_code = 1;
            break;
        }
        
        if (check_PC(top,  0x0000007Cu)) {
            break;
        }

        if (check_stall(top, 10000)){
            fprintf(stderr, "ERROR: Program stalled for 10000 cycles\n");
            exit_code = 1;
            break;
        }
        ////////////////////////
        // Statistics and Traces
        ////////////////////////

        // update all stats
        update_stats(top);
        // update vcd trace
        //update_vcd(tfp, 0, 0);
    
            
    }
    
    ////////////////////////
    // Close all files and free all allocated memory
    ////////////////////////
    #ifdef TRACE_VCD
    if (tfp != NULL)
        tfp->close();
    #endif 
    
    top->final();
    free(prog_path);
    free(line);
    free(mem);
    free(mem_rvalid_queue);
    for(int queue_pos = 0; queue_pos < mem_latency; queue_pos++)
    {
        free(mem_rdata_queue[queue_pos]);
        free(mem_idata_queue[queue_pos]);
    }
    free(mem_rdata_queue);
    free(mem_idata_queue);
    free(mem_err_queue);

    fclose(fprogs);
    fclose(flog);

    return exit_code;
}